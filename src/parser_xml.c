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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "in_internal.h"
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "set.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "validation.h"
#include "xml.h"

/**
 * @brief Internal context for XML YANG data parser.
 *
 * Note that the structure maps to the lyd_ctx which is common for all the data parsers
 */
struct lyd_xml_ctx {
    uint32_t parse_options;        /**< various @ref dataparseroptions. */
    uint32_t validate_options;     /**< various @ref datavalidationoptions. */
    uint32_t int_opts;             /**< internal data parser options */
    uint32_t path_len;             /**< used bytes in the path buffer */
    char path[LYD_PARSER_BUFSIZE]; /**< buffer for the generated path */
    struct ly_set node_types;      /**< set of nodes validated with LY_EINCOMPLETE result */
    struct ly_set meta_types;      /**< set of metadata validated with LY_EINCOMPLETE result */
    struct ly_set node_when;       /**< set of nodes with "when" conditions */
    struct lyd_node *op_node;      /**< if an RPC/action/notification is being parsed, store the pointer to it */

    /* callbacks */
    lyd_ctx_free_clb free;           /* destructor */

    struct lyxml_ctx *xmlctx;      /**< XML context */
};

void
lyd_xml_ctx_free(struct lyd_ctx *lydctx)
{
    struct lyd_xml_ctx *ctx = (struct lyd_xml_ctx *)lydctx;

    lyd_ctx_free(lydctx);
    lyxml_ctx_free(ctx->xmlctx);
    free(ctx);
}

