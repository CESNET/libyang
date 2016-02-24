/**
 * @file parser_json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief JSON data parser for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "parser.h"
#include "printer.h"
#include "tree_internal.h"
#include "validation.h"

static int
lyjson_isspace(int c)
{
    switch(c) {
    case 0x20: /* space */
    case 0x09: /* horizontal tab */
    case 0x0a: /* line feed or new line */
    case 0x0d: /* carriage return */
        return 1;
    default:
        return 0;
    }
}

static unsigned int
skip_ws(const char *data)
{
    unsigned int len = 0;

    /* skip leading whitespaces */
    while (data[len] && lyjson_isspace(data[len])) {
        COUNTLINE(data[len]);
        len++;
    }

    return len;
}

static char *
lyjson_parse_text(const char *data, unsigned int *len)
{
#define BUFSIZE 1024

    char buf[BUFSIZE];
    char *result = NULL, *aux;
    int o, size = 0;
    unsigned int r, i;
    int32_t value;

    for (*len = o = 0; data[*len] && data[*len] != '"'; o++) {
        if (o > BUFSIZE - 3) {
            /* add buffer into the result */
            if (result) {
                size = size + o;
                aux = ly_realloc(result, size + 1);
                if (!aux) {
                    LOGMEM;
                    return NULL;
                }
                result = aux;
            } else {
                size = o;
                result = malloc((size + 1) * sizeof *result);
                if (!result) {
                    LOGMEM;
                    return NULL;
                }
            }
            memcpy(&result[size - o], buf, o);

            /* write again into the beginning of the buffer */
            o = 0;
        }

        if (data[*len] == '\\') {
            /* parse escape sequence */
            (*len)++;
            i = 1;
            switch (data[(*len)]) {
            case '"':
                /* quotation mark */
                value = 0x22;
                break;
            case '\\':
                /* reverse solidus */
                value = 0x5c;
                break;
            case '/':
                /* solidus */
                value = 0x2f;
                break;
            case 'b':
                /* backspace */
                value = 0x08;
                break;
            case 'f':
                /* form feed */
                value = 0x0c;
                break;
            case 'n':
                /* line feed */
                value = 0x0a;
                break;
            case 'r':
                /* carriage return */
                value = 0x0d;
                break;
            case 't':
                /* tab */
                value = 0x09;
                break;
            case 'u':
                /* Basic Multilingual Plane character \uXXXX */
                (*len)++;
                for (value = i = 0; i < 4; i++) {
                    if (isdigit(data[(*len) + i])) {
                        r = (data[(*len) + i] - '0');
                    } else if (data[(*len) + i] > 'F') {
                        r = 10 + (data[(*len) + i] - 'a');
                    } else {
                        r = 10 + (data[(*len) + i] - 'A');
                    }
                    value = (16 * value) + r;
                }
                break;
            default:
                /* invalid escape sequence */
                LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "character escape sequence");
                goto error;

            }
            r = pututf8(&buf[o], value, lineno);
            if (!r) {
                LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "character UTF8 character");
                goto error;
            }
            o += r - 1; /* o is ++ in for loop */
            (*len) += i; /* number of read characters */
        } else if (data[*len] < 0x20 || data[*len] == 0x5c) {
            /* control characters must be escaped */
            LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "control character (unescaped)");
            goto error;
        } else {
            /* unescaped character */
            buf[o] = data[*len];
            COUNTLINE(buf[o]);
            (*len)++;
        }
    }

#undef BUFSIZE

    if (o) {
        if (result) {
            size = size + o;
            aux = ly_realloc(result, size + 1);
            if (!aux) {
                LOGMEM;
                return NULL;
            }
            result = aux;
        } else {
            size = o;
            result = malloc((size + 1) * sizeof *result);
            if (!result) {
                LOGMEM;
                return NULL;
            }
        }
        memcpy(&result[size - o], buf, o);
    }
    if (result) {
        result[size] = '\0';
    } else {
        size = 0;
        result = strdup("");
    }

    return result;

error:
    free(result);
    return NULL;
}

