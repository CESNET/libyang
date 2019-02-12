/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema tree implementation
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <ctype.h>
#include <stdio.h>

#include "libyang.h"
#include "context.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**
 * @brief Duplicate string into dictionary
 * @param[in] CTX libyang context of the dictionary.
 * @param[in] ORIG String to duplicate.
 * @param[out] DUP Where to store the result.
 */
#define DUP_STRING(CTX, ORIG, DUP) if (ORIG) {DUP = lydict_insert(CTX, ORIG, 0);}

#define COMPILE_ARRAY_GOTO(CTX, ARRAY_P, ARRAY_C, OPTIONS, ITER, FUNC, RET, GOTO) \
    if (ARRAY_P) { \
        LY_ARRAY_CREATE_GOTO((CTX)->ctx, ARRAY_C, LY_ARRAY_SIZE(ARRAY_P), RET, GOTO); \
        size_t __array_offset = LY_ARRAY_SIZE(ARRAY_C); \
        for (ITER = 0; ITER < LY_ARRAY_SIZE(ARRAY_P); ++ITER) { \
            LY_ARRAY_INCREMENT(ARRAY_C); \
            RET = FUNC(CTX, &(ARRAY_P)[ITER], OPTIONS, &(ARRAY_C)[ITER + __array_offset]); \
            LY_CHECK_GOTO(RET != LY_SUCCESS, GOTO); \
        } \
    }

#define COMPILE_ARRAY_UNIQUE_GOTO(CTX, ARRAY_P, ARRAY_C, OPTIONS, ITER, FUNC, RET, GOTO) \
    if (ARRAY_P) { \
        LY_ARRAY_CREATE_GOTO((CTX)->ctx, ARRAY_C, LY_ARRAY_SIZE(ARRAY_P), RET, GOTO); \
        size_t __array_offset = LY_ARRAY_SIZE(ARRAY_C); \
        for (ITER = 0; ITER < LY_ARRAY_SIZE(ARRAY_P); ++ITER) { \
            LY_ARRAY_INCREMENT(ARRAY_C); \
            RET = FUNC(CTX, &(ARRAY_P)[ITER], OPTIONS, ARRAY_C, &(ARRAY_C)[ITER + __array_offset]); \
            LY_CHECK_GOTO(RET != LY_SUCCESS, GOTO); \
        } \
    }

#define COMPILE_MEMBER_GOTO(CTX, MEMBER_P, MEMBER_C, OPTIONS, FUNC, RET, GOTO) \
    if (MEMBER_P) { \
        MEMBER_C = calloc(1, sizeof *(MEMBER_C)); \
        LY_CHECK_ERR_GOTO(!(MEMBER_C), LOGMEM((CTX)->ctx); RET = LY_EMEM, GOTO); \
        RET = FUNC(CTX, MEMBER_P, OPTIONS, MEMBER_C); \
        LY_CHECK_GOTO(RET != LY_SUCCESS, GOTO); \
    }

#define COMPILE_MEMBER_ARRAY_GOTO(CTX, MEMBER_P, ARRAY_C, OPTIONS, FUNC, RET, GOTO) \
    if (MEMBER_P) { \
        LY_ARRAY_CREATE_GOTO((CTX)->ctx, ARRAY_C, 1, RET, GOTO); \
        size_t __array_offset = LY_ARRAY_SIZE(ARRAY_C); \
        LY_ARRAY_INCREMENT(ARRAY_C); \
        RET = FUNC(CTX, MEMBER_P, OPTIONS, &(ARRAY_C)[__array_offset]); \
        LY_CHECK_GOTO(RET != LY_SUCCESS, GOTO); \
    }

#define COMPILE_CHECK_UNIQUENESS(CTX, ARRAY, MEMBER, EXCL, STMT, IDENT) \
    if (ARRAY) { \
        for (unsigned int u__ = 0; u__ < LY_ARRAY_SIZE(ARRAY); ++u__) { \
            if (&(ARRAY)[u__] != EXCL && (void*)((ARRAY)[u__].MEMBER) == (void*)(IDENT)) { \
                LOGVAL((CTX)->ctx, LY_VLOG_STR, (CTX)->path, LY_VCODE_DUPIDENT, IDENT, STMT); \
                return LY_EVALID; \
            } \
        } \
    }

static struct lysc_ext_instance *
lysc_ext_instance_dup(struct ly_ctx *ctx, struct lysc_ext_instance *orig)
{
    /* TODO */
    (void) ctx;
    (void) orig;
    return NULL;
}

static struct lysc_pattern*
lysc_pattern_dup(struct lysc_pattern *orig)
{
    ++orig->refcount;
    return orig;
}

static struct lysc_pattern**
lysc_patterns_dup(struct ly_ctx *ctx, struct lysc_pattern **orig)
{
    struct lysc_pattern **dup = NULL;
    unsigned int u;

    LY_ARRAY_CREATE_RET(ctx, dup, LY_ARRAY_SIZE(orig), NULL);
    LY_ARRAY_FOR(orig, u) {
        dup[u] = lysc_pattern_dup(orig[u]);
        LY_ARRAY_INCREMENT(dup);
    }
    return dup;
}

struct lysc_range*
lysc_range_dup(struct ly_ctx *ctx, const struct lysc_range *orig)
{
    struct lysc_range *dup;
    LY_ERR ret;

    dup = calloc(1, sizeof *dup);
    LY_CHECK_ERR_RET(!dup, LOGMEM(ctx), NULL);
    if (orig->parts) {
        LY_ARRAY_CREATE_GOTO(ctx, dup->parts, LY_ARRAY_SIZE(orig->parts), ret, cleanup);
        LY_ARRAY_SIZE(dup->parts) = LY_ARRAY_SIZE(orig->parts);
        memcpy(dup->parts, orig->parts, LY_ARRAY_SIZE(dup->parts) * sizeof *dup->parts);
    }
    DUP_STRING(ctx, orig->eapptag, dup->eapptag);
    DUP_STRING(ctx, orig->emsg, dup->emsg);
    dup->exts = lysc_ext_instance_dup(ctx, orig->exts);

    return dup;
cleanup:
    free(dup);
    (void) ret; /* set but not used due to the return type */
    return NULL;
}

struct iff_stack {
    int size;
    int index;     /* first empty item */
    uint8_t *stack;
};

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

static uint8_t
iff_stack_pop(struct iff_stack *stack)
{
    stack->index--;
    return stack->stack[stack->index];
}

static void
iff_stack_clean(struct iff_stack *stack)
{
    stack->size = 0;
    free(stack->stack);
}

static void
iff_setop(uint8_t *list, uint8_t op, int pos)
{
    uint8_t *item;
    uint8_t mask = 3;

    assert(pos >= 0);
    assert(op <= 3); /* max 2 bits */

    item = &list[pos / 4];
    mask = mask << 2 * (pos % 4);
    *item = (*item) & ~mask;
    *item = (*item) | (op << 2 * (pos % 4));
}

#define LYS_IFF_LP 0x04 /* ( */
#define LYS_IFF_RP 0x08 /* ) */

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
 * @param[in] mod Module where the feature was referenced (used to resolve prefix of the feature).
 * @param[in] name Name of the feature including possible prefix.
 * @param[in] len Length of the string representing the feature identifier in the name variable (mandatory!).
 * @return Pointer to the feature structure if found, NULL otherwise.
 */
static struct lysc_feature *
lys_feature_find(struct lys_module *mod, const char *name, size_t len)
{
    size_t i;
    struct lysc_feature *f, *flist;

    for (i = 0; i < len; ++i) {
        if (name[i] == ':') {
            /* we have a prefixed feature */
            mod = lys_module_find_prefix(mod, name, i);
            LY_CHECK_RET(!mod, NULL);

            name = &name[i + 1];
            len = len - i - 1;
        }
    }

    /* we have the correct module, get the feature */
    if (mod->implemented) {
        /* module is implemented so there is already the compiled schema */
        flist = mod->compiled->features;
    } else {
        flist = mod->off_features;
    }
    LY_ARRAY_FOR(flist, i) {
        f = &flist[i];
        if (!strncmp(f->name, name, len) && f->name[len] == '\0') {
            return f;
        }
    }

    return NULL;
}

static LY_ERR
lys_compile_ext(struct lysc_ctx *ctx, struct lysp_ext_instance *ext_p, int UNUSED(options), struct lysc_ext_instance *ext)
{
    const char *name;
    unsigned int u;
    const struct lys_module *mod;
    struct lysp_ext *edef = NULL;

    DUP_STRING(ctx->ctx, ext_p->argument, ext->argument);
    ext->insubstmt = ext_p->insubstmt;
    ext->insubstmt_index = ext_p->insubstmt_index;

    /* get module where the extension definition should be placed */
    for (u = 0; ext_p->name[u] != ':'; ++u);
    mod = lys_module_find_prefix(ctx->mod_def, ext_p->name, u);
    LY_CHECK_ERR_RET(!mod, LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                                  "Invalid prefix \"%.*s\" used for extension instance identifier.", u, ext_p->name),
                     LY_EVALID);
    LY_CHECK_ERR_RET(!mod->parsed->extensions,
                     LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                            "Extension instance \"%s\" refers \"%s\" module that does not contain extension definitions.",
                            ext_p->name, mod->name),
                     LY_EVALID);
    name = &ext_p->name[u + 1];
    /* find the extension definition there */
    for (ext = NULL, u = 0; u < LY_ARRAY_SIZE(mod->parsed->extensions); ++u) {
        if (!strcmp(name, mod->parsed->extensions[u].name)) {
            edef = &mod->parsed->extensions[u];
            break;
        }
    }
    LY_CHECK_ERR_RET(!edef, LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                                   "Extension definition of extension instance \"%s\" not found.", ext_p->name),
                     LY_EVALID);
    /* TODO plugins */

    return LY_SUCCESS;
}

static LY_ERR
lys_compile_iffeature(struct lysc_ctx *ctx, const char **value, int UNUSED(options), struct lysc_iffeature *iff)
{
    const char *c = *value;
    int r, rc = EXIT_FAILURE;
    int i, j, last_not, checkversion = 0;
    unsigned int f_size = 0, expr_size = 0, f_exp = 1;
    uint8_t op;
    struct iff_stack stack = {0, 0, NULL};
    struct lysc_feature *f;

    assert(c);

    /* pre-parse the expression to get sizes for arrays, also do some syntax checks of the expression */
    for (i = j = last_not = 0; c[i]; i++) {
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

        if (!strncmp(&c[i], "not", r = 3) || !strncmp(&c[i], "and", r = 3) || !strncmp(&c[i], "or", r = 2)) {
            if (c[i + r] == '\0') {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid value \"%s\" of if-feature - unexpected end of expression.", *value);
                return LY_EVALID;
            } else if (!isspace(c[i + r])) {
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
                f_exp++;
                /* not a not operation */
                last_not = 0;
            }
            i += r;
        } else {
            f_size++;
            last_not = 0;
        }
        expr_size++;

        while (!isspace(c[i])) {
            if (!c[i] || c[i] == ')') {
                i--;
                break;
            }
            i++;
        }
    }
    if (j || f_exp != f_size) {
        /* not matching count of ( and ) */
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
               "Invalid value \"%s\" of if-feature - non-matching opening and closing parentheses.", *value);
        return LY_EVALID;
    }

    if (checkversion || expr_size > 1) {
        /* check that we have 1.1 module */
        if (ctx->mod_def->version != LYS_VERSION_1_1) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                   "Invalid value \"%s\" of if-feature - YANG 1.1 expression in YANG 1.0 module.", *value);
            return LY_EVALID;
        }
    }

    /* allocate the memory */
    LY_ARRAY_CREATE_RET(ctx->ctx, iff->features, f_size, LY_EMEM);
    iff->expr = calloc((j = (expr_size / 4) + ((expr_size % 4) ? 1 : 0)), sizeof *iff->expr);
    stack.stack = malloc(expr_size * sizeof *stack.stack);
    LY_CHECK_ERR_GOTO(!stack.stack || !iff->expr, LOGMEM(ctx->ctx), error);

    stack.size = expr_size;
    f_size--; expr_size--; /* used as indexes from now */

    for (i--; i >= 0; i--) {
        if (c[i] == ')') {
            /* push it on stack */
            iff_stack_push(&stack, LYS_IFF_RP);
            continue;
        } else if (c[i] == '(') {
            /* pop from the stack into result all operators until ) */
            while((op = iff_stack_pop(&stack)) != LYS_IFF_RP) {
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
            if (stack.index && stack.stack[stack.index - 1] == LYS_IFF_NOT) {
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
            f = lys_feature_find(ctx->mod_def, &c[i], j - i);
            LY_CHECK_ERR_GOTO(!f, LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                                         "Invalid value \"%s\" of if-feature - unable to find feature \"%.*s\".", *value, j - i, &c[i]);
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
               "Invalid value \"%s\" of if-feature - processing error.", *value);
        rc = LY_EINT;
    } else {
        rc = LY_SUCCESS;
    }

error:
    /* cleanup */
    iff_stack_clean(&stack);

    return rc;
}

static struct lysc_node *
lys_compile_xpath_context(struct lysc_node *start)
{
    for (; start && !(start->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_ACTION | LYS_NOTIF));
            start = start->parent);
    return start;
}

static LY_ERR
lys_compile_when(struct lysc_ctx *ctx, struct lysp_when *when_p, int options, struct lysc_when **when)
{
    unsigned int u;
    LY_ERR ret = LY_SUCCESS;

    *when = calloc(1, sizeof **when);
    (*when)->refcount = 1;
    (*when)->cond = lyxp_expr_parse(ctx->ctx, when_p->cond);
    DUP_STRING(ctx->ctx, when_p->dsc, (*when)->dsc);
    DUP_STRING(ctx->ctx, when_p->ref, (*when)->ref);
    LY_CHECK_ERR_GOTO(!(*when)->cond, ret = ly_errcode(ctx->ctx), done);
    COMPILE_ARRAY_GOTO(ctx, when_p->exts, (*when)->exts, options, u, lys_compile_ext, ret, done);

done:
    return ret;
}

static LY_ERR
lys_compile_must(struct lysc_ctx *ctx, struct lysp_restr *must_p, int options, struct lysc_must *must)
{
    unsigned int u;
    LY_ERR ret = LY_SUCCESS;

    must->cond = lyxp_expr_parse(ctx->ctx, must_p->arg);
    LY_CHECK_ERR_GOTO(!must->cond, ret = ly_errcode(ctx->ctx), done);

    DUP_STRING(ctx->ctx, must_p->eapptag, must->eapptag);
    DUP_STRING(ctx->ctx, must_p->emsg, must->emsg);
    DUP_STRING(ctx->ctx, must_p->dsc, must->dsc);
    DUP_STRING(ctx->ctx, must_p->ref, must->ref);
    COMPILE_ARRAY_GOTO(ctx, must_p->exts, must->exts, options, u, lys_compile_ext, ret, done);

done:
    return ret;
}

static LY_ERR
lys_compile_import(struct lysc_ctx *ctx, struct lysp_import *imp_p, int options, struct lysc_import *imp)
{
    unsigned int u;
    struct lys_module *mod = NULL;
    LY_ERR ret = LY_SUCCESS;

    DUP_STRING(ctx->ctx, imp_p->prefix, imp->prefix);
    COMPILE_ARRAY_GOTO(ctx, imp_p->exts, imp->exts, options, u, lys_compile_ext, ret, done);
    imp->module = imp_p->module;

    /* make sure that we have the parsed version (lysp_) of the imported module to import groupings or typedefs.
     * The compiled version is needed only for augments, deviates and leafrefs, so they are checked (and added,
     * if needed) when these nodes are finally being instantiated and validated at the end of schema compilation. */
    if (!imp->module->parsed) {
        /* try to use filepath if present */
        if (imp->module->filepath) {
            mod = (struct lys_module*)lys_parse_path(ctx->ctx, imp->module->filepath,
                                 !strcmp(&imp->module->filepath[strlen(imp->module->filepath - 4)], ".yin") ? LYS_IN_YIN : LYS_IN_YANG);
            if (mod != imp->module) {
                LOGERR(ctx->ctx, LY_EINT, "Filepath \"%s\" of the module \"%s\" does not match.",
                       imp->module->filepath, imp->module->name);
                mod = NULL;
            }
        }
        if (!mod) {
            if (lysp_load_module(ctx->ctx, imp->module->name, imp->module->revision, 0, 1, &mod)) {
                LOGERR(ctx->ctx, LY_ENOTFOUND, "Unable to reload \"%s\" module to import it into \"%s\", source data not found.",
                       imp->module->name, ctx->mod->name);
                return LY_ENOTFOUND;
            }
        }
    }

done:
    return ret;
}

static LY_ERR
lys_compile_identity(struct lysc_ctx *ctx, struct lysp_ident *ident_p, int options, struct lysc_ident *idents, struct lysc_ident *ident)
{
    unsigned int u;
    LY_ERR ret = LY_SUCCESS;

    COMPILE_CHECK_UNIQUENESS(ctx, idents, name, ident, "identity", ident_p->name);
    DUP_STRING(ctx->ctx, ident_p->name, ident->name);
    DUP_STRING(ctx->ctx, ident_p->ref, ident->dsc);
    DUP_STRING(ctx->ctx, ident_p->ref, ident->dsc);
    COMPILE_ARRAY_GOTO(ctx, ident_p->iffeatures, ident->iffeatures, options, u, lys_compile_iffeature, ret, done);
    /* backlings (derived) can be added no sooner than when all the identities in the current module are present */
    COMPILE_ARRAY_GOTO(ctx, ident_p->exts, ident->exts, options, u, lys_compile_ext, ret, done);
    ident->flags = ident_p->flags;

done:
    return ret;
}

