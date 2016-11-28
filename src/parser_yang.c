/**
 * @file parser_yang.c
 * @author Pavol Vican
 * @brief YANG parser for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <ctype.h>
#include <assert.h>
#include "parser_yang.h"
#include "parser_yang_lex.h"
#include "parser.h"
#include "xpath.h"

static void yang_free_import(struct ly_ctx *ctx, struct lys_import *imp, uint8_t start, uint8_t size);
static void yang_free_include(struct ly_ctx *ctx, struct lys_include *inc, uint8_t start, uint8_t size);
static int yang_check_sub_module(struct lys_module *module, struct unres_schema *unres, struct lys_node *node);
static void free_yang_common(struct lys_module *module, struct lys_node *node);
void lys_iffeature_free(struct lys_iffeature *iffeature, uint8_t iffeature_size);

static int
yang_check_string(struct lys_module *module, const char **target, char *what, char *where, char *value)
{
    if (*target) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, what, where);
        free(value);
        return 1;
    } else {
        *target = lydict_insert_zc(module->ctx, value);
        return 0;
    }
}

int
yang_read_common(struct lys_module *module, char *value, enum yytokentype type)
{
    int ret = 0;

    switch (type) {
    case MODULE_KEYWORD:
        module->name = lydict_insert_zc(module->ctx, value);
        break;
    case NAMESPACE_KEYWORD:
        ret = yang_check_string(module, &module->ns, "namespace", "module", value);
        break;
    case ORGANIZATION_KEYWORD:
        ret = yang_check_string(module, &module->org, "organization", "module", value);
        break;
    case CONTACT_KEYWORD:
        ret = yang_check_string(module, &module->contact, "contact", "module", value);
        break;
    default:
        free(value);
        LOGINT;
        ret = EXIT_FAILURE;
        break;
    }

    return ret;
}

int
yang_check_version(struct lys_module *module, struct lys_submodule *submodule, char *value, int repeat)
{
    int ret = EXIT_SUCCESS;

    if (repeat) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "yang version", "module");
        ret = EXIT_FAILURE;
    } else {
        if (!strcmp(value, "1")) {
            if (submodule) {
                if (module->version > 1) {
                    LOGVAL(LYE_INVER, LY_VLOG_NONE, NULL);
                    ret = EXIT_FAILURE;
                 }
            } else {
                module->version = 1;
            }
        } else if (!strcmp(value, "1.1")) {
            if (submodule) {
                if (module->version != 2) {
                    LOGVAL(LYE_INVER, LY_VLOG_NONE, NULL);
                    ret = EXIT_FAILURE;
                }
            } else {
                module->version = 2;
            }
        } else {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "yang-version");
            ret = EXIT_FAILURE;
        }
    }
    free(value);
    return ret;
}

int
yang_read_prefix(struct lys_module *module, struct lys_import *imp, char *value)
{
    int ret = 0;

    if (!imp && lyp_check_identifier(value, LY_IDENT_PREFIX, module, NULL)) {
        free(value);
        return EXIT_FAILURE;
    }

    if (imp) {
        ret = yang_check_string(module, &imp->prefix, "prefix", "import", value);
    } else {
        ret = yang_check_string(module, &module->prefix, "prefix", "module", value);
    }

    return ret;
}

static int
yang_fill_import(struct lys_module *module, struct lys_import *imp_old, struct lys_import *imp_new, char *value)
{
    const char *exp;
    int rc;

    if (!imp_old->prefix) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", "import");
        goto error;
    } else {
        if (lyp_check_identifier(imp_old->prefix, LY_IDENT_PREFIX, module, NULL)) {
            goto error;
        }
    }
    memcpy(imp_new, imp_old, sizeof *imp_old);
    exp = lydict_insert_zc(module->ctx, value);
    rc = lyp_check_import(module, exp, imp_new);
    lydict_remove(module->ctx, exp);
    module->imp_size++;
    if (rc) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

error:
    free(value);
    lydict_remove(module->ctx, imp_old->dsc);
    lydict_remove(module->ctx, imp_old->ref);
    return EXIT_FAILURE;
}

int
yang_read_description(struct lys_module *module, void *node, char *value, char *where)
{
    int ret;
    char *dsc = "description";

    if (!node) {
        ret = yang_check_string(module, &module->dsc, dsc, "module", value);
    } else {
        if (!strcmp("revision", where)) {
            ret = yang_check_string(module, &((struct lys_revision *)node)->dsc, dsc, where, value);
        } else if (!strcmp("import", where)){
            ret = yang_check_string(module, &((struct lys_import *)node)->dsc, dsc, where, value);
        } else if (!strcmp("include", where)){
            ret = yang_check_string(module, &((struct lys_include *)node)->dsc, dsc, where, value);
        } else {
            ret = yang_check_string(module, &((struct lys_node *)node)->dsc, dsc, where, value);
        }
    }
    return ret;
}

int
yang_read_reference(struct lys_module *module, void *node, char *value, char *where)
{
    int ret;
    char *ref = "reference";

    if (!node) {
        ret = yang_check_string(module, &module->ref, "reference", "module", value);
    } else {
        if (!strcmp("revision", where)) {
            ret = yang_check_string(module, &((struct lys_revision *)node)->ref, ref, where, value);
        } else if (!strcmp("import", where)){
            ret = yang_check_string(module, &((struct lys_import *)node)->ref, ref, where, value);
        } else if (!strcmp("include", where)){
            ret = yang_check_string(module, &((struct lys_include *)node)->ref, ref, where, value);
        } else {
            ret = yang_check_string(module, &((struct lys_node *)node)->ref, ref, where, value);
        }
    }
    return ret;
}

void
yang_read_revision(struct lys_module *module, char *value, struct lys_revision *retval)
{
    /* first member of array is last revision */
    if ((module->rev_size - 1) && strcmp(module->rev[0].date, value) < 0) {
        memcpy(retval->date, module->rev[0].date, LY_REV_SIZE);
        memcpy(module->rev[0].date, value, LY_REV_SIZE);
        retval->dsc = module->rev[0].dsc;
        retval->ref = module->rev[0].ref;
        retval = module->rev;
        retval->dsc = NULL;
        retval->ref = NULL;
    } else {
        memcpy(retval->date, value, LY_REV_SIZE);
    }
    free(value);
}

int
yang_add_elem(struct lys_node_array **node, uint32_t *size)
{
    if (!(*size % LY_ARRAY_SIZE)) {
        if (!(*node = ly_realloc(*node, (*size + LY_ARRAY_SIZE) * sizeof **node))) {
            LOGMEM;
            return EXIT_FAILURE;
        } else {
            memset(*node + *size, 0, LY_ARRAY_SIZE * sizeof **node);
        }
    }
    (*size)++;
    return EXIT_SUCCESS;
}

int
yang_read_if_feature(struct lys_module *module, void *ptr, void *parent, char *value,
                     struct unres_schema *unres, enum yytokentype type)
{
    const char *exp;
    int ret;
    struct lys_feature *f;
    struct lys_ident *i;
    struct lys_node *n;
    struct lys_type_enum *e;
    struct lys_type_bit *b;
    struct lys_refine *r;

    if ((module->version != 2) && ((value[0] == '(') || strchr(value, ' '))) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
        free(value);
        return EXIT_FAILURE;
    }

    if (!(exp = transform_iffeat_schema2json(module, value))) {
        free(value);
        return EXIT_FAILURE;
    }
    free(value);

    switch (type) {
    case FEATURE_KEYWORD:
        f = (struct lys_feature *) ptr;
        ret = resolve_iffeature_compile(&f->iffeature[f->iffeature_size], exp, (struct lys_node *)f, 1, unres);
        f->iffeature_size++;
        break;
    case IDENTITY_KEYWORD:
        i = (struct lys_ident *) ptr;
        ret = resolve_iffeature_compile(&i->iffeature[i->iffeature_size], exp, (struct lys_node *)i, 0, unres);
        i->iffeature_size++;
        break;
    case ENUM_KEYWORD:
        e = (struct lys_type_enum *) ptr;
        ret = resolve_iffeature_compile(&e->iffeature[e->iffeature_size], exp, (struct lys_node *) parent, 0, unres);
        e->iffeature_size++;
        break;
    case BIT_KEYWORD:
        b = (struct lys_type_bit *) ptr;
        ret = resolve_iffeature_compile(&b->iffeature[b->iffeature_size], exp, (struct lys_node *) parent, 0, unres);
        b->iffeature_size++;
        break;
    case REFINE_KEYWORD:
        r = &((struct lys_node_uses *)ptr)->refine[((struct lys_node_uses *)ptr)->refine_size - 1];
        ret = resolve_iffeature_compile(&r->iffeature[r->iffeature_size], exp, (struct lys_node *) ptr, 0, unres);
        r->iffeature_size++;
        break;
    default:
        n = (struct lys_node *) ptr;
        ret = resolve_iffeature_compile(&n->iffeature[n->iffeature_size], exp, n, 0, unres);
        n->iffeature_size++;
        break;
    }
    lydict_remove(module->ctx, exp);

    if (ret) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int
yang_check_flags(uint16_t *flags, uint16_t mask, char *what, char *where, uint16_t value, int shortint)
{
    if (*flags & mask) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, what, where);
        return EXIT_FAILURE;
    } else {
        if (shortint) {
            *((uint8_t *)flags) |= (uint8_t)value;
        } else {
            *flags |= value;
        }
        return EXIT_SUCCESS;
    }
}

int
yang_read_base(struct lys_module *module, struct lys_ident *ident, char *value, struct unres_schema *unres)
{
    const char *exp;

    exp = transform_schema2json(module, value);
    free(value);
    if (!exp) {
        return EXIT_FAILURE;
    }

    if (unres_schema_add_str(module, unres, ident, UNRES_IDENT, exp) == -1) {
        lydict_remove(module->ctx, exp);
        return EXIT_FAILURE;
    }

    lydict_remove(module->ctx, exp);
    return EXIT_SUCCESS;
}

int
yang_read_message(struct lys_module *module,struct lys_restr *save,char *value, char *what, int message)
{
    int ret;

    if (message == ERROR_APP_TAG_KEYWORD) {
        ret = yang_check_string(module, &save->eapptag, "error_app_tag", what, value);
    } else {
        ret = yang_check_string(module, &save->emsg, "error_message", what, value);
    }
    return ret;
}

int
yang_read_presence(struct lys_module *module, struct lys_node_container *cont, char *value)
{
    if (cont->presence) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, cont, "presence", "container");
        free(value);
        return EXIT_FAILURE;
    } else {
        cont->presence = lydict_insert_zc(module->ctx, value);
        return EXIT_SUCCESS;
    }
}

void *
yang_read_when(struct lys_module *module, struct lys_node *node, enum yytokentype type, char *value)
{
    struct lys_when *retval;

    retval = calloc(1, sizeof *retval);
    if (!retval) {
        LOGMEM;
        free(value);
        return NULL;
    }
    retval->cond = transform_schema2json(module, value);
    if (!retval->cond) {
        goto error;
    }
    switch (type) {
    case CONTAINER_KEYWORD:
        if (((struct lys_node_container *)node)->when) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, node, "when", "container");
            goto error;
        }
        ((struct lys_node_container *)node)->when = retval;
        break;
    case ANYDATA_KEYWORD:
    case ANYXML_KEYWORD:
        if (((struct lys_node_anydata *)node)->when) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, node, "when", (type == ANYXML_KEYWORD) ? "anyxml" : "anydata");
            goto error;
        }
        ((struct lys_node_anydata *)node)->when = retval;
        break;
    case CHOICE_KEYWORD:
        if (((struct lys_node_choice *)node)->when) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, node, "when", "choice");
            goto error;
        }
        ((struct lys_node_choice *)node)->when = retval;
        break;
    case CASE_KEYWORD:
        if (((struct lys_node_case *)node)->when) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, node, "when", "case");
            goto error;
        }
        ((struct lys_node_case *)node)->when = retval;
        break;
    case LEAF_KEYWORD:
        if (((struct lys_node_leaf *)node)->when) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, node, "when", "leaf");
            goto error;
        }
        ((struct lys_node_leaf *)node)->when = retval;
        break;
    case LEAF_LIST_KEYWORD:
        if (((struct lys_node_leaflist *)node)->when) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, node, "when", "leaflist");
            goto error;
        }
        ((struct lys_node_leaflist *)node)->when = retval;
        break;
    case LIST_KEYWORD:
        if (((struct lys_node_list *)node)->when) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, node, "when", "list");
            goto error;
        }
        ((struct lys_node_list *)node)->when = retval;
        break;
    case USES_KEYWORD:
        if (((struct lys_node_uses *)node)->when) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, node, "when", "uses");
            goto error;
        }
        ((struct lys_node_uses *)node)->when = retval;
        break;
    case AUGMENT_KEYWORD:
        if (((struct lys_node_augment *)node)->when) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, node, "when", "augment");
            goto error;
        }
        ((struct lys_node_augment *)node)->when = retval;
        break;
    default:
        goto error;
        break;
    }
    free(value);
    return retval;

