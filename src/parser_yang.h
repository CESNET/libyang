/**
 * @file parser_yang.h
 * @author Pavol Vican
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
#define LYS_DATADEF 0x04

struct lys_node_array{
    uint8_t if_features;
    uint8_t must;
    uint8_t unique;
    uint8_t tpdf;
    uint8_t keys;
    uint8_t expr_size;
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
    uint32_t size;
    uint32_t next;
    struct lys_node_array *node;
};

struct type_choice {
  char *s;
  struct lys_node_choice *ptr_choice;
};

struct type_leaflist {
    struct lys_node_leaflist *ptr_leaflist;
    uint8_t flag;
};

struct type_list {
    struct lys_node_list *ptr_list;
    uint8_t flag;
    int line;
};

struct type_ident {
    int line;
    char s[0];
};

int yang_read_common(struct lys_module *module,char *value, int type, int line);

int yang_read_prefix(struct lys_module *module, void *save, char *value,int type,int line);

/**
 * @brief Get free member of array
 *
 * @param[in/out] ptr Pointer to the array.
 * @param[in/out] act_size Pointer to the current size of array.
 * @param[in] type Type of array.
 * @param[in] sizeof_struct
 * @return first free member of array, NULL on error.
 */
void *yang_elem_of_array(void **ptr, uint8_t *act_size, int type, int sizeof_struct);

/**
 * @brief Add node to the array
 *
 * @param[in/out] node Pointer to the array.
 * @param[in/out] size Pointer to the current size of array.
 * @return 1 on success, 0 on error.
 */
int yang_add_elem(struct lys_node_array **node, int *size);

int yang_fill_import(struct lys_module *module, struct lys_import *imp, char *value, int line);

int yang_read_description(struct lys_module *module, void *node, char *value, int type, int line);

int yang_read_reference(struct lys_module *module, void *node, char *value, int type, int line);

void *yang_read_revision(struct lys_module *module, char *value);

void *yang_read_feature(struct lys_module *module, char *value, int line);

int yang_read_if_feature(struct lys_module *module, void *ptr, char *value, struct unres_schema *unres, int type, int line);

int yang_read_status(void *node, int value, int type, int line);

void *yang_read_identity(struct lys_module *module, char *value);

int yang_read_base(struct lys_module *module, struct lys_ident *ident, char *value, struct unres_schema *unres, int line);

void *yang_read_must(struct lys_module *module, struct lys_node *node, char *value, int type, int line);

int yang_read_message(struct lys_module *module,struct lys_restr *save,char *value, int type, int message, int line);

int yang_read_presence(struct lys_module *module, struct lys_node_container *cont, char *value, int line);

int yang_read_config(void *node, int value, int type, int line);

void *yang_read_when(struct lys_module *module, struct lys_node *node, int type, char *value, int line);

int yang_read_mandatory(void *node, int value, int type, int line);

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

int yang_read_default(struct lys_module *module, void *node, char *value, int type, int line);

int yang_read_units(struct lys_module *module, void *node, char *value, int type, int line);

int yang_read_key(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres, int line);

int yang_read_unique(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres);

#endif /* LY_PARSER_YANG_H_ */