/**
 * @brief Find and process the referenced base identities from another identity or identityref
 *
 * For bases in identity se backlinks to them from the base identities. For identityref, store
 * the array of pointers to the base identities. So one of the ident or bases parameter must be set
 * to distinguish these two use cases.
 *
 * @param[in] ctx Compile context, not only for logging but also to get the current module to resolve prefixes.
 * @param[in] bases_p Array of names (including prefix if necessary) of base identities.
 * @param[in] ident Referencing identity to work with.
 * @param[in] bases Array of bases of identityref to fill in.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_identity_bases(struct lysc_ctx *ctx, const char **bases_p,  struct lysc_ident *ident, struct lysc_ident ***bases)
{
    unsigned int u, v;
    const char *s, *name;
    struct lys_module *mod;
    struct lysc_ident **idref;

    assert(ident || bases);

    if (LY_ARRAY_SIZE(bases_p) > 1 && ctx->mod_def->version < 2) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
               "Multiple bases in %s are allowed only in YANG 1.1 modules.", ident ? "identity" : "identityref type");
        return LY_EVALID;
    }

    for (u = 0; u < LY_ARRAY_SIZE(bases_p); ++u) {
        s = strchr(bases_p[u], ':');
        if (s) {
            /* prefixed identity */
            name = &s[1];
            mod = lys_module_find_prefix(ctx->mod_def, bases_p[u], s - bases_p[u]);
        } else {
            name = bases_p[u];
            mod = ctx->mod_def;
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
        if (mod->compiled && mod->compiled->identities) {
            for (v = 0; v < LY_ARRAY_SIZE(mod->compiled->identities); ++v) {
                if (!strcmp(name, mod->compiled->identities[v].name)) {
                    if (ident) {
                        /* we have match! store the backlink */
                        LY_ARRAY_NEW_RET(ctx->ctx, mod->compiled->identities[v].derived, idref, LY_EMEM);
                        *idref = ident;
                    } else {
                        /* we have match! store the found identity */
                        LY_ARRAY_NEW_RET(ctx->ctx, *bases, idref, LY_EMEM);
                        *idref = &mod->compiled->identities[v];
                    }
                    break;
                }
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
    unsigned int i;

    for (i = 0; i < LY_ARRAY_SIZE(idents_p); ++i) {
        if (!idents_p[i].bases) {
            continue;
        }
        LY_CHECK_RET(lys_compile_identity_bases(ctx, idents_p[i].bases, &idents[i], NULL));
    }
    return LY_SUCCESS;
}

LY_ERR
lys_feature_precompile(struct ly_ctx *ctx, struct lysp_feature *features_p, struct lysc_feature **features)
{
    unsigned int offset = 0, u;
    struct lysc_ctx context = {0};

    assert(ctx);
    context.ctx = ctx;

    if (!features_p) {
        return LY_SUCCESS;
    }
    if (*features) {
        offset = LY_ARRAY_SIZE(*features);
    }

    LY_ARRAY_CREATE_RET(ctx, *features, LY_ARRAY_SIZE(features_p), LY_EMEM);
    LY_ARRAY_FOR(features_p, u) {
        LY_ARRAY_INCREMENT(*features);
        COMPILE_CHECK_UNIQUENESS(&context, *features, name, &(*features)[offset + u], "feature", features_p[u].name);
        DUP_STRING(ctx, features_p[u].name, (*features)[offset + u].name);
        DUP_STRING(ctx, features_p[u].dsc, (*features)[offset + u].dsc);
        DUP_STRING(ctx, features_p[u].ref, (*features)[offset + u].ref);
        (*features)[offset + u].flags = features_p[u].flags;
    }

    return LY_SUCCESS;
}

/**
 * @brief Create pre-compiled features array.
 *
 * See lys_feature_precompile() for more details.
 *
 * @param[in] ctx Compile context.
 * @param[in] feature_p Parsed feature definition to compile.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in,out] features List of already (pre)compiled features to find the corresponding precompiled feature structure.
 * @return LY_ERR value.
 */
static LY_ERR
lys_feature_precompile_finish(struct lysc_ctx *ctx, struct lysp_feature *feature_p, int options, struct lysc_feature *features)
{
    unsigned int u, v, x;
    struct lysc_feature *feature, **df;
    LY_ERR ret = LY_SUCCESS;

    /* find the preprecompiled feature */
    LY_ARRAY_FOR(features, x) {
        if (strcmp(features[x].name, feature_p->name)) {
            continue;
        }
        feature = &features[x];

        /* finish compilation started in lys_feature_precompile() */
        COMPILE_ARRAY_GOTO(ctx, feature_p->exts, feature->exts, options, u, lys_compile_ext, ret, done);
        COMPILE_ARRAY_GOTO(ctx, feature_p->iffeatures, feature->iffeatures, options, u, lys_compile_iffeature, ret, done);
        if (feature->iffeatures) {
            for (u = 0; u < LY_ARRAY_SIZE(feature->iffeatures); ++u) {
                if (feature->iffeatures[u].features) {
                    for (v = 0; v < LY_ARRAY_SIZE(feature->iffeatures[u].features); ++v) {
                        /* add itself into the dependants list */
                        LY_ARRAY_NEW_RET(ctx->ctx, feature->iffeatures[u].features[v]->depfeatures, df, LY_EMEM);
                        *df = feature;
                    }
                    /* TODO check for circular dependency */
                }
            }
        }
    done:
        return ret;
    }

    LOGINT(ctx->ctx);
    return LY_EINT;
}

static void
lys_feature_precompile_revert(struct lysc_ctx *ctx, struct lys_module *mod)
{
    unsigned int u, v;

    /* keep the off_features list until the complete lys_module is freed */
    mod->off_features = mod->compiled->features;
    mod->compiled->features = NULL;

    /* in the off_features list, remove all the parts (from finished compiling process)
     * which may points into the data being freed here */
    LY_ARRAY_FOR(mod->off_features, u) {
        LY_ARRAY_FOR(mod->off_features[u].iffeatures, v) {
            lysc_iffeature_free(ctx->ctx, &mod->off_features[u].iffeatures[v]);
        }
        LY_ARRAY_FREE(mod->off_features[u].iffeatures);
        mod->off_features[u].iffeatures = NULL;

        LY_ARRAY_FOR(mod->off_features[u].exts, v) {
            lysc_ext_instance_free(ctx->ctx, &(mod->off_features[u].exts)[v]);
        }
        LY_ARRAY_FREE(mod->off_features[u].exts);
        mod->off_features[u].exts = NULL;
    }
}

/**
 * @brief Validate and normalize numeric value from a range definition.
 * @param[in] ctx Compile context.
 * @param[in] basetype Base YANG built-in type of the node connected with the range restriction. Actually only LY_TYPE_DEC64 is important to
 * allow processing of the fractions. The fraction point is extracted from the value which is then normalize according to given frdigits into
 * valcopy to allow easy parsing and storing of the value. libyang stores decimal number without the decimal point which is always recovered from
 * the known fraction-digits value. So, with fraction-digits 2, number 3.14 is stored as 314 and number 1 is stored as 100.
 * @param[in] frdigits The fraction-digits of the type in case of LY_TYPE_DEC64.
 * @param[in] value String value of the range boundary.
 * @param[out] len Number of the processed bytes from the value. Processing stops on the first character which is not part of the number boundary.
 * @param[out] valcopy NULL-terminated string with the numeric value to parse and store.
 * @return LY_ERR value - LY_SUCCESS, LY_EMEM, LY_EVALID (no number) or LY_EINVAL (decimal64 not matching fraction-digits value).
 */
static LY_ERR
range_part_check_value_syntax(struct lysc_ctx *ctx, LY_DATA_TYPE basetype, uint8_t frdigits, const char *value, size_t *len, char **valcopy)
{
    size_t fraction = 0, size;

    *len = 0;

    assert(value);
    /* parse value */
    if (!isdigit(value[*len]) && (value[*len] != '-') && (value[*len] != '+')) {
        return LY_EVALID;
    }

    if ((value[*len] == '-') || (value[*len] == '+')) {
        ++(*len);
    }

    while (isdigit(value[*len])) {
        ++(*len);
    }

    if ((basetype != LY_TYPE_DEC64) || (value[*len] != '.') || !isdigit(value[*len + 1])) {
        if (basetype == LY_TYPE_DEC64) {
            goto decimal;
        } else {
            *valcopy = strndup(value, *len);
            return LY_SUCCESS;
        }
    }
    fraction = *len;

    ++(*len);
    while (isdigit(value[*len])) {
        ++(*len);
    }

    if (basetype == LY_TYPE_DEC64) {
decimal:
        assert(frdigits);
        if (*len - 1 - fraction > frdigits) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                   "Range boundary \"%.*s\" of decimal64 type exceeds defined number (%u) of fraction digits.",
                   *len, value, frdigits);
            return LY_EINVAL;
        }
        if (fraction) {
            size = (*len) + (frdigits - ((*len) - 1 - fraction));
        } else {
            size = (*len) + frdigits + 1;
        }
        *valcopy = malloc(size * sizeof **valcopy);
        LY_CHECK_ERR_RET(!(*valcopy), LOGMEM(ctx->ctx), LY_EMEM);

        (*valcopy)[size - 1] = '\0';
        if (fraction) {
            memcpy(&(*valcopy)[0], &value[0], fraction);
            memcpy(&(*valcopy)[fraction], &value[fraction + 1], (*len) - 1 - (fraction));
            memset(&(*valcopy)[(*len) - 1], '0', frdigits - ((*len) - 1 - fraction));
        } else {
            memcpy(&(*valcopy)[0], &value[0], *len);
            memset(&(*valcopy)[*len], '0', frdigits);
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Check that values in range are in ascendant order.
 * @param[in] unsigned_value Flag to note that we are working with unsigned values.
 * @param[in] max Flag to distinguish if checking min or max value. min value must be strictly higher than previous,
 * max can be also equal.
 * @param[in] value Current value to check.
 * @param[in] prev_value The last seen value.
 * @return LY_SUCCESS or LY_EEXIST for invalid order.
 */
static LY_ERR
range_part_check_ascendancy(int unsigned_value, int max, int64_t value, int64_t prev_value)
{
    if (unsigned_value) {
        if ((max && (uint64_t)prev_value > (uint64_t)value) || (!max && (uint64_t)prev_value >= (uint64_t)value)) {
            return LY_EEXIST;
        }
    } else {
        if ((max && prev_value > value) || (!max && prev_value >= value)) {
            return LY_EEXIST;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Set min/max value of the range part.
 * @param[in] ctx Compile context.
 * @param[in] part Range part structure to fill.
 * @param[in] max Flag to distinguish if storing min or max value.
 * @param[in] prev The last seen value to check that all values in range are specified in ascendant order.
 * @param[in] basetype Type of the value to get know implicit min/max values and other checking rules.
 * @param[in] first Flag for the first value of the range to avoid ascendancy order.
 * @param[in] length_restr Flag to distinguish between range and length restrictions. Only for logging.
 * @param[in] frdigits The fraction-digits value in case of LY_TYPE_DEC64 basetype.
 * @param[in] base_range Range from the type from which the current type is derived (if not built-in) to get type's min and max values.
 * @param[in,out] value Numeric range value to be stored, if not provided the type's min/max value is set.
 * @return LY_ERR value - LY_SUCCESS, LY_EDENIED (value brokes type's boundaries), LY_EVALID (not a number),
 * LY_EEXIST (value is smaller than the previous one), LY_EINVAL (decimal64 value does not corresponds with the
 * frdigits value), LY_EMEM.
 */
static LY_ERR
range_part_minmax(struct lysc_ctx *ctx, struct lysc_range_part *part, int max, int64_t prev, LY_DATA_TYPE basetype, int first, int length_restr,
                  uint8_t frdigits, struct lysc_range *base_range, const char **value)
{
    LY_ERR ret = LY_SUCCESS;
    char *valcopy = NULL;
    size_t len;

    if (value) {
        ret = range_part_check_value_syntax(ctx, basetype, frdigits, *value, &len, &valcopy);
        LY_CHECK_GOTO(ret, finalize);
    }
    if (!valcopy && base_range) {
        if (max) {
            part->max_64 = base_range->parts[LY_ARRAY_SIZE(base_range->parts) - 1].max_64;
        } else {
            part->min_64 = base_range->parts[0].min_64;
        }
        if (!first) {
            ret = range_part_check_ascendancy(basetype <= LY_TYPE_STRING ? 1 : 0, max, max ? part->max_64 : part->min_64, prev);
        }
        goto finalize;
    }

    switch (basetype) {
    case LY_TYPE_INT8: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, INT64_C(-128), INT64_C(127), 10, max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(127);
        } else {
            part->min_64 = INT64_C(-128);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_INT16: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, INT64_C(-32768), INT64_C(32767), 10, max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(32767);
        } else {
            part->min_64 = INT64_C(-32768);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_INT32: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, INT64_C(-2147483648), INT64_C(2147483647), 10, max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(2147483647);
        } else {
            part->min_64 = INT64_C(-2147483648);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_INT64: /* range */
    case LY_TYPE_DEC64: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, INT64_C(-9223372036854775807) - INT64_C(1), INT64_C(9223372036854775807), 10,
                               max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(9223372036854775807);
        } else {
            part->min_64 = INT64_C(-9223372036854775807) - INT64_C(1);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT8: /* range */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, UINT64_C(255), 10, max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(255);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT16: /* range */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, UINT64_C(65535), 10, max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(65535);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT32: /* range */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, UINT64_C(4294967295), 10, max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(4294967295);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT64: /* range */
    case LY_TYPE_STRING: /* length */
    case LY_TYPE_BINARY: /* length */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, UINT64_C(18446744073709551615), 10, max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(18446744073709551615);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    default:
        LOGINT(ctx->ctx);
        ret = LY_EINT;
    }

finalize:
    if (ret == LY_EDENIED) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
               "Invalid %s restriction - value \"%s\" does not fit the type limitations.",
               length_restr ? "length" : "range", valcopy ? valcopy : *value);
    } else if (ret == LY_EVALID) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
               "Invalid %s restriction - invalid value \"%s\".",
               length_restr ? "length" : "range", valcopy ? valcopy : *value);
    } else if (ret == LY_EEXIST) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid %s restriction - values are not in ascending order (%s).",
                       length_restr ? "length" : "range",
                       (valcopy && basetype != LY_TYPE_DEC64) ? valcopy : value ? *value : max ? "max" : "min");
    } else if (!ret && value) {
        *value = *value + len;
    }
    free(valcopy);
    return ret;
}

/**
 * @brief Compile the parsed range restriction.
 * @param[in] ctx Compile context.
 * @param[in] range_p Parsed range structure to compile.
 * @param[in] basetype Base YANG built-in type of the node with the range restriction.
 * @param[in] length_restr Flag to distinguish between range and length restrictions. Only for logging.
 * @param[in] frdigits The fraction-digits value in case of LY_TYPE_DEC64 basetype.
 * @param[in] base_range Range restriction of the type from which the current type is derived. The current
 * range restriction must be more restrictive than the base_range.
 * @param[in,out] range Pointer to the created current range structure.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_type_range(struct lysc_ctx *ctx, struct lysp_restr *range_p, LY_DATA_TYPE basetype, int length_restr, uint8_t frdigits,
                       struct lysc_range *base_range, struct lysc_range **range)
{
    LY_ERR ret = LY_EVALID;
    const char *expr;
    struct lysc_range_part *parts = NULL, *part;
    int range_expected = 0, uns;
    unsigned int parts_done = 0, u, v;

    assert(range);
    assert(range_p);

    expr = range_p->arg;
    while(1) {
        if (isspace(*expr)) {
            ++expr;
        } else if (*expr == '\0') {
            if (range_expected) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid %s restriction - unexpected end of the expression after \"..\" (%s).",
                       length_restr ? "length" : "range", range_p->arg);
                goto cleanup;
            } else if (!parts || parts_done == LY_ARRAY_SIZE(parts)) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid %s restriction - unexpected end of the expression (%s).",
                       length_restr ? "length" : "range", range_p->arg);
                goto cleanup;
            }
            parts_done++;
            break;
        } else if (!strncmp(expr, "min", 3)) {
            if (parts) {
                /* min cannot be used elsewhere than in the first part */
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid %s restriction - unexpected data before min keyword (%.*s).", length_restr ? "length" : "range",
                       expr - range_p->arg, range_p->arg);
                goto cleanup;
            }
            expr += 3;

            LY_ARRAY_NEW_GOTO(ctx->ctx, parts, part, ret, cleanup);
            LY_CHECK_GOTO(range_part_minmax(ctx, part, 0, 0, basetype, 1, length_restr, frdigits, base_range, NULL), cleanup);
            part->max_64 = part->min_64;
        } else if (*expr == '|') {
            if (!parts || range_expected) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid %s restriction - unexpected beginning of the expression (%s).", length_restr ? "length" : "range", expr);
                goto cleanup;
            }
            expr++;
            parts_done++;
            /* process next part of the expression */
        } else if (!strncmp(expr, "..", 2)) {
            expr += 2;
            while (isspace(*expr)) {
                expr++;
            }
            if (!parts || LY_ARRAY_SIZE(parts) == parts_done) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid %s restriction - unexpected \"..\" without a lower bound.", length_restr ? "length" : "range");
                goto cleanup;
            }
            /* continue expecting the upper boundary */
            range_expected = 1;
        } else if (isdigit(*expr) || (*expr == '-') || (*expr == '+')) {
            /* number */
            if (range_expected) {
                part = &parts[LY_ARRAY_SIZE(parts) - 1];
                LY_CHECK_GOTO(range_part_minmax(ctx, part, 1, part->min_64, basetype, 0, length_restr, frdigits, NULL, &expr), cleanup);
                range_expected = 0;
            } else {
                LY_ARRAY_NEW_GOTO(ctx->ctx, parts, part, ret, cleanup);
                LY_CHECK_GOTO(range_part_minmax(ctx, part, 0, parts_done ? parts[LY_ARRAY_SIZE(parts) - 2].max_64 : 0,
                                                basetype, parts_done ? 0 : 1, length_restr, frdigits, NULL, &expr), cleanup);
                part->max_64 = part->min_64;
            }

            /* continue with possible another expression part */
        } else if (!strncmp(expr, "max", 3)) {
            expr += 3;
            while (isspace(*expr)) {
                expr++;
            }
            if (*expr != '\0') {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG, "Invalid %s restriction - unexpected data after max keyword (%s).",
                       length_restr ? "length" : "range", expr);
                goto cleanup;
            }
            if (range_expected) {
                part = &parts[LY_ARRAY_SIZE(parts) - 1];
                LY_CHECK_GOTO(range_part_minmax(ctx, part, 1, part->min_64, basetype, 0, length_restr, frdigits, base_range, NULL), cleanup);
                range_expected = 0;
            } else {
                LY_ARRAY_NEW_GOTO(ctx->ctx, parts, part, ret, cleanup);
                LY_CHECK_GOTO(range_part_minmax(ctx, part, 1, parts_done ? parts[LY_ARRAY_SIZE(parts) - 2].max_64 : 0,
                                                basetype, parts_done ? 0 : 1, length_restr, frdigits, base_range, NULL), cleanup);
                part->min_64 = part->max_64;
            }
        } else {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG, "Invalid %s restriction - unexpected data (%s).",
                   length_restr ? "length" : "range", expr);
            goto cleanup;
        }
    }

    /* check with the previous range/length restriction */
    if (base_range) {
        switch (basetype) {
        case LY_TYPE_BINARY:
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_UINT64:
        case LY_TYPE_STRING:
            uns = 1;
            break;
        case LY_TYPE_DEC64:
        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
        case LY_TYPE_INT64:
            uns = 0;
            break;
        default:
            LOGINT(ctx->ctx);
            ret = LY_EINT;
            goto cleanup;
        }
        for (u = v = 0; u < parts_done && v < LY_ARRAY_SIZE(base_range->parts); ++u) {
            if ((uns && parts[u].min_u64 < base_range->parts[v].min_u64) || (!uns && parts[u].min_64 < base_range->parts[v].min_64)) {
                goto baseerror;
            }
            /* current lower bound is not lower than the base */
            if (base_range->parts[v].min_64 == base_range->parts[v].max_64) {
                /* base has single value */
                if (base_range->parts[v].min_64 == parts[u].min_64) {
                    /* both lower bounds are the same */
                    if (parts[u].min_64 != parts[u].max_64) {
                        /* current continues with a range */
                        goto baseerror;
                    } else {
                        /* equal single values, move both forward */
                        ++v;
                        continue;
                    }
                } else {
                    /* base is single value lower than current range, so the
                     * value from base range is removed in the current,
                     * move only base and repeat checking */
                    ++v;
                    --u;
                    continue;
                }
            } else {
                /* base is the range */
                if (parts[u].min_64 == parts[u].max_64) {
                    /* current is a single value */
                    if ((uns && parts[u].max_u64 > base_range->parts[v].max_u64) || (!uns && parts[u].max_64 > base_range->parts[v].max_64)) {
                        /* current is behind the base range, so base range is omitted,
                         * move the base and keep the current for further check */
                        ++v;
                        --u;
                    } /* else it is within the base range, so move the current, but keep the base */
                    continue;
                } else {
                    /* both are ranges - check the higher bound, the lower was already checked */
                    if ((uns && parts[u].max_u64 > base_range->parts[v].max_u64) || (!uns && parts[u].max_64 > base_range->parts[v].max_64)) {
                        /* higher bound is higher than the current higher bound */
                        if ((uns && parts[u].min_u64 > base_range->parts[v].max_u64) || (!uns && parts[u].min_64 > base_range->parts[v].max_64)) {
                            /* but the current lower bound is also higher, so the base range is omitted,
                             * continue with the same current, but move the base */
                            --u;
                            ++v;
                            continue;
                        }
                        /* current range starts within the base range but end behind it */
                        goto baseerror;
                    } else {
                        /* current range is smaller than the base,
                         * move current, but stay with the base */
                        continue;
                    }
                }
            }
        }
        if (u != parts_done) {
baseerror:
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                   "Invalid %s restriction - the derived restriction (%s) is not equally or more limiting.",
                   length_restr ? "length" : "range", range_p->arg);
            goto cleanup;
        }
    }

    if (!(*range)) {
        *range = calloc(1, sizeof **range);
        LY_CHECK_ERR_RET(!(*range), LOGMEM(ctx->ctx), LY_EMEM);
    }

    /* we rewrite the following values as the types chain is being processed */
    if (range_p->eapptag) {
        lydict_remove(ctx->ctx, (*range)->eapptag);
        (*range)->eapptag = lydict_insert(ctx->ctx, range_p->eapptag, 0);
    }
    if (range_p->emsg) {
        lydict_remove(ctx->ctx, (*range)->emsg);
        (*range)->emsg = lydict_insert(ctx->ctx, range_p->emsg, 0);
    }
    if (range_p->dsc) {
        lydict_remove(ctx->ctx, (*range)->dsc);
        (*range)->dsc = lydict_insert(ctx->ctx, range_p->dsc, 0);
    }
    if (range_p->ref) {
        lydict_remove(ctx->ctx, (*range)->ref);
        (*range)->ref = lydict_insert(ctx->ctx, range_p->ref, 0);
    }
    /* extensions are taken only from the last range by the caller */

    (*range)->parts = parts;
    parts = NULL;
    ret = LY_SUCCESS;
cleanup:
    /* TODO clean up */
    LY_ARRAY_FREE(parts);

    return ret;
}

/**
 * @brief Checks pattern syntax.
 *
 * @param[in] ctx Compile context.
 * @param[in] pattern Pattern to check.
 * @param[in,out] pcre_precomp Precompiled PCRE pattern. If NULL, the compiled information used to validate pattern are freed.
 * @return LY_ERR value - LY_SUCCESS, LY_EMEM, LY_EVALID.
 */
