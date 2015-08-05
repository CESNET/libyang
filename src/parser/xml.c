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
#include "../resolve.h"
#include "../xml.h"
#include "../tree_internal.h"

#define LY_NSNC "urn:ietf:params:xml:ns:netconf:base:1.0"

/* kind == 0 - unsigned (unum used), 1 - signed (snum used), 2 - floating point (fnum used) */
static int
validate_length_range(uint8_t kind, uint64_t unum, int64_t snum, long double fnum, struct lys_type *type,
                      struct lyxml_elem *xml, const char *str_val, int log)
{
    struct len_ran_intv *intv = NULL, *tmp_intv;
    int ret = 1;

    assert(!get_len_ran_interval(NULL, type, 0, &intv));
    if (!intv) {
        return 0;
    }

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

    if (ret && log) {
        LOGVAL(LYE_OORVAL, LOGLINE(xml), str_val);
    }
    return ret;
}

static int
validate_pattern(const char *str, struct lys_type *type, struct lyxml_elem *xml, const char *str_val, int log)
{
    int i;
    regex_t preq;
    char *posix_regex;

    assert(type->base == LY_TYPE_STRING);

    if (type->der && validate_pattern(str, &type->der->type, xml, str_val, log)) {
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
        if (regcomp(&preq, posix_regex, REG_EXTENDED | REG_NOSUB)) {
            LOGINT;
            return EXIT_FAILURE;
        }
        free(posix_regex);

        if (regexec(&preq, str, 0, 0, 0)) {
            regfree(&preq);
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), str_val, xml->name);
            }
            return 1;
        }
        regfree(&preq);
    }

    return 0;
}

static struct lys_node *
xml_data_search_schemanode(struct lyxml_elem *xml, struct lys_node *start)
{
    struct lys_node *result, *aux;

    LY_TREE_FOR(start, result) {
        /* skip groupings */
        if (result->nodetype == LYS_GROUPING) {
            continue;
        }

        /* go into cases, choices, uses */
        if (result->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES)) {
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
parse_int(const char *str_val, struct lyxml_elem *xml, int64_t min, int64_t max, int base, int64_t *ret, int log)
{
    char *strptr;

    /* convert to 64-bit integer, all the redundant characters are handled */
    errno = 0;
    strptr = NULL;
    *ret = strtoll(str_val, &strptr, base);
    if (errno || (*ret < min) || (*ret > max)) {
        if (log) {
            LOGVAL(LYE_OORVAL, LOGLINE(xml), str_val, xml->name);
        }
        return 1;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), str_val, xml->name);
            }
            return 1;
        }
    }

    return 0;
}

static int
parse_uint(const char *str_val, struct lyxml_elem *xml, uint64_t max, int base, uint64_t *ret, int log)
{
    char *strptr;

    errno = 0;
    strptr = NULL;
    *ret = strtoull(str_val, &strptr, base);
    if (errno || (*ret > max)) {
        if (log) {
            LOGVAL(LYE_OORVAL, LOGLINE(xml), str_val, xml->name);
        }
        return 1;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), str_val, xml->name);
            }
            return 1;
        }
    }

    return 0;
}

static struct lys_type *
get_next_union_type(struct lys_type *type, struct lys_type *prev_type, int *found)
{
    int i;
    struct lys_type *ret = NULL;

    for (i = 0; i < type->info.uni.count; ++i) {
        if (type->info.uni.types[i].base == LY_TYPE_UNION) {
            ret = get_next_union_type(&type->info.uni.types[i], prev_type, found);
            if (ret) {
                break;;
            }
            continue;
        }

        if (!prev_type || *found) {
            ret = &type->info.uni.types[i];
            break;
        }

        if (&type->info.uni.types[i] == prev_type) {
            *found = 1;
        }
    }

    if (!ret && type->der) {
        ret = get_next_union_type(&type->der->type, prev_type, found);
    }

    return ret;
}

