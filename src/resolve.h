/**
 * @file resolve.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang resolve header
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

#ifndef _RESOLVE_H
#define _RESOLVE_H

#include "libyang.h"
#include "tree_internal.h"

struct len_ran_intv {
    /* 0 - unsigned, 1 - signed, 2 - floating point */
    uint8_t kind;
    union {
        struct {
            uint64_t min;
            uint64_t max;
        } uval;

        struct {
            int64_t min;
            int64_t max;
        } sval;

        struct {
            long double min;
            long double max;
        } fval;
    } value;

    struct len_ran_intv *next;
};

int resolve_len_ran_interval(const char *str_restr, struct lys_type *type, int superior_restr,
                             struct len_ran_intv **local_intv);

struct lys_node *resolve_schema_nodeid(const char *id, struct lys_node *start, struct lys_module *mod,
                                       LYS_NODE node_type);

struct lys_ident *resolve_identityref(struct lys_ident *base, const char *name, const char *ns);

int resolve_uses(struct lys_node_uses *uses, struct unres_schema *unres, uint32_t line);

struct lys_node *resolve_child(struct lys_node *parent, const char *name, int len, LYS_NODE type);

int resolve_path_arg_data(struct unres_data *unres, const char *path, struct unres_data **ret);

int resolve_instid(struct unres_data *unres, const char *path, int path_len, struct unres_data **ret);

struct lys_tpdf *resolve_superior_type(const char *name, const char *prefix, struct lys_module *module,
                                      struct lys_node *parent);

int resolve_augment(struct lys_node_augment *aug, struct lys_node *siblings, struct lys_module *module);

int resolve_unique(struct lys_node *parent, const char *uniq_str, struct lys_unique *uniq_s, uint32_t line);

int resolve_unres(struct lys_module *mod, struct unres_schema *unres);

void unres_add_str(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, const char *str,
                   uint32_t line);

void unres_add_node(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                     struct lys_node *node, uint32_t line);

int unres_dup(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, void *new_item);

int unres_find(struct unres_schema *unres, void *item, enum UNRES_ITEM type);

#endif /* _RESOLVE_H */
