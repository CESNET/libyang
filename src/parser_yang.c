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