static int
_xml_get_value(struct lyd_node *node, struct lys_type *node_type, struct lyxml_elem *xml,
               struct leafref_instid **unres, int log)
{
    #define DECSIZE 21
    struct lyd_node_leaf *leaf = (struct lyd_node_leaf *)node;
    struct lys_type *type;
    struct lyxml_ns *ns;
    char dec[DECSIZE];
    char *strptr;
    const char *name;
    int64_t num;
    uint64_t unum;
    int len;
    int c, i, j, d;
    int found;
    struct leafref_instid *new_unres;

    leaf->value_str = xml->content;
    xml->content = NULL;

    switch (node_type->base) {
    case LY_TYPE_BINARY:
        leaf->value.binary = leaf->value_str;
        leaf->value_type = LY_TYPE_BINARY;

        if (node_type->info.binary.length
                && validate_length_range(0, strlen(leaf->value.binary), 0, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_BITS:
        leaf->value_type = LY_TYPE_BITS;

        /* locate bits structure with the bits definitions */
        for (type = node_type; type->der->type.der; type = &type->der->type);

        /* allocate the array of  pointers to bits definition */
        leaf->value.bit = calloc(type->info.bits.count, sizeof *leaf->value.bit);

        if (!leaf->value_str) {
            /* no bits set */
            break;
        }

        c = 0;
        i = 0;
        while (leaf->value_str[c]) {
            /* skip leading whitespaces */
            while(isspace(leaf->value_str[c])) {
                c++;
            }

            /* get the length of the bit identifier */
            for (len = 0; leaf->value_str[c] && !isspace(leaf->value_str[c]); c++, len++);

            /* go back to the beginning of the identifier */
            c = c - len;

            /* find bit definition, identifiers appear ordered by their posititon */
            for (found = 0; i < type->info.bits.count; i++) {
                if (!strncmp(type->info.bits.bit[i].name, &leaf->value_str[c], len)
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
                if (log) {
                    LOGVAL(LYE_INVAL, LOGLINE(xml), leaf->value_str, xml->name);
                }
                return EXIT_FAILURE;
            }

            c = c + len;
        }

        break;

    case LY_TYPE_BOOL:
        if (!strcmp(leaf->value_str, "true")) {
            leaf->value.bool = 1;
        } /* else false, so keep it zero */
        break;

    case LY_TYPE_DEC64:
        /* locate dec64 structure with the fraction-digits value */
        for (type = node_type; type->der->type.der; type = &type->der->type);

        for (c = 0; isspace(leaf->value_str[c]); c++);
        for (len = 0; leaf->value_str[c] && !isspace(leaf->value_str[c]); c++, len++);
        c = c - len;
        if (len > DECSIZE) {
            /* too long */
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), leaf->value_str, xml->name);
            }
            return EXIT_FAILURE;
        }

        /* normalize the number */
        dec[0] = '\0';
        for (i = j = d = found = 0; i < DECSIZE; i++) {
            if (leaf->value_str[c + i] == '.') {
                found = 1;
                j = type->info.dec64.dig;
                i--;
                c++;
                continue;
            }
            if (leaf->value_str[c + i] == '\0') {
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
                    if (log) {
                        LOGVAL(LYE_OORVAL, LOGLINE(xml), leaf->value_str, xml->name);
                    }
                    return EXIT_FAILURE;
                }
                dec[i] = '0';
            } else {
                if (!isdigit(leaf->value_str[c + i])) {
                    if (i || leaf->value_str[c] != '-') {
                        if (log) {
                            LOGVAL(LYE_INVAL, LOGLINE(xml), leaf->value_str, xml->name);
                        }
                        return EXIT_FAILURE;
                    }
                } else {
                    d++;
                }
                if (d > DECSIZE - 2 || (found && !j)) {
                    if (log) {
                        LOGVAL(LYE_OORVAL, LOGLINE(xml), leaf->value_str, xml->name);
                    }
                    return EXIT_FAILURE;
                }
                dec[i] = leaf->value_str[c + i];
            }
            if (j) {
                j--;
            }
        }

