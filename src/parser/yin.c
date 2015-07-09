/**
 * @file yin.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YIN parser for libyang
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "../libyang.h"
#include "../common.h"
#include "../context.h"
#include "../dict.h"
#include "../parser.h"

#include "../tree_internal.h"
#include "../xml.h"

enum LY_IDENT {
    LY_IDENT_SIMPLE,   /* only syntax rules */
    LY_IDENT_FEATURE,
    LY_IDENT_IDENTITY,
    LY_IDENT_TYPE,
    LY_IDENT_NODE,
    LY_IDENT_NAME,     /* uniqueness across the siblings */
    LY_IDENT_PREFIX
};

#define LY_NSYIN "urn:ietf:params:xml:ns:yang:yin:1"
#define LY_NSNACM "urn:ietf:params:xml:ns:yang:ietf-netconf-acm"

#define GETVAL(value, node, arg)                                    \
	value = lyxml_get_attr(node, arg, NULL);                        \
	if (!value) {                                                   \
		LOGVAL(VE_MISSARG, LOGLINE(node), arg, node->name);         \
		goto error;                                                 \
	}

#define OPT_IDENT   0x01
#define OPT_CONFIG  0x02
#define OPT_MODULE  0x04
#define OPT_INHERIT 0x08
#define OPT_NACMEXT 0x10
static int read_yin_common(struct ly_module *, struct ly_mnode *, struct ly_mnode *, struct lyxml_elem *, int);

struct obj_list {
    void *obj;
    struct obj_list *next;
    unsigned int line;
};

static struct ly_mnode *read_yin_choice(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin,
                                        int resolve, struct obj_list **unres);
static struct ly_mnode *read_yin_case(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin,
                                      int resolve, struct obj_list **unres);
static struct ly_mnode *read_yin_anyxml(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin,
                                        int resolve);
static struct ly_mnode *read_yin_container(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin,
                                           int resolve, struct obj_list **unres);
static struct ly_mnode *read_yin_leaf(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin,
                                      int resolve);
static struct ly_mnode *read_yin_leaflist(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin,
                                          int resolve);
static struct ly_mnode *read_yin_list(struct ly_module *module,struct ly_mnode *parent, struct lyxml_elem *yin,
                                      int resolve, struct obj_list **unres);
static struct ly_mnode *read_yin_uses(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *node,
                                      int resolve, struct obj_list **unres);
static struct ly_mnode *read_yin_grouping(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *node,
                                          int resolve, struct obj_list **unres);
static struct ly_when *read_yin_when(struct ly_module *module,struct lyxml_elem *yin);

