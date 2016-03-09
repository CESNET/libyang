/**
 * @file parser_yang.c
 * @author Pavol Vican
 * @brief YANG parser for libyang
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

#include <ctype.h>
#include <pcre.h>
#include "parser_yang.h"
#include "parser_yang_bis.h"
#include "parser.h"
#include "xpath.h"

static int 
yang_check_string(struct lys_module *module, const char **target, char *what, char *where, char *value, int line)
{
    if (*target) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, what, where);
        free(value);
        return 1;
    } else {
        *target = lydict_insert_zc(module->ctx, value);
        return 0;
    }
}

int 
yang_read_common(struct lys_module *module, char *value, int type, int line) 
{
    int ret = 0;

    switch (type) {
    case MODULE_KEYWORD:
        module->name = lydict_insert_zc(module->ctx, value);
        break;
    case NAMESPACE_KEYWORD:
        ret = yang_check_string(module, &module->ns, "namespace", "module", value, line);
        break;
    case ORGANIZATION_KEYWORD:
        ret = yang_check_string(module, &module->org, "organization", "module", value, line);
        break;
    case CONTACT_KEYWORD:
        ret = yang_check_string(module, &module->contact, "contact", "module", value, line);
        break;
    }

    return ret;
}

int 
yang_read_prefix(struct lys_module *module, void *save, char *value, int type, int line) 
{
    int ret = 0;

    if (lyp_check_identifier(value, LY_IDENT_PREFIX, line, module, NULL)) {
        free(value);
        return EXIT_FAILURE;
    }
    switch (type){
    case MODULE_KEYWORD:
        ret = yang_check_string(module, &module->prefix, "prefix", "module", value, line);
        break;
    case IMPORT_KEYWORD:
        ((struct lys_import *)save)->prefix = lydict_insert_zc(module->ctx, value);
        break;
    }

    return ret;
}

void *
yang_elem_of_array(void **ptr, uint8_t *act_size, int type, int sizeof_struct)
{
    void *retval;

    if (!(*act_size % LY_ARRAY_SIZE) && !(*ptr = ly_realloc(*ptr, (*act_size + LY_ARRAY_SIZE) * sizeof_struct))) {
        LOGMEM;
        return NULL;
    }
    switch (type) {
    case IMPORT_KEYWORD:
        retval = &((struct lys_import *)(*ptr))[*act_size];
        break;
    case REVISION_KEYWORD:
        retval = &((struct lys_revision *)(*ptr))[*act_size];
        break;
    }
    (*act_size)++;
    memset(retval,0,sizeof_struct);
    return retval;
}

int
yang_fill_import(struct lys_module *module, struct lys_import *imp, char *value, int line)
{
    int count, i;

    /* check for circular import, store it if passed */
    if (!module->ctx->models.parsing) {
        count = 0;
    } else {
        for (count = 0; module->ctx->models.parsing[count]; ++count) {
            if (value == module->ctx->models.parsing[count]) {
                LOGERR(LY_EVALID, "Circular import dependency on the module \"%s\".", value);
                goto error;
            }
        }
    }
    ++count;
    module->ctx->models.parsing =
            ly_realloc(module->ctx->models.parsing, (count + 1) * sizeof *module->ctx->models.parsing);
    if (!module->ctx->models.parsing) {
        LOGMEM;
        goto error;
    }
    module->ctx->models.parsing[count - 1] = value;
    module->ctx->models.parsing[count] = NULL;

    /* try to load the module */
    imp->module = (struct lys_module *)ly_ctx_get_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL);
    if (!imp->module) {
        /* whether to use a user callback is decided in the function */
        imp->module = (struct lys_module *)ly_ctx_load_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL);
    }

    /* remove the new module name now that its parsing is finished (even if failed) */
    if (module->ctx->models.parsing[count] || (module->ctx->models.parsing[count - 1] != value)) {
        LOGINT;
    }
    --count;
    if (count) {
        module->ctx->models.parsing[count] = NULL;
    } else {
        free(module->ctx->models.parsing);
        module->ctx->models.parsing = NULL;
    }

    /* check the result */
    if (!imp->module) {
        LOGERR(LY_EVALID, "Importing \"%s\" module into \"%s\" failed.", value, module->name);
        goto error;
    }

    module->imp_size++;

    /* check duplicities in imported modules */
    for (i = 0; i < module->imp_size - 1; i++) {
        if (!strcmp(module->imp[i].module->name, module->imp[module->imp_size - 1].module->name)) {
            LOGVAL(LYE_SPEC, line, LY_VLOG_NONE, NULL, "Importing module \"%s\" repeatedly.", module->imp[i].module->name);
            goto error;
        }
    }

    free(value);
    return EXIT_SUCCESS;

    error:

    free(value);
    return EXIT_FAILURE;
}

int
yang_read_description(struct lys_module *module, void *node, char *value, int type, int line)
{
    int ret;

    if (!node) {
        ret = yang_check_string(module, &module->dsc, "description", "module", value, line);
    } else {
        switch (type) {
        case REVISION_KEYWORD:
            ret = yang_check_string(module, &((struct lys_revision *) node)->dsc, "description", "revision", value, line);
            break;
        case FEATURE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_feature *) node)->dsc, "description", "feature", value, line);
            break;
        case IDENTITY_KEYWORD:
            ret = yang_check_string(module, &((struct lys_ident *) node)->dsc, "description", "identity", value, line);
            break;
        case MUST_KEYWORD:
            ret = yang_check_string(module, &((struct lys_restr *) node)->dsc, "description", "must", value, line);
            break;
        case WHEN_KEYWORD:
            ret = yang_check_string(module, &((struct lys_when *) node)->dsc, "description", "when" , value, line);
            break;
        case CONTAINER_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_container *) node)->dsc, "description", "container", value, line);
            break;
        case ANYXML_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_anyxml *) node)->dsc, "description", "anyxml", value, line);
            break;
        case CHOICE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_choice *) node)->dsc, "description", "choice", value, line);
            break;
        case CASE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_case *) node)->dsc, "description", "case", value, line);
            break;
        case GROUPING_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_grp *) node)->dsc, "description", "grouping", value, line);
            break;
        case LEAF_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_leaf *) node)->dsc, "description", "leaf", value, line);
            break;
        case LEAF_LIST_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_leaflist *) node)->dsc, "description", "leaflist", value, line);
            break;
        case LIST_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_list *) node)->dsc, "description", "list", value, line);
            break;
        case LENGTH_KEYWORD:
            ret = yang_check_string(module, &((struct lys_restr *) node)->dsc, "description", "length", value, line);
            break;
        case PATTERN_KEYWORD:
            ret = yang_check_string(module, &((struct lys_restr *) node)->dsc, "description", "pattern", value, line);
            break;
        case RANGE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_restr *) node)->dsc, "description", "range", value, line);
            break;
        case ENUM_KEYWORD:
            ret = yang_check_string(module, &((struct lys_type_enum *) node)->dsc, "description", "enum", value, line);
            break;
        case BIT_KEYWORD:
            ret = yang_check_string(module, &((struct lys_type_bit *) node)->dsc, "description", "bit", value, line);
            break;
        case TYPEDEF_KEYWORD:
            ret = yang_check_string(module, &((struct lys_tpdf *) node)->dsc, "description", "typedef", value, line);
            break;
        case USES_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_uses *) node)->dsc, "description", "uses", value, line);
            break;
        case REFINE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_refine *) node)->dsc, "description", "refine", value, line);
            break;
        case AUGMENT_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_augment *) node)->dsc, "description", "augment", value, line);
            break;
        case RPC_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_rpc *) node)->dsc, "description", "rpc", value, line);
            break;
        case NOTIFICATION_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_notif *) node)->dsc, "description", "notification", value, line);
            break;
        case DEVIATION_KEYWORD:
            ret = yang_check_string(module, &((struct lys_deviation *) node)->dsc, "description", "deviation", value, line);
            break;
        }
    }
    return ret;
}

