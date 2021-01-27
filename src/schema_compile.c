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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "in.h"
#include "log.h"
#include "parser_schema.h"
#include "path.h"
#include "plugins_exts.h"
#include "plugins_exts_internal.h"
#include "plugins_types.h"
#include "schema_compile_amend.h"
#include "schema_compile_node.h"
#include "schema_features.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

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
    if (ret) {
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
    }
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
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid XML prefix of \"%.*s\" namespace used for extension instance identifier.", u, ext_p->name);
            ret = LY_EVALID;
            goto cleanup;
        }
    } else {
        prefixed_name = ext_p->name;
    }
    lysc_update_path(ctx, NULL, prefixed_name);

    if (!ext_mod) {
        ext_mod = u ? ly_resolve_prefix(ctx->ctx, prefixed_name, u - 1, LY_PREF_SCHEMA, ctx->pmod) : ctx->pmod->mod;
        if (!ext_mod) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid prefix \"%.*s\" used for extension instance identifier.", u, prefixed_name);
            ret = LY_EVALID;
            goto cleanup;
        } else if (!ext_mod->parsed->extensions) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
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
        LOGVAL(ctx->ctx, LYVE_REFERENCE,
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
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
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
    lysc_update_path(ctx, NULL, NULL);
    if (prefixed_name) {
        lysc_update_path(ctx, NULL, NULL);
        if (prefixed_name != ext_p->name) {
            lydict_remove(ctx->ctx, prefixed_name);
        }
    }

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

    LOG_LOCBACK(0, 0, 1, 0);
    LOG_LOCSET(NULL, NULL, ctx->path, NULL);
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
                        ".yin") ? LYS_IN_YIN : LYS_IN_YANG, NULL, &mod));
                if (mod != imp_p->module) {
                    LOGERR(ctx->ctx, LY_EINT, "Filepath \"%s\" of the module \"%s\" does not match.",
                            imp_p->module->filepath, imp_p->module->name);
                    mod = NULL;
                }
            }
            ly_in_free(in, 1);
        }
        if (!mod) {
            if (lysp_load_module(ctx->ctx, imp_p->module->name, imp_p->module->revision, 0, NULL, ctx->unres,
                    (struct lys_module **)&mod)) {
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
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_ctx context = {0};
    struct lysc_ident *ident;
    LY_ERR ret = LY_SUCCESS;
    ly_bool enabled;

    assert(ctx_sc || ctx);

    if (!ctx_sc) {
        context.ctx = ctx;
        context.cur_mod = parsed_mod ? parsed_mod->mod : NULL;
        context.pmod = parsed_mod;
        context.path_len = 1;
        context.path[0] = '/';
        ctx_sc = &context;
    }

    if (!identities_p) {
        return LY_SUCCESS;
    }

    lysc_update_path(ctx_sc, NULL, "{identity}");
    LY_ARRAY_FOR(identities_p, u) {
        /* evaluate if-features */
        LY_CHECK_RET(lys_eval_iffeatures(ctx, identities_p[u].iffeatures, &enabled));
        if (!enabled) {
            continue;
        }

        lysc_update_path(ctx_sc, NULL, identities_p[u].name);

        /* add new compiled identity */
        LY_ARRAY_NEW_RET(ctx_sc->ctx, *identities, ident, LY_EMEM);

        DUP_STRING_GOTO(ctx_sc->ctx, identities_p[u].name, ident->name, ret, done);
        DUP_STRING_GOTO(ctx_sc->ctx, identities_p[u].dsc, ident->dsc, ret, done);
        DUP_STRING_GOTO(ctx_sc->ctx, identities_p[u].ref, ident->ref, ret, done);
        ident->module = ctx_sc->cur_mod;
        /* backlinks (derived) can be added no sooner than when all the identities in the current module are present */
        COMPILE_EXTS_GOTO(ctx_sc, identities_p[u].exts, ident->exts, ident, LYEXT_PAR_IDENT, ret, done);
        ident->flags = identities_p[u].flags;

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
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Identity \"%s\" is indirectly derived from itself.", ident->name);
            ret = LY_EVALID;
            goto cleanup;
        }
        ret = ly_set_add(&recursion, derived[u], 0, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    for (v = 0; v < recursion.count; ++v) {
        drv = recursion.objs[v];
        for (u = 0; u < LY_ARRAY_COUNT(drv->derived); ++u) {
            if (ident == drv->derived[u]) {
                LOGVAL(ctx->ctx, LYVE_REFERENCE,
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
        struct lysc_ident *ident, struct lysc_ident ***bases, ly_bool *enabled)
{
    LY_ARRAY_COUNT_TYPE u, v;
    const char *s, *name;
    const struct lys_module *mod;
    struct lysc_ident **idref;

    assert((ident && enabled) || bases);

    if ((LY_ARRAY_COUNT(bases_p) > 1) && (ctx->pmod->version < LYS_VERSION_1_1)) {
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                "Multiple bases in %s are allowed only in YANG 1.1 modules.", ident ? "identity" : "identityref type");
        return LY_EVALID;
    }

    LY_ARRAY_FOR(bases_p, u) {
        s = strchr(bases_p[u], ':');
        if (s) {
            /* prefixed identity */
            name = &s[1];
            mod = ly_resolve_prefix(ctx->ctx, bases_p[u], s - bases_p[u], LY_PREF_SCHEMA, (void *)base_pmod);
        } else {
            name = bases_p[u];
            mod = base_pmod->mod;
        }
        if (!mod) {
            if (ident) {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid prefix used for base (%s) of identity \"%s\".", bases_p[u], ident->name);
            } else {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid prefix used for base (%s) of identityref.", bases_p[u]);
            }
            return LY_EVALID;
        }

        idref = NULL;
        LY_ARRAY_FOR(mod->identities, v) {
            if (!strcmp(name, mod->identities[v].name)) {
                if (ident) {
                    if (ident == &mod->identities[v]) {
                        LOGVAL(ctx->ctx, LYVE_REFERENCE,
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
                /* look into the parsed module to check whether the identity is not merely disabled */
                LY_ARRAY_FOR(mod->parsed->identities, v) {
                    if (!strcmp(mod->parsed->identities[v].name, name)) {
                        *enabled = 0;
                        return LY_SUCCESS;
                    }
                }
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Unable to find base (%s) of identity \"%s\".", bases_p[u], ident->name);
            } else {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Unable to find base (%s) of identityref.", bases_p[u]);
            }
            return LY_EVALID;
        }
    }

    if (ident) {
        *enabled = 1;
    }
    return LY_SUCCESS;
}

/**
 * @brief For the given array of identities, set the backlinks from all their base identities.
 * @param[in] ctx Compile context, not only for logging but also to get the current module to resolve prefixes.
 * @param[in] idents_p Array of identities definitions from the parsed schema structure.
 * @param[in,out] idents Array of referencing identities to which the backlinks are supposed to be set. Any
 * identities with disabled bases are removed.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_identities_derived(struct lysc_ctx *ctx, struct lysp_ident *idents_p, struct lysc_ident **idents)
{
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool enabled;

    lysc_update_path(ctx, NULL, "{identity}");

restart:
    for (u = 0, v = 0; u < LY_ARRAY_COUNT(*idents); ++u) {
        /* find matching parsed identity, the disabled ones are missing in the compiled array */
        while (v < LY_ARRAY_COUNT(idents_p)) {
            if (idents_p[v].name == (*idents)[u].name) {
                break;
            }
            ++v;
        }
        assert(v < LY_ARRAY_COUNT(idents_p));

        if (!idents_p[v].bases) {
            continue;
        }
        lysc_update_path(ctx, NULL, (*idents)[u].name);
        LY_CHECK_RET(lys_compile_identity_bases(ctx, (*idents)[u].module->parsed, idents_p[v].bases, &(*idents)[u], NULL,
                &enabled));
        lysc_update_path(ctx, NULL, NULL);

        if (!enabled) {
            /* remove the identity */
            lysc_ident_free(ctx->ctx, &(*idents)[u]);
            LY_ARRAY_DECREMENT(*idents);
            if (u < LY_ARRAY_COUNT(*idents)) {
                memmove(&(*idents)[u], &(*idents)[u + 1], (LY_ARRAY_COUNT(*idents) - u) * sizeof **idents);
            }

            /* revert compilation of all the previous identities */
            for (v = 0; v < u; ++v) {
                LY_ARRAY_FREE((*idents)[v].derived);
            }

            /* free the whole array if there are no identites left */
            if (!LY_ARRAY_COUNT(*idents)) {
                LY_ARRAY_FREE(*idents);
                *idents = NULL;
            }

            /* restart the whole process without this identity */
            goto restart;
        }
    }

    lysc_update_path(ctx, NULL, NULL);
    return LY_SUCCESS;
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
    struct lysp_qname *iffeatures, *iffeat;
    void *parsed = NULL, **compiled = NULL;
    ly_bool enabled;

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
            LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s%s%s\" extension instance.",
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
                            LOGVAL(ctx->ctx, LY_VCODE_DUPSTMT, stmt->stmt);
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
                            LOGVAL(ctx->ctx, LY_VCODE_DUPSTMT, stmt->stmt);
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
                    r = lys_compile_type(ctx, NULL, flags ? *flags : 0, ctx->pmod, ext->name, parsed,
                            (struct lysc_type **)compiled, units && !*units ? units : NULL, NULL);
                    lysp_type_free(ctx->ctx, parsed);
                    free(parsed);
                    LY_CHECK_ERR_GOTO(r, ret = r, cleanup);
                    break;
                }
                case LY_STMT_IF_FEATURE:
                    iffeatures = NULL;
                    LY_ARRAY_NEW_GOTO(ctx->ctx, iffeatures, iffeat, ret, cleanup);
                    iffeat->str = stmt->arg;
                    iffeat->mod = ctx->pmod;
                    r = lys_eval_iffeatures(ctx->ctx, iffeatures, &enabled);
                    LY_ARRAY_FREE(iffeatures);
                    LY_CHECK_ERR_GOTO(r, ret = r, cleanup);
                    if (!enabled) {
                        /* it is disabled, remove the whole extension instance */
                        return LY_ENOT;
                    }
                    break;
                /* TODO support other substatements (parse stmt to lysp and then compile lysp to lysc),
                 * also note that in many statements their extensions are not taken into account  */
                default:
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Statement \"%s\" is not supported as an extension (found in \"%s%s%s\") substatement.",
                            stmt->stmt, ext->name, ext->argument ? " " : "", ext->argument ? ext->argument : "");
                    goto cleanup;
                }
            }
        }

        if (((substmts[u].cardinality == LY_STMT_CARD_MAND) || (substmts[u].cardinality == LY_STMT_CARD_SOME)) && !stmt_present) {
            LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Missing mandatory keyword \"%s\" as a child of \"%s%s%s\".",
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
    LY_ERR ret = LY_SUCCESS;

    memset(&tmp_set, 0, sizeof tmp_set);

    /* prepare in_ctx of the set */
    for (i = 0; i < set->used; ++i) {
        xp_scnode = &set->val.scnodes[i];

        if (xp_scnode->in_ctx != LYXP_SET_SCNODE_START_USED) {
            /* check node when, skip the context node (it was just checked) */
            xp_scnode->in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
        }
    }

    for (i = 0; i < set->used; ++i) {
        xp_scnode = &set->val.scnodes[i];
        if (xp_scnode->in_ctx != LYXP_SET_SCNODE_ATOM_CTX) {
            /* already checked */
            continue;
        }

        if ((xp_scnode->type != LYXP_NODE_ELEM) || (xp_scnode->scnode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) ||
                !lysc_node_when(xp_scnode->scnode)) {
            /* no when to check */
            xp_scnode->in_ctx = LYXP_SET_SCNODE_ATOM;
            continue;
        }

        node = xp_scnode->scnode;
        do {
            struct lysc_when **when_list, *when;

            LOG_LOCSET(node, NULL, NULL, NULL);
            when_list = lysc_node_when(node);
            LY_ARRAY_FOR(when_list, u) {
                when = when_list[u];
                ret = lyxp_atomize(set->ctx, when->cond, node->module, LY_PREF_SCHEMA_RESOLVED, when->prefixes,
                        when->context, &tmp_set, LYXP_SCNODE_SCHEMA);
                if (ret != LY_SUCCESS) {
                    LOGVAL(set->ctx, LYVE_SEMANTICS, "Invalid when condition \"%s\".", when->cond->expr);
                    goto cleanup;
                }

                for (j = 0; j < tmp_set.used; ++j) {
                    /* skip roots'n'stuff */
                    if (tmp_set.val.scnodes[j].type == LYXP_NODE_ELEM) {
                        /* try to find this node in our set */
                        uint32_t idx;
                        if (lyxp_set_scnode_contains(set, tmp_set.val.scnodes[j].scnode, LYXP_NODE_ELEM, -1, &idx) &&
                                (set->val.scnodes[idx].in_ctx == LYXP_SET_SCNODE_START_USED)) {
                            LOGVAL(set->ctx, LYVE_SEMANTICS, "When condition includes a self-reference.");
                            ret = LY_EVALID;
                            goto cleanup;
                        }

                        /* needs to be checked, if in both sets, will be ignored */
                        tmp_set.val.scnodes[j].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
                    } else {
                        /* no when, nothing to check */
                        tmp_set.val.scnodes[j].in_ctx = LYXP_SET_SCNODE_ATOM;
                    }
                }

                /* merge this set into the global when set */
                lyxp_set_scnode_merge(set, &tmp_set);
            }

            /* check when of non-data parents as well */
            node = node->parent;

            LOG_LOCBACK(1, 0, 0, 0);
        } while (node && (node->nodetype & (LYS_CASE | LYS_CHOICE)));

        /* this node when was checked (xp_scnode could have been reallocd) */
        set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_START_USED;
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
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "A %s definition \"%s\" is not allowed to reference %s definition \"%s\".",
                    flg1 == LYS_STATUS_CURR ? "current" : "deprecated", name1,
                    flg2 == LYS_STATUS_OBSLT ? "obsolete" : "deprecated", name2);
        }
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