static int
dup_typedef_check(const char *type, struct ly_tpdf *tpdf, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        if (!strcmp(type, tpdf[i].name)) {
            /* name collision */
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}


static int
dup_feature_check(const char *id, struct ly_module *module)
{
    int i;

    for (i = 0; i < module->features_size; i++) {
        if (!strcmp(id, module->features[i].name)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static int
dup_prefix_check(const char *prefix, struct ly_module *module)
{
    int i;

    if (!module->type && module->prefix && !strcmp(module->prefix, prefix)) {
        return EXIT_FAILURE;
    }
    for (i = 0; i < module->imp_size; i++) {
        if (!strcmp(module->imp[i].prefix, prefix)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static int
check_identifier(const char *id, enum LY_IDENT type, unsigned int line,
                 struct ly_module *module, struct ly_mnode *parent)
{
    int i;
    int size;
    struct ly_tpdf *tpdf;
    struct ly_mnode *mnode;

    assert(id);

    /* check id syntax */
    if (!(id[0] >= 'A' && id[0] <= 'Z') && !(id[0] >= 'a' && id[0] <= 'z') && id[0] != '_') {
        LOGVAL(VE_INID, line, id, "invalid start character");
        return EXIT_FAILURE;
    }
    for (i = 1; id[i]; i++) {
        if (!(id[i] >= 'A' && id[i] <= 'Z') && !(id[i] >= 'a' && id[i] <= 'z')
                && !(id[i] >= '0' && id[i] <= '9') && id[i] != '_' && id[i] != '-' && id[i] != '.') {
            LOGVAL(VE_INID, line, id, "invalid character");
            return EXIT_FAILURE;
        }
    }

    if (i > 64) {
        LOGWRN("Identifier \"%s\" is long, you should use something shorter.", id);
    }

    switch (type) {
    case LY_IDENT_NAME:
        /* check uniqueness of the node within its siblings */
        if (!parent) {
            break;
        }

        LY_TREE_FOR(parent->child, mnode) {
            if (mnode->name == id) {
                LOGVAL(VE_INID, line, id, "name duplication");
                return EXIT_FAILURE;
            }
        }
        break;
    case LY_IDENT_TYPE:
        assert(module);

        /* check collision with the built-in types */
        if (!strcmp(id, "binary") || !strcmp(id, "bits") ||
                !strcmp(id, "boolean") || !strcmp(id, "decimal64") ||
                !strcmp(id, "empty") || !strcmp(id, "enumeration") ||
                !strcmp(id, "identityref") || !strcmp(id, "instance-identifier") ||
                !strcmp(id, "int8") || !strcmp(id, "int16") ||
                !strcmp(id, "int32") || !strcmp(id, "int64") ||
                !strcmp(id, "leafref") || !strcmp(id, "string") ||
                !strcmp(id, "uint8") || !strcmp(id, "uint16") ||
                !strcmp(id, "uint32") || !strcmp(id, "uint64") || !strcmp(id, "union")) {
            LOGVAL(VE_SPEC, line, "Typedef name duplicates built-in type.");
            return EXIT_FAILURE;
        }

        /* check locally scoped typedefs (avoid name shadowing) */
        for (; parent; parent = parent->parent) {
            switch (parent->nodetype) {
            case LY_NODE_CONTAINER:
                size = ((struct ly_mnode_container *)parent)->tpdf_size;
                tpdf = ((struct ly_mnode_container *)parent)->tpdf;
                break;
            case LY_NODE_LIST:
                size = ((struct ly_mnode_list *)parent)->tpdf_size;
                tpdf = ((struct ly_mnode_list *)parent)->tpdf;
                break;
            case LY_NODE_GROUPING:
                size = ((struct ly_mnode_grp *)parent)->tpdf_size;
                tpdf = ((struct ly_mnode_grp *)parent)->tpdf;
                break;
            default:
                continue;
            }

            if (dup_typedef_check(id, tpdf, size)) {
                LOGVAL(VE_DUPID, line, "typedef", id);
                return EXIT_FAILURE;
            }
        }

        /* check top-level names */
        if (dup_typedef_check(id, module->tpdf, module->tpdf_size)) {
            LOGVAL(VE_DUPID, line, "typedef", id);
            return EXIT_FAILURE;
        }

        /* check submodule's top-level names */
        for (i = 0; i < module->inc_size; i++) {
            if (dup_typedef_check(id, module->inc[i].submodule->tpdf, module->inc[i].submodule->tpdf_size)) {
                LOGVAL(VE_DUPID, line, "typedef", id);
                return EXIT_FAILURE;
            }
        }

        /* check top-level names in the main module */
        if (module->type) {
            if (dup_typedef_check(id, ((struct ly_submodule *)module)->belongsto->tpdf,
                                  ((struct ly_submodule *)module)->belongsto->tpdf_size)) {
                LOGVAL(VE_DUPID, line, "typedef", id);
                return EXIT_FAILURE;
            }
        }

        break;
    case LY_IDENT_PREFIX:
        assert(module);

        if (module->type) {
            /* go to the main module */
            module = ((struct ly_submodule *)module)->belongsto;
        }

        /* check the main module itself */
        if (dup_prefix_check(id, module)) {
            LOGVAL(VE_DUPID, line, "prefix", id);
            return EXIT_FAILURE;
        }

        /* and all its submodules */
        for (i = 0; i < module->inc_size; i++) {
            if (dup_prefix_check(id, (struct ly_module *)module->inc[i].submodule)) {
                LOGVAL(VE_DUPID, line, "prefix", id);
                return EXIT_FAILURE;
            }
        }
        break;
    case LY_IDENT_FEATURE:
        assert(module);

        /* check feature name uniqness*/
        /* check features in the current module */
        if (dup_feature_check(id, module)) {
            LOGVAL(VE_DUPID, line, "feature", id);
            return EXIT_FAILURE;
        }

        /* and all its submodules */
        for (i = 0; i < module->inc_size; i++) {
            if (dup_feature_check(id, (struct ly_module *)module->inc[i].submodule)) {
                LOGVAL(VE_DUPID, line, "feature", id);
                return EXIT_FAILURE;
            }
        }
        break;

    default:
        /* no check required */
        break;
    }

    return EXIT_SUCCESS;
}

static int
check_key(struct ly_mnode_leaf *key, uint8_t flags, struct ly_mnode_leaf **list, int index, unsigned int line,
          const char *name, int len)
{
    char *dup = NULL;
    int j;

    /* existence */
    if (!key) {
        if (name[len] != '\0') {
            dup = strdup(name);
            dup[len] = '\0';
            name = dup;
        }
        LOGVAL(VE_KEY_MISS, line, name);
        free(dup);
        return EXIT_FAILURE;
    }

    /* uniqueness */
    for (j = index - 1; j >= 0; j--) {
        if (list[index] == list[j]) {
            LOGVAL(VE_KEY_DUP, line, key->name);
            return EXIT_FAILURE;
        }
    }

    /* key is a leaf */
    if (key->nodetype != LY_NODE_LEAF) {
        LOGVAL(VE_KEY_NLEAF, line, key->name);
        return EXIT_FAILURE;
    }

    /* type of the leaf is not built-in empty */
    if (key->type.base == LY_TYPE_EMPTY) {
        LOGVAL(VE_KEY_TYPE, line, key->name);
        return EXIT_FAILURE;
    }

    /* config attribute is the same as of the list */
    if ((flags & LY_NODE_CONFIG_MASK) != (key->flags & LY_NODE_CONFIG_MASK)) {
        LOGVAL(VE_KEY_CONFIG, line, key->name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int
check_mandatory(struct ly_mnode *mnode)
{
    struct ly_mnode *child;

    assert(mnode);

    if (mnode->flags & LY_NODE_MAND_TRUE) {
        return EXIT_FAILURE;
    }

    if (mnode->nodetype == LY_NODE_CASE || mnode->nodetype == LY_NODE_CHOICE) {
        LY_TREE_FOR(mnode->child, child) {
            if (check_mandatory(child)) {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

static int
check_default(struct ly_type *type, const char *value)
{
    /* TODO - RFC 6020, sec. 7.3.4 */
    (void)type;
    (void)value;
    return EXIT_SUCCESS;
}

static int
check_date(const char *date, unsigned int line)
{
    int i;

    assert(date);

    if (strlen(date) != LY_REV_SIZE - 1) {
        goto error;
    }

    for (i = 0; i < LY_REV_SIZE - 1; i++) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') {
                goto error;
            }
        } else if (!isdigit(date[i])) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    LOGVAL(VE_INDATE, line, date);
    return EXIT_FAILURE;
}

static int
check_length(const char *expr, struct ly_type *type, unsigned int line)
{
    const char *c = expr;
    char *tail;
    uint64_t limit = 0, n;
    int flg = 1; /* first run flag */

    assert(expr);

    /* TODO check compatibility with the restriction defined on type from which this type is derived,
     * it will be the same function to check that the value from instance data respect the restriction */
    (void)type;

lengthpart:

    while (isspace(*c)) {
        c++;
    }

    /* lower boundary or explicit number */
    if (!strncmp(c, "max", 3)) {
max:
        c += 3;
        while (isspace(*c)) {
            c++;
        }
        if (*c != '\0') {
            goto error;
        }

        return EXIT_SUCCESS;

    } else if (!strncmp(c, "min", 3)) {
        if (!flg) {
            /* min cannot be used elsewhere than in the first length-part */
            goto error;
        } else {
            flg = 0;
            /* remember value/lower boundary */
            limit = 0;
        }
        c += 3;
        while (isspace(*c)) {
            c++;
        }

        if (*c == '|') {
            c++;
            /* process next length-parth */
            goto lengthpart;
        } else if (*c == '\0') {
            return EXIT_SUCCESS;
        } else if (!strncmp(c, "..", 2)) {
upper:
            c += 2;
            while (isspace(*c)) {
                c++;
            }
            if (*c == '\0') {
                goto error;
            }

            /* upper boundary */
            if (!strncmp(c, "max", 3)) {
                goto max;
            }

            if (!isdigit(*c)) {
                goto error;
            }

            n = strtol(c, &tail, 10);
            c = tail;
            while (isspace(*c)) {
                c++;
            }
            if (n <= limit) {
                goto error;
            }
            if (*c == '\0') {
                return EXIT_SUCCESS;
            } else if (*c == '|') {
                c++;
                /* remember the uppre boundary for check in next part */
                limit = n;
                /* process next length-parth */
                goto lengthpart;
            } else {
                goto error;
            }
        } else {
            goto error;
        }

    } else if (isdigit(*c)) {
        /* number */
        n = strtol(c, &tail, 10);
        c = tail;
        while (isspace(*c)) {
            c++;
        }
        /* skip limit check in first length-part check */
        if (!flg && n <= limit) {
            goto error;
        }
        flg = 0;
        limit = n;

        if (*c == '|') {
            c++;
            /* process next length-parth */
            goto lengthpart;
        } else if (*c == '\0') {
            return EXIT_SUCCESS;
        } else if (!strncmp(c, "..", 2)) {
            goto upper;
        }
    } /* else error */

error:

    LOGVAL(VE_INARG, line, expr, "length");
    return EXIT_FAILURE;
}

static const char *
read_yin_subnode(struct ly_ctx *ctx, struct lyxml_elem *node, const char *name)
{
    int len;

    /* there should be <text> child */
    if (!node->child || !node->child->name || strcmp(node->child->name, name)) {
        LOGWRN("Expected \"%s\" element in \"%s\" element.", name, node->name);
    } else if (node->child->content) {
        len = strlen(node->child->content);
        return lydict_insert(ctx, node->child->content, len);
    }

    LOGVAL(VE_INARG, LOGLINE(node), name, node->name);
    return NULL;
}

static struct ly_tpdf *
find_superior_type(const char *name, struct ly_module *module, struct ly_mnode *parent)
{
    int i, j, found = 0;
    int prefix_len = 0;
    const char *qname;
    struct ly_tpdf *tpdf;
    int tpdf_size;

    qname = strchr(name, ':');

    if (!qname) {
        /* no prefix, try built-in types */
        for (i = 1; i < LY_DATA_TYPE_COUNT; i++) {
            if (!strcmp(ly_types[i].def->name, name)) {
                return ly_types[i].def;
            }
        }
        qname = name;
    } else {
        /* set qname to correct position after colon */
        prefix_len = qname - name;
        qname++;

        if (!strncmp(name, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
            /* prefix refers to the current module, ignore it */
            prefix_len = 0;
        }
    }

    if (!prefix_len && parent) {
        /* search in local typedefs */
        while (parent) {
            switch (parent->nodetype) {
            case LY_NODE_CONTAINER:
                tpdf_size = ((struct ly_mnode_container *)parent)->tpdf_size;
                tpdf = ((struct ly_mnode_container *)parent)->tpdf;
                break;

            case LY_NODE_LIST:
                tpdf_size = ((struct ly_mnode_list *)parent)->tpdf_size;
                tpdf = ((struct ly_mnode_list *)parent)->tpdf;
                break;

            case LY_NODE_GROUPING:
                tpdf_size = ((struct ly_mnode_grp *)parent)->tpdf_size;
                tpdf = ((struct ly_mnode_grp *)parent)->tpdf;
                break;

                /* TODO add rpc, notification, input, output */
            default:
                parent = parent->parent;
                continue;
            }

            for (i = 0; i < tpdf_size; i++) {
                if (!strcmp(tpdf[i].name, qname)) {
                    return &tpdf[i];
                }
            }

            parent = parent->parent;
        }
    } else if (prefix_len) {
        /* get module where to search */
        for (i = 0; i < module->imp_size; i++) {
            if (!strncmp(module->imp[i].prefix, name, prefix_len) && !module->imp[i].prefix[prefix_len]) {
                module = module->imp[i].module;
                found = 1;
                break;
            }
        }
        if (!found) {
            return NULL;
        }
    }

    /* search in top level typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        if (!strcmp(module->tpdf[i].name, qname)) {
            return &module->tpdf[i];
        }
    }

    /* search in submodules */
    for (i = 0; i < module->inc_size; i++) {
        for (j = 0; j < module->inc[i].submodule->tpdf_size; j++) {
            if (!strcmp(module->inc[i].submodule->tpdf[j].name, qname)) {
                return &module->inc[i].submodule->tpdf[j];
            }
        }
    }

    return NULL;
}

static struct ly_ident *
find_base_ident_sub(struct ly_module *module, struct ly_ident *ident, const char *basename)
{
    unsigned int i, j;
    struct ly_ident *base_iter = NULL;
    struct ly_ident_der *der;

    /* search module */
    for (i = 0; i < module->ident_size; i++) {
        if (!strcmp(basename, module->ident[i].name)) {

            if (!ident) {
                /* just search for type, so do not modify anything, just return
                 * the base identity pointer
                 */
                return &module->ident[i];
            }

            /* we are resolving identity definition, so now update structures */
            ident->base = base_iter = &module->ident[i];

            break;
        }
    }

    /* search submodules */
    if (!base_iter) {
        for (j = 0; j < module->inc_size; j++) {
            for (i = 0; i < module->inc[j].submodule->ident_size; i++) {
                if (!strcmp(basename, module->inc[j].submodule->ident[i].name)) {

                    if (!ident) {
                        return &module->inc[j].submodule->ident[i];
                    }

                    ident->base = base_iter = &module->inc[j].submodule->ident[i];
                    break;
                }
            }
        }
    }

    /* we found it somewhere */
    if (base_iter) {
        while (base_iter) {
            for (der = base_iter->der; der && der->next; der = der->next);
            if (der) {
                der->next = malloc(sizeof *der);
                der = der->next;
            } else {
                ident->base->der = der = malloc(sizeof *der);
            }
            der->next = NULL;
            der->ident = ident;

            base_iter = base_iter->base;
        }
        return ident->base;
    }

    return NULL;
}

static struct ly_ident *
find_base_ident(struct ly_module *module, struct ly_ident *ident, struct lyxml_elem *node)
{
    const char *name;
    int prefix_len = 0;
    int i, found = 0;
    struct ly_ident *result;
    const char *basename;

    basename = lyxml_get_attr(node, "name", NULL);
    if (!basename) {
        LOGVAL(VE_MISSARG, LOGLINE(node), "name", "base");
        return NULL;
    }

    /* search for the base identity */
    name = strchr(basename, ':');
    if (name) {
        /* set name to correct position after colon */
        prefix_len = name - basename;
        name++;

        if (!strncmp(basename, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
            /* prefix refers to the current module, ignore it */
            prefix_len = 0;
        }
    } else {
        name = basename;
    }

    if (prefix_len) {
        /* get module where to search */
        for (i = 0; i < module->imp_size; i++) {
            if (!strncmp(module->imp[i].prefix, basename, prefix_len)
                && !module->imp[i].prefix[prefix_len]) {
                module = module->imp[i].module;
                found = 1;
                break;
            }
        }
        if (!found) {
            /* identity refers unknown data model */
            LOGVAL(VE_INPREFIX, LOGLINE(node), basename);
            return NULL;
        }
    } else {
        /* search in submodules */
        for (i = 0; i < module->inc_size; i++) {
            result = find_base_ident_sub((struct ly_module *)module->inc[i].submodule, ident, name);
            if (result) {
                return result;
            }
        }
    }

    /* search in the identified module */
    result = find_base_ident_sub(module, ident, name);
    if (!result) {
        LOGVAL(VE_INARG, LOGLINE(node), basename, ident ? "identity" : "type");
    }

    return result;
}

static int
fill_yin_identity(struct ly_module *module, struct lyxml_elem *yin, struct ly_ident *ident)
{
    struct lyxml_elem *node;

    if (read_yin_common(module, NULL, (struct ly_mnode *)ident, yin, OPT_IDENT | OPT_MODULE)) {
        return EXIT_FAILURE;
    }

    LY_TREE_FOR(yin->child, node) {
        if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(node->name, "base")) {
            if (ident->base) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), "base", "identity");
                return EXIT_FAILURE;
            }
            if (!find_base_ident(module, ident, node)) {
                return EXIT_FAILURE;
            }
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(node), node->name, "identity");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static int
read_restr_substmt(struct ly_ctx *ctx, struct ly_restr *restr, struct lyxml_elem *yin)
{
    struct lyxml_elem *child;
    const char *value;

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(child->name, "description")) {
            if (restr->dsc) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                return EXIT_FAILURE;
            }
            restr->dsc = read_yin_subnode(ctx, child, "text");
            if (!restr->dsc) {
                return EXIT_FAILURE;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (restr->ref) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                return EXIT_FAILURE;
            }
            restr->ref = read_yin_subnode(ctx, child, "text");
            if (!restr->ref) {
                return EXIT_FAILURE;
            }
        } else if (!strcmp(child->name, "error-app-tag")) {
            if (restr->eapptag) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                return EXIT_FAILURE;
            }
            GETVAL(value, child, "value");
            restr->eapptag = lydict_insert(ctx, value, 0);
        } else if (!strcmp(child->name, "error-message")) {
            if (restr->emsg) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                return EXIT_FAILURE;
            }
            restr->emsg = read_yin_subnode(ctx, child, "value");
            if (!restr->emsg) {
                return EXIT_FAILURE;
            }
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
fill_yin_type(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin, struct ly_type *type, struct obj_list **unres)
{
    const char *value, *delim, *name;
    struct lyxml_elem *next, *node;
    struct ly_restr **restr;
    struct obj_list *unres_new;
    struct ly_type_bit bit;
    int i, j;
    int64_t v, v_;
    int64_t p, p_;

    GETVAL(value, yin, "name");

    if (!type->prefix) {
        /* if we are trying to resolve unresolved type,
         * prefix is already stored
         */
        delim = strchr(value, ':');
        if (delim) {
            type->prefix = lydict_insert(module->ctx, value, delim - value);
        }
    }

    type->der = find_superior_type(value, module, parent);
    if (!type->der) {
        if (unres) {
            /* store it for later resolving */
            LOGVRB("Unresolved type of \"%s\" (line %d), trying to resolve it later", value, LOGLINE(yin));
            unres_new = calloc(1, sizeof *unres_new);
            if (*unres) {
                unres_new->next = *unres;
            }
            /* keep XML data for later processing */
            type->der = (struct ly_tpdf *)lyxml_dup_elem(module->ctx, yin, NULL, 1);

            unres_new->obj = type;
            unres_new->line = LOGLINE(yin);

            /* put it at the beginning of the unresolved list */
            *unres = unres_new;
            return EXIT_SUCCESS;
        }
        LOGVAL(VE_INARG, LOGLINE(yin), value, yin->name);
        goto error;
    }
    type->base = type->der->type.base;

    switch (type->base) {
    case LY_TYPE_BITS:
        /* RFC 6020 9.7.4 - bit */

        /* get bit specifications, at least one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "bit")) {
                type->info.bits.count++;
            } else {
                LOGVAL(VE_INSTMT, LOGLINE(yin->child), yin->child->name);
                goto error;
            }
        }
        if (!type->info.bits.count) {
            if (type->der->type.der) {
                /* this is just a derived type with no bit specified/required */
                break;
            }
            LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "bit", "type");
            goto error;
        }

        type->info.bits.bit = calloc(type->info.bits.count, sizeof *type->info.bits.bit);
        p = 0;
        i = -1;
        LY_TREE_FOR(yin->child, next) {
            i++;

            GETVAL(value, next, "name");
            if (check_identifier(value, LY_IDENT_SIMPLE, LOGLINE(next), NULL, NULL)) {
                goto error;
            }
            type->info.bits.bit[i].name = lydict_insert(module->ctx, value, strlen(value));
            if (read_yin_common(module, NULL, (struct ly_mnode *)&type->info.bits.bit[i], next, 0)) {
                type->info.bits.count = i + 1;
                goto error;
            }

            /* check the name uniqueness */
            for (j = 0; j < i; j++) {
                if (!strcmp(type->info.bits.bit[j].name, type->info.bits.bit[i].name)) {
                    LOGVAL(VE_BITS_DUPNAME, LOGLINE(next), type->info.bits.bit[i].name);
                    type->info.bits.count = i + 1;
                    goto error;
                }
            }

            p_ = -1;
            LY_TREE_FOR(next->child, node) {
                if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                    /* garbage */
                    continue;
                }

                if (!strcmp(node->name, "position")) {
                    GETVAL(value, node, "value");
                    p_ = strtol(value, NULL, 10);

                    /* range check */
                    if (p_ < 0 || p_ > UINT32_MAX) {
                        LOGVAL(VE_INARG, LOGLINE(node), value, "bit/position");
                        type->info.bits.count = i + 1;
                        goto error;
                    }
                    type->info.bits.bit[i].pos = (uint32_t)p_;

                    /* keep the highest enum value for automatic increment */
                    if (type->info.bits.bit[i].pos >= p) {
                        p = type->info.bits.bit[i].pos;
                        p++;
                    } else {
                        /* check that the value is unique */
                        for (j = 0; j < i; j++) {
                            if (type->info.bits.bit[j].pos == type->info.bits.bit[i].pos) {
                                LOGVAL(VE_BITS_DUPVAL, LOGLINE(node), type->info.bits.bit[i].pos, type->info.bits.bit[i].name);
                                type->info.bits.count = i + 1;
                                goto error;
                            }
                        }
                    }
                } else {
                    LOGVAL(VE_INSTMT, LOGLINE(next), next->name);
                    goto error;
                }
            }
            if (p_ == -1) {
                /* assign value automatically */
                if (p > UINT32_MAX) {
                    LOGVAL(VE_INARG, LOGLINE(next), "4294967295", "bit/position");
                    type->info.bits.count = i + 1;
                    goto error;
                }
                type->info.bits.bit[i].pos = (uint32_t)p;
                p++;
            }

            /* keep them ordered by position */
            j = i;
            while (j && type->info.bits.bit[j - 1].pos > type->info.bits.bit[j].pos) {
                /* switch them */
                memcpy(&bit, &type->info.bits.bit[j], sizeof bit);
                memcpy(&type->info.bits.bit[j], &type->info.bits.bit[j - 1], sizeof bit);
                memcpy(&type->info.bits.bit[j - 1], &bit, sizeof bit);
                j--;
            }
        }
        break;

    case LY_TYPE_DEC64:
        /* RFC 6020 9.2.4 - range and 9.3.4 - fraction-digits */
        LY_TREE_FOR(yin->child, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                continue;
            }

            if (!strcmp(node->name, "range")) {
                if (type->info.dec64.range) {
                    LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (check_length(value, type, LOGLINE(node))) {
                    goto error;
                }
                type->info.dec64.range = calloc(1, sizeof *type->info.dec64.range);
                type->info.dec64.range->expr = lydict_insert(module->ctx, value, 0);

                /* get possible substatements */
                if (read_restr_substmt(module->ctx, type->info.dec64.range, node)) {
                    goto error;
                }
            } else if (!strcmp(node->name, "fraction-digits")) {
                if (type->info.dec64.dig) {
                    LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }
                GETVAL(value, node, "value");
                v = strtol(value, NULL, 10);

                /* range check */
                if (v < 1 || v > 18) {
                    LOGVAL(VE_INARG, LOGLINE(node), value, node->name);
                    goto error;
                }
                type->info.dec64.dig = (uint8_t)v;
            } else {
                LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }

        /* mandatory sub-statement(s) check */
        if (!type->info.dec64.dig && !type->der->type.der) {
            /* decimal64 type directly derived from built-in type requires fraction-digits */
            LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "fraction-digits", "type");
            goto error;
        }
        break;

    case LY_TYPE_ENUM:
        /* RFC 6020 9.6 - enum */

        /* get enum specifications, at least one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "enum")) {
                type->info.enums.count++;
            } else {
                LOGVAL(VE_INSTMT, LOGLINE(yin->child), yin->child->name);
                goto error;
            }
        }
        if (!type->info.enums.count) {
            if (type->der->type.der) {
                /* this is just a derived type with no enum specified/required */
                break;
            }
            LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "enum", "type");
            goto error;
        }

        type->info.enums.list = calloc(type->info.enums.count, sizeof *type->info.enums.list);
        v = 0;
        i = -1;
        LY_TREE_FOR(yin->child, next) {
            i++;

            GETVAL(value, next, "name");
            if (check_identifier(value, LY_IDENT_SIMPLE, LOGLINE(next), NULL, NULL)) {
                goto error;
            }
            type->info.enums.list[i].name = lydict_insert(module->ctx, value, strlen(value));
            if (read_yin_common(module, NULL, (struct ly_mnode *)&type->info.enums.list[i], next, 0)) {
                type->info.enums.count = i + 1;
                goto error;
            }

            /* the assigned name MUST NOT have any leading or trailing whitespace characters */
            value = type->info.enums.list[i].name;
            if (isspace(value[0]) || isspace(value[strlen(value) - 1])) {
                LOGVAL(VE_ENUM_WS, LOGLINE(next), value);
                type->info.enums.count = i + 1;
                goto error;
            }

            /* check the name uniqueness */
            for (j = 0; j < i; j++) {
                if (!strcmp(type->info.enums.list[j].name, type->info.enums.list[i].name)) {
                    LOGVAL(VE_ENUM_DUPNAME, LOGLINE(next), type->info.enums.list[i].name);
                    type->info.enums.count = i + 1;
                    goto error;
                }
            }

            v_ = -1;
            LY_TREE_FOR(next->child, node) {
                if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                    /* garbage */
                    continue;
                }

                if (!strcmp(node->name, "value")) {
                    GETVAL(value, node, "value");
                    v_ = strtol(value, NULL, 10);

                    /* range check */
                    if (v_ < INT32_MIN || v_ > INT32_MAX) {
                        LOGVAL(VE_INARG, LOGLINE(node), value, "enum/value");
                        type->info.enums.count = i + 1;
                        goto error;
                    }
                    type->info.enums.list[i].value = v_;

                    /* keep the highest enum value for automatic increment */
                    if (type->info.enums.list[i].value > v) {
                        v = type->info.enums.list[i].value;
                        v++;
                    } else {
                        /* check that the value is unique */
                        for (j = 0; j < i; j++) {
                            if (type->info.enums.list[j].value == type->info.enums.list[i].value) {
                                LOGVAL(VE_ENUM_DUPVAL, LOGLINE(node), type->info.enums.list[i].value,
                                       type->info.enums.list[i].name);
                                type->info.enums.count = i + 1;
                                goto error;
                            }
                        }
                    }
                } else {
                    LOGVAL(VE_INSTMT, LOGLINE(next), next->name);
                    goto error;
                }
            }
            if (v_ == -1) {
                /* assign value automatically */
                if (v > INT32_MAX) {
                    LOGVAL(VE_INARG, LOGLINE(next), "2147483648", "enum/value");
                    type->info.enums.count = i + 1;
                    goto error;
                }
                type->info.enums.list[i].value = v;
                v++;
            }
        }
        break;

    case LY_TYPE_IDENT:
        /* RFC 6020 9.10 - base */

        /* get base specification, exactly one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (strcmp(yin->child->name, "base")) {
                LOGVAL(VE_INSTMT, LOGLINE(yin->child), yin->child->name);
                goto error;
            }
        }

        if (!yin->child) {
            if (type->der->type.der) {
                /* this is just a derived type with no base specified/required */
                break;
            }
            LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "base", "type");
            goto error;
        }
        if (yin->child->next) {
            LOGVAL(VE_TOOMANY, LOGLINE(yin->child->next), yin->child->next->name, yin->name);
            goto error;
        }
        type->info.ident.ref = find_base_ident(module, NULL, yin->child);
        if (!type->info.ident.ref) {
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_INST:
        /* RFC 6020 9.13.2 - require-instance */
        LY_TREE_FOR(yin->child, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                continue;
            }

            if (!strcmp(node->name, "require-instance")) {
                if (type->info.inst.req) {
                    LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }
                GETVAL(value, node, "value");
                if (strcmp(value, "true")) {
                    type->info.inst.req = 1;
                } else if (strcmp(value, "false")) {
                    type->info.inst.req = -1;
                } else {
                    LOGVAL(VE_INARG, LOGLINE(node), value, node->name);
                    goto error;
                }
            } else {
                LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }
        break;

    case LY_TYPE_BINARY:
        /* RFC 6020 9.8.1, 9.4.4 - length, number of octets it contains */
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        /* RFC 6020 9.2.4 - range */

        /* length and range are actually the same restriction, so process
         * them by this common code, we just need to differ the name and
         * structure where the information will be stored
         */
        if (type->base == LY_TYPE_BINARY) {
            restr = &type->info.binary.length;
            name = "length";
        } else {
            restr = &type->info.num.range;
            name = "range";
        }

        LY_TREE_FOR(yin->child, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                continue;
            }

            if (!strcmp(node->name, name)) {
                if (*restr) {
                    LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (check_length(value, type, LOGLINE(node))) {
                    goto error;
                }
                *restr = calloc(1, sizeof **restr);
                (*restr)->expr = lydict_insert(module->ctx, value, 0);

                /* get possible substatements */
                if (read_restr_substmt(module->ctx, *restr, node)) {
                    goto error;
                }
            } else {
                LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }
        break;

    case LY_TYPE_LEAFREF:
        /* RFC 6020 9.9.2 - path */
        LY_TREE_FOR(yin->child, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                continue;
            }

            if (!strcmp(node->name, "path")) {
                if (type->info.lref.path) {
                    LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                /* TODO
                 * it would be nice to perform here a check that target is leaf or leaf-list,
                 * but schema is not finished yet and path can point almost to anywhere, so
                 * we will have to check the path at the end of parsing the schema.
                 */
                type->info.lref.path = lydict_insert(module->ctx, value, 0);
            } else {
                LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }

        if (!type->info.lref.path) {
            if (type->der->type.der) {
                /* this is just a derived type with no path specified/required */
                break;
            }
            LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "path", "type");
            goto error;
        }
        break;

    case LY_TYPE_STRING:
        /* RFC 6020 9.4.4 - length */
        /* RFC 6020 9.4.6 - pattern */
        i = 0;
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "length")) {
                if (type->info.str.length) {
                    LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (check_length(value, type, LOGLINE(node))) {
                    goto error;
                }
                type->info.str.length = calloc(1, sizeof *type->info.str.length);
                type->info.str.length->expr = lydict_insert(module->ctx, value, 0);

                /* get possible sub-statements */
                if (read_restr_substmt(module->ctx, type->info.str.length, node)) {
                    goto error;
                }
                lyxml_free_elem(module->ctx, node);
            } else if (!strcmp(node->name, "pattern")) {
                i++;
            } else {
                LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }
        /* store patterns in array */
        if (i) {
            type->info.str.patterns = calloc(i, sizeof *type->info.str.patterns);
            LY_TREE_FOR(yin->child, node) {
                GETVAL(value, yin->child, "value");
                type->info.str.patterns[type->info.str.pat_count].expr = lydict_insert(module->ctx, value, 0);

                /* get possible sub-statements */
                if (read_restr_substmt(module->ctx, &type->info.str.patterns[type->info.str.pat_count], yin->child)) {
                    goto error;
                }
                type->info.str.pat_count++;
            }
        }
        break;

    case LY_TYPE_UNION:
        /* RFC 6020 7.4 - type */
        /* count number of types in union */
        i = 0;
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "type")) {
                i++;
            } else {
                LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }

        if (!i) {
            if (type->der->type.der) {
                /* this is just a derived type with no base specified/required */
                break;
            }
            LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "type", "(union) type");
            goto error;
        }

        /* allocate array for union's types ... */
        type->info.uni.type = calloc(i, sizeof *type->info.uni.type);
        /* ... and fill the structures */
        LY_TREE_FOR(yin->child, node) {
            if (fill_yin_type(module, parent, node, &type->info.uni.type[type->info.uni.count], unres)) {
                goto error;
            }
            type->info.uni.count++;

            /* union's type cannot be empty or leafref */
            if (type->info.uni.type[type->info.uni.count - 1].base == LY_TYPE_EMPTY) {
                LOGVAL(VE_INARG, LOGLINE(node), "empty", node->name);
                goto error;
            } else if (type->info.uni.type[type->info.uni.count - 1].base == LY_TYPE_LEAFREF) {
                LOGVAL(VE_INARG, LOGLINE(node), "leafref", node->name);
                goto error;
            }
        }
        break;

    default:
        /* no sub-statement allowed in:
         * LY_TYPE_BOOL, LY_TYPE_EMPTY
         */
        LY_TREE_FOR(yin->child, node) {
            if (node->ns && !strcmp(node->ns->value, LY_NSYIN)) {
                LOGVAL(VE_INSTMT, LOGLINE(yin->child), yin->child->name);
                goto error;
            }
        }
        break;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static int