static LY_ERR
lydxml_metadata(struct lyd_xml_ctx *lydctx, struct lyd_meta **meta)
{
    LY_ERR ret = LY_EVALID;
    const struct lyxml_ns *ns;
    struct lys_module *mod;
    const char *name;
    size_t name_len;
    struct lyxml_ctx *xmlctx = lydctx->xmlctx;

    *meta = NULL;

    while (xmlctx->status == LYXML_ATTRIBUTE) {
        if (!xmlctx->prefix_len) {
            /* in XML, all attributes must be prefixed
             * TODO exception for NETCONF filters which are supposed to map to the ietf-netconf without prefix */
            if (lydctx->parse_options & LYD_PARSE_STRICT) {
                LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Missing mandatory prefix for XML metadata \"%.*s\".",
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
        ns = lyxml_ns_get(&xmlctx->ns, xmlctx->prefix, xmlctx->prefix_len);
        if (!ns) {
            /* unknown namespace, XML error */
            LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", xmlctx->prefix_len, xmlctx->prefix);
            goto cleanup;
        }
        mod = ly_ctx_get_module_implemented_ns(xmlctx->ctx, ns->uri);
        if (!mod) {
            /* module is not implemented or not present in the schema */
            if (lydctx->parse_options & LYD_PARSE_STRICT) {
                LOGVAL(xmlctx->ctx, LYVE_REFERENCE,
                        "Unknown (or not implemented) YANG module with namespace \"%s\" for metadata \"%.*s%s%.*s\".",
                        ns->uri, xmlctx->prefix_len, xmlctx->prefix, xmlctx->prefix_len ? ":" : "", xmlctx->name_len,
                        xmlctx->name);
                goto cleanup;
            }
            goto skip_attr;
        }

        /* remember meta name and get its content */
        name = xmlctx->name;
        name_len = xmlctx->name_len;
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        assert(xmlctx->status == LYXML_ATTR_CONTENT);

        /* create metadata */
        ret = lyd_parser_create_meta((struct lyd_ctx *)lydctx, NULL, meta, mod, name, name_len, xmlctx->value,
                xmlctx->value_len, &xmlctx->dynamic, LY_PREF_XML, &xmlctx->ns, LYD_HINT_DATA);
        LY_CHECK_GOTO(ret, cleanup);

        /* next attribute */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
    }

    ret = LY_SUCCESS;

cleanup:
    if (ret) {
        lyd_free_meta_siblings(*meta);
        *meta = NULL;
    }
    return ret;
}

static LY_ERR
lydxml_attrs(struct lyxml_ctx *xmlctx, struct lyd_attr **attr)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lyxml_ns *ns;
    void *val_prefix_data;
    LY_PREFIX_FORMAT format;
    struct lyd_attr *attr2;
    const char *name, *prefix;
    size_t name_len, prefix_len;

    assert(attr);
    *attr = NULL;

    while (xmlctx->status == LYXML_ATTRIBUTE) {
        ns = NULL;
        if (xmlctx->prefix_len) {
            /* get namespace of the attribute */
            ns = lyxml_ns_get(&xmlctx->ns, xmlctx->prefix, xmlctx->prefix_len);
            if (!ns) {
                LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", xmlctx->prefix_len, xmlctx->prefix);
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
        LY_CHECK_GOTO(ret = ly_store_prefix_data(xmlctx->ctx, xmlctx->value, xmlctx->value_len, LY_PREF_XML,
                &xmlctx->ns, &format, &val_prefix_data), cleanup);

        /* attr2 is always changed to the created attribute */
        ret = lyd_create_attr(NULL, &attr2, xmlctx->ctx, name, name_len, prefix, prefix_len, ns ? ns->uri : NULL,
                ns ? strlen(ns->uri) : 0, xmlctx->value, xmlctx->value_len, &xmlctx->dynamic, format, val_prefix_data, 0);
        LY_CHECK_GOTO(ret, cleanup);

        if (!*attr) {
            *attr = attr2;
        }

        /* next attribute */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
    }

cleanup:
    if (ret) {
        lyd_free_attr_siblings(xmlctx->ctx, *attr);
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
    while ((snode = lys_getnext(snode, list, NULL, 0)) && (snode->flags & LYS_KEY)) {
        ret = ly_set_add(&key_set, (void *)snode, 1, NULL);
        LY_CHECK_GOTO(ret, cleanup);
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
            r = _lys_value_validate(NULL, snode, xmlctx->value, xmlctx->value_len, LY_PREF_XML, &xmlctx->ns);
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

static LY_ERR
lydxml_data_skip(struct lyxml_ctx *xmlctx)
{
    uint32_t parents_count;

    /* remember current number of parents */
    parents_count = xmlctx->elements.count;

    /* skip after the content */
    while (xmlctx->status != LYXML_ELEM_CONTENT) {
        LY_CHECK_RET(lyxml_ctx_next(xmlctx));
    }
    LY_CHECK_RET(lyxml_ctx_next(xmlctx));

    /* skip all children elements, recursively, if any */
    while (parents_count < xmlctx->elements.count) {
        LY_CHECK_RET(lyxml_ctx_next(xmlctx));
    }

    /* close element */
    assert(xmlctx->status == LYXML_ELEM_CLOSE);
    LY_CHECK_RET(lyxml_ctx_next(xmlctx));

    return LY_SUCCESS;
}

static LY_ERR
lydxml_data_check_opaq(struct lyd_xml_ctx *lydctx, const struct lysc_node **snode)
{
    LY_ERR ret = LY_SUCCESS;
    enum LYXML_PARSER_STATUS prev_status;
    const char *prev_current, *pname, *pprefix;
    size_t pprefix_len, pname_len;
    struct lyxml_ctx *xmlctx = lydctx->xmlctx;

    if ((lydctx->parse_options & LYD_PARSE_OPAQ) && ((*snode)->nodetype & (LYD_NODE_TERM | LYS_LIST))) {
        /* backup parser */
        prev_status = xmlctx->status;
        pprefix = xmlctx->prefix;
        pprefix_len = xmlctx->prefix_len;
        pname = xmlctx->name;
        pname_len = xmlctx->name_len;
        prev_current = xmlctx->in->current;
        if ((xmlctx->status == LYXML_ELEM_CONTENT) && xmlctx->dynamic) {
            /* it was backed up, do not free */
            xmlctx->dynamic = 0;
        }

        /* skip attributes */
        while (xmlctx->status == LYXML_ATTRIBUTE) {
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), restore);
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), restore);
        }

        if ((*snode)->nodetype & LYD_NODE_TERM) {
            /* value may not be valid in which case we parse it as an opaque node */
            if (_lys_value_validate(NULL, *snode, xmlctx->value, xmlctx->value_len, LY_PREF_XML, &xmlctx->ns)) {
                *snode = NULL;
            }
        } else {
            /* skip content */
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), restore);

            if (lydxml_check_list(xmlctx, *snode)) {
                /* invalid list, parse as opaque if it missing/has invalid some keys */
                *snode = NULL;
            }
        }

restore:
        /* restore parser */
        if (xmlctx->dynamic) {
            free((char *)xmlctx->value);
        }
        xmlctx->status = prev_status;
        xmlctx->prefix = pprefix;
        xmlctx->prefix_len = pprefix_len;
        xmlctx->name = pname;
        xmlctx->name_len = pname_len;
        xmlctx->in->current = prev_current;
    }

    return ret;
}

