/**
 * @file printer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Wrapper for all libyang printers.
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

#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "tree.h"

/* printer/-.c */
int yang_print_model(FILE * f, struct ly_module *module);
int tree_print_model(FILE * f, struct ly_module *module);
int info_print_model(FILE * f, struct ly_module *module, const char *target_node);

int json_print_data(FILE *f, struct lyd_node *root);

API int
lys_print(FILE *f, struct ly_module *module, LY_MOUTFORMAT format, const char *target_node)
{
    if (!f || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    switch (format) {
    case LY_OUT_YIN:
        LOGERR(LY_EINVAL, "YIN output format not supported yet.");
        return EXIT_FAILURE;
    case LY_OUT_YANG:
        return yang_print_model(f, module);
    case LY_OUT_TREE:
        return tree_print_model(f, module);
    case LY_OUT_INFO:
        return info_print_model(f, module, target_node);
    default:
        LOGERR(LY_EINVAL, "Unknown output format.");
        return EXIT_FAILURE;
    }
}

API int
lyd_print(FILE * f, struct lyd_node *root, LY_DFORMAT format)
{
    if (!f || !root) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    switch (format) {
    case LY_DATA_XML:
        LOGERR(LY_EINVAL, "XML output format not supported yet.");
        return EXIT_FAILURE;
    case LY_DATA_JSON:
        return json_print_data(f, root);
    default:
        LOGERR(LY_EINVAL, "Unknown output format.");
        return EXIT_FAILURE;
    }
}
