/**
 * @file schema_compile.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema compilation.
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "schema_compile.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "log.h"
#include "in.h"
#include "parser_schema.h"
#include "path.h"
#include "plugins_exts.h"
#include "plugins_exts_internal.h"
#include "plugins_types.h"
#include "schema_compile_amend.h"
#include "schema_compile_node.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

#define COMPILE_CHECK_UNIQUENESS_ARRAY(CTX, ARRAY, MEMBER, EXCL, STMT, IDENT) \
    if (ARRAY) { \
        for (LY_ARRAY_COUNT_TYPE u__ = 0; u__ < LY_ARRAY_COUNT(ARRAY); ++u__) { \
            if (&(ARRAY)[u__] != EXCL && (void*)((ARRAY)[u__].MEMBER) == (void*)(IDENT)) { \
                LOGVAL((CTX)->ctx, LY_VLOG_STR, (CTX)->path, LY_VCODE_DUPIDENT, IDENT, STMT); \
                return LY_EVALID; \
            } \
        } \
    }

/**
 * @brief Fill in the prepared compiled extensions definition structure according to the parsed extension definition.
 */
static LY_ERR
lys_compile_extension(struct lysc_ctx *ctx, const struct lys_module *ext_mod, struct lysp_ext *ext_p, struct lysc_ext **ext)
{
    LY_ERR ret = LY_SUCCESS;

    if (!ext_p->compiled) {
        lysc_update_path(ctx, NULL, "{extension}");
        lysc_update_path(ctx, NULL, ext_p->name);

        /* compile the extension definition */
        ext_p->compiled = calloc(1, sizeof **ext);
        ext_p->compiled->refcount = 1;
        DUP_STRING_GOTO(ctx->ctx, ext_p->name, ext_p->compiled->name, ret, done);
        DUP_STRING_GOTO(ctx->ctx, ext_p->argument, ext_p->compiled->argument, ret, done);
        ext_p->compiled->module = (struct lys_module *)ext_mod;
        COMPILE_EXTS_GOTO(ctx, ext_p->exts, ext_p->compiled->exts, *ext, LYEXT_PAR_EXT, ret, done);

        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);

        /* find extension definition plugin */
        ext_p->compiled->plugin = lyext_get_plugin(ext_p->compiled);
    }

    *ext = lysc_ext_dup(ext_p->compiled);

done:
    return ret;
}

LY_ERR
lys_compile_ext(struct lysc_ctx *ctx, struct lysp_ext_instance *ext_p, struct lysc_ext_instance *ext, void *parent,
        LYEXT_PARENT parent_type, const struct lys_module *ext_mod)
{
    LY_ERR ret = LY_SUCCESS;
    const char *name;
    size_t u;
    LY_ARRAY_COUNT_TYPE v;
    const char *prefixed_name = NULL;

    DUP_STRING(ctx->ctx, ext_p->argument, ext->argument, ret);
    LY_CHECK_RET(ret);

    ext->insubstmt = ext_p->insubstmt;
    ext->insubstmt_index = ext_p->insubstmt_index;
    ext->module = ctx->cur_mod;
    ext->parent = parent;
    ext->parent_type = parent_type;

    lysc_update_path(ctx, ext->parent_type == LYEXT_PAR_NODE ? (struct lysc_node *)ext->parent : NULL, "{extension}");

    /* get module where the extension definition should be placed */
    for (u = strlen(ext_p->name); u && ext_p->name[u - 1] != ':'; --u) {}
    if (ext_p->yin) {
        /* YIN parser has to replace prefixes by the namespace - XML namespace/prefix pairs may differs form the YANG schema's
         * namespace/prefix pair. YIN parser does not have the imports available, so mapping from XML namespace to the
         * YANG (import) prefix must be done here. */
        if (!ly_strncmp(ctx->pmod->mod->ns, ext_p->name, u - 1)) {
            LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, &ext_p->name[u], 0, &prefixed_name), cleanup);
            u = 0;
        } else {
            LY_ARRAY_FOR(ctx->pmod->imports, v) {
                if (!ly_strncmp(ctx->pmod->imports[v].module->ns, ext_p->name, u - 1)) {
                    char *s;
                    LY_CHECK_ERR_GOTO(asprintf(&s, "%s:%s", ctx->pmod->imports[v].prefix, &ext_p->name[u]) == -1,
                            ret = LY_EMEM, cleanup);
                    LY_CHECK_GOTO(ret = lydict_insert_zc(ctx->ctx, s, &prefixed_name), cleanup);
                    u = strlen(ctx->pmod->imports[v].prefix) + 1; /* add semicolon */
                    break;
                }
            }
        }
        if (!prefixed_name) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                    "Invalid XML prefix of \"%.*s\" namespace used for extension instance identifier.", u, ext_p->name);
            ret = LY_EVALID;
            goto cleanup;
        }
    } else {
        prefixed_name = ext_p->name;
    }
    lysc_update_path(ctx, NULL, prefixed_name);

    if (!ext_mod) {
        ext_mod = u ? lysp_module_find_prefix(ctx->pmod, prefixed_name, u - 1) : ctx->pmod->mod;
        if (!ext_mod) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                    "Invalid prefix \"%.*s\" used for extension instance identifier.", u, prefixed_name);
            ret = LY_EVALID;
            goto cleanup;
        } else if (!ext_mod->parsed->extensions) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                    "Extension instance \"%s\" refers \"%s\" module that does not contain extension definitions.",
                    prefixed_name, ext_mod->name);
            ret = LY_EVALID;
            goto cleanup;
        }
    }
    name = &prefixed_name[u];

    /* find the parsed extension definition there */
    LY_ARRAY_FOR(ext_mod->parsed->extensions, v) {
        if (!strcmp(name, ext_mod->parsed->extensions[v].name)) {
            /* compile extension definition and assign it */
            LY_CHECK_GOTO(ret = lys_compile_extension(ctx, ext_mod, &ext_mod->parsed->extensions[v], &ext->def), cleanup);
            break;
        }
    }
    if (!ext->def) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                "Extension definition of extension instance \"%s\" not found.", prefixed_name);
        ret = LY_EVALID;
        goto cleanup;
    }

    /* unify the parsed extension from YIN and YANG sources. Without extension definition, it is not possible
     * to get extension's argument from YIN source, so it is stored as one of the substatements. Here we have
     * to find it, mark it with LYS_YIN_ARGUMENT and store it in the compiled structure. */
    if (ext_p->yin && ext->def->argument && !ext->argument) {
        /* Schema was parsed from YIN and an argument is expected, ... */
        struct lysp_stmt *stmt = NULL;

        if (ext->def->flags & LYS_YINELEM_TRUE) {
            /* ... argument was the first XML child element */
            if (ext_p->child && !(ext_p->child->flags & LYS_YIN_ATTR)) {
                /* TODO check namespace of the statement */
                if (!strcmp(ext_p->child->stmt, ext->def->argument)) {
                    stmt = ext_p->child;
                }
            }
        } else {
            /* ... argument was one of the XML attributes which are represented as child stmt
             * with LYS_YIN_ATTR flag */
            for (stmt = ext_p->child; stmt && (stmt->flags & LYS_YIN_ATTR); stmt = stmt->next) {
                if (!strcmp(stmt->stmt, ext->def->argument)) {
                    /* this is the extension's argument */
                    break;
                }
            }
        }
        if (!stmt) {
            /* missing extension's argument */
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                    "Extension instance \"%s\" misses argument \"%s\".", prefixed_name, ext->def->argument);
            ret = LY_EVALID;
            goto cleanup;

        }
        LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, stmt->arg, 0, &ext->argument), cleanup);
        stmt->flags |= LYS_YIN_ARGUMENT;
    }
    if (prefixed_name != ext_p->name) {
        lydict_remove(ctx->ctx, ext_p->name);
        ext_p->name = prefixed_name;
        if (!ext_p->argument && ext->argument) {
            LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, ext->argument, 0, &ext_p->argument), cleanup);
        }
    }

    if (ext->def->plugin && ext->def->plugin->compile) {
        if (ext->argument) {
            lysc_update_path(ctx, (struct lysc_node *)ext, ext->argument);
        }
        LY_CHECK_GOTO(ret = ext->def->plugin->compile(ctx, ext_p, ext), cleanup);
        if (ext->argument) {
            lysc_update_path(ctx, NULL, NULL);
        }
    }
    ext_p->compiled = ext;

cleanup:
    if (prefixed_name && (prefixed_name != ext_p->name)) {
        lydict_remove(ctx->ctx, prefixed_name);
    }

    lysc_update_path(ctx, NULL, NULL);
    lysc_update_path(ctx, NULL, NULL);

    return ret;
}

struct lysc_ext *
lysc_ext_dup(struct lysc_ext *orig)
{
    ++orig->refcount;
    return orig;
}

static void
lysc_unres_dflt_free(const struct ly_ctx *ctx, struct lysc_unres_dflt *r)
{
    assert(!r->dflt || !r->dflts);
    if (r->dflt) {
        lysp_qname_free((struct ly_ctx *)ctx, r->dflt);
        free(r->dflt);
    } else {
        FREE_ARRAY((struct ly_ctx *)ctx, r->dflts, lysp_qname_free);
    }
    free(r);
}

void
lysc_update_path(struct lysc_ctx *ctx, struct lysc_node *parent, const char *name)
{
    int len;
    uint8_t nextlevel = 0; /* 0 - no starttag, 1 - '/' starttag, 2 - '=' starttag + '}' endtag */

    if (!name) {
        /* removing last path segment */
        if (ctx->path[ctx->path_len - 1] == '}') {
            for ( ; ctx->path[ctx->path_len] != '=' && ctx->path[ctx->path_len] != '{'; --ctx->path_len) {}
            if (ctx->path[ctx->path_len] == '=') {
                ctx->path[ctx->path_len++] = '}';
            } else {
                /* not a top-level special tag, remove also preceiding '/' */
                goto remove_nodelevel;
            }
        } else {
remove_nodelevel:
            for ( ; ctx->path[ctx->path_len] != '/'; --ctx->path_len) {}
            if (ctx->path_len == 0) {
                /* top-level (last segment) */
                ctx->path_len = 1;
            }
        }
        /* set new terminating NULL-byte */
        ctx->path[ctx->path_len] = '\0';
    } else {
        if (ctx->path_len > 1) {
            if (!parent && (ctx->path[ctx->path_len - 1] == '}') && (ctx->path[ctx->path_len - 2] != '\'')) {
                /* extension of the special tag */
                nextlevel = 2;
                --ctx->path_len;
            } else {
                /* there is already some path, so add next level */
                nextlevel = 1;
            }
        } /* else the path is just initiated with '/', so do not add additional slash in case of top-level nodes */

        if (nextlevel != 2) {
            if ((parent && (parent->module == ctx->cur_mod)) || (!parent && (ctx->path_len > 1) && (name[0] == '{'))) {
                /* module not changed, print the name unprefixed */
                len = snprintf(&ctx->path[ctx->path_len], LYSC_CTX_BUFSIZE - ctx->path_len, "%s%s", nextlevel ? "/" : "", name);
            } else {
                len = snprintf(&ctx->path[ctx->path_len], LYSC_CTX_BUFSIZE - ctx->path_len, "%s%s:%s", nextlevel ? "/" : "", ctx->cur_mod->name, name);
            }
        } else {
            len = snprintf(&ctx->path[ctx->path_len], LYSC_CTX_BUFSIZE - ctx->path_len, "='%s'}", name);
        }
        if (len >= LYSC_CTX_BUFSIZE - (int)ctx->path_len) {
            /* output truncated */
            ctx->path_len = LYSC_CTX_BUFSIZE - 1;
        } else {
            ctx->path_len += len;
        }
    }
}

