/**
 * @file compat.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Compatibility functions - implemented basic functions which are not available on all the platforms.
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <string.h>
#include <unistd.h>

#ifndef HAVE_STRNSTR
char *
strnstr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0')
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}
#endif

#ifndef  HAVE_GET_CURRENT_DIR_NAME
char *
get_current_dir_name(void)
{
    char tmp[PATH_MAX];
    char *retval;

    if (getcwd(tmp, sizeof(tmp))) {
        retval = strdup(tmp);
        LY_CHECK_ERR_RET(!retval, LOGMEM(NULL), NULL);
        return retval;
    }
    return NULL;
}
#endif