/**
 * @brief Parse XML subtree.
 *
 * @param[in] lydctx XML YANG data parser context.
 * @param[in] parent Parent node where the children are inserted. NULL in case of parsing top-level elements.
 * @param[out] node Resulting list of the parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_subtree_r(struct lyd_xml_ctx *lydctx, struct lyd_node_inner *parent, struct lyd_node **first_p)
{
    LY_ERR ret = LY_SUCCESS;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    struct lyxml_ctx *xmlctx;
    const struct ly_ctx *ctx;
    const struct lyxml_ns *ns;
    struct lyd_meta *meta = NULL;
    struct lyd_attr *attr = NULL;
    const struct lysc_node *snode;
    struct lys_module *mod;
    uint32_t prev_opts;
    struct lyd_node *node = NULL, *anchor;
    void *val_prefix_data;
    LY_PREFIX_FORMAT format;
    uint32_t getnext_opts;

    xmlctx = lydctx->xmlctx;
    ctx = xmlctx->ctx;
    getnext_opts = lydctx->int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0;

    assert(xmlctx->status == LYXML_ELEMENT);

    /* remember element prefix and name */
    prefix = xmlctx->prefix;
    prefix_len = xmlctx->prefix_len;
    name = xmlctx->name;
    name_len = xmlctx->name_len;

    /* get the element module */
    ns = lyxml_ns_get(&xmlctx->ns, prefix, prefix_len);
    if (!ns) {
        LOGVAL(ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", prefix_len, prefix);
        ret = LY_EVALID;
        goto error;
    }
    mod = ly_ctx_get_module_implemented_ns(ctx, ns->uri);
    if (!mod) {
        if (lydctx->parse_options & LYD_PARSE_STRICT) {
            LOGVAL(ctx, LYVE_REFERENCE, "No module with namespace \"%s\" in the context.", ns->uri);
            ret = LY_EVALID;
            goto error;
        }
        if (!(lydctx->parse_options & LYD_PARSE_OPAQ)) {
            /* skip element with children */
            LY_CHECK_GOTO(ret = lydxml_data_skip(xmlctx), error);
            return LY_SUCCESS;
        }
    }

    /* parser next */
    LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), error);

    /* get the schema node */
    snode = NULL;
    if (mod && (!parent || parent->schema)) {
        snode = lys_find_child(parent ? parent->schema : NULL, mod, name, name_len, 0, getnext_opts);
        if (!snode) {
            if (lydctx->parse_options & LYD_PARSE_STRICT) {
                LOGVAL(ctx, LYVE_REFERENCE, "Element \"%.*s\" not found in the \"%s\" module.", name_len, name, mod->name);
                ret = LY_EVALID;
                goto error;
            } else if (!(lydctx->parse_options & LYD_PARSE_OPAQ)) {
                /* skip element with children */
                LY_CHECK_GOTO(ret = lydxml_data_skip(xmlctx), error);
                return LY_SUCCESS;
            }
        } else {
            /* check that schema node is valid and can be used */
            LY_CHECK_GOTO(ret = lyd_parser_check_schema((struct lyd_ctx *)lydctx, snode), error);
            LY_CHECK_GOTO(ret = lydxml_data_check_opaq(lydctx, &snode), error);
        }
    }

    /* create metadata/attributes */
    if (xmlctx->status == LYXML_ATTRIBUTE) {
        if (snode) {
            ret = lydxml_metadata(lydctx, &meta);
            LY_CHECK_GOTO(ret, error);
        } else {
            assert(lydctx->parse_options & LYD_PARSE_OPAQ);
            ret = lydxml_attrs(xmlctx, &attr);
            LY_CHECK_GOTO(ret, error);
        }
    }

    assert(xmlctx->status == LYXML_ELEM_CONTENT);
    if (!snode) {
        assert(lydctx->parse_options & LYD_PARSE_OPAQ);

        if (xmlctx->ws_only) {
            /* ignore WS-only value */
            xmlctx->value_len = 0;
            val_prefix_data = NULL;
            format = LY_PREF_XML;
        } else {
            /* get value prefixes */
            ret = ly_store_prefix_data(xmlctx->ctx, xmlctx->value, xmlctx->value_len, LY_PREF_XML,
                    &xmlctx->ns, &format, &val_prefix_data);
            LY_CHECK_GOTO(ret, error);
        }

        /* create node */
        ret = lyd_create_opaq(ctx, name, name_len, prefix, prefix_len, ns->uri, strlen(ns->uri), xmlctx->value,
                xmlctx->value_len, &xmlctx->dynamic, format, val_prefix_data, LYD_HINT_DATA, &node);
        LY_CHECK_GOTO(ret, error);

        /* parser next */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), error);

        /* process children */
        while (xmlctx->status == LYXML_ELEMENT) {
            ret = lydxml_subtree_r(lydctx, (struct lyd_node_inner *)node, lyd_node_children_p(node));
            LY_CHECK_GOTO(ret, error);
        }
    } else if (snode->nodetype & LYD_NODE_TERM) {
        /* create node */
        LY_CHECK_GOTO(ret = lyd_parser_create_term((struct lyd_ctx *)lydctx, snode, xmlctx->value, xmlctx->value_len,
                &xmlctx->dynamic, LY_PREF_XML, &xmlctx->ns, LYD_HINT_DATA, &node), error);
        LOG_LOCSET(ctx, snode, node, NULL, NULL);

        if (parent && (node->schema->flags & LYS_KEY)) {
            /* check the key order, the anchor must never be a key */
            anchor = lyd_insert_get_next_anchor(parent->child, node);
            if (anchor && (anchor->schema->flags & LYS_KEY)) {
                if (lydctx->parse_options & LYD_PARSE_STRICT) {
                    LOGVAL(ctx, LYVE_DATA, "Invalid position of the key \"%s\" in a list.", node->schema->name);
                    ret = LY_EVALID;
                    goto error;
                } else {
                    LOGWRN(ctx, "Invalid position of the key \"%s\" in a list.", node->schema->name);
                }
            }
        }

        /* parser next */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), error);

        /* no children expected */
        if (xmlctx->status == LYXML_ELEMENT) {
            LOGVAL(ctx, LYVE_SYNTAX, "Child element \"%.*s\" inside a terminal node \"%s\" found.",
                    xmlctx->name_len, xmlctx->name, snode->name);
            ret = LY_EVALID;
            goto error;
        }
    } else if (snode->nodetype & LYD_NODE_INNER) {
        if (!xmlctx->ws_only) {
            /* value in inner node */
            LOGVAL(ctx, LYVE_SYNTAX, "Text value \"%.*s\" inside an inner node \"%s\" found.",
                    xmlctx->value_len, xmlctx->value, snode->name);
            ret = LY_EVALID;
            goto error;
        }

        /* create node */
        ret = lyd_create_inner(snode, &node);
        LY_CHECK_GOTO(ret, error);

        LOG_LOCSET(ctx, snode, node, NULL, NULL);

        /* parser next */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), error);

        /* process children */
        while (xmlctx->status == LYXML_ELEMENT) {
            ret = lydxml_subtree_r(lydctx, (struct lyd_node_inner *)node, lyd_node_children_p(node));
            LY_CHECK_GOTO(ret, error);
        }

        if (snode->nodetype == LYS_LIST) {
            /* check all keys exist */
            LY_CHECK_GOTO(ret = lyd_parse_check_keys(node), error);
        }

        if (!(lydctx->parse_options & LYD_PARSE_ONLY)) {
            /* new node validation, autodelete CANNOT occur, all nodes are new */
            ret = lyd_validate_new(lyd_node_children_p(node), snode, NULL, NULL);
            LY_CHECK_GOTO(ret, error);

            /* add any missing default children */
            ret = lyd_new_implicit_r(node, lyd_node_children_p(node), NULL, NULL, &lydctx->node_types, &lydctx->node_when,
                    (lydctx->validate_options & LYD_VALIDATE_NO_STATE) ? LYD_IMPLICIT_NO_STATE : 0, NULL);
            LY_CHECK_GOTO(ret, error);
        }

        if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
            /* rememeber the RPC/action/notification */
            lydctx->op_node = node;
        }
    } else if (snode->nodetype & LYD_NODE_ANY) {
        if (!xmlctx->ws_only) {
            /* value in inner node */
            LOGVAL(ctx, LYVE_SYNTAX, "Text value \"%.*s\" inside an any node \"%s\" found.",
                    xmlctx->value_len, xmlctx->value, snode->name);
            ret = LY_EVALID;
            goto error;
        }

        /* parser next */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), error);

        /* parse any data tree with correct options */
        prev_opts = lydctx->parse_options;
        lydctx->parse_options &= ~LYD_PARSE_STRICT;
        lydctx->parse_options |= LYD_PARSE_OPAQ;
        anchor = NULL;
        while (xmlctx->status == LYXML_ELEMENT) {
            ret = lydxml_subtree_r(lydctx, NULL, &anchor);
            LY_CHECK_ERR_GOTO(ret, lydctx->parse_options = prev_opts, error);
        }
        lydctx->parse_options = prev_opts;

        /* create node */
        ret = lyd_create_any(snode, anchor, LYD_ANYDATA_DATATREE, 1, &node);
        LY_CHECK_GOTO(ret, error);
    }
    assert(node);

    /* add/correct flags */
    if (snode) {
        lyd_parse_set_data_flags(node, &lydctx->node_when, &meta, lydctx->parse_options);
    }

    /* parser next */
    assert(xmlctx->status == LYXML_ELEM_CLOSE);
    LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), error);

    /* add metadata/attributes */
    if (snode) {
        lyd_insert_meta(node, meta, 0);
    } else {
        lyd_insert_attr(node, attr);
    }

    /* insert, keep first pointer correct */
    lyd_insert_node((struct lyd_node *)parent, first_p, node);
    while (!parent && (*first_p)->prev->next) {
        *first_p = (*first_p)->prev;
    }

    LOG_LOCBACK(ctx, node ? 1 : 0, node ? 1 : 0, 0, 0);
    return LY_SUCCESS;