error:
    free(value);
    lys_when_free(module->ctx, retval);
    return NULL;
}

void *
yang_read_node(struct lys_module *module, struct lys_node *parent, struct lys_node **root,
               char *value, int nodetype, int sizeof_struct)
{
    struct lys_node *node, **child;

    node = calloc(1, sizeof_struct);
    if (!node) {
        free(value);
        LOGMEM;
        return NULL;
    }
    if (value) {
        node->name = lydict_insert_zc(module->ctx, value);
    }
    node->module = module;
    node->nodetype = nodetype;
    node->parent = parent;

    /* insert the node into the schema tree */
    child = (parent) ? &parent->child : root;
    if (*child) {
        (*child)->prev->next = node;
        (*child)->prev = node;
    } else {
        *child = node;
        node->prev = node;
    }
    return node;
}

void *
yang_read_action(struct lys_module *module, struct lys_node *parent, struct lys_node **root, char *value)
{
    struct lys_node *node;

    if (module->version != 2) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "action");
        return NULL;
    }

    for (node = parent; node; node = lys_parent(node)) {
        if ((node->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF))
                || ((node->nodetype == LYS_LIST) && !((struct lys_node_list *)node)->keys_size)) {
            LOGVAL(LYE_INPAR, LY_VLOG_NONE, NULL, strnodetype(node->nodetype), "action");
            return NULL;
        }
    }
    return yang_read_node(module, parent, root, value, LYS_ACTION, sizeof(struct lys_node_rpc_action));
}

int
yang_read_default(struct lys_module *module, void *node, char *value, enum yytokentype type)
{
    int ret;

    switch (type) {
    case LEAF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_node_leaf *) node)->dflt, "default", "leaf", value);
        break;
    case TYPEDEF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_tpdf *) node)->dflt, "default", "typedef", value);
        break;
    default:
        free(value);
        LOGINT;
        ret = EXIT_FAILURE;
        break;
    }
    return ret;
}

int
yang_read_units(struct lys_module *module, void *node, char *value, enum yytokentype type)
{
    int ret;

    switch (type) {
    case LEAF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_node_leaf *) node)->units, "units", "leaf", value);
        break;
    case LEAF_LIST_KEYWORD:
        ret = yang_check_string(module, &((struct lys_node_leaflist *) node)->units, "units", "leaflist", value);
        break;
    case TYPEDEF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_tpdf *) node)->units, "units", "typedef", value);
        break;
    default:
        free(value);
        LOGINT;
        ret = EXIT_FAILURE;
        break;
    }
    return ret;
}

int
yang_read_key(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres)
{
    char *exp, *value;
    struct lys_node *node;

    exp = value = (char *) list->keys;
    while ((value = strpbrk(value, " \t\n"))) {
        list->keys_size++;
        while (isspace(*value)) {
            value++;
        }
    }
    list->keys_size++;

    list->keys_str = lydict_insert_zc(module->ctx, exp);
    list->keys = calloc(list->keys_size, sizeof *list->keys);
    if (!list->keys) {
        LOGMEM;
        return EXIT_FAILURE;
    }
    for (node = list->parent; node && node->nodetype != LYS_GROUPING; node = lys_parent(node));
    if (!node && unres_schema_add_node(module, unres, list, UNRES_LIST_KEYS, NULL) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int
yang_fill_unique(struct lys_module *module, struct lys_node_list *list, struct lys_unique *unique, char *value, struct unres_schema *unres)
{
    int i, j;
    char *vaux;
    struct unres_list_uniq *unique_info;

    /* count the number of unique leafs in the value */
    vaux = value;
    while ((vaux = strpbrk(vaux, " \t\n"))) {
       unique->expr_size++;
        while (isspace(*vaux)) {
            vaux++;
        }
    }
    unique->expr_size++;
    unique->expr = calloc(unique->expr_size, sizeof *unique->expr);
    if (!unique->expr) {
        LOGMEM;
        goto error;
    }

    for (i = 0; i < unique->expr_size; i++) {
        vaux = strpbrk(value, " \t\n");
        if (!vaux) {
            /* the last token, lydict_insert() will count its size on its own */
            vaux = value;
        }

        /* store token into unique structure */
        unique->expr[i] = lydict_insert(module->ctx, value, vaux - value);

        /* check that the expression does not repeat */
        for (j = 0; j < i; j++) {
            if (ly_strequal(unique->expr[j], unique->expr[i], 1)) {
                LOGVAL(LYE_INARG, LY_VLOG_LYS, list, unique->expr[i], "unique");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The identifier is not unique");
                goto error;
            }
        }
        /* try to resolve leaf */
        if (unres) {
            unique_info = malloc(sizeof *unique_info);
            unique_info->list = (struct lys_node *)list;
            unique_info->expr = unique->expr[i];
            unique_info->trg_type = &unique->trg_type;
            if (unres_schema_add_node(module, unres, unique_info, UNRES_LIST_UNIQ, NULL) == -1) {
                goto error;
            }
        } else {
            if (resolve_unique((struct lys_node *)list, unique->expr[i], &unique->trg_type)) {
                goto error;
            }
        }

        /* move to next token */
        value = vaux;
        while(isspace(*value)) {
            value++;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_unique(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres)
{
    uint8_t k;
    char *str;

    for(k=0; k<list->unique_size; k++) {
        str = (char *)list->unique[k].expr;
        if (yang_fill_unique(module, list, &list->unique[k], str, unres)) {
            goto error;
        }
        free(str);
    }
    return EXIT_SUCCESS;

error:
    free(str);
    return EXIT_FAILURE;
}

int
yang_read_leafref_path(struct lys_module *module, struct yang_type *stype, char *value)
{
    if (stype->base && (stype->base != LY_TYPE_LEAFREF)) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "require-instance");
        goto error;
    }
    if (stype->type->info.lref.path) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "path", "type");
        goto error;
    }
    stype->type->info.lref.path = lydict_insert_zc(module->ctx, value);
    stype->base = LY_TYPE_LEAFREF;
    return EXIT_SUCCESS;

error:
    free(value);
    return EXIT_FAILURE;
}

int
yang_read_require_instance(struct yang_type *stype, int req)
{
    if (stype->base && (stype->base != LY_TYPE_LEAFREF)) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "require-instance");
        return EXIT_FAILURE;
    }
    if (stype->type->info.lref.req) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "require-instance", "type");
        return EXIT_FAILURE;
    }
    stype->type->info.lref.req = req;
    stype->base = LY_TYPE_LEAFREF;
    return EXIT_SUCCESS;
}

