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


struct lys_module *lyp_search_file(struct ly_ctx *ctx, struct lys_module *module, const char *name,
                                   const char *revision);

void lyp_set_implemented(struct lys_module *module);

struct lys_type *lyp_get_next_union_type(struct lys_type *type, struct lys_type *prev_type, int *found);

int lyp_parse_value(struct lyd_node_leaf_list *node, struct lys_type *stype, int resolve, struct unres_data *unres,
                    uint32_t line);

int lyp_check_length_range(const char *expr, struct lys_type *type);

#endif /* LY_PARSER_H_ */