static unsigned int
lyjson_parse_number(const char *data)
{
    unsigned int len;
    unsigned int i = 0;

    for (len = 0;
         data[len] && data[len] != ',' && data[len] != ']' && data[len] != '}' && !lyjson_isspace(data[len]);
         len++) {

        switch(data[len]) {
        case '0':
            if (!i && isdigit(data[len + 1])) {
                /* leading 0 is not allowed */
                LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON number (leading zero)");
                return 0;
            }
            /* no break */
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            i = 1;
            /* no break */
        case 0x2d: /* minus */
            /* ok */
            break;
        default:
            LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "character in JSON Number value");
            return 0;
        }
    }

    return len;
}

static unsigned int
lyjson_parse_boolean(const char *data)
{
    unsigned int len = 0;

    if (!strncmp(data, "false", 5)) {
        len = 5;
    } else if (!strncmp(data, "true", 4)) {
        len = 4;
    }

    if (data[len] && data[len] != ',' && data[len] != ']' && data[len] != '}' && !lyjson_isspace(data[len])) {
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON literal value (expected true or false)");
        return 0;
    }

    return len;
}

static unsigned int
json_get_anyxml(struct lyd_node_anyxml *axml, const char *data)
{
    unsigned int len = 0;
    char stop, start;
    int level = 0;

    switch (data[len]) {
    case '"':
        start = 0;
        stop = '"';
        level = 1;
        break;
    case '[':
        start = '[';
        stop = ']';
        break;
    case '{':
        start = '{';
        stop = '}';
        break;
    default:
        /* number or one of literals */
        while (!isspace(data[len])) {
            len++;
        }
        axml->value = NULL; /* TODO ??? */
        return len;
    }

    while (data[len]) {
        if (start && data[len] == start) {
            if (!len || data[len - 1] != '\\') {
                level++;
            }
        } else if (data[len] == stop) {
            if (!len || data[len - 1] != '\\') {
                level--;
            }
        }
        stop = len;
        len++;
        if (!level) {
            /* we are done */
            axml->value = NULL; /* TODO ??? */
            return len;
        }
    }

    return 0;
}

static unsigned int
json_get_value(struct lyd_node_leaf_list *leaf, const char *data, int options, struct unres_data *unres)
{
    struct lyd_node_leaf_list *new, *diter;
    struct lys_type *stype;
    struct ly_ctx *ctx;
    unsigned int len = 0, r;
    int resolve;
    char *str;

    assert(leaf && data && unres);
    ctx = leaf->schema->module->ctx;

    if (options & (LYD_OPT_FILTER | LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) {
        resolve = 0;
    } else {
        resolve = 1;
    }

    stype = &((struct lys_node_leaf *)leaf->schema)->type;

    if (options & LYD_OPT_FILTER) {
        /* no value in filter (selection) node is accepted in this case */
        if (!strncmp(&data[len], "null", 4)) {
            leaf->value_type = stype->base;
            len +=4;
            goto end;
        }
    }

    if (leaf->schema->nodetype == LYS_LEAFLIST) {
        /* expecting begin-array */
        if (data[len++] != '[') {
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, leaf, "JSON data (expected begin-array)");
            return 0;
        }

repeat:
        len += skip_ws(&data[len]);
    }

    /* will be changed in case of union */
    leaf->value_type = stype->base;

    if (data[len] == '"') {
        /* string representations */
        if (data[len++] != '"') {
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, leaf,
                   "JSON data (missing quotation-mark at the beginning of string)");
            return 0;
        }
        str = lyjson_parse_text(&data[len], &r);
        if (!str) {
            LOGVAL(LYE_PATH, 0, LY_VLOG_LYD, leaf);
            return 0;
        }
        leaf->value_str = lydict_insert_zc(ctx, str);
        if (data[len + r] != '"') {
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, leaf,
                   "JSON data (missing quotation-mark at the end of string)");
            return 0;
        }
        len += r + 1;
    } else if (data[len] == '-' || isdigit(data[len])) {
        /* numeric type */
        r = lyjson_parse_number(&data[len]);
        if (!r) {
            LOGVAL(LYE_PATH, 0, LY_VLOG_LYD, leaf);
            return 0;
        }
        leaf->value_str = lydict_insert(ctx, &data[len], r);
        len += r;
    } else if (data[len] == 'f' || data[len] == 't') {
        /* boolean */
        r = lyjson_parse_boolean(&data[len]);
        if (!r) {
            LOGVAL(LYE_PATH, 0, LY_VLOG_LYD, leaf);
            return 0;
        }
        leaf->value_str = lydict_insert(ctx, &data[len], r);
        len += r;
    } else if (!strncmp(&data[len], "[null]", 6)) {
        /* empty */
        leaf->value_str = NULL;
        len += 6;
    } else {
        /* error */
        LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, leaf, "JSON data (unexpected value)");
        return 0;
    }

    if (lyp_parse_value(leaf, NULL, resolve, unres, lineno)) {
        ly_errno = LY_EVALID;
        return 0;
    }

    if (leaf->schema->nodetype == LYS_LEAFLIST) {
        /* repeat until end-array */
        len += skip_ws(&data[len]);
        if (data[len] == ',') {
            /* another instance of the leaf-list */
            new = calloc(1, sizeof(struct lyd_node_leaf_list));
            if (!new) {
                LOGMEM;
                return 0;
            }
            new->parent = leaf->parent;
            new->prev = (struct lyd_node *)leaf;
            leaf->next = (struct lyd_node *)new;

            /* fix the "last" pointer */
            for (diter = leaf; diter->prev != (struct lyd_node *)leaf; diter = (struct lyd_node_leaf_list *)diter->prev);
            diter->prev = (struct lyd_node *)new;

            new->schema = leaf->schema;

            /* repeat value parsing */
            leaf = new;
            len++;
            goto repeat;
        } else if (data[len] == ']') {
            len++;
            len += skip_ws(&data[len]);
        } else {
            /* something unexpected */
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, leaf, "JSON data (expecting value-separator or end-array)");
            return 0;
        }
    }