error:
    LOG_LOCBACK(ctx, node ? 1 : 0, node ? 1 : 0, 0, 0);
    lyd_free_meta_siblings(meta);
    lyd_free_attr_siblings(ctx, attr);
    lyd_free_tree(node);
    return ret;
}

LY_ERR
lyd_parse_xml_data(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_options, uint32_t validate_options,
        struct lyd_node **tree_p, struct lyd_ctx **lydctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_xml_ctx *lydctx;

    assert(!(parse_options & ~LYD_PARSE_OPTS_MASK));
    assert(!(validate_options & ~LYD_VALIDATE_OPTS_MASK));

    /* init context */
    lydctx = calloc(1, sizeof *lydctx);
    LY_CHECK_ERR_RET(!lydctx, LOGMEM(ctx), LY_EMEM);
    LY_CHECK_GOTO(ret = lyxml_ctx_new(ctx, in, &lydctx->xmlctx), cleanup);
    lydctx->parse_options = parse_options;
    lydctx->validate_options = validate_options;
    lydctx->free = lyd_xml_ctx_free;

    /* parse XML data */
    while (lydctx->xmlctx->status == LYXML_ELEMENT) {
        LY_CHECK_GOTO(ret = lydxml_subtree_r(lydctx, NULL, tree_p), cleanup);
    }

cleanup:
    /* there should be no unres stored if validation should be skipped */
    assert(!(parse_options & LYD_PARSE_ONLY) || (!lydctx->node_types.count && !lydctx->meta_types.count &&
            !lydctx->node_when.count));

    if (ret) {
        lyd_xml_ctx_free((struct lyd_ctx *)lydctx);
        lyd_free_all(*tree_p);
        *tree_p = NULL;
    } else {
        *lydctx_p = (struct lyd_ctx *)lydctx;

        /* the XML context is no more needed, freeing it also stops logging line numbers which would be confusing now */
        lyxml_ctx_free(lydctx->xmlctx);
        lydctx->xmlctx = NULL;
    }
    return ret;
}