fill_yin_typedef(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin, struct ly_tpdf *tpdf, struct obj_list **unres)
{
    const char *value;
    struct lyxml_elem *node;

    GETVAL(value, yin, "name");
    if (check_identifier(value, LY_IDENT_TYPE, LOGLINE(yin), module, parent)) {
        goto error;
    }
    tpdf->name = lydict_insert(module->ctx, value, strlen(value));

    /* generic part - status, description, reference */
    if (read_yin_common(module, NULL, (struct ly_mnode *)tpdf, yin, OPT_IDENT)) {
        goto error;
    }

    LY_TREE_FOR(yin->child, node) {
        if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(node->name, "type")) {
            if (tpdf->type.der) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            if (fill_yin_type(module, parent, node, &tpdf->type, unres)) {
                goto error;
            }
        } else if (!strcmp(node->name, "default")) {
            if (tpdf->dflt) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "value");
            tpdf->dflt = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(node->name, "units")) {
            if (tpdf->units) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "name");
            tpdf->units = lydict_insert(module->ctx, value, strlen(value));
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(node), value);
            goto error;
        }
    }

    /* check mandatory value */
    if (!tpdf->type.der) {
        LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "type", yin->name);
        goto error;
    }

    /* check default value */
    if (check_default(&tpdf->type, tpdf->dflt)) {
        goto error;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static struct ly_feature *
resolve_feature(const char *name, struct ly_module *module, unsigned int line)
{
    const char *prefix;
    unsigned int prefix_len = 0;
    int i, j, found = 0;

    assert(name);
    assert(module);

    /* check prefix */
    prefix = name;
    name = strchr(prefix, ':');
    if (name) {
        /* there is prefix */
        prefix_len = name - prefix;
        name++;

        /* check whether the prefix points to the current module */
        if (!strncmp(prefix, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
            /* then ignore prefix and works as there is no prefix */
            prefix_len = 0;
        }
    } else {
        /* no prefix, set pointers correctly */
        name = prefix;
    }

    if (prefix_len) {
        /* search in imported modules */
        for (i = 0; i < module->imp_size; i++) {
            if (!strncmp(module->imp[i].prefix, prefix, prefix_len) && !module->imp[i].prefix[prefix_len]) {
                module = module->imp[i].module;
                found = 1;
                break;
            }
        }
        if (!found) {
            /* identity refers unknown data model */
            LOGVAL(VE_INPREFIX, line, prefix);
            return NULL;
        }
    } else {
        /* search in submodules */
        for (i = 0; i < module->inc_size; i++) {
            for (j = 0; j < module->inc[i].submodule->features_size; j++) {
                if (!strcmp(name, module->inc[i].submodule->features[j].name)) {
                    return &(module->inc[i].submodule->features[j]);
                }
            }
        }
    }

    /* search in the identified module */
    for (j = 0; j < module->features_size; j++) {
        if (!strcmp(name, module->features[j].name)) {
            return &module->features[j];
        }
    }

    /* not found */
    return NULL;
}

static int
fill_yin_feature(struct ly_module *module, struct lyxml_elem *yin, struct ly_feature *f)
{
    const char *value;
    struct lyxml_elem *child, *next;
    int c = 0;

    GETVAL(value, yin, "name");
    if (check_identifier(value, LY_IDENT_FEATURE, LOGLINE(yin), module, NULL)) {
        goto error;
    }
    f->name = lydict_insert(module->ctx, value, strlen(value));
    f->module = module;

    if (read_yin_common(module, NULL, (struct ly_mnode *)f, yin, 0)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, child);
            continue;
        }

        if (!strcmp(child->name, "if-feature")) {
            c++;
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }
    }

    if (c) {
        f->features = calloc(c, sizeof *f->features);
    }
    LY_TREE_FOR(yin->child, child) {
        GETVAL(value, child, "name");
        f->features[f->features_size] = resolve_feature(value, module, LOGLINE(child));
        if (!f->features[f->features_size]) {
            goto error;
        }
        f->features_size++;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static int
fill_yin_must(struct ly_module *module, struct lyxml_elem *yin, struct ly_restr *must)
{
    const char *value;

    GETVAL(value, yin, "condition");
    must->expr = lydict_insert(module->ctx, value, strlen(value));

    return read_restr_substmt(module->ctx, must, yin);

error: /* GETVAL requires this label */

    return EXIT_FAILURE;
}

static int
parse_unique(struct ly_mnode *parent, struct lyxml_elem *node, struct ly_unique *uniq_s)
{
    const char *value;
    char *uniq_str = NULL, *uniq_val, *start;
    int i, j;

    /* count the number of unique values */
    GETVAL(value, node, "tag");
    uniq_val = uniq_str = strdup(value);
    uniq_s->leafs_size = 0;
    while ((uniq_val = strpbrk(uniq_val, " \t\n"))) {
        uniq_s->leafs_size++;
        while (isspace(*uniq_val)) {
            uniq_val++;
        }
    }
    uniq_s->leafs_size++;
    uniq_s->leafs = calloc(uniq_s->leafs_size, sizeof *uniq_s->leafs);

    /* interconnect unique values with the leafs */
    uniq_val = uniq_str;
    for (i = 0; uniq_val && i < uniq_s->leafs_size; i++) {
        start = uniq_val;
        if ((uniq_val = strpbrk(start, " \t\n"))) {
            *uniq_val = '\0'; /* add terminating NULL byte */
            uniq_val++;
            while (isspace(*uniq_val)) {
                uniq_val++;
            }
        } /* else only one nodeid present/left already NULL byte terminated */

        uniq_s->leafs[i] = (struct ly_mnode_leaf *)resolve_schema_nodeid(start, parent, parent->module, LY_NODE_USES);
        if (!uniq_s->leafs[i] || uniq_s->leafs[i]->nodetype != LY_NODE_LEAF) {
            LOGVAL(VE_INARG, LOGLINE(node), start, node->name);
            if (!uniq_s->leafs[i]) {
                LOGVAL(VE_SPEC, 0, "Target leaf not found.");
            } else {
                LOGVAL(VE_SPEC, 0, "Target is not a leaf.");
            }
            goto error;
        }

        for (j = 0; j < i; j++) {
            if (uniq_s->leafs[j] == uniq_s->leafs[i]) {
                LOGVAL(VE_INARG, LOGLINE(node), start, node->name);
                LOGVAL(VE_SPEC, 0, "The identifier is not unique");
                goto error;
            }
        }
    }

    free(uniq_str);
    return EXIT_SUCCESS;

error:

    free(uniq_s->leafs);
    free(uniq_str);

    return EXIT_FAILURE;
}

/*
 * type: 0 - min, 1 - max
 */
static int
deviate_minmax(struct ly_mnode *target, struct lyxml_elem *node, struct ly_deviate *d, int type)
{
    const char *value;
    char *endptr;
    unsigned long val;
    uint32_t *ui32val;

    /* check target node type */
    if (target->nodetype == LY_NODE_LEAFLIST) {
        if (type) {
            ui32val = &((struct ly_mnode_leaflist *)target)->max;
        } else {
            ui32val = &((struct ly_mnode_leaflist *)target)->min;
        }
    } else if (target->nodetype == LY_NODE_LIST) {
        if (type) {
            ui32val = &((struct ly_mnode_list *)target)->max;
        } else {
            ui32val = &((struct ly_mnode_list *)target)->min;
        }
    } else {
        LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
        LOGVAL(VE_SPEC, 0, "Target node does not allow \"%s\" property.", node->name);
        goto error;
    }

    GETVAL(value, node, "value");
    while (isspace(value[0])) {
        value++;
    }

    /* convert it to uint32_t */
    errno = 0;
    endptr = NULL;
    val = strtoul(value, &endptr, 10);
    if (*endptr || value[0] == '-' || errno || val > UINT32_MAX) {
        LOGVAL(VE_INARG, LOGLINE(node), value, node->name);
        goto error;
    }
    if (type) {
        d->max = (uint32_t)val;
    } else {
        d->min = (uint32_t)val;
    }

    if (d->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if (*ui32val) {
            LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
            LOGVAL(VE_SPEC, 0, "Adding property that already exists.");
            goto error;
        }
    }

    if (d->mod == LY_DEVIATE_DEL) {
        /* check values */
        if ((uint32_t)val != *ui32val) {
            LOGVAL(VE_INARG, LOGLINE(node), value, node->name);
            LOGVAL(VE_SPEC, 0, "Value differs from the target being deleted.");
            goto error;
        }
        /* remove current min-elements value of the target */
        *ui32val = 0;
    } else { /* add (already checked) and replace */
        /* set new value specified in deviation */
        *ui32val = (uint32_t)val;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static int
fill_yin_deviation(struct ly_module *module, struct lyxml_elem *yin, struct ly_deviation *dev)
{
    const char *value, **stritem;
    struct lyxml_elem *next, *child, *develem;
    int c_dev = 0, c_must, c_uniq;
    int f_min = 0; /* flags */
    int i, j;
    struct ly_deviate *d = NULL;
    struct ly_mnode *mnode = NULL;
    struct ly_mnode_choice *choice = NULL;
    struct ly_mnode_leaf *leaf = NULL;
    struct ly_mnode_list *list = NULL;
    struct ly_type *t = NULL;
    uint8_t *trg_must_size = NULL;
    struct ly_restr **trg_must = NULL;

    GETVAL(value, yin, "target-node");
    dev->target_name = lydict_insert(module->ctx, value, 0);

    /* resolve target node */
    dev->target = resolve_schema_nodeid(dev->target_name, NULL, module, LY_NODE_AUGMENT);
    if (!dev->target) {
        LOGVAL(VE_INARG, LOGLINE(yin), dev->target_name, yin->name);
        goto error;
    }
    if (dev->target->module == module) {
        LOGVAL(VE_SPEC, LOGLINE(yin), "Deviating own module is not allowed.");
        goto error;
    }
    /* mark the target module as deviated */
    dev->target->module->deviated = 1;

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, child);
            continue;
        }

        if (!strcmp(child->name, "description")) {
            if (dev->dsc) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            dev->dsc = read_yin_subnode(module->ctx, child, "text");
            if (!dev->dsc) {
                goto error;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (dev->ref) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            dev->ref = read_yin_subnode(module->ctx, child, "text");
            if (!dev->ref) {
                goto error;
            }
        } else if (!strcmp(child->name, "deviate")) {
            c_dev++;

            /* skip lyxml_free_elem() at the end of the loop, node will be
             * further processed later
             */
            continue;

        } else {
            LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }

        lyxml_free_elem(module->ctx, child);
    }

    if (c_dev) {
        dev->deviate = calloc(c_dev, sizeof *dev->deviate);
    }

    LY_TREE_FOR(yin->child, develem) {
        /* init */
        f_min = 0;
        c_must = 0;
        c_uniq = 0;

        /* get deviation type */
        GETVAL(value, develem, "value");
        if (!strcmp(value, "not-supported")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_NO;
            /* no property expected in this case */
            if (develem->child) {
                LOGVAL(VE_INSTMT, LOGLINE(develem->child), develem->child->name);
                goto error;
            }

            /* and neither any other deviate statement is expected,
             * not-supported deviation must be the only deviation of the target
             */
            if (dev->deviate_size || develem->next) {
                LOGVAL(VE_INARG, LOGLINE(develem), value, develem->name);
                LOGVAL(VE_SPEC, 0, "\"not-supported\" deviation cannot be combined with any other deviation.");
                goto error;
            }


            /* remove target node */
            ly_mnode_free(dev->target);
            dev->target = NULL;

            dev->deviate_size = 1;
            return EXIT_SUCCESS;
        } else if (!strcmp(value, "add")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_ADD;
        } else if (!strcmp(value, "replace")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_RPL;
        } else if (!strcmp(value, "delete")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_DEL;
        } else {
            LOGVAL(VE_INARG, LOGLINE(develem), value, develem->name);
            goto error;
        }
        d = &dev->deviate[dev->deviate_size];

        /* process deviation properties */
        LY_TREE_FOR_SAFE(develem->child, next, child) {
            if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, child);
                continue;
            }

            if (!strcmp(child->name, "config")) {
                if (d->flags & LY_NODE_CONFIG_MASK) {
                    LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }

                /* for we deviate from RFC 6020 and allow config property even it is/is not
                 * specified in the target explicitly since config property inherits. So we expect
                 * that config is specified in every node. But for delete, we check that the value
                 * is the same as here in deviation
                 */
                GETVAL(value, child, "value");
                if (!strcmp(value, "false")) {
                    d->flags |= LY_NODE_CONFIG_R;
                } else if (!strcmp(value, "true")) {
                    d->flags |= LY_NODE_CONFIG_W;
                } else {
                    LOGVAL(VE_INARG, LOGLINE(child), value, child->name);
                    goto error;
                }

                if (d->mod == LY_DEVIATE_DEL) {
                    /* check values */
                    if ((d->flags & LY_NODE_CONFIG_MASK) != (dev->target->flags & LY_NODE_CONFIG_MASK)) {
                        LOGVAL(VE_INARG, LOGLINE(child), value, child->name);
                        LOGVAL(VE_SPEC, 0, "Value differs from the target being deleted.");
                        goto error;
                    }
                    /* remove current config value of the target ... */
                    dev->target->flags &= ~LY_NODE_CONFIG_MASK;

                    /* ... and inherit config value from the target's parent */
                    if (dev->target->parent) {
                        dev->target->flags |= dev->target->parent->flags & LY_NODE_CONFIG_MASK;
                    } else {
                        /* default config is true */
                        dev->target->flags |= LY_NODE_CONFIG_W;
                    }
                } else { /* add and replace are the same in this case */
                    /* remove current config value of the target ... */
                    dev->target->flags &= ~LY_NODE_CONFIG_MASK;

                    /* ... and replace it with the value specified in deviation */
                    dev->target->flags |= d->flags & LY_NODE_CONFIG_MASK;
                }
            } else if (!strcmp(child->name, "default")) {
                if (d->dflt) {
                    LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }
                GETVAL(value, child, "value");
                d->dflt = lydict_insert(module->ctx, value, 0);

                if (dev->target->nodetype == LY_NODE_CHOICE) {
                    choice = (struct ly_mnode_choice *)dev->target;

                    if (d->mod == LY_DEVIATE_ADD) {
                        /* check that there is no current value */
                        if (choice->dflt) {
                            LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                            LOGVAL(VE_SPEC, 0, "Adding property that already exists.");
                            goto error;
                        }
                    }

                    mnode = resolve_schema_nodeid(d->dflt, (struct ly_mnode *)choice, choice->module, LY_NODE_CHOICE);
                    if (d->mod == LY_DEVIATE_DEL) {
                        if (!choice->dflt || choice->dflt != mnode) {
                            LOGVAL(VE_INARG, LOGLINE(child), value, child->name);
                            LOGVAL(VE_SPEC, 0, "Value differs from the target being deleted.");
                            goto error;
                        }
                    } else { /* add (already checked) and replace */
                        choice->dflt = mnode;
                        if (!choice->dflt) {
                            /* default branch not found */
                            LOGVAL(VE_INARG, LOGLINE(yin), value, "default");
                            goto error;
                        }
                    }
                } else if (dev->target->nodetype == LY_NODE_LEAF) {
                    leaf = (struct ly_mnode_leaf *)dev->target;

                    if (d->mod == LY_DEVIATE_ADD) {
                        /* check that there is no current value */
                        if (leaf->dflt) {
                            LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                            LOGVAL(VE_SPEC, 0, "Adding property that already exists.");
                            goto error;
                        }
                    }

                    if (d->mod == LY_DEVIATE_DEL) {
                        if (!leaf->dflt || leaf->dflt != d->dflt) {
                            LOGVAL(VE_INARG, LOGLINE(child), value, child->name);
                            LOGVAL(VE_SPEC, 0, "Value differs from the target being deleted.");
                            goto error;
                        }
                        /* remove value */
                        lydict_remove(leaf->module->ctx, leaf->dflt);
                        leaf->dflt = NULL;
                    } else { /* add (already checked) and replace */
                        /* remove value */
                        lydict_remove(leaf->module->ctx, leaf->dflt);

                        /* set new value */
                        leaf->dflt = lydict_insert(leaf->module->ctx, d->dflt, 0);
                    }
                } else {
                    /* invalid target for default value */
                    LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(VE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }
            } else if (!strcmp(child->name, "mandatory")) {
                if (d->flags & LY_NODE_MAND_MASK) {
                    LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }

                /* check target node type */
                if (!(dev->target->nodetype &= (LY_NODE_LEAF | LY_NODE_CHOICE | LY_NODE_ANYXML))) {
                    LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(VE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                GETVAL(value, child, "value");
                if (!strcmp(value, "false")) {
                    d->flags |= LY_NODE_MAND_FALSE;
                } else if (!strcmp(value, "true")) {
                    d->flags |= LY_NODE_MAND_TRUE;
                } else {
                    LOGVAL(VE_INARG, LOGLINE(child), value, child->name);
                    goto error;
                }

                if (d->mod == LY_DEVIATE_ADD) {
                    /* check that there is no current value */
                    if (dev->target->flags & LY_NODE_MAND_MASK) {
                        LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                        LOGVAL(VE_SPEC, 0, "Adding property that already exists.");
                        goto error;
                    }
                }

                if (d->mod == LY_DEVIATE_DEL) {
                    /* check values */
                    if ((d->flags & LY_NODE_MAND_MASK) != (dev->target->flags & LY_NODE_MAND_MASK)) {
                        LOGVAL(VE_INARG, LOGLINE(child), value, child->name);
                        LOGVAL(VE_SPEC, 0, "Value differs from the target being deleted.");
                        goto error;
                    }
                    /* remove current mandatory value of the target */
                    dev->target->flags &= ~LY_NODE_MAND_MASK;
                } else { /* add (already checked) and replace */
                    /* remove current mandatory value of the target ... */
                    dev->target->flags &= ~LY_NODE_MAND_MASK;

                    /* ... and replace it with the value specified in deviation */
                    dev->target->flags |= d->flags & LY_NODE_MAND_MASK;
                }
            } else if (!strcmp(child->name, "min-elements")) {
                if (f_min) {
                    LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }
                f_min = 1;

                if (deviate_minmax(dev->target, child, d, 0)) {
                    goto error;
                }
            } else if (!strcmp(child->name, "max-elements")) {
                if (d->max) {
                    LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }

                if (deviate_minmax(dev->target, child, d, 1)) {
                    goto error;
                }
            } else if (!strcmp(child->name, "must")) {
                c_must++;
                /* skip lyxml_free_elem() at the end of the loop, this node will be processed later */
                continue;
            } else if (!strcmp(child->name, "type")) {
                if (d->type) {
                    LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }

                /* check target node type */
                if (dev->target->nodetype == LY_NODE_LEAF) {
                    t = &((struct ly_mnode_leaf *)dev->target)->type;
                } else if (dev->target->nodetype == LY_NODE_LEAFLIST) {
                    t = &((struct ly_mnode_leaflist *)dev->target)->type;
                } else {
                    LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(VE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                if (d->mod == LY_DEVIATE_ADD) {
                    /* not allowed, type is always present at the target */
                    LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(VE_SPEC, 0, "Adding property that already exists.");
                    goto error;
                } else if (d->mod == LY_DEVIATE_DEL) {
                    /* not allowed, type cannot be deleted from the target */
                    LOGVAL(VE_INARG, LOGLINE(child), value, child->name);
                    LOGVAL(VE_SPEC, 0, "Deleteing type from the target is not allowed.");
                    goto error;
                }

                /* replace */
                /* remove current units value of the target ... */
                ly_type_free(dev->target->module->ctx, t);

                /* ... and replace it with the value specified in deviation */
                if (fill_yin_type(module, dev->target, child, t, NULL)) {
                    goto error;
                }
                d->type = t;
            } else if (!strcmp(child->name, "unique")) {
                c_uniq++;
                /* skip lyxml_free_elem() at the end of the loop, this node will be processed later */
                continue;
            } else if (!strcmp(child->name, "units")) {
                if (d->units) {
                    LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }

                /* check target node type */
                if (dev->target->nodetype == LY_NODE_LEAFLIST) {
                    stritem = &((struct ly_mnode_leaflist *)dev->target)->units;
                } else if (dev->target->nodetype == LY_NODE_LEAF) {
                    stritem = &((struct ly_mnode_leaf *)dev->target)->units;
                } else {
                    LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(VE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                /* get units value */
                GETVAL(value, child, "name");
                d->units = lydict_insert(module->ctx, value, 0);

                /* apply to target */
                if (d->mod == LY_DEVIATE_ADD) {
                    /* check that there is no current value */
                    if (*stritem) {
                        LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                        LOGVAL(VE_SPEC, 0, "Adding property that already exists.");
                        goto error;
                    }
                }

                if (d->mod == LY_DEVIATE_DEL) {
                    /* check values */
                    if (*stritem != d->units) {
                        LOGVAL(VE_INARG, LOGLINE(child), value, child->name);
                        LOGVAL(VE_SPEC, 0, "Value differs from the target being deleted.");
                        goto error;
                    }
                    /* remove current units value of the target */
                    lydict_remove(dev->target->module->ctx, *stritem);
                } else { /* add (already checked) and replace */
                    /* remove current units value of the target ... */
                    lydict_remove(dev->target->module->ctx, *stritem);

                    /* ... and replace it with the value specified in deviation */
                    *stritem = lydict_insert(module->ctx, value, 0);
                }
            } else {
                LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                goto error;
            }

            lyxml_free_elem(module->ctx, child);
        }

        if (c_must) {
            /* check target node type */
            switch (dev->target->nodetype) {
            case LY_NODE_LEAF:
                trg_must = &((struct ly_mnode_leaf *)dev->target)->must;
                trg_must_size = &((struct ly_mnode_leaf *)dev->target)->must_size;
                break;
            case LY_NODE_CONTAINER:
                trg_must = &((struct ly_mnode_container *)dev->target)->must;
                trg_must_size = &((struct ly_mnode_container *)dev->target)->must_size;
                break;
            case LY_NODE_LEAFLIST:
                trg_must = &((struct ly_mnode_leaflist *)dev->target)->must;
                trg_must_size = &((struct ly_mnode_leaflist *)dev->target)->must_size;
                break;
            case LY_NODE_LIST:
                trg_must = &((struct ly_mnode_list *)dev->target)->must;
                trg_must_size = &((struct ly_mnode_list *)dev->target)->must_size;
                break;
            case LY_NODE_ANYXML:
                trg_must = &((struct ly_mnode_anyxml *)dev->target)->must;
                trg_must_size = &((struct ly_mnode_anyxml *)dev->target)->must_size;
                break;
            default:
                LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                LOGVAL(VE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                goto error;
            }

            if (d->mod == LY_DEVIATE_RPL) {
                /* remove target's musts and allocate new array for it */
                if (!*trg_must) {
                    LOGVAL(VE_INARG, LOGLINE(develem), "replace", "deviate");
                    LOGVAL(VE_SPEC, 0, "Property \"must\" to replace does not exists in target.");
                    goto error;
                }

                for (i = 0; i < list->must_size; i++) {
                    ly_restr_free(dev->target->module->ctx, &(*trg_must[i]));
                }
                free(*trg_must);
                *trg_must = d->must = calloc(c_must, sizeof *d->must);
                d->must_size = c_must;
                *trg_must_size = 0;
            } else if (d->mod == LY_DEVIATE_ADD) {
                /* reallocate the must array of the target */
                d->must = realloc(*trg_must, (c_must + *trg_must_size) * sizeof *d->must);
                *trg_must = d->must;
                d->must = &(*trg_must[*trg_must_size]);
                d->must_size = c_must;
            } else { /* LY_DEVIATE_DEL */
                d->must = calloc(c_must, sizeof *d->must);
            }
        }
        if (c_uniq) {
            /* check target node type */
            if (dev->target->nodetype != LY_NODE_LIST) {
                LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                LOGVAL(VE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                goto error;
            }

            list = (struct ly_mnode_list *)dev->target;
            if (d->mod == LY_DEVIATE_RPL) {
                /* remove target's unique and allocate new array for it */
                if (!list->unique) {
                    LOGVAL(VE_INARG, LOGLINE(develem), "replace", "deviate");
                    LOGVAL(VE_SPEC, 0, "Property \"unique\" to replace does not exists in target.");
                    goto error;
                }

                for (i = 0; i < list->unique_size; i++) {
                    free(list->unique[i].leafs);
                }
                free(list->unique);
                list->unique = d->unique = calloc(c_uniq, sizeof *d->unique);
                d->unique_size = c_uniq;
                list->unique_size = 0;
            } else if (d->mod == LY_DEVIATE_ADD) {
                /* reallocate the unique array of the target */
                d->unique = realloc(list->unique, (c_uniq + list->unique_size) * sizeof *d->unique);
                list->unique = d->unique;
                d->unique = &list->unique[list->unique_size];
                d->unique_size = c_uniq;
            } else { /* LY_DEVIATE_DEL */
                d->unique = calloc(c_uniq, sizeof *d->unique);
            }
        }

        /* process deviation properties with 0..n cardinality */
        LY_TREE_FOR(develem->child, child) {
            if (!strcmp(child->name, "must")) {
                if (d->mod == LY_DEVIATE_DEL) {
                    if (fill_yin_must(module, child, &d->must[d->must_size])) {
                        goto error;
                    }

                    /* find must to delete, we are ok with just matching conditions */
                    for (i = 0; i < *trg_must_size; i++) {
                        if (d->must[d->must_size].expr == (*trg_must)[i].expr) {
                            /* we have a match, free the must structure ... */
                            ly_restr_free(dev->target->module->ctx, &(*trg_must[i]));
                            /* ... and maintain the array */
                            (*trg_must_size)--;
                            if (i != *trg_must_size) {
                                (*trg_must)[i].expr = (*trg_must)[*trg_must_size].expr;
                                (*trg_must)[i].dsc = (*trg_must)[*trg_must_size].dsc;
                                (*trg_must)[i].ref = (*trg_must)[*trg_must_size].ref;
                                (*trg_must)[i].eapptag = (*trg_must)[*trg_must_size].eapptag;
                                (*trg_must)[i].emsg = (*trg_must)[*trg_must_size].emsg;
                            }
                            if (!(*trg_must_size)) {
                                free(*trg_must);
                                *trg_must = NULL;
                            } else {
                                (*trg_must)[*trg_must_size].expr = NULL;
                                (*trg_must)[*trg_must_size].dsc = NULL;
                                (*trg_must)[*trg_must_size].ref = NULL;
                                (*trg_must)[*trg_must_size].eapptag = NULL;
                                (*trg_must)[*trg_must_size].emsg = NULL;
                            }

                            i = -1; /* set match flag */
                            break;
                        }
                    }
                    d->must_size++;
                    if (i != -1) {
                        /* no match found */
                        LOGVAL(VE_INARG, LOGLINE(child), d->must[d->must_size - 1].expr, child->name);
                        LOGVAL(VE_SPEC, 0, "Value does not match any must from the target.");
                        goto error;
                    }
                } else { /* replace or add */
                    if (fill_yin_must(dev->target->module, child, &((*trg_must)[*trg_must_size]))) {
                        goto error;
                    }
                    (*trg_must_size)++;
                }
            } else if (!strcmp(child->name, "unique")) {
                if (d->mod == LY_DEVIATE_DEL) {
                    if (parse_unique(dev->target, child, &d->unique[d->unique_size])) {
                        goto error;
                    }

                    /* find unique structures to delete */
                    for (i = 0; i < list->unique_size; i++) {
                        if (list->unique[i].leafs_size != d->unique[d->unique_size].leafs_size) {
                            continue;
                        }

                        for (j = 0; j < d->unique[d->unique_size].leafs_size; j++) {
                            if (list->unique[i].leafs[j] != d->unique[d->unique_size].leafs[j]) {
                                break;
                            }
                        }

                        if (j == d->unique[d->unique_size].leafs_size) {
                            /* we have a match, free the unique structure ... */
                            free(list->unique[i].leafs);
                            /* ... and maintain the array */
                            list->unique_size--;
                            if (i != list->unique_size) {
                                list->unique[i].leafs_size = list->unique[list->unique_size].leafs_size;
                                list->unique[i].leafs = list->unique[list->unique_size].leafs;
                            }

                            if (!list->unique_size) {
                                free(list->unique);
                                list->unique = NULL;
                            } else {
                                list->unique[list->unique_size].leafs_size = 0;
                                list->unique[list->unique_size].leafs = NULL;
                            }

                            i = -1; /* set match flag */
                            break;
                        }
                    }

                    d->unique_size++;
                    if (i != -1) {
                        /* no match found */
                        LOGVAL(VE_INARG, LOGLINE(child), lyxml_get_attr(child, "tag", NULL), child->name);
                        LOGVAL(VE_SPEC, 0, "Value differs from the target being deleted.");
                        goto error;
                    }
                } else { /* replace or add */
                    if (parse_unique(dev->target, child, &list->unique[list->unique_size])) {
                        goto error;
                    }
                    list->unique_size++;
                }
            }
        }

        dev->deviate_size++;
    }

    return EXIT_SUCCESS;

error:

    if (dev->deviate) {
        for (i = 0; i < dev->deviate_size; i++) {
            lydict_remove(module->ctx, dev->deviate[i].dflt);
            lydict_remove(module->ctx, dev->deviate[i].units);

            if (dev->deviate[i].mod == LY_DEVIATE_DEL) {
                for (j = 0; j < dev->deviate[i].must_size; j++) {
                    ly_restr_free(module->ctx, &dev->deviate[i].must[j]);
                }
                free(dev->deviate[i].must);

                for (j = 0; j < dev->deviate[i].unique_size; j++) {
                    free(dev->deviate[i].unique[j].leafs);
                }
                free(dev->deviate[i].unique);
            }
        }
        free(dev->deviate);
    }

    return EXIT_FAILURE;
}

static int
fill_yin_augment(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin, struct ly_augment *aug)
{
    const char *value;
    struct lyxml_elem *next, *child;
    int c = 0;

    GETVAL(value, yin, "target-node");
    aug->target_name = lydict_insert(module->ctx, value, 0);
    aug->parent = parent;

    if (read_yin_common(module, NULL, (struct ly_mnode *)aug, yin, OPT_NACMEXT)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, child);
            continue;
        }

        if (!strcmp(child->name, "if-feature")) {
            c++;
        } else if (!strcmp(child->name, "when")) {
            if (aug->when) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }

            aug->when = read_yin_when(module, child);
            lyxml_free_elem(module->ctx, child);

            if (!aug->when) {
                goto error;
            }

        /* check allowed sub-statements */
        } else if (strcmp(child->name, "anyxml") && strcmp(child->name, "case") && strcmp(child->name, "choice") &&
                strcmp(child->name, "container") && strcmp(child->name, "leaf-list") && strcmp(child->name, "leaf") &&
                strcmp(child->name, "list") && strcmp(child->name, "uses")) {
            LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }
    }

    if (c) {
        aug->features = calloc(c, sizeof *aug->features);
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!strcmp(child->name, "if-feature")) {
            GETVAL(value, child, "name");
            aug->features[aug->features_size] = resolve_feature(value, module, LOGLINE(child));
            if (!aug->features[aug->features_size]) {
                goto error;
            }
            aug->features_size++;
            lyxml_free_elem(module->ctx, child);
        }

        /* data nodes are still kept under yin */
    }

    /* do not resolve data now, just keep the definition which will be parsed later
     * when we will have the target node
     */
    lyxml_unlink_elem(yin);
    aug->child = (struct ly_mnode *)yin;

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static int
fill_yin_refine(struct ly_module *module, struct lyxml_elem *yin, struct ly_refine *rfn)
{
    struct lyxml_elem *sub, *next;
    const char *value;
    char *endptr;
    int f_mand = 0, f_min = 0, f_max = 0;
    int c_must = 0;
    int r;
    unsigned long int val;

    GETVAL(value, yin, "target-node");
    rfn->target = lydict_insert(module->ctx, value, strlen(value));

    if (read_yin_common(module, NULL, (struct ly_mnode *)rfn, yin, OPT_CONFIG)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        /* limited applicability */
        if (!strcmp(sub->name, "default")) {
            /* leaf or choice */
            if (rfn->mod.dflt) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LY_NODE_LEAF | LY_NODE_CHOICE);
                if (!rfn->target_type) {
                    LOGVAL(VE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
                    goto error;
                }
            } else {
                rfn->target_type = LY_NODE_LEAF | LY_NODE_CHOICE;
            }

            GETVAL(value, sub, "value");
            rfn->mod.dflt = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "mandatory")) {
            /* leaf, choice or anyxml */
            if (f_mand) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in leaf is not sufficient, we would allow
             * multiple mandatory statements with the "false" value
             */
            f_mand = 1;

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LY_NODE_LEAF | LY_NODE_CHOICE | LY_NODE_ANYXML);
                if (!rfn->target_type) {
                    LOGVAL(VE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
                    goto error;
                }
            } else {
                rfn->target_type = LY_NODE_LEAF | LY_NODE_CHOICE | LY_NODE_ANYXML;
            }

            GETVAL(value, sub, "value");
            if (!strcmp(value, "true")) {
                rfn->flags |= LY_NODE_MAND_TRUE;
            } else if (!strcmp(value, "false")) {
                rfn->flags |= LY_NODE_MAND_FALSE;
            } else {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
        } else if (!strcmp(sub->name, "min-elements")) {
            /* list or leaf-list */
            if (f_min) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_min = 1;

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LY_NODE_LIST | LY_NODE_LEAFLIST);
                if (!rfn->target_type) {
                    LOGVAL(VE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
                    goto error;
                }
            } else {
                rfn->target_type = LY_NODE_LIST | LY_NODE_LEAFLIST;
            }

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            endptr = NULL;
            val = strtoul(value, &endptr, 10);
            if (*endptr || value[0] == '-' || errno || val > UINT32_MAX) {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            rfn->mod.list.min = (uint32_t) val;

            /* magic - bit 3 in flags means min set */
            rfn->flags |= 0x04;
        } else if (!strcmp(sub->name, "max-elements")) {
            /* list or leaf-list */
            if (f_max) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_max = 1;

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LY_NODE_LIST | LY_NODE_LEAFLIST);
                if (!rfn->target_type) {
                    LOGVAL(VE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
                    goto error;
                }
            } else {
                rfn->target_type = LY_NODE_LIST | LY_NODE_LEAFLIST;
            }

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            endptr = NULL;
            val = strtoul(value, &endptr, 10);
            if (*endptr || value[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            rfn->mod.list.max = (uint32_t) val;

            /* magic - bit 4 in flags means min set */
            rfn->flags |= 0x08;
        } else if (!strcmp(sub->name, "presence")) {
            /* container */
            if (rfn->mod.presence) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= LY_NODE_CONTAINER;
                if (!rfn->target_type) {
                    LOGVAL(VE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
                    goto error;
                }
            } else {
                rfn->target_type = LY_NODE_CONTAINER;
            }

            GETVAL(value, sub, "value");
            rfn->mod.presence = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "must")) {
            /* leaf-list, list, container or anyxml */
            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LY_NODE_LIST | LY_NODE_LEAFLIST | LY_NODE_CONTAINER | LY_NODE_ANYXML);
                if (!rfn->target_type) {
                    LOGVAL(VE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
                    goto error;
                }
            } else {
                rfn->target_type = LY_NODE_LIST | LY_NODE_LEAFLIST | LY_NODE_CONTAINER | LY_NODE_ANYXML;
            }

            c_must++;
            continue;

        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    /* process nodes with cardinality of 0..n */
    if (c_must) {
        rfn->must = calloc(c_must, sizeof *rfn->must);
    }
    LY_TREE_FOR(yin->child, sub) {
        r = fill_yin_must(module, sub, &rfn->must[rfn->must_size]);
        rfn->must_size++;

        if (r) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static int
fill_yin_import(struct ly_module *module, struct lyxml_elem *yin, struct ly_import *imp)
{
    struct lyxml_elem *child;
    const char *value;

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(child->name, "prefix")) {
            GETVAL(value, child, "value");
            if (check_identifier(value, LY_IDENT_PREFIX, LOGLINE(child), module, NULL)) {
                goto error;
            }
            imp->prefix = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(child->name, "revision-date")) {
            if (imp->rev[0]) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), "revision-date", yin->name);
                goto error;
            }
            GETVAL(value, child, "date");
            if (check_date(value, LOGLINE(child))) {
                goto error;
            }
            memcpy(imp->rev, value, LY_REV_SIZE - 1);
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }
    }

    /* check mandatory information */
    if (!imp->prefix) {
        LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "prefix", yin->name);
        goto error;
    }

    GETVAL(value, yin, "module");
    imp->module = ly_ctx_get_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL, 1);
    if (!imp->module) {
        LOGERR(LY_EVALID, "Importing \"%s\" module into \"%s\" failed.", value, module->name);
        LOGVAL(VE_INARG, LOGLINE(yin), value, yin->name);
        goto error;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static int
fill_yin_include(struct ly_module *module, struct lyxml_elem *yin, struct ly_include *inc)
{
    struct lyxml_elem *child;
    const char *value;

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }
        if (!strcmp(child->name, "revision-date")) {
            if (inc->rev[0]) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), "revision-date", yin->name);
                goto error;
            }
            GETVAL(value, child, "date");
            if (check_date(value, LOGLINE(child))) {
                goto error;
            }
            memcpy(inc->rev, value, LY_REV_SIZE - 1);
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }
    }

    GETVAL(value, yin, "module");
    inc->submodule = ly_ctx_get_submodule(module, value, inc->rev[0] ? inc->rev : NULL, 1);
    if (!inc->submodule) {
        LOGERR(LY_EVALID, "Including \"%s\" module into \"%s\" failed.", value, module->name);
        LOGVAL(VE_INARG, LOGLINE(yin), value, yin->name);
        goto error;
    }

    /* check that belongs-to corresponds */
    if (module->type) {
        module = ((struct ly_submodule *)module)->belongsto;
    }
    if (inc->submodule->belongsto != module) {
        LOGVAL(VE_INARG, LOGLINE(yin), value, yin->name);
        LOGVAL(VE_SPEC, 0, "The included module does not belongs-to the \"%s\" module", module->name);
        goto error;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/*
 * Covers:
 * description, reference, status, optionaly config
 *
 */
static int
read_yin_common(struct ly_module *module, struct ly_mnode *parent,
                struct ly_mnode *mnode, struct lyxml_elem *xmlnode, int opt)
{
    const char *value;
    struct lyxml_elem *sub, *next;
    struct ly_ctx *const ctx = module->ctx;

    if (opt & OPT_MODULE) {
        mnode->module = module;
    }

    if (opt & OPT_IDENT) {
        GETVAL(value, xmlnode, "name");
        if (check_identifier(value, LY_IDENT_NAME, LOGLINE(xmlnode), NULL, NULL)) {
            goto error;
        }
        mnode->name = lydict_insert(ctx, value, strlen(value));
    }

    /* inherit NACM flags */
    if ((opt & OPT_NACMEXT) && parent) {
        mnode->nacm = parent->nacm;
    }

    /* process local parameters */
    LY_TREE_FOR_SAFE(xmlnode->child, next, sub) {
        if (!sub->ns) {
            /* garbage */
            lyxml_free_elem(ctx, sub);
            continue;
        }
        if  (strcmp(sub->ns->value, LY_NSYIN)) {
            /* NACM extensions */
            if ((opt & OPT_NACMEXT) && !strcmp(sub->ns->value, LY_NSNACM)) {
                if (!strcmp(sub->name, "default-deny-write")) {
                    mnode->nacm |= LY_NACM_DENYW;
                } else if (!strcmp(sub->name, "default-deny-all")) {
                    mnode->nacm |= LY_NACM_DENYA;
                } else {
                    LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
                    goto error;
                }
            }

            /* else garbage */
            lyxml_free_elem(ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "description")) {
            if (mnode->dsc) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
                goto error;
            }
            mnode->dsc = read_yin_subnode(ctx, sub, "text");
            if (!mnode->dsc) {
                goto error;
            }
        } else if (!strcmp(sub->name, "reference")) {
            if (mnode->ref) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
                goto error;
            }
            mnode->ref = read_yin_subnode(ctx, sub, "text");
            if (!mnode->ref) {
                goto error;
            }
        } else if (!strcmp(sub->name, "status")) {
            if (mnode->flags & LY_NODE_STATUS_MASK) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            if (!strcmp(value, "current")) {
                mnode->flags |= LY_NODE_STATUS_CURR;
            } else if (!strcmp(value, "deprecated")) {
                mnode->flags |= LY_NODE_STATUS_DEPRC;
            } else if (!strcmp(value, "obsolete")) {
                mnode->flags |= LY_NODE_STATUS_OBSLT;
            } else {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
        } else if ((opt & OPT_CONFIG) && !strcmp(sub->name, "config")) {
            if (mnode->flags & LY_NODE_CONFIG_MASK) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            if (!strcmp(value, "false")) {
                mnode->flags |= LY_NODE_CONFIG_R;
            } else if (!strcmp(value, "true")) {
                mnode->flags |= LY_NODE_CONFIG_W;
            } else {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
        } else {
            /* skip the lyxml_free_elem */
            continue;
        }
        lyxml_free_elem(ctx, sub);
    }

    if ((opt & OPT_INHERIT) && !(mnode->flags & LY_NODE_CONFIG_MASK)) {
        /* get config flag from parent */
        if (parent) {
            mnode->flags |= parent->flags & LY_NODE_CONFIG_MASK;
        } else {
            /* default config is true */
            mnode->flags |= LY_NODE_CONFIG_W;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static struct ly_when *
read_yin_when(struct ly_module *module,struct lyxml_elem *yin)
{
    struct ly_when *retval = NULL;
    struct lyxml_elem *child;
    const char *value;

    retval = calloc(1, sizeof *retval);

    GETVAL(value, yin, "condition");
    retval->cond = lydict_insert(module->ctx, value, 0);

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(child->name, "description")) {
            if (retval->dsc) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            retval->dsc = read_yin_subnode(module->ctx, child, "text");
            if (!retval->dsc) {
                goto error;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (retval->ref) {
                LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            retval->ref = read_yin_subnode(module->ctx, child, "text");
            if (!retval->ref) {
                goto error;
            }
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }
    }

    return retval;

error:

    ly_mnode_free((struct ly_mnode *)retval);
    return NULL;
}

/* additional check in case statement - the child must be unique across
 * all other case names and its data children
 */
static int
check_branch_id(struct ly_mnode *parent, struct ly_mnode *new, struct ly_mnode *excl, int line)
{
    struct ly_mnode *mnode, *submnode;

    if (new->nodetype == LY_NODE_CHOICE) {
        /* we have nested choice in case, so we need recursion */
        LY_TREE_FOR(new->child, mnode) {
            if (mnode->nodetype == LY_NODE_CASE) {
                LY_TREE_FOR(mnode->child, submnode) {
                    if (check_branch_id(parent, submnode, new, line)) {
                        return EXIT_FAILURE;
                    }
                }
            } else if (check_branch_id(parent, mnode, new, line)) {
                return EXIT_FAILURE;
            }
        }
    } else {
        LY_TREE_FOR(parent->child, mnode) {
            if (mnode == excl) {
                continue;
            }

            if (!strcmp(new->name, mnode->name)) {
                LOGVAL(VE_INID, line, new->name, "duplicated identifier within a choice's cases");
                return EXIT_FAILURE;
            }
            if (mnode->nodetype == LY_NODE_CASE) {
                LY_TREE_FOR(mnode->child, submnode) {
                    if (!strcmp(new->name, submnode->name)) {
                        LOGVAL(VE_INID, line, new->name, "duplicated identifier within a choice's cases");
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

static struct ly_mnode *
read_yin_case(struct ly_module *module,
              struct ly_mnode *parent, struct lyxml_elem *yin, int resolve, struct obj_list **unres)
{
    struct lyxml_elem *sub, *next;
    struct ly_mnode_case *mcase;
    struct ly_mnode *retval, *mnode = NULL;
    int c_ftrs = 0;
    const char *value;

    mcase = calloc(1, sizeof *mcase);
    mcase->nodetype = LY_NODE_CASE;
    mcase->prev = (struct ly_mnode *)mcase;
    retval = (struct ly_mnode *)mcase;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_INHERIT | OPT_NACMEXT)) {
        goto error;
    }

    /* process choice's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "container")) {
            mnode = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            mnode = read_yin_leaflist(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "leaf")) {
            mnode = read_yin_leaf(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "list")) {
            mnode = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            mnode = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            mnode = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            mnode = read_yin_anyxml(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "if-feature")) {
                c_ftrs++;
                /* skip lyxml_free_elem() at the end of the loop, sub is processed later */
                continue;
        } else if (!strcmp(sub->name, "when")) {
            if (mcase->when) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            mcase->when = read_yin_when(module, sub);
            if (!mcase->when) {
                goto error;
            }
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }

        if (!mnode) {
            goto error;
        } else if (check_branch_id(parent, mnode, mnode, LOGLINE(sub))) {
            goto error;
        }

        mnode = NULL;
        lyxml_free_elem(module->ctx, sub);
    }

    if (c_ftrs) {
        mcase->features = calloc(c_ftrs, sizeof *mcase->features);
    }
    LY_TREE_FOR(yin->child, sub) {
        GETVAL(value, sub, "name");
        mcase->features[mcase->features_size] = resolve_feature(value, module, LOGLINE(sub));
        if (!mcase->features[mcase->features_size]) {
            goto error;
        }
        mcase->features_size++;
    }

    /* insert the node into the schema tree */
    if (ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    return retval;

error:

    ly_mnode_free(retval);

    return NULL;
}

static struct ly_mnode *
read_yin_choice(struct ly_module *module,
                struct ly_mnode *parent, struct lyxml_elem *yin, int resolve, struct obj_list **unres)
{
    struct lyxml_elem *sub, *next;
    struct ly_ctx *const ctx = module->ctx;
    struct ly_mnode *retval, *mnode = NULL;
    struct ly_mnode_choice *choice;
    const char *value;
    char *dflt_str = NULL;
    int f_mand = 0, c_ftrs = 0;

    choice = calloc(1, sizeof *choice);
    choice->nodetype = LY_NODE_CHOICE;
    choice->prev = (struct ly_mnode *)choice;
    retval = (struct ly_mnode *)choice;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG | OPT_NACMEXT | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    /* process choice's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "container")) {
            if (!(mnode = read_yin_container(module, retval, sub, resolve, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "leaf-list")) {
            if (!(mnode = read_yin_leaflist(module, retval, sub, resolve))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "leaf")) {
            if (!(mnode = read_yin_leaf(module, retval, sub, resolve))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "list")) {
            if (!(mnode = read_yin_list(module, retval, sub, resolve, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "case")) {
            if (!(mnode = read_yin_case(module, retval, sub, resolve, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "anyxml")) {
            if (!(mnode = read_yin_anyxml(module, retval, sub, resolve))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "default")) {
            if (dflt_str) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            dflt_str = strdup(value);
        } else if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in leaf is not sufficient, we would allow
             * multiple mandatory statements with the "false" value
             */
            f_mand = 1;

            GETVAL(value, sub, "value");
            if (!strcmp(value, "true")) {
                choice->flags |= LY_NODE_MAND_TRUE;
            } else if (!strcmp(value, "false")) {
                choice->flags |= LY_NODE_MAND_FALSE;
            } else {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }                   /* else false is the default value, so we can ignore it */
        } else if (!strcmp(sub->name, "when")) {
            if (choice->when) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            choice->when = read_yin_when(module, sub);
            if (!choice->when) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;

            /* skip lyxml_free_elem() at the end of the loop, the sub node is processed later */
            continue;
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }

        if (mnode && check_branch_id(retval, mnode, mnode, LOGLINE(sub))) {
            goto error;
        }
        mnode = NULL;
        lyxml_free_elem(ctx, sub);
    }

    if (c_ftrs) {
        choice->features = calloc(c_ftrs, sizeof *choice->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        GETVAL(value, sub, "name");
        choice->features[choice->features_size] = resolve_feature(value, module, LOGLINE(sub));
        if (!choice->features[choice->features_size]) {
            goto error;
        }
        choice->features_size++;
    }

    /* check - default is prohibited in combination with mandatory */
    if (dflt_str && (choice->flags & LY_NODE_MAND_TRUE)) {
        LOGVAL(VE_SPEC, LOGLINE(yin),
               "The \"default\" statement MUST NOT be present on choices where \"mandatory\" is true.");
        goto error;
    }

    /* link default with the case */
    if (dflt_str) {
        choice->dflt = resolve_schema_nodeid(dflt_str, retval, module, LY_NODE_CHOICE);
        if (!choice->dflt) {
            /* default branch not found */
            LOGVAL(VE_INARG, LOGLINE(yin), dflt_str, "default");
            goto error;
        }
    }

    /* insert the node into the schema tree */
    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    free(dflt_str);

    return retval;

error:

    ly_mnode_free(retval);
    free(dflt_str);

    return NULL;
}

static struct ly_mnode *
read_yin_anyxml(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin, int resolve)
{
    struct ly_mnode *retval;
    struct ly_mnode_leaf *anyxml;
    struct lyxml_elem *sub, *next;
    const char *value;
    int r;
    int f_mand = 0;
    int c_must = 0, c_ftrs = 0;

    anyxml = calloc(1, sizeof *anyxml);
    anyxml->nodetype = LY_NODE_ANYXML;
    anyxml->prev = (struct ly_mnode *)anyxml;
    retval = (struct ly_mnode *)anyxml;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG | OPT_NACMEXT | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in leaf is not sufficient, we would allow
             * multiple mandatory statements with the "false" value
             */
            f_mand = 1;

            GETVAL(value, sub, "value");
            if (!strcmp(value, "true")) {
                anyxml->flags |= LY_NODE_MAND_TRUE;
            } else if (!strcmp(value, "false")) {
                anyxml->flags |= LY_NODE_MAND_FALSE;
            } else {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            /* else false is the default value, so we can ignore it */
            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (anyxml->when) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            anyxml->when = read_yin_when(module, sub);
            lyxml_free_elem(module->ctx, sub);

            if (!anyxml->when) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            c_must++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;

        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n */
    if (c_must) {
        anyxml->must = calloc(c_must, sizeof *anyxml->must);
    }
    if (c_ftrs) {
        anyxml->features = calloc(c_ftrs, sizeof *anyxml->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &anyxml->must[anyxml->must_size]);
            anyxml->must_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            anyxml->features[anyxml->features_size] = resolve_feature(value, module, LOGLINE(sub));
            if (!anyxml->features[anyxml->features_size]) {
                goto error;
            }
            anyxml->features_size++;
        }
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    return retval;

error:

    ly_mnode_free(retval);

    return NULL;
}

static struct ly_mnode *
read_yin_leaf(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin, int resolve)
{
    struct ly_mnode *retval;
    struct ly_mnode_leaf *leaf;
    struct lyxml_elem *sub, *next;
    const char *value;
    int r;
    int c_must = 0, c_ftrs = 0, f_mand = 0;

    leaf = calloc(1, sizeof *leaf);
    leaf->nodetype = LY_NODE_LEAF;
    leaf->prev = (struct ly_mnode *)leaf;
    retval = (struct ly_mnode *)leaf;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG | OPT_NACMEXT | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "type")) {
            if (leaf->type.der) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            if (fill_yin_type(module, parent, sub, &leaf->type, NULL)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "default")) {
            if (leaf->dflt) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            leaf->dflt = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "units")) {
            if (leaf->units) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "name");
            leaf->units = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in leaf is not sufficient, we would allow
             * multiple mandatory statements with the "false" value
             */
            f_mand = 1;

            GETVAL(value, sub, "value");
            if (!strcmp(value, "true")) {
                leaf->flags |= LY_NODE_MAND_TRUE;
            } else if (!strcmp(value, "false")) {
                leaf->flags |= LY_NODE_MAND_FALSE;
            } else {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }                   /* else false is the default value, so we can ignore it */
        } else if (!strcmp(sub->name, "when")) {
            if (leaf->when) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            leaf->when = read_yin_when(module, sub);
            if (!leaf->when) {
                goto error;
            }

        } else if (!strcmp(sub->name, "must")) {
            c_must++;
            continue;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
            continue;

        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    /* check mandatory parameters */
    if (!leaf->type.der) {
        LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "type", yin->name);
        goto error;
    }
    if (leaf->dflt && check_default(&leaf->type, leaf->dflt)) {
        goto error;
    }

    /* middle part - process nodes with cardinality of 0..n */
    if (c_must) {
        leaf->must = calloc(c_must, sizeof *leaf->must);
    }
    if (c_ftrs) {
        leaf->features = calloc(c_ftrs, sizeof *leaf->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &leaf->must[leaf->must_size]);
            leaf->must_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            leaf->features[leaf->features_size] = resolve_feature(value, module, LOGLINE(sub));
            if (!leaf->features[leaf->features_size]) {
                goto error;
            }
            leaf->features_size++;
        }
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    return retval;

error:

    ly_mnode_free(retval);

    return NULL;
}

static struct ly_mnode *
read_yin_leaflist(struct ly_module *module, struct ly_mnode *parent, struct lyxml_elem *yin, int resolve)
{
    struct ly_mnode *retval;
    struct ly_mnode_leaflist *llist;
    struct lyxml_elem *sub, *next;
    const char *value;
    char *endptr;
    unsigned long val;
    int r;
    int c_must = 0, c_ftrs = 0;
    int f_ordr = 0, f_min = 0, f_max = 0;

    llist = calloc(1, sizeof *llist);
    llist->nodetype = LY_NODE_LEAFLIST;
    llist->prev = (struct ly_mnode *)llist;
    retval = (struct ly_mnode *)llist;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG | OPT_NACMEXT | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "type")) {
            if (llist->type.der) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            if (fill_yin_type(module, parent, sub, &llist->type, NULL)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "units")) {
            if (llist->units) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "name");
            llist->units = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "ordered-by")) {
            if (f_ordr) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in llist is not sufficient, we would
             * allow multiple ordered-by statements with the "system" value
             */
            f_ordr = 1;

            if (llist->flags & LY_NODE_CONFIG_R) {
                /* RFC 6020, 7.7.5 - ignore ordering when the list represents
                 * state data
                 */
                lyxml_free_elem(module->ctx, sub);
                continue;
            }

            GETVAL(value, sub, "value");
            if (!strcmp(value, "user")) {
                llist->flags |= LY_NODE_USERORDERED;
            } else if (strcmp(value, "system")) {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            } /* else system is the default value, so we can ignore it */

        } else if (!strcmp(sub->name, "must")) {
            c_must++;
            continue;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
            continue;

        } else if (!strcmp(sub->name, "min-elements")) {
            if (f_min) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_min = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            endptr = NULL;
            val = strtoul(value, &endptr, 10);
            if (*endptr || value[0] == '-' || errno || val > UINT32_MAX) {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            llist->min = (uint32_t) val;
        } else if (!strcmp(sub->name, "max-elements")) {
            if (f_max) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_max = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            endptr = NULL;
            val = strtoul(value, &endptr, 10);
            if (*endptr || value[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            llist->max = (uint32_t) val;
        } else if (!strcmp(sub->name, "when")) {
            if (llist->when) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            llist->when = read_yin_when(module, sub);
            if (!llist->when) {
                goto error;
            }
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    /* check constraints */
    if (!llist->type.der) {
        LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "type", yin->name);
        goto error;
    }
    if (llist->max && llist->min > llist->max) {
        LOGVAL(VE_SPEC, LOGLINE(yin), "\"min-elements\" is bigger than \"max-elements\".");
        goto error;
    }

    /* middle part - process nodes with cardinality of 0..n */
    if (c_must) {
        llist->must = calloc(c_must, sizeof *llist->must);
    }
    if (c_ftrs) {
        llist->features = calloc(c_ftrs, sizeof *llist->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &llist->must[llist->must_size]);
            llist->must_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            llist->features[llist->features_size] = resolve_feature(value, module, LOGLINE(sub));
            if (!llist->features[llist->features_size]) {
                goto error;
            }
            llist->features_size++;
        }
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    return retval;

error:

    ly_mnode_free(retval);

    return NULL;
}

static struct ly_mnode *
read_yin_list(struct ly_module *module,
              struct ly_mnode *parent, struct lyxml_elem *yin, int resolve, struct obj_list **unres)
{
    struct ly_mnode *retval, *mnode;
    struct ly_mnode_list *list;
    struct lyxml_elem *sub, *next, root, uniq;
    int i, r;
    size_t len;
    int c_tpdf = 0, c_must = 0, c_uniq = 0, c_ftrs = 0;
    int f_ordr = 0, f_max = 0, f_min = 0;
    const char *key_str = NULL, *value;
    char *auxs;
    unsigned long val;

    /* init */
    memset(&root, 0, sizeof root);
    memset(&uniq, 0, sizeof uniq);

    list = calloc(1, sizeof *list);
    list->nodetype = LY_NODE_LIST;
    list->prev = (struct ly_mnode *)list;
    retval = (struct ly_mnode *)list;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG | OPT_NACMEXT | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    /* process list's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        /* data statements */
        if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(sub);
            lyxml_add_child(&root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "key")) {
            /* check cardinality 0..1 */
            if (list->keys_size) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, list->name);
                goto error;
            }

            /* count the number of keys */
            GETVAL(value, sub, "value");
            key_str = value;
            while ((value = strpbrk(value, " \t\n"))) {
                list->keys_size++;
                while (isspace(*value)) {
                    value++;
                }
            }
            list->keys_size++;
            list->keys = calloc(list->keys_size, sizeof *list->keys);
        } else if (!strcmp(sub->name, "unique")) {
            c_uniq++;
            lyxml_unlink_elem(sub);
            lyxml_add_child(&uniq, sub);
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(sub->name, "must")) {
            c_must++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;

            /* optional stetments */
        } else if (!strcmp(sub->name, "ordered-by")) {
            if (f_ordr) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in llist is not sufficient, we would
             * allow multiple ordered-by statements with the "system" value
             */
            f_ordr = 1;

            if (list->flags & LY_NODE_CONFIG_R) {
                /* RFC 6020, 7.7.5 - ignore ordering when the list represents
                 * state data
                 */
                lyxml_free_elem(module->ctx, sub);
                continue;
            }

            GETVAL(value, sub, "value");
            if (!strcmp(value, "user")) {
                list->flags |= LY_NODE_USERORDERED;
            } else if (strcmp(value, "system")) {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            /* else system is the default value, so we can ignore it */
            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "min-elements")) {
            if (f_min) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_min = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            auxs = NULL;
            val = strtoul(value, &auxs, 10);
            if (*auxs || value[0] == '-' || errno || val > UINT32_MAX) {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            list->min = (uint32_t) val;
            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "max-elements")) {
            if (f_max) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_max = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            auxs = NULL;
            val = strtoul(value, &auxs, 10);
            if (*auxs || value[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            list->max = (uint32_t) val;
            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (list->when) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            list->when = read_yin_when(module, sub);
            lyxml_free_elem(module->ctx, sub);

            if (!list->when) {
                goto error;
            }
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* check - if list is configuration, key statement is mandatory */
    if ((list->flags & LY_NODE_CONFIG_W) && !key_str) {
        LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "key", "list");
        goto error;
    }
    if (list->max && list->min > list->max) {
        LOGVAL(VE_SPEC, LOGLINE(yin), "\"min-elements\" is bigger than \"max-elements\".");
        goto error;
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        list->tpdf = calloc(c_tpdf, sizeof *list->tpdf);
    }
    if (c_must) {
        list->must = calloc(c_must, sizeof *list->must);
    }
    if (c_ftrs) {
        list->features = calloc(c_ftrs, sizeof *list->features);
    }
    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &list->tpdf[list->tpdf_size], NULL);
            list->tpdf_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            list->features[list->features_size] = resolve_feature(value, module, LOGLINE(sub));
            if (!list->features[list->features_size]) {
                goto error;
            }
            list->features_size++;
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &list->must[list->must_size]);
            list->must_size++;

            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            mnode = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            mnode = read_yin_leaflist(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "leaf")) {
            mnode = read_yin_leaf(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "list")) {
            mnode = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            mnode = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            mnode = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            mnode = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            mnode = read_yin_anyxml(module, retval, sub, resolve);
        }
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    if (!key_str) {
        /* config false list without a key */
        return retval;
    }

    /* link key leafs into the list structure and check all constraints  */
    for (i = 0; i < list->keys_size; i++) {
        /* get the key name */
        if ((value = strpbrk(key_str, " \t\n"))) {
            len = value - key_str;
            while (isspace(*value)) {
                value++;
            }
        } else {
            len = strlen(key_str);
        }

        list->keys[i] = find_leaf(retval, key_str, len);

        if (check_key(list->keys[i], list->flags, list->keys, i, LOGLINE(yin), key_str, len)) {
            goto error;
        }

        /* prepare for next iteration */
        while (value && isspace(*value)) {
            value++;
        }
        key_str = value;
    }

    /* process unique statements */
    if (c_uniq) {
        list->unique = calloc(c_uniq, sizeof *list->unique);
    }
    LY_TREE_FOR_SAFE(uniq.child, next, sub) {
        if (parse_unique(retval, sub, &list->unique[list->unique_size++])) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    return retval;

error:

    ly_mnode_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }
    while (uniq.child) {
        lyxml_free_elem(module->ctx, uniq.child);
    }

    return NULL;
}

