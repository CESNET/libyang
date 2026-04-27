/**
 * @file semver.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang extension plugin - ietf-yang-semver
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* strndup */

#include "semver.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "libyang.h"
#include "ly_common.h"
#include "plugins_exts.h"
#include "plugins_internal.h"
#include "tree_schema_internal.h"

struct lyplg_ext_semver_item {
    const char *revision_date;
    const struct lys_ext_instance_semver *semver;
    const char *semver_str;
    ly_bool has_nbc_ext;
};

LIBYANG_API_DEF const struct lys_ext_instance_semver *
lys_semver_get(const struct lys_module *mod, const char **semver_str)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lysc_ext_instance *ext;
    const struct lysp_revision *rev;
    const struct lysp_ext_instance *extp;
    const char *mod_name, *name;

    if (semver_str) {
        *semver_str = NULL;
    }

    if (!mod || !mod->revision) {
        return NULL;
    }

    if (mod->compiled) {
        /* find the compiled extension, if any */
        LY_ARRAY_FOR(mod->compiled->exts, u) {
            ext = &mod->compiled->exts[u];

            if (ext->def && !strcmp(ext->def->module->name, "ietf-yang-semver") && !strcmp(ext->def->name, "version")) {
                if (semver_str) {
                    *semver_str = ext->argument;
                }
                return ext->compiled;
            }
        }

        /* no version if not compiled */
        return NULL;
    }

    if (mod->parsed) {
        /* find the last revision */
        rev = NULL;
        LY_ARRAY_FOR(mod->parsed->revs, u) {
            if (strcmp(mod->parsed->revs[u].date, mod->revision)) {
                continue;
            }

            rev = &mod->parsed->revs[u];
            break;
        }
        assert(rev);

        /* find the version extension instance */
        LY_ARRAY_FOR(rev->exts, u) {
            extp = &rev->exts[u];
            lysp_nodeid_find_module(mod->ctx, extp->name, extp->format, extp->prefix_data, &mod_name, &name);

            if (!strcmp(mod_name, "ietf-yang-semver") && !strcmp(name, "version")) {
                if (semver_str) {
                    *semver_str = extp->argument;
                }
                return extp->parsed;
            }
        }
    }

    return NULL;
}

/**
 * @brief Validate a semantic version.
 *
 * @param[in] ctx Context with shared data and the compiled pattern to use.
 * @param[in] version Version to check.
 * @param[in] version_len Length of @p version.
 * @param[in] bare Set if the version should have only MAJOR.MINOR.PATCH format.
 * @return LY_ERR value.
 */