/**
 * @brief Stack for processing if-feature expressions.
 */
struct iff_stack {
    size_t size;    /**< number of items in the stack */
    size_t index;   /**< first empty item */
    uint8_t *stack; /**< stack - array of @ref ifftokens to create the if-feature expression in prefix format */
};

/**
 * @brief Add @ref ifftokens into the stack.
 * @param[in] stack The if-feature stack to use.
 * @param[in] value One of the @ref ifftokens to store in the stack.
 * @return LY_EMEM in case of memory allocation error
 * @return LY_ESUCCESS if the value successfully stored.
 */
static LY_ERR
iff_stack_push(struct iff_stack *stack, uint8_t value)
{
    if (stack->index == stack->size) {
        stack->size += 4;
        stack->stack = ly_realloc(stack->stack, stack->size * sizeof *stack->stack);
        LY_CHECK_ERR_RET(!stack->stack, LOGMEM(NULL); stack->size = 0, LY_EMEM);
    }
    stack->stack[stack->index++] = value;
    return LY_SUCCESS;
}

/**
 * @brief Get (and remove) the last item form the stack.
 * @param[in] stack The if-feature stack to use.
 * @return The value from the top of the stack.
 */
static uint8_t
iff_stack_pop(struct iff_stack *stack)
{
    assert(stack && stack->index);

    stack->index--;
    return stack->stack[stack->index];
}

/**
 * @brief Clean up the stack.
 * @param[in] stack The if-feature stack to use.
 */
static void
iff_stack_clean(struct iff_stack *stack)
{
    stack->size = 0;
    free(stack->stack);
}

/**
 * @brief Store the @ref ifftokens (@p op) on the given position in the 2bits array
 * (libyang format of the if-feature expression).
 * @param[in,out] list The 2bits array to modify.
 * @param[in] op The operand (@ref ifftokens) to store.
 * @param[in] pos Position (0-based) where to store the given @p op.
 */
static void
iff_setop(uint8_t *list, uint8_t op, size_t pos)
{
    uint8_t *item;
    uint8_t mask = 3;

    assert(op <= 3); /* max 2 bits */

    item = &list[pos / 4];
    mask = mask << 2 * (pos % 4);
    *item = (*item) & ~mask;
    *item = (*item) | (op << 2 * (pos % 4));
}

#define LYS_IFF_LP 0x04 /**< Additional, temporary, value of @ref ifftokens: ( */
#define LYS_IFF_RP 0x08 /**< Additional, temporary, value of @ref ifftokens: ) */

/**
 * @brief Find a feature of the given name and referenced in the given module.
 *
 * If the compiled schema is available (the schema is implemented), the feature from the compiled schema is
 * returned. Otherwise, the special array of pre-compiled features is used to search for the feature. Such
 * features are always disabled (feature from not implemented schema cannot be enabled), but in case the schema
 * will be made implemented in future (no matter if implicitly via augmenting/deviating it or explicitly via
 * ly_ctx_module_implement()), the compilation of these feature structure is finished, but the pointers
 * assigned till that time will be still valid.
 *
 * @param[in] pmod Module where the feature was referenced (used to resolve prefix of the feature).
 * @param[in] name Name of the feature including possible prefix.
 * @param[in] len Length of the string representing the feature identifier in the name variable (mandatory!).
 * @return Pointer to the feature structure if found, NULL otherwise.
 */
static struct lysc_feature *
lys_feature_find(const struct lysp_module *pmod, const char *name, size_t len)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_feature *f;
    const struct lys_module *mod;
    const char *ptr;

    assert(pmod);

    if ((ptr = ly_strnchr(name, ':', len))) {
        /* we have a prefixed feature */
        mod = lysp_module_find_prefix(pmod, name, ptr - name);
        LY_CHECK_RET(!mod, NULL);

        len = len - (ptr - name) - 1;
        name = ptr + 1;
    } else {
        /* local feature */
        mod = pmod->mod;
    }

    /* we have the correct module, get the feature */
    LY_ARRAY_FOR(mod->features, u) {
        f = &mod->features[u];
        if (!ly_strncmp(f->name, name, len)) {
            return f;
        }
    }

    return NULL;
}

LY_ERR
lys_compile_iffeature(struct lysc_ctx *ctx, struct lysp_qname *qname, struct lysc_iffeature *iff)
{
    LY_ERR rc = LY_SUCCESS;
    const char *c = qname->str;
    int64_t i, j;
    int8_t op_len, last_not = 0, checkversion = 0;
    LY_ARRAY_COUNT_TYPE f_size = 0, expr_size = 0, f_exp = 1;
    uint8_t op;
    struct iff_stack stack = {0, 0, NULL};
    struct lysc_feature *f;

    assert(c);

    /* pre-parse the expression to get sizes for arrays, also do some syntax checks of the expression */
    for (i = j = 0; c[i]; i++) {
        if (c[i] == '(') {
            j++;
            checkversion = 1;
            continue;
        } else if (c[i] == ')') {
            j--;
            continue;
        } else if (isspace(c[i])) {
            checkversion = 1;
            continue;
        }

        if (!strncmp(&c[i], "not", op_len = 3) || !strncmp(&c[i], "and", op_len = 3) || !strncmp(&c[i], "or", op_len = 2)) {
            uint64_t spaces;
            for (spaces = 0; c[i + op_len + spaces] && isspace(c[i + op_len + spaces]); spaces++) {}
            if (c[i + op_len + spaces] == '\0') {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                        "Invalid value \"%s\" of if-feature - unexpected end of expression.", qname->str);
                return LY_EVALID;
            } else if (!isspace(c[i + op_len])) {
                /* feature name starting with the not/and/or */
                last_not = 0;
                f_size++;
            } else if (c[i] == 'n') { /* not operation */
                if (last_not) {
                    /* double not */
                    expr_size = expr_size - 2;
                    last_not = 0;
                } else {
                    last_not = 1;
                }
            } else { /* and, or */
                if (f_exp != f_size) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                            "Invalid value \"%s\" of if-feature - missing feature/expression before \"%.*s\" operation.",
                            qname->str, op_len, &c[i]);
                    return LY_EVALID;
                }
                f_exp++;

                /* not a not operation */
                last_not = 0;
            }
            i += op_len;
        } else {
            f_size++;
            last_not = 0;
        }
        expr_size++;

        while (!isspace(c[i])) {
            if (!c[i] || (c[i] == ')') || (c[i] == '(')) {
                i--;
                break;
            }
            i++;
        }
    }
    if (j) {
        /* not matching count of ( and ) */
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                "Invalid value \"%s\" of if-feature - non-matching opening and closing parentheses.", qname->str);
        return LY_EVALID;
    }
    if (f_exp != f_size) {
        /* features do not match the needed arguments for the logical operations */
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                "Invalid value \"%s\" of if-feature - number of features in expression does not match "
                "the required number of operands for the operations.", qname->str);
        return LY_EVALID;
    }

    if (checkversion || (expr_size > 1)) {
        /* check that we have 1.1 module */
        if (qname->mod->version != LYS_VERSION_1_1) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                    "Invalid value \"%s\" of if-feature - YANG 1.1 expression in YANG 1.0 module.", qname->str);
            return LY_EVALID;
        }
    }

    /* allocate the memory */
    LY_ARRAY_CREATE_RET(ctx->ctx, iff->features, f_size, LY_EMEM);
    iff->expr = calloc((j = (expr_size / 4) + ((expr_size % 4) ? 1 : 0)), sizeof *iff->expr);
    stack.stack = malloc(expr_size * sizeof *stack.stack);
    LY_CHECK_ERR_GOTO(!stack.stack || !iff->expr, LOGMEM(ctx->ctx); rc = LY_EMEM, error);

    stack.size = expr_size;
    f_size--; expr_size--; /* used as indexes from now */

    for (i--; i >= 0; i--) {
        if (c[i] == ')') {
            /* push it on stack */
            iff_stack_push(&stack, LYS_IFF_RP);
            continue;
        } else if (c[i] == '(') {
            /* pop from the stack into result all operators until ) */
            while ((op = iff_stack_pop(&stack)) != LYS_IFF_RP) {
                iff_setop(iff->expr, op, expr_size--);
            }
            continue;
        } else if (isspace(c[i])) {
            continue;
        }

        /* end of operator or operand -> find beginning and get what is it */
        j = i + 1;
        while (i >= 0 && !isspace(c[i]) && c[i] != '(') {
            i--;
        }
        i++; /* go back by one step */

        if (!strncmp(&c[i], "not", 3) && isspace(c[i + 3])) {
            if (stack.index && (stack.stack[stack.index - 1] == LYS_IFF_NOT)) {
                /* double not */
                iff_stack_pop(&stack);
            } else {
                /* not has the highest priority, so do not pop from the stack
                 * as in case of AND and OR */
                iff_stack_push(&stack, LYS_IFF_NOT);
            }
        } else if (!strncmp(&c[i], "and", 3) && isspace(c[i + 3])) {
            /* as for OR - pop from the stack all operators with the same or higher
             * priority and store them to the result, then push the AND to the stack */
            while (stack.index && stack.stack[stack.index - 1] <= LYS_IFF_AND) {
                op = iff_stack_pop(&stack);
                iff_setop(iff->expr, op, expr_size--);
            }
            iff_stack_push(&stack, LYS_IFF_AND);
        } else if (!strncmp(&c[i], "or", 2) && isspace(c[i + 2])) {
            while (stack.index && stack.stack[stack.index - 1] <= LYS_IFF_OR) {
                op = iff_stack_pop(&stack);
                iff_setop(iff->expr, op, expr_size--);
            }
            iff_stack_push(&stack, LYS_IFF_OR);
        } else {
            /* feature name, length is j - i */

            /* add it to the expression */
            iff_setop(iff->expr, LYS_IFF_F, expr_size--);

            /* now get the link to the feature definition */
            f = lys_feature_find(qname->mod, &c[i], j - i);
            LY_CHECK_ERR_GOTO(!f, LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                    "Invalid value \"%s\" of if-feature - unable to find feature \"%.*s\".", qname->str, j - i, &c[i]);
                    rc = LY_EVALID, error)
            iff->features[f_size] = f;
            LY_ARRAY_INCREMENT(iff->features);
            f_size--;
        }
    }
    while (stack.index) {
        op = iff_stack_pop(&stack);
        iff_setop(iff->expr, op, expr_size--);
    }

    if (++expr_size || ++f_size) {
        /* not all expected operators and operands found */
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                "Invalid value \"%s\" of if-feature - processing error.", qname->str);
        rc = LY_EINT;
    } else {
        rc = LY_SUCCESS;
    }