end:
    len += skip_ws(&data[len]);
    return len;
}

static unsigned int
json_parse_attr(struct lys_module *parent_module, struct lyd_attr **attr, const char *data)
{
    unsigned int len = 0, r;
    char *str = NULL, *name, *prefix, *value;
    struct lys_module *module = parent_module;
    struct lyd_attr *attr_new, *attr_last = NULL;

    *attr = NULL;

    if (data[len] != '{') {
        if (!strncmp(&data[len], "null", 4)) {
            len += 4;
            len += skip_ws(&data[len]);
            return len;
        }
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON data (missing begin-object)");
        goto error;
    }

repeat:
    len++;
    len += skip_ws(&data[len]);

    if (data[len] != '"') {
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON data (missing quotation-mark at the begining of string)");
        return 0;
    }
    len++;
    str = lyjson_parse_text(&data[len], &r);
    if (!r) {
        goto error;
    } else if (data[len + r] != '"') {
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON data (missing quotation-mark at the end of string)");
        goto error;
    }
    if ((name = strchr(str, ':'))) {
        *name = '\0';
        name++;
        prefix = str;
        module = (struct lys_module *)ly_ctx_get_module(parent_module->ctx, prefix, NULL);
        if (!module) {
            LOGVAL(LYE_INELEM, lineno, 0, NULL, name);
            goto error;
        }
    } else {
        name = str;
    }

    /* prepare data for parsing node content */
    len += r + 1;
    len += skip_ws(&data[len]);
    if (data[len] != ':') {
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON data (missing name-separator)");
        goto error;
    }
    len++;
    len += skip_ws(&data[len]);

    if (data[len] != '"') {
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON data (missing quotation-mark at the beginning of string)");
        goto error;
    }
    len++;
    value = lyjson_parse_text(&data[len], &r);
    if (!r) {
        goto error;
    } else if (data[len + r] != '"') {
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON data (missing quotation-mark at the end of string)");
        free(value);
        goto error;
    }
    len += r + 1;
    len += skip_ws(&data[len]);

    attr_new = malloc(sizeof **attr);
    if (!attr_new) {
        LOGMEM;
        goto error;
    }
    attr_new->module = module;
    attr_new->next = NULL;
    attr_new->name = lydict_insert(module->ctx, name, 0);
    attr_new->value = lydict_insert_zc(module->ctx, value);
    if (!attr_last) {
        *attr = attr_last = attr_new;
    } else {
        attr_last->next = attr_new;
        attr_last = attr_new;
    }

    free(str);
    str = NULL;

    if (data[len] == ',') {
        goto repeat;
    } else if (data[len] != '}') {
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON data (missing end-object)");
        goto error;
    }
    len++;
    len += skip_ws(&data[len]);

    return len;

error:
    free(str);
    if (*attr) {
        lyd_free_attr((*attr)->module->ctx, NULL, *attr, 1);
        *attr = NULL;
    }
    return 0;
}