#if 0
static LY_ERR
lydxml_envelope(struct lyxml_ctx *xmlctx, const char *name, const char *uri, struct lyd_node **envp)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lyxml_ns *ns = NULL;
    struct lyd_attr *attr = NULL;
    const char *prefix;
    size_t prefix_len;

    *envp = NULL;

    assert(xmlctx->status == LYXML_ELEMENT);
    if (ly_strncmp(name, xmlctx->name, xmlctx->name_len)) {
        /* not the expected element */
        return LY_SUCCESS;
    }

    prefix = xmlctx->prefix;
    prefix_len = xmlctx->prefix_len;
    ns = lyxml_ns_get(&xmlctx->ns, prefix, prefix_len);
    if (!ns) {
        LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".",
                prefix_len, prefix);
        return LY_EVALID;
    } else if (strcmp(ns->uri, uri)) {
        /* different namespace */
        return LY_SUCCESS;
    }

    LY_CHECK_RET(lyxml_ctx_next(xmlctx));

    /* create attributes */
    if (xmlctx->status == LYXML_ATTRIBUTE) {
        LY_CHECK_RET(lydxml_attrs(xmlctx, &attr));
    }

    if (!xmlctx->ws_only) {
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Unexpected value \"%.*s\" in the \"%s\" element.",
                xmlctx->value_len, xmlctx->value, name);
        ret = LY_EVALID;
        goto cleanup;
    }

    /* parser next element */
    LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

    /* create node */
    ret = lyd_create_opaq(xmlctx->ctx, name, strlen(name), prefix, prefix_len, uri, strlen(uri), "", 0, NULL,
            LY_PREF_XML, NULL, LYD_NODEHINT_ENVELOPE, envp);
    LY_CHECK_GOTO(ret, cleanup);

    /* assign atributes */
    ((struct lyd_node_opaq *)(*envp))->attr = attr;
    attr = NULL;