static LY_ERR
lys_compile_type_pattern_check(struct lysc_ctx *ctx, const char *pattern, pcre **pcre_precomp)
{
    int idx, idx2, start, end, err_offset, count;
    char *perl_regex, *ptr;
    const char *err_msg, *orig_ptr;
    pcre *precomp;
#define URANGE_LEN 19
    char *ublock2urange[][2] = {
        {"BasicLatin", "[\\x{0000}-\\x{007F}]"},
        {"Latin-1Supplement", "[\\x{0080}-\\x{00FF}]"},
        {"LatinExtended-A", "[\\x{0100}-\\x{017F}]"},
        {"LatinExtended-B", "[\\x{0180}-\\x{024F}]"},
        {"IPAExtensions", "[\\x{0250}-\\x{02AF}]"},
        {"SpacingModifierLetters", "[\\x{02B0}-\\x{02FF}]"},
        {"CombiningDiacriticalMarks", "[\\x{0300}-\\x{036F}]"},
        {"Greek", "[\\x{0370}-\\x{03FF}]"},
        {"Cyrillic", "[\\x{0400}-\\x{04FF}]"},
        {"Armenian", "[\\x{0530}-\\x{058F}]"},
        {"Hebrew", "[\\x{0590}-\\x{05FF}]"},
        {"Arabic", "[\\x{0600}-\\x{06FF}]"},
        {"Syriac", "[\\x{0700}-\\x{074F}]"},
        {"Thaana", "[\\x{0780}-\\x{07BF}]"},
        {"Devanagari", "[\\x{0900}-\\x{097F}]"},
        {"Bengali", "[\\x{0980}-\\x{09FF}]"},
        {"Gurmukhi", "[\\x{0A00}-\\x{0A7F}]"},
        {"Gujarati", "[\\x{0A80}-\\x{0AFF}]"},
        {"Oriya", "[\\x{0B00}-\\x{0B7F}]"},
        {"Tamil", "[\\x{0B80}-\\x{0BFF}]"},
        {"Telugu", "[\\x{0C00}-\\x{0C7F}]"},
        {"Kannada", "[\\x{0C80}-\\x{0CFF}]"},
        {"Malayalam", "[\\x{0D00}-\\x{0D7F}]"},
        {"Sinhala", "[\\x{0D80}-\\x{0DFF}]"},
        {"Thai", "[\\x{0E00}-\\x{0E7F}]"},
        {"Lao", "[\\x{0E80}-\\x{0EFF}]"},
        {"Tibetan", "[\\x{0F00}-\\x{0FFF}]"},
        {"Myanmar", "[\\x{1000}-\\x{109F}]"},
        {"Georgian", "[\\x{10A0}-\\x{10FF}]"},
        {"HangulJamo", "[\\x{1100}-\\x{11FF}]"},
        {"Ethiopic", "[\\x{1200}-\\x{137F}]"},
        {"Cherokee", "[\\x{13A0}-\\x{13FF}]"},
        {"UnifiedCanadianAboriginalSyllabics", "[\\x{1400}-\\x{167F}]"},
        {"Ogham", "[\\x{1680}-\\x{169F}]"},
        {"Runic", "[\\x{16A0}-\\x{16FF}]"},
        {"Khmer", "[\\x{1780}-\\x{17FF}]"},
        {"Mongolian", "[\\x{1800}-\\x{18AF}]"},
        {"LatinExtendedAdditional", "[\\x{1E00}-\\x{1EFF}]"},
        {"GreekExtended", "[\\x{1F00}-\\x{1FFF}]"},
        {"GeneralPunctuation", "[\\x{2000}-\\x{206F}]"},
        {"SuperscriptsandSubscripts", "[\\x{2070}-\\x{209F}]"},
        {"CurrencySymbols", "[\\x{20A0}-\\x{20CF}]"},
        {"CombiningMarksforSymbols", "[\\x{20D0}-\\x{20FF}]"},
        {"LetterlikeSymbols", "[\\x{2100}-\\x{214F}]"},
        {"NumberForms", "[\\x{2150}-\\x{218F}]"},
        {"Arrows", "[\\x{2190}-\\x{21FF}]"},
        {"MathematicalOperators", "[\\x{2200}-\\x{22FF}]"},
        {"MiscellaneousTechnical", "[\\x{2300}-\\x{23FF}]"},
        {"ControlPictures", "[\\x{2400}-\\x{243F}]"},
        {"OpticalCharacterRecognition", "[\\x{2440}-\\x{245F}]"},
        {"EnclosedAlphanumerics", "[\\x{2460}-\\x{24FF}]"},
        {"BoxDrawing", "[\\x{2500}-\\x{257F}]"},
        {"BlockElements", "[\\x{2580}-\\x{259F}]"},
        {"GeometricShapes", "[\\x{25A0}-\\x{25FF}]"},
        {"MiscellaneousSymbols", "[\\x{2600}-\\x{26FF}]"},
        {"Dingbats", "[\\x{2700}-\\x{27BF}]"},
        {"BraillePatterns", "[\\x{2800}-\\x{28FF}]"},
        {"CJKRadicalsSupplement", "[\\x{2E80}-\\x{2EFF}]"},
        {"KangxiRadicals", "[\\x{2F00}-\\x{2FDF}]"},
        {"IdeographicDescriptionCharacters", "[\\x{2FF0}-\\x{2FFF}]"},
        {"CJKSymbolsandPunctuation", "[\\x{3000}-\\x{303F}]"},
        {"Hiragana", "[\\x{3040}-\\x{309F}]"},
        {"Katakana", "[\\x{30A0}-\\x{30FF}]"},
        {"Bopomofo", "[\\x{3100}-\\x{312F}]"},
        {"HangulCompatibilityJamo", "[\\x{3130}-\\x{318F}]"},
        {"Kanbun", "[\\x{3190}-\\x{319F}]"},
        {"BopomofoExtended", "[\\x{31A0}-\\x{31BF}]"},
        {"EnclosedCJKLettersandMonths", "[\\x{3200}-\\x{32FF}]"},
        {"CJKCompatibility", "[\\x{3300}-\\x{33FF}]"},
        {"CJKUnifiedIdeographsExtensionA", "[\\x{3400}-\\x{4DB5}]"},
        {"CJKUnifiedIdeographs", "[\\x{4E00}-\\x{9FFF}]"},
        {"YiSyllables", "[\\x{A000}-\\x{A48F}]"},
        {"YiRadicals", "[\\x{A490}-\\x{A4CF}]"},
        {"HangulSyllables", "[\\x{AC00}-\\x{D7A3}]"},
        {"PrivateUse", "[\\x{E000}-\\x{F8FF}]"},
        {"CJKCompatibilityIdeographs", "[\\x{F900}-\\x{FAFF}]"},
        {"AlphabeticPresentationForms", "[\\x{FB00}-\\x{FB4F}]"},
        {"ArabicPresentationForms-A", "[\\x{FB50}-\\x{FDFF}]"},
        {"CombiningHalfMarks", "[\\x{FE20}-\\x{FE2F}]"},
        {"CJKCompatibilityForms", "[\\x{FE30}-\\x{FE4F}]"},
        {"SmallFormVariants", "[\\x{FE50}-\\x{FE6F}]"},
        {"ArabicPresentationForms-B", "[\\x{FE70}-\\x{FEFE}]"},
        {"HalfwidthandFullwidthForms", "[\\x{FF00}-\\x{FFEF}]"},
        {NULL, NULL}
    };

    /* adjust the expression to a Perl equivalent
     * http://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#regexs */

    /* we need to replace all "$" with "\$", count them now */
    for (count = 0, ptr = strpbrk(pattern, "^$"); ptr; ++count, ptr = strpbrk(ptr + 1, "^$"));

    perl_regex = malloc((strlen(pattern) + 4 + count) * sizeof(char));
    LY_CHECK_ERR_RET(!perl_regex, LOGMEM(ctx->ctx), LY_EMEM);
    perl_regex[0] = '\0';

    ptr = perl_regex;

    if (strncmp(pattern + strlen(pattern) - 2, ".*", 2)) {
        /* we will add line-end anchoring */
        ptr[0] = '(';
        ++ptr;
    }

    for (orig_ptr = pattern; orig_ptr[0]; ++orig_ptr) {
        if (orig_ptr[0] == '$') {
            ptr += sprintf(ptr, "\\$");
        } else if (orig_ptr[0] == '^') {
            ptr += sprintf(ptr, "\\^");
        } else {
            ptr[0] = orig_ptr[0];
            ++ptr;
        }
    }

    if (strncmp(pattern + strlen(pattern) - 2, ".*", 2)) {
        ptr += sprintf(ptr, ")$");
    } else {
        ptr[0] = '\0';
        ++ptr;
    }

    /* substitute Unicode Character Blocks with exact Character Ranges */
    while ((ptr = strstr(perl_regex, "\\p{Is"))) {
        start = ptr - perl_regex;

        ptr = strchr(ptr, '}');
        if (!ptr) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_INREGEXP,
                   pattern, perl_regex + start + 2, "unterminated character property");
            free(perl_regex);
            return LY_EVALID;
        }
        end = (ptr - perl_regex) + 1;

        /* need more space */
        if (end - start < URANGE_LEN) {
            perl_regex = ly_realloc(perl_regex, strlen(perl_regex) + (URANGE_LEN - (end - start)) + 1);
            LY_CHECK_ERR_RET(!perl_regex, LOGMEM(ctx->ctx); free(perl_regex), LY_EMEM);
        }

        /* find our range */
        for (idx = 0; ublock2urange[idx][0]; ++idx) {
            if (!strncmp(perl_regex + start + 5, ublock2urange[idx][0], strlen(ublock2urange[idx][0]))) {
                break;
            }
        }
        if (!ublock2urange[idx][0]) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_INREGEXP,
                   pattern, perl_regex + start + 5, "unknown block name");
            free(perl_regex);
            return LY_EVALID;
        }

        /* make the space in the string and replace the block (but we cannot include brackets if it was already enclosed in them) */
        for (idx2 = 0, count = 0; idx2 < start; ++idx2) {
            if ((perl_regex[idx2] == '[') && (!idx2 || (perl_regex[idx2 - 1] != '\\'))) {
                ++count;
            }
            if ((perl_regex[idx2] == ']') && (!idx2 || (perl_regex[idx2 - 1] != '\\'))) {
                --count;
            }
        }
        if (count) {
            /* skip brackets */
            memmove(perl_regex + start + (URANGE_LEN - 2), perl_regex + end, strlen(perl_regex + end) + 1);
            memcpy(perl_regex + start, ublock2urange[idx][1] + 1, URANGE_LEN - 2);
        } else {
            memmove(perl_regex + start + URANGE_LEN, perl_regex + end, strlen(perl_regex + end) + 1);
            memcpy(perl_regex + start, ublock2urange[idx][1], URANGE_LEN);
        }
    }

    /* must return 0, already checked during parsing */
    precomp = pcre_compile(perl_regex, PCRE_UTF8 | PCRE_ANCHORED | PCRE_DOLLAR_ENDONLY | PCRE_NO_AUTO_CAPTURE,
                           &err_msg, &err_offset, NULL);
    if (!precomp) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_INREGEXP, pattern, perl_regex + err_offset, err_msg);
        free(perl_regex);
        return LY_EVALID;
    }
    free(perl_regex);

    if (pcre_precomp) {
        *pcre_precomp = precomp;
    } else {
        free(precomp);
    }

    return LY_SUCCESS;

#undef URANGE_LEN
}

/**
 * @brief Compile parsed pattern restriction in conjunction with the patterns from base type.
 * @param[in] ctx Compile context.
 * @param[in] patterns_p Array of parsed patterns from the current type to compile.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in] base_patterns Compiled patterns from the type from which the current type is derived.
 * Patterns from the base type are inherited to have all the patterns that have to match at one place.
 * @param[out] patterns Pointer to the storage for the patterns of the current type.
 * @return LY_ERR LY_SUCCESS, LY_EMEM, LY_EVALID.
 */
static LY_ERR
lys_compile_type_patterns(struct lysc_ctx *ctx, struct lysp_restr *patterns_p, int options,
                          struct lysc_pattern **base_patterns, struct lysc_pattern ***patterns)
{
    struct lysc_pattern **pattern;
    unsigned int u, v;
    const char *err_msg;
    LY_ERR ret = LY_SUCCESS;

    /* first, copy the patterns from the base type */
    if (base_patterns) {
        *patterns = lysc_patterns_dup(ctx->ctx, base_patterns);
        LY_CHECK_ERR_RET(!(*patterns), LOGMEM(ctx->ctx), LY_EMEM);
    }

    LY_ARRAY_FOR(patterns_p, u) {
        LY_ARRAY_NEW_RET(ctx->ctx, (*patterns), pattern, LY_EMEM);
        *pattern = calloc(1, sizeof **pattern);
        ++(*pattern)->refcount;

        ret = lys_compile_type_pattern_check(ctx, &patterns_p[u].arg[1], &(*pattern)->expr);
        LY_CHECK_RET(ret);
        (*pattern)->expr_extra = pcre_study((*pattern)->expr, 0, &err_msg);
        if (err_msg) {
            LOGWRN(ctx->ctx, "Studying pattern \"%s\" failed (%s).", pattern, err_msg);
        }

        if (patterns_p[u].arg[0] == 0x15) {
            (*pattern)->inverted = 1;
        }
        DUP_STRING(ctx->ctx, patterns_p[u].eapptag, (*pattern)->eapptag);
        DUP_STRING(ctx->ctx, patterns_p[u].emsg, (*pattern)->emsg);
        DUP_STRING(ctx->ctx, patterns_p[u].dsc, (*pattern)->dsc);
        DUP_STRING(ctx->ctx, patterns_p[u].ref, (*pattern)->ref);
        COMPILE_ARRAY_GOTO(ctx, patterns_p[u].exts, (*pattern)->exts,
                           options, v, lys_compile_ext, ret, done);
    }
done:
    return ret;
}

/**
 * @brief map of the possible restrictions combination for the specific built-in type.
 */
static uint16_t type_substmt_map[LY_DATA_TYPE_COUNT] = {
    0 /* LY_TYPE_UNKNOWN */,
    LYS_SET_LENGTH /* LY_TYPE_BINARY */,
    LYS_SET_RANGE /* LY_TYPE_UINT8 */,
    LYS_SET_RANGE /* LY_TYPE_UINT16 */,
    LYS_SET_RANGE /* LY_TYPE_UINT32 */,
    LYS_SET_RANGE /* LY_TYPE_UINT64 */,
    LYS_SET_LENGTH | LYS_SET_PATTERN /* LY_TYPE_STRING */,
    LYS_SET_BIT /* LY_TYPE_BITS */,
    0 /* LY_TYPE_BOOL */,
    LYS_SET_FRDIGITS | LYS_SET_RANGE /* LY_TYPE_DEC64 */,
    0 /* LY_TYPE_EMPTY */,
    LYS_SET_ENUM /* LY_TYPE_ENUM */,
    LYS_SET_BASE /* LY_TYPE_IDENT */,
    LYS_SET_REQINST /* LY_TYPE_INST */,
    LYS_SET_REQINST | LYS_SET_PATH /* LY_TYPE_LEAFREF */,
    LYS_SET_TYPE /* LY_TYPE_UNION */,
    LYS_SET_RANGE /* LY_TYPE_INT8 */,
    LYS_SET_RANGE /* LY_TYPE_INT16 */,
    LYS_SET_RANGE /* LY_TYPE_INT32 */,
    LYS_SET_RANGE /* LY_TYPE_INT64 */
};

/**
 * @brief stringification of the YANG built-in data types
 */
const char* ly_data_type2str[LY_DATA_TYPE_COUNT] = {"unknown", "binary", "8bit unsigned integer", "16bit unsigned integer",
    "32bit unsigned integer", "64bit unsigned integer", "string", "bits", "boolean", "decimal64", "empty", "enumeration",
    "identityref", "instance-identifier", "leafref", "union", "8bit integer", "16bit integer", "32bit integer", "64bit integer"
};

/**
 * @brief Compile parsed type's enum structures (for enumeration and bits types).
 * @param[in] ctx Compile context.
 * @param[in] enums_p Array of the parsed enum structures to compile.
 * @param[in] basetype Base YANG built-in type from which the current type is derived. Only LY_TYPE_ENUM and LY_TYPE_BITS are expected.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in] base_enums Array of the compiled enums information from the (latest) base type to check if the current enums are compatible.
 * @param[out] enums Newly created array of the compiled enums information for the current type.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_type_enums(struct lysc_ctx *ctx, struct lysp_type_enum *enums_p, LY_DATA_TYPE basetype, int options,
                       struct lysc_type_enum_item *base_enums, struct lysc_type_enum_item **enums)
{
    LY_ERR ret = LY_SUCCESS;
    unsigned int u, v, match;
    int32_t value = 0;
    uint32_t position = 0;
    struct lysc_type_enum_item *e, storage;

    if (base_enums && ctx->mod_def->version < 2) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG, "%s type can be subtyped only in YANG 1.1 modules.",
               basetype == LY_TYPE_ENUM ? "Enumeration" : "Bits");
        return LY_EVALID;
    }

    LY_ARRAY_FOR(enums_p, u) {
        LY_ARRAY_NEW_RET(ctx->ctx, *enums, e, LY_EMEM);
        DUP_STRING(ctx->ctx, enums_p[u].name, e->name);
        DUP_STRING(ctx->ctx, enums_p[u].ref, e->dsc);
        DUP_STRING(ctx->ctx, enums_p[u].ref, e->ref);
        if (base_enums) {
            /* check the enum/bit presence in the base type - the set of enums/bits in the derived type must be a subset */
            LY_ARRAY_FOR(base_enums, v) {
                if (!strcmp(e->name, base_enums[v].name)) {
                    break;
                }
            }
            if (v == LY_ARRAY_SIZE(base_enums)) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid %s - derived type adds new item \"%s\".",
                       basetype == LY_TYPE_ENUM ? "enumeration" : "bits", e->name);
                return LY_EVALID;
            }
            match = v;
        }

        if (basetype == LY_TYPE_ENUM) {
            if (enums_p[u].flags & LYS_SET_VALUE) {
                e->value = (int32_t)enums_p[u].value;
                if (!u || e->value >= value) {
                    value = e->value + 1;
                }
                /* check collision with other values */
                for (v = 0; v < LY_ARRAY_SIZE(*enums) - 1; ++v) {
                    if (e->value == (*enums)[v].value) {
                        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                               "Invalid enumeration - value %d collide in items \"%s\" and \"%s\".",
                               e->value, e->name, (*enums)[v].name);
                        return LY_EVALID;
                    }
                }
            } else if (base_enums) {
                /* inherit the assigned value */
                e->value = base_enums[match].value;
                if (!u || e->value >= value) {
                    value = e->value + 1;
                }
            } else {
                /* assign value automatically */
                if (u && value == INT32_MIN) {
                    /* counter overflow */
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                           "Invalid enumeration - it is not possible to auto-assign enum value for "
                           "\"%s\" since the highest value is already 2147483647.", e->name);
                    return LY_EVALID;
                }
                e->value = value++;
            }
        } else { /* LY_TYPE_BITS */
            if (enums_p[u].flags & LYS_SET_VALUE) {
                e->value = (int32_t)enums_p[u].value;
                if (!u || (uint32_t)e->value >= position) {
                    position = (uint32_t)e->value + 1;
                }
                /* check collision with other values */
                for (v = 0; v < LY_ARRAY_SIZE(*enums) - 1; ++v) {
                    if (e->value == (*enums)[v].value) {
                        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                               "Invalid bits - position %u collide in items \"%s\" and \"%s\".",
                               (uint32_t)e->value, e->name, (*enums)[v].name);
                        return LY_EVALID;
                    }
                }
            } else if (base_enums) {
                /* inherit the assigned value */
                e->value = base_enums[match].value;
                if (!u || (uint32_t)e->value >= position) {
                    position = (uint32_t)e->value + 1;
                }
            } else {
                /* assign value automatically */
                if (u && position == 0) {
                    /* counter overflow */
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                           "Invalid bits - it is not possible to auto-assign bit position for "
                           "\"%s\" since the highest value is already 4294967295.", e->name);
                    return LY_EVALID;
                }
                e->value = position++;
            }
        }

        if (base_enums) {
            /* the assigned values must not change from the derived type */
            if (e->value != base_enums[match].value) {
                if (basetype == LY_TYPE_ENUM) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid enumeration - value of the item \"%s\" has changed from %d to %d in the derived type.",
                       e->name, base_enums[match].value, e->value);
                } else {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid bits - position of the item \"%s\" has changed from %u to %u in the derived type.",
                       e->name, (uint32_t)base_enums[match].value, (uint32_t)e->value);
                }
                return LY_EVALID;
            }
        }

        COMPILE_ARRAY_GOTO(ctx, enums_p[u].iffeatures, e->iffeatures, options, v, lys_compile_iffeature, ret, done);
        COMPILE_ARRAY_GOTO(ctx, enums_p[u].exts, e->exts, options, v, lys_compile_ext, ret, done);

        if (basetype == LY_TYPE_BITS) {
            /* keep bits ordered by position */
            for (v = u; v && (*enums)[v - 1].value > e->value; --v);
            if (v != u) {
                memcpy(&storage, e, sizeof *e);
                memmove(&(*enums)[v + 1], &(*enums)[v], (u - v) * sizeof **enums);
                memcpy(&(*enums)[v], &storage, sizeof storage);
            }
        }
    }