int
yang_read_reference(struct lys_module *module, void *node, char *value, int type, int line)
{
    int ret;

    if (!node) {
        ret = yang_check_string(module, &module->ref, "reference", "module", value, line);
    } else {
        switch (type) {
        case REVISION_KEYWORD:
            ret = yang_check_string(module, &((struct lys_revision *) node)->ref, "reference", "revision", value, line);
            break;
        case FEATURE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_feature *) node)->ref, "reference", "feature", value, line);
            break;
        case IDENTITY_KEYWORD:
            ret = yang_check_string(module, &((struct lys_ident *) node)->ref, "reference", "identity", value, line);
            break;
        case MUST_KEYWORD:
            ret = yang_check_string(module, &((struct lys_restr *) node)->ref, "reference", "must", value, line);
            break;
        case WHEN_KEYWORD:
            ret = yang_check_string(module, &((struct lys_when *) node)->ref, "reference", "when", value, line);
            break;
        case CONTAINER_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_container *) node)->ref, "reference", "container", value, line);
            break;
        case ANYXML_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_anyxml *) node)->ref, "reference", "anyxml", value, line);
            break;
        case CHOICE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_anyxml *) node)->ref, "reference", "choice", value, line);
            break;
        case CASE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_anyxml *) node)->ref, "reference", "case", value, line);
            break;
        case GROUPING_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_grp *) node)->ref, "reference", "grouping", value, line);
            break;
        case LEAF_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_leaf *) node)->ref, "reference", "leaf", value, line);
            break;
        case LEAF_LIST_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_leaflist *) node)->ref, "reference", "leaflist", value, line);
            break;
        case LIST_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_list *) node)->ref, "reference", "list", value, line);
            break;
        case LENGTH_KEYWORD:
            ret = yang_check_string(module, &((struct lys_restr *) node)->ref, "reference", "length", value, line);
            break;
        case PATTERN_KEYWORD:
            ret = yang_check_string(module, &((struct lys_restr *) node)->ref, "reference", "pattern", value, line);
            break;
        case RANGE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_restr *) node)->ref, "reference", "range", value, line);
            break;
        case ENUM_KEYWORD:
            ret = yang_check_string(module, &((struct lys_type_enum *) node)->ref, "reference", "enum", value, line);
            break;
        case BIT_KEYWORD:
            ret = yang_check_string(module, &((struct lys_type_bit *) node)->ref, "reference", "bit", value, line);
            break;
        case TYPEDEF_KEYWORD:
            ret = yang_check_string(module, &((struct lys_tpdf *) node)->ref, "reference", "typedef", value, line);
            break;
        case USES_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_uses *) node)->ref, "reference", "uses", value, line);
            break;
        case REFINE_KEYWORD:
            ret = yang_check_string(module, &((struct lys_refine *) node)->ref, "reference", "refine", value, line);
            break;
        case AUGMENT_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_augment *) node)->ref, "reference", "augment", value, line);
            break;
        case RPC_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_rpc *) node)->ref, "reference", "rpc", value, line);
            break;
        case NOTIFICATION_KEYWORD:
            ret = yang_check_string(module, &((struct lys_node_notif *) node)->ref, "reference", "notification", value, line);
            break;
        case DEVIATION_KEYWORD:
            ret = yang_check_string(module, &((struct lys_deviation *) node)->ref, "reference", "deviation", value, line);
            break;
        }
    }
    return ret;
}

void *
yang_read_revision(struct lys_module *module, char *value)
{
    struct lys_revision *retval;

    retval = &module->rev[module->rev_size];

    /* first member of array is last revision */
    if (module->rev_size && strcmp(module->rev[0].date, value) < 0) {
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
    module->rev_size++;
    free(value);
    return retval;
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

void *
yang_read_feature(struct lys_module *module, char *value, int line)
{
    struct lys_feature *retval;

    /* check uniqueness of feature's names */
    if (lyp_check_identifier(value, LY_IDENT_FEATURE, line, module, NULL)) {
        goto error;
    }
    retval = &module->features[module->features_size];
    retval->name = lydict_insert_zc(module->ctx, value);
    retval->module = module;
    module->features_size++;
    return retval;

error:
    free(value);
    return NULL;
}

int
yang_read_if_feature(struct lys_module *module, void *ptr, char *value, struct unres_schema *unres, int type, int line)
{
    const char *exp;
    int ret;
    struct lys_feature *f;
    struct lys_node *n;

    if (!(exp = transform_schema2json(module, value, line))) {
        free(value);
        return EXIT_FAILURE;
    }
    free(value);

    /* hack - store pointer to the parent node for later status check */
    if (type == FEATURE_KEYWORD) {
        f = (struct lys_feature *) ptr;
        f->features[f->features_size] = f;
        ret = unres_schema_add_str(module, unres, &f->features[f->features_size], UNRES_IFFEAT, exp, line);
        f->features_size++;
    } else {
        n = (struct lys_node *) ptr;
        n->features[n->features_size] = (struct lys_feature *) n;
        ret = unres_schema_add_str(module, unres, &n->features[n->features_size], UNRES_IFFEAT, exp, line);
        n->features_size++;
    }

    lydict_remove(module->ctx, exp);
    if (ret == -1) {

        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int 
yang_check_flags(uint8_t *flags, uint8_t mask, char *what, char *where, int value, int line)
{
    if (*flags & mask) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, what, where);
        return EXIT_FAILURE;
    } else {
        *flags |= value;
        return EXIT_SUCCESS;
    }
}

int
yang_read_status(void *node, int value, int type, int line)
{
    int retval;

    switch (type) {
    case FEATURE_KEYWORD:
        retval = yang_check_flags(&((struct lys_feature *) node)->flags, LYS_STATUS_MASK, "status", "feature", value, line);
        break;
    case IDENTITY_KEYWORD:
        retval = yang_check_flags(&((struct lys_ident *) node)->flags, LYS_STATUS_MASK, "status", "identity", value, line);
        break;
    case CONTAINER_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_container *) node)->flags, LYS_STATUS_MASK, "status", "container", value, line);
        break;
    case ANYXML_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_anyxml *) node)->flags, LYS_STATUS_MASK, "status", "anyxml", value, line);
        break;
    case CHOICE_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_anyxml *) node)->flags, LYS_STATUS_MASK, "status", "choice", value, line);
        break;
    case CASE_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_case *) node)->flags, LYS_STATUS_MASK, "status", "case", value, line);
        break;
    case GROUPING_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_grp *) node)->flags, LYS_STATUS_MASK, "status", "grouping", value, line);
        break;
    case LEAF_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_leaf *) node)->flags, LYS_STATUS_MASK, "status", "leaf", value, line);
        break;
    case LEAF_LIST_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_leaflist *) node)->flags, LYS_STATUS_MASK, "status", "leaflist", value, line);
        break;
    case LIST_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_list *) node)->flags, LYS_STATUS_MASK, "status", "list", value, line);
        break;
    case ENUM_KEYWORD:
        retval = yang_check_flags(&((struct lys_type_enum *) node)->flags, LYS_STATUS_MASK, "status", "enum", value, line);
        break;
    case BIT_KEYWORD:
        retval = yang_check_flags(&((struct lys_type_bit *) node)->flags, LYS_STATUS_MASK, "status", "bit", value, line);
        break;
    case TYPEDEF_KEYWORD:
        retval = yang_check_flags(&((struct lys_tpdf *) node)->flags, LYS_STATUS_MASK, "status", "typedef", value, line);
        break;
    case USES_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_uses *) node)->flags, LYS_STATUS_MASK, "status", "uses", value, line);
        break;
    case AUGMENT_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_augment *) node)->flags, LYS_STATUS_MASK, "status", "augment", value, line);
        break;
    case RPC_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_rpc *) node)->flags, LYS_STATUS_MASK, "status", "rpc", value, line);
        break;
    case NOTIFICATION_KEYWORD:
        retval = yang_check_flags(&((struct lys_node_notif *) node)->flags, LYS_STATUS_MASK, "status", "notification", value, line);
        break;
    }
    return retval;
}

void *
yang_read_identity(struct lys_module *module, char *value)
{
    struct lys_ident *ret;

    ret = &module->ident[module->ident_size];
    ret->name = lydict_insert_zc(module->ctx, value);
    ret->module = module;
    module->ident_size++;
    return ret;
}

