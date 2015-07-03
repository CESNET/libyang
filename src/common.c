/**
 * @file common.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common libyang routines implementations
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

#include "common.h"
#include "libyang.h"

/*
 * libyang errno
 */
LY_ERR ly_errno = 0;

char *
strnchr(const char *s, int c, unsigned int len)
{
    for (; *s != (char)c; ++s, --len) {
        if ((*s == '\0') || (!len)) {
            return NULL;
        }
    }
    return (char *)s;
}

const char *
strnodetype(LY_NODE_TYPE type)
{
    switch (type) {
    case LY_NODE_AUGMENT:
        return "augment";
    case LY_NODE_CONTAINER:
        return "container";
    case LY_NODE_CHOICE:
        return "choice";
    case LY_NODE_LEAF:
        return "leaf";
    case LY_NODE_LEAFLIST:
        return "leaf-list";
    case LY_NODE_LIST:
        return "list";
    case LY_NODE_ANYXML:
        return "anyxml";
    case LY_NODE_USES:
        return "uses";
    case LY_NODE_GROUPING:
        return "grouping";
    case LY_NODE_CASE:
        return "case";
    case LY_NODE_INPUT:
        return "input";
    case LY_NODE_OUTPUT:
        return "output";
    case LY_NODE_NOTIF:
        return "notification";
    case LY_NODE_RPC:
        return "rpc";
    }

    return NULL;
}