int
yang_read_identyref(struct lys_module *module, struct yang_type *stype, char *expr, struct unres_schema *unres)
{
    const char *value;
    int rc;

    if (stype->base && stype->base != LY_TYPE_IDENT) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "base");
        return EXIT_FAILURE;
    }

    stype->base = LY_TYPE_IDENT;
    /* store in the JSON format */
    value = transform_schema2json(module, expr);
    free(expr);

    if (!value) {
        return EXIT_FAILURE;
    }
    rc = unres_schema_add_str(module, unres, stype->type, UNRES_TYPE_IDENTREF, value);
    lydict_remove(module->ctx, value);

    if (rc == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
yang_check_type(struct lys_module *module, struct lys_node *parent, struct yang_type *typ, struct lys_type *type, int tpdftype, struct unres_schema *unres)
{
    int i, j, rc, ret = -1;
    int8_t req;
    const char *name, *value;
    LY_DATA_TYPE base = 0;
    struct lys_node *siter;
    struct lys_type *dertype;
    struct lys_type_enum *enms_sc = NULL;
    struct lys_type_bit *bits_sc = NULL;
    struct lys_type_bit bit_tmp;

    value = transform_schema2json(module, typ->name);
    if (!value) {
        goto error;
    }

    i = parse_identifier(value);
    if (i < 1) {
        LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, value[-i], &value[-i]);
        lydict_remove(module->ctx, value);
        goto error;
    }
    /* module name */
    name = value;
    if (value[i]) {
        type->module_name = lydict_insert(module->ctx, value, i);
        name += i;
        if ((name[0] != ':') || (parse_identifier(name + 1) < 1)) {
            LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, name[0], name);
            lydict_remove(module->ctx, value);
            goto error;
        }
        ++name;
    }

    rc = resolve_superior_type(name, type->module_name, module, parent, &type->der);
    if (rc == -1) {
        LOGVAL(LYE_INMOD, LY_VLOG_NONE, NULL, type->module_name);
        lydict_remove(module->ctx, value);
        goto error;

    /* the type could not be resolved or it was resolved to an unresolved typedef or leafref */
    } else if (rc == EXIT_FAILURE) {
        LOGVAL(LYE_NORESOLV, LY_VLOG_NONE, NULL, "type", name);
        lydict_remove(module->ctx, value);
        ret = EXIT_FAILURE;
        goto error;
    }
    lydict_remove(module->ctx, value);

    if (type->base == LY_TYPE_ERR) {
        /* resolved type in grouping, decrease the grouping's nacm number to indicate that one less
         * unresolved item left inside the grouping, LY_TYPE_ERR used as a flag for types inside a grouping.  */
        for (siter = parent; siter && (siter->nodetype != LYS_GROUPING); siter = lys_parent(siter));
        if (siter) {
            if (!((struct lys_node_grp *)siter)->nacm) {
                LOGINT;
                goto error;
            }
            ((struct lys_node_grp *)siter)->nacm--;
        } else {
            LOGINT;
            goto error;
        }
    }

    /* check status */
    if (lyp_check_status(type->parent->flags, type->parent->module, type->parent->name,
                         type->der->flags, type->der->module, type->der->name, parent)) {
        goto error;
    }

    base = typ->base;
    type->base = type->der->type.base;
    if (base == 0) {
        base = type->der->type.base;
    }
    switch (base) {
    case LY_TYPE_STRING:
        if (type->base == LY_TYPE_BINARY) {
            if (type->info.str.pat_count) {
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Binary type could not include pattern statement.");
                goto error;
            }
            type->info.binary.length = type->info.str.length;
            if (type->info.binary.length && lyp_check_length_range(type->info.binary.length->expr, type)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, type->info.binary.length->expr, "length");
                goto error;
            }
        } else if (type->base == LY_TYPE_STRING) {
            if (type->info.str.length && lyp_check_length_range(type->info.str.length->expr, type)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, type->info.str.length->expr, "length");
                goto error;
            }
        } else {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        break;
    case LY_TYPE_DEC64:
        if (type->base == LY_TYPE_DEC64) {
            /* mandatory sub-statement(s) check */
            if (!type->info.dec64.dig && !type->der->type.der) {
                /* decimal64 type directly derived from built-in type requires fraction-digits */
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "fraction-digits", "type");
                goto error;
            }
            if (type->info.dec64.dig && type->der->type.der) {
                /* type is not directly derived from buit-in type and fraction-digits statement is prohibited */
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "fraction-digits");
                goto error;
            }

            /* copy fraction-digits specification from parent type for easier internal use */
            if (type->der->type.der) {
                type->info.dec64.dig = type->der->type.info.dec64.dig;
                type->info.dec64.div = type->der->type.info.dec64.div;
            }
            if (type->info.dec64.range && lyp_check_length_range(type->info.dec64.range->expr, type)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, type->info.dec64.range->expr, "range");
                goto error;
            }
        } else if (type->base >= LY_TYPE_INT8 && type->base <=LY_TYPE_UINT64) {
            if (type->info.dec64.dig) {
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Numerical type could not include fraction statement.");
                goto error;
            }
            type->info.num.range = type->info.dec64.range;
            if (type->info.num.range && lyp_check_length_range(type->info.num.range->expr, type)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, type->info.num.range->expr, "range");
                goto error;
            }
        } else {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        break;
    case LY_TYPE_ENUM:
        if (type->base != LY_TYPE_ENUM) {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        dertype = &type->der->type;

        if (!dertype->der) {
            if (!type->info.enums.count) {
                /* type is derived directly from buit-in enumeartion type and enum statement is required */
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "enum", "type");
                goto error;
            }
        } else {
            for (; !dertype->info.enums.count; dertype = &dertype->der->type);
            if (module->version < 2 && type->info.enums.count) {
                /* type is not directly derived from built-in enumeration type and enum statement is prohibited
                 * in YANG 1.0, since YANG 1.1 enum statements can be used to restrict the base enumeration type */
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "enum");
                goto error;
            }

            /* restricted enumeration type - the name MUST be used in the base type */
            enms_sc = dertype->info.enums.enm;
            for(i = 0; i < type->info.enums.count; i++) {
                for (j = 0; j < dertype->info.enums.count; j++) {
                    if (ly_strequal(enms_sc[j].name, type->info.enums.enm[i].name, 1)) {
                        break;
                    }
                }
                if (j == dertype->info.enums.count) {
                    LOGVAL(LYE_ENUM_INNAME, LY_VLOG_NONE, NULL, type->info.enums.enm[i].name);
                    goto error;
                }

                if (type->info.enums.enm[i].flags & LYS_AUTOASSIGNED) {
                    /* automatically assign value from base type */
                    type->info.enums.enm[i].value = enms_sc[j].value;
                } else {
                    /* check that the assigned value corresponds to the original
                     * value of the enum in the base type */
                    if (type->info.enums.enm[i].value != enms_sc[j].value) {
                        /* type->info.enums.enm[i].value - assigned value in restricted enum
                         * enms_sc[j].value - value assigned to the corresponding enum (detected above) in base type */
                        LOGVAL(LYE_ENUM_INVAL, LY_VLOG_NONE, NULL, type->info.enums.enm[i].value,
                               type->info.enums.enm[i].name, enms_sc[j].value);
                        goto error;
                    }
                }
            }
        }
        break;
    case LY_TYPE_BITS:
        if (type->base != LY_TYPE_BITS) {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        dertype = &type->der->type;

        if (!dertype->der) {
            if (!type->info.bits.count) {
                /* type is derived directly from buit-in bits type and bit statement is required */
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "bit", "type");
                goto error;
            }
        } else {
            for (; !dertype->info.enums.count; dertype = &dertype->der->type);
            if (module->version < 2 && type->info.bits.count) {
                /* type is not directly derived from buit-in bits type and bit statement is prohibited,
                 * since YANG 1.1 the bit statements can be used to restrict the base bits type */
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "bit");
                goto error;
            }

            bits_sc = dertype->info.bits.bit;
            for (i = 0; i < type->info.bits.count; i++) {
                for (j = 0; j < dertype->info.bits.count; j++) {
                    if (ly_strequal(bits_sc[j].name, type->info.bits.bit[i].name, 1)) {
                        break;
                    }
                }
                if (j == dertype->info.bits.count) {
                    LOGVAL(LYE_BITS_INNAME, LY_VLOG_NONE, NULL, type->info.bits.bit[i].name);
                    goto error;
                }

                /* restricted bits type */
                if (type->info.bits.bit[i].flags & LYS_AUTOASSIGNED) {
                    /* automatically assign position from base type */
                    type->info.bits.bit[i].pos = bits_sc[j].pos;
                } else {
                    /* check that the assigned position corresponds to the original
                     * position of the bit in the base type */
                    if (type->info.bits.bit[i].pos != bits_sc[j].pos) {
                        /* type->info.bits.bit[i].pos - assigned position in restricted bits
                         * bits_sc[j].pos - position assigned to the corresponding bit (detected above) in base type */
                        LOGVAL(LYE_BITS_INVAL, LY_VLOG_NONE, NULL, type->info.bits.bit[i].pos,
                               type->info.bits.bit[i].name, bits_sc[j].pos);
                        goto error;
                    }
                }
            }
        }

        for (i = type->info.bits.count - 1; i > 0; i--) {
            j = i;

            /* keep them ordered by position */
            while (j && type->info.bits.bit[j - 1].pos > type->info.bits.bit[j].pos) {
                /* switch them */
                memcpy(&bit_tmp, &type->info.bits.bit[j], sizeof bit_tmp);
                memcpy(&type->info.bits.bit[j], &type->info.bits.bit[j - 1], sizeof bit_tmp);
                memcpy(&type->info.bits.bit[j - 1], &bit_tmp, sizeof bit_tmp);
                j--;
            }
        }
        break;
    case LY_TYPE_LEAFREF:
        if (type->base == LY_TYPE_INST) {
            if (type->info.lref.path) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "path");
                goto error;
            }
            if ((req = type->info.lref.req)) {
                type->info.inst.req = req;
            }
        } else if (type->base == LY_TYPE_LEAFREF) {
            /* require-instance only YANG 1.1 */
            if (type->info.lref.req && (module->version < 2)) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "require-instance");
                goto error;
            }
            /* flag resolving for later use */
            if (!tpdftype) {
                for (siter = parent; siter && siter->nodetype != LYS_GROUPING; siter = lys_parent(siter));
                if (siter) {
                    /* just a flag - do not resolve */
                    tpdftype = 1;
                }
            }

            if (type->info.lref.path) {
                if (type->der->type.der) {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "path");
                    goto error;
                }
                value = type->info.lref.path;
                /* store in the JSON format */
                type->info.lref.path = transform_schema2json(module, value);
                lydict_remove(module->ctx, value);
                if (!type->info.lref.path) {
                    goto error;
                }
                /* try to resolve leafref path only when this is instantiated
                 * leaf, so it is not:
                 * - typedef's type,
                 * - in  grouping definition,
                 * - just instantiated in a grouping definition,
                 * because in those cases the nodes referenced in path might not be present
                 * and it is not a bug.  */
                if (!tpdftype && unres_schema_add_node(module, unres, type, UNRES_TYPE_LEAFREF, parent) == -1) {
                    goto error;
                }
            } else if (!type->der->type.der) {
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "path", "type");
                goto error;
            } else {
                /* copy leafref definition into the derived type */
                type->info.lref.path = lydict_insert(module->ctx, type->der->type.info.lref.path, 0);
                /* and resolve the path at the place we are (if not in grouping/typedef) */
                if (!tpdftype && unres_schema_add_node(module, unres, type, UNRES_TYPE_LEAFREF, parent) == -1) {
                    goto error;
                }

                /* add pointer to leafref target, only on leaves (not in typedefs) */
                if (type->info.lref.target && lys_leaf_add_leafref_target(type->info.lref.target, (struct lys_node *)type->parent)) {
                    goto error;
                }
            }
        } else {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        break;
    case LY_TYPE_IDENT:
        if (type->base != LY_TYPE_IDENT) {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        if (type->der->type.der) {
            if (type->info.ident.ref) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "base");
                goto error;
            }
        } else {
            if (!type->info.ident.ref) {
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "base", "type");
                goto error;
            }
        }
        break;
    case LY_TYPE_UNION:
        if (type->base != LY_TYPE_UNION) {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        if (!type->info.uni.types) {
            if (type->der->type.der) {
                /* this is just a derived type with no additional type specified/required */
                break;
            }
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "(union) type");
            goto error;
        }
        for (i = 0; i < type->info.uni.count; i++) {
            type->info.uni.types[i].parent = type->parent;
            if (unres_schema_add_node(module, unres, &type->info.uni.types[i],
                                      tpdftype ? UNRES_TYPE_DER_TPDF : UNRES_TYPE_DER, parent) == -1) {
                goto error;
            }
            if (module->version < 2) {
                if (type->info.uni.types[i].base == LY_TYPE_EMPTY) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "empty", typ->name);
                    goto error;
                } else if (type->info.uni.types[i].base == LY_TYPE_LEAFREF) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "leafref", typ->name);
                    goto error;
                }
            }
        }
        break;

    default:
        if (base >= LY_TYPE_BINARY && base <= LY_TYPE_UINT64) {
            if (type->base != base) {
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
                goto error;
            }
        } else {
            LOGINT;
            goto error;
        }
    }
    return EXIT_SUCCESS;

error:
    if (type->module_name) {
        lydict_remove(module->ctx, type->module_name);
        type->module_name = NULL;
    }
    if (base) {
        type->base = base;
    }
    return ret;
}

void *
yang_read_type(struct lys_module *module, void *parent, char *value, enum yytokentype type)
{
    struct yang_type *typ;
    struct type_deviation *dev;
    struct lys_tpdf *tmp_parent;

    typ = calloc(1, sizeof *typ);
    if (!typ) {
        LOGMEM;
        return NULL;
    }

    typ->flags = LY_YANG_STRUCTURE_FLAG;
    switch (type) {
    case LEAF_KEYWORD:
        if (((struct lys_node_leaf *)parent)->type.der) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, parent, "type", "leaf");
            goto error;
        }
        ((struct lys_node_leaf *)parent)->type.der = (struct lys_tpdf *)typ;
        ((struct lys_node_leaf *)parent)->type.parent = (struct lys_tpdf *)parent;
        typ->type = &((struct lys_node_leaf *)parent)->type;
        break;
    case LEAF_LIST_KEYWORD:
        if (((struct lys_node_leaflist *)parent)->type.der) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, parent, "type", "leaf-list");
            goto error;
        }
        ((struct lys_node_leaflist *)parent)->type.der = (struct lys_tpdf *)typ;
        ((struct lys_node_leaflist *)parent)->type.parent = (struct lys_tpdf *)parent;
        typ->type = &((struct lys_node_leaflist *)parent)->type;
        break;
    case UNION_KEYWORD:
        ((struct lys_type *)parent)->der = (struct lys_tpdf *)typ;
        typ->type = (struct lys_type *)parent;
        break;
    case TYPEDEF_KEYWORD:
        if (((struct lys_tpdf *)parent)->type.der) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "type", "typedef");
            goto error;
        }
        ((struct lys_tpdf *)parent)->type.der = (struct lys_tpdf *)typ;
        typ->type = &((struct lys_tpdf *)parent)->type;
        break;
    case REPLACE_KEYWORD:
        /* deviation replace type*/
        dev = (struct type_deviation *)parent;
        if (dev->deviate->type) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "type", "deviation");
            goto error;
        }
        /* check target node type */
        if (dev->target->nodetype == LYS_LEAF) {
            typ->type = &((struct lys_node_leaf *)dev->target)->type;
        } else if (dev->target->nodetype == LYS_LEAFLIST) {
            typ->type = &((struct lys_node_leaflist *)dev->target)->type;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "type");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"type\" property.");
            goto error;
        }

        /* remove type and initialize it */
        lys_type_free(module->ctx, typ->type);
        tmp_parent = typ->type->parent;
        memset(typ->type, 0, sizeof *typ->type);
        typ->type->parent = tmp_parent;

        /* replace it with the value specified in deviation */
        /* HACK for unres */
        typ->type->der = (struct lys_tpdf *)typ;
        dev->deviate->type = typ->type;
        break;
    default:
        goto error;
        break;
    }
    typ->name = lydict_insert_zc(module->ctx, value);
    return typ;

error:
    free(value);
    free(typ);
    return NULL;
}

void
yang_delete_type(struct lys_module *module, struct yang_type *stype)
{
    int i;

    if (!stype) {
        return;
    }
    stype->type->base = stype->base;
    stype->type->der = NULL;
    lydict_remove(module->ctx, stype->name);
    if (stype->base == LY_TYPE_UNION) {
        for (i = 0; i < stype->type->info.uni.count; i++) {
            if (stype->type->info.uni.types[i].der) {
                yang_delete_type(module, (struct yang_type *)stype->type->info.uni.types[i].der);
            }
        }
    }
    free(stype);
}

void *
yang_read_length(struct lys_module *module, struct yang_type *typ, char *value)
{
    struct lys_restr **length;

    if (typ->base == 0 || typ->base == LY_TYPE_STRING) {
        length = &typ->type->info.str.length;
        typ->base = LY_TYPE_STRING;
    } else if (typ->base == LY_TYPE_BINARY) {
        length = &typ->type->info.binary.length;
    } else {
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected length statement.");
        goto error;
    }

    if (*length) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "length", "type");
        goto error;
    }
    *length = calloc(1, sizeof **length);
    if (!*length) {
        LOGMEM;
        goto error;
    }
    (*length)->expr = lydict_insert_zc(module->ctx, value);
    return *length;

error:
    free(value);
    return NULL;

}

int
yang_read_pattern(struct lys_module *module, struct lys_restr *pattern, char *value, char modifier)
{
    char *buf;
    size_t len;

    if (lyp_check_pattern(value, NULL)) {
        free(value);
        return EXIT_FAILURE;
    }

    len = strlen(value);
    buf = malloc((len + 2) * sizeof *buf); /* modifier byte + value + terminating NULL byte */

    if (!buf) {
        LOGMEM;
        free(value);
        return EXIT_FAILURE;
    }

    buf[0] = modifier;
    strcpy(&buf[1], value);
    free(value);

    pattern->expr = lydict_insert_zc(module->ctx, buf);
    return EXIT_SUCCESS;
}

void *
yang_read_range(struct  lys_module *module, struct yang_type *typ, char *value)
{
    if (typ->base != 0 && typ->base != LY_TYPE_DEC64) {
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected range statement.");
        goto error;
    }
    typ->base = LY_TYPE_DEC64;
    if (typ->type->info.dec64.range) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "range", "type");
        goto error;
    }
    typ->type->info.dec64.range = calloc(1, sizeof *typ->type->info.dec64.range);
    if (!typ->type->info.dec64.range) {
        LOGMEM;
        goto error;
    }
    typ->type->info.dec64.range->expr = lydict_insert_zc(module->ctx, value);
    return typ->type->info.dec64.range;