int
yang_read_base(struct lys_module *module, struct lys_ident *ident, char *value, struct unres_schema *unres, int line)
{
    const char *exp;

    if (ident->base) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, "base", "identity");
        return EXIT_FAILURE;
    }
    exp = transform_schema2json(module, value, line);
    free(value);
    if (!exp) {
        return EXIT_FAILURE;
    }
    if (unres_schema_add_str(module, unres, ident, UNRES_IDENT, exp, line) == -1) {
        lydict_remove(module->ctx, exp);
        return EXIT_FAILURE;
    }

    /* hack - store some address due to detection of unresolved base*/
    if (!ident->base) {
        ident->base = (void *)1;
    }

    lydict_remove(module->ctx, exp);
    return EXIT_SUCCESS;
}

void *
yang_read_must(struct lys_module *module, struct lys_node *node, char *value, int type, int line)
{
    struct lys_restr *retval;

    switch (type) {
    case CONTAINER_KEYWORD:
        retval = &((struct lys_node_container *)node)->must[((struct lys_node_container *)node)->must_size++];
        break;
    case ANYXML_KEYWORD:
        retval = &((struct lys_node_anyxml *)node)->must[((struct lys_node_anyxml *)node)->must_size++];
        break;
    case LEAF_KEYWORD:
        retval = &((struct lys_node_leaf *)node)->must[((struct lys_node_leaf *)node)->must_size++];
        break;
    case LEAF_LIST_KEYWORD:
        retval = &((struct lys_node_leaflist *)node)->must[((struct lys_node_leaflist *)node)->must_size++];
        break;
    case LIST_KEYWORD:
        retval = &((struct lys_node_list *)node)->must[((struct lys_node_list *)node)->must_size++];
        break;
    case REFINE_KEYWORD:
        retval = &((struct lys_refine *)node)->must[((struct lys_refine *)node)->must_size++];
        break;
    case ADD_KEYWORD:
        retval = &(*((struct type_deviation *)node)->trg_must)[(*((struct type_deviation *)node)->trg_must_size)++];
        memset(retval, 0, sizeof *retval);
        break;
    case DELETE_KEYWORD:
        retval = &((struct type_deviation *)node)->deviate->must[((struct type_deviation *)node)->deviate->must_size++];
        break;
    }
    retval->expr = transform_schema2json(module, value, line);
    if (!retval->expr || lyxp_syntax_check(retval->expr, line)) {
        goto error;
    }
    free(value);
    return retval;

error:
    free(value);
    return NULL;
}

int
yang_read_message(struct lys_module *module,struct lys_restr *save,char *value, int type, int message, int line)
{
    int ret;
    char *exp;

    switch (type) {
    case MUST_KEYWORD:
        exp = "must";
        break;
    case LENGTH_KEYWORD:
        exp = "length";
        break;
    case PATTERN_KEYWORD:
        exp = "pattern";
        break;
    case RANGE_KEYWORD:
        exp = "range";
        break;
    }
    if (message==ERROR_APP_TAG_KEYWORD) {
        ret = yang_check_string(module, &save->eapptag, "error_app_tag", exp, value, line);
    } else {
        ret = yang_check_string(module, &save->emsg, "error_app_tag", exp, value, line);
    }
    return ret;
}

int
yang_read_presence(struct lys_module *module, struct lys_node_container *cont, char *value, int line)
{
    if (cont->presence) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, cont, "presence", "container");
        free(value);
        return EXIT_FAILURE;
    } else {
        cont->presence = lydict_insert_zc(module->ctx, value);
        return EXIT_SUCCESS;
    }
}

int
yang_read_config(void *node, int value, int type, int line)
{
    int ret;

    switch (type) {
    case CONTAINER_KEYWORD:
        ret = yang_check_flags(&((struct lys_node_container *)node)->flags, LYS_CONFIG_MASK, "config", "container", value, line);
        break;
    case ANYXML_KEYWORD:
        ret = yang_check_flags(&((struct lys_node_anyxml *)node)->flags, LYS_CONFIG_MASK, "config", "anyxml", value, line);
        break;
    case CHOICE_KEYWORD:
        ret = yang_check_flags(&((struct lys_node_choice *)node)->flags, LYS_CONFIG_MASK, "config", "choice", value, line);
        break;
    case LEAF_KEYWORD:
        ret = yang_check_flags(&((struct lys_node_leaf *)node)->flags, LYS_CONFIG_MASK, "config", "leaf", value, line);
        break;
    case LEAF_LIST_KEYWORD:
        ret = yang_check_flags(&((struct lys_node_leaflist *)node)->flags, LYS_CONFIG_MASK, "config", "leaflist", value, line);
        break;
    case LIST_KEYWORD:
        ret = yang_check_flags(&((struct lys_node_list *)node)->flags, LYS_CONFIG_MASK, "config", "list", value, line);
        break;
    case REFINE_KEYWORD:
        ret = yang_check_flags(&((struct lys_refine *)node)->flags, LYS_CONFIG_MASK, "config", "refine", value, line);
        break;
    }
    return ret;
}

void *
yang_read_when(struct lys_module *module, struct lys_node *node, int type, char *value, int line)
{
    struct lys_when *retval;

    retval = calloc(1, sizeof *retval);
    if (!retval) {
        LOGMEM;
        free(value);
        return NULL;
    }
    retval->cond = transform_schema2json(module, value, line);
    if (!retval->cond || lyxp_syntax_check(retval->cond, line)) {
        goto error;
    }
    switch (type) {
    case CONTAINER_KEYWORD:
        if (((struct lys_node_container *)node)->when) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, node, "when", "container");
            goto error;
        }
        ((struct lys_node_container *)node)->when = retval;
        break;
    case ANYXML_KEYWORD:
        if (((struct lys_node_anyxml *)node)->when) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, node, "when", "anyxml");
            goto error;
        }
        ((struct lys_node_anyxml *)node)->when = retval;
        break;
    case CHOICE_KEYWORD:
        if (((struct lys_node_choice *)node)->when) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, node, "when", "choice");
            goto error;
        }
        ((struct lys_node_choice *)node)->when = retval;
        break;
    case CASE_KEYWORD:
        if (((struct lys_node_case *)node)->when) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, node, "when", "case");
            goto error;
        }
        ((struct lys_node_case *)node)->when = retval;
        break;
    case LEAF_KEYWORD:
        if (((struct lys_node_leaf *)node)->when) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, node, "when", "leaf");
            goto error;
        }
        ((struct lys_node_leaf *)node)->when = retval;
        break;
    case LEAF_LIST_KEYWORD:
        if (((struct lys_node_leaflist *)node)->when) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, node, "when", "leaflist");
            goto error;
        }
        ((struct lys_node_leaflist *)node)->when = retval;
        break;
    case LIST_KEYWORD:
        if (((struct lys_node_list *)node)->when) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, node, "when", "list");
            goto error;
        }
        ((struct lys_node_list *)node)->when = retval;
        break;
    case USES_KEYWORD:
        if (((struct lys_node_uses *)node)->when) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, node, "when", "uses");
            goto error;
        }
        ((struct lys_node_uses *)node)->when = retval;
        break;
    case AUGMENT_KEYWORD:
        if (((struct lys_node_augment *)node)->when) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYS, node, "when", "augment");
            goto error;
        }
        ((struct lys_node_augment *)node)->when = retval;
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
yang_read_node(struct lys_module *module, struct lys_node *parent, char *value, int nodetype, int sizeof_struct)
{
    struct lys_node *node;

    node = calloc(1, sizeof_struct);
    if (!node) {
        LOGMEM;
        return NULL;
    }
    if (value) {
        node->name = lydict_insert_zc(module->ctx, value);
    }
    node->module = module;
    node->nodetype = nodetype;
    node->prev = node;

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, module->type ? ((struct lys_submodule *)module)->belongsto: module, node)) {
        if (value) {
            lydict_remove(module->ctx, node->name);
        }
        free(node);
        return NULL;
    }
    return node;
}

int
yang_read_mandatory(void *node, int value, int type, int line)
{
    int ret;

    switch (type) {
    case ANYXML_KEYWORD:
        ret = yang_check_flags(&((struct lys_node_anyxml *)node)->flags, LYS_MAND_MASK, "mandatory", "anyxml", value, line);
        break;
    case CHOICE_KEYWORD:
        ret = yang_check_flags(&((struct lys_node_choice *)node)->flags, LYS_MAND_MASK, "mandatory", "choice", value, line);
        break;
    case LEAF_KEYWORD:
        ret = yang_check_flags(&((struct lys_node_leaf *)node)->flags, LYS_MAND_MASK, "mandatory", "leaf", value, line);
        break;
    case REFINE_KEYWORD:
        ret = yang_check_flags(&((struct lys_refine *)node)->flags, LYS_MAND_MASK, "mandatory", "refine", value, line);
        break;
    }
    return ret;
}

