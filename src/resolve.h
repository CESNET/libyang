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

/**
 * @brief Type of an unresolved item (in either SCHEMA or DATA)
 */
enum UNRES_ITEM {
    UNRES_RESOLVED,      /* a resolved item */

    /* SCHEMA */
    UNRES_IDENT,         /* unresolved derived identities */
    UNRES_TYPE_IDENTREF, /* check identityref value */
    UNRES_TYPE_LEAFREF,  /* check leafref value */
    UNRES_TYPE_DER,      /* unresolved derived type */
    UNRES_IFFEAT,        /* unresolved if-feature */
    UNRES_USES,          /* unresolved uses grouping (refines and augments in it are resolved as well) */
    UNRES_TYPE_DFLT,     /* validate default type value */
    UNRES_CHOICE_DFLT,   /* check choice default case */
    UNRES_LIST_KEYS,     /* list keys */
    UNRES_LIST_UNIQ,     /* list uniques */

    /* DATA */
    UNRES_LEAFREF,       /* unresolved leafref reference */
    UNRES_INSTID,        /* unresolved instance-identifier reference */
    UNRES_WHEN,          /* unresolved when condition */
    UNRES_MUST           /* unresolved must condition */
};

/**
 * @brief Unresolved items in DATA
 */
struct unres_data {
    struct lyd_node **node;
    enum UNRES_ITEM *type;
#ifndef NDEBUG
    uint32_t *line;
#endif
    uint32_t count;
};

/**
 * @brief Unresolved items in a SCHEMA
 */
struct unres_schema {
    void **item;            /* array of pointers, each is determined by the type (one of lys_* structures) */
    enum UNRES_ITEM *type;  /* array of unres types */
    void **str_snode;       /* array of pointers, each is determined by the type (a string, a lys_node *, or NULL) */
    struct lys_module **module; /* array of pointers to the item's module */
#ifndef NDEBUG
    uint32_t *line;         /* array of lines for each unres item */
#endif
    uint32_t count;         /* count of unres items */
};

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

int parse_identifier(const char *id);

int resolve_len_ran_interval(const char *str_restr, struct lys_type *type, int superior_restr,
                             struct len_ran_intv **local_intv);

int resolve_superior_type(const char *name, const char *prefix, const struct lys_module *module,
                          const struct lys_node *parent, struct lys_tpdf **ret);

int resolve_unique(struct lys_node *parent, const char *uniq_str, int first, uint32_t line);

int resolve_schema_nodeid(const char *id, const struct lys_node *start, const struct lys_module *mod,
                          LYS_NODE node_type, const struct lys_node **ret);
struct lyd_node *resolve_data_nodeid(const char *id, struct lyd_node *start);

int resolve_augment(struct lys_node_augment *aug, struct lys_node *siblings);

struct lys_ident *resolve_identref(struct lys_ident *base, const char *ident_name, uint32_t line);

int resolve_unres_schema(struct lys_module *mod, struct unres_schema *unres);

int unres_schema_add_str(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                         const char *str, uint32_t line);

int unres_schema_add_node(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                          struct lys_node *node, uint32_t line);

int unres_schema_dup(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                     void *new_item);

int unres_schema_find(struct unres_schema *unres, void *item, enum UNRES_ITEM type);

void unres_schema_free(struct lys_module *module, struct unres_schema **unres);

int resolve_unres_data_item(struct lyd_node *dnode, enum UNRES_ITEM type, int first, uint32_t line);

int unres_data_add(struct unres_data *unres, struct lyd_node *node, enum UNRES_ITEM type, uint32_t line);

int resolve_unres_data(struct unres_data *unres);

#endif /* _RESOLVE_H */