static struct ly_mnode *
read_yin_container(struct ly_module *module,
                   struct ly_mnode *parent, struct lyxml_elem *yin, int resolve, struct obj_list **unres)
{
    struct lyxml_elem *sub, *next, root;
    struct ly_mnode *mnode = NULL;
    struct ly_mnode *retval;
    struct ly_mnode_container *cont;
    const char *value;
    int r;
    int c_tpdf = 0, c_must = 0, c_ftrs = 0;

    /* init */
    memset(&root, 0, sizeof root);

    cont = calloc(1, sizeof *cont);
    cont->nodetype = LY_NODE_CONTAINER;
    cont->prev = (struct ly_mnode *)cont;
    retval = (struct ly_mnode *)cont;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG | OPT_NACMEXT | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    /* process container's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "presence")) {
            if (cont->presence) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            cont->presence = lydict_insert(module->ctx, value, strlen(value));

            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (cont->when) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            cont->when = read_yin_when(module, sub);
            lyxml_free_elem(module->ctx, sub);

            if (!cont->when) {
                goto error;
            }

            /* data statements */
        } else if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(sub);
            lyxml_add_child(&root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(sub->name, "must")) {
            c_must++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        cont->tpdf = calloc(c_tpdf, sizeof *cont->tpdf);
    }
    if (c_must) {
        cont->must = calloc(c_must, sizeof *cont->must);
    }
    if (c_ftrs) {
        cont->features = calloc(c_ftrs, sizeof *cont->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &cont->tpdf[cont->tpdf_size], NULL);
            cont->tpdf_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &cont->must[cont->must_size]);
            cont->must_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            cont->features[cont->features_size] = resolve_feature(value, module, LOGLINE(sub));
            if (!cont->features[cont->features_size++]) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            mnode = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            mnode = read_yin_leaflist(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "leaf")) {
            mnode = read_yin_leaf(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "list")) {
            mnode = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            mnode = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            mnode = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            mnode = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            mnode = read_yin_anyxml(module, retval, sub, resolve);
        }
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    return retval;

