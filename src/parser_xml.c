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
#include <limits.h>
#include <sys/types.h>
#include <pcre.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "resolve.h"
#include "xml.h"
#include "tree_internal.h"
#include "validation.h"

#define LY_NSNC "urn:ietf:params:xml:ns:netconf:base:1.0"

/**
 * @brief Transform data from XML format (prefixes and separate NS definitions) to
 *        JSON format (prefixes are module names instead).
 *        Logs directly.
 *
 * @param[in] ctx Main context with the dictionary.
 * @param[in] xml XML data value.
 *
 * @return Transformed data or NULL on error.
 */
static const char *
transform_data_xml2json(struct ly_ctx *ctx, struct lyxml_elem *xml, int log)
{
    const char *in, *id;
    char *out, *col, *prefix;
    size_t out_size, out_used, id_len, rc;
    struct lys_module *mod;
    struct lyxml_ns *ns;

    in = xml->content;
    out_size = strlen(in)+1;
    out = malloc(out_size);
    out_used = 0;

    while (1) {
        col = strchr(in, ':');
        /* we're finished, copy the remaining part */
        if (!col) {
            strcpy(&out[out_used], in);
            out_used += strlen(in)+1;
            assert(out_size == out_used);
            return lydict_insert_zc(ctx, out);
        }
        id = strpbrk_backwards(col-1, "/ [", (col-in)-1);
        if ((id[0] == '/') || (id[0] == ' ') || (id[0] == '[')) {
            ++id;
        }
        id_len = col-id;
        rc = parse_identifier(id);
        if (rc < id_len) {
            if (log) {
                LOGVAL(LYE_INCHAR, LOGLINE(xml), id[rc], &id[rc]);
            }
            free(out);
            return NULL;
        }

        /* get the module */
        prefix = strndup(id, id_len);
        ns = lyxml_get_ns(xml, prefix);
        free(prefix);
        if (!ns) {
            if (log) {
                LOGVAL(LYE_SPEC, LOGLINE(xml), "XML namespace with prefix \"%.*s\" not defined.", id_len, id);
            }
            free(out);
            return NULL;
        }
        mod = ly_ctx_get_module_by_ns(ctx, ns->value, NULL);
        if (!mod) {
            if (log) {
                LOGVAL(LYE_SPEC, LOGLINE(xml), "Module with the namespace \"%s\" could not be found.", ns->value);
            }
            free(out);
            return NULL;
        }

        /* adjust out size (it can even decrease in some strange cases) */
        out_size += strlen(mod->name)-id_len;
        out = realloc(out, out_size);

        /* copy the data before prefix */
        strncpy(&out[out_used], in, id-in);
        out_used += id-in;

        /* copy the model name */
        strcpy(&out[out_used], mod->name);
        out_used += strlen(mod->name);

        /* copy ':' */
        out[out_used] = ':';
        ++out_used;

        /* finally adjust in pointer for next round */
        in = col+1;
    }

    /* unreachable */
    assert(0);
    return NULL;
}

/* logs directly
 *
 * kind == 0 - unsigned (unum used), 1 - signed (snum used), 2 - floating point (fnum used)
 */
static int
validate_length_range(uint8_t kind, uint64_t unum, int64_t snum, long double fnum, struct lys_type *type,
                      const char *str_val, uint32_t line)
{
    struct len_ran_intv *intv = NULL, *tmp_intv;
    int ret = EXIT_FAILURE;

    if (resolve_len_ran_interval(NULL, type, 0, &intv)) {
        /* already done during schema parsing */
        LOGINT;
        return EXIT_FAILURE;
    }
    if (!intv) {
        return EXIT_SUCCESS;
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
            ret = EXIT_SUCCESS;
            break;
        }
    }

    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }

    if (ret) {
        LOGVAL(LYE_OORVAL, line, (str_val ? str_val : ""));
    }
    return ret;
}

