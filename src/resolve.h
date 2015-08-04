#ifndef _RESOLVE_H
#define _RESOLVE_H

#include "libyang.h"
#include "tree_internal.h"

struct lys_node *resolve_schema_nodeid(const char *id, struct lys_node *start, struct ly_module *mod,
                                       LYS_NODE node_type);

struct ly_ident *resolve_identityref(struct ly_ident *base, const char *name, const char *ns);

int resolve_uses(struct ly_mnode_uses *uses, unsigned int line, struct unres_item *unres);

struct lys_node *resolve_child(struct lys_node *parent, const char *name, int len, LYS_NODE type);

int resolve_path_arg(struct leafref_instid *unres, const char *path, struct leafref_instid **ret);

int resolve_instid(struct leafref_instid *unres, const char *path, int path_len, struct leafref_instid **ret);

struct lys_tpdf *resolve_superior_type(const char *name, const char *prefix, struct ly_module *module,
                                      struct lys_node *parent);

int resolve_unique(struct lys_node *parent, const char *uniq_str, struct lys_unique *uniq_s, int line);

int resolve_unres(struct ly_module *mod, struct unres_item *unres);

void add_unres_str(struct ly_module *mod, struct unres_item *unres, void *item, enum UNRES_ITEM type, const char *str,
                   int line);

void add_unres_mnode(struct ly_module *mod, struct unres_item *unres, void *item, enum UNRES_ITEM type,
                     struct lys_node *mnode, int line);

void dup_unres(struct ly_module *mod, struct unres_item *unres, void *item, enum UNRES_ITEM type, void *new_item);

int find_unres(struct unres_item *unres, void *item, enum UNRES_ITEM type);

#endif /* _RESOLVE_H */