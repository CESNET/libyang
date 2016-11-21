/**
 * @file parser_yang.h
 * @author Pavol Vican
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

#ifndef LY_PARSER_YANG_H_
#define LY_PARSER_YANG_H_

#include <stdlib.h>
#include <string.h>

#include "tree_schema.h"
#include "resolve.h"
#include "common.h"
#include "context.h"

#define LY_ARRAY_SIZE 32
#define LY_READ_ALL 1
#define LY_READ_ONLY_SIZE 0
#define LYS_SYSTEMORDERED 0x40
#define LYS_ORDERED_MASK 0xC0
#define LYS_MIN_ELEMENTS 0x01
#define LYS_MAX_ELEMENTS 0x02
#define LYS_RPC_INPUT 0x01
#define LYS_RPC_OUTPUT 0x02
#define LYS_DATADEF 0x04
#define LYS_TYPE_DEF 0x08
#define CONFIG_INHERIT_DISABLE 0x00
#define CONFIG_INHERIT_ENABLE  0x01
#define CONFIG_IGNORE 0x02
#define CONFIG_MASK 0x03
#define LYS_CHOICE_DEFAULT 0x10

struct lys_node_array{
    uint8_t if_features;
    uint8_t must;
    union {
        uint8_t tpdf;
        uint8_t dflt;
    };
    uint8_t unique;
    union {
        uint uni;
        uint16_t flags;
    };
    union {
        uint enm;
        uint pattern;
        uint bit;
        uint deviate;
        uint keys;
        uint base;
    };
    uint16_t refine;
    uint16_t augment;

};

struct lys_array_size {
    uint8_t rev;
    uint8_t imp;
    uint8_t inc;
    uint32_t ident;
    uint8_t features;
    uint8_t augment;
    uint8_t deviation;
    uint8_t tpdf;
    uint32_t size;
    uint32_t next;
    struct lys_node_array *node;
};

struct type_node {
    union {
        struct lys_node_leaflist *ptr_leaflist;
        struct lys_node_list *ptr_list;
        struct lys_node_leaf *ptr_leaf;
        struct lys_tpdf *ptr_tpdf;
        struct lys_node_anydata *ptr_anydata;
        struct lys_node_augment *ptr_augment;
        struct lys_node_rpc_action *ptr_rpc;
        struct lys_node_inout *ptr_inout;
        struct lys_node_choice *ptr_choice;
    };
    uint flag;
};

struct type_deviation {
    struct lys_deviation *deviation;
    struct lys_node *target;
    struct lys_deviate *deviate;
    struct lys_restr **trg_must;
    uint8_t *trg_must_size;
    struct ly_set *dflt_check;
};

struct yang_type {
    char flags;       /**< this is used to distinguish lyxml_elem * from a YANG temporary parsing structure */
    LY_DATA_TYPE base;
    const char *name;
    struct lys_type *type;
};

#include "parser_yang_bis.h"

char *yang_read_string(const char *input, char *output, int size, int offset, int indent, int version);

int yang_read_common(struct lys_module *module,char *value, enum yytokentype type);

int yang_read_prefix(struct lys_module *module, struct lys_import *imp, char *value);

int yang_check_version(struct lys_module *module, struct lys_submodule *submodule, char *value, int repeat);

/**
 * @brief Add node to the array
 *
 * @param[in/out] node Pointer to the array.
 * @param[in/out] size Pointer to the current size of array.
 * @return 1 on success, 0 on error.
 */
int yang_add_elem(struct lys_node_array **node, uint32_t *size);

int yang_check_imports(struct lys_module *module, struct unres_schema *unres);

int yang_read_description(struct lys_module *module, void *node, char *value, char *where);

int yang_read_reference(struct lys_module *module, void *node, char *value, char *where);

void yang_read_revision(struct lys_module *module, char *value, struct lys_revision *retval);

int yang_read_if_feature(struct lys_module *module, void *ptr, char *value, struct unres_schema *unres, enum yytokentype type);

void *yang_read_identity(struct lys_module *module, char *value);

int yang_read_identyref(struct lys_module *module, struct yang_type *stype, char *expr, struct unres_schema *unres);

int yang_read_base(struct lys_module *module, struct lys_ident *ident, char *value, struct unres_schema *unres);

void *yang_read_must(struct lys_module *module, struct lys_node *node, char *value, enum yytokentype type);

int yang_read_message(struct lys_module *module,struct lys_restr *save,char *value, char *what, int message);

int yang_read_presence(struct lys_module *module, struct lys_node_container *cont, char *value);

int yang_read_config(void *node, int value, enum yytokentype type);

int store_flags(struct lys_node *node, uint8_t flags, int config_opt);

void *yang_read_when(struct lys_module *module, struct lys_node *node, enum yytokentype type, char *value);

