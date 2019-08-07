/**
 * @file printer_xml.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML printer for libyang data structure
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "plugins_types.h"
#include "printer_data.h"
#include "printer_internal.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"

/**
 * @brief JSON mapping of YANG modules to prefixes in values.
 *
 * Implementation of ly_clb_get_prefix.
 */
const char *
json_print_get_prefix(const struct lys_module *mod, void *UNUSED(private))
{
    return mod->name;
}