done:
    return ret;
}

#define MOVE_PATH_PARENT(NODE, LIMIT_COND, TERM, ERR_MSG, ...) \
    for ((NODE) = (NODE)->parent; \
         (NODE) && !((NODE)->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_ACTION | LYS_NOTIF | LYS_ACTION)); \
         (NODE) = (NODE)->parent); \
    if (!(NODE) && (LIMIT_COND)) { /* we are going higher than top-level */ \
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE, ERR_MSG, ##__VA_ARGS__); \
        TERM; \
    }

/**
 * @brief Validate the predicate(s) from the leafref path.
 * @param[in] ctx Compile context
 * @param[in, out] predicate Pointer to the predicate in the leafref path. The pointer is moved after the validated predicate(s).
 * Since there can be multiple adjacent predicates for lists with multiple keys, all such predicates are validated.
 * @param[in] start_node Path context node (where the path is instantiated).
 * @param[in] context_node Predicate context node (where the predicate is placed).
 * @param[in] path_context Schema where the path was defined to correct resolve of the prefixes.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_leafref_predicate_validate(struct lysc_ctx *ctx, const char **predicate, const struct lysc_node *start_node,
                                       const struct lysc_node_list *context_node, const struct lys_module *path_context)
{
    LY_ERR ret = LY_EVALID;
    const struct lys_module *mod;
    const struct lysc_node *src_node, *dst_node;
    const char *path_key_expr, *pke_start, *src, *src_prefix, *dst, *dst_prefix;
    size_t src_len, src_prefix_len, dst_len, dst_prefix_len;
    unsigned int dest_parent_times, c, u;
    const char *start, *end, *pke_end;
    struct ly_set keys = {0};
    int i;

    assert(path_context);

    while (**predicate == '[') {
        start = (*predicate)++;

        while (isspace(**predicate)) {
            ++(*predicate);
        }
        LY_CHECK_GOTO(lys_parse_nodeid(predicate, &src_prefix, &src_prefix_len, &src, &src_len), cleanup);
        while (isspace(**predicate)) {
            ++(*predicate);
        }
        if (**predicate != '=') {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path predicate \"%.*s\" - missing \"=\" after node-identifier.",
                   *predicate - start + 1, start);
            goto cleanup;
        }
        ++(*predicate);
        while (isspace(**predicate)) {
            ++(*predicate);
        }

        if ((end = pke_end = strchr(*predicate, ']')) == NULL) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path predicate \"%s\" - missing predicate termination.", start);
            goto cleanup;
        }
        --pke_end;
        while (isspace(*pke_end)) {
            --pke_end;
        }
        ++pke_end;
        /* localize path-key-expr */
        pke_start = path_key_expr = *predicate;
        /* move after the current predicate */
        *predicate = end + 1;

        /* source (must be leaf or leaf-list) */
        if (src_prefix) {
            mod = lys_module_find_prefix(path_context, src_prefix, src_prefix_len);
            if (!mod) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                       "Invalid leafref path predicate \"%.*s\" - prefix \"%.*s\" not defined in module \"%s\".",
                       *predicate - start, start, src_prefix_len, src_prefix, path_context->name);
                goto cleanup;
            }
        } else {
            mod = start_node->module;
        }
        src_node = NULL;
        if (context_node->keys) {
            for (u = 0; u < LY_ARRAY_SIZE(context_node->keys); ++u) {
                if (!strncmp(src, context_node->keys[u]->name, src_len) && context_node->keys[u]->name[src_len] == '\0') {
                    src_node = (const struct lysc_node*)context_node->keys[u];
                    break;
                }
            }
        }
        if (!src_node) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path predicate \"%.*s\" - predicate's key node \"%.*s\" not found.",
                   *predicate - start, start, src_len, src, mod->name);
            goto cleanup;
        }

        /* check that there is only one predicate for the */
        c = keys.count;
        i = ly_set_add(&keys, (void*)src_node, 0);
        LY_CHECK_GOTO(i == -1, cleanup);
        if (keys.count == c) { /* node was already present in the set */
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path predicate \"%.*s\" - multiple equality tests for the key \"%s\".",
                   *predicate - start, start, src_node->name);
            goto cleanup;
        }

        /* destination */
        dest_parent_times = 0;
        dst_node = start_node;

        /* current-function-invocation *WSP "/" *WSP rel-path-keyexpr */
        if (strncmp(path_key_expr, "current()", 9)) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path predicate \"%.*s\" - missing current-function-invocation.",
                   *predicate - start, start);
            goto cleanup;
        }
        path_key_expr += 9;
        while (isspace(*path_key_expr)) {
            ++path_key_expr;
        }

        if (*path_key_expr != '/') {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path predicate \"%.*s\" - missing \"/\" after current-function-invocation.",
                   *predicate - start, start);
            goto cleanup;
        }
        ++path_key_expr;
        while (isspace(*path_key_expr)) {
            ++path_key_expr;
        }

        /* rel-path-keyexpr:
         * 1*(".." *WSP "/" *WSP) *(node-identifier *WSP "/" *WSP) node-identifier */
        while (!strncmp(path_key_expr, "..", 2)) {
            ++dest_parent_times;
            path_key_expr += 2;
            while (isspace(*path_key_expr)) {
                ++path_key_expr;
            }
            if (*path_key_expr != '/') {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                       "Invalid leafref path predicate \"%.*s\" - missing \"/\" in \"../\" rel-path-keyexpr pattern.",
                       *predicate - start, start);
                goto cleanup;
            }
            ++path_key_expr;
            while (isspace(*path_key_expr)) {
                ++path_key_expr;
            }

            /* path is supposed to be evaluated in data tree, so we have to skip
             * all schema nodes that cannot be instantiated in data tree */
            MOVE_PATH_PARENT(dst_node, !strncmp(path_key_expr, "..", 2), goto cleanup,
                             "Invalid leafref path predicate \"%.*s\" - too many \"..\" in rel-path-keyexpr.",
                             *predicate - start, start);
        }
        if (!dest_parent_times) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path predicate \"%.*s\" - at least one \"..\" is expected in rel-path-keyexpr.",
                   *predicate - start, start);
            goto cleanup;
        }
        if (path_key_expr == pke_end) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path predicate \"%.*s\" - at least one node-identifier is expected in rel-path-keyexpr.",
                   *predicate - start, start);
            goto cleanup;
        }

        while(path_key_expr != pke_end) {
            if (lys_parse_nodeid(&path_key_expr, &dst_prefix, &dst_prefix_len, &dst, &dst_len)) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid node identifier in leafref path predicate - character %d (of %.*s).",
                       path_key_expr - start + 1, *predicate - start, start);
                goto cleanup;
            }

            if (dst_prefix) {
                mod = lys_module_find_prefix(path_context, dst_prefix, dst_prefix_len);
            } else {
                mod = start_node->module;
            }
            if (!mod) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                       "Invalid leafref path predicate \"%.*s\" - unable to find module of the node \"%.*s\" in rel-path_keyexpr.",
                       *predicate - start, start, dst_len, dst);
                goto cleanup;
            }

            dst_node = lys_child(dst_node, mod, dst, dst_len, 0, LYS_GETNEXT_NOSTATECHECK);
            if (!dst_node) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                       "Invalid leafref path predicate \"%.*s\" - unable to find node \"%.*s\" in the rel-path_keyexpr.",
                       *predicate - start, start, path_key_expr - pke_start, pke_start);
                goto cleanup;
            }
        }
        if (!(dst_node->nodetype & (dst_node->module->version < LYS_VERSION_1_1 ? LYS_LEAF : LYS_LEAF | LYS_LEAFLIST))) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path predicate \"%.*s\" - rel-path_keyexpr \"%.*s\" refers %s instead of leaf.",
                   *predicate - start, start, path_key_expr - pke_start, pke_start, lys_nodetype2str(dst_node->nodetype));
            goto cleanup;
        }
    }

    ret = LY_SUCCESS;
cleanup:
    ly_set_erase(&keys, NULL);
    return ret;
}

/**
 * @brief Parse path-arg (leafref). Get tokens of the path by repetitive calls of the function.
 *
 * path-arg            = absolute-path / relative-path
 * absolute-path       = 1*("/" (node-identifier *path-predicate))
 * relative-path       = 1*(".." "/") descendant-path
 *
 * @param[in,out] path Path to parse.
 * @param[out] prefix Prefix of the token, NULL if there is not any.
 * @param[out] pref_len Length of the prefix, 0 if there is not any.
 * @param[out] name Name of the token.
 * @param[out] nam_len Length of the name.
 * @param[out] parent_times Number of leading ".." in the path. Must be 0 on the first call,
 *                          must not be changed between consecutive calls. -1 if the
 *                          path is absolute.
 * @param[out] has_predicate Flag to mark whether there is a predicate specified.
 * @return LY_ERR value: LY_SUCCESS or LY_EINVAL in case of invalid character in the path.
 */
static LY_ERR
lys_path_token(const char **path, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len,
               int *parent_times, int *has_predicate)
{
    int par_times = 0;

    assert(path && *path);
    assert(parent_times);
    assert(prefix);
    assert(prefix_len);
    assert(name);
    assert(name_len);
    assert(has_predicate);

    *prefix = NULL;
    *prefix_len = 0;
    *name = NULL;
    *name_len = 0;
    *has_predicate = 0;

    if (!*parent_times) {
        if (!strncmp(*path, "..", 2)) {
            *path += 2;
            ++par_times;
            while (!strncmp(*path, "/..", 3)) {
                *path += 3;
                ++par_times;
            }
        }
        if (par_times) {
            *parent_times = par_times;
        } else {
            *parent_times = -1;
        }
    }

    if (**path != '/') {
        return LY_EINVAL;
    }
    /* skip '/' */
    ++(*path);

    /* node-identifier ([prefix:]name) */
    LY_CHECK_RET(lys_parse_nodeid(path, prefix, prefix_len, name, name_len));

    if ((**path == '/' && (*path)[1]) || !**path) {
        /* path continues by another token or this is the last token */
        return LY_SUCCESS;
    } else if ((*path)[0] != '[') {
        /* unexpected character */
        return LY_EINVAL;
    } else {
        /* predicate starting with [ */
        *has_predicate = 1;
        return LY_SUCCESS;
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
    unsigned int u, v, count;
    struct ly_set features = {0};

    for (iter = refnode; iter; iter = iter->parent) {
        if (iter->iffeatures) {
            LY_ARRAY_FOR(iter->iffeatures, u) {
                LY_ARRAY_FOR(iter->iffeatures[u].features, v) {
                    LY_CHECK_GOTO(ly_set_add(&features, iter->iffeatures[u].features[v], 0) == -1, cleanup);
                }
            }
        }
    }

    /* we should have, in features set, a superset of features applicable to the target node.
     * So when adding features applicable to the target into the features set, we should not be
     * able to actually add any new feature, otherwise it is not a subset of features applicable
     * to the leafref itself. */
    count = features.count;
    for (iter = target; iter; iter = iter->parent) {
        if (iter->iffeatures) {
            LY_ARRAY_FOR(iter->iffeatures, u) {
                LY_ARRAY_FOR(iter->iffeatures[u].features, v) {
                    if ((unsigned int)ly_set_add(&features, iter->iffeatures[u].features[v], 0) >= count) {
                        /* new feature was added (or LY_EMEM) */
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

/**
 * @brief Validate the leafref path.
 * @param[in] ctx Compile context
 * @param[in] startnode Path context node (where the leafref path begins/is placed).
 * @param[in] leafref Leafref to validate.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_leafref_validate(struct lysc_ctx *ctx, struct lysc_node *startnode, struct lysc_type_leafref *leafref)
{
    const struct lysc_node *node = NULL, *parent = NULL;
    const struct lys_module *mod;
    struct lysc_type *type;
    const char *id, *prefix, *name;
    size_t prefix_len, name_len;
    int parent_times = 0, has_predicate;
    unsigned int iter, u;
    LY_ERR ret = LY_SUCCESS;

    assert(ctx);
    assert(startnode);
    assert(leafref);

    /* TODO leafref targets may be not implemented, in such a case we actually could make (we did it in libyang1) such a models implemented */

    iter = 0;
    id = leafref->path;
    while(*id && (ret = lys_path_token(&id, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate)) == LY_SUCCESS) {
        if (!iter) { /* first iteration */
            /* precess ".." in relative paths */
            if (parent_times > 0) {
                /* move from the context node */
                for (u = 0, parent = startnode; u < (unsigned int)parent_times; u++) {
                    /* path is supposed to be evaluated in data tree, so we have to skip
                    * all schema nodes that cannot be instantiated in data tree */
                    MOVE_PATH_PARENT(parent, u < (unsigned int)parent_times - 1, return LY_EVALID,
                                     "Invalid leafref path \"%s\" - too many \"..\" in the path.", leafref->path);
                }
            }
        }

        if (prefix) {
            mod = lys_module_find_prefix(leafref->path_context, prefix, prefix_len);
        } else {
            mod = startnode->module;
        }
        if (!mod) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path - unable to find module connected with the prefix of the node \"%.*s\".",
                   id - leafref->path, leafref->path);
            return LY_EVALID;
        }

        node = lys_child(parent, mod, name, name_len, 0, LYS_GETNEXT_NOSTATECHECK);
        if (!node) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path - unable to find \"%.*s\".", id - leafref->path, leafref->path);
            return LY_EVALID;
        }
        parent = node;

        if (has_predicate) {
            /* we have predicate, so the current result must be list */
            if (node->nodetype != LYS_LIST) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                       "Invalid leafref path - node \"%.*s\" is expected to be a list, but it is %s.",
                       id - leafref->path, leafref->path, lys_nodetype2str(node->nodetype));
                return LY_EVALID;
            }

            LY_CHECK_RET(lys_compile_leafref_predicate_validate(ctx, &id, startnode, (struct lysc_node_list*)node, leafref->path_context),
                         LY_EVALID);
        }

        ++iter;
    }
    if (ret) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
               "Invalid leafref path at character %d (%s).", id - leafref->path + 1, leafref->path);
        return LY_EVALID;
    }

    if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
               "Invalid leafref path \"%s\" - target node is %s instead of leaf or leaf-list.",
               leafref->path, lys_nodetype2str(node->nodetype));
        return LY_EVALID;
    }

    /* check status */
    if (lysc_check_status(ctx, startnode->flags, startnode->module, startnode->name, node->flags, node->module, node->name)) {
        return LY_EVALID;
    }

    /* check config */
    if (leafref->require_instance && (startnode->flags & LYS_CONFIG_W)) {
        if (node->flags & LYS_CONFIG_R) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path \"%s\" - target is supposed to represent configuration data (as the leafref does), but it does not.",
                   leafref->path);
            return LY_EVALID;
        }
    }

    /* store the target's type and check for circular chain of leafrefs */
    leafref->realtype = ((struct lysc_node_leaf*)node)->type;
    for (type = leafref->realtype; type && type->basetype == LY_TYPE_LEAFREF; type = ((struct lysc_type_leafref*)type)->realtype) {
        if (type == (struct lysc_type*)leafref) {
            /* circular chain detected */
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid leafref path \"%s\" - circular chain of leafrefs detected.", leafref->path);
            return LY_EVALID;
        }
    }

    /* check if leafref and its target are under common if-features */
    if (lys_compile_leafref_features_validate(startnode, node)) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
               "Invalid leafref path \"%s\" - set of features applicable to the leafref target is not a subset of features applicable to the leafref itself.",
               leafref->path);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

static LY_ERR lys_compile_type(struct lysc_ctx *ctx, struct lysp_node *context_node_p, uint16_t context_flags, struct lysp_module *context_mod, const char *context_name,
                               struct lysp_type *type_p, int options, struct lysc_type **type, const char **units);