LY_ERR
lys_compile_expr_implement(const struct ly_ctx *ctx, const struct lyxp_expr *expr, LY_PREFIX_FORMAT format,
        void *prefix_data, ly_bool implement, struct lys_glob_unres *unres, const struct lys_module **mod_p)
{
    uint32_t i;
    const char *ptr, *start;
    const struct lys_module *mod;

    assert(implement || mod_p);

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
            if (implement) {
                LY_CHECK_RET(lys_set_implemented_r((struct lys_module *)mod, NULL, unres));
            } else {
                *mod_p = mod;
                break;
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Check when/must expressions of a node on a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Node to check.
 * @param[in,out] unres Global unres structure.
 * @return LY_ERR value
 */
static LY_ERR
lys_compile_unres_xpath(struct lysc_ctx *ctx, const struct lysc_node *node, struct lys_glob_unres *unres)
{
    struct lyxp_set tmp_set;
    uint32_t i, opts;
    LY_ARRAY_COUNT_TYPE u;
    ly_bool input_done = 0;
    struct lysc_when **whens = NULL;
    struct lysc_must *musts = NULL;
    LY_ERR ret = LY_SUCCESS;
    const struct lysc_node *op;
    const struct lys_module *mod;

    LOG_LOCSET(node, NULL, NULL, NULL);

    memset(&tmp_set, 0, sizeof tmp_set);
    opts = LYXP_SCNODE_SCHEMA;
    if (node->flags & LYS_CONFIG_R) {
        for (op = node->parent; op && !(op->nodetype & (LYS_RPC | LYS_ACTION)); op = op->parent) {}
        if (op) {
            /* we are actually in output */
            opts = LYXP_SCNODE_OUTPUT;
        }
    }

    whens = lysc_node_when(node);
    musts = lysc_node_musts(node);

    LY_ARRAY_FOR(whens, u) {
        /* first check whether all the referenced modules are implemented */
        mod = NULL;
        ret = lys_compile_expr_implement(ctx->ctx, whens[u]->cond, LY_PREF_SCHEMA_RESOLVED, whens[u]->prefixes,
                ctx->ctx->flags & LY_CTX_REF_IMPLEMENTED, unres, &mod);
        if (ret) {
            goto cleanup;
        } else if (mod) {
            LOGWRN(ctx->ctx, "When condition \"%s\" check skipped because referenced module \"%s\" is not implemented.",
                    whens[u]->cond->expr, mod->name);
            continue;
        }

        /* check "when" */
        ret = lyxp_atomize(ctx->ctx, whens[u]->cond, node->module, LY_PREF_SCHEMA_RESOLVED, whens[u]->prefixes,
                whens[u]->context, &tmp_set, opts);
        if (ret) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid when condition \"%s\".", whens[u]->cond->expr);
            goto cleanup;
        }

        ctx->path[0] = '\0';
        lysc_path((struct lysc_node *)node, LYSC_PATH_LOG, ctx->path, LYSC_CTX_BUFSIZE);
        for (i = 0; i < tmp_set.used; ++i) {
            /* skip roots'n'stuff */
            if ((tmp_set.val.scnodes[i].type == LYXP_NODE_ELEM) && (tmp_set.val.scnodes[i].in_ctx != LYXP_SET_SCNODE_START_USED)) {
                struct lysc_node *schema = tmp_set.val.scnodes[i].scnode;

                /* XPath expression cannot reference "lower" status than the node that has the definition */
                ret = lysc_check_status(ctx, whens[u]->flags, node->module, node->name, schema->flags, schema->module,
                        schema->name);
                LY_CHECK_GOTO(ret, cleanup);

                /* check dummy node accessing */
                if (schema == node) {
                    LOGVAL(ctx->ctx, LYVE_SEMANTICS, "When condition is accessing its own conditional node.");
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
        mod = NULL;
        ret = lys_compile_expr_implement(ctx->ctx, musts[u].cond, LY_PREF_SCHEMA_RESOLVED, musts[u].prefixes,
                ctx->ctx->flags & LY_CTX_REF_IMPLEMENTED, unres, &mod);
        if (ret) {
            goto cleanup;
        } else if (mod) {
            LOGWRN(ctx->ctx, "Must condition \"%s\" check skipped because referenced module \"%s\" is not implemented.",
                    musts[u].cond->expr, mod->name);
            continue;
        }

        /* check "must" */
        ret = lyxp_atomize(ctx->ctx, musts[u].cond, node->module, LY_PREF_SCHEMA_RESOLVED, musts[u].prefixes, node,
                &tmp_set, opts);
        if (ret) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid must restriction \"%s\".", musts[u].cond->expr);
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
        whens = NULL;
        musts = ((struct lysc_node_action *)node)->output.musts;
        opts = LYXP_SCNODE_OUTPUT;
        goto check_musts;
    }

cleanup:
    lyxp_set_free_content(&tmp_set);
    LOG_LOCBACK(1, 0, 0, 0);
    return ret;
}

/**
 * @brief Check leafref for its target existence on a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Context node for the leafref.
 * @param[in] lref Leafref to check/resolve.
 * @param[in,out] unres Global unres structure.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_leafref(struct lysc_ctx *ctx, const struct lysc_node *node, struct lysc_type_leafref *lref,
        struct lys_glob_unres *unres)
{
    const struct lysc_node *target = NULL, *siter;
    struct ly_path *p;
    struct lysc_type *type;

    assert(node->nodetype & (LYS_LEAF | LYS_LEAFLIST));

    /* try to find the target */
    LY_CHECK_RET(ly_path_compile(ctx->ctx, lref->cur_mod, node, lref->path, LY_PATH_LREF_TRUE, lysc_is_output(node) ?
            LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_MANY, LY_PREF_SCHEMA_RESOLVED, lref->prefixes,
            unres, &p));

    /* get the target node */
    target = p[LY_ARRAY_COUNT(p) - 1].node;
    ly_path_free(node->module->ctx, p);

    if (!(target->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid leafref path \"%s\" - target node is %s instead of leaf or leaf-list.",
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
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid leafref path \"%s\" - target is supposed"
                    " to represent configuration data (as the leafref does), but it does not.", lref->path->expr);
            return LY_EVALID;
        }
    }

    /* store the target's type and check for circular chain of leafrefs */
    lref->realtype = ((struct lysc_node_leaf *)target)->type;
    for (type = lref->realtype; type && type->basetype == LY_TYPE_LEAFREF; type = ((struct lysc_type_leafref *)type)->realtype) {
        if (type == (struct lysc_type *)lref) {
            /* circular chain detected */
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid leafref path \"%s\" - circular chain of leafrefs detected.",
                    lref->path->expr);
            return LY_EVALID;
        }
    }

    /* TODO check if leafref and its target are under common if-features */

    return LY_SUCCESS;
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
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_dflt(struct lysc_ctx *ctx, struct lysc_node *node, struct lysc_type *type, const char *dflt,
        const struct lysp_module *dflt_pmod, struct lyd_value *storage, struct lys_glob_unres *unres)
{
    LY_ERR ret;
    uint32_t options;
    struct ly_err_item *err = NULL;

    options = (ctx->ctx->flags & LY_CTX_REF_IMPLEMENTED) ? LY_TYPE_STORE_IMPLEMENT : 0;
    ret = type->plugin->store(ctx->ctx, type, dflt, strlen(dflt), options, LY_PREF_SCHEMA, (void *)dflt_pmod,
            LYD_HINT_SCHEMA, node, storage, unres, &err);
    if (ret == LY_EINCOMPLETE) {
        /* we have no data so we will not be resolving it */
        ret = LY_SUCCESS;
    }

    if (ret) {
        LOG_LOCSET(node, NULL, NULL, NULL);
        if (err) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid default - value does not fit the type (%s).", err->msg);
            ly_err_free(err);
        } else {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid default - value does not fit the type.");
        }
        LOG_LOCBACK(1, 0, 0, 0);
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
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_leaf_dlft(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf, struct lysp_qname *dflt,
        struct lys_glob_unres *unres)
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
    ret = lys_compile_unres_dflt(ctx, (struct lysc_node *)leaf, leaf->type, dflt->str, dflt->mod, leaf->dflt, unres);
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
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_llist_dflts(struct lysc_ctx *ctx, struct lysc_node_leaflist *llist, struct lysp_qname *dflt,
        struct lysp_qname *dflts, struct lys_glob_unres *unres)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE orig_count, u, v;

    assert(dflt || dflts);

    /* in case there were already some defaults and we are adding new by deviations */
    orig_count = LY_ARRAY_COUNT(llist->dflts);

    /* allocate new items */
    LY_ARRAY_CREATE_RET(ctx->ctx, llist->dflts, orig_count + (dflts ? LY_ARRAY_COUNT(dflts) : 1), LY_EMEM);

    /* fill each new default value */
    if (dflts) {
        LY_ARRAY_FOR(dflts, u) {
            llist->dflts[orig_count + u] = calloc(1, sizeof **llist->dflts);
            ret = lys_compile_unres_dflt(ctx, (struct lysc_node *)llist, llist->type, dflts[u].str, dflts[u].mod,
                    llist->dflts[orig_count + u], unres);
            LY_CHECK_ERR_RET(ret, free(llist->dflts[orig_count + u]), ret);
            LY_ARRAY_INCREMENT(llist->dflts);
        }
    } else {
        llist->dflts[orig_count] = calloc(1, sizeof **llist->dflts);
        ret = lys_compile_unres_dflt(ctx, (struct lysc_node *)llist, llist->type, dflt->str, dflt->mod,
                llist->dflts[orig_count], unres);
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
                    LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Configuration leaf-list has multiple defaults of the same value \"%s\".",
                            llist->dflts[u]->canonical);
                    lysc_update_path(ctx, NULL, NULL);
                    return LY_EVALID;
                }
            }
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lys_compile_unres_glob(struct ly_ctx *ctx, struct lys_glob_unres *unres)
{
    LY_ERR ret;
    struct lysc_node *node;
    struct lysc_type *type, *typeiter;
    struct lysc_type_leafref *lref;
    struct lysc_ctx cctx = {0};
    LY_ARRAY_COUNT_TYPE v;
    uint32_t i;

    if (unres->recompile) {
        /* recompile all the modules and resolve the new unres instead (during recompilation) */
        unres->recompile = 0;
        return lys_recompile(ctx, 1);
    }

    /* fake compile context */
    cctx.ctx = ctx;
    cctx.path_len = 1;
    cctx.path[0] = '/';

    /* for leafref, we need 2 rounds - first detects circular chain by storing the first referred type (which
     * can be also leafref, in case it is already resolved, go through the chain and check that it does not
     * point to the starting leafref type). The second round stores the first non-leafref type for later data validation. */
    for (i = 0; i < unres->leafrefs.count; ++i) {
        LY_ERR ret = LY_SUCCESS;
        node = unres->leafrefs.objs[i];
        cctx.cur_mod = node->module;
        cctx.pmod = node->module->parsed;

        LOG_LOCSET(node, NULL, NULL, NULL);

        assert(node->nodetype & (LYS_LEAF | LYS_LEAFLIST));
        type = ((struct lysc_node_leaf *)node)->type;
        if (type->basetype == LY_TYPE_LEAFREF) {
            ret = lys_compile_unres_leafref(&cctx, node, (struct lysc_type_leafref *)type, unres);
        } else if (type->basetype == LY_TYPE_UNION) {
            LY_ARRAY_FOR(((struct lysc_type_union *)type)->types, v) {
                if (((struct lysc_type_union *)type)->types[v]->basetype == LY_TYPE_LEAFREF) {
                    lref = (struct lysc_type_leafref *)((struct lysc_type_union *)type)->types[v];
                    ret = lys_compile_unres_leafref(&cctx, node, lref, unres);
                    if (ret) {
                        break;
                    }
                }
            }
        }

        LOG_LOCBACK(1, 0, 0, 0);
        if (ret) {
            return ret;
        }
    }
    while (unres->leafrefs.count) {
        node = unres->leafrefs.objs[unres->leafrefs.count - 1];
        cctx.cur_mod = node->module;
        cctx.pmod = node->module->parsed;

        LOG_LOCSET(node, NULL, NULL, NULL);

        /* store pointer to the real type */
        type = ((struct lysc_node_leaf *)node)->type;
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
        LOG_LOCBACK(1, 0, 0, 0);

        ly_set_rm_index(&unres->leafrefs, unres->leafrefs.count - 1, NULL);
    }

    /* check xpath */
    while (unres->xpath.count) {
        node = unres->xpath.objs[unres->xpath.count - 1];
        cctx.cur_mod = node->module;
        cctx.pmod = node->module->parsed;

        LOG_LOCSET(node, NULL, NULL, NULL);

        ret = lys_compile_unres_xpath(&cctx, node, unres);
        LOG_LOCBACK(1, 0, 0, 0);
        LY_CHECK_RET(ret);

        ly_set_rm_index(&unres->xpath, unres->xpath.count - 1, NULL);
    }

    /* finish incomplete default values compilation */
    while (unres->dflts.count) {
        struct lysc_unres_dflt *r = unres->dflts.objs[unres->dflts.count - 1];
        cctx.cur_mod = r->leaf->module;
        cctx.pmod = r->leaf->module->parsed;

        LOG_LOCSET((struct lysc_node *)r->leaf, NULL, NULL, NULL);

        if (r->leaf->nodetype == LYS_LEAF) {
            ret = lys_compile_unres_leaf_dlft(&cctx, r->leaf, r->dflt, unres);
        } else {
            ret = lys_compile_unres_llist_dflts(&cctx, r->llist, r->dflt, r->dflts, unres);
        }
        LOG_LOCBACK(1, 0, 0, 0);
        LY_CHECK_RET(ret);

        lysc_unres_dflt_free(ctx, r);
        ly_set_rm_index(&unres->dflts, unres->dflts.count - 1, NULL);
    }

    /* some unres items may have been added */
    if (unres->leafrefs.count || unres->xpath.count || unres->dflts.count) {
        return lys_compile_unres_glob(ctx, unres);
    }

    return LY_SUCCESS;
}