error:

    ly_mnode_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }

    return NULL;
}

static struct ly_mnode *
read_yin_grouping(struct ly_module *module,
                  struct ly_mnode *parent, struct lyxml_elem *node, int resolve, struct obj_list **unres)
{
    struct lyxml_elem *sub, *next, root;
    struct ly_mnode *mnode = NULL;
    struct ly_mnode *retval;
    struct ly_mnode_grp *grp;
    int r;
    int c_tpdf = 0;

    /* init */
    memset(&root, 0, sizeof root);

    grp = calloc(1, sizeof *grp);
    grp->nodetype = LY_NODE_GROUPING;
    grp->prev = (struct ly_mnode *)grp;
    retval = (struct ly_mnode *)grp;

    if (read_yin_common(module, parent, retval, node, OPT_IDENT | OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(node->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        /* data statements */
        if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(sub);
            lyxml_add_child(&root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        grp->tpdf = calloc(c_tpdf, sizeof *grp->tpdf);
    }
    LY_TREE_FOR(node->child, sub) {
        r = fill_yin_typedef(module, retval, sub, &grp->tpdf[grp->tpdf_size], NULL);
        grp->tpdf_size++;

        if (r) {
            goto error;
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            mnode = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            mnode = read_yin_leaflist(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "leaf")) {
            mnode = read_yin_leaf(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "list")) {
            mnode = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            mnode = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            mnode = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            mnode = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            mnode = read_yin_anyxml(module, retval, sub, resolve);
        }
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    return retval;

error:

    ly_mnode_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }

    return NULL;
}

static struct ly_mnode *
read_yin_input_output(struct ly_module *module,
                      struct ly_mnode *parent, struct lyxml_elem *yin, int resolve, struct obj_list **unres)
{
    struct lyxml_elem *sub, *next, root;
    struct ly_mnode *mnode = NULL;
    struct ly_mnode *retval;
    struct ly_mnode_input_output *inout;
    int r;
    int c_tpdf = 0;

    /* init */
    memset(&root, 0, sizeof root);

    inout = calloc(1, sizeof *inout);

    if (!strcmp(yin->name, "input")) {
        inout->nodetype = LY_NODE_INPUT;
    } else if (!strcmp(yin->name, "output")) {
        inout->nodetype = LY_NODE_OUTPUT;
    } else {
        assert(0);
    }

    inout->prev = (struct ly_mnode *)inout;
    retval = (struct ly_mnode *)inout;

    if (read_yin_common(module, parent, retval, yin, OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    /* data statements */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(sub);
            lyxml_add_child(&root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;

        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        inout->tpdf = calloc(c_tpdf, sizeof *inout->tpdf);
    }

    LY_TREE_FOR(yin->child, sub) {
        r = fill_yin_typedef(module, retval, sub, &inout->tpdf[inout->tpdf_size], NULL);
        inout->tpdf_size++;

        if (r) {
            goto error;
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            mnode = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            mnode = read_yin_leaflist(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "leaf")) {
            mnode = read_yin_leaf(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "list")) {
            mnode = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            mnode = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            mnode = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            mnode = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            mnode = read_yin_anyxml(module, retval, sub, resolve);
        }
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    return retval;

error:

    ly_mnode_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }

    return NULL;
}

static struct ly_mnode *
read_yin_notif(struct ly_module *module,
               struct ly_mnode *parent, struct lyxml_elem *yin, int resolve, struct obj_list **unres)
{
    struct lyxml_elem *sub, *next, root;
    struct ly_mnode *mnode = NULL;
    struct ly_mnode *retval;
    struct ly_mnode_notif *notif;
    const char *value;
    int r;
    int c_tpdf = 0, c_ftrs = 0;

    memset(&root, 0, sizeof root);

    notif = calloc(1, sizeof *notif);
    notif->nodetype = LY_NODE_NOTIF;
    notif->prev = (struct ly_mnode *)notif;
    retval = (struct ly_mnode *)notif;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    /* process rpc's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        /* data statements */
        if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(sub);
            lyxml_add_child(&root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        notif->tpdf = calloc(c_tpdf, sizeof *notif->tpdf);
    }
    if (c_ftrs) {
        notif->features = calloc(c_ftrs, sizeof *notif->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &notif->tpdf[notif->tpdf_size], NULL);
            notif->tpdf_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-features")) {
            GETVAL(value, sub, "name");
            notif->features[notif->features_size] = resolve_feature(value, module, LOGLINE(sub));
            if (!notif->features[notif->features_size++]) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            mnode = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            mnode = read_yin_leaflist(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "leaf")) {
            mnode = read_yin_leaf(module, retval, sub, resolve);
        } else if (!strcmp(sub->name, "list")) {
            mnode = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            mnode = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            mnode = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            mnode = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            mnode = read_yin_anyxml(module, retval, sub, resolve);
        }
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    return retval;

error:

    ly_mnode_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }

    return NULL;
}

static struct ly_mnode *
read_yin_rpc(struct ly_module *module,
             struct ly_mnode *parent, struct lyxml_elem *yin, int resolve, struct obj_list **unres)
{
    struct lyxml_elem *sub, *next, root;
    struct ly_mnode *mnode = NULL;
    struct ly_mnode *retval;
    struct ly_mnode_rpc *rpc;
    const char *value;
    int r;
    int c_tpdf = 0, c_ftrs = 0;

    /* init */
    memset(&root, 0, sizeof root);

    rpc = calloc(1, sizeof *rpc);
    rpc->nodetype = LY_NODE_RPC;
    rpc->prev = (struct ly_mnode *)rpc;
    retval = (struct ly_mnode *)rpc;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    /* process rpc's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "input")) {
            if (rpc->child
                && (rpc->child->nodetype == LY_NODE_INPUT
                    || (rpc->child->next && rpc->child->next->nodetype == LY_NODE_INPUT))) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            lyxml_unlink_elem(sub);
            lyxml_add_child(&root, sub);
        } else if (!strcmp(sub->name, "output")) {
            if (rpc->child
                && (rpc->child->nodetype == LY_NODE_INPUT
                    || (rpc->child->next && rpc->child->next->nodetype == LY_NODE_INPUT))) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            lyxml_unlink_elem(sub);
            lyxml_add_child(&root, sub);

            /* data statements */
        } else if (!strcmp(sub->name, "grouping")) {
            lyxml_unlink_elem(sub);
            lyxml_add_child(&root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        rpc->tpdf = calloc(c_tpdf, sizeof *rpc->tpdf);
    }
    if (c_ftrs) {
        rpc->features = calloc(c_ftrs, sizeof *rpc->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &rpc->tpdf[rpc->tpdf_size], NULL);
            rpc->tpdf_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            rpc->features[rpc->features_size] = resolve_feature(value, module, LOGLINE(sub));
            if (!rpc->features[rpc->features_size++]) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "grouping")) {
            mnode = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "input")) {
            mnode = read_yin_input_output(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "output")) {
            mnode = read_yin_input_output(module, retval, sub, resolve, unres);
        }
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    return retval;

error:

    ly_mnode_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }

    return NULL;
}

static int
find_grouping(struct ly_mnode *parent, struct ly_mnode_uses *uses, int line)
{
    struct ly_module *searchmod = NULL, *module = uses->module;
    struct ly_mnode *mnode, *mnode_aux;
    const char *name;
    int prefix_len = 0;
    int i;

    /* get referenced grouping */
    name = strchr(uses->name, ':');
    if (!name) {
        /* no prefix, search in local tree */
        name = uses->name;
    } else {
        /* there is some prefix, check if it refer the same data model */

        /* set name to correct position after colon */
        prefix_len = name - uses->name;
        name++;

        if (!strncmp(uses->name, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
            /* prefix refers to the current module, ignore it */
            prefix_len = 0;
        }
    }

    /* search */
    if (prefix_len) {
        /* in top-level groupings of some other module */
        for (i = 0; i < module->imp_size; i++) {
            if (!strncmp(module->imp[i].prefix, uses->name, prefix_len)
                && !module->imp[i].prefix[prefix_len]) {
                searchmod = module->imp[i].module;
                break;
            }
        }
        if (!searchmod) {
            /* uses refers unknown data model */
            LOGVAL(VE_INPREFIX, line, name);
            return EXIT_FAILURE;
        }

        LY_TREE_FOR(searchmod->data, mnode) {
            if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
                uses->grp = (struct ly_mnode_grp *)mnode;
                return EXIT_SUCCESS;
            }
        }
    } else {
        /* in local tree hierarchy */
        for (mnode_aux = parent; mnode_aux; mnode_aux = mnode_aux->parent) {
            LY_TREE_FOR(mnode_aux->child, mnode) {
                if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
                    uses->grp = (struct ly_mnode_grp *)mnode;
                    return EXIT_SUCCESS;
                }
            }
        }

        /* search in top level of the current module */
        LY_TREE_FOR(module->data, mnode) {
            if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
                uses->grp = (struct ly_mnode_grp *)mnode;
                return EXIT_SUCCESS;
            }
        }