/**
 * @brief The core of the lys_compile_type() - compile information about the given type (from typedef or leaf/leaf-list).
 * @param[in] ctx Compile context.
 * @param[in] context_node_p Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] context_flags Flags of the context node or the referencing typedef to correctly check status of referencing and referenced objects.
 * @param[in] context_mod Module of the context node or the referencing typedef to correctly check status of referencing and referenced objects.
 * @param[in] context_name Name of the context node or referencing typedef for logging.
 * @param[in] type_p Parsed type to compile.
 * @param[in] module Context module for the leafref path (to correctly resolve prefixes in path)
 * @param[in] basetype Base YANG built-in type of the type to compile.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in] tpdfname Name of the type's typedef, serves as a flag - if it is leaf/leaf-list's type, it is NULL.
 * @param[in] base The latest base (compiled) type from which the current type is being derived.
 * @param[out] type Newly created type structure with the filled information about the type.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_type_(struct lysc_ctx *ctx, struct lysp_node *context_node_p, uint16_t context_flags, struct lysp_module *context_mod, const char *context_name,
                  struct lysp_type *type_p, struct lys_module *module, LY_DATA_TYPE basetype, int options, const char *tpdfname,
                  struct lysc_type *base,  struct lysc_type **type)
{
    LY_ERR ret = LY_SUCCESS;
    unsigned int u, v, additional;
    struct lysc_type_bin *bin;
    struct lysc_type_num *num;
    struct lysc_type_str *str;
    struct lysc_type_bits *bits;
    struct lysc_type_enum *enumeration;
    struct lysc_type_dec *dec;
    struct lysc_type_identityref *idref;
    struct lysc_type_union *un, *un_aux;
    void *p;

    switch (basetype) {
    case LY_TYPE_BINARY:
        bin = (struct lysc_type_bin*)(*type);

        /* RFC 7950 9.8.1, 9.4.4 - length, number of octets it contains */
        if (type_p->length) {
            ret = lys_compile_type_range(ctx, type_p->length, basetype, 1, 0,
                                         base ? ((struct lysc_type_bin*)base)->length : NULL, &bin->length);
            LY_CHECK_RET(ret);
            if (!tpdfname) {
                COMPILE_ARRAY_GOTO(ctx, type_p->length->exts, bin->length->exts,
                                   options, u, lys_compile_ext, ret, done);
            }
        }

        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_bin));
        }
        break;
    case LY_TYPE_BITS:
        /* RFC 7950 9.7 - bits */
        bits = (struct lysc_type_bits*)(*type);
        if (type_p->bits) {
            ret = lys_compile_type_enums(ctx, type_p->bits, basetype, options,
                                         base ? (struct lysc_type_enum_item*)((struct lysc_type_bits*)base)->bits : NULL,
                                         (struct lysc_type_enum_item**)&bits->bits);
            LY_CHECK_RET(ret);
        }

        if (!base && !type_p->flags) {
            /* type derived from bits built-in type must contain at least one bit */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "bit", "bits type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "bit", "bits type", "");
                free(*type);
                *type = NULL;
            }
            return LY_EVALID;
        }

        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_bits));
        }
        break;
    case LY_TYPE_DEC64:
        dec = (struct lysc_type_dec*)(*type);

        /* RFC 7950 9.3.4 - fraction-digits */
        if (!base) {
            if (!type_p->fraction_digits) {
                if (tpdfname) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "fraction-digits", "decimal64 type ", tpdfname);
                } else {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "fraction-digits", "decimal64 type", "");
                    free(*type);
                    *type = NULL;
                }
                return LY_EVALID;
            }
        } else if (type_p->fraction_digits) {
            /* fraction digits is prohibited in types not directly derived from built-in decimal64 */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid fraction-digits substatement for type \"%s\" not directly derived from decimal64 built-in type.",
                       tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                       "Invalid fraction-digits substatement for type not directly derived from decimal64 built-in type.");
                free(*type);
                *type = NULL;
            }
            return LY_EVALID;
        }
        dec->fraction_digits = type_p->fraction_digits;

        /* RFC 7950 9.2.4 - range */
        if (type_p->range) {
            ret = lys_compile_type_range(ctx, type_p->range, basetype, 0, dec->fraction_digits,
                                         base ? ((struct lysc_type_dec*)base)->range : NULL, &dec->range);
            LY_CHECK_RET(ret);
            if (!tpdfname) {
                COMPILE_ARRAY_GOTO(ctx, type_p->range->exts, dec->range->exts,
                                   options, u, lys_compile_ext, ret, done);
            }
        }

        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_dec));
        }
        break;
    case LY_TYPE_STRING:
        str = (struct lysc_type_str*)(*type);

        /* RFC 7950 9.4.4 - length */
        if (type_p->length) {
            ret = lys_compile_type_range(ctx, type_p->length, basetype, 1, 0,
                                         base ? ((struct lysc_type_str*)base)->length : NULL, &str->length);
            LY_CHECK_RET(ret);
            if (!tpdfname) {
                COMPILE_ARRAY_GOTO(ctx, type_p->length->exts, str->length->exts,
                                   options, u, lys_compile_ext, ret, done);
            }
        } else if (base && ((struct lysc_type_str*)base)->length) {
            str->length = lysc_range_dup(ctx->ctx, ((struct lysc_type_str*)base)->length);
        }

        /* RFC 7950 9.4.5 - pattern */
        if (type_p->patterns) {
            ret = lys_compile_type_patterns(ctx, type_p->patterns, options,
                                            base ? ((struct lysc_type_str*)base)->patterns : NULL, &str->patterns);
            LY_CHECK_RET(ret);
        } else if (base && ((struct lysc_type_str*)base)->patterns) {
            str->patterns = lysc_patterns_dup(ctx->ctx, ((struct lysc_type_str*)base)->patterns);
        }

        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_str));
        }
        break;
    case LY_TYPE_ENUM:
        enumeration = (struct lysc_type_enum*)(*type);

        /* RFC 7950 9.6 - enum */
        if (type_p->enums) {
            ret = lys_compile_type_enums(ctx, type_p->enums, basetype, options,
                                         base ? ((struct lysc_type_enum*)base)->enums : NULL, &enumeration->enums);
            LY_CHECK_RET(ret);
        }

        if (!base && !type_p->flags) {
            /* type derived from enumerations built-in type must contain at least one enum */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "enum", "enumeration type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "enum", "enumeration type", "");
                free(*type);
                *type = NULL;
            }
            return LY_EVALID;
        }

        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_enum));
        }
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        num = (struct lysc_type_num*)(*type);

        /* RFC 6020 9.2.4 - range */
        if (type_p->range) {
            ret = lys_compile_type_range(ctx, type_p->range, basetype, 0, 0,
                                         base ? ((struct lysc_type_num*)base)->range : NULL, &num->range);
            LY_CHECK_RET(ret);
            if (!tpdfname) {
                COMPILE_ARRAY_GOTO(ctx, type_p->range->exts, num->range->exts,
                                   options, u, lys_compile_ext, ret, done);
            }
        }

        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_num));
        }
        break;
    case LY_TYPE_IDENT:
        idref = (struct lysc_type_identityref*)(*type);

        /* RFC 7950 9.10.2 - base */
        if (type_p->bases) {
            if (base) {
                /* only the directly derived identityrefs can contain base specification */
                if (tpdfname) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                           "Invalid base substatement for the type \"%s\" not directly derived from identityref built-in type.",
                           tpdfname);
                } else {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                           "Invalid base substatement for the type not directly derived from identityref built-in type.");
                    free(*type);
                    *type = NULL;
                }
                return LY_EVALID;
            }
            ret = lys_compile_identity_bases(ctx, type_p->bases, NULL, &idref->bases);
            LY_CHECK_RET(ret);
        }

        if (!base && !type_p->flags) {
            /* type derived from identityref built-in type must contain at least one base */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "base", "identityref type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "base", "identityref type", "");
                free(*type);
                *type = NULL;
            }
            return LY_EVALID;
        }

        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_identityref));
        }
        break;
    case LY_TYPE_LEAFREF:
        /* RFC 7950 9.9.3 - require-instance */
        if (type_p->flags & LYS_SET_REQINST) {
            if (context_mod->mod->version < LYS_VERSION_1_1) {
                if (tpdfname) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                           "Leafref type \"%s\" can be restricted by require-instance statement only in YANG 1.1 modules.", tpdfname);
                } else {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                           "Leafref type can be restricted by require-instance statement only in YANG 1.1 modules.");
                    free(*type);
                    *type = NULL;
                }
                return LY_EVALID;
            }
            ((struct lysc_type_leafref*)(*type))->require_instance = type_p->require_instance;
        } else if (base) {
            /* inherit */
            ((struct lysc_type_leafref*)(*type))->require_instance = ((struct lysc_type_leafref*)base)->require_instance;
        } else {
            /* default is true */
            ((struct lysc_type_leafref*)(*type))->require_instance = 1;
        }
        if (type_p->path) {
            DUP_STRING(ctx->ctx, (void*)type_p->path, ((struct lysc_type_leafref*)(*type))->path);
            ((struct lysc_type_leafref*)(*type))->path_context = module;
        } else if (base) {
            DUP_STRING(ctx->ctx, ((struct lysc_type_leafref*)base)->path, ((struct lysc_type_leafref*)(*type))->path);
            ((struct lysc_type_leafref*)(*type))->path_context = ((struct lysc_type_leafref*)base)->path_context;
        } else if (tpdfname) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "path", "leafref type ", tpdfname);
            return LY_EVALID;
        } else {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "path", "leafref type", "");
            free(*type);
            *type = NULL;
            return LY_EVALID;
        }
        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_leafref));
        }
        break;
    case LY_TYPE_INST:
        /* RFC 7950 9.9.3 - require-instance */
        if (type_p->flags & LYS_SET_REQINST) {
            ((struct lysc_type_instanceid*)(*type))->require_instance = type_p->require_instance;
        } else {
            /* default is true */
            ((struct lysc_type_instanceid*)(*type))->require_instance = 1;
        }

        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_instanceid));
        }
        break;
    case LY_TYPE_UNION:
        un = (struct lysc_type_union*)(*type);

        /* RFC 7950 7.4 - type */
        if (type_p->types) {
            if (base) {
                /* only the directly derived union can contain types specification */
                if (tpdfname) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                           "Invalid type substatement for the type \"%s\" not directly derived from union built-in type.",
                           tpdfname);
                } else {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG,
                           "Invalid type substatement for the type not directly derived from union built-in type.");
                    free(*type);
                    *type = NULL;
                }
                return LY_EVALID;
            }
            /* compile the type */
            additional = 0;
            LY_ARRAY_CREATE_RET(ctx->ctx, un->types, LY_ARRAY_SIZE(type_p->types), LY_EVALID);
            for (u = 0; u < LY_ARRAY_SIZE(type_p->types); ++u) {
                ret = lys_compile_type(ctx, context_node_p, context_flags, context_mod, context_name, &type_p->types[u], options, &un->types[u + additional], NULL);
                if (un->types[u + additional]->basetype == LY_TYPE_UNION) {
                    /* add space for additional types from the union subtype */
                    un_aux = (struct lysc_type_union *)un->types[u + additional];
                    p = ly_realloc(((uint32_t*)(un->types) - 1), sizeof(uint32_t) + ((LY_ARRAY_SIZE(type_p->types) + additional + LY_ARRAY_SIZE(un_aux->types) - 1) * sizeof *(un->types)));
                    LY_CHECK_ERR_RET(!p, LOGMEM(ctx->ctx);lysc_type_free(ctx->ctx, (struct lysc_type*)un_aux), LY_EMEM);
                    un->types = (void*)((uint32_t*)(p) + 1);

                    /* copy subtypes of the subtype union */
                    for (v = 0; v < LY_ARRAY_SIZE(un_aux->types); ++v) {
                        if (un_aux->types[v]->basetype == LY_TYPE_LEAFREF) {
                            /* duplicate the whole structure because of the instance-specific path resolving for realtype */
                            un->types[u + additional] = calloc(1, sizeof(struct lysc_type_leafref));
                            LY_CHECK_ERR_RET(!un->types[u + additional], LOGMEM(ctx->ctx);lysc_type_free(ctx->ctx, (struct lysc_type*)un_aux), LY_EMEM);
                            ((struct lysc_type_leafref*)un->types[u + additional])->basetype = LY_TYPE_LEAFREF;
                            DUP_STRING(ctx->ctx, ((struct lysc_type_leafref*)un_aux->types[v])->path, ((struct lysc_type_leafref*)un->types[u + additional])->path);
                            ((struct lysc_type_leafref*)un->types[u + additional])->refcount = 1;
                            ((struct lysc_type_leafref*)un->types[u + additional])->require_instance = ((struct lysc_type_leafref*)un_aux->types[v])->require_instance;
                            ((struct lysc_type_leafref*)un->types[u + additional])->path_context = ((struct lysc_type_leafref*)un_aux->types[v])->path_context;
                            /* TODO extensions */

                        } else {
                            un->types[u + additional] = un_aux->types[v];
                            ++un_aux->types[v]->refcount;
                        }
                        ++additional;
                        LY_ARRAY_INCREMENT(un->types);
                    }
                    /* compensate u increment in main loop */
                    --additional;

                    /* free the replaced union subtype */
                    lysc_type_free(ctx->ctx, (struct lysc_type*)un_aux);
                } else {
                    LY_ARRAY_INCREMENT(un->types);
                }
                LY_CHECK_RET(ret);
            }
        }

        if (!base && !type_p->flags) {
            /* type derived from union built-in type must contain at least one type */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "type", "union type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_MISSCHILDSTMT, "type", "union type", "");
                free(*type);
                *type = NULL;
            }
            return LY_EVALID;
        }

        if (tpdfname) {
            type_p->compiled = *type;
            *type = calloc(1, sizeof(struct lysc_type_union));
        }
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
    case LY_TYPE_UNKNOWN: /* just to complete switch */
        break;
    }
    LY_CHECK_ERR_RET(!(*type), LOGMEM(ctx->ctx), LY_EMEM);
done:
    return ret;
}

/**
 * @brief Compile information about the leaf/leaf-list's type.
 * @param[in] ctx Compile context.
 * @param[in] context_node_p Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] context_flags Flags of the context node or the referencing typedef to correctly check status of referencing and referenced objects.
 * @param[in] context_mod Module of the context node or the referencing typedef to correctly check status of referencing and referenced objects.
 * @param[in] context_name Name of the context node or referencing typedef for logging.
 * @param[in] type_p Parsed type to compile.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[out] type Newly created (or reused with increased refcount) type structure with the filled information about the type.
 * @param[out] units Storage for inheriting units value from the typedefs the current type derives from.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_type(struct lysc_ctx *ctx, struct lysp_node *context_node_p, uint16_t context_flags, struct lysp_module *context_mod, const char *context_name,
                 struct lysp_type *type_p, int options, struct lysc_type **type, const char **units)
{
    LY_ERR ret = LY_SUCCESS;
    unsigned int u;
    int dummyloops = 0;
    struct type_context {
        const struct lysp_tpdf *tpdf;
        struct lysp_node *node;
        struct lysp_module *mod;
    } *tctx, *tctx_prev = NULL;
    LY_DATA_TYPE basetype = LY_TYPE_UNKNOWN;
    struct lysc_type *base = NULL, *prev_type;
    struct ly_set tpdf_chain = {0};
    const char *dflt = NULL;

    (*type) = NULL;

    tctx = calloc(1, sizeof *tctx);
    LY_CHECK_ERR_RET(!tctx, LOGMEM(ctx->ctx), LY_EMEM);
    for (ret = lysp_type_find(type_p->name, context_node_p, ctx->mod_def->parsed,
                             &basetype, &tctx->tpdf, &tctx->node, &tctx->mod);
            ret == LY_SUCCESS;
            ret = lysp_type_find(tctx_prev->tpdf->type.name, tctx_prev->node, tctx_prev->mod,
                                         &basetype, &tctx->tpdf, &tctx->node, &tctx->mod)) {
        if (basetype) {
            break;
        }

        /* check status */
        ret = lysc_check_status(ctx, context_flags, context_mod, context_name,
                                tctx->tpdf->flags, tctx->mod, tctx->node ? tctx->node->name : tctx->tpdf->name);
        LY_CHECK_ERR_GOTO(ret,  free(tctx), cleanup);

        if (units && !*units) {
            /* inherit units */
            DUP_STRING(ctx->ctx, tctx->tpdf->units, *units);
        }
        if (!dflt) {
            /* inherit default */
            dflt = tctx->tpdf->dflt;
        }
        if (dummyloops && (!units || *units) && dflt) {
            basetype = ((struct type_context*)tpdf_chain.objs[tpdf_chain.count - 1])->tpdf->type.compiled->basetype;
            break;
        }

        if (tctx->tpdf->type.compiled) {
            /* it is not necessary to continue, the rest of the chain was already compiled,
             * but we still may need to inherit default and units values, so start dummy loops */
            basetype = tctx->tpdf->type.compiled->basetype;
            ly_set_add(&tpdf_chain, tctx, LY_SET_OPT_USEASLIST);
            if ((units && !*units) || !dflt) {
                dummyloops = 1;
                goto preparenext;
            } else {
                tctx = NULL;
                break;
            }
        }

        /* store information for the following processing */
        ly_set_add(&tpdf_chain, tctx, LY_SET_OPT_USEASLIST);

preparenext:
        /* prepare next loop */
        tctx_prev = tctx;
        tctx = calloc(1, sizeof *tctx);
        LY_CHECK_ERR_RET(!tctx, LOGMEM(ctx->ctx), LY_EMEM);
    }
    free(tctx);

    /* allocate type according to the basetype */
    switch (basetype) {
    case LY_TYPE_BINARY:
        *type = calloc(1, sizeof(struct lysc_type_bin));
        break;
    case LY_TYPE_BITS:
        *type = calloc(1, sizeof(struct lysc_type_bits));
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        *type = calloc(1, sizeof(struct lysc_type));
        break;
    case LY_TYPE_DEC64:
        *type = calloc(1, sizeof(struct lysc_type_dec));
        break;
    case LY_TYPE_ENUM:
        *type = calloc(1, sizeof(struct lysc_type_enum));
        break;
    case LY_TYPE_IDENT:
        *type = calloc(1, sizeof(struct lysc_type_identityref));
        break;
    case LY_TYPE_INST:
        *type = calloc(1, sizeof(struct lysc_type_instanceid));
        break;
    case LY_TYPE_LEAFREF:
        *type = calloc(1, sizeof(struct lysc_type_leafref));
        break;
    case LY_TYPE_STRING:
        *type = calloc(1, sizeof(struct lysc_type_str));
        break;
    case LY_TYPE_UNION:
        *type = calloc(1, sizeof(struct lysc_type_union));
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        *type = calloc(1, sizeof(struct lysc_type_num));
        break;
    case LY_TYPE_UNKNOWN:
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
               "Referenced type \"%s\" not found.", tctx_prev ? tctx_prev->tpdf->type.name : type_p->name);
        ret = LY_EVALID;
        goto cleanup;
    }
    LY_CHECK_ERR_GOTO(!(*type), LOGMEM(ctx->ctx), cleanup);
    if (~type_substmt_map[basetype] & type_p->flags) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG, "Invalid type restrictions for %s type.",
               ly_data_type2str[basetype]);
        free(*type);
        (*type) = NULL;
        ret = LY_EVALID;
        goto cleanup;
    }

    /* get restrictions from the referred typedefs */
    for (u = tpdf_chain.count - 1; u + 1 > 0; --u) {
        tctx = (struct type_context*)tpdf_chain.objs[u];
        if (tctx->tpdf->type.compiled) {
            base = tctx->tpdf->type.compiled;
            continue;
        } else if (basetype != LY_TYPE_LEAFREF && (u != tpdf_chain.count - 1) && !(tctx->tpdf->type.flags)) {
            /* no change, just use the type information from the base */
            base = ((struct lysp_tpdf*)tctx->tpdf)->type.compiled = ((struct type_context*)tpdf_chain.objs[u + 1])->tpdf->type.compiled;
            ++base->refcount;
            continue;
        }

        ++(*type)->refcount;
        if (~type_substmt_map[basetype] & tctx->tpdf->type.flags) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SYNTAX_YANG, "Invalid type \"%s\" restriction(s) for %s type.",
                   tctx->tpdf->name, ly_data_type2str[basetype]);
            ret = LY_EVALID;
            goto cleanup;
        } else if (basetype == LY_TYPE_EMPTY && tctx->tpdf->dflt) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                   "Invalid type \"%s\" - \"empty\" type must not have a default value (%s).",
                   tctx->tpdf->name, tctx->tpdf->dflt);
            ret = LY_EVALID;
            goto cleanup;
        }

        (*type)->basetype = basetype;
        prev_type = *type;
        ret = lys_compile_type_(ctx, tctx->node, tctx->tpdf->flags, tctx->mod, tctx->tpdf->name, &((struct lysp_tpdf*)tctx->tpdf)->type,
                                basetype & (LY_TYPE_LEAFREF | LY_TYPE_UNION) ? lysp_find_module(ctx->ctx, tctx->mod) : NULL,
                                basetype, options, tctx->tpdf->name, base, type);
        LY_CHECK_GOTO(ret, cleanup);
        base = prev_type;
    }

    /* process the type definition in leaf */
    if (type_p->flags || !base || basetype == LY_TYPE_LEAFREF) {
        /* get restrictions from the node itself */
        (*type)->basetype = basetype;
        ++(*type)->refcount;
        ret = lys_compile_type_(ctx, context_node_p, context_flags, context_mod, context_name, type_p, ctx->mod_def, basetype, options, NULL, base, type);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        /* no specific restriction in leaf's type definition, copy from the base */
        free(*type);
        (*type) = base;
        ++(*type)->refcount;
    }
    if (!(*type)->dflt) {
        DUP_STRING(ctx->ctx, dflt, (*type)->dflt);
    }

    COMPILE_ARRAY_GOTO(ctx, type_p->exts, (*type)->exts, options, u, lys_compile_ext, ret, cleanup);

cleanup:
    ly_set_erase(&tpdf_chain, free);
    return ret;
}

static LY_ERR lys_compile_node(struct lysc_ctx *ctx, struct lysp_node *node_p, int options, struct lysc_node *parent, uint16_t uses_status);

/**
 * @brief Compile parsed container node information.
 * @param[in] ctx Compile context
 * @param[in] node_p Parsed container node.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the container-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_container(struct lysc_ctx *ctx, struct lysp_node *node_p, int options, struct lysc_node *node)
{
    struct lysp_node_container *cont_p = (struct lysp_node_container*)node_p;
    struct lysc_node_container *cont = (struct lysc_node_container*)node;
    struct lysp_node *child_p;
    unsigned int u;
    LY_ERR ret = LY_SUCCESS;

    if (cont_p->presence) {
        cont->flags |= LYS_PRESENCE;
    }

    LY_LIST_FOR(cont_p->child, child_p) {
        LY_CHECK_RET(lys_compile_node(ctx, child_p, options, node, 0));
    }

    COMPILE_ARRAY_GOTO(ctx, cont_p->musts, cont->musts, options, u, lys_compile_must, ret, done);
    //COMPILE_ARRAY_GOTO(ctx, cont_p->actions, cont->actions, options, u, lys_compile_action, ret, done);
    //COMPILE_ARRAY_GOTO(ctx, cont_p->notifs, cont->notifs, options, u, lys_compile_notif, ret, done);

done:
    return ret;
}

/**
 * @brief Compile parsed leaf node information.
 * @param[in] ctx Compile context
 * @param[in] node_p Parsed leaf node.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the leaf-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_leaf(struct lysc_ctx *ctx, struct lysp_node *node_p, int options, struct lysc_node *node)
{
    struct lysp_node_leaf *leaf_p = (struct lysp_node_leaf*)node_p;
    struct lysc_node_leaf *leaf = (struct lysc_node_leaf*)node;
    unsigned int u;
    LY_ERR ret = LY_SUCCESS;

    COMPILE_ARRAY_GOTO(ctx, leaf_p->musts, leaf->musts, options, u, lys_compile_must, ret, done);
    DUP_STRING(ctx->ctx, leaf_p->units, leaf->units);
    DUP_STRING(ctx->ctx, leaf_p->dflt, leaf->dflt);
    if (leaf->dflt) {
        leaf->flags |= LYS_SET_DFLT;
    }

    ret = lys_compile_type(ctx, node_p, node_p->flags, ctx->mod_def->parsed, node_p->name, &leaf_p->type, options, &leaf->type,
                           leaf->units ? NULL : &leaf->units);
    LY_CHECK_GOTO(ret, done);
    if (!leaf->dflt && !(leaf->flags & LYS_MAND_TRUE)) {
        DUP_STRING(ctx->ctx, leaf->type->dflt, leaf->dflt);
    }
    if (leaf->type->basetype == LY_TYPE_LEAFREF) {
        /* store to validate the path in the current context at the end of schema compiling when all the nodes are present */
        ly_set_add(&ctx->unres, leaf, 0);
    } else if (leaf->type->basetype == LY_TYPE_UNION) {
        LY_ARRAY_FOR(((struct lysc_type_union*)leaf->type)->types, u) {
            if (((struct lysc_type_union*)leaf->type)->types[u]->basetype == LY_TYPE_LEAFREF) {
                /* store to validate the path in the current context at the end of schema compiling when all the nodes are present */
                ly_set_add(&ctx->unres, leaf, 0);
            }
        }
    } else if (leaf->type->basetype == LY_TYPE_EMPTY && leaf_p->dflt) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
               "Leaf of type \"empty\" must not have a default value (%s).",leaf_p->dflt);
        return LY_EVALID;
    }

    /* TODO validate default value according to the type, possibly postpone the check when the leafref target is known */