struct attr_cont {
    struct attr_cont *next;
    struct lyd_attr *attr;
    struct lys_node *schema;
    unsigned int index;    /** non-zero only in case of leaf-list */
};

static int
store_attrs(struct ly_ctx *ctx, struct attr_cont *attrs, struct lyd_node *first)
{
    struct lyd_node *diter;
    struct attr_cont *iter;
    unsigned int flag_leaflist = 0;

    while (attrs) {
        iter = attrs;
        attrs = attrs->next;

        if (iter->index) {
            flag_leaflist = 1;
        }

        LY_TREE_FOR(first, diter) {
            if (iter->schema != diter->schema) {
                continue;
            }

            if (flag_leaflist && flag_leaflist != iter->index) {
                flag_leaflist++;
                continue;
            }

            /* we have match */
            if (diter->attr) {
                LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, diter,
                       "attribute (multiple attribute definitions belong to a single element)");
                free(iter);
                goto error;
            }

            diter->attr = iter->attr;
            break;
        }

        if (!diter) {
            LOGVAL(LYE_XML_MISS, lineno, 0, NULL, "element for the specified attribute", iter->attr->name);
            lyd_free_attr(iter->schema->module->ctx, NULL, iter->attr, 1);
            free(iter);
            goto error;
        }
        free(iter);
    }

    return 0;

error:

    while (attrs) {
        iter = attrs;
        attrs = attrs->next;

        lyd_free_attr(ctx, NULL, iter->attr, 1);
        free(iter);
    }

    return -1;
}

static unsigned int
json_parse_data(struct ly_ctx *ctx, const char *data, const struct lys_node *schema_parent, struct lyd_node **parent,
                struct lyd_node *prev, struct attr_cont **attrs, int options, struct unres_data *unres)
{
    unsigned int len = 0;
    unsigned int r;
    unsigned int flag_leaflist = 0;
    char *name, *prefix = NULL, *str = NULL;
    const struct lys_module *module = NULL;
    struct lys_node *schema = NULL;
    struct lyd_node *result = NULL, *new, *list, *diter = NULL;
    struct lyd_attr *attr;
    struct attr_cont *attrs_aux;

