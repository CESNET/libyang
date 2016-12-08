/**
 * @file extensions.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YANG extensions routines implementation
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "common.h"
#include "extensions.h"
#include "libyang.h"

API LYEXT_TYPE
lys_ext_instance_type(struct lys_ext_instance *ext)
{
    if (!ext) {
        return LYEXT_ERR;
    }

    if (!ext->def->plugin) {
        /* default extension type - LY_EXT_FLAG */
        return 0;
    } else {
        return ext->def->plugin->type;
    }
}