error:
    /* cleanup */
    iff_stack_clean(&stack);

    return rc;
}

/**
 * @brief Compile information in the import statement - make sure there is the target module
 * @param[in] ctx Compile context.
 * @param[in] imp_p The parsed import statement structure to fill the module to.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_import(struct lysc_ctx *ctx, struct lysp_import *imp_p)
{
    const struct lys_module *mod = NULL;
    LY_ERR ret = LY_SUCCESS;

    /* make sure that we have the parsed version (lysp_) of the imported module to import groupings or typedefs.
     * The compiled version is needed only for augments, deviates and leafrefs, so they are checked (and added,
     * if needed) when these nodes are finally being instantiated and validated at the end of schema compilation. */
    if (!imp_p->module->parsed) {
        /* try to use filepath if present */
        if (imp_p->module->filepath) {
            struct ly_in *in;
            if (ly_in_new_filepath(imp_p->module->filepath, 0, &in)) {
                LOGINT(ctx->ctx);
            } else {
                LY_CHECK_RET(lys_parse(ctx->ctx, in, !strcmp(&imp_p->module->filepath[strlen(imp_p->module->filepath - 4)],
                        ".yin") ? LYS_IN_YIN : LYS_IN_YANG, &mod));
                if (mod != imp_p->module) {
                    LOGERR(ctx->ctx, LY_EINT, "Filepath \"%s\" of the module \"%s\" does not match.",
                            imp_p->module->filepath, imp_p->module->name);
                    mod = NULL;
                }
            }
            ly_in_free(in, 1);
        }
        if (!mod) {
            if (lysp_load_module(ctx->ctx, imp_p->module->name, imp_p->module->revision, 0, 1, (struct lys_module **)&mod)) {
                LOGERR(ctx->ctx, LY_ENOTFOUND, "Unable to reload \"%s\" module to import it into \"%s\", source data not found.",
                        imp_p->module->name, ctx->cur_mod->name);
                return LY_ENOTFOUND;
            }
        }
    }

    return ret;
}

LY_ERR
lys_identity_precompile(struct lysc_ctx *ctx_sc, struct ly_ctx *ctx, struct lysp_module *parsed_mod,
        struct lysp_ident *identities_p, struct lysc_ident **identities)
{
    LY_ARRAY_COUNT_TYPE offset = 0, u, v;
    struct lysc_ctx context = {0};
    LY_ERR ret = LY_SUCCESS;

    assert(ctx_sc || ctx);

    if (!ctx_sc) {
        context.ctx = ctx;
        context.cur_mod = parsed_mod->mod;
        context.pmod = parsed_mod;
        context.path_len = 1;
        context.path[0] = '/';
        ctx_sc = &context;
    }

    if (!identities_p) {
        return LY_SUCCESS;
    }
    if (*identities) {
        offset = LY_ARRAY_COUNT(*identities);
    }

    lysc_update_path(ctx_sc, NULL, "{identity}");
    LY_ARRAY_CREATE_RET(ctx_sc->ctx, *identities, LY_ARRAY_COUNT(identities_p), LY_EMEM);
    LY_ARRAY_FOR(identities_p, u) {
        lysc_update_path(ctx_sc, NULL, identities_p[u].name);

        LY_ARRAY_INCREMENT(*identities);
        COMPILE_CHECK_UNIQUENESS_ARRAY(ctx_sc, *identities, name, &(*identities)[offset + u], "identity", identities_p[u].name);
        DUP_STRING_GOTO(ctx_sc->ctx, identities_p[u].name, (*identities)[offset + u].name, ret, done);
        DUP_STRING_GOTO(ctx_sc->ctx, identities_p[u].dsc, (*identities)[offset + u].dsc, ret, done);
        DUP_STRING_GOTO(ctx_sc->ctx, identities_p[u].ref, (*identities)[offset + u].ref, ret, done);
        (*identities)[offset + u].module = ctx_sc->cur_mod;
        COMPILE_ARRAY_GOTO(ctx_sc, identities_p[u].iffeatures, (*identities)[offset + u].iffeatures, v,
                lys_compile_iffeature, ret, done);
        /* backlinks (derived) can be added no sooner than when all the identities in the current module are present */
        COMPILE_EXTS_GOTO(ctx_sc, identities_p[u].exts, (*identities)[offset + u].exts, &(*identities)[offset + u],
                LYEXT_PAR_IDENT, ret, done);
        (*identities)[offset + u].flags = identities_p[u].flags;

        lysc_update_path(ctx_sc, NULL, NULL);
    }
    lysc_update_path(ctx_sc, NULL, NULL);
done:
    return ret;
}

/**
 * @brief Check circular dependency of identities - identity MUST NOT reference itself (via their base statement).
 *
 * The function works in the same way as lys_compile_feature_circular_check() with different structures and error messages.
 *
 * @param[in] ctx Compile context for logging.
 * @param[in] ident The base identity (its derived list is being extended by the identity being currently processed).
 * @param[in] derived The list of derived identities of the identity being currently processed (not the one provided as @p ident)
 * @return LY_SUCCESS if everything is ok.
 * @return LY_EVALID if the identity is derived from itself.
 */
static LY_ERR
lys_compile_identity_circular_check(struct lysc_ctx *ctx, struct lysc_ident *ident, struct lysc_ident **derived)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct ly_set recursion = {0};
    struct lysc_ident *drv;

    if (!derived) {
        return LY_SUCCESS;
    }

    for (u = 0; u < LY_ARRAY_COUNT(derived); ++u) {
        if (ident == derived[u]) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                    "Identity \"%s\" is indirectly derived from itself.", ident->name);
            ret = LY_EVALID;
            goto cleanup;
        }
        ret = ly_set_add(&recursion, derived[u], 0, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    for (v = 0; v < recursion.count; ++v) {
        drv = recursion.objs[v];
        if (!drv->derived) {
            continue;
        }
        for (u = 0; u < LY_ARRAY_COUNT(drv->derived); ++u) {
            if (ident == drv->derived[u]) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                        "Identity \"%s\" is indirectly derived from itself.", ident->name);
                ret = LY_EVALID;
                goto cleanup;
            }
            ret = ly_set_add(&recursion, drv->derived[u], 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    ly_set_erase(&recursion, NULL);
    return ret;
}

LY_ERR
lys_compile_identity_bases(struct lysc_ctx *ctx, const struct lysp_module *base_pmod, const char **bases_p,
        struct lysc_ident *ident, struct lysc_ident ***bases)
{
    LY_ARRAY_COUNT_TYPE u, v;
    const char *s, *name;
    const struct lys_module *mod;
    struct lysc_ident **idref;

    assert(ident || bases);

    if ((LY_ARRAY_COUNT(bases_p) > 1) && (ctx->pmod->version < LYS_VERSION_1_1)) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                "Multiple bases in %s are allowed only in YANG 1.1 modules.", ident ? "identity" : "identityref type");
        return LY_EVALID;
    }

    LY_ARRAY_FOR(bases_p, u) {
        s = strchr(bases_p[u], ':');
        if (s) {
            /* prefixed identity */
            name = &s[1];
            mod = lysp_module_find_prefix(base_pmod, bases_p[u], s - bases_p[u]);
        } else {
            name = bases_p[u];
            mod = base_pmod->mod;
        }
        if (!mod) {
            if (ident) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                        "Invalid prefix used for base (%s) of identity \"%s\".", bases_p[u], ident->name);
            } else {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                        "Invalid prefix used for base (%s) of identityref.", bases_p[u]);
            }
            return LY_EVALID;
        }

        idref = NULL;
        LY_ARRAY_FOR(mod->identities, v) {
            if (!strcmp(name, mod->identities[v].name)) {
                if (ident) {
                    if (ident == &mod->identities[v]) {
                        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                                "Identity \"%s\" is derived from itself.", ident->name);
                        return LY_EVALID;
                    }
                    LY_CHECK_RET(lys_compile_identity_circular_check(ctx, &mod->identities[v], ident->derived));
                    /* we have match! store the backlink */
                    LY_ARRAY_NEW_RET(ctx->ctx, mod->identities[v].derived, idref, LY_EMEM);
                    *idref = ident;
                } else {
                    /* we have match! store the found identity */
                    LY_ARRAY_NEW_RET(ctx->ctx, *bases, idref, LY_EMEM);
                    *idref = &mod->identities[v];
                }
                break;
            }
        }
        if (!idref || !(*idref)) {
            if (ident) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                        "Unable to find base (%s) of identity \"%s\".", bases_p[u], ident->name);
            } else {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                        "Unable to find base (%s) of identityref.", bases_p[u]);
            }
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief For the given array of identities, set the backlinks from all their base identities.
 * @param[in] ctx Compile context, not only for logging but also to get the current module to resolve prefixes.
 * @param[in] idents_p Array of identities definitions from the parsed schema structure.
 * @param[in] idents Array of referencing identities to which the backlinks are supposed to be set.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_identities_derived(struct lysc_ctx *ctx, struct lysp_ident *idents_p, struct lysc_ident *idents)
{
    LY_ARRAY_COUNT_TYPE u;

    lysc_update_path(ctx, NULL, "{identity}");
    for (u = 0; u < LY_ARRAY_COUNT(idents_p); ++u) {
        if (!idents_p[u].bases) {
            continue;
        }
        lysc_update_path(ctx, NULL, idents[u].name);
        LY_CHECK_RET(lys_compile_identity_bases(ctx, idents[u].module->parsed, idents_p[u].bases, &idents[u], NULL));
        lysc_update_path(ctx, NULL, NULL);
    }
    lysc_update_path(ctx, NULL, NULL);
    return LY_SUCCESS;
}