done:
    return ret;
}

/**
 * @brief Compile parsed leaf-list node information.
 * @param[in] ctx Compile context
 * @param[in] node_p Parsed leaf-list node.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the leaf-list-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_leaflist(struct lysc_ctx *ctx, struct lysp_node *node_p, int options, struct lysc_node *node)
{
    struct lysp_node_leaflist *llist_p = (struct lysp_node_leaflist*)node_p;
    struct lysc_node_leaflist *llist = (struct lysc_node_leaflist*)node;
    unsigned int u, v;
    LY_ERR ret = LY_SUCCESS;

    COMPILE_ARRAY_GOTO(ctx, llist_p->musts, llist->musts, options, u, lys_compile_must, ret, done);
    DUP_STRING(ctx->ctx, llist_p->units, llist->units);

    if (llist_p->dflts) {
        LY_ARRAY_CREATE_GOTO(ctx->ctx, llist->dflts, LY_ARRAY_SIZE(llist_p->dflts), ret, done);
        LY_ARRAY_FOR(llist_p->dflts, u) {
            DUP_STRING(ctx->ctx, llist_p->dflts[u], llist->dflts[u]);
            LY_ARRAY_INCREMENT(llist->dflts);
        }
    }

    llist->min = llist_p->min;
    if (llist->min) {
        llist->flags |= LYS_MAND_TRUE;
    }
    llist->max = llist_p->max ? llist_p->max : (uint32_t)-1;

    ret = lys_compile_type(ctx, node_p, node_p->flags, ctx->mod_def->parsed, node_p->name, &llist_p->type, options, &llist->type,
                           llist->units ? NULL : &llist->units);
    LY_CHECK_GOTO(ret, done);
    if (llist->type->dflt && !llist->dflts && !llist->min) {
        LY_ARRAY_CREATE_GOTO(ctx->ctx, llist->dflts, 1, ret, done);
        DUP_STRING(ctx->ctx, llist->type->dflt, llist->dflts[0]);
        LY_ARRAY_INCREMENT(llist->dflts);
    }

    if (llist->type->basetype == LY_TYPE_LEAFREF) {
        /* store to validate the path in the current context at the end of schema compiling when all the nodes are present */
        ly_set_add(&ctx->unres, llist, 0);
    } else if (llist->type->basetype == LY_TYPE_UNION) {
        LY_ARRAY_FOR(((struct lysc_type_union*)llist->type)->types, u) {
            if (((struct lysc_type_union*)llist->type)->types[u]->basetype == LY_TYPE_LEAFREF) {
                /* store to validate the path in the current context at the end of schema compiling when all the nodes are present */
                ly_set_add(&ctx->unres, llist, 0);
            }
        }
    } else if (llist->type->basetype == LY_TYPE_EMPTY) {
        if (ctx->mod_def->version < LYS_VERSION_1_1) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                   "Leaf-list of type \"empty\" is allowed only in YANG 1.1 modules.");
            return LY_EVALID;
        } else if (llist_p->dflts) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                   "Leaf-list of type \"empty\" must not have a default value (%s).", llist_p->dflts[0]);
            return LY_EVALID;
        }
    }

    if ((llist->flags & LYS_CONFIG_W) && llist->dflts && LY_ARRAY_SIZE(llist->dflts)) {
        /* configuration data values must be unique - so check the default values */
        LY_ARRAY_FOR(llist->dflts, u) {
            for (v = u + 1; v < LY_ARRAY_SIZE(llist->dflts); ++v) {
                if (!strcmp(llist->dflts[u], llist->dflts[v])) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                           "Configuration leaf-list has multiple defaults of the same value \"%s\".", llist->dflts[v]);
                    return LY_EVALID;
                }
            }
        }
    }

    /* TODO validate default value according to the type, possibly postpone the check when the leafref target is known */

done:
    return ret;
}

/**
 * @brief Compile parsed list node information.
 * @param[in] ctx Compile context
 * @param[in] node_p Parsed list node.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the list-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_list(struct lysc_ctx *ctx, struct lysp_node *node_p, int options, struct lysc_node *node)
{
    struct lysp_node_list *list_p = (struct lysp_node_list*)node_p;
    struct lysc_node_list *list = (struct lysc_node_list*)node;
    struct lysp_node *child_p;
    struct lysc_node_leaf **key, ***unique;
    size_t len;
    unsigned int u, v;
    const char *keystr, *delim;
    int config;
    LY_ERR ret = LY_SUCCESS;

    list->min = list_p->min;
    if (list->min) {
        list->flags |= LYS_MAND_TRUE;
    }
    list->max = list_p->max ? list_p->max : (uint32_t)-1;

    LY_LIST_FOR(list_p->child, child_p) {
        LY_CHECK_RET(lys_compile_node(ctx, child_p, options, node, 0));
    }

    COMPILE_ARRAY_GOTO(ctx, list_p->musts, list->musts, options, u, lys_compile_must, ret, done);

    /* keys */
    if ((list->flags & LYS_CONFIG_W) && (!list_p->key || !list_p->key[0])) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS, "Missing key in list representing configuration data.");
        return LY_EVALID;
    }

    /* find all the keys (must be direct children) */
    keystr = list_p->key;
    while (keystr) {
        delim = strpbrk(keystr, " \t\n");
        if (delim) {
            len = delim - keystr;
            while (isspace(*delim)) {
                ++delim;
            }
        } else {
            len = strlen(keystr);
        }

        /* key node must be present */
        LY_ARRAY_NEW_RET(ctx->ctx, list->keys, key, LY_EMEM);
        *key = (struct lysc_node_leaf*)lys_child(node, node->module, keystr, len, LYS_LEAF, LYS_GETNEXT_NOCHOICE | LYS_GETNEXT_NOSTATECHECK);
        if (!(*key)) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "The list's key \"%.*s\" not found.", len, keystr);
            return LY_EVALID;
        }
        /* keys must be unique */
        for(u = 0; u < LY_ARRAY_SIZE(list->keys) - 1; ++u) {
            if (*key == list->keys[u]) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "Duplicated key identifier \"%.*s\".", len, keystr);
                return LY_EVALID;
            }
        }
        /* key must have the same config flag as the list itself */
        if ((list->flags & LYS_CONFIG_MASK) != ((*key)->flags & LYS_CONFIG_MASK)) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS, "Key of the configuration list must not be status leaf.");
            return LY_EVALID;
        }
        if (ctx->mod_def->version < LYS_VERSION_1_1) {
            /* YANG 1.0 denies key to be of empty type */
            if ((*key)->type->basetype == LY_TYPE_EMPTY) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "Key of a list can be of type \"empty\" only in YANG 1.1 modules.");
                return LY_EVALID;
            }
        } else {
            /* when and if-feature are illegal on list keys */
            if ((*key)->when) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "List's key \"%s\" must not have any \"when\" statement.", (*key)->name);
                return LY_EVALID;
            }
            if ((*key)->iffeatures) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "List's key \"%s\" must not have any \"if-feature\" statement.", (*key)->name);
                return LY_EVALID;
            }
        }

        /* check status */
        LY_CHECK_RET(lysc_check_status(ctx, list->flags, list->module, list->name,
                                       (*key)->flags, (*key)->module, (*key)->name));

        /* ignore default values of the key */
        if ((*key)->dflt) {
            lydict_remove(ctx->ctx, (*key)->dflt);
            (*key)->dflt = NULL;
        }
        /* mark leaf as key */
        (*key)->flags |= LYS_KEY;

        /* next key value */
        keystr = delim;
    }

    /* uniques */
    if (list_p->uniques) {
        for (v = 0; v < LY_ARRAY_SIZE(list_p->uniques); ++v) {
            config = -1;
            LY_ARRAY_NEW_RET(ctx->ctx, list->uniques, unique, LY_EMEM);
            keystr = list_p->uniques[v];
            while (keystr) {
                delim = strpbrk(keystr, " \t\n");
                if (delim) {
                    len = delim - keystr;
                    while (isspace(*delim)) {
                        ++delim;
                    }
                } else {
                    len = strlen(keystr);
                }

                /* unique node must be present */
                LY_ARRAY_NEW_RET(ctx->ctx, *unique, key, LY_EMEM);
                ret = lys_resolve_schema_nodeid(ctx, keystr, len, node, LYS_LEAF, 0, (const struct lysc_node**)key);
                if (ret != LY_SUCCESS) {
                    if (ret == LY_EDENIED) {
                        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                               "Unique's descendant-schema-nodeid \"%.*s\" refers to a %s node instead of a leaf.",
                               len, keystr, lys_nodetype2str((*key)->nodetype));
                    }
                    return LY_EVALID;
                }

                /* all referenced leafs must be of the same config type */
                if (config != -1 && ((((*key)->flags & LYS_CONFIG_W) && config == 0) || (((*key)->flags & LYS_CONFIG_R) && config == 1))) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                           "Unique statement \"%s\" refers to leafs with different config type.", list_p->uniques[v]);
                    return LY_EVALID;
                } else if ((*key)->flags & LYS_CONFIG_W) {
                    config = 1;
                } else { /* LYS_CONFIG_R */
                    config = 0;
                }

                /* check status */
                LY_CHECK_RET(lysc_check_status(ctx, list->flags, list->module, list->name,
                                               (*key)->flags, (*key)->module, (*key)->name));

                /* mark leaf as unique */
                (*key)->flags |= LYS_UNIQUE;

                /* next unique value in line */
                keystr = delim;
            }
            /* next unique definition */
        }
    }

    //COMPILE_ARRAY_GOTO(ctx, list_p->actions, list->actions, options, u, lys_compile_action, ret, done);
    //COMPILE_ARRAY_GOTO(ctx, list_p->notifs, list->notifs, options, u, lys_compile_notif, ret, done);

done:
    return ret;
}

static LY_ERR
lys_compile_node_choice_dflt(struct lysc_ctx *ctx, const char *dflt, struct lysc_node_choice *ch)
{
    struct lysc_node *iter, *node = (struct lysc_node*)ch;
    const char *prefix = NULL, *name;
    size_t prefix_len = 0;

    /* could use lys_parse_nodeid(), but it checks syntax which is already done in this case by the parsers */
    name = strchr(dflt, ':');
    if (name) {
        prefix = dflt;
        prefix_len = name - prefix;
        ++name;
    } else {
        name = dflt;
    }
    if (prefix && (strncmp(prefix, node->module->prefix, prefix_len) || node->module->prefix[prefix_len] != '\0')) {
        /* prefixed default case make sense only for the prefix of the schema itself */
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
               "Invalid default case referencing a case from different YANG module (by prefix \"%.*s\").",
               prefix_len, prefix);
        return LY_EVALID;
    }
    ch->dflt = (struct lysc_node_case*)lys_child(node, node->module, name, 0, LYS_CASE, LYS_GETNEXT_NOSTATECHECK | LYS_GETNEXT_WITHCASE);
    if (!ch->dflt) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
               "Default case \"%s\" not found.", dflt);
        return LY_EVALID;
    }
    /* no mandatory nodes directly under the default case */
    LY_LIST_FOR(ch->dflt->child, iter) {
        if (iter->flags & LYS_MAND_TRUE) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                   "Mandatory node \"%s\" under the default case \"%s\".", iter->name, dflt);
            return LY_EVALID;
        }
    }
    ch->dflt->flags |= LYS_SET_DFLT;
    return LY_SUCCESS;
}

/**
 * @brief Compile parsed choice node information.
 * @param[in] ctx Compile context
 * @param[in] node_p Parsed choice node.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the choice-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_choice(struct lysc_ctx *ctx, struct lysp_node *node_p, int options, struct lysc_node *node)
{
    struct lysp_node_choice *ch_p = (struct lysp_node_choice*)node_p;
    struct lysc_node_choice *ch = (struct lysc_node_choice*)node;
    struct lysp_node *child_p, *case_child_p;
    struct lys_module;
    LY_ERR ret = LY_SUCCESS;

    LY_LIST_FOR(ch_p->child, child_p) {
        if (child_p->nodetype == LYS_CASE) {
            LY_LIST_FOR(((struct lysp_node_case*)child_p)->child, case_child_p) {
                LY_CHECK_RET(lys_compile_node(ctx, case_child_p, options, node, 0));
            }
        } else {
            LY_CHECK_RET(lys_compile_node(ctx, child_p, options, node, 0));
        }
    }

    /* default branch */
    if (ch_p->dflt) {
        LY_CHECK_RET(lys_compile_node_choice_dflt(ctx, ch_p->dflt, ch));
    }

    return ret;
}

/**
 * @brief Compile parsed anydata or anyxml node information.
 * @param[in] ctx Compile context
 * @param[in] node_p Parsed anydata or anyxml node.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the any-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_any(struct lysc_ctx *ctx, struct lysp_node *node_p, int options, struct lysc_node *node)
{
    struct lysp_node_anydata *any_p = (struct lysp_node_anydata*)node_p;
    struct lysc_node_anydata *any = (struct lysc_node_anydata*)node;
    unsigned int u;
    LY_ERR ret = LY_SUCCESS;

    COMPILE_ARRAY_GOTO(ctx, any_p->musts, any->musts, options, u, lys_compile_must, ret, done);

    if (any->flags & LYS_CONFIG_W) {
        LOGWRN(ctx->ctx, "Use of %s to define configuration data is not recommended.",
               ly_stmt2str(any->nodetype == LYS_ANYDATA ? YANG_ANYDATA : YANG_ANYXML));
    }
done:
    return ret;
}

static LY_ERR
lys_compile_status_check(struct lysc_ctx *ctx, uint16_t node_flags, uint16_t parent_flags)
{
    /* check status compatibility with the parent */
    if ((parent_flags & LYS_STATUS_MASK) > (node_flags & LYS_STATUS_MASK)) {
        if (node_flags & LYS_STATUS_CURR) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                   "A \"current\" status is in conflict with the parent's \"%s\" status.",
                   (parent_flags & LYS_STATUS_DEPRC) ? "deprecated" : "obsolete");
        } else { /* LYS_STATUS_DEPRC */
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                   "A \"deprecated\" status is in conflict with the parent's \"obsolete\" status.");
        }
        return LY_EVALID;
    }
    return LY_SUCCESS;
}

static LY_ERR
lys_compile_status(struct lysc_ctx *ctx, struct lysc_node *node, uint16_t parent_flags)
{
    /* status - it is not inherited by specification, but it does not make sense to have
     * current in deprecated or deprecated in obsolete, so we do print warning and inherit status */
    if (!(node->flags & LYS_STATUS_MASK)) {
        if (parent_flags & (LYS_STATUS_DEPRC | LYS_STATUS_OBSLT)) {
            if ((parent_flags & 0x3) != 0x3) {
                /* do not print the warning when inheriting status from uses - the uses_status value has a special
                 * combination of bits (0x3) which marks the uses_status value */
                LOGWRN(ctx->ctx, "Missing explicit \"%s\" status that was already specified in parent, inheriting.",
                       (parent_flags & LYS_STATUS_DEPRC) ? "deprecated" : "obsolete");
            }
            node->flags |= parent_flags & LYS_STATUS_MASK;
        } else {
            node->flags |= LYS_STATUS_CURR;
        }
    } else if (parent_flags & LYS_STATUS_MASK) {
        return lys_compile_status_check(ctx, node->flags, parent_flags);
    }
    return LY_SUCCESS;
}

static LY_ERR
lys_compile_node_uniqness(struct lysc_ctx *ctx, const struct lysc_node *children,
                          const struct lysc_action *actions, const struct lysc_notif *notifs,
                          const char *name, void *exclude)
{
    const struct lysc_node *iter;
    unsigned int u;

    LY_LIST_FOR(children, iter) {
        if (iter != exclude && iter->module == ctx->mod && !strcmp(name, iter->name)) {
            goto error;
        }
    }
    LY_ARRAY_FOR(actions, u) {
        if (&actions[u] != exclude && actions[u].module == ctx->mod && !strcmp(name, actions[u].name)) {
            goto error;
        }
    }
    LY_ARRAY_FOR(notifs, u) {
        if (&notifs[u] != exclude && notifs[u].module == ctx->mod && !strcmp(name, notifs[u].name)) {
            goto error;
        }
    }
    return LY_SUCCESS;
error:
    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_DUPIDENT, name, "data definition");
    return LY_EEXIST;
}

/**
 * @brief Connect the node into the siblings list and check its name uniqueness.
 *
 * @param[in] ctx Compile context
 * @param[in] parent Parent node holding the children list, in case of node from a choice's case,
 * the choice itself is expected instead of a specific case node.
 * @param[in] node Schema node to connect into the list.
 * @return LY_ERR value - LY_SUCCESS or LY_EEXIST.
 */
static LY_ERR
lys_compile_node_connect(struct lysc_ctx *ctx, struct lysc_node *parent, struct lysc_node *node)
{
    struct lysc_node **children;

    if (node->nodetype == LYS_CASE) {
        children = (struct lysc_node**)&((struct lysc_node_choice*)parent)->cases;
    } else {
        children = lysc_node_children_p(parent);
    }
    if (children) {
        if (!(*children)) {
            /* first child */
            *children = node;
        } else if (*children != node) {
            /* by the condition in previous branch we cover the choice/case children
             * - the children list is shared by the choice and the the first case, in addition
             * the first child of each case must be referenced from the case node. So the node is
             * actually always already inserted in case it is the first children - so here such
             * a situation actually corresponds to the first branch */
            /* insert at the end of the parent's children list */
            (*children)->prev->next = node;
            node->prev = (*children)->prev;
            (*children)->prev = node;

            /* check the name uniqueness */
            if (lys_compile_node_uniqness(ctx, *children, lysc_node_actions(parent),
                                          lysc_node_notifs(parent), node->name, node)) {
                return LY_EEXIST;
            }
        }
    }
    return LY_SUCCESS;
}

/**
 * @param[in] node_p Node image from the parsed tree. If the case is explicit, it is the LYS_CASE node, but in case of implicit case,
 *                   it is the LYS_CHOICE node or LYS_AUGMENT node.
 */
static struct lysc_node_case*
lys_compile_node_case(struct lysc_ctx *ctx, struct lysp_node *node_p, int options, struct lysc_node_choice *ch, struct lysc_node *child)
{
    struct lysc_node *iter;
    struct lysc_node_case *cs;
    struct lysc_when **when;
    unsigned int u;
    LY_ERR ret;

#define UNIQUE_CHECK(NAME, MOD) \
    LY_LIST_FOR((struct lysc_node*)ch->cases, iter) { \
        if (iter->module == MOD && !strcmp(iter->name, NAME)) { \
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LY_VCODE_DUPIDENT, NAME, "case"); \
            return NULL; \
        } \
    }

    if (node_p->nodetype == LYS_CHOICE || node_p->nodetype == LYS_AUGMENT) {
        UNIQUE_CHECK(child->name, ctx->mod);

        /* we have to add an implicit case node into the parent choice */
        cs = calloc(1, sizeof(struct lysc_node_case));
        DUP_STRING(ctx->ctx, child->name, cs->name);
        cs->flags = ch->flags & LYS_STATUS_MASK;
    } else if (node_p->nodetype == LYS_CASE) {
        if (ch->cases && (node_p == ch->cases->prev->sp)) {
            /* the case is already present since the child is not its first children */
            return (struct lysc_node_case*)ch->cases->prev;
        }
        UNIQUE_CHECK(node_p->name, ctx->mod);

        /* explicit parent case is not present (this is its first child) */
        cs = calloc(1, sizeof(struct lysc_node_case));
        DUP_STRING(ctx->ctx, node_p->name, cs->name);
        cs->flags = LYS_STATUS_MASK & node_p->flags;
        cs->sp = node_p;

        /* check the case's status (don't need to solve uses_status since case statement cannot be directly in grouping statement */
        LY_CHECK_RET(lys_compile_status(ctx, (struct lysc_node*)cs, ch->flags), NULL);

        if (node_p->when) {
            LY_ARRAY_NEW_GOTO(ctx->ctx, cs->when, when, ret, error);
            ret = lys_compile_when(ctx, node_p->when, options, when);
            LY_CHECK_GOTO(ret, error);
            (*when)->context = lys_compile_xpath_context(ch->parent);
        }
        COMPILE_ARRAY_GOTO(ctx, node_p->iffeatures, cs->iffeatures, options, u, lys_compile_iffeature, ret, error);
    } else {
        LOGINT(ctx->ctx);
        goto error;
    }
    cs->module = ctx->mod;
    cs->prev = (struct lysc_node*)cs;
    cs->nodetype = LYS_CASE;
    lys_compile_node_connect(ctx, (struct lysc_node*)ch, (struct lysc_node*)cs);
    cs->parent = (struct lysc_node*)ch;
    cs->child = child;

    return cs;