cleanup:
    lyd_free_attr_siblings(xmlctx->ctx, attr);
    return ret;
}

#endif

LY_ERR
lyd_parse_xml_rpc(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_xml_ctx lydctx = {0};
    struct lyd_node *tree = NULL;

    /* init */
    LY_CHECK_GOTO(ret = lyxml_ctx_new(ctx, in, &lydctx.xmlctx), cleanup);
    lydctx.parse_options = LYD_PARSE_ONLY | LYD_PARSE_STRICT;
    lydctx.int_opts = LYD_INTOPT_RPC;

#if 0
    /* parse "rpc", if any */
    LY_CHECK_GOTO(ret = lydxml_envelope(lydctx.xmlctx, "rpc", "urn:ietf:params:xml:ns:netconf:base:1.0", &rpc_e), cleanup);

    if (rpc_e) {
        /* parse "action", if any */
        LY_CHECK_GOTO(ret = lydxml_envelope(lydctx.xmlctx, "action", "urn:ietf:params:xml:ns:yang:1", &act_e), cleanup);
    }
#endif

    /* parse the rest of data normally */
    LY_CHECK_GOTO(ret = lydxml_subtree_r(&lydctx, NULL, &tree), cleanup);

    /* make sure we have parsed some operation and it is the only subtree */
    if (!lydctx.op_node) {
        LOGVAL(ctx, LYVE_DATA, "Missing the \"rpc\"/\"action\" node.");
        ret = LY_EVALID;
        goto cleanup;
    } else if (lydctx.xmlctx->status == LYXML_ELEMENT) {
        LOGVAL(ctx, LYVE_SYNTAX, "Unexpected sibling element of \"%s\".",
                tree->schema->name);
        ret = LY_EVALID;
        goto cleanup;
    }

    if (op_p) {
        *op_p = lydctx.op_node;
    }
    assert(tree);
    if (tree_p) {
        *tree_p = tree;
    }

cleanup:
    /* we have used parse_only flag */
    assert(!lydctx.node_types.count && !lydctx.meta_types.count && !lydctx.node_when.count);
    lyxml_ctx_free(lydctx.xmlctx);
    if (ret) {
        lyd_free_all(tree);
    }
    return ret;
}