LY_ERR
lys_feature_precompile(struct lysc_ctx *ctx_sc, struct ly_ctx *ctx, struct lysp_module *parsed_mod,
        struct lysp_feature *features_p, struct lysc_feature **features)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE offset = 0, u;
    struct lysc_ctx context = {0};

    assert(ctx_sc || ctx);

    if (!ctx_sc) {
        context.ctx = ctx;
        context.cur_mod = parsed_mod->mod;
        context.pmod = parsed_mod;
        context.path_len = 1;
        context.path[0] = '/';
        ctx_sc = &context;
    }

    if (!features_p) {
        return LY_SUCCESS;
    }
    if (*features) {
        offset = LY_ARRAY_COUNT(*features);
    }

    lysc_update_path(ctx_sc, NULL, "{feature}");
    LY_ARRAY_CREATE_RET(ctx_sc->ctx, *features, LY_ARRAY_COUNT(features_p), LY_EMEM);
    LY_ARRAY_FOR(features_p, u) {
        lysc_update_path(ctx_sc, NULL, features_p[u].name);

        LY_ARRAY_INCREMENT(*features);
        COMPILE_CHECK_UNIQUENESS_ARRAY(ctx_sc, *features, name, &(*features)[offset + u], "feature", features_p[u].name);
        DUP_STRING_GOTO(ctx_sc->ctx, features_p[u].name, (*features)[offset + u].name, ret, done);
        DUP_STRING_GOTO(ctx_sc->ctx, features_p[u].dsc, (*features)[offset + u].dsc, ret, done);
        DUP_STRING_GOTO(ctx_sc->ctx, features_p[u].ref, (*features)[offset + u].ref, ret, done);
        (*features)[offset + u].flags = features_p[u].flags;
        (*features)[offset + u].module = ctx_sc->cur_mod;

        lysc_update_path(ctx_sc, NULL, NULL);
    }
    lysc_update_path(ctx_sc, NULL, NULL);

done:
    return ret;
}

/**
 * @brief Check circular dependency of features - feature MUST NOT reference itself (via their if-feature statement).
 *
 * The function works in the same way as lys_compile_identity_circular_check() with different structures and error messages.
 *
 * @param[in] ctx Compile context for logging.
 * @param[in] feature The feature referenced in if-feature statement (its depfeatures list is being extended by the feature
 *            being currently processed).
 * @param[in] depfeatures The list of depending features of the feature being currently processed (not the one provided as @p feature)
 * @return LY_SUCCESS if everything is ok.
 * @return LY_EVALID if the feature references indirectly itself.
 */
static LY_ERR
lys_compile_feature_circular_check(struct lysc_ctx *ctx, struct lysc_feature *feature, struct lysc_feature **depfeatures)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct ly_set recursion = {0};
    struct lysc_feature *drv;

    if (!depfeatures) {
        return LY_SUCCESS;
    }

    for (u = 0; u < LY_ARRAY_COUNT(depfeatures); ++u) {
        if (feature == depfeatures[u]) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                    "Feature \"%s\" is indirectly referenced from itself.", feature->name);
            ret = LY_EVALID;
            goto cleanup;
        }
        ret = ly_set_add(&recursion, depfeatures[u], 0, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    for (v = 0; v < recursion.count; ++v) {
        drv = recursion.objs[v];
        if (!drv->depfeatures) {
            continue;
        }
        for (u = 0; u < LY_ARRAY_COUNT(drv->depfeatures); ++u) {
            if (feature == drv->depfeatures[u]) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                        "Feature \"%s\" is indirectly referenced from itself.", feature->name);
                ret = LY_EVALID;
                goto cleanup;
            }
            ly_set_add(&recursion, drv->depfeatures[u], 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    ly_set_erase(&recursion, NULL);
    return ret;
}

/**
 * @brief Create pre-compiled features array.
 *
 * See lys_feature_precompile() for more details.
 *
 * @param[in] ctx Compile context.
 * @param[in] feature_p Parsed feature definition to compile.
 * @param[in,out] features List of already (pre)compiled features to find the corresponding precompiled feature structure.
 * @return LY_ERR value.
 */
static LY_ERR
lys_feature_precompile_finish(struct lysc_ctx *ctx, struct lysp_feature *feature_p, struct lysc_feature *features)
{
    LY_ARRAY_COUNT_TYPE u, v, x;
    struct lysc_feature *feature, **df;
    LY_ERR ret = LY_SUCCESS;

    /* find the preprecompiled feature */
    LY_ARRAY_FOR(features, x) {
        if (strcmp(features[x].name, feature_p->name)) {
            continue;
        }
        feature = &features[x];
        lysc_update_path(ctx, NULL, "{feature}");
        lysc_update_path(ctx, NULL, feature_p->name);

        /* finish compilation started in lys_feature_precompile() */
        COMPILE_EXTS_GOTO(ctx, feature_p->exts, feature->exts, feature, LYEXT_PAR_FEATURE, ret, done);
        COMPILE_ARRAY_GOTO(ctx, feature_p->iffeatures, feature->iffeatures, u, lys_compile_iffeature, ret, done);
        if (feature->iffeatures) {
            for (u = 0; u < LY_ARRAY_COUNT(feature->iffeatures); ++u) {
                if (feature->iffeatures[u].features) {
                    for (v = 0; v < LY_ARRAY_COUNT(feature->iffeatures[u].features); ++v) {
                        /* check for circular dependency - direct reference first,... */
                        if (feature == feature->iffeatures[u].features[v]) {
                            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                                    "Feature \"%s\" is referenced from itself.", feature->name);
                            return LY_EVALID;
                        }
                        /* ... and indirect circular reference */
                        LY_CHECK_RET(lys_compile_feature_circular_check(ctx, feature->iffeatures[u].features[v], feature->depfeatures));

                        /* add itself into the dependants list */
                        LY_ARRAY_NEW_RET(ctx->ctx, feature->iffeatures[u].features[v]->depfeatures, df, LY_EMEM);
                        *df = feature;
                    }
                }
            }
        }
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
done:
        return ret;
    }

    LOGINT(ctx->ctx);
    return LY_EINT;
}

void
lys_feature_precompile_revert(struct lysc_ctx *ctx, struct lys_module *mod)
{
    LY_ARRAY_COUNT_TYPE u, v;

    /* in the dis_features list, remove all the parts (from finished compiling process)
     * which may points into the data being freed here */
    LY_ARRAY_FOR(mod->features, u) {
        LY_ARRAY_FOR(mod->features[u].iffeatures, v) {
            lysc_iffeature_free(ctx->ctx, &mod->features[u].iffeatures[v]);
        }
        LY_ARRAY_FREE(mod->features[u].iffeatures);
        mod->features[u].iffeatures = NULL;

        LY_ARRAY_FOR(mod->features[u].exts, v) {
            lysc_ext_instance_free(ctx->ctx, &(mod->features[u].exts)[v]);
        }
        LY_ARRAY_FREE(mod->features[u].exts);
        mod->features[u].exts = NULL;
    }
}

/**
 * @brief Check the features used in if-feature statements applicable to the leafref and its target.
 *
 * The set of features used for target must be a subset of features used for the leafref.
 * This is not a perfect, we should compare the truth tables but it could require too much resources
 * and RFC 7950 does not require it explicitely, so we simplify that.
 *
 * @param[in] refnode The leafref node.
 * @param[in] target Tha target node of the leafref.
 * @return LY_SUCCESS or LY_EVALID;
 */
static LY_ERR
lys_compile_leafref_features_validate(const struct lysc_node *refnode, const struct lysc_node *target)
{
    LY_ERR ret = LY_EVALID;
    const struct lysc_node *iter;
    LY_ARRAY_COUNT_TYPE u, v;
    struct ly_set features = {0};

    for (iter = refnode; iter; iter = iter->parent) {
        if (iter->iffeatures) {
            LY_ARRAY_FOR(iter->iffeatures, u) {
                LY_ARRAY_FOR(iter->iffeatures[u].features, v) {
                    LY_CHECK_GOTO(ly_set_add(&features, iter->iffeatures[u].features[v], 0, NULL), cleanup);
                }
            }
        }
    }

    /* we should have, in features set, a superset of features applicable to the target node.
     * If the feature is not present, we don;t have a subset of features applicable
     * to the leafref itself. */
    for (iter = target; iter; iter = iter->parent) {
        if (iter->iffeatures) {
            LY_ARRAY_FOR(iter->iffeatures, u) {
                LY_ARRAY_FOR(iter->iffeatures[u].features, v) {
                    if (!ly_set_contains(&features, iter->iffeatures[u].features[v], NULL)) {
                        /* feature not present */
                        goto cleanup;
                    }
                }
            }
        }
    }
    ret = LY_SUCCESS;

cleanup:
    ly_set_erase(&features, NULL);
    return ret;
}

static void *
lys_compile_extension_instance_storage(enum ly_stmt stmt, struct lysc_ext_substmt *substmts)
{
    for (LY_ARRAY_COUNT_TYPE u = 0; substmts[u].stmt; ++u) {
        if (substmts[u].stmt == stmt) {
            return substmts[u].storage;
        }
    }
    return NULL;
}