        /* search in top-level of included modules */
        for (i = 0; i < module->inc_size; i++) {
            LY_TREE_FOR(module->inc[i].submodule->data, mnode) {
                if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
                    uses->grp = (struct ly_mnode_grp *)mnode;
                    return EXIT_SUCCESS;
                }
            }
        }
    }

    /* not found, but no explicit error occured */
    return EXIT_SUCCESS;
}

static int
resolve_augment(struct ly_augment *aug, struct ly_mnode *parent, struct ly_module *module, unsigned int line)
{
    struct lyxml_elem *yin, *sub;
    struct ly_mnode *mnode;

    assert(module);

    /* resolve target node */
    aug->target = resolve_schema_nodeid(aug->target_name, parent, module, LY_NODE_AUGMENT);
    if (!aug->target) {
        LOGVAL(VE_INARG, line, aug->target_name, "uses");
        return EXIT_FAILURE;
    }

    if (!aug->child) {
        /* nothing to do */
        return EXIT_SUCCESS;
    }

    yin = (struct lyxml_elem *)aug->child;

    /* inherit config information from parent, augment does not have
     * config property, but we need to keep the information for subelements
     */
    aug->flags |= aug->target->flags & LY_NODE_CONFIG_MASK;

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "container")) {
            mnode = read_yin_container(module, aug->target, sub, 1, NULL);
        } else if (!strcmp(sub->name, "leaf-list")) {
            mnode = read_yin_leaflist(module, aug->target, sub, 1);
        } else if (!strcmp(sub->name, "leaf")) {
            mnode = read_yin_leaf(module, aug->target, sub, 1);
        } else if (!strcmp(sub->name, "list")) {
            mnode = read_yin_list(module, aug->target, sub, 1, NULL);
        } else if (!strcmp(sub->name, "uses")) {
            mnode = read_yin_uses(module, aug->target, sub, 1, NULL);
        } else if (!strcmp(sub->name, "choice")) {
            mnode = read_yin_case(module, aug->target, sub, 1, NULL);
        } else if (!strcmp(sub->name, "case")) {
            mnode = read_yin_case(module, aug->target, sub, 1, NULL);
        } else if (!strcmp(sub->name, "anyxml")) {
            mnode = read_yin_anyxml(module, aug->target, sub, 1);
        } else {
            /* never should be here, since it was already pre-parsed by fill_yin_augment() */
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            return EXIT_FAILURE;
        }

        if (!mnode) {
            return EXIT_FAILURE;
        }
        /* check for mandatory nodes - if the target node is in another module
         * the added nodes cannot be mandatory
         */
        if (check_mandatory(mnode)) {
            LOGVAL(VE_SPEC, LOGLINE(sub), "When augmenting data in another module, mandatory statement is not allowed.");
            return EXIT_FAILURE;
        }

        /* the parent pointer will point to the augment node, but all
         * siblings pointers and possibly the child node in target does
         * not know about the augment and follow the standard schema tree
         * structure
         */
        mnode->parent = (struct ly_mnode *)aug;
        mnode = NULL;
    }

    lyxml_free_elem(module->ctx, yin);
    aug->child = NULL;

    return EXIT_SUCCESS;
}