error:
    free(value);
    return NULL;
}

int
yang_read_fraction(struct yang_type *typ, uint32_t value)
{
    unsigned int i;

    if (typ->base == 0 || typ->base == LY_TYPE_DEC64) {
        typ->base = LY_TYPE_DEC64;
    } else {
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected fraction-digits statement.");
        goto error;
    }
    if (typ->type->info.dec64.dig) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "fraction-digits", "type");
        goto error;
    }
    /* range check */
    if (value < 1 || value > 18) {
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", value, "fraction-digits");
        goto error;
    }
    typ->type->info.dec64.dig = value;
    typ->type->info.dec64.div = 10;
    for (i = 1; i < value; i++) {
        typ->type->info.dec64.div *= 10;
    }
    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_enum(struct lys_module *module, struct yang_type *typ, struct lys_type_enum *enm, char *value)
{
    int i, j;

    typ->base = LY_TYPE_ENUM;
    if (!value[0]) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "enum name");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Enum name must not be empty.");
        free(value);
        goto error;
    }

    enm->name = lydict_insert_zc(module->ctx, value);

    /* the assigned name MUST NOT have any leading or trailing whitespace characters */
    if (isspace(enm->name[0]) || isspace(enm->name[strlen(enm->name) - 1])) {
        LOGVAL(LYE_ENUM_WS, LY_VLOG_NONE, NULL, enm->name);
        goto error;
    }

    j = typ->type->info.enums.count - 1;
    /* check the name uniqueness */
    for (i = 0; i < j; i++) {
        if (!strcmp(typ->type->info.enums.enm[i].name, typ->type->info.enums.enm[j].name)) {
            LOGVAL(LYE_ENUM_DUPNAME, LY_VLOG_NONE, NULL, typ->type->info.enums.enm[i].name);
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_check_enum(struct yang_type *typ, struct lys_type_enum *enm, int64_t *value, int assign)
{
    int i, j;

    if (!assign) {
        /* assign value automatically */
        if (*value > INT32_MAX) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "2147483648", "enum/value");
            goto error;
        }
        enm->value = *value;
        enm->flags |= LYS_AUTOASSIGNED;
        (*value)++;
    } else if (typ->type->info.enums.enm == enm) {
        /* change value, which is assigned automatically, if first enum has value. */
        *value = typ->type->info.enums.enm[0].value;
        (*value)++;
    }

    /* check that the value is unique */
    j = typ->type->info.enums.count-1;
    for (i = 0; i < j; i++) {
        if (typ->type->info.enums.enm[i].value == typ->type->info.enums.enm[j].value) {
            LOGVAL(LYE_ENUM_DUPVAL, LY_VLOG_NONE, NULL,
                   typ->type->info.enums.enm[j].value, typ->type->info.enums.enm[j].name,
                   typ->type->info.enums.enm[i].name);
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_bit(struct lys_module *module, struct yang_type *typ, struct lys_type_bit *bit, char *value)
{
    int i, j;

    bit->name = lydict_insert_zc(module->ctx, value);
    if (lyp_check_identifier(bit->name, LY_IDENT_SIMPLE, NULL, NULL)) {
        free(value);
        goto error;
    }

    j = typ->type->info.bits.count - 1;
    /* check the name uniqueness */
    for (i = 0; i < j; i++) {
        if (!strcmp(typ->type->info.bits.bit[i].name, bit->name)) {
            LOGVAL(LYE_BITS_DUPNAME, LY_VLOG_NONE, NULL, bit->name);
            goto error;
        }
    }
    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_check_bit(struct yang_type *typ, struct lys_type_bit *bit, int64_t *value, int assign)
{
    int i,j;

    if (!assign) {
        /* assign value automatically */
        if (*value > UINT32_MAX) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "4294967295", "bit/position");
            goto error;
        }
        bit->pos = (uint32_t)*value;
        bit->flags |= LYS_AUTOASSIGNED;
        (*value)++;
    }

    j = typ->type->info.bits.count - 1;
    /* check that the value is unique */
    for (i = 0; i < j; i++) {
        if (typ->type->info.bits.bit[i].pos == bit->pos) {
            LOGVAL(LYE_BITS_DUPVAL, LY_VLOG_NONE, NULL, bit->pos, bit->name, typ->type->info.bits.bit[i].name);
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

void *
yang_read_refine(struct lys_module *module, struct lys_node_uses *uses, char *value)
{
    struct lys_refine *rfn;

    rfn = &uses->refine[uses->refine_size];
    uses->refine_size++;
    rfn->target_name = transform_schema2json(module, value);
    free(value);
    if (!rfn->target_name) {
        return NULL;
    }
    return rfn;
}

void *
yang_read_augment(struct lys_module *module, struct lys_node *parent, char *value)
{
    struct lys_node_augment *aug;

    if (parent) {
        aug = &((struct lys_node_uses *)parent)->augment[((struct lys_node_uses *)parent)->augment_size];
    } else {
        aug = &module->augment[module->augment_size];
    }
    aug->nodetype = LYS_AUGMENT;
    aug->target_name = transform_schema2json(module, value);
    free(value);
    if (!aug->target_name) {
        return NULL;
    }
    aug->parent = parent;
    aug->module = module;
    if (parent) {
        ((struct lys_node_uses *)parent)->augment_size++;
    } else {
        module->augment_size++;
    }
    return aug;
}

void *
yang_read_deviation(struct lys_module *module, char *value)
{
    struct lys_node *dev_target = NULL;
    struct lys_deviation *dev;
    struct type_deviation *deviation = NULL;
    int rc;

    dev = &module->deviation[module->deviation_size];
    dev->target_name = transform_schema2json(module, value);
    free(value);
    if (!dev->target_name) {
        goto error;
    }

    deviation = calloc(1, sizeof *deviation);
    if (!deviation) {
        LOGMEM;
        goto error;
    }

    /* resolve target node */
    rc = resolve_augment_schema_nodeid(dev->target_name, NULL, module, 1, (const struct lys_node **)&dev_target);
    if (rc || !dev_target) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, dev->target_name, "deviation");
        goto error;
    }
    if (dev_target->module == lys_main_module(module)) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, dev->target_name, "deviation");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Deviating own module is not allowed.");
        goto error;
    }

    /*save pointer to the deviation and deviated target*/
    deviation->deviation = dev;
    deviation->target = dev_target;

    deviation->dflt_check = ly_set_new();
    if (!deviation->dflt_check) {
        LOGMEM;
        goto error;
    }

    return deviation;

error:
    free(deviation);
    lydict_remove(module->ctx, dev->target_name);
    return NULL;
}

int
yang_read_deviate_unsupported(struct type_deviation *dev)
{
    int i;

    if (dev->deviation->deviate_size) {
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"not-supported\" deviation cannot be combined with any other deviation.");
        return EXIT_FAILURE;
    }
    dev->deviation->deviate[dev->deviation->deviate_size].mod = LY_DEVIATE_NO;

    /* you cannot remove a key leaf */
    if ((dev->target->nodetype == LYS_LEAF) && dev->target->parent && (dev->target->parent->nodetype == LYS_LIST)) {
        for (i = 0; i < ((struct lys_node_list *)dev->target->parent)->keys_size; ++i) {
            if (((struct lys_node_list *)dev->target->parent)->keys[i] == (struct lys_node_leaf *)dev->target) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "not-supported", "deviation");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"not-supported\" deviation cannot remove a list key.");
                return EXIT_FAILURE;
            }
        }
    }

    /* unlink and store the original node */
    lys_node_unlink(dev->target);
    dev->deviation->orig_node = dev->target;

    dev->deviation->deviate_size = 1;
    return EXIT_SUCCESS;
}