#if 0
static LY_ERR
lydxml_notif_envelope(struct lyxml_ctx *xmlctx, struct lyd_node **envp)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lyxml_ns *ns = NULL;
    struct lyd_attr *attr = NULL;
    struct lyd_node *et;
    const char *prefix;
    size_t prefix_len;

    *envp = NULL;

    /* container envelope */
    LY_CHECK_GOTO(ret = lydxml_envelope(xmlctx, "notification", "urn:ietf:params:xml:ns:netconf:notification:1.0",
            envp), cleanup);

    /* no envelope, fine */
    if (!*envp) {
        goto cleanup;
    }

    /* child "eventTime" */
    if ((xmlctx->status != LYXML_ELEMENT) || ly_strncmp("eventTime", xmlctx->name, xmlctx->name_len)) {
        LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Missing the \"eventTime\" element.");
        ret = LY_EVALID;
        goto cleanup;
    }

    prefix = xmlctx->prefix;
    prefix_len = xmlctx->prefix_len;
    ns = lyxml_ns_get(&xmlctx->ns, prefix, prefix_len);
    if (!ns) {
        LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", prefix_len, prefix);
        ret = LY_EVALID;
        goto cleanup;
    } else if (strcmp(ns->uri, "urn:ietf:params:xml:ns:netconf:notification:1.0")) {
        LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Invalid namespace \"%s\" of \"eventTime\".", ns->uri);
        ret = LY_EVALID;
        goto cleanup;
    }

    LY_CHECK_RET(lyxml_ctx_next(xmlctx));

    /* create attributes */
    if (xmlctx->status == LYXML_ATTRIBUTE) {
        LY_CHECK_RET(lydxml_attrs(xmlctx, &attr));
    }

    /* validate value */
    /* TODO */
    /*if (!xmlctx->ws_only) {
        LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX, "Unexpected value \"%.*s\" in the \"%s\" element.",
               xmlctx->value_len, xmlctx->value, name);
        ret = LY_EVALID;
        goto cleanup;
    }*/

    /* create node */
    ret = lyd_create_opaq(xmlctx->ctx, "eventTime", ly_strlen_const("eventTime"), prefix, prefix_len,
            ns->uri, strlen(ns->uri), xmlctx->value, xmlctx->value_len, NULL, LY_PREF_XML, NULL, LYD_NODEHINT_ENVELOPE, &et);
    LY_CHECK_GOTO(ret, cleanup);

    /* assign atributes */
    ((struct lyd_node_opaq *)et)->attr = attr;
    attr = NULL;

    /* insert */
    lyd_insert_node(*envp, NULL, et);

    /* finish parsing */
    LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
    if (xmlctx->status != LYXML_ELEM_CLOSE) {
        assert(xmlctx->status == LYXML_ELEMENT);
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Unexpected sibling element \"%.*s\" of \"eventTime\".",
                xmlctx->name_len, xmlctx->name);
        ret = LY_EVALID;
        goto cleanup;
    }
    LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

