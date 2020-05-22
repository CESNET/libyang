/*
 * Copyright (c) 2018-2019 Robin Jarry
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <libyang/libyang.h>

static LY_ERR lypy_get_errno(void)
{
    return ly_errno;
}

static void lypy_set_errno(LY_ERR err)
{
    ly_errno = err;
}

static uint8_t lypy_module_implemented(const struct lys_module *module)
{
    if (module)
        return module->implemented;
    return 0;
}