static LY_ERR
semver_check_string(const struct ly_ctx *ctx, const char *version, uint32_t version_len, ly_bool bare)
{
    LY_ERR r;
    struct ly_ctx_shared_data *sdata;
    struct ly_err_item *err = NULL;
    const char *ptr;

    assert(ctx && version && version_len);

    sdata = ly_ctx_shared_data_get(ctx);
    assert(sdata && sdata->semver_pattern);

    /* length */
    if ((version_len < LY_SEMVER_VERSION_MIN_LEN) || (version_len > LY_SEMVER_VERSION_MAX_LEN)) {
        return LY_ENOT;
    }

    /* pattern */
    r = ly_pat_match(sdata->semver_pattern, LY_SEMVER_VERSION_PATTERN, 0, version, version_len, &err);
    ly_err_free(err);
    LY_CHECK_RET(r);

    if (bare) {
        /* check there are no metadata after semver */
        ptr = version;
        while (isdigit(ptr[0])) {
            ++ptr;
        }

        assert(ptr[0] == '.');
        ++ptr;
        while (isdigit(ptr[0])) {
            ++ptr;
        }

        assert(ptr[0] == '.');
        ++ptr;
        while (isdigit(ptr[0])) {
            ++ptr;
        }

        if (ptr[0]) {
            return LY_ENOT;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse a validated semantic version into separate fields.
 *
 * @param[in] version String version to parse.
 * @param[in] version_len Length of @p version.
 * @param[out] semver Created semver structure.
 * @return LY_ERR value.
 */
static LY_ERR
semver_parse_string(const char *version, uint32_t version_len, struct lys_ext_instance_semver **semver)
{
    char *ptr, *ptr2;
    struct lys_ext_instance_semver *s;

    *semver = calloc(1, sizeof **semver);
    LY_CHECK_ERR_RET(!*semver, LOGMEM(NULL), LY_EMEM);
    s = *semver;

    /* major */
    s->major = strtol(version, &ptr, 10);
    assert(ptr && (ptr[0] == '.'));
    ++ptr;

    /* minor */
    s->minor = strtol(ptr, &ptr, 10);
    assert(ptr && (ptr[0] == '.'));
    ++ptr;

    /* patch */
    s->patch = strtol(ptr, &ptr, 10);

    if (ptr[0] == '_') {
        /* compat */
        ++ptr;
        if (ptr[0] == 'c') {
            assert(!strncmp(ptr, "compatible", 10));
            s->compat = LYS_EXT_SEMVER_COMPAT_COMPATIBLE;
            ptr += 10;
        } else {
            assert(!strncmp(ptr, "non_compatible", 14));
            s->compat = LYS_EXT_SEMVER_COMPAT_NON_COMPATIBLE;
            ptr += 14;
        }
    }

    if (ptr[0] == '-') {
        /* pre-release meta */
        ++ptr;
        for (ptr2 = ptr; (ptr2 - ptr < version_len) && (ptr2[0] != '+'); ++ptr2) {}

        s->pre_release_meta = strndup(ptr, ptr2 - ptr);
        LY_CHECK_ERR_RET(!s->pre_release_meta, LOGMEM(NULL), LY_EMEM);
        ptr = ptr2;
    }

    if (ptr[0] == '+') {
        /* build meta */
        ++ptr;
        for (ptr2 = ptr; (ptr2 - ptr < version_len); ++ptr2) {}

        s->build_meta = strndup(ptr, ptr2 - ptr);
        LY_CHECK_ERR_RET(!s->build_meta, LOGMEM(NULL), LY_EMEM);
        ptr = ptr2;
    }

    return LY_SUCCESS;
}

LY_ERR
lyplg_ext_semver_parse(const struct ly_ctx *ctx, const char *version, uint32_t version_len, ly_bool bare,
        struct lys_ext_instance_semver **semver)
{
    if (!ctx || !version) {
        return LY_EINVAL;
    }

    if (!version_len) {
        version_len = strlen(version);
    }

    /* check */
    LY_CHECK_RET(semver_check_string(ctx, version, version_len, bare));

    if (!semver) {
        /* matched, do not parse */
        return LY_SUCCESS;
    }

    /* parse */
    LY_CHECK_RET(semver_parse_string(version, version_len, semver));

    return LY_SUCCESS;
}

void
lyplg_ext_semver_free(struct lys_ext_instance_semver *semver)
{
    if (!semver) {
        return;
    }

    free(semver->pre_release_meta);
    free(semver->build_meta);
    free(semver);
}

/**
 * @brief Check whether there is rev:non-backwards-comaptible extension in a revision.
 *
 * @param[in] ctx Context to use.
 * @param[in] rev Revision to check.
 * @return 1 if nbc ext instance has been found;
 * @return 0 otherwise.
 */
static ly_bool
semver_has_nbc_ext(const struct ly_ctx *ctx, const struct lysp_revision *rev)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lysp_ext_instance *extp;
    const char *mod_name, *name;

    LY_ARRAY_FOR(rev->exts, u) {
        extp = &rev->exts[u];
        lysp_nodeid_find_module(ctx, extp->name, extp->format, extp->prefix_data, &mod_name, &name);

        if (!strcmp(mod_name, "ietf-yang-revisions") && !strcmp(name, "non-backwards-compatible")) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Perform all the version checks with a collected array of all the versions.
 *
 * @param[in] pctx Parse context.
 * @param[in] ext Parsed ext instance.
 * @param[in] items Collected previous versions.
 * @param[in] count Count of @p items.
 * @return LY_ERR value.
 */
static LY_ERR
semver_check_prev_array(struct lysp_ctx *pctx, struct lysp_ext_instance *ext, struct lyplg_ext_semver_item *items,
        uint32_t count)
{
    const struct lys_ext_instance_semver *semver;
    const char *revision_date;
    ly_bool has_nbc_ext;
    uint32_t i;

    /* learn about the current version ext instance */
    semver = ext->parsed;
    revision_date = ((struct lysp_revision *)ext->parent)->date;
    has_nbc_ext = semver_has_nbc_ext(PARSER_CTX(pctx), ext->parent);

    for (i = 0; i < count; ++i) {
        if (items[i].semver->major != semver->major) {
            /* not the same MAJOR version, nothing to check */
            continue;
        }

        if ((items[i].semver->minor <= semver->minor) && has_nbc_ext &&
                (semver->compat != LYS_EXT_SEMVER_COMPAT_NON_COMPATIBLE)) {
            /* missing NBC MAJOR version increase #1 */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                    "Missing new major version with NBC changes in \"%s\" revision %s compared to \"%s\" revision %s.",
                    ext->argument, revision_date, items[i].semver_str, items[i].revision_date);
            return LY_EVALID;
        } else if ((items[i].semver->minor >= semver->minor) && items[i].has_nbc_ext &&
                (items[i].semver->compat != LYS_EXT_SEMVER_COMPAT_NON_COMPATIBLE)) {
            /* missing NBC MAJOR version increase #2 */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                    "Missing new major version with NBC changes in \"%s\" revision %s compared to \"%s\" revision %s.",
                    items[i].semver_str, items[i].revision_date, ext->argument, revision_date);
            return LY_EVALID;
        } else if (items[i].semver->minor != semver->minor) {
            /* not the same MINOR version, nothing else to check */
            continue;
        }

        if (items[i].semver->patch == semver->patch) {
            /* non-unique version */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                    "Duplicate version \"%s\" revision %s and \"%s\" revision %s.", ext->argument, revision_date,
                    items[i].semver_str, items[i].revision_date);
            return LY_EVALID;
        } else if (semver->major == 0) {
            /* pre-release version, nothing else to check */
            continue;
        }

        if ((items[i].semver->patch < semver->patch) && (strcmp(items[i].revision_date, revision_date) > 0)) {
            /* higher PATCH version */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                    "Invalid higher version \"%s\" revision %s compared to \"%s\" revision %s.", ext->argument, revision_date,
                    items[i].semver_str, items[i].revision_date);
            return LY_EVALID;
        } else if ((items[i].semver->patch < semver->patch) && (items[i].semver->compat > semver->compat)) {
            /* sticks COMPAT modifier #1 */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                    "Invalid compat change in version \"%s\" revision %s compared to \"%s\" revision %s.",
                    ext->argument, revision_date, items[i].semver_str, items[i].revision_date);
            return LY_EVALID;
        } else if ((items[i].semver->patch > semver->patch) && (items[i].semver->compat < semver->compat)) {
            /* sticks COMPAT modifier #2 */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                    "Invalid compat change in version \"%s\" revision %s compared to \"%s\" revision %s.",
                    items[i].semver_str, items[i].revision_date, ext->argument, revision_date);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Check version with all the previous module revision statements with versions.
 *
 * @param[in] pctx Parse context.
 * @param[in] ext Parsed ext instance.
 * @return LY_ERR value.
 */
static LY_ERR
semver_check_prev(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    const struct lysp_module *pmod;
    const struct lysp_revision *rev;
    const struct lysp_ext_instance *extp, *extp_ver;
    struct lyplg_ext_semver_item *semvers = NULL;
    const char *mod_name, *name;
    uint32_t semver_count = 0;
    LY_ARRAY_COUNT_TYPE u, v;
    LY_ERR rc = LY_SUCCESS;
    ly_bool has_nbc_ext;

    /* collect all the versions */
    pmod = PARSER_CUR_PMOD(pctx);
    LY_ARRAY_FOR(pmod->revs, u) {
        rev = &pmod->revs[u];
        if (ext->parent == rev) {
            /* skip this revision */
            continue;
        }

        extp_ver = NULL;
        has_nbc_ext = 0;
        LY_ARRAY_FOR(rev->exts, v) {
            extp = &rev->exts[v];
            lysp_nodeid_find_module(PARSER_CTX(pctx), extp->name, extp->format, extp->prefix_data, &mod_name, &name);

            if (!strcmp(mod_name, "ietf-yang-semver") && !strcmp(name, "version")) {
                if (!extp->parsed) {
                    /* collect only parsed revisions */
                    break;
                }

                extp_ver = extp;
            } else if (!strcmp(mod_name, "ietf-yang-revisions") && !strcmp(name, "non-backwards-compatible")) {
                has_nbc_ext = 1;
            }

            if (extp_ver && has_nbc_ext) {
                /* all exts found */
                break;
            }
        }

        if (extp_ver) {
            /* new revision with version */
            semvers = ly_realloc(semvers, (semver_count + 1) * sizeof *semvers);
            if (!semvers) {
                lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
                rc = LY_EMEM;
                goto cleanup;
            }
            semvers[semver_count].revision_date = rev->date;
            semvers[semver_count].semver = extp_ver->parsed;
            semvers[semver_count].semver_str = extp_ver->argument;
            semvers[semver_count].has_nbc_ext = has_nbc_ext;
            ++semver_count;
        }
    }

    /* perform all the checks */
    LY_CHECK_GOTO(rc = semver_check_prev_array(pctx, ext, semvers, semver_count), cleanup);

cleanup:
    free(semvers);
    return rc;
}

/**
 * @brief Parse ietf-yang-semver version extension.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
version_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    const struct lysp_revision *rev;
    LY_ARRAY_COUNT_TYPE u;

    /* check that the extension is instantiated at an allowed place - revision */
    if (ext->parent_stmt != LY_STMT_REVISION) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is allowed only in a \"revision\" statement, "
                "but it is placed in \"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    /* check argument */
    assert(ext->argument);
    if (semver_check_string(PARSER_CTX(pctx), ext->argument, strlen(ext->argument), 0)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s argument yang-semantic-version \"%s\" invalid.",
                ext->name, ext->argument);
        return LY_EVALID;
    }

    /* check for duplication */
    rev = ext->parent;
    LY_ARRAY_FOR(rev->exts, u) {
        if ((&rev->exts[u] != ext) && (rev->exts[u].name == ext->name)) {
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is instantiated multiple times.", ext->name);
            return LY_EVALID;
        }
    }

    /* parse */
    LY_CHECK_RET(semver_parse_string(ext->argument, strlen(ext->argument), (struct lys_ext_instance_semver **)&ext->parsed));

    /* version check compared to previous versions */
    LY_CHECK_RET(semver_check_prev(pctx, ext));

    return LY_SUCCESS;
}

