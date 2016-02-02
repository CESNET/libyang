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
    int ret=0;

    switch (type) {
    case MODULE_KEYWORD:
        module->name = lydict_insert_zc(module->ctx, value);
        break;
    case NAMESPACE_KEYWORD:
        ret = yang_check_string(module, &module->ns, "namespace", "module", value, line);
        break;
    }

    return ret;
}

int 
yang_read_prefix(struct lys_module *module, void *save, char *value, int type, int line) 
{
    int ret;

    switch (type){
    case MODULE_KEYWORD:
        ret = yang_check_string(module, &module->prefix, "prefix", "module", value, line);
        break;
    }
    
    return ret;
}