void
lys_compile_unres_glob_revert(struct ly_ctx *ctx, struct lys_glob_unres *unres)
{
    uint32_t i;
    struct lys_module *m;

    for (i = 0; i < unres->implementing.count; ++i) {
        m = unres->implementing.objs[i];
        assert(m->implemented);

        /* make the module correctly non-implemented again */
        m->implemented = 0;
        lys_precompile_augments_deviations_revert(ctx, m);
    }

    for (i = 0; i < unres->creating.count; ++i) {
        m = unres->creating.objs[i];

        /* remove the module from the context and free it */
        ly_set_rm(&ctx->list, m, NULL);
        lys_module_free(m, NULL);
    }

    if (unres->implementing.count) {
        /* recompile because some implemented modules are no longer implemented */
        lys_recompile(ctx, 0);
    }
}

void
lys_compile_unres_glob_erase(const struct ly_ctx *ctx, struct lys_glob_unres *unres)
{
    uint32_t i;

    ly_set_erase(&unres->implementing, NULL);
    ly_set_erase(&unres->creating, NULL);
    for (i = 0; i < unres->dflts.count; ++i) {
        lysc_unres_dflt_free(ctx, unres->dflts.objs[i]);
    }
    ly_set_erase(&unres->dflts, NULL);
    ly_set_erase(&unres->xpath, NULL);
    ly_set_erase(&unres->leafrefs, NULL);
}

