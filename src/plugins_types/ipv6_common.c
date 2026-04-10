/*
 * Copyright 2026 6WIND S.A.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "ipv6_common.h"
#include "ly_common.h"

static int
ipv6_dual_to_compressed(char *ipv6_str)
{
    char *ipv4_str = NULL;
    uint16_t parts[2];
    uint8_t bytes[4];

    ipv4_str = strrchr(ipv6_str, ':');
    if (ipv4_str == NULL) {
        return -1;
    }

    /* Parse the IPv4 part */
    if (sscanf(ipv4_str + 1, "%hhu.%hhu.%hhu.%hhu",
            &bytes[0], &bytes[1], &bytes[2], &bytes[3]) != 4) {
        return -1;
    }

    parts[0] = (bytes[0] << 8) | bytes[1];
    parts[1] = (bytes[2] << 8) | bytes[3];

    sprintf(ipv4_str + 1, "%x:%x", parts[0], parts[1]);

    return 0;
}

LY_ERR
ipv6address_ip2str(struct in6_addr *addr, uint8_t dual_stack_format,
        char **str_addr, int str_addr_len)
{
    *str_addr = malloc(str_addr_len);
    if (!*str_addr) {
        return LY_EMEM;
    }

    /* convert back to string */
    if (!inet_ntop(AF_INET6, addr, *str_addr, INET6_ADDRSTRLEN)) {
        free(*str_addr);
        return LY_EINVAL;
    }

    /* if compress format is expected convert from dual to compress format */
    if (!dual_stack_format && strchr(*str_addr, '.')) {
        if (ipv6_dual_to_compressed(*str_addr)) {
            free(*str_addr);
            return LY_EINVAL;
        }
    }

    return LY_SUCCESS;
}
