/**
 * @file parser_xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML data parser for libyang
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "context.h"
#include "dict.h"
#include "log.h"
#include "plugins_types.h"
#include "set.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "xml.h"
#include "validation.h"

/**
 * @brief Internal context for XML YANG data parser.
 */
struct lyd_xml_ctx {
    struct lyxml_ctx *xmlctx;        /**< XML context */

    uint32_t options;                /**< various @ref dataparseroptions. */
    uint32_t path_len;               /**< used bytes in the path buffer */
#define LYD_PARSER_BUFSIZE 4078
    char path[LYD_PARSER_BUFSIZE];   /**< buffer for the generated path */
    struct ly_set unres_node_type;   /**< set of nodes validated with LY_EINCOMPLETE result */
    struct ly_set unres_meta_type;   /**< set of metadata validated with LY_EINCOMPLETE result */
    struct ly_set when_check;        /**< set of nodes with "when" conditions */
};

/**
 * @brief XML-parser's implementation of ly_type_resolve_prefix() callback to provide mapping between prefixes used
 * in the values to the schema via XML namespaces.
 */
static const struct lys_module *
lydxml_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *parser)
{
    const struct lyxml_ns *ns;
    struct lyxml_ctx *xmlctx = (struct lyxml_ctx *)parser;

    ns = lyxml_ns_get(xmlctx, prefix, prefix_len);
    if (!ns) {
        return NULL;
    }

    return ly_ctx_get_module_implemented_ns(ctx, ns->uri);
}

static LY_ERR
lydxml_metadata(struct lyxml_ctx *xmlctx, const struct lysc_node *sparent, int strict, struct ly_set *type_meta_check,
                struct lyd_meta **meta)
{
    LY_ERR ret = LY_EVALID;
    const struct lyxml_ns *ns;
    struct lys_module *mod;
    const char *name;
    size_t name_len;

    *meta = NULL;

    while (xmlctx->status == LYXML_ATTRIBUTE) {
        if (!xmlctx->prefix_len) {
            /* in XML, all attributes must be prefixed
             * TODO exception for NETCONF filters which are supposed to map to the ietf-netconf without prefix */
            if (strict) {
                LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_REFERENCE, "Missing mandatory prefix for XML metadata \"%.*s\".",
                       xmlctx->name_len, xmlctx->name);
                goto cleanup;
            }

skip_attr:
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            assert(xmlctx->status == LYXML_ATTR_CONTENT);
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            continue;
        }

        /* get namespace of the attribute to find its annotation definition */
        ns = lyxml_ns_get(xmlctx, xmlctx->prefix, xmlctx->prefix_len);
        if (!ns) {
            /* unknown namespace, XML error */
            LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".",
                   xmlctx->prefix_len, xmlctx->prefix);
            goto cleanup;
        }
        mod = ly_ctx_get_module_implemented_ns(xmlctx->ctx, ns->uri);
        if (!mod) {
            /* module is not implemented or not present in the schema */
            if (strict) {
                LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_REFERENCE,
                       "Unknown (or not implemented) YANG module with namespace \"%s\" for metadata \"%.*s%s%.*s\".",
                       ns->uri, xmlctx->prefix_len, xmlctx->prefix, xmlctx->prefix_len ? ":" : "", xmlctx->name_len,
                       xmlctx->name);
                goto cleanup;
            }
            goto skip_attr;
        }

        /* remember attr name and get its content */
        name = xmlctx->name;
        name_len = xmlctx->name_len;
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        assert(xmlctx->status == LYXML_ATTR_CONTENT);

        /* create metadata */
        ret = lyd_create_meta(NULL, meta, mod, name, name_len, xmlctx->value, xmlctx->value_len, &xmlctx->dynamic,
                              lydxml_resolve_prefix, xmlctx, LYD_XML, sparent);
        if (ret == LY_EINCOMPLETE) {
            if (type_meta_check) {
                ly_set_add(type_meta_check, meta, LY_SET_OPT_USEASLIST);
            }
        } else if (ret) {
            goto cleanup;
        }

        /* next attribute */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
    }

    ret = LY_SUCCESS;