    /* each YANG data node representation starts with string (node identifier) */
    if (data[len] != '"') {
        LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, (*parent),
               "JSON data (missing quotation-mark at the beginning of string)");
        return 0;
    }
    len++;

    str = lyjson_parse_text(&data[len], &r);
    if (!r) {
        goto error;
    } else if (data[len + r] != '"') {
        LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, (*parent),
               "JSON data (missing quotation-mark at the end of string)");
        goto error;
    }
    if ((name = strchr(str, ':'))) {
        *name = '\0';
        name++;
        prefix = str;
        if (prefix[0] == '@') {
            prefix++;
        }
    } else {
        name = str;
        if (name[0] == '@') {
            name++;
        }
    }

    /* prepare data for parsing node content */
    len += r + 1;
    len += skip_ws(&data[len]);
    if (data[len] != ':') {
        LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, (*parent), "JSON data (missing name-separator)");
        goto error;
    }
    len++;
    len += skip_ws(&data[len]);

    if (str[0] == '@' && !str[1]) {
        /* process attribute of the parent object (container or list) */
        if (!*parent) {
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, (*parent), "attribute with no corresponding element to belongs to");
            goto error;
        }

        r = json_parse_attr((*parent)->schema->module, &attr, &data[len]);
        if (!r) {
            LOGVAL(LYE_PATH, 0, LY_VLOG_LYD, (*parent));
            goto error;
        }
        len += r;

        if ((*parent)->attr) {
            lyd_free_attr(ctx, NULL, attr, 1);
        } else {
            (*parent)->attr = attr;
        }
        free(str);
        return len;
    }

    /* find schema node */
    if (!(*parent)) {
        /* starting in root */
        /* get the proper schema */
        module = ly_ctx_get_module(ctx, prefix, NULL);
        if (module) {
            /* get the proper schema node */
            LY_TREE_FOR(module->data, schema) {
                /* skip nodes in module's data which are not expected here according to options' data type */
                if (options & LYD_OPT_RPC) {
                    if (schema->nodetype != LYS_RPC) {
                        continue;
                    }
                } else if (options & LYD_OPT_NOTIF) {
                    if (schema->nodetype != LYS_NOTIF) {
                        continue;
                    }
                } else if (!(options & LYD_OPT_RPCREPLY)) {
                    /* rest of the data types except RPCREPLY which cannot be here */
                    if (schema->nodetype & (LYS_RPC | LYS_NOTIF)) {
                        continue;
                    }
                }
                if (!strcmp(schema->name, name)) {
                    break;
                }
            }
        } else {
            LOGVAL(LYE_INELEM, lineno, LY_VLOG_LYD, (*parent), name);
            goto error;
        }
    } else {
        /* parsing some internal node, we start with parent's schema pointer */
        if (prefix) {
            /* get the proper schema */
            module = ly_ctx_get_module(ctx, prefix, NULL);
            if (!module) {
                LOGVAL(LYE_INELEM, lineno, LY_VLOG_LYD, (*parent), name);
                goto error;
            }
        }

        /* go through RPC's input/output following the options' data type */
        if ((*parent)->schema->nodetype == LYS_RPC) {
            while ((schema = (struct lys_node *)lys_getnext(schema, (*parent)->schema, module, LYS_GETNEXT_WITHINOUT))) {
                if ((options & LYD_OPT_RPC) && schema->nodetype == LYS_INPUT) {
                    break;
                } else if ((options & LYD_OPT_RPCREPLY) && schema->nodetype == LYS_OUTPUT) {
                    break;
                }
            }
            if (!schema) {
                LOGVAL(LYE_INELEM, lineno, LY_VLOG_LYD, (*parent), name);
                goto error;
            }
            schema_parent = schema;
            schema = NULL;
        }

        if (schema_parent) {
            while ((schema = (struct lys_node *)lys_getnext(schema, schema_parent, module, 0))) {
                if (!strcmp(schema->name, name)) {
                    break;
                }
            }
        } else {
            while ((schema = (struct lys_node *)lys_getnext(schema, (*parent)->schema, module, 0))) {
                if (!strcmp(schema->name, name)) {
                    break;
                }
            }
        }
    }
    if (!schema) {
        LOGVAL(LYE_INELEM, lineno, LY_VLOG_LYD, (*parent), name);
        goto error;
    }

    if (str[0] == '@') {
        /* attribute for some sibling node */
        if (data[len] == '[') {
            flag_leaflist = 1;
            len++;
            len += skip_ws(&data[len]);
        }

attr_repeat:
        r = json_parse_attr(schema->module, &attr, &data[len]);
        if (!r) {
            LOGVAL(LYE_PATH, 0, LY_VLOG_LYD, (*parent));
            goto error;
        }
        len += r;

        if (attr) {
            attrs_aux = malloc(sizeof *attrs_aux);
            if (!attrs_aux) {
                LOGMEM;
                goto error;
            }
            attrs_aux->attr = attr;
            attrs_aux->index = flag_leaflist;
            attrs_aux->schema = schema;
            attrs_aux->next = *attrs;
            *attrs = attrs_aux;
        } else if (!flag_leaflist) {
            /* error */
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, (*parent), "attribute data");
            goto error;
        }

        if (flag_leaflist) {
            if (data[len] == ',') {
                len++;
                len += skip_ws(&data[len]);
                flag_leaflist++;
                goto attr_repeat;
            } else if (data[len] != ']') {
                LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, (*parent), "JSON data (missing end-array)");
                goto error;
            }
            len++;
            len += skip_ws(&data[len]);
        }

        free(str);
        return len;
    }

    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_NOTIF:
    case LYS_RPC:
        result = calloc(1, sizeof *result);
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        result = calloc(1, sizeof(struct lyd_node_leaf_list));
        break;
    case LYS_ANYXML:
        result = calloc(1, sizeof(struct lyd_node_anyxml));
        break;
    default:
        LOGINT;
        goto error;
    }
    if (!result) {
        LOGMEM;
        goto error;
    }

    result->parent = *parent;
    if (*parent && !(*parent)->child) {
        (*parent)->child = result;
    }
    if (prev) {
        result->prev = prev;
        prev->next = result;

        /* fix the "last" pointer */
        if (*parent) {
            diter = (*parent)->child;
        } else {
            for (diter = prev; diter->prev != prev; diter = diter->prev);
        }
        diter->prev = result;
    } else {
        result->prev = result;
    }
    result->schema = schema;
    result->validity = LYD_VAL_NOT;

    if (!(options & LYD_OPT_TRUSTED) && lyv_data_context(result, options, lineno, unres)) {
        goto error;
    }

    /* TODO handle notifications */

    /* type specific processing */
    if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        /* type detection and assigning the value */
        r = json_get_value((struct lyd_node_leaf_list *)result, &data[len], options, unres);
        if (!r) {
            goto error;
        }
        while(result->next) {
            result = result->next;
        }

        len += r;
        len += skip_ws(&data[len]);
    } else if (schema->nodetype == LYS_ANYXML) {
        r = json_get_anyxml((struct lyd_node_anyxml *)result, &data[len]);
        if (!r) {
            goto error;
        }
        len += r;
        len += skip_ws(&data[len]);
    } else if (schema->nodetype & (LYS_CONTAINER | LYS_RPC)) {
        if (data[len] != '{') {
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, result, "JSON data (missing begin-object)");
            goto error;
        }
        len++;
        len += skip_ws(&data[len]);

        if (data[len] != '}') {
            /* non-empty container */
            len--;
            diter = NULL;
            attrs_aux = NULL;
            do {
                len++;
                len += skip_ws(&data[len]);

                r = json_parse_data(ctx, &data[len], NULL, &result, diter, &attrs_aux, options, unres);
                if (!r) {
                    goto error;
                }
                len += r;

                if (result->child) {
                    diter = result->child->prev;
                }
            } while(data[len] == ',');

            /* store attributes */
            if (store_attrs(ctx, attrs_aux, result->child)) {
                goto error;
            }
        }

        if (data[len] != '}') {
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, result, "JSON data (missing end-object)");
            goto error;
        }
        len++;
        len += skip_ws(&data[len]);

    } else if (schema->nodetype == LYS_LIST) {
        if (data[len] != '[') {
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, result, "JSON data (missing begin-array)");
            goto error;
        }

        list = result;
        do {
            len++;
            len += skip_ws(&data[len]);

            if (options & LYD_OPT_FILTER) {
                /* filter selection node ? */
                if (data[len] == ']') {
                    break;
                } else if (!strcmp(&data[len], "null")) {
                    len += 4;
                    len += skip_ws(&data[len]);
                    break;
                }
            }

            if (data[len] != '{') {
                LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, result,
                       "JSON data (missing list instance's begin-object)");
                goto error;
            }
            diter = NULL;
            attrs_aux = NULL;
            do {
                len++;
                len += skip_ws(&data[len]);

                r = json_parse_data(ctx, &data[len], NULL, &list, diter, &attrs_aux, options, unres);
                if (!r) {
                    goto error;
                }
                len += r;

                if (list->child) {
                    diter = list->child->prev;
                }
            } while(data[len] == ',');

            /* store attributes */
            if (store_attrs(ctx, attrs_aux, list->child)) {
                goto error;
            }

            if (data[len] != '}') {
                /* expecting end-object */
                LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, result,
                       "JSON data (missing list instance's end-object)");
                goto error;
            }
            len++;
            len += skip_ws(&data[len]);

            if (data[len] == ',') {
                /* various validation checks */
                ly_errno = 0;
                if (!(options & LYD_OPT_TRUSTED) && lyv_data_content(list, options, lineno, unres)) {
                    if (ly_errno) {
                        goto error;
                    }
                }
                /* validation successful */
                list->validity = LYD_VAL_OK;

                /* another instance of the list */
                new = calloc(1, sizeof *new);
                if (!new) {
                    goto error;
                }
                new->parent = list->parent;
                new->prev = list;
                list->next = new;

                /* fix the "last" pointer */
                if (*parent) {
                    diter = (*parent)->child;
                } else {
                    for (diter = prev; diter->prev != prev; diter = diter->prev);
                }
                diter->prev = new;

                new->schema = list->schema;
                list = new;
            }
        } while (data[len] == ',');
        result = list;

        if (data[len] != ']') {
            LOGVAL(LYE_XML_INVAL, lineno, LY_VLOG_LYD, result,
                   "JSON data (missing end-array)");
            goto error;
        }
        len++;
        len += skip_ws(&data[len]);
    }

    /* various validation checks */
    ly_errno = 0;
    if (!(options & LYD_OPT_TRUSTED) && lyv_data_content(result, options, lineno, unres)) {
        if (ly_errno) {
            goto error;
        }
    }

    /* validation successful */
    result->validity = LYD_VAL_OK;

    if (!(*parent)) {
        *parent = result;
    }

    free(str);
    return len;