/**
 * @brief Finish compilation of all the module unres sets in a compile context.
 *
 * @param[in] ctx Compile context with unres sets.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_mod(struct lysc_ctx *ctx)
{
    struct lysc_node *node;
    struct lysc_augment *aug;
    struct lysc_deviation *dev;
    uint32_t i;

    /* remove all disabled nodes */
    for (i = 0; i < ctx->disabled.count; ++i) {
        node = ctx->disabled.snodes[i];
        if (node->flags & LYS_KEY) {
            LOG_LOCSET(node, NULL, NULL, NULL);
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Key \"%s\" is disabled by its if-features.", node->name);
            LOG_LOCBACK(1, 0, 0, 0);
            return LY_EVALID;
        }

        lysc_node_free(ctx->ctx, node, 1);
    }

    /* check that all augments were applied */
    for (i = 0; i < ctx->augs.count; ++i) {
        aug = ctx->augs.objs[i];
        lysc_update_path(ctx, NULL, "{augment}");
        lysc_update_path(ctx, NULL, aug->nodeid->expr);
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Augment target node \"%s\" from module \"%s\" was not found.",
                aug->nodeid->expr, LYSP_MODULE_NAME(aug->nodeid_pmod));
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
    }
    if (ctx->augs.count) {
        return LY_ENOTFOUND;
    }

    /* check that all deviations were applied */
    for (i = 0; i < ctx->devs.count; ++i) {
        dev = ctx->devs.objs[i];
        lysc_update_path(ctx, NULL, "{deviation}");
        lysc_update_path(ctx, NULL, dev->nodeid->expr);
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Deviation(s) target node \"%s\" from module \"%s\" was not found.",
                dev->nodeid->expr, LYSP_MODULE_NAME(dev->dev_pmods[0]));
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
    }
    if (ctx->devs.count) {
        return LY_ENOTFOUND;
    }

    return LY_SUCCESS;
}