int
yang_read_deviate(struct type_deviation *dev, LYS_DEVIATE_TYPE mod)
{
    struct unres_schema tmp_unres;

    dev->deviation->deviate[dev->deviation->deviate_size].mod = mod;
    dev->deviate = &dev->deviation->deviate[dev->deviation->deviate_size];
    dev->deviation->deviate_size++;
    dev->trg_must_size = NULL;
    if (dev->deviation->deviate[0].mod == LY_DEVIATE_NO) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "not-supported");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"not-supported\" deviation cannot be combined with any other deviation.");
        return EXIT_FAILURE;
    }

    /* store a shallow copy of the original node */
    if (!dev->deviation->orig_node) {
        memset(&tmp_unres, 0, sizeof tmp_unres);
        dev->deviation->orig_node = lys_node_dup(dev->target->module, NULL, dev->target, 0, &tmp_unres, 1);
        /* just to be safe */
        if (tmp_unres.count) {
            LOGINT;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int
yang_read_deviate_units(struct ly_ctx *ctx, struct type_deviation *dev, char *value)
{
    const char **stritem;

    if (dev->deviate->units) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "units", "deviate");
        free(value);
        goto error;
    }

    /* check target node type */
    if (dev->target->nodetype == LYS_LEAFLIST) {
        stritem = &((struct lys_node_leaflist *)dev->target)->units;
    } else if (dev->target->nodetype == LYS_LEAF) {
        stritem = &((struct lys_node_leaf *)dev->target)->units;
    } else {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "units");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"units\" property.");
        free(value);
        goto error;
    }

    dev->deviate->units = lydict_insert_zc(ctx, value);

    if (dev->deviate->mod == LY_DEVIATE_DEL) {
        /* check values */
        if (!ly_strequal(*stritem, dev->deviate->units, 1)) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, dev->deviate->units, "units");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
            goto error;
        }
        /* remove current units value of the target */
        lydict_remove(ctx, *stritem);
    } else {
        if (dev->deviate->mod == LY_DEVIATE_ADD) {
            /* check that there is no current value */
            if (*stritem) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "units");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
                goto error;
            }
        } else { /* replace */
            if (!*stritem) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "units");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
                goto error;
            }
        }
        /* remove current units value of the target ... */
        lydict_remove(ctx, *stritem);

        /* ... and replace it with the value specified in deviation */
        *stritem = lydict_insert(ctx, dev->deviate->units, 0);
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_must(struct type_deviation *dev, uint8_t c_must)
{
    /* check target node type */
    switch (dev->target->nodetype) {
    case LYS_LEAF:
        dev->trg_must = &((struct lys_node_leaf *)dev->target)->must;
        dev->trg_must_size = &((struct lys_node_leaf *)dev->target)->must_size;
        break;
    case LYS_CONTAINER:
        dev->trg_must = &((struct lys_node_container *)dev->target)->must;
        dev->trg_must_size = &((struct lys_node_container *)dev->target)->must_size;
        break;
    case LYS_LEAFLIST:
        dev->trg_must = &((struct lys_node_leaflist *)dev->target)->must;
        dev->trg_must_size = &((struct lys_node_leaflist *)dev->target)->must_size;
        break;
    case LYS_LIST:
        dev->trg_must = &((struct lys_node_list *)dev->target)->must;
        dev->trg_must_size = &((struct lys_node_list *)dev->target)->must_size;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        dev->trg_must = &((struct lys_node_anydata *)dev->target)->must;
        dev->trg_must_size = &((struct lys_node_anydata *)dev->target)->must_size;
        break;
    default:
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "must");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"must\" property.");
        goto error;
    }

    /* flag will be checked again, clear it for now */
    dev->target->flags &= ~LYS_VALID_DEP;

    if (dev->deviate->mod == LY_DEVIATE_ADD) {
        /* reallocate the must array of the target */
        dev->deviate->must = ly_realloc(*dev->trg_must, (c_must + *dev->trg_must_size) * sizeof *dev->deviate->must);
        if (!dev->deviate->must) {
            LOGMEM;
            goto error;
        }
        *dev->trg_must = dev->deviate->must;
        dev->deviate->must = &((*dev->trg_must)[*dev->trg_must_size]);
        dev->deviate->must_size = c_must;
    } else {
        /* LY_DEVIATE_DEL */
        dev->deviate->must = calloc(c_must, sizeof *dev->deviate->must);
        if (!dev->deviate->must) {
            LOGMEM;
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_unique(struct type_deviation *dev, uint8_t c_uniq)
{
    struct lys_node_list *list;

    /* check target node type */
    if (dev->target->nodetype != LYS_LIST) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "unique");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"unique\" property.");
        goto error;
    }

    list = (struct lys_node_list *)dev->target;
    if (dev->deviate->mod == LY_DEVIATE_ADD) {
        /* reallocate the unique array of the target */
        dev->deviate->unique = ly_realloc(list->unique, (c_uniq + list->unique_size) * sizeof *dev->deviate->unique);
        if (!dev->deviate->unique) {
            LOGMEM;
            goto error;
        }
        list->unique = dev->deviate->unique;
        dev->deviate->unique = &list->unique[list->unique_size];
        dev->deviate->unique_size = c_uniq;
        memset(dev->deviate->unique, 0, c_uniq * sizeof *dev->deviate->unique);
    } else {
        /* LY_DEVIATE_DEL */
        dev->deviate->unique = calloc(c_uniq, sizeof *dev->deviate->unique);
        if (!dev->deviate->unique) {
            LOGMEM;
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_default(struct lys_module *module, struct type_deviation *dev, uint8_t c_dflt)
{
    int i;
    struct lys_node_leaflist *llist;

    /* check target node type */
    if (module->version < 2 && dev->target->nodetype == LYS_LEAFLIST) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"default\" property.");
        goto error;
    } else if (c_dflt > 1 && dev->target->nodetype != LYS_LEAFLIST) { /* from YANG 1.1 */
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow multiple \"default\" properties.");
        goto error;
    } else if (!(dev->target->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE))) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"default\" property.");
        goto error;
    }

    if (dev->deviate->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if ((dev->target->nodetype == LYS_LEAF && ((struct lys_node_leaf *)dev->target)->dflt) ||
                (dev->target->nodetype == LYS_CHOICE && ((struct lys_node_choice *)dev->target)->dflt)) {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
            goto error;
        }

        /* check collision with mandatory/min-elements */
        if ((dev->target->flags & LYS_MAND_TRUE) ||
                (dev->target->nodetype == LYS_LEAFLIST && ((struct lys_node_leaflist *)dev->target)->min)) {
            LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "default", "deviation");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                   "Adding the \"default\" statement is forbidden on %s statement.",
                   (dev->target->flags & LYS_MAND_TRUE) ? "nodes with the \"mandatory\"" : "leaflists with non-zero \"min-elements\"");
            goto error;
        }
    } else if (dev->deviate->mod == LY_DEVIATE_RPL) {
        /* check that there was a value before */
        if (((dev->target->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && !((struct lys_node_leaf *)dev->target)->dflt) ||
                (dev->target->nodetype == LYS_CHOICE && !((struct lys_node_choice *)dev->target)->dflt)) {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
            goto error;
        }
    }

    if (dev->target->nodetype == LYS_LEAFLIST) {
        /* reallocate default list in the target */
        llist = (struct lys_node_leaflist *)dev->target;
        if (dev->deviate->mod == LY_DEVIATE_ADD) {
            /* reallocate (enlarge) the unique array of the target */
            llist->dflt = ly_realloc(llist->dflt, (c_dflt + llist->dflt_size) * sizeof *dev->deviate->dflt);
        } else if (dev->deviate->mod == LY_DEVIATE_RPL) {
            /* reallocate (replace) the unique array of the target */
            for (i = 0; i < llist->dflt_size; i++) {
                lydict_remove(llist->module->ctx, llist->dflt[i]);
            }
            llist->dflt = ly_realloc(llist->dflt, c_dflt * sizeof *dev->deviate->dflt);
            llist->dflt_size = 0;
        }
    }

    dev->deviate->dflt = calloc(c_dflt, sizeof *dev->deviate->dflt);
    if (!dev->deviate->dflt) {
        LOGMEM;
        goto error;
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_fill_deviate_default(struct ly_ctx *ctx, struct type_deviation *dev, char *exp)
{
    struct lys_node *node;
    struct lys_node_choice *choice;
    struct lys_node_leaf *leaf;
    struct lys_node_leaflist *llist;
    int rc, i;
    unsigned int u;
    const char *value;

    value = lydict_insert_zc(ctx, exp);
    u = strlen(value);
    dev->deviate->dflt[dev->deviate->dflt_size++] = value;

    if (dev->target->nodetype == LYS_CHOICE) {
        choice = (struct lys_node_choice *)dev->target;
        rc = resolve_choice_default_schema_nodeid(value, choice->child, (const struct lys_node **)&node);
        if (rc || !node) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
            goto error;
        }
        if (dev->deviate->mod == LY_DEVIATE_DEL) {
            if (!choice->dflt || (choice->dflt != node)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                goto error;
            }
        } else { /* add or replace */
            choice->dflt = node;
            if (!choice->dflt) {
                /* default branch not found */
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                goto error;
            }
        }
    } else if (dev->target->nodetype == LYS_LEAF) {
        leaf = (struct lys_node_leaf *)dev->target;
        if (dev->deviate->mod == LY_DEVIATE_DEL) {
            if (!leaf->dflt || !ly_strequal(leaf->dflt, value, 1)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                goto error;
            }
            /* remove value */
            lydict_remove(ctx, leaf->dflt);
            leaf->dflt = NULL;
            leaf->flags &= ~LYS_DFLTJSON;
        } else { /* add (already checked) and replace */
            /* remove value */
            lydict_remove(ctx, leaf->dflt);
            leaf->flags &= ~LYS_DFLTJSON;

            /* set new value */
            leaf->dflt = lydict_insert(ctx, value, u);

            /* remember to check it later (it may not fit now, but the type can be deviated too) */
            ly_set_add(dev->dflt_check, dev->target, 0);
        }
    } else { /* LYS_LEAFLIST */
        llist = (struct lys_node_leaflist *)dev->target;
        if (dev->deviate->mod == LY_DEVIATE_DEL) {
            /* find and remove the value in target list */
            for (i = 0; i < llist->dflt_size; i++) {
                if (llist->dflt[i] && ly_strequal(llist->dflt[i], value, 1)) {
                    /* match, remove the value */
                    lydict_remove(llist->module->ctx, llist->dflt[i]);
                    llist->dflt[i] = NULL;
                    break;
                }
            }
            if (i == llist->dflt_size) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The default value to delete not found in the target node.");
                goto error;
            }
        } else {
            /* add or replace, anyway we place items into the deviate's list
               which propagates to the target */
            /* we just want to check that the value isn't already in the list */
            for (i = 0; i < llist->dflt_size; i++) {
                if (ly_strequal(llist->dflt[i], value, 1)) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Duplicated default value \"%s\".", value);
                    goto error;
                }
            }
            /* store it in target node */
            llist->dflt[llist->dflt_size++] = lydict_insert(ctx, value, u);

            /* remember to check it later (it may not fit now, but the type can be deviated too) */
            ly_set_add(dev->dflt_check, dev->target, 0);
            llist->flags &= ~LYS_DFLTJSON;
        }
    }

    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}


int
yang_read_deviate_config(struct type_deviation *dev, uint8_t value)
{
    if (dev->deviate->flags & LYS_CONFIG_MASK) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
        goto error;
    }

    /* for we deviate from RFC 6020 and allow config property even it is/is not
     * specified in the target explicitly since config property inherits. So we expect
     * that config is specified in every node. But for delete, we check that the value
     * is the same as here in deviation
     */
    dev->deviate->flags |= value;

    /* add and replace are the same in this case */
    /* remove current config value of the target ... */
    dev->target->flags &= ~LYS_CONFIG_MASK;

    /* ... and replace it with the value specified in deviation */
    dev->target->flags |= dev->deviate->flags & LYS_CONFIG_MASK;

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_mandatory(struct type_deviation *dev, uint8_t value)
{
    struct lys_node *parent;

    if (dev->deviate->flags & LYS_MAND_MASK) {
        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
        goto error;
    }

    /* check target node type */
    if (!(dev->target->nodetype & (LYS_LEAF | LYS_CHOICE | LYS_ANYDATA))) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "mandatory");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"mandatory\" property.");
        goto error;
    }

    dev->deviate->flags |= value;

    if (dev->deviate->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if (dev->target->flags & LYS_MAND_MASK) {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "mandatory");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
            goto error;
        } else {
            if (dev->target->nodetype == LYS_LEAF && ((struct lys_node_leaf *)dev->target)->dflt) {
                /* RFC 6020, 7.6.4 - default statement must not with mandatory true */
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "leaf");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                goto error;
            } else if (dev->target->nodetype == LYS_CHOICE && ((struct lys_node_choice *)dev->target)->dflt) {
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "choice");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on choices with \"default\".");
                goto error;
            }
        }
    } else { /* replace */
        if (!(dev->target->flags & LYS_MAND_MASK)) {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "mandatory");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
            goto error;
        }
    }

    /* remove current mandatory value of the target ... */
    dev->target->flags &= ~LYS_MAND_MASK;

    /* ... and replace it with the value specified in deviation */
    dev->target->flags |= dev->deviate->flags & LYS_MAND_MASK;

    /* check for mandatory node in default case, first find the closest parent choice to the changed node */
    for (parent = dev->target->parent;
         parent && !(parent->nodetype & (LYS_CHOICE | LYS_GROUPING | LYS_ACTION));
         parent = parent->parent) {
        if (parent->nodetype == LYS_CONTAINER && ((struct lys_node_container *)parent)->presence) {
            /* stop also on presence containers */
            break;
        }
    }
    /* and if it is a choice with the default case, check it for presence of a mandatory node in it */
    if (parent && parent->nodetype == LYS_CHOICE && ((struct lys_node_choice *)parent)->dflt) {
        if (lyp_check_mandatory_choice(parent)) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_minmax(struct type_deviation *dev, uint32_t value, int type)
{
    uint32_t *ui32val, *min, *max;

    /* check target node type */
    if (dev->target->nodetype == LYS_LEAFLIST) {
        max = &((struct lys_node_leaflist *)dev->target)->max;
        min = &((struct lys_node_leaflist *)dev->target)->min;
    } else if (dev->target->nodetype == LYS_LIST) {
        max = &((struct lys_node_list *)dev->target)->max;
        min = &((struct lys_node_list *)dev->target)->min;
    } else {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, (type) ? "max-elements" : "min-elements");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", (type) ? "max-elements" : "min-elements");
        goto error;
    }

    if (type) {
        dev->deviate->max = value;
        dev->deviate->max_set = 1;
        ui32val = max;
    } else {
        dev->deviate->min = value;
        dev->deviate->min_set = 1;
        ui32val = min;
    }

    if (dev->deviate->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if (*ui32val) {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, (type) ? "max-elements" : "min-elements");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
            goto error;
        }
    } else if (dev->deviate->mod == LY_DEVIATE_RPL) {
        /* unfortunately, there is no way to check reliably that there
         * was a value before, it could have been the default */
    }

    /* add (already checked) and replace */
    /* set new value specified in deviation */
    *ui32val = value;

    /* check min-elements is smaller than max-elements */
    if (*max && *min > *max) {
        if (type) {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"max-elements\".", value);
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"max-elements\" is smaller than \"min-elements\".");
        } else {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"min-elements\".", value);
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
        }
        goto error;
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_check_deviate_must(struct ly_ctx *ctx, struct type_deviation *dev)
{
    int i;

    /* find must to delete, we are ok with just matching conditions */
    for (i = 0; i < *dev->trg_must_size; i++) {
        if (ly_strequal(dev->deviate->must[dev->deviate->must_size - 1].expr, (*dev->trg_must)[i].expr, 1)) {
            /* we have a match, free the must structure ... */
            lys_restr_free(ctx, &((*dev->trg_must)[i]));
            /* ... and maintain the array */
            (*dev->trg_must_size)--;
            if (i != *dev->trg_must_size) {
                (*dev->trg_must)[i].expr = (*dev->trg_must)[*dev->trg_must_size].expr;
                (*dev->trg_must)[i].dsc = (*dev->trg_must)[*dev->trg_must_size].dsc;
                (*dev->trg_must)[i].ref = (*dev->trg_must)[*dev->trg_must_size].ref;
                (*dev->trg_must)[i].eapptag = (*dev->trg_must)[*dev->trg_must_size].eapptag;
                (*dev->trg_must)[i].emsg = (*dev->trg_must)[*dev->trg_must_size].emsg;
            }
            if (!(*dev->trg_must_size)) {
                free(*dev->trg_must);
                *dev->trg_must = NULL;
            } else {
                (*dev->trg_must)[*dev->trg_must_size].expr = NULL;
                (*dev->trg_must)[*dev->trg_must_size].dsc = NULL;
                (*dev->trg_must)[*dev->trg_must_size].ref = NULL;
                (*dev->trg_must)[*dev->trg_must_size].eapptag = NULL;
                (*dev->trg_must)[*dev->trg_must_size].emsg = NULL;
            }

            i = -1; /* set match flag */
            break;
        }
    }
    if (i != -1) {
        /* no match found */
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, dev->deviate->must[dev->deviate->must_size - 1].expr, "must");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value does not match any must from the target.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
yang_check_deviate_unique(struct lys_module *module, struct type_deviation *dev, char *value)
{
    struct lys_node_list *list;
    int i, j;

    list = (struct lys_node_list *)dev->target;
    if (yang_fill_unique(module, list, &dev->deviate->unique[dev->deviate->unique_size], value, NULL)) {
        dev->deviate->unique_size++;
        goto error;
    }

    /* find unique structures to delete */
    for (i = 0; i < list->unique_size; i++) {
        if (list->unique[i].expr_size != dev->deviate->unique[dev->deviate->unique_size].expr_size) {
            continue;
        }

        for (j = 0; j < dev->deviate->unique[dev->deviate->unique_size].expr_size; j++) {
            if (!ly_strequal(list->unique[i].expr[j], dev->deviate->unique[dev->deviate->unique_size].expr[j], 1)) {
                break;
            }
        }

        if (j == dev->deviate->unique[dev->deviate->unique_size].expr_size) {
            /* we have a match, free the unique structure ... */
            for (j = 0; j < list->unique[i].expr_size; j++) {
                lydict_remove(module->ctx, list->unique[i].expr[j]);
            }
            free(list->unique[i].expr);
            /* ... and maintain the array */
            list->unique_size--;
            if (i != list->unique_size) {
                list->unique[i].expr_size = list->unique[list->unique_size].expr_size;
                list->unique[i].expr = list->unique[list->unique_size].expr;
            }

            if (!list->unique_size) {
                free(list->unique);
                list->unique = NULL;
            } else {
                list->unique[list->unique_size].expr_size = 0;
                list->unique[list->unique_size].expr = NULL;
            }

            i = -1; /* set match flag */
            break;
        }
    }
    dev->deviate->unique_size++;

    if (i != -1) {
        /* no match found */
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "unique");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
        goto error;
    }

    free(value);
    return EXIT_SUCCESS;

error:
    free(value);
    return EXIT_FAILURE;
}