error:
    return NULL;

#undef UNIQUE_CHECK
}

static LY_ERR
lys_compile_refine_config(struct lysc_ctx *ctx, struct lysc_node *node, struct lysp_refine *rfn, int inheriting)
{
    struct lysc_node *child;
    uint16_t config = rfn->flags & LYS_CONFIG_MASK;

    if (config == (node->flags & LYS_CONFIG_MASK)) {
        /* nothing to do */
        return LY_SUCCESS;
    }

    if (!inheriting) {
        /* explicit refine */
        if (config == LYS_CONFIG_W && node->parent && (node->parent->flags & LYS_CONFIG_R)) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                   "Invalid refine of config in \"%s\" - configuration node cannot be child of any state data node.",
                   rfn->nodeid);
            return LY_EVALID;
        }
    }
    node->flags &= ~LYS_CONFIG_MASK;
    node->flags |= config;

    /* inherit the change into the children */
    LY_LIST_FOR((struct lysc_node*)lysc_node_children(node), child) {
        LY_CHECK_RET(lys_compile_refine_config(ctx, child, rfn, 1));
    }

    /* TODO actions and notifications */

    return LY_SUCCESS;
}

void
lys_compile_mandatory_parents(struct lysc_node *parent, int add)
{
    struct lysc_node *iter;

    if (add) { /* set flag */
        for (; parent &&  parent->nodetype == LYS_CONTAINER && !(parent->flags & LYS_MAND_TRUE) && !(parent->flags & LYS_PRESENCE);
                parent = parent->parent) {
            parent->flags |= LYS_MAND_TRUE;
        }
    } else { /* unset flag */
        for (; parent && parent->nodetype == LYS_CONTAINER && (parent->flags & LYS_MAND_TRUE); parent = parent->parent) {
            for (iter = (struct lysc_node*)lysc_node_children(parent); iter; iter = iter->next) {
                if (iter->flags && LYS_MAND_TRUE) {
                    /* there is another mandatory node */
                    return;
                }
            }
            /* unset mandatory flag - there is no mandatory children in the non-presence container */
            parent->flags &= ~LYS_MAND_TRUE;
        }
    }
}

/**
 * @brief Compile parsed uses statement - resolve target grouping and connect its content into parent.
 * If present, also apply uses's modificators.
 *
 * @param[in] ctx Compile context
 * @param[in] uses_p Parsed uses schema node.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in] parent Compiled parent node where the content of the referenced grouping is supposed to be connected. It is
 * NULL for top-level nodes, in such a case the module where the node will be connected is taken from
 * the compile context.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_uses(struct lysc_ctx *ctx, struct lysp_node_uses *uses_p, int options, struct lysc_node *parent)
{
    struct lysp_node *node_p;
    struct lysc_node *node, *child;
    /* context_node_fake allows us to temporarily isolate the nodes inserted from the grouping instead of uses */
    struct lysc_node_container context_node_fake =
        {.nodetype = LYS_CONTAINER,
         .module = ctx->mod,
         .flags = parent ? parent->flags : 0,
         .child = NULL, .next = NULL,
         .prev = (struct lysc_node*)&context_node_fake};
    const struct lysp_grp *grp = NULL;
    unsigned int u, v, grp_stack_count;
    int found;
    const char *id, *name, *prefix;
    size_t prefix_len, name_len;
    struct lys_module *mod, *mod_old;
    struct lysp_refine *rfn;
    LY_ERR ret = LY_EVALID;
    uint32_t min, max;
    struct ly_set refined = {0};
    struct lysc_when **when, *when_shared;

    /* search for the grouping definition */
    found = 0;
    id = uses_p->name;
    lys_parse_nodeid(&id, &prefix, &prefix_len, &name, &name_len);
    if (prefix) {
        mod = lys_module_find_prefix(ctx->mod_def, prefix, prefix_len);
        if (!mod) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid prefix used for grouping reference (%s).", uses_p->name);
            return LY_EVALID;
        }
    } else {
        mod = ctx->mod_def;
    }
    if (mod == ctx->mod_def) {
        for (node_p = uses_p->parent; !found && node_p; node_p = node_p->parent) {
            grp = lysp_node_groupings(node_p);
            LY_ARRAY_FOR(grp, u) {
                if (!strcmp(grp[u].name, name)) {
                    grp = &grp[u];
                    found = 1;
                    break;
                }
            }
        }
    }
    if (!found) {
        /* search in top-level groupings of the main module ... */
        grp = mod->parsed->groupings;
        if (grp) {
            for (u = 0; !found && u < LY_ARRAY_SIZE(grp); ++u) {
                if (!strcmp(grp[u].name, name)) {
                    grp = &grp[u];
                    found = 1;
                }
            }
        }
        if (!found && mod->parsed->includes) {
            /* ... and all the submodules */
            for (u = 0; !found && u < LY_ARRAY_SIZE(mod->parsed->includes); ++u) {
                grp = mod->parsed->includes[u].submodule->groupings;
                if (grp) {
                    for (v = 0; !found && v < LY_ARRAY_SIZE(grp); ++v) {
                        if (!strcmp(grp[v].name, name)) {
                            grp = &grp[v];
                            found = 1;
                        }
                    }
                }
            }
        }
    }
    if (!found) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
               "Grouping \"%s\" referenced by a uses statement not found.", uses_p->name);
        return LY_EVALID;
    }

    /* grouping must not reference themselves - stack in ctx maintains list of groupings currently being applied */
    grp_stack_count = ctx->groupings.count;
    ly_set_add(&ctx->groupings, (void*)grp, 0);
    if (grp_stack_count == ctx->groupings.count) {
        /* the target grouping is already in the stack, so we are already inside it -> circular dependency */
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
               "Grouping \"%s\" references itself through a uses statement.", grp->name);
        return LY_EVALID;
    }

    /* switch context's mod_def */
    mod_old = ctx->mod_def;
    ctx->mod_def = mod;

    /* check status */
    LY_CHECK_GOTO(lysc_check_status(ctx, uses_p->flags, mod_old, uses_p->name, grp->flags, mod, grp->name), error);

    LY_LIST_FOR(grp->data, node_p) {
        /* 0x3 in uses_status is a special bits combination to be able to detect status flags from uses */
        LY_CHECK_GOTO(lys_compile_node(ctx, node_p, options, parent, (uses_p->flags & LYS_STATUS_MASK) | 0x3), error);
        child = parent ? lysc_node_children(parent)->prev : ctx->mod->compiled->data->prev;

        /* some preparation for applying refines */
        if (grp->data == node_p) {
            /* remember the first child */
            context_node_fake.child = child;
        }
    }
    when_shared = NULL;
    LY_LIST_FOR(context_node_fake.child, child) {
        child->parent = (struct lysc_node*)&context_node_fake;

        /* pass uses's when to all the children */
        if (uses_p->when) {
            LY_ARRAY_NEW_GOTO(ctx->ctx, child->when, when, ret, error);
            if (!when_shared) {
                ret = lys_compile_when(ctx, uses_p->when, options, when);
                LY_CHECK_GOTO(ret, error);
                (*when)->context = lys_compile_xpath_context(parent);
                when_shared = *when;
            } else {
                ++when_shared->refcount;
                (*when) = when_shared;
            }
        }
    }
    if (context_node_fake.child) {
        child = context_node_fake.child->prev;
        context_node_fake.child->prev = parent ? lysc_node_children(parent)->prev : ctx->mod->compiled->data->prev;
    }

    /* TODO: apply augment */

    /* reload previous context's mod_def */
    ctx->mod_def = mod_old;

    /* apply refine */
    LY_ARRAY_FOR(uses_p->refines, struct lysp_refine, rfn) {
        LY_CHECK_GOTO(lys_resolve_schema_nodeid(ctx, rfn->nodeid, 0, (struct lysc_node*)&context_node_fake, 0, 0, (const struct lysc_node**)&node),
                      error);
        ly_set_add(&refined, node, LY_SET_OPT_USEASLIST);

        /* default value */
        if (rfn->dflts) {
            if ((node->nodetype != LYS_LEAFLIST) && LY_ARRAY_SIZE(rfn->dflts) > 1) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "Invalid refine of default in \"%s\" - %s cannot hold %d default values.",
                       rfn->nodeid, lys_nodetype2str(node->nodetype), LY_ARRAY_SIZE(rfn->dflts));
                goto error;
            }
            if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE))) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "Invalid refine of default in \"%s\" - %s cannot hold default value(s).",
                       rfn->nodeid, lys_nodetype2str(node->nodetype));
                goto error;
            }
            if (node->nodetype == LYS_LEAF) {
                FREE_STRING(ctx->ctx, ((struct lysc_node_leaf*)node)->dflt);
                DUP_STRING(ctx->ctx, rfn->dflts[0], ((struct lysc_node_leaf*)node)->dflt);
                node->flags |= LYS_SET_DFLT;
                /* TODO check the default value according to type */
            } else if (node->nodetype == LYS_LEAFLIST) {
                if (ctx->mod->version < 2) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                           "Invalid refine of default in leaf-list - the default statement is allowed only in YANG 1.1 modules.");
                    goto error;
                }
                LY_ARRAY_FOR(((struct lysc_node_leaflist*)node)->dflts, u) {
                    lydict_remove(ctx->ctx, ((struct lysc_node_leaflist*)node)->dflts[u]);
                }
                LY_ARRAY_FREE(((struct lysc_node_leaflist*)node)->dflts);
                ((struct lysc_node_leaflist*)node)->dflts = NULL;
                LY_ARRAY_CREATE_GOTO(ctx->ctx, ((struct lysc_node_leaflist*)node)->dflts, LY_ARRAY_SIZE(rfn->dflts), ret, error);
                LY_ARRAY_FOR(rfn->dflts, u) {
                    LY_ARRAY_INCREMENT(((struct lysc_node_leaflist*)node)->dflts);
                    DUP_STRING(ctx->ctx, rfn->dflts[u], ((struct lysc_node_leaflist*)node)->dflts[u]);
                }
                /* TODO check the default values according to type */
            } else if (node->nodetype == LYS_CHOICE) {
                if (((struct lysc_node_choice*)node)->dflt) {
                    /* unset LYS_SET_DFLT from the current default case */
                    ((struct lysc_node_choice*)node)->dflt->flags &= ~LYS_SET_DFLT;
                }
                LY_CHECK_GOTO(lys_compile_node_choice_dflt(ctx, rfn->dflts[0], (struct lysc_node_choice*)node), error);
            }
        }

        /* description */
        if (rfn->dsc) {
            FREE_STRING(ctx->ctx, node->dsc);
            node->dsc = lydict_insert(ctx->ctx, rfn->dsc, 0);
        }

        /* reference */
        if (rfn->ref) {
            FREE_STRING(ctx->ctx, node->ref);
            node->ref = lydict_insert(ctx->ctx, rfn->ref, 0);
        }

        /* config */
        if (rfn->flags & LYS_CONFIG_MASK) {
            LY_CHECK_GOTO(lys_compile_refine_config(ctx, node, rfn, 0), error);
        }

        /* mandatory */
        if (rfn->flags & LYS_MAND_MASK) {
            if (!(node->nodetype & (LYS_LEAF | LYS_ANYDATA | LYS_ANYXML | LYS_CHOICE))) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "Invalid refine of mandatory in \"%s\" - %s cannot hold mandatory statement.",
                       rfn->nodeid, lys_nodetype2str(node->nodetype));
                goto error;
            }
            /* in compiled flags, only the LYS_MAND_TRUE is present */
            if (rfn->flags & LYS_MAND_TRUE) {
                /* check if node has default value */
                if (node->nodetype & LYS_LEAF) {
                    if (node->flags & LYS_SET_DFLT) {
                        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                               "Invalid refine of mandatory in \"%s\" - leaf already has \"default\" statement.", rfn->nodeid);
                        goto error;
                    } else {
                        /* remove the default value taken from the leaf's type */
                        FREE_STRING(ctx->ctx, ((struct lysc_node_leaf*)node)->dflt);
                        ((struct lysc_node_leaf*)node)->dflt = NULL;
                    }
                } else if ((node->nodetype & LYS_CHOICE) && ((struct lysc_node_choice*)node)->dflt) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                           "Invalid refine of mandatory in \"%s\" - choice already has \"default\" statement.", rfn->nodeid);
                    goto error;
                }
                if (node->parent && (node->parent->flags & LYS_SET_DFLT)) {
                    LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                           "Invalid refine of mandatory in \"%s\" - %s under the default case.",
                           rfn->nodeid, lys_nodetype2str(node->nodetype));
                    goto error;
                }

                node->flags |= LYS_MAND_TRUE;
                lys_compile_mandatory_parents(node->parent, 1);
            } else {
                /* make mandatory false */
                node->flags &= ~LYS_MAND_TRUE;
                lys_compile_mandatory_parents(node->parent, 0);
                if ((node->nodetype & LYS_LEAF) && !((struct lysc_node_leaf*)node)->dflt) {
                    /* get the type's default value if any */
                    DUP_STRING(ctx->ctx, ((struct lysc_node_leaf*)node)->type->dflt, ((struct lysc_node_leaf*)node)->dflt);
                }
            }
        }

        /* presence */
        if (rfn->presence) {
            if (node->nodetype != LYS_CONTAINER) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "Invalid refine of presence statement in \"%s\" - %s cannot hold the presence statement.",
                       rfn->nodeid, lys_nodetype2str(node->nodetype));
                goto error;
            }
            node->flags |= LYS_PRESENCE;
        }

        /* must */
        if (rfn->musts) {
            switch (node->nodetype) {
            case LYS_LEAF:
                COMPILE_ARRAY_GOTO(ctx, rfn->musts, ((struct lysc_node_leaf*)node)->musts, options, u, lys_compile_must, ret, error);
                break;
            case LYS_LEAFLIST:
                COMPILE_ARRAY_GOTO(ctx, rfn->musts, ((struct lysc_node_leaflist*)node)->musts, options, u, lys_compile_must, ret, error);
                break;
            case LYS_LIST:
                COMPILE_ARRAY_GOTO(ctx, rfn->musts, ((struct lysc_node_list*)node)->musts, options, u, lys_compile_must, ret, error);
                break;
            case LYS_CONTAINER:
                COMPILE_ARRAY_GOTO(ctx, rfn->musts, ((struct lysc_node_container*)node)->musts, options, u, lys_compile_must, ret, error);
                break;
            case LYS_ANYXML:
            case LYS_ANYDATA:
                COMPILE_ARRAY_GOTO(ctx, rfn->musts, ((struct lysc_node_anydata*)node)->musts, options, u, lys_compile_must, ret, error);
                break;
            default:
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "Invalid refine of must statement in \"%s\" - %s cannot hold any must statement.",
                       rfn->nodeid, lys_nodetype2str(node->nodetype));
                goto error;
            }
        }

        /* min/max-elements */
        if (rfn->flags & (LYS_SET_MAX | LYS_SET_MIN)) {
            switch (node->nodetype) {
            case LYS_LEAFLIST:
                if (rfn->flags & LYS_SET_MAX) {
                    ((struct lysc_node_leaflist*)node)->max = rfn->max ? rfn->max : (uint32_t)-1;
                }
                if (rfn->flags & LYS_SET_MIN) {
                    ((struct lysc_node_leaflist*)node)->min = rfn->min;
                    if (rfn->min) {
                        node->flags |= LYS_MAND_TRUE;
                        lys_compile_mandatory_parents(node->parent, 1);
                    } else {
                        node->flags &= ~LYS_MAND_TRUE;
                        lys_compile_mandatory_parents(node->parent, 0);
                    }
                }
                break;
            case LYS_LIST:
                if (rfn->flags & LYS_SET_MAX) {
                    ((struct lysc_node_list*)node)->max = rfn->max ? rfn->max : (uint32_t)-1;
                }
                if (rfn->flags & LYS_SET_MIN) {
                    ((struct lysc_node_list*)node)->min = rfn->min;
                    if (rfn->min) {
                        node->flags |= LYS_MAND_TRUE;
                        lys_compile_mandatory_parents(node->parent, 1);
                    } else {
                        node->flags &= ~LYS_MAND_TRUE;
                        lys_compile_mandatory_parents(node->parent, 0);
                    }
                }
                break;
            default:
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "Invalid refine of %s statement in \"%s\" - %s cannot hold this statement.",
                       (rfn->flags & LYS_SET_MAX) ? "max-elements" : "min-elements", rfn->nodeid, lys_nodetype2str(node->nodetype));
                goto error;
            }
        }

        /* if-feature */
        if (rfn->iffeatures) {
            /* any node in compiled tree can get additional if-feature, so do not check nodetype */
            COMPILE_ARRAY_GOTO(ctx, rfn->iffeatures, node->iffeatures, options, u, lys_compile_iffeature, ret, error);
        }
    }
    /* fix connection of the children nodes from fake context node back into the parent */
    if (context_node_fake.child) {
        context_node_fake.child->prev = child;
    }
    LY_LIST_FOR(context_node_fake.child, child) {
        child->parent = parent;
    }

    /* do some additional checks of the changed nodes when all the refines are applied */
    for (u = 0; u < refined.count; ++u) {
        node = (struct lysc_node*)refined.objs[u];
        rfn = &uses_p->refines[u];

        /* check possible conflict with default value (default added, mandatory left true) */
        if ((node->flags & LYS_MAND_TRUE) &&
                (((node->nodetype & LYS_CHOICE) && ((struct lysc_node_choice*)node)->dflt) ||
                ((node->nodetype & LYS_LEAF) && (node->flags & LYS_SET_DFLT)))) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                   "Invalid refine of default in \"%s\" - the node is mandatory.", rfn->nodeid);
            goto error;
        }

        if (rfn->flags & (LYS_SET_MAX | LYS_SET_MIN)) {
            if (node->nodetype == LYS_LIST) {
                min = ((struct lysc_node_list*)node)->min;
                max = ((struct lysc_node_list*)node)->max;
            } else {
                min = ((struct lysc_node_leaflist*)node)->min;
                max = ((struct lysc_node_leaflist*)node)->max;
            }
            if (min > max) {
                LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                       "Invalid refine of %s statement in \"%s\" - \"min-elements\" is bigger than \"max-elements\".",
                       (rfn->flags & LYS_SET_MAX) ? "max-elements" : "min-elements", rfn->nodeid);
                goto error;
            }
        }
    }

    ret = LY_SUCCESS;
error:
    /* reload previous context's mod_def */
    ctx->mod_def = mod_old;
    /* remove the grouping from the stack for circular groupings dependency check */
    ly_set_rm_index(&ctx->groupings, ctx->groupings.count - 1, NULL);
    assert(ctx->groupings.count == grp_stack_count);
    ly_set_erase(&refined, NULL);

    return ret;
}