/**
 * @brief Erase all the module unres sets in a compile context.
 *
 * @param[in] ctx Compile context with unres sets.
 * @param[in] error Whether the compilation finished with an error or not.
 */
static void
lys_compile_unres_mod_erase(struct lysc_ctx *ctx, ly_bool error)
{
    uint32_t i;

    ly_set_erase(&ctx->groupings, NULL);
    ly_set_erase(&ctx->tpdf_chain, NULL);
    ly_set_erase(&ctx->disabled, NULL);

    if (!error) {
        /* there can be no leftover deviations or augments */
        LY_CHECK_ERR_RET(ctx->augs.count, LOGINT(ctx->ctx), );
        LY_CHECK_ERR_RET(ctx->devs.count, LOGINT(ctx->ctx), );

        ly_set_erase(&ctx->augs, NULL);
        ly_set_erase(&ctx->devs, NULL);
        ly_set_erase(&ctx->uses_augs, NULL);
        ly_set_erase(&ctx->uses_rfns, NULL);
    } else {
        for (i = 0; i < ctx->augs.count; ++i) {
            lysc_augment_free(ctx->ctx, ctx->augs.objs[i]);
        }
        ly_set_erase(&ctx->augs, NULL);
        for (i = 0; i < ctx->devs.count; ++i) {
            lysc_deviation_free(ctx->ctx, ctx->devs.objs[i]);
        }
        ly_set_erase(&ctx->devs, NULL);
        for (i = 0; i < ctx->uses_augs.count; ++i) {
            lysc_augment_free(ctx->ctx, ctx->uses_augs.objs[i]);
        }
        ly_set_erase(&ctx->uses_augs, NULL);
        for (i = 0; i < ctx->uses_rfns.count; ++i) {
            lysc_refine_free(ctx->ctx, ctx->uses_rfns.objs[i]);
        }
        ly_set_erase(&ctx->uses_rfns, NULL);
    }
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
        LY_CHECK_RET(lys_compile_identities_derived(ctx, ctx->cur_mod->parsed->identities, &ctx->cur_mod->identities));
    }
    lysc_update_path(ctx, NULL, "{submodule}");
    LY_ARRAY_FOR(ctx->cur_mod->parsed->includes, u) {

        submod = ctx->cur_mod->parsed->includes[u].submodule;
        if (submod->identities) {
            lysc_update_path(ctx, NULL, submod->name);
            LY_CHECK_RET(lys_compile_identities_derived(ctx, submod->identities, &ctx->cur_mod->identities));
            lysc_update_path(ctx, NULL, NULL);
        }
    }
    lysc_update_path(ctx, NULL, NULL);

    return LY_SUCCESS;
}

