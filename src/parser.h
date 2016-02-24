/**
 * @file parser.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Parsers for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PARSER_H_
#define LY_PARSER_H_

#include "libyang.h"
#include "tree_schema.h"
#include "tree_internal.h"

#ifndef NDEBUG
unsigned int *lineno_location(void); /* implemented in xml.c */
#define lineno (*lineno_location())
#define COUNTLINE(c) if ((c) == 0xa) {lineno++;}
#else
#define lineno 0
#define COUNTLINE(c)
#endif

/**
 * @defgroup yin YIN format support
 * @{
 */
struct lys_module *yin_read_module(struct ly_ctx *ctx, const char *data, int implement);
struct lys_submodule *yin_read_submodule(struct lys_module *module, const char *data, struct unres_schema *unres);

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
struct lyd_node *lyd_parse_json(struct ly_ctx *ctx, const struct lys_node *parent, const char *data, int options);

/**@} jsondata */

enum LY_IDENT {
    LY_IDENT_SIMPLE,   /* only syntax rules */
    LY_IDENT_FEATURE,
    LY_IDENT_IDENTITY,
    LY_IDENT_TYPE,
    LY_IDENT_NODE,
    LY_IDENT_NAME,     /* uniqueness across the siblings */
    LY_IDENT_PREFIX
};

struct lys_module *lyp_search_file(struct ly_ctx *ctx, struct lys_module *module, const char *name,
                                   const char *revision, struct unres_schema *unres);

void lyp_set_implemented(struct lys_module *module);

struct lys_type *lyp_get_next_union_type(struct lys_type *type, struct lys_type *prev_type, int *found);

int lyp_parse_value(struct lyd_node_leaf_list *leaf, struct lyxml_elem *xml, int resolve, struct unres_data *unres, int line);

int lyp_check_length_range(const char *expr, struct lys_type *type);

int fill_yin_type(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_type *type,
                  struct unres_schema *unres);

int lyp_check_status(uint8_t flags1, struct lys_module *mod1, const char *name1,
                     uint8_t flags2, struct lys_module *mod2, const char *name2,
                     unsigned int line, const struct lys_node *node);

/**
 * @brief Get know if the node is part of the RPC's input/output
 *
 * @param node Schema node to be examined.
 * @return 1 for true, 0 for false
 */
int lyp_is_rpc(struct lys_node *node);

/**
 * @brief Check validity of parser options.
 *
 * @param options Parser options to be checked.
 * @retrun 0 for ok, 1 when multiple data types bits are set.
 */
int lyp_check_options(int options);

int lyp_check_identifier(const char *id, enum LY_IDENT type, unsigned int line, struct lys_module *module,
                         struct lys_node *parent);
int lyp_check_date(const char *date, unsigned int line);
int lyp_check_mandatory(struct lys_node *node);

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
unsigned int pututf8(char *dst, int32_t value, uint32_t line);

#endif /* LY_PARSER_H_ */