int
yang_read_default(struct lys_module *module, void *node, char *value, int type, int line)
{
    int ret;

    switch (type) {
    case LEAF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_node_leaf *) node)->dflt, "default", "leaf", value, line);
        break;
    case TYPEDEF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_tpdf *) node)->dflt, "default", "typedef", value, line);
        break;
    }
    return ret;
}

int
yang_read_units(struct lys_module *module, void *node, char *value, int type, int line)
{
    int ret;

    switch (type) {
    case LEAF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_node_leaf *) node)->units, "units", "leaf", value, line);
        break;
    case LEAF_LIST_KEYWORD:
        ret = yang_check_string(module, &((struct lys_node_leaflist *) node)->units, "units", "leaflist", value, line);
        break;
    case TYPEDEF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_tpdf *) node)->units, "units", "typedef", value, line);
        break;
    }
    return ret;
}

int
yang_read_key(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres, int line)
{
    char *exp, *value;

    exp = value = (char *) list->keys;
    while ((value = strpbrk(value, " \t\n"))) {
        list->keys_size++;
        while (isspace(*value)) {
            value++;
        }
    }
    list->keys_size++;
    list->keys = calloc(list->keys_size, sizeof *list->keys);
    if (!list->keys) {
        LOGMEM;
        goto error;
    }
    if (unres_schema_add_str(module, unres, list, UNRES_LIST_KEYS, exp, line) == -1) {
        goto error;
    }
    free(exp);
    return EXIT_SUCCESS;

error:
    free(exp);
    return EXIT_FAILURE;
}