LY_ERR
lys_recompile(struct ly_ctx *ctx, ly_bool log)
{
    uint32_t idx;
    struct lys_module *mod;
    struct lys_glob_unres unres = {0};
    LY_ERR ret = LY_SUCCESS;
    uint32_t prev_lo = 0;

    if (!log) {
        /* recompile, must succeed because the modules were already compiled; hide messages because any
         * warnings were already printed, are not really relevant, and would hide the real error */
        prev_lo = ly_log_options(0);
    }

    /* free all the modules */
    for (idx = 0; idx < ctx->list.count; ++idx) {
        mod = ctx->list.objs[idx];
        if (mod->compiled) {
            /* free the module */
            lysc_module_free(mod->compiled, NULL);
            mod->compiled = NULL;
        }

        /* free precompiled iffeatures */
        lys_free_feature_iffeatures(mod->parsed);
    }

    /* recompile all the modules */
    for (idx = 0; idx < ctx->list.count; ++idx) {
        mod = ctx->list.objs[idx];
        if (!mod->implemented || mod->compiled) {
            /* nothing to do */
            continue;
        }

        /* recompile */
        ret = lys_compile(mod, 0, &unres);
        if (ret) {
            if (!log) {
                LOGERR(mod->ctx, ret, "Recompilation of module \"%s\" failed.", mod->name);
            }
            goto cleanup;
        }
    }

    /* resolve global unres */
    LY_CHECK_GOTO(ret = lys_compile_unres_glob(ctx, &unres), cleanup);

cleanup:
    if (!log) {
        ly_log_options(prev_lo);
    }
    lys_compile_unres_glob_erase(ctx, &unres);
    return ret;
}