LY_ERR
lys_compile_extension_instance(struct lysc_ctx *ctx, const struct lysp_ext_instance *ext, struct lysc_ext_substmt *substmts)
{
    LY_ERR ret = LY_EVALID, r;
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_stmt *stmt;
    struct lysp_qname qname;
    void *parsed = NULL, **compiled = NULL;

    /* check for invalid substatements */
    for (stmt = ext->child; stmt; stmt = stmt->next) {
        if (stmt->flags & (LYS_YIN_ATTR | LYS_YIN_ARGUMENT)) {
            continue;
        }
        for (u = 0; substmts[u].stmt; ++u) {
            if (substmts[u].stmt == stmt->kw) {
                break;
            }
        }
        if (!substmts[u].stmt) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s%s%s\" extension instance.",
                    stmt->stmt, ext->name, ext->argument ? " " : "", ext->argument ? ext->argument : "");
            goto cleanup;
        }
    }

    /* TODO store inherited data, e.g. status first, but mark them somehow to allow to overwrite them and not detect duplicity */

    /* keep order of the processing the same as the order in the defined substmts,
     * the order is important for some of the statements depending on others (e.g. type needs status and units) */
    for (u = 0; substmts[u].stmt; ++u) {
        ly_bool stmt_present = 0;

        for (stmt = ext->child; stmt; stmt = stmt->next) {
            if (substmts[u].stmt != stmt->kw) {
                continue;
            }

            stmt_present = 1;
            if (substmts[u].storage) {
                switch (stmt->kw) {
                case LY_STMT_STATUS:
                    assert(substmts[u].cardinality < LY_STMT_CARD_SOME);
                    LY_CHECK_ERR_GOTO(r = lysp_stmt_parse(ctx, stmt, stmt->kw, &substmts[u].storage, /* TODO */ NULL), ret = r, cleanup);
                    break;
                case LY_STMT_UNITS: {
                    const char **units;

                    if (substmts[u].cardinality < LY_STMT_CARD_SOME) {
                        /* single item */
                        if (*((const char **)substmts[u].storage)) {
                            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_DUPSTMT, stmt->stmt);
                            goto cleanup;
                        }
                        units = (const char **)substmts[u].storage;
                    } else {
                        /* sized array */
                        const char ***units_array = (const char ***)substmts[u].storage;
                        LY_ARRAY_NEW_GOTO(ctx->ctx, *units_array, units, ret, cleanup);
                    }
                    r = lydict_insert(ctx->ctx, stmt->arg, 0, units);
                    LY_CHECK_ERR_GOTO(r, ret = r, cleanup);
                    break;
                }
                case LY_STMT_TYPE: {
                    uint16_t *flags = lys_compile_extension_instance_storage(LY_STMT_STATUS, substmts);
                    const char **units = lys_compile_extension_instance_storage(LY_STMT_UNITS, substmts);

                    if (substmts[u].cardinality < LY_STMT_CARD_SOME) {
                        /* single item */
                        if (*(struct lysc_type **)substmts[u].storage) {
                            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_DUPSTMT, stmt->stmt);
                            goto cleanup;
                        }
                        compiled = substmts[u].storage;
                    } else {
                        /* sized array */
                        struct lysc_type ***types = (struct lysc_type ***)substmts[u].storage, **type = NULL;
                        LY_ARRAY_NEW_GOTO(ctx->ctx, *types, type, ret, cleanup);
                        compiled = (void *)type;
                    }

                    r = lysp_stmt_parse(ctx, stmt, stmt->kw, &parsed, NULL);
                    LY_CHECK_ERR_GOTO(r, ret = r, cleanup);
                    r = lys_compile_type(ctx, ext->parent_type == LYEXT_PAR_NODE ? ((struct lysc_node *)ext->parent)->sp : NULL,
                            flags ? *flags : 0, ctx->pmod, ext->name, parsed, (struct lysc_type **)compiled,
                            units && !*units ? units : NULL, NULL);
                    lysp_type_free(ctx->ctx, parsed);
                    free(parsed);
                    LY_CHECK_ERR_GOTO(r, ret = r, cleanup);
                    break;
                }
                case LY_STMT_IF_FEATURE: {
                    struct lysc_iffeature *iff = NULL;

                    if (substmts[u].cardinality < LY_STMT_CARD_SOME) {
                        /* single item */
                        if (((struct lysc_iffeature *)substmts[u].storage)->features) {
                            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_DUPSTMT, stmt->stmt);
                            goto cleanup;
                        }
                        iff = (struct lysc_iffeature *)substmts[u].storage;
                    } else {
                        /* sized array */
                        struct lysc_iffeature **iffs = (struct lysc_iffeature **)substmts[u].storage;
                        LY_ARRAY_NEW_GOTO(ctx->ctx, *iffs, iff, ret, cleanup);
                    }
                    qname.str = stmt->arg;
                    qname.mod = ctx->pmod;
                    LY_CHECK_ERR_GOTO(r = lys_compile_iffeature(ctx, &qname, iff), ret = r, cleanup);
                    break;
                }
                /* TODO support other substatements (parse stmt to lysp and then compile lysp to lysc),
                 * also note that in many statements their extensions are not taken into account  */
                default:
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG, "Statement \"%s\" is not supported as an extension (found in \"%s%s%s\") substatement.",
                            stmt->stmt, ext->name, ext->argument ? " " : "", ext->argument ? ext->argument : "");
                    goto cleanup;
                }
            }
        }

        if (((substmts[u].cardinality == LY_STMT_CARD_MAND) || (substmts[u].cardinality == LY_STMT_CARD_SOME)) && !stmt_present) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG, "Missing mandatory keyword \"%s\" as a child of \"%s%s%s\".",
                    ly_stmt2str(substmts[u].stmt), ext->name, ext->argument ? " " : "", ext->argument ? ext->argument : "");
            goto cleanup;
        }
    }

    ret = LY_SUCCESS;

cleanup:
    return ret;
}

/**
 * @brief Check when for cyclic dependencies.
 *
 * @param[in] set Set with all the referenced nodes.
 * @param[in] node Node whose "when" referenced nodes are in @p set.
 * @return LY_ERR value
 */
static LY_ERR
lys_compile_unres_when_cyclic(struct lyxp_set *set, const struct lysc_node *node)
{
    struct lyxp_set tmp_set;
    struct lyxp_set_scnode *xp_scnode;
    uint32_t i, j;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_when *when;
    LY_ERR ret = LY_SUCCESS;

    memset(&tmp_set, 0, sizeof tmp_set);

    /* prepare in_ctx of the set */
    for (i = 0; i < set->used; ++i) {
        xp_scnode = &set->val.scnodes[i];

        if (xp_scnode->in_ctx != -1) {
            /* check node when, skip the context node (it was just checked) */
            xp_scnode->in_ctx = 1;
        }
    }

    for (i = 0; i < set->used; ++i) {
        xp_scnode = &set->val.scnodes[i];
        if (xp_scnode->in_ctx != 1) {
            /* already checked */
            continue;
        }

        if ((xp_scnode->type != LYXP_NODE_ELEM) || (xp_scnode->scnode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) ||
                !xp_scnode->scnode->when) {
            /* no when to check */
            xp_scnode->in_ctx = 0;
            continue;
        }

        node = xp_scnode->scnode;
        do {
            LY_ARRAY_FOR(node->when, u) {
                when = node->when[u];
                ret = lyxp_atomize(when->cond, node->module, LY_PREF_SCHEMA_RESOLVED, when->prefixes, when->context,
                        &tmp_set, LYXP_SCNODE_SCHEMA);
                if (ret != LY_SUCCESS) {
                    LOGVAL(set->ctx, LY_VLOG_LYSC, node, LYVE_SEMANTICS, "Invalid when condition \"%s\".", when->cond->expr);
                    goto cleanup;
                }

                for (j = 0; j < tmp_set.used; ++j) {
                    /* skip roots'n'stuff */
                    if (tmp_set.val.scnodes[j].type == LYXP_NODE_ELEM) {
                        /* try to find this node in our set */
                        uint32_t idx;
                        if (lyxp_set_scnode_contains(set, tmp_set.val.scnodes[j].scnode, LYXP_NODE_ELEM, -1, &idx) && (set->val.scnodes[idx].in_ctx == -1)) {
                            LOGVAL(set->ctx, LY_VLOG_LYSC, node, LY_VCODE_CIRC_WHEN, node->name, set->val.scnodes[idx].scnode->name);
                            ret = LY_EVALID;
                            goto cleanup;
                        }

                        /* needs to be checked, if in both sets, will be ignored */
                        tmp_set.val.scnodes[j].in_ctx = 1;
                    } else {
                        /* no when, nothing to check */
                        tmp_set.val.scnodes[j].in_ctx = 0;
                    }
                }

                /* merge this set into the global when set */
                lyxp_set_scnode_merge(set, &tmp_set);
            }

            /* check when of non-data parents as well */
            node = node->parent;
        } while (node && (node->nodetype & (LYS_CASE | LYS_CHOICE)));

        /* this node when was checked (xp_scnode could have been reallocd) */
        set->val.scnodes[i].in_ctx = -1;
    }

cleanup:
    lyxp_set_free_content(&tmp_set);
    return ret;
}

LY_ERR
lysc_check_status(struct lysc_ctx *ctx, uint16_t flags1, void *mod1, const char *name1, uint16_t flags2, void *mod2,
        const char *name2)
{
    uint16_t flg1, flg2;

    flg1 = (flags1 & LYS_STATUS_MASK) ? (flags1 & LYS_STATUS_MASK) : LYS_STATUS_CURR;
    flg2 = (flags2 & LYS_STATUS_MASK) ? (flags2 & LYS_STATUS_MASK) : LYS_STATUS_CURR;

    if ((flg1 < flg2) && (mod1 == mod2)) {
        if (ctx) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                    "A %s definition \"%s\" is not allowed to reference %s definition \"%s\".",
                    flg1 == LYS_STATUS_CURR ? "current" : "deprecated", name1,
                    flg2 == LYS_STATUS_OBSLT ? "obsolete" : "deprecated", name2);
        }
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Check parsed expression for any prefixes of unimplemented modules.
 *
 * @param[in] ctx libyang context.
 * @param[in] expr Parsed expression.
 * @param[in] format Prefix format.
 * @param[in] prefix_data Format-specific data (see ::ly_resolve_prefix()).
 * @param[out] mod_p Optional module that is not implemented.
 * @return Whether all the found modules are implemented or at least one is not.
 */