int
yang_fill_unique(struct lys_module *module, struct lys_node_list *list, struct lys_unique *unique, char *value, struct unres_schema *unres, int line)
{
    int i, j;
    char *vaux;

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
                LOGVAL(LYE_INARG, line, LY_VLOG_LYS, list, unique->expr[i], "unique");
                LOGVAL(LYE_SPEC, 0, 0, NULL, "The identifier is not unique");
                goto error;
            }
        }
        /* try to resolve leaf */
        if (unres) {
            unres_schema_add_str(module, unres, (struct lys_node *)list, UNRES_LIST_UNIQ, unique->expr[i], line);
        } else {
            if (resolve_unique((struct lys_node *)list, unique->expr[i], 0, line)) {
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
    struct type_ident *ident;

    for(k=0; k<list->unique_size; k++) {
        ident = (struct type_ident *)list->unique[k].expr;
        if (yang_fill_unique(module, list, &list->unique[k], ident->s, unres, ident->line)) {
            goto error;
        }
        free(ident);
    }
    return EXIT_SUCCESS;

error:
    free(ident);
    return EXIT_FAILURE;
}

static int
yang_read_identyref(struct lys_module *module, struct lys_type *type, struct unres_schema *unres, int line)
{
    const char *value, *tmp;
    int rc, ret = EXIT_FAILURE;

    value = tmp = type->info.lref.path;
    /* store in the JSON format */
    value = transform_schema2json(module, value, line);
    if (!value) {
        goto end;
    }
    rc = unres_schema_add_str(module, unres, type, UNRES_TYPE_IDENTREF, value, line);
    lydict_remove(module->ctx, value);

    if (rc == -1) {
        goto end;
    }

    ret = EXIT_SUCCESS;

end:
    lydict_remove(module->ctx, tmp);
    return ret;
}

int
yang_check_type(struct lys_module *module, struct lys_node *parent, struct yang_type *typ, struct unres_schema *unres)
{
    int i, rc;
    int ret = -1;
    const char *name, *value;
    LY_DATA_TYPE base;
    struct yang_type *typ_tmp;

    base = typ->type->base;
    value = transform_schema2json(module, typ->name, typ->line);
    if (!value) {
        goto error;
    }

    i = parse_identifier(value);
    if (i < 1) {
        LOGVAL(LYE_INCHAR, typ->line, LY_VLOG_NONE, NULL, value[-i], &value[-i]);
        lydict_remove(module->ctx, value);
        goto error;
    }
    /* module name*/
    name = value;
    if (value[i]) {
        typ->type->module_name = lydict_insert(module->ctx, value, i);
        name += i;
        if ((name[0] != ':') || (parse_identifier(name + 1) < 1)) {
            LOGVAL(LYE_INCHAR, typ->line, LY_VLOG_NONE, NULL, name[0], name);
            lydict_remove(module->ctx, value);
            goto error;
        }
        ++name;
    }

    rc = resolve_superior_type(name, typ->type->module_name, module, parent, &typ->type->der);
    lydict_remove(module->ctx, value);
    if (rc == -1) {
        LOGVAL(LYE_INMOD, typ->line, LY_VLOG_NONE, NULL, typ->type->module_name);
        goto error;

    /* the type could not be resolved or it was resolved to an unresolved typedef*/
    } else if (rc == EXIT_FAILURE) {
        ret = EXIT_FAILURE;
        goto error;
    }
    typ->type->base = typ->type->der->type.base;
    if (base == 0) {
        base = typ->type->der->type.base;
    }
    switch (base) {
    case LY_TYPE_STRING:
        if (typ->type->base == LY_TYPE_BINARY) {
            if (typ->type->info.str.pat_count) {
                LOGVAL(LYE_SPEC, typ->line, LY_VLOG_NONE, NULL, "Binary type could not include pattern statement.");
                typ->type->base = base;
                goto error;
            }
            typ->type->info.binary.length = typ->type->info.str.length;
            if (typ->type->info.binary.length && lyp_check_length_range(typ->type->info.binary.length->expr, typ->type)) {
                LOGVAL(LYE_INARG, typ->line, LY_VLOG_NONE, NULL, typ->type->info.binary.length->expr, "length");
                goto error;
            }
        } else if (typ->type->base == LY_TYPE_STRING) {
            if (typ->type->info.str.length && lyp_check_length_range(typ->type->info.str.length->expr, typ->type)) {
                LOGVAL(LYE_INARG, typ->line, LY_VLOG_NONE, NULL, typ->type->info.str.length->expr, "length");
                goto error;
            }
        } else {
            LOGVAL(LYE_SPEC, typ->line, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", parent->name);
            goto error;
        }
        break;
    case LY_TYPE_DEC64:
        if (typ->type->base == LY_TYPE_DEC64) {
            if (typ->type->info.dec64.range && lyp_check_length_range(typ->type->info.dec64.range->expr, typ->type)) {
                LOGVAL(LYE_INARG, typ->line, LY_VLOG_NONE, NULL, typ->type->info.dec64.range->expr, "range");
                goto error;
            }
            /* mandatory sub-statement(s) check */
            if (!typ->type->info.dec64.dig && !typ->type->der->type.der) {
                /* decimal64 type directly derived from built-in type requires fraction-digits */
                LOGVAL(LYE_MISSSTMT2, typ->line, LY_VLOG_NONE, NULL, "fraction-digits", "type");
                goto error;
            }
            if (typ->type->info.dec64.dig && typ->type->der->type.der) {
                /* type is not directly derived from buit-in type and fraction-digits statement is prohibited */
                LOGVAL(LYE_INSTMT, typ->line, LY_VLOG_NONE, NULL, "fraction-digits");
                goto error;
            }
        } else if (typ->type->base >= LY_TYPE_INT8 && typ->type->base <=LY_TYPE_UINT64) {
            if (typ->type->info.dec64.dig) {
                LOGVAL(LYE_SPEC, typ->line, LY_VLOG_NONE, NULL, "Numerical type could not include fraction statement.");
                typ->type->base = base;
                goto error;
            }
            typ->type->info.num.range = typ->type->info.dec64.range;
            if (typ->type->info.num.range && lyp_check_length_range(typ->type->info.num.range->expr, typ->type)) {
                LOGVAL(LYE_INARG, typ->line, LY_VLOG_NONE, NULL, typ->type->info.num.range->expr, "range");
                goto error;
            }
        } else {
            LOGVAL(LYE_SPEC, typ->line, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", parent->name);
            goto error;
        }
        break;
    case LY_TYPE_ENUM:
        if (typ->type->base != LY_TYPE_ENUM) {
            LOGVAL(LYE_SPEC, typ->line, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", parent->name);
            goto error;
        }
        if (!typ->type->der->type.der && !typ->type->info.enums.count) {
            /* type is derived directly from buit-in enumeartion type and enum statement is required */
            LOGVAL(LYE_MISSSTMT2, typ->line, LY_VLOG_NONE, NULL, "enum", "type");
            goto error;
        }
        if (typ->type->der->type.der && typ->type->info.enums.count) {
            /* type is not directly derived from buit-in enumeration type and enum statement is prohibited */
            LOGVAL(LYE_INSTMT, typ->line, LY_VLOG_NONE, NULL, "enum");
            goto error;
        }
        break;
    case LY_TYPE_LEAFREF:
        if (typ->type->base == LY_TYPE_IDENT && typ->flags & LYS_TYPE_BASE) {
            if (yang_read_identyref(module, typ->type, unres, typ->line)) {
                goto error;
            }
        } else if (typ->type->base == LY_TYPE_LEAFREF) {
            if (typ->type->info.lref.path && !typ->type->der->type.der) {
                value = typ->type->info.lref.path;
                /* store in the JSON format */
                typ->type->info.lref.path = transform_schema2json(module, value, typ->line);
                lydict_remove(module->ctx, value);
                if (!typ->type->info.lref.path) {
                    goto error;
                }
                if (unres_schema_add_node(module, unres, typ->type, UNRES_TYPE_LEAFREF, parent, typ->line) == -1) {
                    goto error;
                }
            } else if (!typ->type->info.lref.path && !typ->type->der->type.der) {
                LOGVAL(LYE_MISSSTMT2, typ->line, LY_VLOG_NONE, NULL, "path", "type");
                goto error;
            } else {
                LOGVAL(LYE_INSTMT, typ->line, LY_VLOG_NONE, NULL, "path");
                goto error;
            }
        } else {
            LOGVAL(LYE_SPEC, typ->line, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", parent->name);
            goto error;
        }
        break;
    case LY_TYPE_IDENT:
        if (typ->type->der->type.der) {
            /* this is just a derived type with no base specified/required */
            break;
        } else {
            LOGVAL(LYE_MISSSTMT2, typ->line, LY_VLOG_NONE, NULL, "base", "type");
            goto error;
        }
        break;
    case LY_TYPE_UNION:
        if (typ->type->base != LY_TYPE_UNION) {
            typ->type->base = LY_TYPE_UNION;
            LOGVAL(LYE_SPEC, typ->line, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", parent->name);
            goto error;
        }
        if (!typ->type->info.uni.types) {
            if (typ->type->der->type.der) {
                /* this is just a derived type with no additional type specified/required */
                break;
            }
            LOGVAL(LYE_MISSSTMT2, typ->line, LY_VLOG_NONE, NULL, "type", "(union) type");
            goto error;
        }
        for (i = 0; i < typ->type->info.uni.count; i++) {
            typ_tmp = (struct yang_type *)typ->type->info.uni.types[i].der;
            if (unres_schema_add_node(module, unres, &typ->type->info.uni.types[i], UNRES_TYPE_DER, parent, typ_tmp->line)) {
                goto error;
            }
            if (typ->type->info.uni.types[i].base == LY_TYPE_EMPTY) {
                LOGVAL(LYE_INARG, typ->line, LY_VLOG_NONE, NULL, "empty", typ->name);
                goto error;
            } else if (typ->type->info.uni.types[i].base == LY_TYPE_LEAFREF) {
                LOGVAL(LYE_INARG, typ->line, LY_VLOG_NONE, NULL, "leafref", typ->name);
                goto error;
            }
        }
        break;

    default:
        if (base >= LY_TYPE_BINARY && base <= LY_TYPE_UINT64) {
            if (typ->type->base != base) {
                LOGVAL(LYE_SPEC, typ->line, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", parent->name);
                goto error;
            }
        } else {
            LOGINT;
            goto error;
        }
    }
    return EXIT_SUCCESS;

error:
    typ->type->base = base;
    if (typ->type->module_name) {
        lydict_remove(module->ctx, typ->type->module_name);
        typ->type->module_name = NULL;
    }
    return ret;
}

void *
yang_read_type(struct lys_module *module, void *parent, char *value, int type, int line)
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
        ((struct lys_node_leaf *)parent)->type.der = (struct lys_tpdf *)typ;
        ((struct lys_node_leaf *)parent)->type.parent = (struct lys_tpdf *)parent;
        typ->type = &((struct lys_node_leaf *)parent)->type;
        break;
    case LEAF_LIST_KEYWORD:
        ((struct lys_node_leaflist *)parent)->type.der = (struct lys_tpdf *)typ;
        ((struct lys_node_leaflist *)parent)->type.parent = (struct lys_tpdf *)parent;
        typ->type = &((struct lys_node_leaflist *)parent)->type;
        break;
    case UNION_KEYWORD:
        ((struct lys_type *)parent)->der = (struct lys_tpdf *)typ;
        typ->type = (struct lys_type *)parent;
        break;
    case TYPEDEF_KEYWORD:
        ((struct lys_tpdf *)parent)->type.der = (struct lys_tpdf *)typ;
        typ->type = &((struct lys_tpdf *)parent)->type;
        break;
    case REPLACE_KEYWORD:
        /* deviation replace type*/
        dev = (struct type_deviation *)parent;
        if (dev->deviate->type) {
            LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, "type", "deviation");
            goto error;
        }
        /* check target node type */
        if (dev->target->nodetype == LYS_LEAF) {
            typ->type = &((struct lys_node_leaf *)dev->target)->type;
        } else if (dev->target->nodetype == LYS_LEAFLIST) {
            typ->type = &((struct lys_node_leaflist *)dev->target)->type;
        } else {
            LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "type");
            LOGVAL(LYE_SPEC, 0, 0, NULL, "Target node does not allow \"type\" property.");
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
    }
    typ->name = value;
    typ->line = line;
    return typ;

error:
    free(typ);
    return NULL;
}

void *
yang_read_length(struct lys_module *module, struct yang_type *typ, char *value, int line)
{
    struct lys_restr **length;

    if (typ->type->base == 0 || typ->type->base == LY_TYPE_STRING) {
        length = &typ->type->info.str.length;
        typ->type->base = LY_TYPE_STRING;
    } else if (typ->type->base == LY_TYPE_BINARY) {
        length = &typ->type->info.binary.length;
    } else {
        LOGVAL(LYE_SPEC, line, LY_VLOG_NONE, NULL, "Unexpected length statement.");
        goto error;
    }

    if (*length) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, "length", "type");
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

void *
yang_read_pattern(struct lys_module *module, struct yang_type *typ, char *value, int line)
{
    pcre *precomp;
    int err_offset;
    const char *err_ptr;

    /* check that the regex is valid */
    precomp = pcre_compile(value, PCRE_NO_AUTO_CAPTURE, &err_ptr, &err_offset, NULL);
    if (!precomp) {
        LOGVAL(LYE_INREGEX, line, LY_VLOG_NONE, NULL, value, err_ptr);
        free(value);
        return NULL;
    }
    free(precomp);

    typ->type->info.str.patterns[typ->type->info.str.pat_count].expr = lydict_insert_zc(module->ctx, value);
    typ->type->info.str.pat_count++;
    return &typ->type->info.str.patterns[typ->type->info.str.pat_count-1];
}

void *
yang_read_range(struct  lys_module *module, struct yang_type *typ, char *value, int line)
{
    if (typ->type->base != 0 && typ->type->base != LY_TYPE_DEC64) {
        LOGVAL(LYE_SPEC, line, LY_VLOG_NONE, NULL, "Unexpected range statement.");
        goto error;
    }
    typ->type->base = LY_TYPE_DEC64;
    if (typ->type->info.dec64.range) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, "range", "type");
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
yang_read_fraction(struct yang_type *typ, uint32_t value, int line)
{
    if (typ->type->base == 0 || typ->type->base == LY_TYPE_DEC64) {
        typ->type->base = LY_TYPE_DEC64;
    } else {
        LOGVAL(LYE_SPEC, line, LY_VLOG_NONE, NULL, "Unexpected fraction-digits statement.");
        goto error;
    }
    if (typ->type->info.dec64.dig) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, "fraction-digits", "type");
        goto error;
    }
    /* range check */
    if (value < 1 || value > 18) {
        LOGVAL(LYE_SPEC, line, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", value, "fraction-digits");
        goto error;
    }
    typ->type->info.dec64.dig = value;
    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

void *
yang_read_enum(struct lys_module *module, struct yang_type *typ, char *value, int line)
{
    struct lys_type_enum *enm;
    int i;

    enm = &typ->type->info.enums.enm[typ->type->info.enums.count];
    enm->name = lydict_insert_zc(module->ctx, value);

    /* the assigned name MUST NOT have any leading or trailing whitespace characters */
    if (isspace(enm->name[0]) || isspace(enm->name[strlen(enm->name) - 1])) {
        LOGVAL(LYE_ENUM_WS, line, LY_VLOG_NONE, NULL, enm->name);
        goto error;
    }

    /* check the name uniqueness */
    for (i = 0; i < typ->type->info.enums.count; i++) {
        if (!strcmp(typ->type->info.enums.enm[i].name, typ->type->info.enums.enm[typ->type->info.enums.count].name)) {
            LOGVAL(LYE_ENUM_DUPNAME, line, LY_VLOG_NONE, NULL, typ->type->info.enums.enm[i].name);
            goto error;
        }
    }

    typ->type->info.enums.count++;
    return enm;

error:
    typ->type->info.enums.count++;
    return NULL;
}

int
yang_check_enum(struct yang_type *typ, struct lys_type_enum *enm, int64_t *value, int assign, int line)
{
    int i, j;

    if (!assign) {
        /* assign value automatically */
        if (*value > INT32_MAX) {
            LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, "2147483648", "enum/value");
            goto error;
        }
        enm->value = *value;
        enm->flags |= LYS_AUTOASSIGNED;
        (*value)++;
    }

    /* check that the value is unique */
    j = typ->type->info.enums.count-1;
    for (i = 0; i < j; i++) {
        if (typ->type->info.enums.enm[i].value == typ->type->info.enums.enm[j].value) {
            LOGVAL(LYE_ENUM_DUPVAL, line, LY_VLOG_NONE, NULL,
                   typ->type->info.enums.enm[j].value, typ->type->info.enums.enm[j].name);
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

void *
yang_read_bit(struct lys_module *module, struct yang_type *typ, char *value, int line)
{
    int i;
    struct lys_type_bit *bit;

    bit = &typ->type->info.bits.bit[typ->type->info.bits.count];
    if (lyp_check_identifier(value, LY_IDENT_SIMPLE, line, NULL, NULL)) {
        LOGVAL(LYE_PATH, 0, LY_VLOG_NONE, NULL);
        free(value);
        goto error;
    }
    bit->name = lydict_insert_zc(module->ctx, value);

    /* check the name uniqueness */
    for (i = 0; i < typ->type->info.bits.count; i++) {
        if (!strcmp(typ->type->info.bits.bit[i].name, bit->name)) {
            LOGVAL(LYE_BITS_DUPNAME, line, LY_VLOG_NONE, NULL, bit->name);
            typ->type->info.bits.count++;
            goto error;
        }
    }
    typ->type->info.bits.count++;
    return bit;

error:
    return NULL;
}

int
yang_check_bit(struct yang_type *typ, struct lys_type_bit *bit, int64_t *value, int assign, int line)
{
    int i,j;
    struct lys_type_bit bit_tmp;

    if (!assign) {
        /* assign value automatically */
        if (*value > UINT32_MAX) {
            LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, "4294967295", "bit/position");
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
            LOGVAL(LYE_BITS_DUPVAL, line, LY_VLOG_NONE, NULL, bit->pos, bit->name);
            goto error;
        }
    }

    /* keep them ordered by position */
    while (j && typ->type->info.bits.bit[j - 1].pos > typ->type->info.bits.bit[j].pos) {
        /* switch them */
        memcpy(&bit_tmp, &typ->type->info.bits.bit[j], sizeof bit_tmp);
        memcpy(&typ->type->info.bits.bit[j], &typ->type->info.bits.bit[j - 1], sizeof bit_tmp);
        memcpy(&typ->type->info.bits.bit[j - 1], &bit_tmp, sizeof bit_tmp);
        j--;
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

void *
yang_read_typedef(struct lys_module *module, struct lys_node *parent, char *value, int line)
{
    struct lys_tpdf *ret;

    if (lyp_check_identifier(value, LY_IDENT_TYPE, line, module, parent)) {
        free(value);
        return NULL;
    }
    if (!parent) {
        ret = &module->tpdf[module->tpdf_size];
        ret->type.parent = NULL;
        module->tpdf_size++;
    } else {
        switch (parent->nodetype) {
        case LYS_GROUPING:
            ret = &((struct lys_node_grp *)parent)->tpdf[((struct lys_node_grp *)parent)->tpdf_size];
            ((struct lys_node_grp *)parent)->tpdf_size++;
            break;
        case LYS_CONTAINER:
            ret = &((struct lys_node_container *)parent)->tpdf[((struct lys_node_container *)parent)->tpdf_size];
            ((struct lys_node_container *)parent)->tpdf_size++;
            break;
        case LYS_LIST:
            ret = &((struct lys_node_list *)parent)->tpdf[((struct lys_node_list *)parent)->tpdf_size];
            ((struct lys_node_list *)parent)->tpdf_size++;
            break;
        case LYS_RPC:
            ret = &((struct lys_node_rpc *)parent)->tpdf[((struct lys_node_rpc *)parent)->tpdf_size];
            ((struct lys_node_rpc *)parent)->tpdf_size++;
            break;
        case LYS_INPUT:
        case LYS_OUTPUT:
            ret = &((struct lys_node_rpc_inout *)parent)->tpdf[((struct lys_node_rpc_inout *)parent)->tpdf_size];
            ((struct lys_node_rpc_inout *)parent)->tpdf_size++;
            break;
        case LYS_NOTIF:
            ret = &((struct lys_node_notif *)parent)->tpdf[((struct lys_node_notif *)parent)->tpdf_size];
            ((struct lys_node_notif *)parent)->tpdf_size++;
            break;
        }
        ret->type.parent = (struct lys_tpdf *)parent;
    }

    ret->name = lydict_insert_zc(module->ctx, value);
    ret->module = module;
    return ret;
}

void *
yang_read_refine(struct lys_module *module, struct lys_node_uses *uses, char *value, int line)
{
    struct lys_refine *rfn;

    rfn = &uses->refine[uses->refine_size];
    uses->refine_size++;
    rfn->target_name = transform_schema2json(module, value, line);
    free(value);
    if (!rfn->target_name) {
        return NULL;
    }
    return rfn;
}

void *
yang_read_augment(struct lys_module *module, struct lys_node *parent, char *value, int line)
{
    struct lys_node_augment *aug;
    uint16_t *size;

    if (parent) {
        aug = &((struct lys_node_uses *)parent)->augment[((struct lys_node_uses *)parent)->augment_size];
    } else {
        aug = &module->augment[module->augment_size];
    }
    aug->nodetype = LYS_AUGMENT;
    aug->target_name = transform_schema2json(module, value, line);
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
yang_read_deviation(struct lys_module *module, char *value, int line)
{
    struct lys_node *dev_target = NULL;
    struct lys_deviation *dev;
    struct type_deviation *deviation = NULL;
    int i, j, rc;

    deviation = calloc(1, sizeof *deviation);
    if (!deviation) {
        LOGMEM;
        goto error;
    }

    dev = &module->deviation[module->deviation_size];
    dev->target_name = transform_schema2json(module, value, line);
    free(value);
    if (!dev->target_name) {
        goto error;
    }

    /* resolve target node */
    rc = resolve_augment_schema_nodeid(dev->target_name, NULL, module, (const struct lys_node **)&dev_target);
    if (rc || !dev_target) {
        LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, dev->target_name, "deviations");
        goto error;
    }
    if (dev_target->module == lys_module(module)) {
        LOGVAL(LYE_SPEC, line, LY_VLOG_NONE, NULL, "Deviating own module is not allowed.");
        goto error;
    }
    /*save pointer to the deviation and deviated target*/
    deviation->deviation = dev;
    deviation->target = dev_target;

    /* mark the target module as deviated */
    dev_target->module->deviated = 1;

    /* copy our imports to the deviated module (deviations may need them to work) */
    for (i = 0; i < module->imp_size; ++i) {
        for (j = 0; j < dev_target->module->imp_size; ++j) {
            if (module->imp[i].module == dev_target->module->imp[j].module) {
                break;
            }
        }

        if (j < dev_target->module->imp_size) {
            /* import is already there */
            continue;
        }

        /* copy the import, mark it as external */
        ++dev_target->module->imp_size;
        dev_target->module->imp = ly_realloc(dev_target->module->imp, dev_target->module->imp_size * sizeof *dev_target->module->imp);
        if (!dev_target->module->imp) {
            LOGMEM;
            goto error;
        }
        dev_target->module->imp[dev_target->module->imp_size - 1].module = module->imp[i].module;
        dev_target->module->imp[dev_target->module->imp_size - 1].prefix = lydict_insert(module->ctx, module->imp[i].prefix, 0);
        memcpy(dev_target->module->imp[dev_target->module->imp_size - 1].rev, module->imp[i].rev, LY_REV_SIZE);
        dev_target->module->imp[dev_target->module->imp_size - 1].external = 1;
    }

    /* copy ourselves to the deviated module as a special import (if we haven't yet, there could be more deviations of the same module) */
    for (i = 0; i < dev_target->module->imp_size; ++i) {
        if (dev_target->module->imp[i].module == module) {
            break;
        }
    }

    if (i == dev_target->module->imp_size) {
        ++dev_target->module->imp_size;
        dev_target->module->imp = ly_realloc(dev_target->module->imp, dev_target->module->imp_size * sizeof *dev_target->module->imp);
        if (!dev_target->module->imp) {
            LOGMEM;
            goto error;
        }
        dev_target->module->imp[dev_target->module->imp_size - 1].module = module;
        dev_target->module->imp[dev_target->module->imp_size - 1].prefix = lydict_insert(module->ctx, module->prefix, 0);
        if (module->rev_size) {
            memcpy(dev_target->module->imp[dev_target->module->imp_size - 1].rev, module->rev[0].date, LY_REV_SIZE);
        } else {
            memset(dev_target->module->imp[dev_target->module->imp_size - 1].rev, 0, LY_REV_SIZE);
        }
        dev_target->module->imp[dev_target->module->imp_size - 1].external = 2;
    } else {
        /* it could have been added by another deviating module that imported this deviating module */
        dev_target->module->imp[i].external = 2;
    }


    return deviation;

error:
    free(deviation);
    lydict_remove(module->ctx, dev->target_name);
    return NULL;
}

int
yang_read_deviate_unsupported(struct type_deviation *dev, int line)
{
    int i;

    if (dev->deviation->deviate_size) {
        LOGVAL(LYE_SPEC, line, LY_VLOG_NONE, NULL, "\"not-supported\" deviation cannot be combined with any other deviation.");
        return EXIT_FAILURE;
    }
    dev->deviation->deviate[dev->deviation->deviate_size].mod = LY_DEVIATE_NO;

    /* you cannot remove a key leaf */
    if ((dev->target->nodetype == LYS_LEAF) && dev->target->parent && (dev->target->parent->nodetype == LYS_LIST)) {
        for (i = 0; i < ((struct lys_node_list *)dev->target->parent)->keys_size; ++i) {
            if (((struct lys_node_list *)dev->target->parent)->keys[i] == (struct lys_node_leaf *)dev->target) {
                LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, "not-supported", "deviation");
                LOGVAL(LYE_SPEC, 0, 0, NULL, "\"not-supported\" deviation cannot remove a list key.");
                return EXIT_FAILURE;
            }
        }
    }
 
    /* unlink and store the original node */
    dev->deviation->orig_node = dev->target;

    dev->deviation->deviate_size = 1;
    return EXIT_SUCCESS;
}

int
yang_read_deviate(struct type_deviation *dev, LYS_DEVIATE_TYPE mod, int line)
{
    struct unres_schema tmp_unres;

    dev->deviation->deviate[dev->deviation->deviate_size].mod = mod;
    dev->deviate = &dev->deviation->deviate[dev->deviation->deviate_size];
    dev->deviation->deviate_size++;
    if (dev->deviation->deviate[0].mod == LY_DEVIATE_NO) {
        LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "not-supported");
        LOGVAL(LYE_SPEC, 0, 0, NULL, "\"not-supported\" deviation cannot be combined with any other deviation.");
        return EXIT_FAILURE;
    }

    /* store a shallow copy of the original node */
    if (!dev->deviation->orig_node) {
        memset(&tmp_unres, 0, sizeof tmp_unres);
        dev->deviation->orig_node = lys_node_dup(dev->target->module, NULL, dev->target, 0, 0, &tmp_unres, 1);
        /* just to be safe */
        if (tmp_unres.count) {
            LOGINT;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int
yang_read_deviate_units(struct ly_ctx *ctx, struct type_deviation *dev, char *value, int line)
{
    const char **stritem;

    if (dev->deviate->units) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, "units", "deviate");
        free(value);
        goto error;
    }

    /* check target node type */
    if (dev->target->nodetype == LYS_LEAFLIST) {
        stritem = &((struct lys_node_leaflist *)dev->target)->units;
    } else if (dev->target->nodetype == LYS_LEAF) {
        stritem = &((struct lys_node_leaf *)dev->target)->units;
    } else {
        LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "units");
        LOGVAL(LYE_SPEC, 0, 0, NULL, "Target node does not allow \"units\" property.");
        free(value);
        goto error;
    }

    dev->deviate->units = lydict_insert_zc(ctx, value);

    if (dev->deviate->mod == LY_DEVIATE_DEL) {
        /* check values */
        if (*stritem != dev->deviate->units) {
            LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, dev->deviate->units, "units");
            LOGVAL(LYE_SPEC, 0, 0, NULL, "Value differs from the target being deleted.");
            goto error;
        }
        /* remove current units value of the target */
        lydict_remove(ctx, *stritem);
    } else {
        if (dev->deviate->mod == LY_DEVIATE_ADD) {
            /* check that there is no current value */
            if (*stritem) {
                LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "units");
                LOGVAL(LYE_SPEC, 0, 0, NULL, "Adding property that already exists.");
                goto error;
            }
        } else { /* replace */
            if (!*stritem) {
                LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "units");
                LOGVAL(LYE_SPEC, 0, 0, NULL, "Replacing a property that does not exist.");
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
yang_read_deviate_must(struct ly_ctx *ctx, struct type_deviation *dev, uint8_t c_must, int line)
{
    uint8_t i;

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
        dev->trg_must = &((struct lys_node_anyxml *)dev->target)->must;
        dev->trg_must_size = &((struct lys_node_anyxml *)dev->target)->must_size;
        break;
    default:
        LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "must");
        LOGVAL(LYE_SPEC, 0, 0, NULL, "Target node does not allow \"must\" property.");
        goto error;
    }

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
yang_read_deviate_unique(struct ly_ctx *ctx, struct type_deviation *dev, uint8_t c_uniq, int line)
{
    int i, j;
    struct lys_node_list *list;

    /* check target node type */
    if (dev->target->nodetype != LYS_LIST) {
        LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "unique");
        LOGVAL(LYE_SPEC, 0, 0, NULL, "Target node does not allow \"unique\" property.");
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
yang_read_deviate_default(struct ly_ctx *ctx, struct type_deviation *dev, char *value, int line)
{
    int rc;
    struct lys_node_choice *choice;
    struct lys_node_leaf *leaf;
    struct lys_node *node;

    if (dev->deviate->dflt) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, "default", "deviate");
        free(value);
        goto error;
    }

    dev->deviate->dflt = lydict_insert_zc(ctx, value);

    if (dev->target->nodetype == LYS_CHOICE) {
        choice = (struct lys_node_choice *)dev->target;

        rc = resolve_choice_default_schema_nodeid(dev->deviate->dflt, choice->child, (const struct lys_node **)&node);
        if (rc || !node) {
            LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, dev->deviate->dflt, "default");
            goto error;
        }
        if (dev->deviate->mod == LY_DEVIATE_DEL) {
            if (!choice->dflt || (choice->dflt != node)) {
                LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, dev->deviate->dflt, "default");
                LOGVAL(LYE_SPEC, 0, 0, NULL, "Value differs from the target being deleted.");
                goto error;
            }
            choice->dflt = NULL;
        } else {
            if (dev->deviate->mod == LY_DEVIATE_ADD) {
                /* check that there is no current value */
                if (choice->dflt) {
                    LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "default");
                    LOGVAL(LYE_SPEC, 0, 0, NULL, "Adding property that already exists.");
                    goto error;
                }
            } else { /* replace*/
                if (!choice->dflt) {
                    LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "default");
                    LOGVAL(LYE_SPEC, 0, 0, NULL, "Replacing a property that does not exist.");
                    goto error;
                }
            }

            choice->dflt = node;
            if (!choice->dflt) {
                /* default branch not found */
                LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, dev->deviate->dflt, "default");
                goto error;
            }
        }
    } else if (dev->target->nodetype == LYS_LEAF) {
        leaf = (struct lys_node_leaf *)dev->target;

        if (dev->deviate->mod == LY_DEVIATE_DEL) {
            if (!leaf->dflt || (leaf->dflt != dev->deviate->dflt)) {
                LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, dev->deviate->dflt, "default");
                LOGVAL(LYE_SPEC, 0, 0, NULL, "Value differs from the target being deleted.");
                goto error;
            }
            /* remove value */
            lydict_remove(ctx, leaf->dflt);
            leaf->dflt = NULL;
        } else {
            if (dev->deviate->mod == LY_DEVIATE_ADD) {
                /* check that there is no current value */
                if (leaf->dflt) {
                    LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "default");
                    LOGVAL(LYE_SPEC, 0, 0, NULL, "Adding property that already exists.");
                    goto error;
                }
            } else { /* replace*/
                if (!leaf->dflt) {
                    LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "default");
                    LOGVAL(LYE_SPEC, 0, 0, NULL, "Replacing a property that does not exist.");
                    goto error;
                }
            }
            /* remove value */
            lydict_remove(ctx, leaf->dflt);

            /* set new value */
            leaf->dflt = lydict_insert(ctx, dev->deviate->dflt, 0);
        }
    } else {
        /* invalid target for default value */
        LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "default");
        LOGVAL(LYE_SPEC, 0, 0, NULL, "Target node does not allow \"default\" property.");
        goto error;
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_config(struct type_deviation *dev, uint8_t value, int line)
{
    if (dev->deviate->flags & LYS_CONFIG_MASK) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, "config", "deviate");
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
yang_read_deviate_mandatory(struct type_deviation *dev, uint8_t value, int line)
{
    if (dev->deviate->flags & LYS_MAND_MASK) {
        LOGVAL(LYE_TOOMANY, line, LY_VLOG_NONE, NULL, "mandatory", "deviate");
        goto error;
    }

    /* check target node type */
    if (!(dev->target->nodetype & (LYS_LEAF | LYS_CHOICE | LYS_ANYXML))) {
        LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "mandatory");
        LOGVAL(LYE_SPEC, 0, 0, NULL, "Target node does not allow \"mandatory\" property.");
        goto error;
    }

    dev->deviate->flags |= value;

    if (dev->deviate->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if (dev->target->flags & LYS_MAND_MASK) {
            LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "mandatory");
            LOGVAL(LYE_SPEC, 0, 0, NULL, "Adding property that already exists.");
            goto error;
        }
    } else { /* replace */
        if (!(dev->target->flags & LYS_MAND_MASK)) {
            LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, "mandatory");
            LOGVAL(LYE_SPEC, 0, 0, NULL, "Replacing a property that does not exist.");
            goto error;
        }
    }

    /* remove current mandatory value of the target ... */
    dev->target->flags &= ~LYS_MAND_MASK;

    /* ... and replace it with the value specified in deviation */
    dev->target->flags |= dev->deviate->flags & LYS_MAND_MASK;

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_minmax(struct type_deviation *dev, uint32_t value, int type, int line)
{
    uint32_t *ui32val;

    /* check target node type */
    if (dev->target->nodetype == LYS_LEAFLIST) {
        if (type) {
            ui32val = &((struct lys_node_leaflist *)dev->target)->max;
        } else {
            ui32val = &((struct lys_node_leaflist *)dev->target)->min;
        }
    } else if (dev->target->nodetype == LYS_LIST) {
        if (type) {
            ui32val = &((struct lys_node_list *)dev->target)->max;
        } else {
            ui32val = &((struct lys_node_list *)dev->target)->min;
        }
    } else {
        LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, (type) ? "max-elements" : "min-elements");
        LOGVAL(LYE_SPEC, 0, 0, NULL, "Target node does not allow \"%s\" property.", (type) ? "max-elements" : "min-elements");
        goto error;
    }

    if (type) {
        dev->deviate->max = value;
        dev->deviate->max_set = 1;
    } else {
        dev->deviate->min = value;
        dev->deviate->min_set = 1;
    }

    if (dev->deviate->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if (*ui32val) {
            LOGVAL(LYE_INSTMT, line, LY_VLOG_NONE, NULL, (type) ? "max-elements" : "min-elements");
            LOGVAL(LYE_SPEC, 0, 0, NULL, "Adding property that already exists.");
            goto error;
        }
    } else if (dev->deviate->mod == LY_DEVIATE_RPL) {
        /* unfortunately, there is no way to check reliably that there
         * was a value before, it could have been the default */
    }

    /* add (already checked) and replace */
    /* set new value specified in deviation */
    *ui32val = value;

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_check_deviate_must(struct ly_ctx *ctx, struct type_deviation *dev, int line)
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
        LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, dev->deviate->must[dev->deviate->must_size - 1].expr, "must");
        LOGVAL(LYE_SPEC, 0, 0, NULL, "Value does not match any must from the target.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
yang_check_deviate_unique(struct lys_module *module, struct type_deviation *dev, char *value, int line)
{
    struct lys_node_list *list;
    int i, j;

    list = (struct lys_node_list *)dev->target;
    if (yang_fill_unique(module, list, &dev->deviate->unique[dev->deviate->unique_size], value, NULL, line)) {
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
        LOGVAL(LYE_INARG, line, LY_VLOG_NONE, NULL, value, "unique");
        LOGVAL(LYE_SPEC, 0, 0, NULL, "Value differs from the target being deleted.");
        goto error;
    }

    free(value);
    return EXIT_SUCCESS;

error:
    free(value);
    return EXIT_FAILURE;
}

int
yang_check_deviation(struct lys_module *module, struct type_deviation *dev, struct unres_schema *unres, int line)
{
    int i, rc;

    if (dev->target->nodetype == LYS_LEAF) {
        for(i = 0; i < dev->deviation->deviate_size; ++i) {
            if (dev->deviation->deviate[i].mod != LY_DEVIATE_DEL) {
                if (dev->deviation->deviate[i].dflt || dev->deviation->deviate[i].type) {
                    rc = unres_schema_add_str(module, unres, &((struct lys_node_leaf *)dev->target)->type, UNRES_TYPE_DFLT, ((struct lys_node_leaf *)dev->target)->dflt, 0);
                    if (rc == -1) {
                      return EXIT_FAILURE;
                    } else if (rc == EXIT_FAILURE) {
                        LOGVAL(LYE_SPEC, line, LY_VLOG_NONE, NULL, "Leaf \"%s\" default value no longer matches its type.", dev->deviation->target_name);
                        return EXIT_FAILURE;
                    }
                    break;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

int
yang_fill_include(struct lys_module *module, struct lys_submodule *submodule, char *value,
                  char *rev, int inc_size, struct unres_schema *unres, int line)
{
    struct lys_include inc;
    struct lys_module *trg;
    int i;

    trg = (submodule) ? (struct lys_module *)submodule : module;
    inc.submodule = NULL;
    inc.external = 0;
    memcpy(inc.rev, rev, LY_REV_SIZE);
    if (lyp_check_include(module, submodule, value, line, &inc, unres)) {
        goto error;
    }
    memcpy(&trg->inc[inc_size], &inc, sizeof inc);

    /* check duplications in include submodules */
    for (i = 0; i < inc_size; ++i) {
        if (trg->inc[i].submodule && !strcmp(trg->inc[i].submodule->name, trg->inc[inc_size].submodule->name)) {
            LOGVAL(LYE_SPEC, line, LY_VLOG_NONE, NULL, "Including submodule \"%s\" repeatedly.",
                trg->inc[i].submodule->name);
            goto error;
        }
    }
    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}