LY_ERR
lys_compile(struct lys_module *mod, uint32_t options, struct lys_glob_unres *unres)
{
    struct lysc_ctx ctx = {0};
    struct lysc_module *mod_c;
    struct lysp_module *sp;
    struct lysp_submodule *submod;
    struct lysp_node *pnode;
    struct lysp_node_grp *grp;
    LY_ARRAY_COUNT_TYPE u;
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
    ctx.unres = unres;

    mod->compiled = mod_c = calloc(1, sizeof *mod_c);
    LY_CHECK_ERR_RET(!mod_c, LOGMEM(mod->ctx), LY_EMEM);
    mod_c->mod = mod;

    /* process imports */
    LY_ARRAY_FOR(sp->imports, u) {
        LY_CHECK_GOTO(ret = lys_compile_import(&ctx, &sp->imports[u]), error);
    }

    /* identities */
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

    /* top-level RPCs */
    LY_LIST_FOR((struct lysp_node *)sp->rpcs, pnode) {
        LY_CHECK_GOTO(ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL), error);
    }

    /* top-level notifications */
    LY_LIST_FOR((struct lysp_node *)sp->notifs, pnode) {
        LY_CHECK_GOTO(ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL), error);
    }

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

        LY_LIST_FOR((struct lysp_node *)submod->rpcs, pnode) {
            ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL);
            LY_CHECK_GOTO(ret, error);
        }

        LY_LIST_FOR((struct lysp_node *)submod->notifs, pnode) {
            ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL);
            LY_CHECK_GOTO(ret, error);
        }

        COMPILE_EXTS_GOTO(&ctx, submod->exts, mod_c->exts, mod_c, LYEXT_PAR_MODULE, ret, error);
    }
    ctx.pmod = sp;

    /* validate non-instantiated groupings from the parsed schema,
     * without it we would accept even the schemas with invalid grouping specification */
    ctx.options |= LYS_COMPILE_GROUPING;
    LY_LIST_FOR(sp->groupings, grp) {
        if (!(grp->flags & LYS_USED_GRP)) {
            LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, NULL, grp), error);
        }
    }
    LY_LIST_FOR(sp->data, pnode) {
        LY_LIST_FOR((struct lysp_node_grp *)lysp_node_groupings(pnode), grp) {
            if (!(grp->flags & LYS_USED_GRP)) {
                LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, pnode, grp), error);
            }
        }
    }
    LY_ARRAY_FOR(sp->includes, u) {
        submod = sp->includes[u].submodule;
        ctx.pmod = (struct lysp_module *)submod;

        LY_LIST_FOR(submod->groupings, grp) {
            if (!(grp->flags & LYS_USED_GRP)) {
                LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, NULL, grp), error);
            }
        }
        LY_LIST_FOR(submod->data, pnode) {
            LY_LIST_FOR((struct lysp_node_grp *)lysp_node_groupings(pnode), grp) {
                if (!(grp->flags & LYS_USED_GRP)) {
                    LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, pnode, grp), error);
                }
            }
        }
    }
    ctx.pmod = sp;

    LOG_LOCBACK(0, 0, 1, 0);

    /* finish compilation for all unresolved module items in the context */
    LY_CHECK_GOTO(ret = lys_compile_unres_mod(&ctx), error);

    lys_compile_unres_mod_erase(&ctx, 0);
    return LY_SUCCESS;

error:
    LOG_LOCBACK(0, 0, 1, 0);
    lys_precompile_augments_deviations_revert(ctx.ctx, mod);
    lys_compile_unres_mod_erase(&ctx, 1);
    lysc_module_free(mod_c, NULL);
    mod->compiled = NULL;

    return ret;
}
