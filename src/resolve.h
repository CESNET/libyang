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

int resolve_unique(struct lys_node *parent, const char *uniq_str, struct lys_unique *uniq_s, uint32_t line);

int resolve_unres(struct lys_module *mod, struct unres_schema *unres);

void add_unres_str(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, const char *str,
                   uint32_t line);

void add_unres_mnode(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                     struct lys_node *mnode, uint32_t line);

void dup_unres(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, void *new_item);

int find_unres(struct unres_schema *unres, void *item, enum UNRES_ITEM type);

#endif /* _RESOLVE_H */