static ly_bool
lys_compile_expr_target_is_implemented(const struct ly_ctx *ctx, const struct lyxp_expr *expr, LY_PREFIX_FORMAT format,
        void *prefix_data, const struct lys_module **mod_p)
{
    uint32_t i;
    const char *ptr, *start;
    const struct lys_module *mod;

    for (i = 0; i < expr->used; ++i) {
        if ((expr->tokens[i] != LYXP_TOKEN_NAMETEST) && (expr->tokens[i] != LYXP_TOKEN_LITERAL)) {
            /* token cannot have a prefix */
            continue;
        }

        start = expr->expr + expr->tok_pos[i];
        if (!(ptr = ly_strnchr(start, ':', expr->tok_len[i]))) {
            /* token without a prefix */
            continue;
        }

        if (!(mod = ly_resolve_prefix(ctx, start, ptr - start, format, prefix_data))) {
            /* unknown prefix, do not care right now */
            continue;
        }

        if (!mod->implemented) {
            /* unimplemented module found */
            if (mod_p) {
                *mod_p = mod;
            }
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Check when/must expressions of a node on a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Node to check.
 * @return LY_ERR value
 */
static LY_ERR
lys_compile_unres_xpath(struct lysc_ctx *ctx, const struct lysc_node *node)
{
    struct lyxp_set tmp_set;
    uint32_t i, opts;
    LY_ARRAY_COUNT_TYPE u;
    ly_bool input_done = 0;
    struct lysc_when **when = NULL;
    struct lysc_must *musts = NULL;
    LY_ERR ret = LY_SUCCESS;
    const struct lysc_node *op;
    const struct lys_module *mod;

    memset(&tmp_set, 0, sizeof tmp_set);
    opts = LYXP_SCNODE_SCHEMA;
    if (node->flags & LYS_CONFIG_R) {
        for (op = node->parent; op && !(op->nodetype & (LYS_RPC | LYS_ACTION)); op = op->parent) {}
        if (op) {
            /* we are actually in output */
            opts = LYXP_SCNODE_OUTPUT;
        }
    }

    switch (node->nodetype) {
    case LYS_CONTAINER:
        when = ((struct lysc_node_container *)node)->when;
        musts = ((struct lysc_node_container *)node)->musts;
        break;
    case LYS_CHOICE:
        when = ((struct lysc_node_choice *)node)->when;
        break;
    case LYS_LEAF:
        when = ((struct lysc_node_leaf *)node)->when;
        musts = ((struct lysc_node_leaf *)node)->musts;
        break;
    case LYS_LEAFLIST:
        when = ((struct lysc_node_leaflist *)node)->when;
        musts = ((struct lysc_node_leaflist *)node)->musts;
        break;
    case LYS_LIST:
        when = ((struct lysc_node_list *)node)->when;
        musts = ((struct lysc_node_list *)node)->musts;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        when = ((struct lysc_node_anydata *)node)->when;
        musts = ((struct lysc_node_anydata *)node)->musts;
        break;
    case LYS_CASE:
        when = ((struct lysc_node_case *)node)->when;
        break;
    case LYS_NOTIF:
        when = ((struct lysc_notif *)node)->when;
        musts = ((struct lysc_notif *)node)->musts;
        break;
    case LYS_RPC:
    case LYS_ACTION:
        /* first process when and input musts */
        when = ((struct lysc_action *)node)->when;
        musts = ((struct lysc_action *)node)->input.musts;
        break;
    default:
        /* nothing to check */
        break;
    }

    LY_ARRAY_FOR(when, u) {
        /* first check whether all the referenced modules are implemented */
        if (!lys_compile_expr_target_is_implemented(ctx->ctx, when[u]->cond, LY_PREF_SCHEMA_RESOLVED,
                when[u]->prefixes, &mod)) {
            LOGWRN(ctx->ctx, "When condition \"%s\" check skipped because referenced module \"%s\" is not implemented.",
                    when[u]->cond->expr, mod->name);
            continue;
        }

        /* check "when" */
        ret = lyxp_atomize(when[u]->cond, node->module, LY_PREF_SCHEMA_RESOLVED, when[u]->prefixes, when[u]->context,
                &tmp_set, opts);
        if (ret != LY_SUCCESS) {
            LOGVAL(ctx->ctx, LY_VLOG_LYSC, node, LYVE_SEMANTICS, "Invalid when condition \"%s\".", when[u]->cond->expr);
            goto cleanup;
        }

        ctx->path[0] = '\0';
        lysc_path((struct lysc_node *)node, LYSC_PATH_LOG, ctx->path, LYSC_CTX_BUFSIZE);
        for (i = 0; i < tmp_set.used; ++i) {
            /* skip roots'n'stuff */
            if ((tmp_set.val.scnodes[i].type == LYXP_NODE_ELEM) && (tmp_set.val.scnodes[i].in_ctx != -1)) {
                struct lysc_node *schema = tmp_set.val.scnodes[i].scnode;

                /* XPath expression cannot reference "lower" status than the node that has the definition */
                ret = lysc_check_status(ctx, when[u]->flags, node->module, node->name, schema->flags, schema->module,
                        schema->name);
                LY_CHECK_GOTO(ret, cleanup);

                /* check dummy node accessing */
                if (schema == node) {
                    LOGVAL(ctx->ctx, LY_VLOG_LYSC, node, LY_VCODE_DUMMY_WHEN, node->name);
                    ret = LY_EVALID;
                    goto cleanup;
                }
            }
        }

        /* check cyclic dependencies */
        ret = lys_compile_unres_when_cyclic(&tmp_set, node);
        LY_CHECK_GOTO(ret, cleanup);

        lyxp_set_free_content(&tmp_set);
    }

check_musts:
    LY_ARRAY_FOR(musts, u) {
        /* first check whether all the referenced modules are implemented */
        if (!lys_compile_expr_target_is_implemented(ctx->ctx, musts[u].cond, LY_PREF_SCHEMA_RESOLVED,
                musts[u].prefixes, &mod)) {
            LOGWRN(ctx->ctx, "Must condition \"%s\" check skipped because referenced module \"%s\" is not implemented.",
                    musts[u].cond->expr, mod->name);
            continue;
        }

        /* check "must" */
        ret = lyxp_atomize(musts[u].cond, node->module, LY_PREF_SCHEMA_RESOLVED, musts[u].prefixes, node, &tmp_set, opts);
        if (ret != LY_SUCCESS) {
            LOGVAL(ctx->ctx, LY_VLOG_LYSC, node, LYVE_SEMANTICS, "Invalid must restriction \"%s\".", musts[u].cond->expr);
            goto cleanup;
        }

        ctx->path[0] = '\0';
        lysc_path((struct lysc_node *)node, LYSC_PATH_LOG, ctx->path, LYSC_CTX_BUFSIZE);
        for (i = 0; i < tmp_set.used; ++i) {
            /* skip roots'n'stuff */
            if (tmp_set.val.scnodes[i].type == LYXP_NODE_ELEM) {
                /* XPath expression cannot reference "lower" status than the node that has the definition */
                ret = lysc_check_status(ctx, node->flags, node->module, node->name, tmp_set.val.scnodes[i].scnode->flags,
                        tmp_set.val.scnodes[i].scnode->module, tmp_set.val.scnodes[i].scnode->name);
                LY_CHECK_GOTO(ret, cleanup);
            }
        }

        lyxp_set_free_content(&tmp_set);
    }

    if ((node->nodetype & (LYS_RPC | LYS_ACTION)) && !input_done) {
        /* now check output musts */
        input_done = 1;
        when = NULL;
        musts = ((struct lysc_action *)node)->output.musts;
        opts = LYXP_SCNODE_OUTPUT;
        goto check_musts;
    }

cleanup:
    lyxp_set_free_content(&tmp_set);
    return ret;
}

/**
 * @brief Check leafref for its target existence on a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Context node for the leafref.
 * @param[in] lref Leafref to check/resolve.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_leafref(struct lysc_ctx *ctx, const struct lysc_node *node, struct lysc_type_leafref *lref)
{
    const struct lysc_node *target = NULL, *siter;
    struct ly_path *p;
    struct lysc_type *type;

    assert(node->nodetype & (LYS_LEAF | LYS_LEAFLIST));

    /* try to find the target */
    LY_CHECK_RET(ly_path_compile(ctx->ctx, node->module, node, lref->path, LY_PATH_LREF_TRUE, lysc_is_output(node) ?
            LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_MANY, LY_PREF_SCHEMA_RESOLVED, lref->prefixes, &p));

    /* get the target node */
    target = p[LY_ARRAY_COUNT(p) - 1].node;
    ly_path_free(node->module->ctx, p);

    if (!(target->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGVAL(ctx->ctx, LY_VLOG_LYSC, node, LYVE_REFERENCE,
                "Invalid leafref path \"%s\" - target node is %s instead of leaf or leaf-list.",
                lref->path->expr, lys_nodetype2str(target->nodetype));
        return LY_EVALID;
    }

    /* check status */
    ctx->path[0] = '\0';
    lysc_path(node, LYSC_PATH_LOG, ctx->path, LYSC_CTX_BUFSIZE);
    ctx->path_len = strlen(ctx->path);
    if (lysc_check_status(ctx, node->flags, node->module, node->name, target->flags, target->module, target->name)) {
        return LY_EVALID;
    }
    ctx->path_len = 1;
    ctx->path[1] = '\0';

    /* check config */
    if (lref->require_instance) {
        for (siter = node->parent; siter && !(siter->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)); siter = siter->parent) {}
        if (!siter && (node->flags & LYS_CONFIG_W) && (target->flags & LYS_CONFIG_R)) {
            LOGVAL(ctx->ctx, LY_VLOG_LYSC, node, LYVE_REFERENCE, "Invalid leafref path \"%s\" - target is supposed"
                    " to represent configuration data (as the leafref does), but it does not.", lref->path->expr);
            return LY_EVALID;
        }
    }

    /* store the target's type and check for circular chain of leafrefs */
    lref->realtype = ((struct lysc_node_leaf *)target)->type;
    for (type = lref->realtype; type && type->basetype == LY_TYPE_LEAFREF; type = ((struct lysc_type_leafref *)type)->realtype) {
        if (type == (struct lysc_type *)lref) {
            /* circular chain detected */
            LOGVAL(ctx->ctx, LY_VLOG_LYSC, node, LYVE_REFERENCE,
                    "Invalid leafref path \"%s\" - circular chain of leafrefs detected.", lref->path->expr);
            return LY_EVALID;
        }
    }

    /* check if leafref and its target are under common if-features */
    if (lys_compile_leafref_features_validate(node, target)) {
        LOGVAL(ctx->ctx, LY_VLOG_LYSC, node, LYVE_REFERENCE,
                "Invalid leafref path \"%s\" - set of features applicable to the leafref target is not a subset of"
                " features applicable to the leafref itself.", lref->path->expr);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

static LY_ERR
lys_compile_ietf_netconf_wd_annotation(struct lysc_ctx *ctx, struct lys_module *mod)
{
    struct lysc_ext_instance *ext;
    struct lysp_ext_instance *ext_p = NULL;
    struct lysp_stmt *stmt;
    const struct lys_module *ext_mod;
    LY_ERR ret = LY_SUCCESS;

    /* create the parsed extension instance manually */
    ext_p = calloc(1, sizeof *ext_p);
    LY_CHECK_ERR_GOTO(!ext_p, LOGMEM(ctx->ctx); ret = LY_EMEM, cleanup);
    LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, "md:annotation", 0, &ext_p->name), cleanup);
    LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, "default", 0, &ext_p->argument), cleanup);
    ext_p->insubstmt = LYEXT_SUBSTMT_SELF;
    ext_p->insubstmt_index = 0;

    ext_p->child = stmt = calloc(1, sizeof *ext_p->child);
    LY_CHECK_ERR_GOTO(!stmt, LOGMEM(ctx->ctx); ret = LY_EMEM, cleanup);
    LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, "type", 0, &stmt->stmt), cleanup);
    LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, "boolean", 0, &stmt->arg), cleanup);
    stmt->kw = LY_STMT_TYPE;

    /* allocate new extension instance */
    LY_ARRAY_NEW_GOTO(mod->ctx, mod->compiled->exts, ext, ret, cleanup);

    /* manually get extension definition module */
    ext_mod = ly_ctx_get_module_latest(ctx->ctx, "ietf-yang-metadata");

    /* compile the extension instance */
    LY_CHECK_GOTO(ret = lys_compile_ext(ctx, ext_p, ext, mod->compiled, LYEXT_PAR_MODULE, ext_mod), cleanup);

