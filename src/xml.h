/**
 * @file xml.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic XML parser routines.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_XML_H_
#define LY_XML_H_

#include <stdint.h>

#include "context.h"
#include "set.h"

struct lyxml_ns {
    char *prefix;
    char *ns;
};

enum LYXML_PARSER_STATUS {
    LYXML_STATUS_CDSECT,  /* CDATA section */
    LYXML_STATUS_COMMENT, /* XML comment */
};

struct lyxml_context {
    struct ly_ctx *ctx;
    uint64_t line;
    struct ly_set ns;
};

#endif /* LY_XML_H_ */