cleanup:
    if (ret) {
        lyd_free_meta(xmlctx->ctx, *meta, 1);
        *meta = NULL;
    }
    return ret;
}

static LY_ERR
lydxml_attrs(struct lyxml_ctx *xmlctx, struct ly_attr **attr)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lyxml_ns *ns;
    struct ly_prefix *val_prefs;
    struct ly_attr *attr2;
    const char *name, *prefix;
    size_t name_len, prefix_len;

    assert(attr);
    *attr = NULL;

    while (xmlctx->status == LYXML_ATTRIBUTE) {
        ns = NULL;
        if (xmlctx->prefix_len) {
            /* get namespace of the attribute */
            ns = lyxml_ns_get(xmlctx, xmlctx->prefix, xmlctx->prefix_len);
            if (!ns) {
                LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".",
                       xmlctx->prefix_len, xmlctx->prefix);
                ret = LY_EVALID;
                goto cleanup;
            }
        }

        if (*attr) {
            attr2 = *attr;
        } else {
            attr2 = NULL;
        }

        /* remember attr prefix, name, and get its content */
        prefix = xmlctx->prefix;
        prefix_len = xmlctx->prefix_len;
        name = xmlctx->name;
        name_len = xmlctx->name_len;
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        assert(xmlctx->status == LYXML_ATTR_CONTENT);

        /* get value prefixes */
        LY_CHECK_GOTO(ret = lyxml_get_prefixes(xmlctx, xmlctx->value, xmlctx->value_len, &val_prefs), cleanup);

        /* attr2 is always changed to the created attribute */
        ret = ly_create_attr(NULL, &attr2, xmlctx->ctx, name, name_len, xmlctx->value, xmlctx->value_len,
                             &xmlctx->dynamic, LYD_XML, val_prefs, prefix, prefix_len, ns ? ns->uri : NULL);
        LY_CHECK_GOTO(ret, cleanup);

        if (!*attr) {
            *attr = attr2;
        }

        /* next attribute */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
    }

cleanup:
    if (ret) {
        ly_free_attr(xmlctx->ctx, *attr, 1);
        *attr = NULL;
    }
    return ret;
}

static LY_ERR
lydxml_check_list(struct lyxml_ctx *xmlctx, const struct lysc_node *list)
{
    LY_ERR ret = LY_SUCCESS, r;
    enum LYXML_PARSER_STATUS next;
    struct ly_set key_set = {0};
    const struct lysc_node *snode;
    uint32_t i, parents_count;

    assert(list && (list->nodetype == LYS_LIST));

    /* get all keys into a set (keys do not have if-features or anything) */
    snode = NULL;
    while ((snode = lys_getnext(snode, list, NULL, LYS_GETNEXT_NOSTATECHECK)) && (snode->flags & LYS_KEY)) {
        ly_set_add(&key_set, (void *)snode, LY_SET_OPT_USEASLIST);
    }

    while (xmlctx->status == LYXML_ELEMENT) {
        /* find key definition */
        for (i = 0; i < key_set.count; ++i) {
            snode = (const struct lysc_node *)key_set.objs[i];
            if (!ly_strncmp(snode->name, xmlctx->name, xmlctx->name_len)) {
                break;
            }
        }
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

        /* skip attributes */
        while (xmlctx->status == LYXML_ATTRIBUTE) {
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            assert(xmlctx->status == LYXML_ATTR_CONTENT);
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        }

        assert(xmlctx->status == LYXML_ELEM_CONTENT);
        if (i < key_set.count) {
            /* validate the value */
            r = lys_value_validate(NULL, snode, xmlctx->value, xmlctx->value_len, lydxml_resolve_prefix, xmlctx, LYD_XML);
            if (!r) {
                /* key with a valid value, remove from the set */
                ly_set_rm_index(&key_set, i, NULL);
            }
        }

        /* parser next */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

        /* skip any children, resursively */
        parents_count = xmlctx->elements.count;
        while ((parents_count < xmlctx->elements.count) || (xmlctx->status == LYXML_ELEMENT)) {
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        }

        /* parser next, but do not parse closing element of the list because it would remove its namespaces */
        assert(xmlctx->status == LYXML_ELEM_CLOSE);
        LY_CHECK_GOTO(ret = lyxml_ctx_peek(xmlctx, &next), cleanup);
        if (next != LYXML_ELEM_CLOSE) {
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        }
    }

    if (key_set.count) {
        /* some keys are missing/did not validate */
        ret = LY_ENOT;
    }

cleanup:
    ly_set_erase(&key_set, NULL);
    return ret;
}