cleanup:
    lysp_ext_instance_free(ctx->ctx, ext_p);
    free(ext_p);
    return ret;
}

/**
 * @brief Compile default value(s) for leaf or leaf-list expecting a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Leaf or leaf-list to compile the default value(s) for.
 * @param[in] type Type of the default value.
 * @param[in] dflt Default value.
 * @param[in] dflt_pmod Parsed module of the @p dflt to resolve possible prefixes.
 * @param[in,out] storage Storage for the compiled default value.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_dflt(struct lysc_ctx *ctx, struct lysc_node *node, struct lysc_type *type, const char *dflt,
        const struct lysp_module *dflt_pmod, struct lyd_value *storage)
{
    LY_ERR ret;
    struct ly_err_item *err = NULL;

    ret = type->plugin->store(ctx->ctx, type, dflt, strlen(dflt), 0, LY_PREF_SCHEMA, (void *)dflt_pmod,
            LYD_HINT_SCHEMA, node, storage, &err);
    if (ret == LY_EINCOMPLETE) {
        /* we have no data so we will not be resolving it */
        ret = LY_SUCCESS;
    }

    if (ret) {
        ctx->path[0] = '\0';
        lysc_path(node, LYSC_PATH_LOG, ctx->path, LYSC_CTX_BUFSIZE);
        if (err) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                    "Invalid default - value does not fit the type (%s).", err->msg);
            ly_err_free(err);
        } else {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                    "Invalid default - value does not fit the type.");
        }
        return ret;
    }

    ++((struct lysc_type *)storage->realtype)->refcount;
    return LY_SUCCESS;
}

/**
 * @brief Compile default value of a leaf expecting a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] leaf Leaf that the default value is for.
 * @param[in] dflt Default value to compile.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_leaf_dlft(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf, struct lysp_qname *dflt)
{
    LY_ERR ret;

    assert(!leaf->dflt);

    if (leaf->flags & (LYS_MAND_TRUE | LYS_KEY)) {
        /* ignore default values for keys and mandatory leaves */
        return LY_SUCCESS;
    }

    /* allocate the default value */
    leaf->dflt = calloc(1, sizeof *leaf->dflt);
    LY_CHECK_ERR_RET(!leaf->dflt, LOGMEM(ctx->ctx), LY_EMEM);

    /* store the default value */
    ret = lys_compile_unres_dflt(ctx, (struct lysc_node *)leaf, leaf->type, dflt->str, dflt->mod, leaf->dflt);
    if (ret) {
        free(leaf->dflt);
        leaf->dflt = NULL;
    }

    return ret;
}

/**
 * @brief Compile default values of a leaf-list expecting a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] llist Leaf-list that the default value(s) are for.
 * @param[in] dflt Default value to compile, in case of a single value.
 * @param[in] dflts Sized array of default values, in case of more values.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_llist_dflts(struct lysc_ctx *ctx, struct lysc_node_leaflist *llist, struct lysp_qname *dflt,
        struct lysp_qname *dflts)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE orig_count, u, v;

    assert(dflt || dflts);

    if (llist->dflts) {
        /* there were already some defaults and we are adding new by deviations */
        assert(dflts);
        orig_count = LY_ARRAY_COUNT(llist->dflts);
    } else {
        orig_count = 0;
    }

    /* allocate new items */
    if (dflts) {
        LY_ARRAY_CREATE_RET(ctx->ctx, llist->dflts, orig_count + LY_ARRAY_COUNT(dflts), LY_EMEM);
    } else {
        LY_ARRAY_CREATE_RET(ctx->ctx, llist->dflts, orig_count + 1, LY_EMEM);
    }

    /* fill each new default value */
    if (dflts) {
        LY_ARRAY_FOR(dflts, u) {
            llist->dflts[orig_count + u] = calloc(1, sizeof **llist->dflts);
            ret = lys_compile_unres_dflt(ctx, (struct lysc_node *)llist, llist->type, dflts[u].str, dflts[u].mod,
                    llist->dflts[orig_count + u]);
            LY_CHECK_ERR_RET(ret, free(llist->dflts[orig_count + u]), ret);
            LY_ARRAY_INCREMENT(llist->dflts);
        }
    } else {
        llist->dflts[orig_count] = calloc(1, sizeof **llist->dflts);
        ret = lys_compile_unres_dflt(ctx, (struct lysc_node *)llist, llist->type, dflt->str, dflt->mod,
                llist->dflts[orig_count]);
        LY_CHECK_ERR_RET(ret, free(llist->dflts[orig_count]), ret);
        LY_ARRAY_INCREMENT(llist->dflts);
    }

    /* check default value uniqueness */
    if (llist->flags & LYS_CONFIG_W) {
        /* configuration data values must be unique - so check the default values */
        for (u = orig_count; u < LY_ARRAY_COUNT(llist->dflts); ++u) {
            for (v = 0; v < u; ++v) {
                if (!llist->dflts[u]->realtype->plugin->compare(llist->dflts[u], llist->dflts[v])) {
                    lysc_update_path(ctx, llist->parent, llist->name);
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                            "Configuration leaf-list has multiple defaults of the same value \"%s\".",
                            llist->dflts[u]->canonical);
                    lysc_update_path(ctx, NULL, NULL);
                    return LY_EVALID;
                }
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Finish compilation of all the unres sets of a compile context.
 *
 * @param[in] ctx Compile context with unres sets.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres(struct lysc_ctx *ctx)
{
    struct lysc_node *node;
    struct lysc_type *type, *typeiter;
    struct lysc_type_leafref *lref;
    struct lysc_augment *aug;
    struct lysc_deviation *dev;
    LY_ARRAY_COUNT_TYPE v;
    uint32_t i;

    /* for leafref, we need 2 rounds - first detects circular chain by storing the first referred type (which
     * can be also leafref, in case it is already resolved, go through the chain and check that it does not
     * point to the starting leafref type). The second round stores the first non-leafref type for later data validation. */
    for (i = 0; i < ctx->leafrefs.count; ++i) {
        node = ctx->leafrefs.objs[i];
        assert(node->nodetype & (LYS_LEAF | LYS_LEAFLIST));
        type = ((struct lysc_node_leaf *)node)->type;
        if (type->basetype == LY_TYPE_LEAFREF) {
            LY_CHECK_RET(lys_compile_unres_leafref(ctx, node, (struct lysc_type_leafref *)type));
        } else if (type->basetype == LY_TYPE_UNION) {
            LY_ARRAY_FOR(((struct lysc_type_union *)type)->types, v) {
                if (((struct lysc_type_union *)type)->types[v]->basetype == LY_TYPE_LEAFREF) {
                    lref = (struct lysc_type_leafref *)((struct lysc_type_union *)type)->types[v];
                    LY_CHECK_RET(lys_compile_unres_leafref(ctx, node, lref));
                }
            }
        }
    }
    for (i = 0; i < ctx->leafrefs.count; ++i) {
        /* store pointer to the real type */
        type = ((struct lysc_node_leaf *)ctx->leafrefs.objs[i])->type;
        if (type->basetype == LY_TYPE_LEAFREF) {
            for (typeiter = ((struct lysc_type_leafref *)type)->realtype;
                    typeiter->basetype == LY_TYPE_LEAFREF;
                    typeiter = ((struct lysc_type_leafref *)typeiter)->realtype) {}
            ((struct lysc_type_leafref *)type)->realtype = typeiter;
        } else if (type->basetype == LY_TYPE_UNION) {
            LY_ARRAY_FOR(((struct lysc_type_union *)type)->types, v) {
                if (((struct lysc_type_union *)type)->types[v]->basetype == LY_TYPE_LEAFREF) {
                    for (typeiter = ((struct lysc_type_leafref *)((struct lysc_type_union *)type)->types[v])->realtype;
                            typeiter->basetype == LY_TYPE_LEAFREF;
                            typeiter = ((struct lysc_type_leafref *)typeiter)->realtype) {}
                    ((struct lysc_type_leafref *)((struct lysc_type_union *)type)->types[v])->realtype = typeiter;
                }
            }
        }
    }

    /* check xpath */
    for (i = 0; i < ctx->xpath.count; ++i) {
        LY_CHECK_RET(lys_compile_unres_xpath(ctx, ctx->xpath.objs[i]));
    }

    /* finish incomplete default values compilation */
    for (i = 0; i < ctx->dflts.count; ++i) {
        struct lysc_unres_dflt *r = ctx->dflts.objs[i];
        if (r->leaf->nodetype == LYS_LEAF) {
            LY_CHECK_RET(lys_compile_unres_leaf_dlft(ctx, r->leaf, r->dflt));
        } else {
            LY_CHECK_RET(lys_compile_unres_llist_dflts(ctx, r->llist, r->dflt, r->dflts));
        }
    }

    /* check that all augments were applied */
    for (i = 0; i < ctx->augs.count; ++i) {
        aug = ctx->augs.objs[i];
        LOGVAL(ctx->ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE,
                "Augment target node \"%s\" from module \"%s\" was not found.", aug->nodeid->expr,
                LYSP_MODULE_NAME(aug->nodeid_pmod));
    }
    if (ctx->augs.count) {
        return LY_ENOTFOUND;
    }

    /* check that all deviations were applied */
    for (i = 0; i < ctx->devs.count; ++i) {
        dev = ctx->devs.objs[i];
        LOGVAL(ctx->ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE,
                "Deviation(s) target node \"%s\" from module \"%s\" was not found.", dev->nodeid->expr,
                LYSP_MODULE_NAME(dev->dev_pmods[0]));
    }
    if (ctx->devs.count) {
        return LY_ENOTFOUND;
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile features in the current module and all its submodules.
 *
 * @param[in] ctx Compile context.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_features(struct lysc_ctx *ctx)
{
    struct lysp_submodule *submod;
    LY_ARRAY_COUNT_TYPE u, v;

    if (!ctx->cur_mod->features) {
        /* features are compiled directly into the module structure,
         * but it must be done in two steps to allow forward references (via if-feature) between the features themselves */
        LY_CHECK_RET(lys_feature_precompile(ctx, NULL, NULL, ctx->cur_mod->parsed->features, &ctx->cur_mod->features));
        LY_ARRAY_FOR(ctx->cur_mod->parsed->includes, v) {
            submod = ctx->cur_mod->parsed->includes[v].submodule;
            LY_CHECK_RET(lys_feature_precompile(ctx, NULL, NULL, submod->features, &ctx->cur_mod->features));
        }
    }

    /* finish feature compilation, not only for the main module, but also for the submodules.
     * Due to possible forward references, it must be done when all the features (including submodules)
     * are present. */
    LY_ARRAY_FOR(ctx->cur_mod->parsed->features, u) {
        LY_CHECK_RET(lys_feature_precompile_finish(ctx, &ctx->cur_mod->parsed->features[u], ctx->cur_mod->features));
    }

    lysc_update_path(ctx, NULL, "{submodule}");
    LY_ARRAY_FOR(ctx->cur_mod->parsed->includes, v) {
        submod = ctx->cur_mod->parsed->includes[v].submodule;

        lysc_update_path(ctx, NULL, submod->name);
        LY_ARRAY_FOR(submod->features, u) {
            LY_CHECK_RET(lys_feature_precompile_finish(ctx, &submod->features[u], ctx->cur_mod->features));
        }
        lysc_update_path(ctx, NULL, NULL);
    }
    lysc_update_path(ctx, NULL, NULL);

    return LY_SUCCESS;
}

/**
 * @brief Compile identites in the current module and all its submodules.
 *
 * @param[in] ctx Compile context.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_identities(struct lysc_ctx *ctx)
{
    struct lysp_submodule *submod;
    LY_ARRAY_COUNT_TYPE u;

    if (!ctx->cur_mod->identities) {
        LY_CHECK_RET(lys_identity_precompile(ctx, NULL, NULL, ctx->cur_mod->parsed->identities, &ctx->cur_mod->identities));
        LY_ARRAY_FOR(ctx->cur_mod->parsed->includes, u) {
            submod = ctx->cur_mod->parsed->includes[u].submodule;
            LY_CHECK_RET(lys_identity_precompile(ctx, NULL, NULL, submod->identities, &ctx->cur_mod->identities));
        }
    }

    if (ctx->cur_mod->parsed->identities) {
        LY_CHECK_RET(lys_compile_identities_derived(ctx, ctx->cur_mod->parsed->identities, ctx->cur_mod->identities));
    }
    lysc_update_path(ctx, NULL, "{submodule}");
    LY_ARRAY_FOR(ctx->cur_mod->parsed->includes, u) {

        submod = ctx->cur_mod->parsed->includes[u].submodule;
        if (submod->identities) {
            lysc_update_path(ctx, NULL, submod->name);
            LY_CHECK_RET(lys_compile_identities_derived(ctx, submod->identities, ctx->cur_mod->identities));
            lysc_update_path(ctx, NULL, NULL);
        }
    }
    lysc_update_path(ctx, NULL, NULL);

    return LY_SUCCESS;
}

LY_ERR
lys_compile(struct lys_module *mod, uint32_t options)
{
    struct lysc_ctx ctx = {0};
    struct lysc_module *mod_c;
    struct lysp_module *sp;
    struct lysp_submodule *submod;
    struct lysp_node *pnode;
    struct lysp_grp *grps;
    LY_ARRAY_COUNT_TYPE u, v;
    uint32_t i;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, mod, mod->parsed, !mod->compiled, mod->ctx, LY_EINVAL);

    if (!mod->implemented) {
        /* just imported modules are not compiled */
        return LY_SUCCESS;
    }

    /* context will be changed */
    ++mod->ctx->module_set_id;

    sp = mod->parsed;

    ctx.ctx = mod->ctx;
    ctx.cur_mod = mod;
    ctx.pmod = sp;
    ctx.options = options;
    ctx.path_len = 1;
    ctx.path[0] = '/';

    mod->compiled = mod_c = calloc(1, sizeof *mod_c);
    LY_CHECK_ERR_RET(!mod_c, LOGMEM(mod->ctx), LY_EMEM);
    mod_c->mod = mod;

    /* process imports */
    LY_ARRAY_FOR(sp->imports, u) {
        LY_CHECK_GOTO(ret = lys_compile_import(&ctx, &sp->imports[u]), error);
    }

    /* features */
    LY_CHECK_GOTO(ret = lys_compile_features(&ctx), error);

    /* identities, work similarly to features with the precompilation */
    LY_CHECK_GOTO(ret = lys_compile_identities(&ctx), error);

    /* augments and deviations */
    LY_CHECK_GOTO(ret = lys_precompile_augments_deviations(&ctx), error);

    /* compile augments and deviations of our module from other modules so they can be applied during compilation */
    LY_CHECK_GOTO(ret = lys_precompile_own_augments(&ctx), error);
    LY_CHECK_GOTO(ret = lys_precompile_own_deviations(&ctx), error);

    /* data nodes */
    LY_LIST_FOR(sp->data, pnode) {
        LY_CHECK_GOTO(ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL), error);
    }

    /* top-level RPCs and notifications */
    COMPILE_OP_ARRAY_GOTO(&ctx, sp->rpcs, mod_c->rpcs, NULL, u, lys_compile_action, 0, ret, error);
    COMPILE_OP_ARRAY_GOTO(&ctx, sp->notifs, mod_c->notifs, NULL, u, lys_compile_notif, 0, ret, error);

    /* extension instances */
    COMPILE_EXTS_GOTO(&ctx, sp->exts, mod_c->exts, mod_c, LYEXT_PAR_MODULE, ret, error);

    /* the same for submodules */
    LY_ARRAY_FOR(sp->includes, u) {
        submod = sp->includes[u].submodule;
        ctx.pmod = (struct lysp_module *)submod;

        LY_LIST_FOR(submod->data, pnode) {
            ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL);
            LY_CHECK_GOTO(ret, error);
        }

        COMPILE_OP_ARRAY_GOTO(&ctx, submod->rpcs, mod_c->rpcs, NULL, v, lys_compile_action, 0, ret, error);
        COMPILE_OP_ARRAY_GOTO(&ctx, submod->notifs, mod_c->notifs, NULL, v, lys_compile_notif, 0, ret, error);

        COMPILE_EXTS_GOTO(&ctx, submod->exts, mod_c->exts, mod_c, LYEXT_PAR_MODULE, ret, error);
    }

    /* finish compilation for all unresolved items in the context */
    LY_CHECK_GOTO(ret = lys_compile_unres(&ctx), error);

    /* validate non-instantiated groupings from the parsed schema,
     * without it we would accept even the schemas with invalid grouping specification */
    ctx.pmod = sp;
    ctx.options |= LYS_COMPILE_GROUPING;
    LY_ARRAY_FOR(sp->groupings, u) {
        if (!(sp->groupings[u].flags & LYS_USED_GRP)) {
            LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, NULL, &sp->groupings[u]), error);
        }
    }
    LY_LIST_FOR(sp->data, pnode) {
        grps = (struct lysp_grp *)lysp_node_groupings(pnode);
        LY_ARRAY_FOR(grps, u) {
            if (!(grps[u].flags & LYS_USED_GRP)) {
                LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, pnode, &grps[u]), error);
            }
        }
    }
    LY_ARRAY_FOR(sp->includes, u) {
        submod = sp->includes[u].submodule;
        ctx.pmod = (struct lysp_module *)submod;

        LY_ARRAY_FOR(submod->groupings, u) {
            if (!(submod->groupings[u].flags & LYS_USED_GRP)) {
                LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, NULL, &submod->groupings[u]), error);
            }
        }
        LY_LIST_FOR(submod->data, pnode) {
            grps = (struct lysp_grp *)lysp_node_groupings(pnode);
            LY_ARRAY_FOR(grps, u) {
                if (!(grps[u].flags & LYS_USED_GRP)) {
                    LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, pnode, &grps[u]), error);
                }
            }
        }
    }
    ctx.pmod = sp;