int
yang_check_deviation(struct lys_module *module, struct ly_set *dflt_check, struct unres_schema *unres)
{
    int i, rc;
    unsigned int u;
    const char *value, *target_name;
    struct lys_node_leaflist *llist;
    struct lys_node_leaf *leaf;

    /* now check whether default value, if any, matches the type */
    for (u = 0; u < dflt_check->number; ++u) {
        value = NULL;
        rc = EXIT_SUCCESS;
        if (dflt_check->set.s[u]->nodetype == LYS_LEAF) {
            leaf = (struct lys_node_leaf *)dflt_check->set.s[u];
            target_name = leaf->name;
            value = leaf->dflt;
            rc = unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DFLT, (struct lys_node *)(&leaf->dflt));
        } else { /* LYS_LEAFLIST */
            llist = (struct lys_node_leaflist *)dflt_check->set.s[u];
            target_name = llist->name;
            for (i = 0; i < llist->dflt_size; i++) {
                rc = unres_schema_add_node(module, unres, &llist->type, UNRES_TYPE_DFLT,
                                           (struct lys_node *)(&llist->dflt[i]));
                if (rc == -1) {
                    value = llist->dflt[i];
                    break;
                }
            }
        }
        if (rc == -1) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                   "The default value \"%s\" of the deviated node \"%s\"no longer matches its type.",
                   target_name);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;

}

static int
yang_fill_include(struct lys_module *trg, char *value, struct lys_include *inc,
                  struct unres_schema *unres)
{
    struct lys_submodule *submodule;
    struct lys_module *module;
    const char *str;
    int rc;
    int ret = 0;

    str = lydict_insert_zc(trg->ctx, value);
    if (trg->version) {
        submodule = (struct lys_submodule *)trg;
        module = ((struct lys_submodule *)trg)->belongsto;
    } else {
        submodule = NULL;
        module = trg;
    }
    rc = lyp_check_include(module, submodule, str, inc, unres);
    if (!rc) {
        /* success, copy the filled data into the final array */
        memcpy(&trg->inc[trg->inc_size], inc, sizeof *inc);
        trg->inc_size++;
    } else if (rc == -1) {
        ret = -1;
    }

    lydict_remove(trg->ctx, str);
    return ret;
}

int
yang_use_extension(struct lys_module *module, struct lys_node *data_node, void *actual, char *value)
{
    char *prefix;
    char *identif;
    const char *ns = NULL;
    int i;

    /* check to the same pointer */
    if (data_node != actual) {
        return EXIT_SUCCESS;
    }

    prefix = strdup(value);
    if (!prefix) {
        LOGMEM;
        goto error;
    }
    /* find prefix anf identificator*/
    identif = strchr(prefix, ':');
    if (!identif) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, prefix);
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The extension must have prefix.");
        goto error;
    }
    *identif = '\0';
    identif++;

    for(i = 0; i < module->imp_size; ++i) {
        if (!strcmp(module->imp[i].prefix, prefix)) {
            ns = module->imp[i].module->ns;
            break;
        }
    }
    if (!ns && !strcmp(module->prefix, prefix)) {
        ns = (module->type) ? ((struct lys_submodule *)module)->belongsto->ns : module->ns;
    }
    if (ns && !strcmp(ns, LY_NSNACM)) {
        if (!strcmp(identif, "default-deny-write")) {
            data_node->nacm |= LYS_NACM_DENYW;
        } else if (!strcmp(identif, "default-deny-all")) {
            data_node->nacm |= LYS_NACM_DENYA;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, identif);
            goto error;
        }
    }
    free(prefix);
    return EXIT_SUCCESS;

error:
    free(prefix);
    return EXIT_FAILURE;
}

void
nacm_inherit(struct lys_module *module)
{
    struct lys_node *next, *elem, *tmp_node, *tmp_child;

    LY_TREE_DFS_BEGIN(module->data, next, elem) {
        tmp_node = NULL;
        if (elem->parent) {
            switch (elem->nodetype) {
                case LYS_GROUPING:
                    /* extension nacm not inherited*/
                    break;
                case LYS_CHOICE:
                case LYS_ANYXML:
                case LYS_ANYDATA:
                case LYS_USES:
                    if (elem->parent->nodetype != LYS_GROUPING) {
                        elem->nacm |= elem->parent->nacm;
                    }
                    break;
                case LYS_CONTAINER:
                case LYS_LIST:
                case LYS_CASE:
                case LYS_NOTIF:
                case LYS_RPC:
                case LYS_INPUT:
                case LYS_OUTPUT:
                case LYS_AUGMENT:
                    elem->nacm |= elem->parent->nacm;
                    break;
                case LYS_LEAF:
                case LYS_LEAFLIST:
                    tmp_node = elem;
                    tmp_child = elem->child;
                    elem->child = NULL;
                default:
                    break;
            }
        }
        LY_TREE_DFS_END(module->data, next, elem);
        if (tmp_node) {
            tmp_node->child = tmp_child;
        }
    }
}