/**
 * @brief Parse XML elements as YANG data node children the specified parent node.
 *
 * @param[in] lydctx XML YANG data parser context.
 * @param[in] parent Parent node where the children are inserted. NULL in case of parsing top-level elements.
 * @param[in,out] data Pointer to the XML string representation of the YANG data to parse.
 * @param[out] node Resulting list of the parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_data_r(struct lyd_xml_ctx *lydctx, struct lyd_node_inner *parent, const char **data, struct lyd_node **first)
{
    LY_ERR ret = LY_SUCCESS;
    enum LYXML_PARSER_STATUS prev_status;
    const char *prefix, *name, *prev_input, *pname, *pprefix;
    size_t prefix_len, name_len, pprefix_len, pname_len;
    struct lyxml_ctx *xmlctx;
    const struct ly_ctx *ctx;
    const struct lyxml_ns *ns;
    struct lyd_meta *meta = NULL, *meta2, *prev_meta;
    struct ly_attr *attr = NULL;
    const struct lysc_node *snode;
    struct lys_module *mod;
    uint32_t prev_opts, parents_count;
    struct lyd_node *cur = NULL, *anchor;
    struct ly_prefix *val_prefs;

    xmlctx = lydctx->xmlctx;
    ctx = xmlctx->ctx;

    while (xmlctx->status == LYXML_ELEMENT) {
        /* remember element prefix and name */
        prefix = xmlctx->prefix;
        prefix_len = xmlctx->prefix_len;
        name = xmlctx->name;
        name_len = xmlctx->name_len;

        /* get the element module */
        ns = lyxml_ns_get(xmlctx, prefix, prefix_len);
        if (!ns) {
            LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".",
                   prefix_len, prefix);
            ret = LY_EVALID;
            goto cleanup;
        }
        mod = ly_ctx_get_module_implemented_ns(ctx, ns->uri);
        if (!mod && (lydctx->options & LYD_OPT_STRICT)) {
            LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_REFERENCE, "No module with namespace \"%s\" in the context.", ns->uri);
            ret = LY_EVALID;
            goto cleanup;
        }

        /* get the schema node */
        snode = NULL;
        if (mod && (!parent || parent->schema)) {
            /* leave if-feature check for validation */
            snode = lys_find_child(parent ? parent->schema : NULL, mod, name, name_len, 0, LYS_GETNEXT_NOSTATECHECK);
            if (!snode) {
                if (lydctx->options & LYD_OPT_STRICT) {
                    LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_REFERENCE, "Element \"%.*s\" not found in the \"%s\" module.",
                        name_len, name, mod->name);
                    ret = LY_EVALID;
                    goto cleanup;
                } else if (!(lydctx->options & LYD_OPT_OPAQ)) {
                    /* remember current number of parents */
                    parents_count = xmlctx->elements.count;

                    /* skip after the content */
                    while (xmlctx->status != LYXML_ELEM_CONTENT) {
                        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
                    }
                    LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

                    /* skip all children elements, recursively, if any */
                    while (parents_count < xmlctx->elements.count) {
                        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
                    }

                    /* close element */
                    assert(xmlctx->status == LYXML_ELEM_CLOSE);
                    LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
                    continue;
                }
            }
            if (snode) {
                if ((lydctx->options & LYD_OPT_NO_STATE) && (snode->flags & LYS_CONFIG_R)) {
                    LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_INSTATE, snode->name);
                    ret = LY_EVALID;
                    goto cleanup;
                }
                if (snode->nodetype & (LYS_ACTION | LYS_NOTIF)) {
                    LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_DATA, "Unexpected %s element \"%.*s\".",
                           snode->nodetype == LYS_ACTION ? "RPC/action" : "notification", name_len, name);
                    ret = LY_EVALID;
                    goto cleanup;
                }
            }
        }

        /* parser next */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

        if (snode && (lydctx->options & LYD_OPT_OPAQ) && (snode->nodetype & (LYD_NODE_TERM | LYS_LIST))) {
            /* backup parser */
            prev_status = xmlctx->status;
            pprefix = xmlctx->prefix;
            pprefix_len = xmlctx->prefix_len;
            pname = xmlctx->name;
            pname_len = xmlctx->name_len;
            prev_input = xmlctx->input;
            if ((xmlctx->status == LYXML_ELEM_CONTENT) && xmlctx->dynamic) {
                /* it was backed up, do not free */
                xmlctx->dynamic = 0;
            }

            /* skip attributes */
            while (xmlctx->status == LYXML_ATTRIBUTE) {
                LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
                LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            }

            if (snode->nodetype & LYD_NODE_TERM) {
                /* value may not be valid in which case we parse it as an opaque node */
                if (lys_value_validate(NULL, snode, xmlctx->value, xmlctx->value_len, lydxml_resolve_prefix, xmlctx, LYD_XML)) {
                    snode = NULL;
                }
            } else {
                /* skip content */
                LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

                if (lydxml_check_list(xmlctx, snode)) {
                    /* invalid list, parse as opaque if it does */
                    snode = NULL;
                }
            }

            /* restore parser */
            if (xmlctx->dynamic) {
                free((char *)xmlctx->value);
            }
            xmlctx->status = prev_status;
            xmlctx->prefix = pprefix;
            xmlctx->prefix_len = pprefix_len;
            xmlctx->name = pname;
            xmlctx->name_len = pname_len;
            xmlctx->input = prev_input;
        }

        /* create metadata/attributes */
        if (xmlctx->status == LYXML_ATTRIBUTE) {
            if (snode) {
                ret = lydxml_metadata(xmlctx, snode, lydctx->options & LYD_OPT_STRICT, &lydctx->unres_meta_type, &meta);
                LY_CHECK_GOTO(ret, cleanup);
            } else {
                assert(lydctx->options & LYD_OPT_OPAQ);
                ret = lydxml_attrs(xmlctx, &attr);
                LY_CHECK_GOTO(ret, cleanup);
            }
        }

        assert(xmlctx->status == LYXML_ELEM_CONTENT);
        if (!snode) {
            assert(lydctx->options & LYD_OPT_OPAQ);

            if (xmlctx->ws_only) {
                /* ignore WS-only value */
                xmlctx->value_len = 0;
                val_prefs = NULL;
            } else {
                /* get value prefixes */
                ret = lyxml_get_prefixes(xmlctx, xmlctx->value, xmlctx->value_len, &val_prefs);
                LY_CHECK_GOTO(ret, cleanup);
            }

            /* create node */
            ret = lyd_create_opaq(ctx, name, name_len, xmlctx->value, xmlctx->value_len, &xmlctx->dynamic, LYD_XML,
                                  val_prefs, prefix, prefix_len, ns->uri, &cur);
            LY_CHECK_GOTO(ret, cleanup);

            /* parser next */
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

            /* process children */
            if (xmlctx->status == LYXML_ELEMENT) {
                ret = lydxml_data_r(lydctx, (struct lyd_node_inner *)cur, data, lyd_node_children_p(cur));
                LY_CHECK_GOTO(ret, cleanup);
            }
        } else if (snode->nodetype & LYD_NODE_TERM) {
            /* create node */
            ret = lyd_create_term(snode, xmlctx->value, xmlctx->value_len, &xmlctx->dynamic, lydxml_resolve_prefix,
                                  xmlctx, LYD_XML, &cur);
            if (ret == LY_EINCOMPLETE) {
                if (!(lydctx->options & LYD_OPT_PARSE_ONLY)) {
                    ly_set_add(&lydctx->unres_node_type, cur, LY_SET_OPT_USEASLIST);
                }
            } else if (ret) {
                goto cleanup;
            }

            if (parent && (cur->schema->flags & LYS_KEY)) {
                /* check the key order, the anchor must always be the last child */
                anchor = lyd_get_prev_key_anchor(parent->child, cur->schema);
                if ((!anchor && parent->child) || (anchor && anchor->next)) {
                    if (lydctx->options & LYD_OPT_STRICT) {
                        LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_DATA, "Invalid position of the key \"%s\" in a list.",
                                cur->schema->name);
                        ret = LY_EVALID;
                        goto cleanup;
                    } else {
                        LOGWRN(ctx, "Invalid position of the key \"%s\" in a list.", cur->schema->name);
                    }
                }
            }

            /* parser next */
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

            /* no children expected */
            if (xmlctx->status == LYXML_ELEMENT) {
                LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX, "Child element inside a terminal node \"%s\" found.",
                       snode->name);
                ret = LY_EVALID;
                goto cleanup;
            }
        } else if (snode->nodetype & LYD_NODE_INNER) {
            if (!xmlctx->ws_only) {
                /* value in inner node */
                LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX, "Text value inside an inner node \"%s\" found.",
                       snode->name);
                ret = LY_EVALID;
                goto cleanup;
            }

            /* create node */
            ret = lyd_create_inner(snode, &cur);
            LY_CHECK_GOTO(ret, cleanup);

            /* parser next */
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

            /* process children */
            if (xmlctx->status == LYXML_ELEMENT) {
                ret = lydxml_data_r(lydctx, (struct lyd_node_inner *)cur, data, lyd_node_children_p(cur));
                LY_CHECK_GOTO(ret, cleanup);
            }

            if (snode->nodetype == LYS_LIST) {
                /* check all keys exist */
                LY_CHECK_GOTO(ret = lyd_parse_check_keys(cur), cleanup);
            }

            if (!(lydctx->options & LYD_OPT_PARSE_ONLY)) {
                /* new node validation, autodelete CANNOT occur, all nodes are new */
                ret = lyd_validate_new(lyd_node_children_p(cur), snode, NULL);
                LY_CHECK_GOTO(ret, cleanup);

                /* add any missing default children */
                ret = lyd_validate_defaults_r((struct lyd_node_inner *)cur, lyd_node_children_p(cur), NULL, NULL,
                                              &lydctx->unres_node_type, &lydctx->when_check, lydctx->options);
                LY_CHECK_GOTO(ret, cleanup);
            }

            /* hash now that all keys should be parsed, rehash for key-less list */
            if (snode->nodetype == LYS_LIST) {
                lyd_hash(cur);
            }
        } else if (snode->nodetype & LYD_NODE_ANY) {
            if (!xmlctx->ws_only) {
                /* value in inner node */
                LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX, "Text value inside an any node \"%s\" found.",
                       snode->name);
                ret = LY_EVALID;
                goto cleanup;
            }

            /* parser next */
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

            /* parse any data tree with correct options */
            prev_opts = lydctx->options;
            lydctx->options &= ~LYD_OPT_STRICT;
            lydctx->options |= LYD_OPT_OPAQ;
            anchor = NULL;
            ret = lydxml_data_r(lydctx, NULL, data, &anchor);
            lydctx->options = prev_opts;
            LY_CHECK_GOTO(ret, cleanup);

            /* create node */
            ret = lyd_create_any(snode, anchor, LYD_ANYDATA_DATATREE, &cur);
            LY_CHECK_GOTO(ret, cleanup);
        }

        /* correct flags */
        if (snode) {
            if (!(snode->nodetype & (LYS_ACTION | LYS_NOTIF)) && snode->when) {
                if (lydctx->options & LYD_OPT_TRUSTED) {
                    /* just set it to true */
                    cur->flags |= LYD_WHEN_TRUE;
                } else {
                    /* remember we need to evaluate this node's when */
                    ly_set_add(&lydctx->when_check, cur, LY_SET_OPT_USEASLIST);
                }
            }
            if (lydctx->options & LYD_OPT_TRUSTED) {
                /* node is valid */
                cur->flags &= ~LYD_NEW;
            }
            prev_meta = NULL;
            LY_LIST_FOR(meta, meta2) {
                if (!strcmp(meta2->name, "default") && !strcmp(meta2->annotation->module->name, "ietf-netconf-with-defaults")
                        && meta2->value.boolean) {
                    /* node is default according to the metadata */
                    cur->flags |= LYD_DEFAULT;

                    /* delete the metadata */
                    if (prev_meta) {
                        prev_meta->next = meta2->next;
                    } else {
                        meta = meta->next;
                    }
                    lyd_free_meta(ctx, meta2, 0);
                    break;
                }

                prev_meta = meta2;
            }
        }

        /* add metadata/attributes */
        if (snode) {
            cur->meta = meta;
            meta = NULL;
        } else {
            assert(!cur->schema);
            ((struct lyd_node_opaq *)cur)->attr = attr;
            attr = NULL;
        }

        /* insert */
        lyd_insert_node((struct lyd_node *)parent, first, cur);
        cur = NULL;

        /* parser next */
        assert(xmlctx->status == LYXML_ELEM_CLOSE);
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
    }

    /* success */
    ret = LY_SUCCESS;