int
resolve_uses(struct ly_mnode_uses *uses, unsigned int line)
{
    struct ly_ctx *ctx;
    struct ly_mnode *mnode = NULL, *mnode_aux;
    struct ly_refine *rfn;
    struct ly_restr *newmust;
    int i, j;
    uint8_t size;

    /* copy the data nodes from grouping into the uses context */
    LY_TREE_FOR(uses->grp->child, mnode) {
        mnode_aux = ly_mnode_dup(uses->module, mnode, uses->flags, 1, line);
        if (!mnode_aux) {
            LOGVAL(VE_SPEC, line, "Copying data from grouping failed");
            return EXIT_FAILURE;
        }
        if (ly_mnode_addchild((struct ly_mnode *)uses, mnode_aux)) {
            ly_mnode_free(mnode_aux);
            return EXIT_FAILURE;
        }
    }
    ctx = uses->module->ctx;

    /* apply refines */
    for (i = 0; i < uses->refine_size; i++) {
        rfn = &uses->refine[i];
        mnode = resolve_schema_nodeid(rfn->target, (struct ly_mnode *)uses, uses->module, LY_NODE_USES);
        if (!mnode) {
            LOGVAL(VE_INARG, line, rfn->target, "uses");
            return EXIT_FAILURE;
        }

        if (rfn->target_type && !(mnode->nodetype & rfn->target_type)) {
            LOGVAL(VE_SPEC, line, "refine substatements not applicable to the target-node");
            return EXIT_FAILURE;
        }

        /* description on any nodetype */
        if (rfn->dsc) {
            lydict_remove(ctx, mnode->dsc);
            mnode->dsc = lydict_insert(ctx, rfn->dsc, 0);
        }

        /* reference on any nodetype */
        if (rfn->ref) {
            lydict_remove(ctx, mnode->ref);
            mnode->ref = lydict_insert(ctx, rfn->ref, 0);
        }

        /* config on any nodetype */
        if (rfn->flags & LY_NODE_CONFIG_MASK) {
            mnode->flags &= ~LY_NODE_CONFIG_MASK;
            mnode->flags |= (rfn->flags & LY_NODE_CONFIG_MASK);
        }

        /* default value ... */
        if (rfn->mod.dflt) {
            if (mnode->nodetype == LY_NODE_LEAF) {
                /* leaf */
                lydict_remove(ctx, ((struct ly_mnode_leaf *)mnode)->dflt);
                ((struct ly_mnode_leaf *)mnode)->dflt = lydict_insert(ctx, rfn->mod.dflt, 0);
            } else if (mnode->nodetype == LY_NODE_CHOICE) {
                /* choice */
                ((struct ly_mnode_choice *)mnode)->dflt = resolve_schema_nodeid(rfn->mod.dflt, mnode, mnode->module, LY_NODE_CHOICE);
                if (!((struct ly_mnode_choice *)mnode)->dflt) {
                    LOGVAL(VE_INARG, line, rfn->mod.dflt, "default");
                    return EXIT_FAILURE;
                }
            }
        }

        /* mandatory on leaf, anyxml or choice */
        if (rfn->flags & LY_NODE_MAND_MASK) {
            if (mnode->nodetype & (LY_NODE_LEAF | LY_NODE_ANYXML | LY_NODE_CHOICE)) {
                /* remove current value */
                mnode->flags &= ~LY_NODE_MAND_MASK;

                /* set new value */
                mnode->flags |= (rfn->flags & LY_NODE_MAND_MASK);
            }
        }

        /* presence on container */
        if ((mnode->nodetype & LY_NODE_CONTAINER) && rfn->mod.presence) {
            lydict_remove(ctx, ((struct ly_mnode_container *)mnode)->presence);
            ((struct ly_mnode_container *)mnode)->presence = lydict_insert(ctx, rfn->mod.presence, 0);
        }

        /* min/max-elements on list or leaf-list */
        if (mnode->nodetype & (LY_NODE_LEAFLIST | LY_NODE_LIST)) {
            /* magic - bit 3 in flags means min set, bit 4 says max set */
            if (rfn->flags & 0x04) {
                ((struct ly_mnode_list *)mnode)->min = rfn->mod.list.min;
            }
            if (rfn->flags & 0x08) {
                ((struct ly_mnode_list *)mnode)->max = rfn->mod.list.max;
            }
        }

        /* must in leaf, leaf-list, list, container or anyxml */
        if (rfn->must_size) {
            size = ((struct ly_mnode_leaf *)mnode)->must_size + rfn->must_size;
            newmust = realloc(((struct ly_mnode_leaf *)mnode)->must, size * sizeof *rfn->must);
            if (!newmust) {
                LOGMEM;
                return EXIT_FAILURE;
            }
            for (i = 0, j = ((struct ly_mnode_leaf *)mnode)->must_size; i < rfn->must_size; i++, j++) {
                newmust[j].expr = lydict_insert(ctx, rfn->must[i].expr, 0);
                newmust[j].dsc = lydict_insert(ctx, rfn->must[i].dsc, 0);
                newmust[j].ref = lydict_insert(ctx, rfn->must[i].ref, 0);
                newmust[j].eapptag = lydict_insert(ctx, rfn->must[i].eapptag, 0);
                newmust[j].emsg = lydict_insert(ctx, rfn->must[i].emsg, 0);
            }

            ((struct ly_mnode_leaf *)mnode)->must = newmust;
            ((struct ly_mnode_leaf *)mnode)->must_size = size;
        }
    }

    /* apply augments */
    for (i = 0; i < uses->augment_size; i++) {
        if (resolve_augment(&uses->augment[i], (struct ly_mnode *)uses, uses->module, line)) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/*
 * resolve - referenced grouping should be bounded to the namespace (resolved)
 * only when uses does not appear in grouping. In a case of grouping's uses,
 * we just get information but we do not apply augment or refine to it.
 */
static struct ly_mnode *
read_yin_uses(struct ly_module *module,
              struct ly_mnode *parent, struct lyxml_elem *node, int resolve, struct obj_list **unres)
{
    struct lyxml_elem *sub, *next;
    struct ly_mnode *retval;
    struct ly_mnode_uses *uses;
    struct obj_list *unres_new;
    const char *value;
    int c_ref = 0, c_aug = 0, c_ftrs = 0;
    int r;

    uses = calloc(1, sizeof *uses);
    uses->nodetype = LY_NODE_USES;
    uses->prev = (struct ly_mnode *)uses;
    retval = (struct ly_mnode *)uses;

    GETVAL(value, node, "name");
    uses->name = lydict_insert(module->ctx, value, 0);

    if (read_yin_common(module, parent, retval, node, OPT_MODULE |  OPT_NACMEXT | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    /* get other properties of uses */
    LY_TREE_FOR_SAFE(node->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "refine")) {
            c_ref++;
        } else if (!strcmp(sub->name, "augment")) {
            c_aug++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else if (!strcmp(sub->name, "when")) {
            if (uses->when) {
                LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, node->name);
                goto error;
            }

            uses->when = read_yin_when(module, sub);
            lyxml_free_elem(module->ctx, sub);

            if (!uses->when) {
                goto error;
            }
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* process properties with cardinality 0..n */
    if (c_ref) {
        uses->refine = calloc(c_ref, sizeof *uses->refine);
    }
    if (c_aug) {
        uses->augment = calloc(c_aug, sizeof *uses->augment);
    }
    if (c_ftrs) {
        uses->features = calloc(c_ftrs, sizeof *uses->features);
    }

    LY_TREE_FOR(node->child, sub) {
        if (!strcmp(sub->name, "refine")) {
            r = fill_yin_refine(module, sub, &uses->refine[uses->refine_size]);
            uses->refine_size++;
        } else if (!strcmp(sub->name, "augment")) {
            r = fill_yin_augment(module, retval, sub, &uses->augment[uses->augment_size]);
            uses->augment_size++;
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            uses->features[uses->features_size] = resolve_feature(value, module, LOGLINE(sub));
            if (!uses->features[uses->features_size++]) {
                goto error;
            }
            r = 0;
        }

        if (r) {
            goto error;
        }
    }

    if (find_grouping(parent, uses, LOGLINE(node))) {
        goto error;
    }
    if (!uses->grp) {
        LOGVRB("Unresolved uses of \"%s\" (line %d), trying to resolve it later", uses->name, LOGLINE(node));
        unres_new = calloc(1, sizeof *unres_new);
        if (*unres) {
            unres_new->next = *unres;
        }
        unres_new->obj = retval;
        unres_new->line = LOGLINE(node);

        /* put it at the beginning of the unresolved list */
        *unres = unres_new;
    }

    if (parent && ly_mnode_addchild(parent, retval)) {
        goto error;
    }

    if (resolve) {
        /* inherit config flag */
        if (parent) {
            retval->flags |= parent->flags & LY_NODE_CONFIG_MASK;
        } else {
            /* default config is true */
            retval->flags |= LY_NODE_CONFIG_W;
        }
    }

    if (resolve && uses->grp) {
        /* copy the data nodes from grouping into the uses context */
        if (resolve_uses(uses, LOGLINE(node))) {
            goto error;
        }
    }

    return retval;

error:

    ly_mnode_free(retval);

    return NULL;
}

/* common code for yin_read_module() and yin_read_submodule() */
static int
read_sub_module(struct ly_module *module, struct lyxml_elem *yin)
{
    struct ly_ctx *ctx = module->ctx;
    struct ly_submodule *submodule = (struct ly_submodule *)module;
    struct lyxml_elem *next, *node, *child, root, grps, rpcs, notifs;
    struct ly_mnode *mnode = NULL;
    struct obj_list *unres = NULL, *unres_next;       /* unresolved objects */
    const char *value;
    int r;
    int i;
    int unres_flag = 0; /* 0 for uses, 1 for types */
    int belongsto_flag = 0;
    /* counters */
    int c_imp = 0, c_rev = 0, c_tpdf = 0, c_ident = 0, c_inc = 0, c_aug = 0, c_ftrs = 0, c_dev = 0;

    /* init */
    memset(&root, 0, sizeof root);
    memset(&grps, 0, sizeof grps);
    memset(&rpcs, 0, sizeof rpcs);
    memset(&notifs, 0, sizeof notifs);

    /*
     * in the first run, we process elements with cardinality of 1 or 0..1 and
     * count elements with cardinality 0..n. Data elements (choices, containers,
     * leafs, lists, leaf-lists) are moved aside to be processed last, since we
     * need have all top-level and groupings already prepared at that time. In
     * the middle loop, we process other elements with carinality of 0..n since
     * we need to allocate arrays to store them.
     */
    LY_TREE_FOR_SAFE(yin->child, next, node) {
        if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(ctx, node);
            continue;
        }

        if (!module->type && !strcmp(node->name, "namespace")) {
            if (module->ns) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "uri");
            module->ns = lydict_insert(ctx, value, strlen(value));
            lyxml_free_elem(ctx, node);
        } else if (!module->type && !strcmp(node->name, "prefix")) {
            if (module->prefix) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "value");
            if (check_identifier(value, LY_IDENT_PREFIX, LOGLINE(node), module, NULL)) {
                goto error;
            }
            module->prefix = lydict_insert(ctx, value, strlen(value));
            lyxml_free_elem(ctx, node);
        } else if (module->type && !strcmp(node->name, "belongs-to")) {
            if (belongsto_flag) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            belongsto_flag = 1;
            GETVAL(value, node, "module");
            while (submodule->belongsto->type) {
                submodule->belongsto = ((struct ly_submodule *)submodule->belongsto)->belongsto;
            }
            if (value != submodule->belongsto->name) {
                LOGVAL(VE_INARG, LOGLINE(node), value, node->name);
                goto error;
            }

            /* get the prefix substatement, start with checks */
            if (!node->child) {
                LOGVAL(VE_MISSSTMT2, LOGLINE(node), "prefix", node->name);
                goto error;
            } else if (strcmp(node->child->name, "prefix")) {
                LOGVAL(VE_INSTMT, LOGLINE(node->child), node->child->name);
                goto error;
            } else if (node->child->next) {
                LOGVAL(VE_INSTMT, LOGLINE(node->child->next), node->child->next->name);
                goto error;
            }
            /* and now finally get the value */
            GETVAL(value, node->child, "value");
            /* check here differs from a generic prefix check, since this prefix
             * don't have to be unique
             */
            if (check_identifier(value, LY_IDENT_NAME, LOGLINE(node->child), NULL, NULL)) {
                goto error;
            }
            module->prefix = lydict_insert(ctx, value, strlen(value));

            /* we are done with belongs-to */
            lyxml_free_elem(ctx, node);

            /* counters (statements with n..1 cardinality) */
        } else if (!strcmp(node->name, "import")) {
            c_imp++;
        } else if (!strcmp(node->name, "revision")) {
            c_rev++;
        } else if (!strcmp(node->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(node->name, "identity")) {
            c_ident++;
        } else if (!strcmp(node->name, "include")) {
            c_inc++;
        } else if (!strcmp(node->name, "augment")) {
            c_aug++;
        } else if (!strcmp(node->name, "feature")) {
            c_ftrs++;
        } else if (!strcmp(node->name, "deviation")) {
            c_dev++;

            /* data statements */
        } else if (!strcmp(node->name, "container") ||
                !strcmp(node->name, "leaf-list") ||
                !strcmp(node->name, "leaf") ||
                !strcmp(node->name, "list") ||
                !strcmp(node->name, "choice") ||
                !strcmp(node->name, "uses") ||
                !strcmp(node->name, "anyxml")) {
            lyxml_unlink_elem(node);
            lyxml_add_child(&root, node);
        } else if (!strcmp(node->name, "grouping")) {
            /* keep groupings separated and process them before other data statements */
            lyxml_unlink_elem(node);
            lyxml_add_child(&grps, node);

            /* optional statements */
        } else if (!strcmp(node->name, "description")) {
            if (module->dsc) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            module->dsc = read_yin_subnode(ctx, node, "text");
            lyxml_free_elem(ctx, node);
            if (!module->dsc) {
                goto error;
            }
        } else if (!strcmp(node->name, "reference")) {
            if (module->ref) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            module->ref = read_yin_subnode(ctx, node, "text");
            lyxml_free_elem(ctx, node);
            if (!module->ref) {
                goto error;
            }
        } else if (!strcmp(node->name, "organization")) {
            if (module->org) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            module->org = read_yin_subnode(ctx, node, "text");
            lyxml_free_elem(ctx, node);
            if (!module->org) {
                goto error;
            }
        } else if (!strcmp(node->name, "contact")) {
            if (module->contact) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            module->contact = read_yin_subnode(ctx, node, "text");
            lyxml_free_elem(ctx, node);
            if (!module->contact) {
                goto error;
            }
        } else if (!strcmp(node->name, "yang-version")) {
            /* TODO: support YANG 1.1 ? */
            if (module->version) {
                LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "value");
            if (strcmp(value, "1")) {
                LOGVAL(VE_INARG, LOGLINE(node), value, "yang-version");
                goto error;
            }
            module->version = 1;
            lyxml_free_elem(ctx, node);

            /* rpcs & notifications */
        } else if (!strcmp(node->name, "rpc")) {
            lyxml_unlink_elem(node);
            lyxml_add_child(&rpcs, node);
        } else if (!strcmp(node->name, "notification")) {
            lyxml_unlink_elem(node);
            lyxml_add_child(&notifs, node);

        } else if (!strcmp(node->name, "extension")) {
            GETVAL(value, node, "name");

            /* we have 2 supported (hardcoded) extensions:
             * NACM's default-deny-write and default-deny-all
             */
            if (strcmp(module->ns, LY_NSNACM) ||
                    (strcmp(value, "default-deny-write") && strcmp(value, "default-deny-all"))) {
                LOGWRN("Not supported \"%s\" extension statement found, ignoring.", value);
                lyxml_free_elem(ctx, node);
            }
        } else {
            LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
            goto error;
        }
    }

    if (!submodule) {
        /* check for mandatory statements */
        if (!module->ns) {
            LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "namespace", "module");
            goto error;
        }
        if (!module->prefix) {
            LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "prefix", "module");
            goto error;
        }
    }

    /* allocate arrays for elements with cardinality of 0..n */
    if (c_imp) {
        module->imp = calloc(c_imp, sizeof *module->imp);
    }
    if (c_rev) {
        module->rev = calloc(c_rev, sizeof *module->rev);
    }
    if (c_tpdf) {
        module->tpdf = calloc(c_tpdf, sizeof *module->tpdf);
    }
    if (c_ident) {
        module->ident = calloc(c_ident, sizeof *module->ident);
    }
    if (c_inc) {
        module->inc = calloc(c_inc, sizeof *module->inc);
    }
    if (c_aug) {
        module->augment = calloc(c_aug, sizeof *module->augment);
    }
    if (c_ftrs) {
        module->features = calloc(c_ftrs, sizeof *module->features);
    }
    if (c_dev) {
        module->deviation = calloc(c_dev, sizeof *module->deviation);
    }

    /* now we are going to remember unresolved types, the flag is
     * used in case of error to get know how to free the structures
     * in unres list
     */
    unres_flag = 1;

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    LY_TREE_FOR(yin->child, node) {
        if (!strcmp(node->name, "import")) {
            r = fill_yin_import(module, node, &module->imp[module->imp_size]);
            module->imp_size++;
            if (r) {
                goto error;
            }

            /* check duplicities in imported modules */
            for (i = 0; i < module->imp_size - 1; i++) {
                if (!strcmp(module->imp[i].module->name, module->imp[module->imp_size - 1].module->name)) {
                    LOGVAL(VE_SPEC, LOGLINE(node), "Importing module \"%s\" repeatedly.", module->imp[i].module->name);
                    goto error;
                }
            }
        } else if (!strcmp(node->name, "include")) {
            r = fill_yin_include(module, node, &module->inc[module->inc_size]);
            module->inc_size++;
            if (r) {
                goto error;
            }

            /* check duplications in include submodules */
            for (i = 0; i < module->inc_size - 1; i++) {
                if (!strcmp(module->inc[i].submodule->name, module->inc[module->inc_size - 1].submodule->name)) {
                    LOGVAL(VE_SPEC, LOGLINE(node), "Importing module \"%s\" repeatedly.",
                           module->inc[i].submodule->name);
                    goto error;
                }
            }
        } else if (!strcmp(node->name, "revision")) {
            GETVAL(value, node, "date");
            if (check_date(value, LOGLINE(node))) {
                goto error;
            }
            memcpy(module->rev[module->rev_size].date, value, LY_REV_SIZE - 1);
            /* check uniqueness of the revision date - not required by RFC */
            for (i = 0; i < module->rev_size; i++) {
                if (!strcmp(value, module->rev[i].date)) {
                    LOGVAL(VE_INARG, LOGLINE(node), value, node->name);
                    LOGVAL(VE_SPEC, 0, "Revision is not unique.");
                }
            }

            LY_TREE_FOR(node->child, child) {
                if (!strcmp(child->name, "description")) {
                    if (module->rev[module->rev_size].dsc) {
                        LOGVAL(VE_TOOMANY, LOGLINE(node), child->name, node->name);
                        goto error;
                    }
                    module->rev[module->rev_size].dsc = read_yin_subnode(ctx, child, "text");
                    if (!module->rev[module->rev_size].dsc) {
                        goto error;
                    }
                } else if (!strcmp(child->name, "reference")) {
                    if (module->rev[module->rev_size].ref) {
                        LOGVAL(VE_TOOMANY, LOGLINE(node), child->name, node->name);
                        goto error;
                    }
                    module->rev[module->rev_size].ref = read_yin_subnode(ctx, child, "text");
                    if (!module->rev[module->rev_size].ref) {
                        goto error;
                    }
                } else {
                    LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
                    goto error;
                }
            }

            /* keep the latest revision at position 0 */
            if (module->rev_size && strcmp(module->rev[module->rev_size].date, module->rev[0].date) > 0) {
                /* switch their position */
                value = strdup(module->rev[0].date);
                memcpy(module->rev[0].date, module->rev[module->rev_size].date, LY_REV_SIZE - 1);
                memcpy(module->rev[module->rev_size].date, value, LY_REV_SIZE - 1);
                free((char *)value);

                if (module->rev[0].dsc != module->rev[module->rev_size].dsc) {
                    value = module->rev[0].dsc;
                    module->rev[0].dsc = module->rev[module->rev_size].dsc;
                    module->rev[module->rev_size].dsc = value;
                }

                if (module->rev[0].ref != module->rev[module->rev_size].ref) {
                    value = module->rev[0].ref;
                    module->rev[0].ref = module->rev[module->rev_size].ref;
                    module->rev[module->rev_size].ref = value;
                }
            }

            module->rev_size++;
        } else if (!strcmp(node->name, "typedef")) {
            r = fill_yin_typedef(module, NULL, node, &module->tpdf[module->tpdf_size], &unres);
            module->tpdf_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(node->name, "identity")) {
            r = fill_yin_identity(module, node, &module->ident[module->ident_size]);
            module->ident_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(node->name, "feature")) {
            r = fill_yin_feature(module, node, &module->features[module->features_size]);
            module->features_size++;

            if (r) {
                goto error;
            }
        } else if (!strcmp(node->name, "augment")) {
            r = fill_yin_augment(module, NULL, node, &module->augment[module->augment_size]);
            module->augment_size++;

            if (r) {
                goto error;
            }

            /* node is reconnected into the augment, so we have to skip its free at the end of the loop */
            continue;
        } else if (!strcmp(node->name, "deviation")) {
            r = fill_yin_deviation(module, node, &module->deviation[module->deviation_size]);
            module->deviation_size++;

            if (r) {
                goto error;
            }
        }
    }
    /* resolve unresolved types (possible in typedef's with unions */
    while (unres) {
        node = (struct lyxml_elem *)((struct ly_type *)unres->obj)->der;
        if (fill_yin_type(module, NULL, node, (struct ly_type *)unres->obj, NULL)) {
            goto error;
        }

        /* cleanup */
        lyxml_free_elem(ctx, node);
        ((struct ly_type *)unres->obj)->der = NULL;
        unres_next = unres->next;
        free(unres);
        unres = unres_next;

    }
    unres_flag = 0;

    /* process data nodes. Start with groupings to allow uses
     * refer to them
     */
    LY_TREE_FOR_SAFE(grps.child, next, node) {
        mnode = read_yin_grouping(module, NULL, node, 0, &unres);
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(ctx, node);

        /* include data element */
        if (module->data) {
            module->data->prev->next = mnode;
            mnode->prev = module->data->prev;
            module->data->prev = mnode;
        } else {
            module->data = mnode;
        }
    }
    while (unres) {
        if (find_grouping(((struct ly_mnode_uses *)unres->obj)->parent, (struct ly_mnode_uses *)unres->obj, unres->line)) {
            goto error;
        }
        if (!((struct ly_mnode_uses *)unres->obj)->grp) {
            LOGVAL(VE_INARG, unres->line, ((struct ly_mnode_uses *)unres->obj)->name, "uses");
            goto error;
        }
        unres_next = unres->next;
        free(unres);
        unres = unres_next;
    }

    /* parse data nodes, ... */
    LY_TREE_FOR_SAFE(root.child, next, node) {

        if (!strcmp(node->name, "container")) {
            mnode = read_yin_container(module, NULL, node, 1, &unres);
        } else if (!strcmp(node->name, "leaf-list")) {
            mnode = read_yin_leaflist(module, NULL, node, 1);
        } else if (!strcmp(node->name, "leaf")) {
            mnode = read_yin_leaf(module, NULL, node, 1);
        } else if (!strcmp(node->name, "list")) {
            mnode = read_yin_list(module, NULL, node, 1, &unres);
        } else if (!strcmp(node->name, "choice")) {
            mnode = read_yin_choice(module, NULL, node, 1, &unres);
        } else if (!strcmp(node->name, "uses")) {
            mnode = read_yin_uses(module, NULL, node, 1, &unres);
        } else if (!strcmp(node->name, "anyxml")) {
            mnode = read_yin_anyxml(module, NULL, node, 1);
        }
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(ctx, node);

        /* include data element */
        if (module->data) {
            module->data->prev->next = mnode;
            mnode->prev = module->data->prev;
            module->data->prev = mnode;
        } else {
            module->data = mnode;
        }
    }

    /* ... rpcs ... */
    LY_TREE_FOR_SAFE(rpcs.child, next, node) {
        mnode = read_yin_rpc(module, NULL, node, 0, &unres);
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(ctx, node);

        /* include rpc element */
        if (module->rpc) {
            module->rpc->prev->next = mnode;
            mnode->prev = module->rpc->prev;
            module->rpc->prev = mnode;
        } else {
            module->rpc = mnode;
        }
    }

    /* ... and notifications */
    LY_TREE_FOR_SAFE(notifs.child, next, node) {
        mnode = read_yin_notif(module, NULL, node, 0, &unres);
        if (!mnode) {
            goto error;
        }

        lyxml_free_elem(ctx, node);

        /* include notification element */
        if (module->notif) {
            module->notif->prev->next = mnode;
            mnode->prev = module->notif->prev;
            module->notif->prev = mnode;
        } else {
            module->notif = mnode;
        }
    }

    /* and now try to resolve unresolved uses, if any */
    while (unres) {
        /* find referenced grouping */
        if (find_grouping(((struct ly_mnode_uses *)unres->obj)->parent, (struct ly_mnode_uses *)unres->obj, unres->line)) {
            goto error;
        }
        if (!((struct ly_mnode_uses *)unres->obj)->grp) {
            LOGVAL(VE_INARG, unres->line, ((struct ly_mnode_uses *)unres->obj)->name, "uses");
            goto error;
        }

        /* resolve uses by copying grouping content under the uses */
        if (resolve_uses((struct ly_mnode_uses *)unres->obj, unres->line)) {
            goto error;
        }

        unres_next = unres->next;
        free(unres);
        unres = unres_next;
    }

    /* and finally apply augments */
    for (i = 0; i < module->augment_size; i++) {
        if (resolve_augment(&module->augment[i], NULL, module, 0)) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    /* cleanup */
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }
    while (grps.child) {
        lyxml_free_elem(module->ctx, grps.child);
    }
    while (rpcs.child) {
        lyxml_free_elem(module->ctx, rpcs.child);
    }

    while (unres) {
        unres_next = unres->next;
        if (unres_flag) {
            /* free the XML subtrees kept in unresolved type structures */
            node = (struct lyxml_elem *)((struct ly_type *)unres->obj)->der;
            lyxml_free_elem(ctx, node);
        }

        free(unres);
        unres = unres_next;
    }

    return EXIT_FAILURE;
}