int
store_flags(struct lys_node *node, uint8_t flags, int config_opt)
{
    struct lys_node *elem;

    node->flags |= (config_opt == CONFIG_IGNORE) ? flags & (~(LYS_CONFIG_MASK | LYS_CONFIG_SET)): flags;
    if (config_opt == CONFIG_INHERIT_ENABLE) {
        if (!(node->flags & LYS_CONFIG_MASK)) {
            /* get config flag from parent */
            if (node->parent) {
                node->flags |= node->parent->flags & LYS_CONFIG_MASK;
            } else {
                /* default config is true */
                node->flags |= LYS_CONFIG_W;
            }
        } else {
            /* do we even care about config flags? */
            for (elem = node; elem && !(elem->nodetype & (LYS_NOTIF | LYS_INPUT | LYS_OUTPUT | LYS_RPC)); elem = elem->parent);

            if (!elem && (node->flags & LYS_CONFIG_W) && node->parent && (node->parent->flags & LYS_CONFIG_R)) {
                LOGVAL(LYE_INARG, LY_VLOG_LYS, node, "true", "config");
                LOGVAL(LYE_SPEC, LY_VLOG_LYS, node, "State nodes cannot have configuration nodes as children.");
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

void
store_config_flag(struct lys_node *node, int config_opt)
{
    if (config_opt == CONFIG_IGNORE) {
        node->flags |= node->flags & (~(LYS_CONFIG_MASK | LYS_CONFIG_SET));
    } else if (config_opt == CONFIG_INHERIT_ENABLE) {
        if (!(node->flags & LYS_CONFIG_MASK)) {
            /* get config flag from parent */
            if (node->parent) {
                node->flags |= node->parent->flags & LYS_CONFIG_MASK;
            } else {
                /* default config is true */
                node->flags |= LYS_CONFIG_W;
            }
        }
    }
}

int
yang_parse_mem(struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres,
               const char *data, unsigned int size_data, struct lys_node **node)
{
    unsigned int size;
    YY_BUFFER_STATE bp;
    yyscan_t scanner = NULL;
    int ret = EXIT_SUCCESS, remove_import = 1;
    struct lys_module *trg;

    size = (size_data) ? size_data : strlen(data) + 2;
    yylex_init(&scanner);
    bp = yy_scan_buffer((char *)data, size, scanner);
    yy_switch_to_buffer(bp, scanner);
    if (yyparse(scanner, NULL, module, submodule, unres, node, &remove_import)) {
        if (remove_import) {
            trg = (submodule) ? (struct lys_module *)submodule : module;
            yang_free_import(trg->ctx, trg->imp, 0, trg->imp_size);
            yang_free_include(trg->ctx, trg->inc, 0, trg->inc_size);
            trg->inc_size = 0;
            trg->imp_size = 0;
        }
        ret = EXIT_FAILURE;
    }
    yy_delete_buffer(bp, scanner);
    yylex_destroy(scanner);
    return ret;
}

struct lys_module *
yang_read_module(struct ly_ctx *ctx, const char* data, unsigned int size, const char *revision, int implement)
{

    struct lys_module *tmp_module, *module = NULL;
    struct unres_schema *unres = NULL;
    struct lys_node *node = NULL;

    unres = calloc(1, sizeof *unres);
    if (!unres) {
        LOGMEM;
        goto error;
    }

    module = calloc(1, sizeof *module);
    if (!module) {
        LOGMEM;
        goto error;
    }

    /* initiale module */
    module->ctx = ctx;
    module->type = 0;
    module->implemented = (implement ? 1 : 0);

    if (yang_parse_mem(module, NULL, unres, data, size, &node)) {
        free_yang_common(module, node);
        goto error;
    }

    if (yang_check_sub_module(module, unres, node)) {
        goto error;
    }

    if (module && unres->count && resolve_unres_schema(module, unres)) {
        goto error;
    }

    if (revision) {
        /* check revision of the parsed model */
        if (!module->rev_size || strcmp(revision, module->rev[0].date)) {
            LOGVRB("Module \"%s\" parsed with the wrong revision (\"%s\" instead \"%s\").",
                   module->name, module->rev[0].date, revision);
            goto error;
        }
    }

    tmp_module = module;
    if (lyp_ctx_add_module(&module)) {
        goto error;
    }

    if (module == tmp_module) {
        nacm_inherit(module);
    }

    if (module->deviation_size && !module->implemented) {
        LOGVRB("Module \"%s\" includes deviations, changing its conformance to \"implement\".", module->name);
        /* deviations always causes target to be made implemented,
         * but augents and leafrefs not, so we have to apply them now */
        if (lys_set_implemented(module)) {
            goto error;
        }
    }

    unres_schema_free(NULL, &unres);
    LOGVRB("Module \"%s\" successfully parsed.", module->name);
    return module;

error:
    /* cleanup */
    unres_schema_free(module, &unres);
    if (!module || !module->name) {
        free(module);
        if (ly_vecode != LYVE_SUBMODULE) {
            LOGERR(ly_errno, "Module parsing failed.");
        }
        return NULL;
    }

    LOGERR(ly_errno, "Module \"%s\" parsing failed.", module->name);

    lys_sub_module_remove_devs_augs(module);
    lys_free(module, NULL, 1);
    return NULL;
}

struct lys_submodule *
yang_read_submodule(struct lys_module *module, const char *data, unsigned int size, struct unres_schema *unres)
{
    struct lys_submodule *submodule;
    struct lys_node *node = NULL;

    submodule = calloc(1, sizeof *submodule);
    if (!submodule) {
        LOGMEM;
        goto error;
    }

    submodule->ctx = module->ctx;
    submodule->type = 1;
    submodule->belongsto = module;

    if (yang_parse_mem(module, submodule, unres, data, size, &node)) {
        free_yang_common((struct lys_module *)submodule, node);
        goto error;
    }

    if (yang_check_sub_module((struct lys_module *)submodule, unres, node)) {
        goto error;
    }

    LOGVRB("Submodule \"%s\" successfully parsed.", submodule->name);
    return submodule;

error:
    /* cleanup */
    unres_schema_free((struct lys_module *)submodule, &unres);

    if (!submodule || !submodule->name) {
        free(submodule);
        LOGERR(ly_errno, "Submodule parsing failed.");
        return NULL;
    }

    LOGERR(ly_errno, "Submodule \"%s\" parsing failed.", submodule->name);

    lys_sub_module_remove_devs_augs((struct lys_module *)submodule);
    lys_submodule_module_data_free(submodule);
    lys_submodule_free(submodule, NULL);
    return NULL;
}

static int
read_indent(const char *input, int indent, int size, int in_index, int *out_index, char *output)
{
    int k = 0, j;

    while (in_index < size) {
        if (input[in_index] == ' ') {
            k++;
        } else if (input[in_index] == '\t') {
            /* RFC 6020 6.1.3 tab character is treated as 8 space characters */
            k += 8;
        } else  if (input[in_index] == '\\' && input[in_index + 1] == 't') {
            /* RFC 6020 6.1.3 tab character is treated as 8 space characters */
            k += 8;
            ++in_index;
        } else {
            break;
        }
        ++in_index;
        if (k >= indent) {
            for (j = k - indent; j > 0; --j) {
                output[*out_index] = ' ';
                ++(*out_index);
            }
            break;
        }
    }
    return in_index - 1;
}

char *
yang_read_string(const char *input, char *output, int size, int offset, int indent, int version) {
    int i = 0, out_index = offset, space = 0;

    while (i < size) {
        switch (input[i]) {
        case '\n':
            out_index -= space;
            output[out_index] = '\n';
            space = 0;
            i = read_indent(input, indent, size, i + 1, &out_index, output);
            break;
        case ' ':
        case '\t':
            output[out_index] = input[i];
            ++space;
            break;
        case '\\':
            if (input[i + 1] == 'n') {
                out_index -= space;
                output[out_index] = '\n';
                space = 0;
                i = read_indent(input, indent, size, i + 2, &out_index, output);
            } else if (input[i + 1] == 't') {
                output[out_index] = '\t';
                ++i;
                ++space;
            } else if (input[i + 1] == '\\') {
                output[out_index] = '\\';
                ++i;
            } else if ((i + 1) != size && input[i + 1] == '"') {
                output[out_index] = '"';
                ++i;
            } else {
                if (version < 2) {
                    output[out_index] = input[i];
                } else {
                    /* YANG 1.1 backslash must not be followed by any other character */
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, input);
                    return NULL;
                }
            }
            break;
        default:
            output[out_index] = input[i];
            space = 0;
            break;
        }
        ++i;
        ++out_index;
    }
    output[out_index] = '\0';
    if (size != out_index) {
        output = realloc(output, out_index + 1);
        if (!output) {
            LOGMEM;
            return NULL;
        }
    }
    return output;
}

/* free function */

static void yang_type_free(struct ly_ctx *ctx, struct lys_type *type)
{
    struct yang_type *stype = (struct yang_type *)type->der;
    int i;

    if (!stype) {
        return ;
    }
    lydict_remove(ctx, stype->name);
    type->base = stype->base;
    if (type->base == LY_TYPE_IDENT) {
        for (i = 0; i < type->info.ident.count; ++i) {
            free(type->info.ident.ref[i]);
        }
    }
    lys_type_free(ctx, type);
    type->base = LY_TYPE_DER;
    free(stype);
}

static void
yang_tpdf_free(struct ly_ctx *ctx, struct lys_tpdf *tpdf, uint8_t start, uint8_t size)
{
    uint8_t i;

    assert(ctx);
    if (!tpdf) {
        return;
    }

    for (i = start; i < size; ++i) {
        lydict_remove(ctx, tpdf[i].name);
        lydict_remove(ctx, tpdf[i].dsc);
        lydict_remove(ctx, tpdf[i].ref);

        yang_type_free(ctx, &tpdf[i].type);

        lydict_remove(ctx, tpdf[i].units);
        lydict_remove(ctx, tpdf[i].dflt);
    }
}

static void
yang_free_import(struct ly_ctx *ctx, struct lys_import *imp, uint8_t start, uint8_t size)
{
    uint8_t i;

    for (i = start; i < size; ++i){
        free((char *)imp[i].module);
        lydict_remove(ctx, imp[i].prefix);
        lydict_remove(ctx, imp[i].dsc);
        lydict_remove(ctx, imp[i].ref);
    }
}

static void
yang_free_include(struct ly_ctx *ctx, struct lys_include *inc, uint8_t start, uint8_t size)
{
    uint8_t i;

    for (i = start; i < size; ++i){
        free((char *)inc[i].submodule);
        lydict_remove(ctx, inc[i].dsc);
        lydict_remove(ctx, inc[i].ref);
    }
}

static void
yang_free_ident_base(struct lys_ident *ident, uint32_t start, uint32_t size)
{
    uint32_t i;
    uint8_t j;

    /* free base name */
    for (i = start; i < size; ++i) {
        for (j = 0; j < ident[i].base_size; ++j) {
            free(ident[i].base[j]);
        }
    }
}

static void
yang_free_grouping(struct ly_ctx *ctx, struct lys_node_grp * grp)
{
    yang_tpdf_free(ctx, grp->tpdf, 0, grp->tpdf_size);
    free(grp->tpdf);
}

static void
yang_free_container(struct ly_ctx *ctx, struct lys_node_container * cont)
{
    uint8_t i;

    yang_tpdf_free(ctx, cont->tpdf, 0, cont->tpdf_size);
    free(cont->tpdf);
    lydict_remove(ctx, cont->presence);

    for (i = 0; cont->must_size; ++i) {
        lys_restr_free(ctx, &cont->must[i]);
    }
    free(cont->must);

    lys_when_free(ctx, cont->when);
}

static void
yang_free_leaf(struct ly_ctx *ctx, struct lys_node_leaf *leaf)
{
    uint8_t i;

    for (i = 0; i < leaf->must_size; i++) {
        lys_restr_free(ctx, &leaf->must[i]);
    }
    free(leaf->must);

    lys_when_free(ctx, leaf->when);

    yang_type_free(ctx, &leaf->type);
    lydict_remove(ctx, leaf->units);
    lydict_remove(ctx, leaf->dflt);
}

static void
yang_free_leaflist(struct ly_ctx *ctx, struct lys_node_leaflist *leaflist)
{
    uint8_t i;

    for (i = 0; i < leaflist->must_size; i++) {
        lys_restr_free(ctx, &leaflist->must[i]);
    }
    free(leaflist->must);

    for (i = 0; i < leaflist->dflt_size; i++) {
        lydict_remove(ctx, leaflist->dflt[i]);
    }
    free(leaflist->dflt);

    lys_when_free(ctx, leaflist->when);

    yang_type_free(ctx, &leaflist->type);
    lydict_remove(ctx, leaflist->units);
}

static void
yang_free_list(struct ly_ctx *ctx, struct lys_node_list *list)
{
    uint8_t i;

    yang_tpdf_free(ctx, list->tpdf, 0, list->tpdf_size);
    free(list->tpdf);

    for (i = 0; i < list->must_size; ++i) {
        lys_restr_free(ctx, &list->must[i]);
    }
    free(list->must);

    lys_when_free(ctx, list->when);

    for (i = 0; i < list->unique_size; ++i) {
        free(list->unique[i].expr);
    }
    free(list->unique);

    free(list->keys);
}

static void
yang_free_choice(struct ly_ctx *ctx, struct lys_node_choice *choice)
{
    free(choice->dflt);
    lys_when_free(ctx, choice->when);
}

static void
yang_free_anydata(struct ly_ctx *ctx, struct lys_node_anydata *anydata)
{
    uint8_t i;

    for (i = 0; i < anydata->must_size; ++i) {
        lys_restr_free(ctx, &anydata->must[i]);
    }
    free(anydata->must);

    lys_when_free(ctx, anydata->when);
}

static void
yang_free_nodes(struct ly_ctx *ctx, struct lys_node *node)
{
    struct lys_node *tmp, *child, *sibling;

    if (!node) {
        return;
    }
    tmp = node;

    while (tmp) {
        child = tmp->child;
        sibling = tmp->next;
        /* common part */
        lydict_remove(ctx, tmp->name);
        if (!(tmp->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
            lys_iffeature_free(tmp->iffeature, tmp->iffeature_size);
            lydict_remove(ctx, tmp->dsc);
            lydict_remove(ctx, tmp->ref);
        }

        switch (tmp->nodetype) {
        case LYS_GROUPING:
            yang_free_grouping(ctx, (struct lys_node_grp *)tmp);
            break;
        case LYS_CONTAINER:
            yang_free_container(ctx, (struct lys_node_container *)tmp);
            break;
        case LYS_LEAF:
            yang_free_leaf(ctx, (struct lys_node_leaf *)tmp);
            break;
        case LYS_LEAFLIST:
            yang_free_leaflist(ctx, (struct lys_node_leaflist *)tmp);
            break;
        case LYS_LIST:
            yang_free_list(ctx, (struct lys_node_list *)tmp);
            break;
        case LYS_CHOICE:
            yang_free_choice(ctx, (struct lys_node_choice *)tmp);
            break;
        case LYS_CASE:
            lys_when_free(ctx, ((struct lys_node_case *)tmp)->when);
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            yang_free_anydata(ctx, (struct lys_node_anydata *)tmp);
            break;
        default:
            break;
        }

        yang_free_nodes(ctx, child);
        free(tmp);
        tmp = sibling;
    }
}

/* free common item from module and submodule */
static void
free_yang_common(struct lys_module *module, struct lys_node *node)
{
    yang_tpdf_free(module->ctx, module->tpdf, 0, module->tpdf_size);
    module->tpdf_size = 0;
    yang_free_ident_base(module->ident, 0, module->ident_size);
    yang_free_nodes(module->ctx, node);
}

/* check function*/

int
yang_check_imports(struct lys_module *module, struct unres_schema *unres)
{
    struct lys_import *imp;
    struct lys_include *inc;
    uint8_t imp_size, inc_size, j = 0, i = 0;
    size_t size;
    char *s;

    imp = module->imp;
    imp_size = module->imp_size;
    inc = module->inc;
    inc_size = module->inc_size;

    if (imp_size) {
        size = (imp_size * sizeof *module->imp) + sizeof(void*);
        module->imp_size = 0;
        module->imp = calloc(1, size);
        if (!module->imp) {
            LOGMEM;
            goto error;
        }
        /* set stop block for possible realloc */
        module->imp[imp_size].module = (void*)0x1;
    }

    if (inc_size) {
        size = (inc_size * sizeof *module->inc) + sizeof(void*);
        module->inc_size = 0;
        module->inc = calloc(1, size);
        if (!module->inc) {
            LOGMEM;
            goto error;
        }
        /* set stop block for possible realloc */
        module->inc[inc_size].submodule = (void*)0x1;
    }

    for (i = 0; i < imp_size; ++i) {
        s = (char *) imp[i].module;
        imp[i].module = NULL;
        if (yang_fill_import(module, &imp[i], &module->imp[module->imp_size], s)) {
            ++i;
            goto error;
        }
    }
    for (j = 0; j < inc_size; ++j) {
        s = (char *) inc[i].submodule;
        inc[i].submodule = NULL;
        if (yang_fill_include(module, s, &inc[i], unres)) {
            ++i;
            goto error;
        }
    }
    free(inc);
    free(imp);

    return EXIT_SUCCESS;

error:
    yang_free_import(module->ctx, imp, i, imp_size);
    yang_free_include(module->ctx, inc, j, inc_size);
    free(imp);
    free(inc);
    return EXIT_FAILURE;
}

static int
yang_check_type_iffeature(struct lys_module *module, struct unres_schema *unres, struct lys_type *type)
{
    uint i, j, size;
    uint8_t iffeature_size;
    LY_DATA_TYPE stype;

    if (((struct yang_type *)type->der)->base == LY_TYPE_ENUM) {
        stype = LY_TYPE_ENUM;
        size = type->info.enums.count;
    } else if (((struct yang_type *)type->der)->base == LY_TYPE_BITS) {
        stype = LY_TYPE_ENUM;
        size = type->info.enums.count;
    } else {
        return EXIT_SUCCESS;
    }

    for (i = 0; i < size; ++i) {
        if (stype == LY_TYPE_ENUM) {
            iffeature_size = type->info.enums.enm[i].iffeature_size;
            type->info.enums.enm[i].iffeature_size = 0;
            for (j = 0; j < iffeature_size; ++j) {
                if (yang_read_if_feature(module, &type->info.enums.enm[i], type->parent,
                                         (char *)type->info.enums.enm[i].iffeature[j].features, unres, ENUM_KEYWORD)) {
                    goto error;
                }
            }
        } else {
            iffeature_size = type->info.bits.bit[i].iffeature_size;
            type->info.bits.bit[i].iffeature_size = 0;
            for (j = 0; j < iffeature_size; ++j) {
                if (yang_read_if_feature(module, &type->info.bits.bit[i], type->parent,
                                         (char *)type->info.bits.bit[i].iffeature[j].features, unres, BIT_KEYWORD)) {
                    goto error;
                }
            }
        }
    }

    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}

int
yang_check_typedef(struct lys_module *module, struct lys_node *parent, struct unres_schema *unres)
{
    struct lys_tpdf *tpdf;
    uint8_t i, tpdf_size, *ptr_tpdf_size;
    int ret = EXIT_SUCCESS;

    if (!parent) {
        tpdf = module->tpdf;
        ptr_tpdf_size = &module->tpdf_size;
    } else {
        switch (parent->nodetype) {
        case LYS_GROUPING:
            tpdf = ((struct lys_node_grp *)parent)->tpdf;
            ptr_tpdf_size = &((struct lys_node_grp *)parent)->tpdf_size;
            break;
        case LYS_CONTAINER:
            tpdf = ((struct lys_node_container *)parent)->tpdf;
            ptr_tpdf_size = &((struct lys_node_container *)parent)->tpdf_size;
            break;
        case LYS_LIST:
            tpdf = ((struct lys_node_list *)parent)->tpdf;
            ptr_tpdf_size = &((struct lys_node_list *)parent)->tpdf_size;
            break;
        default:
            LOGINT;
            return EXIT_FAILURE;
        }
    }

    tpdf_size = *ptr_tpdf_size;
    *ptr_tpdf_size = 0;

    for (i = 0; i < tpdf_size; ++i) {
        tpdf[i].type.parent = &tpdf[i];
        if (yang_check_type_iffeature(module, unres, &tpdf[i].type)) {
            ret = EXIT_FAILURE;
            break;
        }
        if (unres_schema_add_node(module, unres, &tpdf[i].type, UNRES_TYPE_DER_TPDF, parent) == -1) {
            ret = EXIT_FAILURE;
            break;
        }

        /* check default value*/
        if (tpdf[i].dflt && unres_schema_add_str(module, unres, &tpdf[i].type, UNRES_TYPE_DFLT, tpdf[i].dflt) == -1) {
            ++i;
            ret = EXIT_FAILURE;
            break;
        }
        (*ptr_tpdf_size)++;
    }
    if (i < tpdf_size) {
        yang_tpdf_free(module->ctx, tpdf, i, tpdf_size);
    }

    return ret;
}

static int
yang_check_identities(struct lys_module *module, struct unres_schema *unres)
{
    uint32_t i, size, base_size;
    uint8_t j;

    size = module->ident_size;
    module->ident_size = 0;
    for (i = 0; i < size; ++i) {
        base_size = module->ident[i].base_size;
        module->ident[i].base_size = 0;
        for (j = 0; j < base_size; ++j) {
            if (yang_read_base(module, &module->ident[i], (char *)module->ident[i].base[j], unres)) {
                ++j;
                module->ident_size = size;
                goto error;
            }
        }
        module->ident_size++;
    }

    return EXIT_SUCCESS;

error:
    for (; j< module->ident[i].base_size; ++j) {
        free(module->ident[i].base[j]);
    }
    yang_free_ident_base(module->ident, i + 1, size);
    return EXIT_FAILURE;
}

static int
yang_check_grouping(struct lys_module *module, struct lys_node_grp *node, struct unres_schema *unres)
{
    uint8_t i, size;

    if (yang_check_typedef(module, (struct lys_node *)node, unres)) {
        goto error;
    }

    size = node->iffeature_size;
    node->iffeature_size = 0;
    for (i = 0; i < size; ++i) {
        if (yang_read_if_feature(module, node, NULL, (char *)node->iffeature[i].features, unres, GROUPING_KEYWORD)) {
            node->iffeature_size = size;
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
yang_check_container(struct lys_module *module, struct lys_node_container *node, struct unres_schema *unres)
{
    uint8_t i, size;

    if (yang_check_typedef(module, (struct lys_node *)node, unres)) {
        goto error;
    }

    size = node->iffeature_size;
    node->iffeature_size = 0;
    for (i = 0; i < size; ++i) {
        if (yang_read_if_feature(module, node, NULL, (char *)node->iffeature[i].features, unres, CONTAINER_KEYWORD)) {
            node->iffeature_size = size;
            goto error;
        }
    }

    /* check XPath dependencies */
    if ((node->when || node->must_size) && (unres_schema_add_node(module, unres, node, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}

static int
yang_check_leaf(struct lys_module *module, struct lys_node_leaf *leaf, struct unres_schema *unres)
{
    uint8_t i, size;

    if (yang_check_type_iffeature(module, unres, &leaf->type)) {
        yang_type_free(module->ctx, &leaf->type);
        goto error;
    }

    if (unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DER_TPDF, (struct lys_node *)leaf) == -1) {
        yang_type_free(module->ctx, &leaf->type);
        goto error;
    }

    if (leaf->dflt && unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DFLT, (struct lys_node *)leaf->dflt) == -1) {
        goto error;
    }

    size = leaf->iffeature_size;
    leaf->iffeature_size = 0;
    for (i = 0; i < size; ++i) {
        if (yang_read_if_feature(module, leaf, NULL, (char *)leaf->iffeature[i].features, unres, LEAF_KEYWORD)) {
            leaf->iffeature_size = size;
            goto error;
        }
    }

    /* check XPath dependencies */
    if ((leaf->when || leaf->must_size) && (unres_schema_add_node(module, unres, leaf, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}

static int
yang_check_leaflist(struct lys_module *module, struct lys_node_leaflist *leaflist, struct unres_schema *unres)
{
    uint8_t i, size;

    if (yang_check_type_iffeature(module, unres, &leaflist->type)) {
        yang_type_free(module->ctx, &leaflist->type);
        goto error;
    }

    if (unres_schema_add_node(module, unres, &leaflist->type, UNRES_TYPE_DER_TPDF, (struct lys_node *)leaflist) == -1) {
        yang_type_free(module->ctx, &leaflist->type);
        goto error;
    }

    /* check default value (if not defined, there still could be some restrictions
     * that need to be checked against a default value from a derived type) */
    for (i = 0; i < leaflist->dflt_size; ++i) {
        if (unres_schema_add_node(module, unres, &leaflist->type, UNRES_TYPE_DFLT, (struct lys_node *)(&leaflist->dflt[i])) == -1) {
            goto error;
        }
    }

    size = leaflist->iffeature_size;
    leaflist->iffeature_size = 0;
    for (i = 0; i < size; ++i) {
        if (yang_read_if_feature(module, leaflist, NULL, (char *)leaflist->iffeature[i].features, unres, LEAF_LIST_KEYWORD)) {
            leaflist->iffeature_size = size;
            goto error;
        }
    }

    /* check XPath dependencies */
    if ((leaflist->when || leaflist->must_size) && (unres_schema_add_node(module, unres, leaflist, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}

static int
yang_check_list(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres)
{
    uint8_t size, i;
    
    if (yang_check_typedef(module, (struct lys_node *)list, unres)) {
        goto error;
    }

    size = list->iffeature_size;
    list->iffeature_size = 0;
    for (i = 0; i < size; ++i) {
        if (yang_read_if_feature(module, list, NULL, (char *)list->iffeature[i].features, unres, LIST_KEYWORD)) {
            list->iffeature_size = size;
            goto error;
        }
    }

    if (list->keys && yang_read_key(module, list, unres)) {
        goto error;
    }

    if (yang_read_unique(module, list, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if ((list->when || list->must_size) && (unres_schema_add_node(module, unres, list, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}

static int
yang_check_choice(struct lys_module *module, struct lys_node_choice *choice, struct unres_schema *unres)
{
    char *value;
    uint8_t size, i;

    if (choice->dflt) {
        value = (char *)choice->dflt;
        choice->dflt = NULL;
        if (unres_schema_add_str(module, unres, choice, UNRES_CHOICE_DFLT, value) == -1) {
            free(value);
            goto error;
        }
        free(value);
    }

    size = choice->iffeature_size;
    choice->iffeature_size = 0;
    for (i = 0; i < size; ++i) {
        if (yang_read_if_feature(module, choice, NULL, (char *)choice->iffeature[i].features, unres, CHOICE_KEYWORD)) {
            choice->iffeature_size = size;
            goto error;
        }
    }

    /* check XPath dependencies */
    if ((choice->when) && (unres_schema_add_node(module, unres, choice, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}

static int
yang_check_case(struct lys_module *module, struct lys_node_case *cs, struct unres_schema *unres)
{
    uint8_t size, i;

    size = cs->iffeature_size;
    cs->iffeature_size = 0;
    for (i = 0; i < size; ++i) {
        if (yang_read_if_feature(module, cs, NULL, (char *)cs->iffeature[i].features, unres, CASE_KEYWORD)) {
            cs->iffeature_size = size;
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

static int
yang_check_anydata(struct lys_module *module, struct lys_node_anydata *anydata, struct unres_schema *unres)
{
    uint8_t size, i;

    size = anydata->iffeature_size;
    anydata->iffeature_size = 0;
    for (i = 0; i < size; ++i) {
        if (yang_read_if_feature(module, anydata, NULL, (char *)anydata->iffeature[i].features, unres, ANYXML_KEYWORD)) {
            anydata->iffeature_size = size;
            goto error;
        }
    }

    /* check XPath dependencies */
    if ((anydata->when) && (unres_schema_add_node(module, unres, anydata, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}

static int
yang_check_nodes(struct lys_module *module, struct lys_node *nodes, struct unres_schema *unres)
{
    struct lys_node *node = nodes, *sibling, *child, *parent;

    while (node) {
        sibling = node->next;
        child = node->child;
        parent = node->parent;
        node->next = NULL;
        node->child = NULL;
        node->parent = NULL;
        node->prev = node;

        if (lys_node_addchild(parent, module->type ? ((struct lys_submodule *)module)->belongsto: module, node)) {
            lys_node_unlink(node);
            node->next = sibling;
            sibling = node;
            goto error;
        }
        switch (node->nodetype) {
        case LYS_GROUPING:
            if (yang_check_grouping(module, (struct lys_node_grp *)node, unres)) {
                goto error;
            }
            break;
        case LYS_CONTAINER:
            if (yang_check_container(module, (struct lys_node_container *)node, unres)) {
                goto error;
            }
            break;
        case LYS_LEAF:
            if (yang_check_leaf(module, (struct lys_node_leaf *)node, unres)) {
                goto error;
            }
            break;
        case LYS_LEAFLIST:
            if (yang_check_leaflist(module, (struct lys_node_leaflist *)node, unres)) {
                goto error;
            }
            break;
        case LYS_LIST:
            if (yang_check_list(module, (struct lys_node_list *)node, unres)) {
                goto error;
            }
            break;
        case LYS_CHOICE:
            if (yang_check_choice(module, (struct lys_node_choice *)node, unres)) {
                goto error;
            }
            break;
        case LYS_CASE:
            if (yang_check_case(module, (struct lys_node_case *)node, unres)) {
                goto error;
            }
            break;
        case LYS_ANYDATA:
        case LYS_ANYXML:
            if (yang_check_anydata(module, (struct lys_node_anydata *)node, unres)) {
                goto error;
            }
            break;
        default:
            LOGINT;
            sibling = node;
            child = NULL;
            goto error;
        }
        if (yang_check_nodes(module, child, unres)) {
            child = NULL;
            goto error;
        }
        node = sibling;
    }

    return EXIT_SUCCESS;
error:
    yang_free_nodes(module->ctx, sibling);
    yang_free_nodes(module->ctx, child);
    return EXIT_FAILURE;
}

static int
yang_check_sub_module(struct lys_module *module, struct unres_schema *unres, struct lys_node *node)
{
    uint8_t i, j, size, erase_identities = 1, erase_nodes = 1;
    struct lys_feature *feature; /* shortcut */
    char *s;

    if (yang_check_typedef(module, NULL, unres)) {
        goto error;
    }

    /* check features */
    for (i = 0; i < module->features_size; ++i) {
        feature = &module->features[i];
        size = feature->iffeature_size;
        feature->iffeature_size = 0;
        for (j = 0; j < size; ++j) {
            s = (char *)feature->iffeature[j].features;
            if (yang_read_if_feature(module, feature, NULL, s, unres, FEATURE_KEYWORD)) {
                goto error;
            }
            if (unres_schema_add_node(module, unres, feature, UNRES_FEATURE, NULL) == -1) {
                goto error;
            }
        }
    }
    erase_identities = 0;
    if (yang_check_identities(module, unres)) {
        goto error;
    }
    erase_nodes = 0;
    if (yang_check_nodes(module, node, unres)) {
        goto error;
    }

    return EXIT_SUCCESS;
error:
    if (erase_identities) {
        yang_free_ident_base(module->ident, 0, module->ident_size);
    }
    if (erase_nodes) {
        yang_free_nodes(module->ctx, node);
    }
    return EXIT_FAILURE;
}