        if (parse_int(dec, xml, -9223372036854775807L - 1L, 9223372036854775807L, 10, &num, log)
                || validate_length_range(2, 0, 0, ((long double)num)/(1 << type->info.dec64.dig), node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        leaf->value.dec64 = num;
        break;

    case LY_TYPE_EMPTY:
        /* just check that it is empty */
        if (leaf->value_str && leaf->value_str[0]) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), leaf->value_str, xml->name);
            }
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_ENUM:
        if (!leaf->value_str) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), "", xml->name);
            }
            return EXIT_FAILURE;
        }

        /* locate enums structure with the enumeration definitions */
        for (type = node_type; type->der->type.der; type = &type->der->type);

        /* find matching enumeration value */
        for (i = 0; i < type->info.enums.count; i++) {
            if (!strcmp(leaf->value_str, type->info.enums.enm[i].name)) {
                /* we have match, store pointer to the definition */
                leaf->value.enm = &type->info.enums.enm[i];
                break;
            }
        }

        if (!leaf->value.enm) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), leaf->value_str, xml->name);
            }
            return EXIT_FAILURE;
        }

        break;

    case LY_TYPE_IDENT:
        if ((strptr = strchr(leaf->value_str, ':'))) {
            len = strptr - leaf->value_str;
            if (!len) {
                if (log) {
                    LOGVAL(LYE_INVAL, LOGLINE(xml), leaf->value_str, xml->name);
                }
                return EXIT_FAILURE;
            }
            strptr = strndup(leaf->value_str, len);
        }
        ns = lyxml_get_ns(xml, strptr);
        if (!ns) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), leaf->value_str, xml->name);
            }
            return EXIT_FAILURE;
        }
        if (strptr) {
            free(strptr);
            name = leaf->value_str + len + 1;
        } else {
            name = leaf->value_str;
        }

        leaf->value.ident = resolve_identityref(node_type->info.ident.ref, name, ns->value);
        if (!leaf->value.ident) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), leaf->value_str, xml->name);
            }
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_INST:
        if (!leaf->value_str) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), "", xml->name);
            }
            return EXIT_FAILURE;
        }

        /* validity checking is performed later, right now the data tree
         * is not complete, so many instanceids cannot be resolved
         */
        /* remember the leaf for later checking */
        new_unres = malloc(sizeof *new_unres);
        new_unres->is_leafref = 0;
        new_unres->dnode = node;
        new_unres->next = *unres;
        new_unres->line = LOGLINE(xml);
        *unres = new_unres;
        break;

    case LY_TYPE_LEAFREF:
        if (!leaf->value_str) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), "", xml->name);
            }
            return EXIT_FAILURE;
        }

        /* validity checking is performed later, right now the data tree
         * is not complete, so many leafrefs cannot be resolved
         */
        /* remember the leaf for later checking */
        new_unres = malloc(sizeof *new_unres);
        new_unres->is_leafref = 1;
        new_unres->dnode = node;
        new_unres->next = *unres;
        new_unres->line = LOGLINE(xml);
        *unres = new_unres;
        break;

    case LY_TYPE_STRING:
        leaf->value.string = leaf->value_str;
        leaf->value_type = LY_TYPE_STRING;

        if (node_type->info.str.length
                && validate_length_range(0, strlen(leaf->value.string), 0, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }

        if (node_type->info.str.patterns
                &&  validate_pattern(leaf->value.string, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_UNION:
        found = 0;
        type = get_next_union_type(node_type, NULL, &found);
        for (; type; found = 0, type = get_next_union_type(node_type, type, &found)) {
            xml->content = leaf->value_str;
            if (!_xml_get_value(node, type, xml, unres, 0)) {
                leaf->value_type = type->base;
                break;
            }
        }

        if (!type) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), leaf->value_str, xml->name);
            }
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_INT8:
        if (parse_int(leaf->value_str, xml, -128, 127, 0, &num, log)
                || validate_length_range(1, 0, num, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        leaf->value.int8 = num;
        break;

    case LY_TYPE_INT16:
        if (parse_int(leaf->value_str, xml, -32768, 32767, 0, &num, log)
                || validate_length_range(1, 0, num, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        leaf->value.int16 = num;
        break;

    case LY_TYPE_INT32:
        if (parse_int(leaf->value_str, xml, -2147483648, 2147483647, 0, &num, log)
                || validate_length_range(1, 0, num, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        leaf->value.int32 = num;
        break;

    case LY_TYPE_INT64:
        if (parse_int(leaf->value_str, xml, -9223372036854775807L - 1L, 9223372036854775807L, 0, &num, log)
                || validate_length_range(1, 0, num, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        leaf->value.int64 = num;
        break;

    case LY_TYPE_UINT8:
        if (parse_uint(leaf->value_str, xml, 255, 0, &unum, log)
                || validate_length_range(0, unum, 0, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        leaf->value.uint8 = unum;
        break;

    case LY_TYPE_UINT16:
        if (parse_uint(leaf->value_str, xml, 65535, 0, &unum, log)
                || validate_length_range(0, unum, 0, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        leaf->value.uint16 = unum;
        break;

    case LY_TYPE_UINT32:
        if (parse_uint(leaf->value_str, xml, 4294967295, 0, &unum, log)
                || validate_length_range(0, unum, 0, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        leaf->value.uint32 = unum;
        break;

    case LY_TYPE_UINT64:
        if (parse_uint(leaf->value_str, xml, 18446744073709551615UL, 0, &unum, log)
                || validate_length_range(0, unum, 0, 0, node_type, xml, leaf->value_str, log)) {
            return EXIT_FAILURE;
        }
        leaf->value.uint64 = unum;
        break;

    default:
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int
xml_get_value(struct lyd_node *node, struct lyxml_elem *xml, struct leafref_instid **unres)
{
    return _xml_get_value(node, &((struct lys_node_leaf *)node->schema)->type, xml, unres, 1);
}

struct lyd_node *
xml_parse_data(struct ly_ctx *ctx, struct lyxml_elem *xml, struct lyd_node *parent, struct lyd_node *prev,
               struct leafref_instid **unres)
{
    struct lyd_node *result, *aux;
    struct lys_node *schema = NULL;
    int i, havechildren;

    if (!xml) {
        return NULL;
    }
    if (!xml->ns || !xml->ns->value) {
        LOGVAL(LYE_XML_MISS, LOGLINE(xml), "element's", "namespace");
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
        LOGVAL(LYE_INELEM, LOGLINE(xml), xml->name);
        return NULL;
    }

    switch (schema->nodetype) {
    case LYS_CONTAINER:
        result = calloc(1, sizeof *result);
        havechildren = 1;
        break;
    case LYS_LEAF:
        result = calloc(1, sizeof(struct lyd_node_leaf));
        havechildren = 0;
        break;
    case LYS_LEAFLIST:
        result = calloc(1, sizeof(struct lyd_node_leaflist));
        havechildren = 0;
        break;
    case LYS_LIST:
        result = calloc(1, sizeof(struct lyd_node_list));
        havechildren = 1;
        break;
    case LYS_ANYXML:
        result = calloc(1, sizeof(struct lyd_node_anyxml));
        havechildren = 0;
        break;
    default:
        assert(0);
        return NULL;
    }
    result->parent = parent;
    result->prev = prev;
    result->schema = schema;

    /* type specific processing */
    if (schema->nodetype == LYS_LIST) {
        /* pointers to next and previous instances of the same list */
        for (aux = result->prev; aux; aux = aux->prev) {
            if (aux->schema == result->schema) {
                /* instances of the same list */
                ((struct lyd_node_list *)aux)->lnext = (struct lyd_node_list *)result;
                ((struct lyd_node_list *)result)->lprev = (struct lyd_node_list *)aux;
                break;
            }
        }
    } else if (schema->nodetype == LYS_LEAF) {
        /* type detection and assigning the value */
        if (xml_get_value(result, xml, unres)) {
            goto error;
        }
    } else if (schema->nodetype == LYS_LEAFLIST) {
        /* type detection and assigning the value */
        if (xml_get_value(result, xml, unres)) {
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
    } else if (schema->nodetype == LYS_ANYXML) {
        ((struct lyd_node_anyxml *)result)->value = xml;
        lyxml_unlink_elem(xml);
    }

    /* process children */
    if (havechildren && xml->child) {
        result->child = xml_parse_data(ctx, xml->child, result, NULL, unres);
        if (!result->child) {
            goto error;
        }

    }

    /* process siblings */
    if (xml->next) {
        result->next = xml_parse_data(ctx, xml->next, parent, result, unres);
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
    lyd_free(result);

    return NULL;
}

static int
check_unres(struct leafref_instid **list)
{
    struct lyd_node_leaf *leaf;
    struct lys_node_leaf *sleaf;
    struct leafref_instid *item, *refset = NULL, *ref;

    while (*list) {
        leaf = (struct lyd_node_leaf *)(*list)->dnode;
        sleaf = (struct lys_node_leaf *)(*list)->dnode->schema;

        /* resolve path and create a set of possible leafrefs (we need their values) */
        if ((*list)->is_leafref) {
            if (resolve_path_arg_data(*list, sleaf->type.info.lref.path, &refset)) {
                LOGERR(LY_EVALID, "Leafref \"%s\" could not be resolved.", sleaf->type.info.lref.path);
                goto error;
            }

            while (refset) {
                if (leaf->value_str == ((struct lyd_node_leaf *)refset->dnode)->value_str) {
                    leaf->value.leafref = refset->dnode;
                }
                ref = refset->next;
                free(refset);
                refset = ref;
            }

            if (!leaf->value.leafref) {
                /* reference not found */
                LOGERR(LY_EVALID, "Leafref \"%s\" value \"%s\" did not match any node value.", sleaf->type.info.lref.path, leaf->value_str);
                goto error;
            }

        /* instance-identifier */
        } else {
            if (resolve_instid(*list, leaf->value_str, strlen(leaf->value_str), &refset)
                    || (refset && refset->next)) {
                if (sleaf->type.info.inst.req > -1) {
                    LOGERR(LY_EVALID, "Instance-identifier \"%s\" validation fail.", leaf->value_str);
                    goto error;
                } else {
                    LOGVRB("Instance-identifier \"%s\" validation fail.", leaf->value_str);
                }
            }

            while (refset) {
                ref = refset->next;
                free(refset);
                refset = ref;
            }
        }

        item = (*list)->next;
        free(*list);
        *list = item;
    }

    return EXIT_SUCCESS;

error:

    while (*list) {
        item = (*list)->next;
        free(*list);
        *list = item;
    }

    while (refset) {
        ref = refset->next;
        free(refset);
        refset = ref;
    }

    return EXIT_FAILURE;
}

API struct lyd_node *
xml_read_data(struct ly_ctx *ctx, const char *data)
{
    struct lyxml_elem *xml;
    struct lyd_node *result, *next, *iter;
    struct leafref_instid *unres = NULL;

    xml = lyxml_read(ctx, data, 0);
    if (!xml) {
        return NULL;
    }

    /* check the returned data - the root must be config or data in NETCONF namespace */
    if (!xml->ns || strcmp(xml->ns->value, LY_NSNC) || (strcmp(xml->name, "data") && strcmp(xml->name, "config"))) {
        LOGERR(LY_EINVAL, "XML data parser expect <data> or <config> root in \"%s\" namespace.", LY_NSNC);
        return NULL;
    }

    result = xml_parse_data(ctx, xml->child, NULL, NULL, &unres);
    /* check leafrefs and/or instids if any */
    if (check_unres(&unres)) {
        /* leafref & instid checking failed */
        LY_TREE_FOR_SAFE(result, next, iter) {
            lyd_free(iter);
        }
        result = NULL;
    }

    /* free source XML tree */
    lyxml_free_elem(ctx, xml);

    return result;
}