cleanup:
    lyd_free_meta(ctx, meta, 1);
    ly_free_attr(ctx, attr, 1);
    lyd_free_tree(cur);
    if (ret && *first) {
        lyd_free_siblings(*first);
        *first = NULL;
    }
    return ret;
}

LY_ERR
lyd_parse_xml_data(struct ly_ctx *ctx, const char *data, int options, struct lyd_node **tree)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_xml_ctx lydctx = {0};
    uint32_t i = 0;
    const struct lys_module *mod;
    struct lyd_node *first, *next, **first2;

    /* init context and tree */
    LY_CHECK_GOTO(ret = lyxml_ctx_new(ctx, data, &lydctx.xmlctx), cleanup);
    lydctx.options = options;
    *tree = NULL;

    /* parse XML data */
    ret = lydxml_data_r(&lydctx, NULL, &data, tree);
    LY_CHECK_GOTO(ret, cleanup);

    if (!(options & LYD_OPT_PARSE_ONLY)) {
        next = *tree;
        while (1) {
            if (options & LYD_VALOPT_DATA_ONLY) {
                mod = lyd_data_next_module(&next, &first);
            } else {
                mod = lyd_mod_next_module(next, NULL, 0, ctx, &i, &first);
            }
            if (!mod) {
                break;
            }
            if (first == *tree) {
                /* make sure first2 changes are carried to tree */
                first2 = tree;
            } else {
                first2 = &first;
            }

            /* validate new top-level nodes, autodelete CANNOT occur, all nodes are new */
            ret = lyd_validate_new(first2, NULL, mod);
            LY_CHECK_GOTO(ret, cleanup);

            /* add all top-level defaults for this module */
            ret = lyd_validate_defaults_r(NULL, first2, NULL, mod, &lydctx.unres_node_type, &lydctx.when_check,
                                          options & LYD_VALOPT_MASK);
            LY_CHECK_GOTO(ret, cleanup);

            /* finish incompletely validated terminal values/attributes and when conditions */
            ret = lyd_validate_unres(tree, &lydctx.when_check, &lydctx.unres_node_type, &lydctx.unres_meta_type, LYD_XML,
                                     lydxml_resolve_prefix, lydctx.xmlctx);
            LY_CHECK_GOTO(ret, cleanup);

            /* perform final validation that assumes the data tree is final */
            ret = lyd_validate_siblings_r(*first2, NULL, mod, options & LYD_VALOPT_MASK);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    /* there should be no unresolved types stored */
    assert(!(options & LYD_OPT_PARSE_ONLY) || (!lydctx.unres_node_type.count && !lydctx.unres_meta_type.count
           && !lydctx.when_check.count));

    ly_set_erase(&lydctx.unres_node_type, NULL);
    ly_set_erase(&lydctx.unres_meta_type, NULL);
    ly_set_erase(&lydctx.when_check, NULL);
    lyxml_ctx_free(lydctx.xmlctx);
    if (ret) {
        lyd_free_all(*tree);
        *tree = NULL;
    }
    return ret;
}