#if 0
    /* hack for NETCONF's edit-config's operation attribute. It is not defined in the schema, but since libyang
     * implements YANG metadata (annotations), we need its definition. Because the ietf-netconf schema is not the
     * internal part of libyang, we cannot add the annotation into the schema source, but we do it here to have
     * the anotation definitions available in the internal schema structure. */
    if (ly_strequal(mod->name, "ietf-netconf", 0)) {
        if (lyp_add_ietf_netconf_annotations(mod)) {
            lys_free(mod, NULL, 1, 1);
            return NULL;
        }
    }
#endif

    /* add ietf-netconf-with-defaults "default" metadata to the compiled module */
    if (!strcmp(mod->name, "ietf-netconf-with-defaults")) {
        LY_CHECK_GOTO(ret = lys_compile_ietf_netconf_wd_annotation(&ctx, mod), error);
    }

    /* there can be no leftover deviations */
    LY_CHECK_ERR_GOTO(ctx.devs.count, LOGINT(ctx.ctx); ret = LY_EINT, error);

    for (i = 0; i < ctx.dflts.count; ++i) {
        lysc_unres_dflt_free(ctx.ctx, ctx.dflts.objs[i]);
    }
    ly_set_erase(&ctx.dflts, NULL);
    ly_set_erase(&ctx.xpath, NULL);
    ly_set_erase(&ctx.leafrefs, NULL);
    ly_set_erase(&ctx.groupings, NULL);
    ly_set_erase(&ctx.tpdf_chain, NULL);
    ly_set_erase(&ctx.augs, NULL);
    ly_set_erase(&ctx.devs, NULL);
    ly_set_erase(&ctx.uses_augs, NULL);
    ly_set_erase(&ctx.uses_rfns, NULL);

    return LY_SUCCESS;

error:
    lys_precompile_augments_deviations_revert(ctx.ctx, mod);
    lys_feature_precompile_revert(&ctx, mod);
    for (i = 0; i < ctx.dflts.count; ++i) {
        lysc_unres_dflt_free(ctx.ctx, ctx.dflts.objs[i]);
    }
    ly_set_erase(&ctx.dflts, NULL);
    ly_set_erase(&ctx.xpath, NULL);
    ly_set_erase(&ctx.leafrefs, NULL);
    ly_set_erase(&ctx.groupings, NULL);
    ly_set_erase(&ctx.tpdf_chain, NULL);
    for (i = 0; i < ctx.augs.count; ++i) {
        lysc_augment_free(ctx.ctx, ctx.augs.objs[i]);
    }
    ly_set_erase(&ctx.augs, NULL);
    for (i = 0; i < ctx.devs.count; ++i) {
        lysc_deviation_free(ctx.ctx, ctx.devs.objs[i]);
    }
    ly_set_erase(&ctx.devs, NULL);
    for (i = 0; i < ctx.uses_augs.count; ++i) {
        lysc_augment_free(ctx.ctx, ctx.uses_augs.objs[i]);
    }
    ly_set_erase(&ctx.uses_augs, NULL);
    for (i = 0; i < ctx.uses_rfns.count; ++i) {
        lysc_refine_free(ctx.ctx, ctx.uses_rfns.objs[i]);
    }
    ly_set_erase(&ctx.uses_rfns, NULL);
    lysc_module_free(mod_c, NULL);
    mod->compiled = NULL;

    return ret;
}