/* logs directly */
static int
validate_pattern(const char *str_val, struct lys_type *type, struct lyxml_elem *xml, int log)
{
    int i, err_offset;
    pcre *precomp;
    char *perl_regex;
    const char *err_ptr;

    assert(type->base == LY_TYPE_STRING);

    if (!str_val) {
        str_val = "";
    }

    if (type->der && validate_pattern(str_val, &type->der->type, xml, log)) {
        return EXIT_FAILURE;
    }

    for (i = 0; i < type->info.str.pat_count; ++i) {
        /*
         * adjust the expression to a Perl equivalent
         *
         * http://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#regexs
         */
        perl_regex = malloc((strlen(type->info.str.patterns[i].expr)+2) * sizeof(char));
        perl_regex[0] = '\0';
        strcat(perl_regex, type->info.str.patterns[i].expr);
        if (strncmp(type->info.str.patterns[i].expr
                + strlen(type->info.str.patterns[i].expr) - 2, ".*", 2)) {
            strcat(perl_regex, "$");
        }

        /* must return 0, already checked during parsing */
        precomp = pcre_compile(perl_regex, PCRE_ANCHORED | PCRE_DOLLAR_ENDONLY | PCRE_NO_AUTO_CAPTURE,
                               &err_ptr, &err_offset, NULL);
        if (!precomp) {
            LOGINT;
            free(perl_regex);
            return EXIT_FAILURE;
        }
        free(perl_regex);

        if (pcre_exec(precomp, NULL, str_val, strlen(str_val), 0, 0, NULL, 0)) {
            free(precomp);
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), str_val, xml->name);
            }
            return EXIT_FAILURE;
        }
        free(precomp);
    }

    return EXIT_SUCCESS;
}

/* does not log */
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

/* logs directly */
static int
parse_int(const char *str_val, struct lyxml_elem *xml, int64_t min, int64_t max, int base, int64_t *ret, int log)
{
    char *strptr;

    if (!str_val) {
        if (log) {
            LOGVAL(LYE_INVAL, LOGLINE(xml), "", xml->name);
        }
        return EXIT_FAILURE;
    }

    /* convert to 64-bit integer, all the redundant characters are handled */
    errno = 0;
    strptr = NULL;
    *ret = strtoll(str_val, &strptr, base);
    if (errno || (*ret < min) || (*ret > max)) {
        if (log) {
            LOGVAL(LYE_OORVAL, LOGLINE(xml), str_val, xml->name);
        }
        return EXIT_FAILURE;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), str_val, xml->name);
            }
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
parse_uint(const char *str_val, struct lyxml_elem *xml, uint64_t max, int base, uint64_t *ret, int log)
{
    char *strptr;

    if (!str_val) {
        if (log) {
            LOGVAL(LYE_INVAL, LOGLINE(xml), "", xml->name);
        }
        return EXIT_FAILURE;
    }

    errno = 0;
    strptr = NULL;
    *ret = strtoull(str_val, &strptr, base);
    if (errno || (*ret > max)) {
        if (log) {
            LOGVAL(LYE_OORVAL, LOGLINE(xml), str_val, xml->name);
        }
        return EXIT_FAILURE;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), str_val, xml->name);
            }
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* does not log, cannot fail */
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