/**
 * @brief Compile ietf-yang-semver version extension.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
version_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *extp, struct lysc_ext_instance *ext)
{
    const struct lys_module *mod;
    struct lysp_revision *rev;
    const struct lys_ext_instance_semver *orig_semver;
    struct lys_ext_instance_semver *semver;

    /* get current module */
    mod = lyplg_ext_compile_get_cur_mod(cctx);

    /* check we have the extension in the last revision */
    assert(extp->parent_stmt == LY_STMT_REVISION);
    rev = extp->parent;
    if (strcmp(rev->date, mod->revision)) {
        /* do not compile */
        return LY_ENOT;
    }

    /* make a copy of the semver structure */
    ext->compiled = calloc(1, sizeof *semver);
    if (!ext->compiled) {
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
        return LY_EMEM;
    }
    semver = ext->compiled;
    orig_semver = extp->parsed;

    *semver = *orig_semver;
    semver->pre_release_meta = NULL;
    semver->build_meta = NULL;

    if (orig_semver->pre_release_meta) {
        semver->pre_release_meta = strdup(orig_semver->pre_release_meta);
        if (!semver->pre_release_meta) {
            lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
            return LY_EMEM;
        }
    }
    if (orig_semver->build_meta) {
        semver->build_meta = strdup(orig_semver->build_meta);
        if (!semver->build_meta) {
            lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
            return LY_EMEM;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Pfree ietf-yang-semver version extension.
 *
 * Implementation of ::lyplg_ext_parse_free_clb callback set as lyext_plugin::pfree.
 */
static void
version_pfree(const struct ly_ctx *UNUSED(ctx), struct lysp_ext_instance *ext)
{
    lyplg_ext_semver_free(ext->parsed);
}

/**
 * @brief Cfree ietf-yang-semver version extension.
 *
 * Implementation of ::lyplg_ext_compile_free_clb callback set as lyext_plugin::cfree.
 */
static void
structure_cfree(const struct ly_ctx *UNUSED(ctx), struct lysc_ext_instance *ext)
{
    lyplg_ext_semver_free(ext->compiled);
}

/**
 * @brief Parse ietf-yang-semver recommended-min-version extension.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
min_version_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    struct lysp_import *imp;
    LY_ARRAY_COUNT_TYPE u;

    /* check that the extension is instantiated at an allowed place - import */
    if (ext->parent_stmt != LY_STMT_IMPORT) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is allowed only in an \"import\" statement, "
                "but it is placed in \"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    /* check argument */
    assert(ext->argument);
    LY_CHECK_RET(semver_check_string(PARSER_CTX(pctx), ext->argument, strlen(ext->argument), 1));

    /* check for duplication */
    imp = ext->parent;
    LY_ARRAY_FOR(imp->exts, u) {
        if ((&imp->exts[u] != ext) && (imp->exts[u].name == ext->name)) {
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is instantiated multiple times.", ext->name);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Plugin descriptions for the ietf-yang-semver version and recommended-min-version extensions
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_semver[] = {
    {
        .module = "ietf-yang-semver",
        .revision = NULL,
        .name = "version",

        .plugin.id = "ly2 semver",
        .plugin.parse = version_parse,
        .plugin.compile = version_compile,
        .plugin.printer_info = NULL,
        .plugin.node_xpath = NULL,
        .plugin.snode_xpath = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = version_pfree,
        .plugin.cfree = structure_cfree,
        .plugin.compiled_size = NULL,
        .plugin.compiled_print = NULL
    }, {
        .module = "ietf-yang-semver",
        .revision = NULL,
        .name = "recommended-min-version",

        .plugin.id = "ly2 semver",
        .plugin.parse = min_version_parse,
        .plugin.compile = NULL,
        .plugin.printer_info = NULL,
        .plugin.node_xpath = NULL,
        .plugin.snode_xpath = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL,
        .plugin.compiled_size = NULL,
        .plugin.compiled_print = NULL
    },
    {0} /* terminating zeroed item */
};
