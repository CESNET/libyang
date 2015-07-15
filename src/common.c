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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

/* syntax is assumed to be correct */
struct len_ran_intv*
get_len_ran_interval(const char *str_restr, struct ly_type *type, int superior_restr)
{
    /* 0 - unsigned, 1 - signed, 2 - floating point */
    int kind, ret = 0;
    int64_t local_smin, local_smax;
    uint64_t local_umin, local_umax;
    long double local_fmin, local_fmax;
    const char *seg_ptr, *ptr;
    struct len_ran_intv *local_intv = NULL, *tmp_local_intv, *tmp_intv, *intv = NULL;

    switch (type->base) {
    case LY_TYPE_BINARY:
        kind = 0;
        local_umin = 0;
        local_umax = 18446744073709551615UL;

        if (!str_restr && type->info.binary.length) {
            str_restr = type->info.binary.length->expr;
        }
        break;
    case LY_TYPE_DEC64:
        kind = 2;
        local_fmin = -9223372036854775808.0;
        local_fmin /= 1 << type->info.dec64.dig;
        local_fmax = 9223372036854775807.0;
        local_fmax /= 1 << type->info.dec64.dig;

        if (!str_restr && type->info.dec64.range) {
            str_restr = type->info.dec64.range->expr;
        }
        break;
    case LY_TYPE_INT8:
        kind = 1;
        local_smin = -128;
        local_smax = 127;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT16:
        kind = 1;
        local_smin = -32768;
        local_smax = 32767;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT32:
        kind = 1;
        local_smin = -2147483648;
        local_smax = 2147483647;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT64:
        kind = 1;
        local_smin = -9223372036854775807L - 1L;
        local_smax = 9223372036854775807L;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT8:
        kind = 0;
        local_umin = 0;
        local_umax = 255;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT16:
        kind = 0;
        local_umin = 0;
        local_umax = 65535;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT32:
        kind = 0;
        local_umin = 0;
        local_umax = 4294967295;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT64:
        kind = 0;
        local_umin = 0;
        local_umax = 18446744073709551615UL;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_STRING:
        kind = 0;
        local_umin = 0;
        local_umax = 18446744073709551615UL;

        if (!str_restr && type->info.str.length) {
            str_restr = type->info.str.length->expr;
        }
        break;
    default:
        return NULL;
    }

    /* process superior types */
    if (superior_restr && type->der) {
        intv = get_len_ran_interval(NULL, &type->der->type, superior_restr);
        assert(!intv || (intv->kind == kind));
    }

    if (!str_restr) {
        return intv;
    }

    /* adjust local min and max */
    if (intv) {
        tmp_intv = intv;

        if (kind == 0) {
            local_umin = tmp_intv->value.uval.min;
        } else if (kind == 1) {
            local_smin = tmp_intv->value.sval.min;
        } else if (kind == 2) {
            local_fmin = tmp_intv->value.fval.min;
        }

        while (tmp_intv->next) {
            tmp_intv = tmp_intv->next;
        }

        if (kind == 0) {
            local_umax = tmp_intv->value.uval.max;
        } else if (kind == 1) {
            local_smax = tmp_intv->value.sval.max;
        } else if (kind == 2) {
            local_fmax = tmp_intv->value.fval.max;
        }
    }

    /* finally parse our restriction */
    seg_ptr = str_restr;
    while (1) {
        if (!local_intv) {
            local_intv = malloc(sizeof *local_intv);
            tmp_local_intv = local_intv;
        } else {
            tmp_local_intv->next = malloc(sizeof *local_intv);
            tmp_local_intv = tmp_local_intv->next;
        }

        tmp_local_intv->kind = kind;
        tmp_local_intv->next = NULL;

        /* min */
        ptr = seg_ptr;
        while (isspace(ptr[0])) {
            ++ptr;
        }
        if (isdigit(ptr[0])) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = atoll(ptr);
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = atoll(ptr);
            } else if (kind == 2) {
                tmp_local_intv->value.fval.min = atoll(ptr);
            }

            while (isdigit(ptr[0])) {
                ++ptr;
            }
        } else if (!strncmp(ptr, "min", 3)) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = local_umin;
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = local_smin;
            } else if (kind == 2) {
                tmp_local_intv->value.fval.min = local_fmin;
            }

            ptr += 3;
        } else if (!strncmp(ptr, "max", 3)) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = local_umax;
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = local_smax;
            } else if (kind == 2) {
                tmp_local_intv->value.fval.min = local_fmax;
            }

            ptr += 3;
        } else {
            assert(0);
        }

        while (isspace(ptr[0])) {
            ptr++;
        }

        /* no interval or interval */
        if ((ptr[0] == '|') || !ptr[0]) {
            if (kind == 0) {
                tmp_local_intv->value.uval.max = tmp_local_intv->value.uval.min;
            } else if (kind == 1) {
                tmp_local_intv->value.sval.max = tmp_local_intv->value.sval.min;
            } else if (kind == 2) {
                tmp_local_intv->value.fval.max = tmp_local_intv->value.fval.min;
            }
        } else if (!strncmp(ptr, "..", 2)) {
            /* skip ".." */
            ptr += 2;
            while (isspace(ptr[0])) {
                ++ptr;
            }

            /* max */
            if (isdigit(ptr[0])) {
                if (kind == 0) {
                    tmp_local_intv->value.uval.max = atoll(ptr);
                } else if (kind == 1) {
                    tmp_local_intv->value.sval.max = atoll(ptr);
                } else if (kind == 2) {
                    tmp_local_intv->value.fval.max = atoll(ptr);
                }
            } else if (!strncmp(ptr, "max", 3)) {
                if (kind == 0) {
                    tmp_local_intv->value.uval.max = local_umax;
                } else if (kind == 1) {
                    tmp_local_intv->value.sval.max = local_smax;
                } else if (kind == 2) {
                    tmp_local_intv->value.fval.max = local_fmax;
                }
            } else {
                assert(0);
            }
        } else {
            assert(0);
        }

        /* next segment (next OR) */
        seg_ptr = strchr(seg_ptr, '|');
        if (!seg_ptr) {
            break;
        }
        seg_ptr++;
    }

    /* check local restrictions against superior ones */
    if (intv) {
        tmp_intv = intv;
        tmp_local_intv = local_intv;

        while (tmp_local_intv && tmp_intv) {
            /* reuse local variables */
            if (kind == 0) {
                local_umin = tmp_local_intv->value.uval.min;
                local_umax = tmp_local_intv->value.uval.max;

                /* it must be in this interval */
                if ((local_umin >= tmp_intv->value.uval.min) && (local_umin <= tmp_intv->value.uval.max)) {
                    /* this interval is covered, next one */
                    if (local_umax <= tmp_intv->value.uval.max) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    /* ascending order of restrictions -> fail */
                    } else {
                        ret = 1;
                        goto cleanup;
                    }
                }
            } else if (kind == 1) {
                local_smin = tmp_local_intv->value.sval.min;
                local_smax = tmp_local_intv->value.sval.max;

                if ((local_smin >= tmp_intv->value.sval.min) && (local_smin <= tmp_intv->value.sval.max)) {
                    if (local_smax <= tmp_intv->value.sval.max) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    } else {
                        ret = 1;
                        goto cleanup;
                    }
                }
            } else if (kind == 2) {
                local_fmin = tmp_local_intv->value.fval.min;
                local_fmax = tmp_local_intv->value.fval.max;

                 if ((local_fmin >= tmp_intv->value.fval.min) && (local_fmin <= tmp_intv->value.fval.max)) {
                    if (local_fmax <= tmp_intv->value.fval.max) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    } else {
                        ret = 1;
                        goto cleanup;
                    }
                }
            }

            tmp_intv = tmp_intv->next;
        }

        /* some interval left uncovered -> fail */
        if (tmp_local_intv) {
            ret = 1;
        }

    }

cleanup:
    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }

    /* fail */
    if (ret) {
        while (local_intv) {
            tmp_local_intv = local_intv->next;
            free(local_intv);
            local_intv = tmp_local_intv;
        }
    }

    return local_intv;
}
