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

#include "parser_yang.h"
#include "parser_yang_bis.h"
#include "parser.h"
#include "xpath.h"

static int 
yang_check_string(struct lys_module *module, const char **target, char *what, char *where, char *value, int line)
{
    if (*target) {
        LOGVAL(LYE_TOOMANY, line, what, where);
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
            LOGVAL(LYE_SPEC, line, "Importing module \"%s\" repeatedly.", module->imp[i].module->name);
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
yang_add_elem(struct lys_node_array **node, int *size)
{
    if (!*size % LY_ARRAY_SIZE) {
        if (!(*node = ly_realloc(*node, (*size + LY_ARRAY_SIZE) * sizeof **node))) {
            LOGMEM;
            return EXIT_FAILURE;
        } else {
            memset(*node+*size,0,LY_ARRAY_SIZE*sizeof **node);
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
        LOGVAL(LYE_TOOMANY, line, what, where);
        return EXIT_FAILURE;
    } else {
        *flags = value;
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
        LOGVAL(LYE_TOOMANY, line, "base", "identity");
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
    lydict_remove(module->ctx, exp);
    return EXIT_SUCCESS;
}

void *
yang_read_cont(struct lys_module *module, struct lys_node *parent, char *value)
{
    struct lys_node_container *cont;

    cont = calloc(1, sizeof *cont);
    if (!cont) {
        LOGMEM;
        return NULL;
    }
    cont->module = module;
    cont->name = lydict_insert_zc(module->ctx, value);
    cont->nodetype = LYS_CONTAINER;
    cont->prev = (struct lys_node *)cont;
    if (lys_node_addchild(parent, module->type ? ((struct lys_submodule *)module)->belongsto: module, (struct lys_node *)cont)) {
        lydict_remove(module->ctx, cont->name);
        free(cont);
        return NULL;
    }
    return cont;
}

void *
yang_read_must(struct lys_module *module, struct lys_node *node, char *value, int type, int line)
{
    struct lys_restr *retval;

    switch (type) {
        case CONTAINER_KEYWORD:
            retval = &((struct lys_node_container *)node)->must[((struct lys_node_container *)node)->must_size];
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
        LOGVAL(LYE_TOOMANY, line, "presence", "container");
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
            LOGVAL(LYE_TOOMANY,line,"when","container");
            goto error;
        }
        ((struct lys_node_container *)node)->when = retval;
        break;
    }
    free(value);
    return retval;

error:
    free(value);
    lys_when_free(module->ctx, retval);
    return NULL;
}
