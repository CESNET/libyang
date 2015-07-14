/**
 * @file xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML data parser for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

#include "../libyang.h"
#include "../common.h"
#include "../context.h"
#include "../xml.h"
#include "../tree_internal.h"

#define LY_NSNC "urn:ietf:params:xml:ns:netconf:base:1.0"

/* kind == 0 - unsigned (unum used), 1 - signed (snum used), 2 - floating point (fnum used) */
static int
validate_length_range(uint8_t kind, uint64_t unum, int64_t snum, long double fnum, struct ly_type *type)
{
    struct len_ran_intv *intv, *tmp_intv;
    int ret = 1;

    intv = get_len_ran_interval(NULL, type, 0);
    for (tmp_intv = intv; tmp_intv; tmp_intv = tmp_intv->next) {
        if (((kind == 0) && (unum < tmp_intv->value.uval.min))
                || ((kind == 1) && (snum < tmp_intv->value.sval.min))
                || ((kind == 2) && (fnum < tmp_intv->value.fval.min))) {
            break;
        }

        if (((kind == 0) && (unum >= tmp_intv->value.uval.min) && (unum <= tmp_intv->value.uval.max))
                || ((kind == 1) && (snum >= tmp_intv->value.sval.min) && (snum <= tmp_intv->value.sval.max))
                || ((kind == 2) && (fnum >= tmp_intv->value.fval.min) && (fnum <= tmp_intv->value.fval.max))) {
            ret = 0;
            break;
        }
    }

    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }

    return ret;
}

static int
validate_pattern(const char *str, struct ly_type *type)
{
    int i;
    regex_t preq;
    char *posix_regex;

    assert(type->base == LY_TYPE_STRING);

    if (type->der && validate_pattern(str, &type->der->type)) {
        return 1;
    }

    for (i = 0; i < type->info.str.pat_count; ++i) {
        /*
         * adjust the expression to a POSIX.2 equivalent
         *
         * http://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#regexs
         */
        posix_regex = malloc((strlen(type->info.str.patterns[i].expr)+3) * sizeof(char));
        posix_regex[0] = '\0';

        if (strncmp(type->info.str.patterns[i].expr, ".*", 2)) {
            strcat(posix_regex, "^");
        }
        strcat(posix_regex, type->info.str.patterns[i].expr);
        if (strncmp(type->info.str.patterns[i].expr
                + strlen(type->info.str.patterns[i].expr) - 2, ".*", 2)) {
            strcat(posix_regex, "$");
        }

        /* must return 0, already checked during parsing */
        assert(!regcomp(&preq, posix_regex, REG_EXTENDED | REG_NOSUB));
        free(posix_regex);

        if (regexec(&preq, str, 0, 0, 0)) {
            regfree(&preq);
            return 1;
        }
        regfree(&preq);
    }

    return 0;
}

static struct ly_mnode *
xml_data_search_schemanode(struct lyxml_elem *xml, struct ly_mnode *start)
{
    struct ly_mnode *result, *aux;

    LY_TREE_FOR(start, result) {
        /* skip groupings */
        if (result->nodetype == LY_NODE_GROUPING) {
            continue;
        }

        /* go into cases, choices, uses */
        if (result->nodetype & (LY_NODE_CHOICE | LY_NODE_CASE | LY_NODE_USES)) {
            aux = xml_data_search_schemanode(xml, result->child);
            if (aux) {
                /* we have matching result */
                return aux;
            }
            /* else, continue with next schema node */
            continue;
        }

        /* match data nodes */
        if (result->name == xml->name) {
            /* names matches, what about namespaces? */
            if (result->module->ns == xml->ns->value) {
                /* we have matching result */
                return result;
            }
            /* else, continue with next schema node */
            continue;
        }
    }

    /* no match */
    return NULL;
}