struct ly_submodule *
yin_read_submodule(struct ly_module *module, const char *data)
{
    struct lyxml_elem *yin;
    struct ly_submodule *submodule = NULL;
    const char *value;

    assert(module->ctx);

    yin = lyxml_read(module->ctx, data, 0);
    if (!yin) {
        return NULL;
    }

    /* check root element */
    if (!yin->name || strcmp(yin->name, "submodule")) {
        LOGVAL(VE_INSTMT, LOGLINE(yin), yin->name);
        goto error;
    }

    GETVAL(value, yin, "name");
    if (check_identifier(value, LY_IDENT_NAME, LOGLINE(yin), NULL, NULL)) {
        goto error;
    }

    submodule = calloc(1, sizeof *submodule);
    if (!submodule) {
        LOGMEM;
        goto error;
    }

    submodule->ctx = module->ctx;
    submodule->name = lydict_insert(submodule->ctx, value, strlen(value));
    submodule->type = 1;
    submodule->belongsto = module;

    LOGVRB("reading submodule %s", submodule->name);
    if (read_sub_module((struct ly_module *)submodule, yin)) {
        goto error;
    }

    /* cleanup */
    lyxml_free_elem(module->ctx, yin);

    LOGVRB("submodule %s successfully parsed", submodule->name);

    return submodule;

error:
    /* cleanup */
    lyxml_free_elem(module->ctx, yin);
    ly_submodule_free(submodule);

    return NULL;
}

struct ly_module *
yin_read_module(struct ly_ctx *ctx, const char *data)
{
    struct lyxml_elem *yin;
    struct ly_module *module = NULL, **newlist = NULL;
    const char *value;
    int i;

    yin = lyxml_read(ctx, data, 0);
    if (!yin) {
        return NULL;
    }

    /* check root element */
    if (!yin->name || strcmp(yin->name, "module")) {
        LOGVAL(VE_INSTMT, LOGLINE(yin), yin->name);
        goto error;
    }

    GETVAL(value, yin, "name");
    if (check_identifier(value, LY_IDENT_NAME, LOGLINE(yin), NULL, NULL)) {
        goto error;
    }

    module = calloc(1, sizeof *module);
    if (!module) {
        LOGMEM;
        goto error;
    }

    module->ctx = ctx;
    module->name = lydict_insert(ctx, value, strlen(value));
    module->type = 0;

    LOGVRB("reading module %s", module->name);
    if (read_sub_module(module, yin)) {
        goto error;
    }

    /* add to the context's list of modules */
    if (ctx->models.used == ctx->models.size) {
        newlist = realloc(ctx->models.list, ctx->models.size * 2);
        if (!newlist) {
            LOGMEM;
            goto error;
        }
        for (i = ctx->models.size; i < ctx->models.size * 2; i++) {
            newlist[i] = NULL;
        }
        ctx->models.size *= 2;
        ctx->models.list = newlist;
    }
    for (i = 0; ctx->models.list[i]; i++) {
        /* check name (name/revision) and namespace uniqueness */
        if (!strcmp(ctx->models.list[i]->name, module->name)) {
            if (!ctx->models.list[i]->rev_size && !module->rev_size) {
                /* both data models are same, with no revision specified */
                LOGERR(LY_EINVAL, "Module \"%s\" (no revision in either of them specified) already in context.",
                       module->name);
                goto error;
            } else if (!ctx->models.list[i]->rev_size || !module->rev_size) {
                /* one of the models does not have a revision, so they differs */
                continue;
            } else {
                /* both models have a revision statement which we have to
                 * compare, revision at position 0 is the last revision
                 */
                if (!strcmp(ctx->models.list[i]->rev[0].date, module->rev[0].date)) {
                    /* we have the same modules */
                    LOGERR(LY_EINVAL, "Module \"%s\", revision %s already in context.", module->name,
                           module->rev[0].date);
                    goto error;
                }
            }
        } else if (!strcmp(ctx->models.list[i]->ns, module->ns)) {
            LOGERR(LY_EINVAL, "Two different modules (\"%s\" and \"%s\") have the same namespace \"%s\"",
                   ctx->models.list[i]->name, module->name, module->ns);
            goto error;
        }
    }
    ctx->models.list[i] = module;
    ctx->models.used++;

    /* cleanup */
    lyxml_free_elem(ctx, yin);

    LOGVRB("module %s successfully parsed", module->name);

    return module;

error:
    /* cleanup */
    lyxml_free_elem(ctx, yin);
    ly_module_free(module);

    return NULL;
}
