/**
 * @file version.c
 * @author David Schweizer <dschweizer@netdef.org>
 * @brief Libyang version API calls
 *
 * Copyright (c) 2024 Network Device Education Foundation (NetDEF), Inc.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "version.h"

static struct ly_version _v_so = {
    .major = LY_VERSION_MAJOR,
    .minor = LY_VERSION_MINOR,
    .micro = LY_VERSION_MICRO,
    .str = LY_VERSION
};

static struct ly_version _v_proj = {
    .major = LY_PROJ_VERSION_MAJOR,
    .minor = LY_PROJ_VERSION_MINOR,
    .micro = LY_PROJ_VERSION_MICRO,
    .str = LY_PROJ_VERSION
};

LIBYANG_API_DEF struct ly_version
ly_get_so_version(void)
{
    return _v_so;
}

LIBYANG_API_DEF const char *
ly_get_so_version_str(void)
{
    return _v_so.str;
}

LIBYANG_API_DEF struct ly_version
ly_get_project_version(void)
{
    return _v_proj;
}

LIBYANG_API_DEF const char *
ly_get_project_version_str(void)
{
    return _v_proj.str;
}