/**
 * @brief Compile parsed schema node information.
 * @param[in] ctx Compile context
 * @param[in] node_p Parsed schema node.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in] parent Compiled parent node where the current node is supposed to be connected. It is
 * NULL for top-level nodes, in such a case the module where the node will be connected is taken from
 * the compile context.
 * @param[in] uses_status If the node is being placed instead of uses, here we have the uses's status value (as node's flags).
 * Zero means no uses, non-zero value with no status bit set mean the default status.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node(struct lysc_ctx *ctx, struct lysp_node *node_p, int options, struct lysc_node *parent, uint16_t uses_status)
{
    LY_ERR ret = LY_EVALID;
    struct lysc_node *node;
    struct lysc_node_case *cs;
    struct lysc_when **when;
    unsigned int u;
    LY_ERR (*node_compile_spec)(struct lysc_ctx*, struct lysp_node*, int, struct lysc_node*);

    switch (node_p->nodetype) {
    case LYS_CONTAINER:
        node = (struct lysc_node*)calloc(1, sizeof(struct lysc_node_container));
        node_compile_spec = lys_compile_node_container;
        break;
    case LYS_LEAF:
        node = (struct lysc_node*)calloc(1, sizeof(struct lysc_node_leaf));
        node_compile_spec = lys_compile_node_leaf;
        break;
    case LYS_LIST:
        node = (struct lysc_node*)calloc(1, sizeof(struct lysc_node_list));
        node_compile_spec = lys_compile_node_list;
        break;
    case LYS_LEAFLIST:
        node = (struct lysc_node*)calloc(1, sizeof(struct lysc_node_leaflist));
        node_compile_spec = lys_compile_node_leaflist;
        break;
    case LYS_CHOICE:
        node = (struct lysc_node*)calloc(1, sizeof(struct lysc_node_choice));
        node_compile_spec = lys_compile_node_choice;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        node = (struct lysc_node*)calloc(1, sizeof(struct lysc_node_anydata));
        node_compile_spec = lys_compile_node_any;
        break;
    case LYS_USES:
        return lys_compile_uses(ctx, (struct lysp_node_uses*)node_p, options, parent);
    default:
        LOGINT(ctx->ctx);
        return LY_EINT;
    }
    LY_CHECK_ERR_RET(!node, LOGMEM(ctx->ctx), LY_EMEM);
    node->nodetype = node_p->nodetype;
    node->module = ctx->mod;
    node->prev = node;
    node->flags = node_p->flags & LYS_FLAGS_COMPILED_MASK;

    /* config */
    if (!(node->flags & LYS_CONFIG_MASK)) {
        /* config not explicitely set, inherit it from parent */
        if (parent) {
            node->flags |= parent->flags & LYS_CONFIG_MASK;
        } else {
            /* default is config true */
            node->flags |= LYS_CONFIG_W;
        }
    }
    if (parent && (parent->flags & LYS_CONFIG_R) && (node->flags & LYS_CONFIG_W)) {
        LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
               "Configuration node cannot be child of any state data node.");
        goto error;
    }

    /* *list ordering */
    if (node->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        if ((node->flags & LYS_CONFIG_R) && (node->flags & LYS_ORDBY_MASK)) {
            LOGWRN(ctx->ctx, "The ordered-by statement is ignored in lists representing state data, "
                   "RPC/action output parameters or notification content (%s).", ctx->path);
            node->flags &= ~LYS_ORDBY_MASK;
            node->flags |= LYS_ORDBY_SYSTEM;
        } else if (!(node->flags & LYS_ORDBY_MASK)) {
            /* default ordering is system */
            node->flags |= LYS_ORDBY_SYSTEM;
        }
    }

    /* status - it is not inherited by specification, but it does not make sense to have
     * current in deprecated or deprecated in obsolete, so we do print warning and inherit status */
    if (!parent || parent->nodetype != LYS_CHOICE) {
        /* in case of choice/case's children, postpone the check to the moment we know if
         * the parent is choice (parent here) or some case (so we have to get its flags to check) */
        LY_CHECK_GOTO(lys_compile_status(ctx, node, uses_status ? uses_status : (parent ? parent->flags : 0)), error);
    }

    if (!(options & LYSC_OPT_FREE_SP)) {
        node->sp = node_p;
    }
    DUP_STRING(ctx->ctx, node_p->name, node->name);
    DUP_STRING(ctx->ctx, node_p->dsc, node->dsc);
    DUP_STRING(ctx->ctx, node_p->ref, node->ref);
    if (node_p->when) {
        LY_ARRAY_NEW_GOTO(ctx->ctx, node->when, when, ret, error);
        ret = lys_compile_when(ctx, node_p->when, options, when);
        LY_CHECK_GOTO(ret, error);
        (*when)->context = lys_compile_xpath_context(node);
    }
    COMPILE_ARRAY_GOTO(ctx, node_p->iffeatures, node->iffeatures, options, u, lys_compile_iffeature, ret, error);
    COMPILE_ARRAY_GOTO(ctx, node_p->exts, node->exts, options, u, lys_compile_ext, ret, error);

    /* nodetype-specific part */
    LY_CHECK_GOTO(node_compile_spec(ctx, node_p, options, node), error);

    /* inherit LYS_MAND_TRUE in parent containers */
    if (node->flags & LYS_MAND_TRUE) {
        lys_compile_mandatory_parents(parent, 1);
    }

    /* insert into parent's children */
    if (parent) {
        if (parent->nodetype == LYS_CHOICE) {
            cs = lys_compile_node_case(ctx, node_p->parent, options, (struct lysc_node_choice*)parent, node);
            LY_CHECK_ERR_GOTO(!cs, ret = LY_EVALID, error);
            if (uses_status) {

            }
            /* the postponed status check of the node and its real parent - in case of implicit case,
             * it directly gets the same status flags as the choice;
             * uses_status cannot be applied here since uses cannot be child statement of choice */
            LY_CHECK_GOTO(lys_compile_status(ctx, node, cs->flags), error);
            node->parent = (struct lysc_node*)cs;
        } else { /* other than choice */
            node->parent = parent;
        }
        LY_CHECK_RET(lys_compile_node_connect(ctx, parent->nodetype == LYS_CASE ? parent->parent : parent, node), LY_EVALID);
    } else {
        /* top-level element */
        if (!ctx->mod->compiled->data) {
            ctx->mod->compiled->data = node;
        } else {
            /* insert at the end of the module's top-level nodes list */
            ctx->mod->compiled->data->prev->next = node;
            node->prev = ctx->mod->compiled->data->prev;
            ctx->mod->compiled->data->prev = node;
        }
        if (lys_compile_node_uniqness(ctx, ctx->mod->compiled->data, ctx->mod->compiled->rpcs,
                                      ctx->mod->compiled->notifs, node->name, node)) {
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;

error:
    lysc_node_free(ctx->ctx, node);
    return ret;
}

static void
lys_compile_augment_sort_(struct lysp_augment *aug_p, struct lysp_augment **result)
{
    unsigned int v;
    size_t len;

    len = strlen(aug_p->nodeid);
    LY_ARRAY_FOR(result, v) {
        if (strlen(result[v]->nodeid) <= len) {
            continue;
        }
        if (v < LY_ARRAY_SIZE(result)) {
            /* move the rest of array */
            memmove(&result[v + 1], &result[v], (LY_ARRAY_SIZE(result) - v) * sizeof *result);
            break;
        }
    }
    result[v] = aug_p;
    LY_ARRAY_INCREMENT(result);
}

LY_ERR
lys_compile_augment_sort(struct lysc_ctx *ctx, struct lysp_module *mod_p, struct lysp_augment ***augments)
{
    struct lysp_augment **result = NULL;
    unsigned int u, v;
    size_t count = 0;

    assert(mod_p);
    assert(augments);

    /* get count of the augments in module and all its submodules */
    if (mod_p->augments) {
        count += LY_ARRAY_SIZE(mod_p->augments);
    }
    LY_ARRAY_FOR(mod_p->includes, u) {
        if (mod_p->includes[u].submodule->augments) {
            count += LY_ARRAY_SIZE(mod_p->includes[u].submodule->augments);
        }
    }

    if (!count) {
        *augments = NULL;
        return LY_SUCCESS;
    }
    LY_ARRAY_CREATE_RET(ctx->ctx, result, count, LY_EMEM);

    /* sort by the length of schema-nodeid - we need to solve /x before /x/xy. It is not necessary to group them
     * together, so there can be even /z/y betwwen them. */
    LY_ARRAY_FOR(mod_p->augments, u) {
        lys_compile_augment_sort_(&mod_p->augments[u], result);
    }
    LY_ARRAY_FOR(mod_p->includes, u) {
        LY_ARRAY_FOR(mod_p->includes[u].submodule->augments, v) {
            lys_compile_augment_sort_(&mod_p->includes[u].submodule->augments[v], result);
        }
    }

    *augments = result;
    return LY_SUCCESS;
}

/**
 * @brief Compile the parsed augment connecting it into its target.
 *
 * It is expected that all the data referenced in path are present - augments are ordered so that augment B
 * targeting data from augment A is being compiled after augment A. Also the modules referenced in the path
 * are already implemented and compiled.
 *
 * @param[in] ctx Compile context.
 * @param[in] aug_p Parsed augment to compile.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in] parent Parent node to provide the augment's context. It is NULL for the top level augments and a node holding uses's
 * children in case of the augmenting uses data.
 * @return LY_SUCCESS on success.
 * @return LY_EVALID on failure.
 */
LY_ERR
lys_compile_augment(struct lysc_ctx *ctx, struct lysp_augment *aug_p, int options, const struct lysc_node *parent)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node *node_p, *case_node_p;
    struct lysc_node *target; /* target target of the augment */
    struct lysc_node *node;
    struct lysc_node_case *next_case;
    struct lysc_when **when, *when_shared;
    int allow_mandatory = 0;

    ret = lys_resolve_schema_nodeid(ctx, aug_p->nodeid, 0, parent,
                                               LYS_CONTAINER | LYS_LIST | LYS_CHOICE | LYS_CASE | LYS_INOUT | LYS_NOTIF,
                                               1, (const struct lysc_node**)&target);
    if (ret != LY_SUCCESS) {
        if (ret == LY_EDENIED) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Augment's %s-schema-nodeid \"%s\" refers to a %s node which is not an allowed augment's target.",
                   parent ? "descendant" : "absolute", aug_p->nodeid, lys_nodetype2str(target->nodetype));
        }
        return LY_EVALID;
    }

    /* check for mandatory nodes
     * - new cases augmenting some choice can have mandatory nodes
     * - mandatory nodes are allowed only in case the augmentation is made conditional with a when statement
     */
    if (aug_p->when || target->nodetype == LYS_CHOICE) {
        allow_mandatory = 1;
    }

    when_shared = NULL;
    LY_LIST_FOR(aug_p->child, node_p) {
        /* check if the subnode can be connected to the found target (e.g. case cannot be inserted into container) */
        if (!(target->nodetype == LYS_CHOICE && node_p->nodetype == LYS_CASE)
                && !((target->nodetype & (LYS_CONTAINER | LYS_LIST)) && (node_p->nodetype & (LYS_ACTION | LYS_NOTIF)))
                && !(node_p->nodetype & (LYS_ANYDATA | LYS_CONTAINER | LYS_CHOICE | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_USES))) {
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_REFERENCE,
                   "Invalid augment (%s) of %s node which is not allowed to contain %s node \"%s\".",
                   aug_p->nodeid, lys_nodetype2str(target->nodetype), lys_nodetype2str(node_p->nodetype), node_p->name);
            return LY_EVALID;
        }

        /* compile the children */
        if (node_p->nodetype != LYS_CASE) {
            LY_CHECK_RET(lys_compile_node(ctx, node_p, options, target, 0));
        } else {
            LY_LIST_FOR(((struct lysp_node_case *)node_p)->child, case_node_p) {
                LY_CHECK_RET(lys_compile_node(ctx, case_node_p, options, target, 0));
            }
        }

        /* since the augment node is not present in the compiled tree, we need to pass some of its statements to all its children */
        if (target->nodetype == LYS_CASE) {
            /* the compiled node is the last child of the target (but it is a case, so we have to be careful) */
            next_case = target->next ? (struct lysc_node_case*)target->next : ((struct lysc_node_choice*)target->parent)->cases;
            for (node = (struct lysc_node*)lysc_node_children(target); node->next && node->next != next_case->child; node = node->next);
        } else if (target->nodetype == LYS_CHOICE) {
            /* to pass when statement, we need the last case no matter if it is explicit or implicit case */
            node = ((struct lysc_node_choice*)target)->cases->prev;
        } else {
            /* the compiled node is the last child of the target */
            node = lysc_node_children(target)->prev;
        }

        if (!allow_mandatory && (node->flags & LYS_MAND_TRUE)) {
            node->flags &= ~LYS_MAND_TRUE;
            lys_compile_mandatory_parents(target, 0);
            LOGVAL(ctx->ctx, LY_VLOG_STR, ctx->path, LYVE_SEMANTICS,
                   "Invalid augment (%s) adding mandatory node \"%s\" without making it conditional via when statement.",
                   aug_p->nodeid, node->name);
            return LY_EVALID;
        }

        /* pass augment's when to all the children */
        if (aug_p->when) {
            LY_ARRAY_NEW_GOTO(ctx->ctx, node->when, when, ret, error);
            if (!when_shared) {
                ret = lys_compile_when(ctx, aug_p->when, options, when);
                LY_CHECK_GOTO(ret, error);
                (*when)->context = lys_compile_xpath_context(target);
                when_shared = *when;
            } else {
                ++when_shared->refcount;
                (*when) = when_shared;
            }
        }
    }
    /* TODO actions, notifications */

error:
    return ret;
}

/**
 * @brief Compile the given YANG submodule into the main module.
 * @param[in] ctx Compile context
 * @param[in] inc Include structure from the main module defining the submodule.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR
lys_compile_submodule(struct lysc_ctx *ctx, struct lysp_include *inc, int options)
{
    unsigned int u;
    LY_ERR ret = LY_SUCCESS;
    /* shortcuts */
    struct lysp_submodule *submod = inc->submodule;
    struct lysc_module *mainmod = ctx->mod->compiled;

    if (!mainmod->mod->off_features) {
        /* features are compiled directly into the compiled module structure,
         * but it must be done in two steps to allow forward references (via if-feature) between the features themselves.
         * The features compilation is finished in the main module (lys_compile()). */
        ret = lys_feature_precompile(ctx->ctx, submod->features,
                                     mainmod->mod->off_features ? &mainmod->mod->off_features : &mainmod->features);
        LY_CHECK_GOTO(ret, error);
    }

    COMPILE_ARRAY_UNIQUE_GOTO(ctx, submod->identities, mainmod->identities, options, u, lys_compile_identity, ret, error);

error:
    return ret;
}



LY_ERR
lys_compile(struct lys_module *mod, int options)
{
    struct lysc_ctx ctx = {0};
    struct lysc_module *mod_c;
    struct lysc_type *type, *typeiter;
    struct lysp_module *sp;
    struct lysp_node *node_p;
    struct lysp_augment **augments = NULL;
    struct lys_module *m;
    unsigned int u, v;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, mod, mod->parsed, mod->ctx, LY_EINVAL);

    if (!mod->implemented) {
        /* just imported modules are not compiled */
        return LY_SUCCESS;
    }

    sp = mod->parsed;

    ctx.ctx = mod->ctx;
    ctx.mod = mod;
    ctx.mod_def = mod;

    mod->compiled = mod_c = calloc(1, sizeof *mod_c);
    LY_CHECK_ERR_RET(!mod_c, LOGMEM(mod->ctx), LY_EMEM);
    mod_c->mod = mod;

    COMPILE_ARRAY_GOTO(&ctx, sp->imports, mod_c->imports, options, u, lys_compile_import, ret, error);
    LY_ARRAY_FOR(sp->includes, u) {
        ret = lys_compile_submodule(&ctx, &sp->includes[u], options);
        LY_CHECK_GOTO(ret != LY_SUCCESS, error);
    }
    if (mod->off_features) {
        /* there is already precompiled array of features */
        mod_c->features = mod->off_features;
        mod->off_features = NULL;
    } else {
        /* features are compiled directly into the compiled module structure,
         * but it must be done in two steps to allow forward references (via if-feature) between the features themselves */
        ret = lys_feature_precompile(ctx.ctx, sp->features, &mod_c->features);
        LY_CHECK_GOTO(ret, error);
    }
    /* finish feature compilation, not only for the main module, but also for the submodules.
     * Due to possible forward references, it must be done when all the features (including submodules)
     * are present. */
    LY_ARRAY_FOR(sp->features, u) {
        ret = lys_feature_precompile_finish(&ctx, &sp->features[u], options, mod_c->features);
        LY_CHECK_GOTO(ret != LY_SUCCESS, error);
    }
    LY_ARRAY_FOR(sp->includes, v) {
        LY_ARRAY_FOR(sp->includes[v].submodule->features, u) {
            ret = lys_feature_precompile_finish(&ctx, &sp->includes[v].submodule->features[u], options, mod_c->features);
            LY_CHECK_GOTO(ret != LY_SUCCESS, error);
        }
    }

    COMPILE_ARRAY_UNIQUE_GOTO(&ctx, sp->identities, mod_c->identities, options, u, lys_compile_identity, ret, error);
    if (sp->identities) {
        LY_CHECK_RET(lys_compile_identities_derived(&ctx, sp->identities, mod_c->identities));
    }

    /* data nodes */
    LY_LIST_FOR(sp->data, node_p) {
        ret = lys_compile_node(&ctx, node_p, options, NULL, 0);
        LY_CHECK_GOTO(ret, error);
    }

    /* augments - sort first to cover augments augmenting other augments */
    ret = lys_compile_augment_sort(&ctx, sp, &augments);
    LY_CHECK_GOTO(ret, error);
    LY_ARRAY_FOR(augments, u) {
        ret = lys_compile_augment(&ctx, augments[u], options, NULL);
        LY_CHECK_GOTO(ret, error);
    }
    //COMPILE_ARRAY_GOTO(ctx, sp->rpcs, mod_c->rpcs, options, u, lys_compile_action, ret, error);
    //COMPILE_ARRAY_GOTO(ctx, sp->notifs, mod_c->notifs, options, u, lys_compile_notif, ret, error);

    COMPILE_ARRAY_GOTO(&ctx, sp->exts, mod_c->exts, options, u, lys_compile_ext, ret, error);

    /* validate leafref's paths and when/must xpaths */
    /* for leafref, we need 2 rounds - first detects circular chain by storing the first referred type (which
     * can be also leafref, in case it is already resolved, go through the chain and check that it does not
     * point to the starting leafref type). The second round stores the first non-leafref type for later data validation. */
    for (u = 0; u < ctx.unres.count; ++u) {
        if (((struct lysc_node*)ctx.unres.objs[u])->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
            type = ((struct lysc_node_leaf*)ctx.unres.objs[u])->type;
            if (type->basetype == LY_TYPE_LEAFREF) {
                /* validate the path */
                ret = lys_compile_leafref_validate(&ctx, ((struct lysc_node*)ctx.unres.objs[u]), (struct lysc_type_leafref*)type);
                LY_CHECK_GOTO(ret, error);
            } else if (type->basetype == LY_TYPE_UNION) {
                LY_ARRAY_FOR(((struct lysc_type_union*)type)->types, v) {
                    if (((struct lysc_type_union*)type)->types[v]->basetype == LY_TYPE_LEAFREF) {
                        /* validate the path */
                        ret = lys_compile_leafref_validate(&ctx, ((struct lysc_node*)ctx.unres.objs[u]),
                                                           (struct lysc_type_leafref*)((struct lysc_type_union*)type)->types[v]);
                        LY_CHECK_GOTO(ret, error);
                    }
                }
            }
        }
    }
    for (u = 0; u < ctx.unres.count; ++u) {
        if (((struct lysc_node*)ctx.unres.objs[u])->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
            type = ((struct lysc_node_leaf*)ctx.unres.objs[u])->type;
            if (type->basetype == LY_TYPE_LEAFREF) {
                /* store pointer to the real type */
                for (typeiter = ((struct lysc_type_leafref*)type)->realtype;
                        typeiter->basetype == LY_TYPE_LEAFREF;
                        typeiter = ((struct lysc_type_leafref*)typeiter)->realtype);
                ((struct lysc_type_leafref*)type)->realtype = typeiter;
            } else if (type->basetype == LY_TYPE_UNION) {
                LY_ARRAY_FOR(((struct lysc_type_union*)type)->types, v) {
                    if (((struct lysc_type_union*)type)->types[v]->basetype == LY_TYPE_LEAFREF) {
                        /* store pointer to the real type */
                        for (typeiter = ((struct lysc_type_leafref*)((struct lysc_type_union*)type)->types[v])->realtype;
                                typeiter->basetype == LY_TYPE_LEAFREF;
                                typeiter = ((struct lysc_type_leafref*)typeiter)->realtype);
                        ((struct lysc_type_leafref*)((struct lysc_type_union*)type)->types[v])->realtype = typeiter;
                    }
                }
            }
        }
    }
    ly_set_erase(&ctx.unres, NULL);
    ly_set_erase(&ctx.groupings, NULL);
    LY_ARRAY_FREE(augments);

    if (options & LYSC_OPT_FREE_SP) {
        lysp_module_free(mod->parsed);
        ((struct lys_module*)mod)->parsed = NULL;
    }

    if (!(options & LYSC_OPT_INTERNAL)) {
        /* remove flag of the modules implemented by dependency */
        for (u = 0; u < ctx.ctx->list.count; ++u) {
            m = ctx.ctx->list.objs[u];
            if (m->implemented == 2) {
                m->implemented = 1;
            }
        }
    }

    ((struct lys_module*)mod)->compiled = mod_c;
    return LY_SUCCESS;

error:
    lys_feature_precompile_revert(&ctx, mod);
    ly_set_erase(&ctx.unres, NULL);
    ly_set_erase(&ctx.groupings, NULL);
    LY_ARRAY_FREE(augments);
    lysc_module_free(mod_c, NULL);
    mod->compiled = NULL;

    /* revert compilation of modules implemented by dependency */
    for (u = 0; u < ctx.ctx->list.count; ++u) {
        m = ctx.ctx->list.objs[u];
        if (m->implemented == 2) {
            /* revert features list to the precompiled state */
            lys_feature_precompile_revert(&ctx, m);
            /* mark module as imported-only / not-implemented */
            m->implemented = 0;
            /* free the compiled version of the module */
            lysc_module_free(m->compiled, NULL);
            m->compiled = NULL;
        }
    }

    return ret;
}