static int
xml_get_value(struct lyd_node *node, struct lyxml_elem *xml)
{
#define DECSIZE 21
    struct lyd_node_leaf *leaf = (struct lyd_node_leaf *)node;
    struct ly_mnode_leaf *sleaf = (struct ly_mnode_leaf *)node->schema;
    struct ly_type *type;
    struct lyxml_ns *ns;
    char dec[DECSIZE];
    char *strptr;
    const char *name;
    int len;
    int c, i, j, d;
    int found;

    switch (sleaf->type.base) {
    case LY_TYPE_BINARY:
        leaf->value.binary = xml->content;
        xml->content = NULL;
        leaf->value_type = LY_TYPE_BINARY;

        if (sleaf->type.info.binary.length) {
            if (validate_length_range(0, strlen(leaf->value.binary), 0, 0, &sleaf->type)) {
                LOGVAL(DE_INVAL, LOGLINE(xml), leaf->value.binary, xml->name);
                return EXIT_FAILURE;
            }
        }
        break;

    case LY_TYPE_BITS:
        leaf->value_type = LY_TYPE_BITS;

        /* locate bits structure with the bits definitions */
        for (type = &sleaf->type; type->der->type.der; type = &type->der->type);

        /* allocate the array of  pointers to bits definition */
        leaf->value.bit = calloc(type->info.bits.count, sizeof *leaf->value.bit);

        if (!xml->content) {
            /* no bits set */
            break;
        }

        c = 0;
        i = 0;
        while (xml->content[c]) {
            /* skip leading whitespaces */
            while(isspace(xml->content[c])) {
                c++;
            }

            /* get the length of the bit identifier */
            for (len = 0; xml->content[c] && !isspace(xml->content[c]); c++, len++);

            /* go back to the beginning of the identifier */
            c = c - len;

            /* find bit definition, identifiers appear ordered by their posititon */
            for (found = 0; i < type->info.bits.count; i++) {
                if (!strncmp(type->info.bits.bit[i].name, &xml->content[c], len)
                        && !type->info.bits.bit[i].name[len]) {
                    /* we have match, store the pointer */
                    leaf->value.bit[i] = &type->info.bits.bit[i];

                    /* stop searching */
                    i++;
                    found = 1;
                    break;
                }
            }

            if (!found) {
                /* referenced bit value does not exists */
                LOGVAL(DE_INVAL, LOGLINE(xml), xml->content, xml->name);
                return EXIT_FAILURE;
            }

            c = c + len;
        }

        break;

    case LY_TYPE_BOOL:
        if (!strcmp(xml->content, "true")) {
            leaf->value.bool = 1;
        } /* else false, so keep it zero */
        break;

    case LY_TYPE_DEC64:
        /* locate dec64 structure with the fraction-digits value */
        for (type = &sleaf->type; type->der->type.der; type = &type->der->type);

        for (c = 0; isspace(xml->content[c]); c++);
        for (len = 0; xml->content[c] && !isspace(xml->content[c]); c++, len++);
        c = c - len;
        if (len > DECSIZE) {
            /* too long */
            LOGVAL(DE_INVAL, LOGLINE(xml), xml->content, xml->name);
            return EXIT_FAILURE;
        }

        /* normalize the number */
        dec[0] = '\0';
        for (i = j = d = found = 0; i < DECSIZE; i++) {
            if (xml->content[c + i] == '.') {
                found = 1;
                j = type->info.dec64.dig;
                i--;
                c++;
                continue;
            }
            if (xml->content[c + i] == '\0') {
                c--;
                if (!found) {
                    j = type->info.dec64.dig;
                    found = 1;
                }
                if (!j) {
                    dec[i] = '\0';
                    break;
                }
                d++;
                if (d > DECSIZE - 2) {
                    LOGVAL(DE_OORVAL, LOGLINE(xml), xml->content, xml->name);
                    return EXIT_FAILURE;
                }
                dec[i] = '0';
            } else {
                if (!isdigit(xml->content[c + i])) {
                    if (i || xml->content[c] != '-') {
                        LOGVAL(DE_INVAL, LOGLINE(xml), xml->content, xml->name);
                        return EXIT_FAILURE;
                    }
                } else {
                    d++;
                }
                if (d > DECSIZE - 2 || (found && !j)) {
                    LOGVAL(DE_OORVAL, LOGLINE(xml), xml->content, xml->name);
                    return EXIT_FAILURE;
                }
                dec[i] = xml->content[c + i];
            }
            if (j) {
                j--;
            }
        }

        /* convert string into 64b integer */
        errno = 0;
        strptr = NULL;
        leaf->value.dec64 = strtoll(dec, &strptr, 10);
        if (errno) {
            LOGVAL(DE_OORVAL, LOGLINE(xml), xml->content, xml->name);
            return EXIT_FAILURE;
        } else if (strptr && *strptr) {
            LOGVAL(DE_INVAL, LOGLINE(xml), xml->content, xml->name);
            return EXIT_FAILURE;
        }

        break;

    case LY_TYPE_EMPTY:
        /* just check that it is empty */
        if (xml->content && xml->content[0]) {
            LOGVAL(DE_INVAL, LOGLINE(xml), xml->content, xml->name);
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_ENUM:
        if (!xml->content) {
            LOGVAL(DE_INVAL, LOGLINE(xml), "", xml->name);
            return EXIT_FAILURE;
        }

        /* locate enums structure with the enumeration definitions */
        for (type = &sleaf->type; type->der->type.der; type = &type->der->type);

        /* find matching enumeration value */
        for (i = 0; i < type->info.enums.count; i++) {
            if (!strcmp(xml->content, type->info.enums.list[i].name)) {
                /* we have match, store pointer to the definition */
                leaf->value.enm = &type->info.enums.list[i];
                break;
            }
        }

        if (!leaf->value.enm) {
            LOGVAL(DE_INVAL, LOGLINE(xml), xml->content, xml->name);
            return EXIT_FAILURE;
        }

        break;

    case LY_TYPE_IDENT:
        if ((strptr = strchr(xml->content, ':'))) {
            len = strptr - xml->content;
            if (!len) {
                LOGVAL(DE_INVAL, LOGLINE(xml), xml->content, xml->name);
                return EXIT_FAILURE;
            }
            strptr = strndup(xml->content, len);
        }
        ns = lyxml_get_ns(xml, strptr);
        if (!ns) {
            LOGVAL(DE_INVAL, LOGLINE(xml), xml->content, xml->name);
            return EXIT_FAILURE;
        }
        if (strptr) {
            free(strptr);
            name = xml->content + len + 1;
        } else {
            name = xml->content;
        }

        leaf->value.ident = find_identityref(sleaf->type.info.ident.ref, name, ns->value);
        if (!leaf->value.ident) {
            LOGVAL(DE_INVAL, LOGLINE(xml), xml->content, xml->name);
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_STRING:
        leaf->value.string = xml->content;
        xml->content = NULL;
        leaf->value_type = LY_TYPE_STRING;

        if (sleaf->type.info.str.length) {
            if (validate_length_range(0, strlen(leaf->value.string), 0, 0, &sleaf->type)) {
                LOGVAL(DE_INVAL, LOGLINE(xml), leaf->value.string, xml->name);
                return EXIT_FAILURE;
            }
        }

        if (sleaf->type.info.str.patterns) {
            if (validate_pattern(leaf->value.string, &sleaf->type)) {
                LOGVAL(DE_INVAL, LOGLINE(xml), leaf->value.string, xml->name);
                return EXIT_FAILURE;
            }
        }
        break;

    default:
        /* TODO */
        break;
    }

    return EXIT_SUCCESS;
}

struct lyd_node *
xml_parse_data(struct ly_ctx *ctx, struct lyxml_elem *xml, struct lyd_node *parent, struct lyd_node *prev)
{
    struct lyd_node *result, *aux;
    struct ly_mnode *schema = NULL;
    int i;
    int havechildren = 1;

    if (!xml) {
        return NULL;
    }
    if (!xml->ns || !xml->ns->value) {
        LOGVAL(VE_XML_MISS, LOGLINE(xml), "element's", "namespace");
        return NULL;
    }

    /* find schema node */
    if (!parent) {
        /* starting in root */
        for (i = 0; i < ctx->models.used; i++) {
            /* match data model based on namespace */
            if (ctx->models.list[i]->ns == xml->ns->value) {
                /* get the proper schema node */
                LY_TREE_FOR(ctx->models.list[i]->data, schema) {
                    if (schema->name == xml->name) {
                        break;
                    }
                }
                break;
            }
        }
    } else {
        /* parsing some internal node, we start with parent's schema pointer */
        schema = xml_data_search_schemanode(xml, parent->schema->child);
    }
    if (!schema) {
        LOGVAL(DE_INELEM, LOGLINE(xml), xml->name);
        return NULL;
    }

    /* TODO: fit this into different types of nodes */
    switch (schema->nodetype) {
    case LY_NODE_LIST:
        result = calloc(1, sizeof(struct lyd_node_list));
        break;
    case LY_NODE_LEAF:
        result = calloc(1, sizeof(struct lyd_node_leaf));
        havechildren = 0;
        break;
    case LY_NODE_LEAFLIST:
        result = calloc(1, sizeof(struct lyd_node_leaflist));
        havechildren = 0;
        break;
    default:
        result = calloc(1, sizeof *result);
    }
    result->parent = parent;
    result->prev = prev;
    result->schema = schema;

    /* type specific processing */
    if (schema->nodetype == LY_NODE_LIST) {
        /* pointers to next and previous instances of the same list */
        for (aux = result->prev; aux; aux = aux->prev) {
            if (aux->schema == result->schema) {
                /* instances of the same list */
                ((struct lyd_node_list *)aux)->lnext = (struct lyd_node_list *)result;
                ((struct lyd_node_list *)result)->lprev = (struct lyd_node_list *)aux;
                break;
            }
        }
    } else if (schema->nodetype == LY_NODE_LEAF) {
        /* type detection and assigning the value */
        if (xml_get_value(result, xml)) {
            goto error;
        }
    } else if (schema->nodetype == LY_NODE_LEAFLIST) {
        /* type detection and assigning the value */
        if (xml_get_value(result, xml)) {
            goto error;
        }

        /* pointers to next and previous instances of the same leaflist */
        for (aux = result->prev; aux; aux = aux->prev) {
            if (aux->schema == result->schema) {
                /* instances of the same list */
                ((struct lyd_node_leaflist *)aux)->lnext = (struct lyd_node_leaflist *)result;
                ((struct lyd_node_leaflist *)result)->lprev = (struct lyd_node_leaflist *)aux;
                break;
            }
        }
    }

    /* process children */
    if (havechildren && xml->child) {
        result->child = xml_parse_data(ctx, xml->child, result, NULL);
        if (!result->child) {
            goto error;
        }

    }

    /* process siblings */
    if (xml->next) {
        result->next = xml_parse_data(ctx, xml->next, parent, result);
        if (!result->next) {
            goto error;
        }
    }

    /* fix the "last" pointer */
    if (!result->prev) {
        for (aux = result; aux->next; aux = aux->next);
        result->prev = aux;
    }
    return result;

error:

    if (havechildren) {
        result->child = NULL;
    }
    result->next = NULL;
    result->parent = NULL;
    result->prev = result;
    lyd_node_free(result);

    return NULL;
}

API struct lyd_node *
xml_read_data(struct ly_ctx *ctx, const char *data)
{
    struct lyxml_elem *xml;
    struct lyd_node *result;

    xml = lyxml_read(ctx, data, 0);
    if (!xml) {
        return NULL;
    }

    /* check the returned data - the root must be config or data in NETCONF namespace */
    if (!xml->ns || strcmp(xml->ns->value, LY_NSNC) || (strcmp(xml->name, "data") && strcmp(xml->name, "config"))) {
        LOGERR(LY_EINVAL, "XML data parser expect <data> or <config> root in \"%s\" namespace.", LY_NSNC);
        return NULL;
    }

    result = xml_parse_data(ctx, xml->child, NULL, NULL);
    lyxml_free_elem(ctx, xml);

    return result;
}