error:
    if ((*parent) == result) {
        (*parent) = NULL;
    }
    while (*attrs) {
        attrs_aux = *attrs;
        *attrs = (*attrs)->next;

        lyd_free_attr(ctx, NULL, attrs_aux->attr, 1);
        free(attrs_aux);
    }

    lyd_free(result);
    free(str);

    return 0;
}

struct lyd_node *
lyd_parse_json(struct ly_ctx *ctx, const struct lys_node *parent, const char *data, int options)
{
    struct lyd_node *result = NULL, *next = NULL, *iter = NULL;
    struct unres_data *unres = NULL;
    unsigned int len = 0, r;
    struct attr_cont *attrs = NULL;

    ly_errno = LY_SUCCESS;

    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    unres = calloc(1, sizeof *unres);
    if (!unres) {
        LOGMEM;
        return NULL;
    }

#ifndef NDEBUG
    lineno = 0;
#endif

    /* skip leading whitespaces */
    len += skip_ws(&data[len]);

    /* expect top-level { */
    if (data[len] != '{') {
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON data (missing top level begin-object)");
        goto cleanup;
    }

    do {
        len++;
        len += skip_ws(&data[len]);

        r = json_parse_data(ctx, &data[len], parent, &next, iter, &attrs, options, unres);
        if (!r) {
            result = NULL;
            goto cleanup;
        }
        len += r;

        if (!result) {
            result = next;
        }
        if (next) {
            iter = next;
        }
        next = NULL;
    } while(data[len] == ',');

    if (data[len] != '}') {
        /* expecting end-object */
        LOGVAL(LYE_XML_INVAL, lineno, 0, NULL, "JSON data (missing top-level end-object)");
        LY_TREE_FOR_SAFE(result, next, iter) {
            lyd_free(iter);
        }
        result = NULL;
        goto cleanup;
    }
    len++;
    len += skip_ws(&data[len]);

    /* store attributes */
    if (store_attrs(ctx, attrs, result)) {
        LY_TREE_FOR_SAFE(result, next, iter) {
            lyd_free(iter);
        }
        result = NULL;
        goto cleanup;
    }

    if (!result) {
        LOGERR(LY_EVALID, "Model for the data to be linked with not found.");
        goto cleanup;
    }

    /* check leafrefs and/or instids if any */
    if (result && resolve_unres_data(unres)) {
        /* leafref & instid checking failed */
        LY_TREE_FOR_SAFE(result, next, iter) {
            lyd_free(iter);
        }
        result = NULL;
    }

cleanup:
    free(unres->node);
    free(unres->type);
#ifndef NDEBUG
    free(unres->line);
#endif
    free(unres);

    return result;
}