/**
 * @brief Allocate memory for node and add to the tree
 *
 * @param[in/out] node Pointer to the array.
 * @param[in] parent Pointer to the parent.
 * @param[in] value Name of node
 * @param[in] nodetype Type of node
 * @param[in] sizeof_struct Size of struct
 * @return Pointer to the node, NULL on error.
*/
void * yang_read_node(struct lys_module *module, struct lys_node *parent, char *value, int nodetype, int sizeof_struct);

void *yang_read_action(struct lys_module *module, struct lys_node *parent, char *value);

int yang_read_default(struct lys_module *module, void *node, char *value, enum yytokentype type);

int yang_read_units(struct lys_module *module, void *node, char *value, enum yytokentype type);

int yang_read_key(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres);

int yang_read_unique(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres);

void *yang_read_type(struct lys_module *module, void *parent, char *value, enum yytokentype type);

void *yang_read_length(struct lys_module *module, struct yang_type *typ, char *value);

int yang_check_type(struct lys_module *module, struct lys_node *parent, struct yang_type *typ, struct lys_type *type, int tpdftype, struct unres_schema *unres);

void yang_delete_type(struct lys_module *module, struct yang_type *stype);

int yang_read_leafref_path(struct lys_module *module, struct yang_type *stype, char *value);

int yang_read_require_instance(struct yang_type *stype, int req);

int yang_read_pattern(struct lys_module *module, struct lys_restr *pattern, char *value, char modifier);

void *yang_read_range(struct  lys_module *module, struct yang_type *typ, char *value);

int yang_read_fraction(struct yang_type *typ, uint32_t value);

int yang_read_enum(struct lys_module *module, struct yang_type *typ, struct lys_type_enum *enm, char *value);

int yang_check_enum(struct yang_type *typ, struct lys_type_enum *enm, int64_t *value, int assign);

int yang_read_bit(struct lys_module *module, struct yang_type *typ, struct lys_type_bit *bit, char *value);

int yang_check_bit(struct yang_type *typ, struct lys_type_bit *bit, int64_t *value, int assign);

void *yang_read_typedef(struct lys_module *module, struct lys_node *parent, char *value);

void *yang_read_refine(struct lys_module *module, struct lys_node_uses *uses, char *value);

void *yang_read_augment(struct lys_module *module, struct lys_node *parent, char *value);

void *yang_read_deviation(struct lys_module *module, char *value);

int yang_read_deviate_unsupported(struct type_deviation *dev);

int yang_read_deviate(struct type_deviation *dev, LYS_DEVIATE_TYPE mod);

int yang_read_deviate_units(struct ly_ctx *ctx, struct type_deviation *dev, char *value);

int yang_read_deviate_must(struct type_deviation *dev, uint8_t c_must);

int yang_fill_unique(struct lys_module *module, struct lys_node_list *list, struct lys_unique *unique, char *value, struct unres_schema *unres);

int yang_read_deviate_unique(struct type_deviation *dev, uint8_t c_uniq);

int yang_read_deviate_default(struct lys_module *module, struct type_deviation *dev, uint8_t c_dflt);

int yang_fill_deviate_default(struct ly_ctx *ctx, struct type_deviation *dev, char *exp);

int yang_read_deviate_config(struct type_deviation *dev, uint8_t value);

int yang_read_deviate_mandatory(struct type_deviation *dev, uint8_t value);

/*
 * type: 0 - min, 1 - max
 */
int yang_read_deviate_minmax(struct type_deviation *dev, uint32_t value, int type);

int yang_check_deviate_must(struct ly_ctx *ctx, struct type_deviation *dev);

int yang_check_deviate_unique(struct lys_module *module, struct type_deviation *dev, char *value);

int yang_check_deviation(struct lys_module *module, struct ly_set *dflt_check, struct unres_schema *unres);

int yang_use_extension(struct lys_module *module, struct lys_node *data_node, void *actual, char *value);

int yang_check_flags(uint16_t *flags, uint16_t mask, char *what, char *where, uint16_t value, int shortint);

/* **
 * @brief Parse YANG from in-memory string
 *
 * yang parser expected at the end of the input string 2 zero byte
 *
 * @param[in] module Pointer to the libyang module.
 * @param[in] submodule Pointer to the libyang submodule.
 * @param[in] unres Pointer to a unres_schema
 * @param[in] data Pointer to a NULL-terminated string containing YANG data to parse.
 * @param[in] size_data Size of input string
 * @param[in/out] node Pointer to node
 * @return 0 on success, 1 on error.
 */
int yang_parse_mem(struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres,
                   const char *data, unsigned int size_data, struct lys_node **node);

struct lys_module *yang_read_module(struct ly_ctx *ctx, const char* data, unsigned int size, const char *revision, int implement);

struct lys_submodule *yang_read_submodule(struct lys_module *module, const char *data, unsigned int size, struct unres_schema *unres);

#endif /* LY_PARSER_YANG_H_ */
