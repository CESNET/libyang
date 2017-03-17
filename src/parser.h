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

#include <pcre.h>
#include <sys/mman.h>

#include "libyang.h"
#include "tree_schema.h"
#include "tree_internal.h"

/**
 * @defgroup yin YIN format support
 * @{
 */
struct lys_module *yin_read_module(struct ly_ctx *ctx, const char *data, const char *revision, int implement);
struct lys_submodule *yin_read_submodule(struct lys_module *module, const char *data,struct unres_schema *unres);

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
struct lyd_node *lyd_parse_json(struct ly_ctx *ctx, const char *data, int options, const struct lyd_node *rpc_act,
                                const struct lyd_node *data_tree);

/**@} jsondata */

/**
 * internal options values for schema parsers
 */
#define LYS_PARSE_OPT_CFG_NOINHERIT 0x01 /**< do not inherit config flag */
#define LYS_PARSE_OPT_CFG_IGNORE    0x02 /**< ignore config flag (in rpc, actions, notifications) */
#define LYS_PARSE_OPT_CFG_MASK      0x03
#define LYS_PARSE_OPT_INGRP         0x04 /**< flag to know that parser is inside a grouping */

/* list of YANG statement strings */
extern const char *ly_stmt_str[];

enum LY_IDENT {
    LY_IDENT_SIMPLE,   /* only syntax rules */
    LY_IDENT_FEATURE,
    LY_IDENT_IDENTITY,
    LY_IDENT_TYPE,
    LY_IDENT_NODE,
    LY_IDENT_NAME,     /* uniqueness across the siblings */
    LY_IDENT_PREFIX,
    LY_IDENT_EXTENSION
};
int lyp_yin_fill_ext(void *parent, LYEXT_PAR parent_type, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                     struct lys_module *module, struct lyxml_elem *yin, struct lys_ext_instance ***ext,
                     uint8_t ext_index, struct unres_schema *unres);

int lyp_yin_parse_complex_ext(struct lys_module *mod, struct lys_ext_instance_complex *ext,
                              struct lyxml_elem *yin, struct unres_schema *unres);
int lyp_yin_parse_subnode_ext(struct lys_module *mod, void *elem, LYEXT_PAR elem_type,
                              struct lyxml_elem *yin, LYEXT_SUBSTMT type, uint8_t i, struct unres_schema *unres);

struct lys_module *lyp_search_file(struct ly_ctx *ctx, struct lys_module *module, const char *name,
                                   const char *revision, int implement, struct unres_schema *unres);

struct lys_type *lyp_get_next_union_type(struct lys_type *type, struct lys_type *prev_type, int *found);

/* return: 0 - ret set, ok; 1 - ret not set, no log, unknown meta; -1 - ret not set, log, fatal error */
int lyp_fill_attr(struct ly_ctx *ctx, struct lyd_node *parent, const char *module_ns, const char *module_name,
                  const char *attr_name, const char *attr_value, struct lyxml_elem *xml, struct lyd_attr **ret);

int lyp_check_edit_attr(struct ly_ctx *ctx, struct lyd_attr *attr, struct lyd_node *parent, int *editbits);

struct lys_type *lyp_parse_value(struct lys_type *type, const char **value_, struct lyxml_elem *xml,
                                struct lyd_node_leaf_list *leaf, struct lyd_attr *attr, int store, int dflt);

int lyp_check_length_range(const char *expr, struct lys_type *type);

int lyp_check_pattern(const char *pattern, pcre **pcre_precomp);

int fill_yin_type(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_type *type,
                  int tpdftype, struct unres_schema *unres);

int lyp_check_status(uint16_t flags1, struct lys_module *mod1, const char *name1,
                     uint16_t flags2, struct lys_module *mod2, const char *name2,
                     const struct lys_node *node);

void lyp_del_includedup(struct lys_module *mod);

int dup_typedef_check(const char *type, struct lys_tpdf *tpdf, int size);

int dup_identities_check(const char *id, struct lys_module *module);

/**
 * @brief Get know if the node is part of the RPC/action's input/output
 *
 * @param node Schema node to be examined.
 * @return 1 for true, 0 for false
 */
int lyp_is_rpc_action(struct lys_node *node);

/**
 * @brief Check validity of parser options.
 *
 * @param options Parser options to be checked.
 * @return 0 for ok, 1 when multiple data types bits are set, or incompatible options are used together.
 */
int lyp_check_options(int options);

int lyp_check_identifier(const char *id, enum LY_IDENT type, struct lys_module *module, struct lys_node *parent);
int lyp_check_date(const char *date);
int lyp_check_mandatory_augment(struct lys_node_augment *node, const struct lys_node *target);
int lyp_check_mandatory_choice(struct lys_node *node);

int lyp_check_include(struct lys_module *module, const char *value,
                      struct lys_include *inc, struct unres_schema *unres);
int lyp_check_include_missing(struct lys_module *main_module);
int lyp_check_import(struct lys_module *module, const char *value, struct lys_import *imp);
int lyp_check_circmod_add(struct lys_module *module);
void lyp_check_circmod_pop(struct ly_ctx *ctx);

void lyp_sort_revisions(struct lys_module *module);
int lyp_rfn_apply_ext(struct lys_module *module);
int lyp_deviation_apply_ext(struct lys_module *module);
int lyp_mand_check_ext(struct lys_ext_instance_complex *ext, const char *ext_name);

void lyp_ext_instance_rm(struct ly_ctx *ctx, struct lys_ext_instance ***ext, uint8_t *size, uint8_t index);

/**
 * @brief Propagate imports and includes into the main module
 *
 * @param module Main module
 * @param inc Filled include structure
 * @return 0 for success, 1 for failure
 */
int lyp_propagate_submodule(struct lys_module *module, struct lys_include *inc);

/* return: -1 = error, 0 = succes, 1 = already there */
int lyp_ctx_check_module(struct lys_module *module);

int lyp_ctx_add_module(struct lys_module *module);

/**
 * @brief mmap() wrapper for parsers. To unmap, use lyp_munmap().
 *
 * @param[in] prot The desired memory protection as in case of mmap().
 * @param[in] fd File descriptor for getting data.
 * @param[in] addsize Number of additional bytes to be allocated (and zeroed) after the implicitly added
 *                    string-terminating NULL byte.
 * @param[out] length length of the allocated memory.
 * @return On success, the pointer to the memory where the file data resists is returned. On error, the value MAP_FAILED
 * is returned and #ly_errno value is set.
 */
void *
lyp_mmap(int fd, size_t addsize, size_t *length);

/**
 * @brief Unmap function for the data mapped by lyp_mmap()
 */
int lyp_munmap(void *addr, size_t length);

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
unsigned int pututf8(char *dst, int32_t value);
unsigned int copyutf8(char *dst, const char *src);

/*
 * Internal functions implementing YANG extensions support
 * - implemented in extensions.c
 */

/**
 * @brief If available, get the extension plugin for the specified extension
 * @param[in] name Name of the extension
 * @param[in] module Name of the extension's module
 * @param[in] revision Revision of the extension's module
 * @return pointer to the extension plugin structure, NULL if no plugin available
 */
struct lyext_plugin *ext_get_plugin(const char *name, const char *module, const char *revision);

#endif /* LY_PARSER_H_ */