cleanup:
    if (ret) {
        lyd_free_tree(*envp);
        lyd_free_attr_siblings(xmlctx->ctx, attr);
    }
    return ret;
}

#endif

LY_ERR
lyd_parse_xml_notif(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **ntf_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_xml_ctx lydctx = {0};
    struct lyd_node *tree = NULL;

    /* init */
    LY_CHECK_GOTO(ret = lyxml_ctx_new(ctx, in, &lydctx.xmlctx), cleanup);
    lydctx.parse_options = LYD_PARSE_ONLY | LYD_PARSE_STRICT;
    lydctx.int_opts = LYD_INTOPT_NOTIF;

#if 0
    /* parse "notification" and "eventTime", if present */
    LY_CHECK_GOTO(ret = lydxml_notif_envelope(lydctx.xmlctx, &ntf_e), cleanup);
#endif

    /* parse the rest of data normally */
    LY_CHECK_GOTO(ret = lydxml_subtree_r(&lydctx, NULL, &tree), cleanup);

    /* make sure we have parsed some notification */
    if (!lydctx.op_node) {
        LOGVAL(ctx, LYVE_DATA, "Missing the \"notification\" node.");
        ret = LY_EVALID;
        goto cleanup;
    } else if (lydctx.xmlctx->status == LYXML_ELEMENT) {
        LOGVAL(ctx, LYVE_SYNTAX, "Unexpected sibling element of \"%s\".",
                tree->schema->name);
        ret = LY_EVALID;
        goto cleanup;
    }

    if (ntf_p) {
        *ntf_p = lydctx.op_node;
    }
    assert(tree);
    if (tree_p) {
        *tree_p = tree;
    }

cleanup:
    /* we have used parse_only flag */
    assert(!lydctx.node_types.count && !lydctx.meta_types.count && !lydctx.node_when.count);
    lyxml_ctx_free(lydctx.xmlctx);
    if (ret) {
        lyd_free_all(tree);
    }
    return ret;
}

LY_ERR
lyd_parse_xml_reply(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_xml_ctx lydctx = {0};
    struct lyd_node *tree = NULL;

    /* init */
    LY_CHECK_GOTO(ret = lyxml_ctx_new(ctx, in, &lydctx.xmlctx), cleanup);
    lydctx.parse_options = LYD_PARSE_ONLY | LYD_PARSE_STRICT;
    lydctx.int_opts = LYD_INTOPT_REPLY;

#if 0
    /* parse "rpc-reply", if any */
    LY_CHECK_GOTO(ret = lydxml_envelope(lydctx.xmlctx, "rpc-reply", "urn:ietf:params:xml:ns:netconf:base:1.0", &rpcr_e),
            cleanup);
#endif

    /* parse the rest of data normally but connect them to the duplicated operation */
    while (lydctx.xmlctx->status == LYXML_ELEMENT) {
        ret = lydxml_subtree_r(&lydctx, NULL, &tree);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (op_p) {
        *op_p = lydctx.op_node;
    }
    if (tree_p) {
        *tree_p = tree;
    }

cleanup:
    /* we have used parse_only flag */
    assert(!lydctx.node_types.count && !lydctx.meta_types.count && !lydctx.node_when.count);
    lyxml_ctx_free(lydctx.xmlctx);
    if (ret) {
        lyd_free_all(tree);
    }
    return ret;
}
