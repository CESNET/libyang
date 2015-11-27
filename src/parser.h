/**
 * @file parser.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Parsers for libyang
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

#ifndef LY_PARSER_H_
#define LY_PARSER_H_

#include "libyang.h"
#include "tree_schema.h"
#include "tree_internal.h"

#ifndef NDEBUG
#define COUNTLINE(c) if ((c) == 0xa) {lineno++;}
#else
#define lineno 0
#define COUNTLINE(C)
#endif

/**
 * @defgroup yin YIN format support
 * @{
 */
struct lys_module *yin_read_module(struct ly_ctx *ctx, const char *data, int implement, struct unres_schema *unres);
struct lys_submodule *yin_read_submodule(struct lys_module *module, const char *data, int implement,
                                         struct unres_schema *unres);

/**@} yin */

/**
 * @defgroup xmldata XML data format support
 * @{
 */
struct lyd_node *xml_read_data(struct ly_ctx *ctx, const char *data, int options);

/**@} xmldata */

/**
 * @defgroup jsondata JSON data format support
 * @{
 */
struct lyd_node *lyd_parse_json(struct ly_ctx *ctx, const char *data, int options);

/**@} jsondata */


struct lys_module *lyp_search_file(struct ly_ctx *ctx, struct lys_module *module, const char *name,
                                   const char *revision);

void lyp_set_implemented(struct lys_module *module);

struct lys_type *lyp_get_next_union_type(struct lys_type *type, struct lys_type *prev_type, int *found);

int lyp_parse_value(struct lyd_node_leaf_list *node, struct lys_type *stype, int resolve, struct unres_data *unres,
                    uint32_t line);

int lyp_check_length_range(const char *expr, struct lys_type *type);

int fill_yin_type(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_type *type,
              struct unres_schema *unres);

/**
 * Store UTF-8 character specified as 4byte integer into the dst buffer.
 * Returns number of written bytes (4 max), expects that dst has enough space.
 *
 * UTF-8 mapping:
 * 00000000 -- 0000007F:    0xxxxxxx
 * 00000080 -- 000007FF:    110xxxxx 10xxxxxx
 * 00000800 -- 0000FFFF:    1110xxxx 10xxxxxx 10xxxxxx
 * 00010000 -- 001FFFFF:    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 */
unsigned int pututf8(char *dst, int32_t value, uint32_t lineno);

#endif /* LY_PARSER_H_ */