/* logs directly */
static int
_xml_get_value(struct lyd_node *node, struct lys_type *node_type, struct lyxml_elem *xml,
               int options, struct unres_data *unres, int log)
{
    #define DECSIZE 21
    struct lyd_node_leaf *leaf = (struct lyd_node_leaf *)node;
    struct lys_type *type;
    char dec[DECSIZE];
    int64_t num;
    uint64_t unum;
    int len;
    int c, i, j, d;
    int found;

    assert(node && node_type && xml && unres);

    leaf->value_str = xml->content;
    xml->content = NULL;

    /* will be change in case of union */
    leaf->value_type = node_type->base;

    if ((options & LYD_OPT_FILTER) && !leaf->value_str) {
        /* no value in filter (selection) node -> nothing more is needed */
        return EXIT_SUCCESS;
    }

    switch (node_type->base) {
    case LY_TYPE_BINARY:
        leaf->value.binary = leaf->value_str;

        if (validate_length_range(0, (leaf->value.binary ? strlen(leaf->value.binary) : 0), 0, 0, node_type,
                                  leaf->value.binary, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_BITS:
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
                || validate_length_range(2, 0, 0, ((long double)num)/(1 << type->info.dec64.dig), node_type,
                                         leaf->value_str, log ? LOGLINE(xml) : UINT_MAX)) {
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
        if (!leaf->value_str) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), "", xml->name);
            }
            return EXIT_FAILURE;
        }

        /* convert the path from the XML form using XML namespaces into the JSON format
         * using module names as namespaces
         */
        xml->content = leaf->value_str;
        leaf->value_str = transform_data_xml2json(node->schema->module->ctx, xml, log);
        lydict_remove(node->schema->module->ctx, xml->content);
        xml->content = NULL;
        if (!leaf->value_str) {
            return EXIT_FAILURE;
        }

        leaf->value.ident = resolve_identref_json(node_type->info.ident.ref, leaf->value_str,
                                                  log ? LOGLINE(xml) : UINT_MAX);
        if (!leaf->value.ident) {
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

        /* convert the path from the XML form using XML namespaces into the JSON format
         * using module names as namespaces
         */
        xml->content = leaf->value_str;
        leaf->value_str = transform_data_xml2json(node->schema->module->ctx, xml, log);
        lydict_remove(node->schema->module->ctx, xml->content);
        xml->content = NULL;
        if (!leaf->value_str) {
            return EXIT_FAILURE;
        }

        if (options & (LYD_OPT_EDIT | LYD_OPT_FILTER)) {
            leaf->value_type |= LY_TYPE_INST_UNRES;
        } else {
            /* validity checking is performed later, right now the data tree
             * is not complete, so many instanceids cannot be resolved
             */
            /* remember the leaf for later checking */
            if (unres_data_add(unres, node, (log ? LOGLINE(xml) : UINT_MAX))) {
                return EXIT_FAILURE;
            }
        }
        break;

    case LY_TYPE_LEAFREF:
        if (!leaf->value_str) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), "", xml->name);
            }
            return EXIT_FAILURE;
        }

        if (options & (LYD_OPT_EDIT | LYD_OPT_FILTER)) {
            do {
                type = &((struct lys_node_leaf *)leaf->schema)->type.info.lref.target->type;
            } while (type->base == LY_TYPE_LEAFREF);
            leaf->value_type = type->base | LY_TYPE_LEAFREF_UNRES;
        } else {
            /* validity checking is performed later, right now the data tree
             * is not complete, so many leafrefs cannot be resolved
             */
            /* remember the leaf for later checking */
            if (unres_data_add(unres, node, (log ? LOGLINE(xml) : UINT_MAX))) {
                return EXIT_FAILURE;
            }
        }
        break;

    case LY_TYPE_STRING:
        leaf->value.string = leaf->value_str;

        if (validate_length_range(0, (leaf->value.string ? strlen(leaf->value.string) : 0), 0, 0, node_type,
                leaf->value.string, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }

        if (validate_pattern(leaf->value.string, node_type, xml, log)) {
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_UNION:
        found = 0;
        type = get_next_union_type(node_type, NULL, &found);
        for (; type; found = 0, type = get_next_union_type(node_type, type, &found)) {
            xml->content = leaf->value_str;
            if (!_xml_get_value(node, type, xml, options, unres, 0)) {
                leaf->value_type = type->base;
                break;
            }
        }

        if (!type) {
            if (log) {
                LOGVAL(LYE_INVAL, LOGLINE(xml), (leaf->value_str ? leaf->value_str : ""), xml->name);
            }
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_INT8:
        if (parse_int(leaf->value_str, xml, -128, 127, 0, &num, log)
                || validate_length_range(1, 0, num, 0, node_type, leaf->value_str, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }
        leaf->value.int8 = num;
        break;

    case LY_TYPE_INT16:
        if (parse_int(leaf->value_str, xml, -32768, 32767, 0, &num, log)
                || validate_length_range(1, 0, num, 0, node_type, leaf->value_str, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }
        leaf->value.int16 = num;
        break;

    case LY_TYPE_INT32:
        if (parse_int(leaf->value_str, xml, -2147483648, 2147483647, 0, &num, log)
                || validate_length_range(1, 0, num, 0, node_type, leaf->value_str, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }
        leaf->value.int32 = num;
        break;

    case LY_TYPE_INT64:
        if (parse_int(leaf->value_str, xml, -9223372036854775807L - 1L, 9223372036854775807L, 0, &num, log)
                || validate_length_range(1, 0, num, 0, node_type, leaf->value_str, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }
        leaf->value.int64 = num;
        break;

    case LY_TYPE_UINT8:
        if (parse_uint(leaf->value_str, xml, 255, 0, &unum, log)
                || validate_length_range(0, unum, 0, 0, node_type, leaf->value_str, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }
        leaf->value.uint8 = unum;
        break;

    case LY_TYPE_UINT16:
        if (parse_uint(leaf->value_str, xml, 65535, 0, &unum, log)
                || validate_length_range(0, unum, 0, 0, node_type, leaf->value_str, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }
        leaf->value.uint16 = unum;
        break;

    case LY_TYPE_UINT32:
        if (parse_uint(leaf->value_str, xml, 4294967295, 0, &unum, log)
                || validate_length_range(0, unum, 0, 0, node_type, leaf->value_str, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }
        leaf->value.uint32 = unum;
        break;

    case LY_TYPE_UINT64:
        if (parse_uint(leaf->value_str, xml, 18446744073709551615UL, 0, &unum, log)
                || validate_length_range(0, unum, 0, 0, node_type, leaf->value_str, log ? LOGLINE(xml) : UINT_MAX)) {
            return EXIT_FAILURE;
        }
        leaf->value.uint64 = unum;
        break;

    default:
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* logs indirectly */
static int
xml_get_value(struct lyd_node *node, struct lyxml_elem *xml, int options, struct unres_data *unres)
{
    return _xml_get_value(node, &((struct lys_node_leaf *)node->schema)->type, xml, options, unres, 1);
}

/* logs directly */
struct lyd_node *
xml_parse_data(struct ly_ctx *ctx, struct lyxml_elem *xml, struct lyd_node *parent, struct lyd_node *prev,
               int options, struct unres_data *unres)
{
    struct lyd_node *result = NULL, *diter;
    struct lys_node *schema = NULL;
    struct lyxml_attr *attr;
    struct lyxml_elem *prev_xml;
    int i, havechildren;

    if (!xml) {
        LOGINT;
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
        if ((options & LYD_OPT_STRICT) || ly_ctx_get_module_by_ns(ctx, xml->ns->value, NULL)) {
            LOGVAL(LYE_INELEM, LOGLINE(xml), xml->name);
            return NULL;
        } else {
            goto siblings;
        }
    }

    if (lyv_data_context(schema, LOGLINE(xml), options)) {
        return NULL;
    }

    /* check insert attribute and its values */
    if (options & LYD_OPT_EDIT) {
        i = 0;
        for (attr = xml->attr; attr; attr = attr->next) {
            if (attr->type != LYXML_ATTR_STD || !attr->ns ||
                    strcmp(attr->name, "insert") || strcmp(attr->ns->value, LY_NSYANG)) {
                continue;
            }

            /* insert attribute present */
            if (!(schema->flags & LYS_USERORDERED)) {
                /* ... but it is not expected */
                LOGVAL(LYE_INATTR, LOGLINE(xml), "insert", schema->name);
                return NULL;
            }

            if (i) {
                LOGVAL(LYE_TOOMANY, LOGLINE(xml), "insert attributes", xml->name);
                return NULL;
            }
            if (!strcmp(attr->value, "first") || !strcmp(attr->value, "last")) {
                i = 1;
            } else if (!strcmp(attr->value, "before") || !strcmp(attr->value, "after")) {
                i = 2;
            } else {
                LOGVAL(LYE_INARG, LOGLINE(xml), attr->value, attr->name);
                return NULL;
            }
        }

        for (attr = xml->attr; attr; attr = attr->next) {
            if (attr->type != LYXML_ATTR_STD || !attr->ns ||
                    strcmp(attr->name, "value") || strcmp(attr->ns->value, LY_NSYANG)) {
                continue;
            }

            /* the value attribute is present */
            if (i < 2) {
                /* but it shouldn't */
                LOGVAL(LYE_INATTR, LOGLINE(xml), "value", schema->name);
                return NULL;
            }
            i++;
        }
        if (i == 2) {
            /* missing value attribute for "before" or "after" */
            LOGVAL(LYE_MISSATTR, LOGLINE(xml), "value", xml->name);
            return NULL;
        } else if (i > 3) {
            /* more than one instance of the value attribute */
            LOGVAL(LYE_TOOMANY, LOGLINE(xml), "value attributes", xml->name);
            return NULL;
        }
    }

    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
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
    case LYS_ANYXML:
        result = calloc(1, sizeof(struct lyd_node_anyxml));
        havechildren = 0;
        break;
    default:
        LOGINT;
        return NULL;
    }
    result->parent = parent;
    if (parent && !parent->child) {
        parent->child = result;
    }
    if (prev) {
        result->prev = prev;
        prev->next = result;

        /* fix the "last" pointer */
        for (diter = prev; diter->prev != prev; diter = diter->prev);
        diter->prev = result;
    } else {
        result->prev = result;
    }
    result->schema = schema;

    /* type specific processing */
    if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        /* type detection and assigning the value */
        if (xml_get_value(result, xml, options, unres)) {
            goto error;
        }
    } else if (schema->nodetype == LYS_ANYXML && !(options & LYD_OPT_FILTER)) {
        prev_xml = xml->prev;
        ((struct lyd_node_anyxml *)result)->value = xml;
        lyxml_unlink_elem(ctx, xml, 1);
        /* pretend we're processing previous element,
         * so that next is correct (after unlinking xml)
         */
        xml = prev_xml;
    }

    /* process children */
    if (havechildren && xml->child) {
        xml_parse_data(ctx, xml->child, result, NULL, options, unres);
        if (ly_errno) {
            goto error;
        }
    }

    result->attr = (struct lyd_attr *)xml->attr;
    xml->attr = NULL;

    /* various validation checks */
    ly_errno = 0;
    if (lyv_data_content(result, LOGLINE(xml), options)) {
        if (ly_errno) {
            goto error;
        } else {
            goto cleargotosiblings;
        }
    }

siblings:
    /* process siblings */
    if (xml->next) {
        if (result) {
            xml_parse_data(ctx, xml->next, parent, result, options, unres);
        } else {
            xml_parse_data(ctx, xml->next, parent, prev, options, unres);
        }
        if (ly_errno) {
            goto error;
        }
    }

    return result;

error:

    /* cleanup */
    lyd_free(result);

    return NULL;

cleargotosiblings:

    /* remove the result ... */
    lyd_free(result);
    result = NULL;

    /* ... and then go to siblings label */
    goto siblings;
}

/* logs indirectly */
struct lyd_node *
xml_read_data(struct ly_ctx *ctx, const char *data, int options)
{
    struct lyxml_elem *xml;
    struct lyd_node *result, *next, *iter;
    struct unres_data *unres = NULL;

    xml = lyxml_read(ctx, data, 0);
    if (!xml) {
        return NULL;
    }

    unres = calloc(1, sizeof *unres);

    ly_errno = 0;
    result = xml_parse_data(ctx, xml->child, NULL, NULL, options, unres);

    /* check leafrefs and/or instids if any */
    if (result && resolve_unres_data(unres)) {
        /* leafref & instid checking failed */
        LY_TREE_FOR_SAFE(result, next, iter) {
            lyd_free(iter);
        }
        result = NULL;
    }

    free(unres->dnode);
#ifndef NDEBUG
    free(unres->line);
#endif
    free(unres);

    /* free source XML tree */
    lyxml_free_elem(ctx, xml);

    return result;
}
