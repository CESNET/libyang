/**
 * @file plugins.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulate with the type and extension plugins.
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "plugins.h"
#include "plugins_internal.h"

#include <assert.h>
#include <dirent.h>
#ifndef STATIC
# include <dlfcn.h>
#endif
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ly_common.h"
#include "ly_config.h"
#include "plugins_exts.h"
#include "plugins_types.h"
#include "set.h"

/*
 * internal type plugins records
 */
extern const struct lyplg_type_record plugins_binary[];
extern const struct lyplg_type_record plugins_bits[];
extern const struct lyplg_type_record plugins_boolean[];
extern const struct lyplg_type_record plugins_decimal64[];
extern const struct lyplg_type_record plugins_empty[];
extern const struct lyplg_type_record plugins_enumeration[];
extern const struct lyplg_type_record plugins_identityref[];
extern const struct lyplg_type_record plugins_instanceid[];
extern const struct lyplg_type_record plugins_integer[];
extern const struct lyplg_type_record plugins_leafref[];
extern const struct lyplg_type_record plugins_string[];
extern const struct lyplg_type_record plugins_union[];

/*
 * yang
 */
extern const struct lyplg_type_record plugins_instanceid_keys[];

/*
 * ietf-inet-types
 */
extern const struct lyplg_type_record plugins_ipv4_address[];
extern const struct lyplg_type_record plugins_ipv4_address_no_zone[];
extern const struct lyplg_type_record plugins_ipv6_address[];
extern const struct lyplg_type_record plugins_ipv6_address_no_zone[];
extern const struct lyplg_type_record plugins_ipv4_prefix[];
extern const struct lyplg_type_record plugins_ipv6_prefix[];

/*
 * ietf-yang-types
 */
extern const struct lyplg_type_record plugins_date_and_time[];
extern const struct lyplg_type_record plugins_hex_string[];
extern const struct lyplg_type_record plugins_xpath10[];

/*
 * ietf-netconf-acm
 */
extern const struct lyplg_type_record plugins_node_instanceid[];

/*
 * libnetconf2-netconf-server
 */
extern const struct lyplg_type_record plugins_time_period[];

/*
 * lyds_tree
 */
extern const struct lyplg_type_record plugins_lyds_tree[];

/*
 * internal extension plugins records
 */
extern struct lyplg_ext_record plugins_metadata[];
extern struct lyplg_ext_record plugins_nacm[];
extern struct lyplg_ext_record plugins_yangdata[];
extern struct lyplg_ext_record plugins_schema_mount[];
extern struct lyplg_ext_record plugins_structure[];

static pthread_mutex_t plugins_guard = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Counter for currently present contexts able to refer to the loaded plugins.
 *
 * Plugins are shared among all the created contexts. They are loaded with the creation of the very first context and
 * unloaded with the destroy of the last context. Therefore, to reload the list of plugins, all the contexts must be
 * destroyed and with the creation of a first new context after that, the plugins will be reloaded.
 */
static uint32_t context_refcount;

/**
 * @brief Record describing an implemented extension.
 *
 * Matches ::lyplg_ext_record and ::lyplg_type_record
 */
struct lyplg_record {
    const char *module;          /**< name of the module where the extension/type is defined */
    const char *revision;        /**< optional module revision - if not specified, the plugin applies to any revision,
                                      which is not an optimal approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same pointer to the plugin functions. The
                                      only valid use case for the NULL revision is the case the module has no revision. */
    const char *name;            /**< name of the extension/typedef */
    int8_t plugin[];             /**< specific plugin type's data - ::lyplg_ext or ::lyplg_type */
};

#ifndef STATIC
static struct ly_set plugins_handlers;
#endif
struct ly_set ly_plugins_types;
struct ly_set ly_plugins_extensions;

/* global counters for the number of static plugins */
uint32_t ly_static_type_plugins_count;
uint32_t ly_static_ext_plugins_count;

LIBYANG_API_DEF struct lyplg_type *
lysc_get_type_plugin(uintptr_t plugin_ref)
{
    if (!plugin_ref) {
        return NULL;
    }

    if (plugin_ref <= ly_plugins_types.count) {
        /* plugin is static, fetch it from the global list */
        return &((struct lyplg_type_record *)ly_plugins_types.objs[plugin_ref - 1])->plugin;
    } else {
        /* plugin is dynamic, return the pointer */
        return (struct lyplg_type *)plugin_ref;
    }
}

LIBYANG_API_DEF struct lyplg_ext *
lysc_get_ext_plugin(uintptr_t plugin_ref)
{
    if (!plugin_ref) {
        return NULL;
    }

    if (plugin_ref <= ly_plugins_extensions.count) {
        /* plugin is static, fetch it from the global list */
        return &((struct lyplg_ext_record *)ly_plugins_extensions.objs[plugin_ref - 1])->plugin;
    } else {
        /* plugin is dynamic, return the pointer */
        return (struct lyplg_ext *)plugin_ref;
    }
}

/**
 * @brief Iterate over list of loaded plugins of the given @p type.
 *
 * @param[in] ctx Context for which the plugin is searched for.
 * @param[in] type Type of the plugins to iterate.
 * @param[in,out] index Iterator - set to 0 for the first call.
 * @return The plugin records, NULL if no more record is available.
 */
static struct lyplg_record *
plugins_iter(const struct ly_ctx *ctx, enum LYPLG type, uint32_t *index)
{
    const struct ly_set *plugins;

    assert(index);

    if (type == LYPLG_EXTENSION) {
        plugins = ctx ? &ctx->plugins_extensions : &ly_plugins_extensions;
    } else {
        plugins = ctx ? &ctx->plugins_types : &ly_plugins_types;
    }

    if (*index == plugins->count) {
        return NULL;
    }

    *index += 1;
    return plugins->objs[*index - 1];
}

/**
 * @brief Find the give @p type plugin record.
 *
 * @param[in] ctx Context for which the plugin record is searched for, NULL for built-in plugins.
 * @param[in] type Type of the plugin record to find.
 * @param[in] module Module name of the plugin record.
 * @param[in] revision Revision of the @p module.
 * @param[in] name Name of the plugin record.
 * @param[out] record_idx Optional output parameter to get the index of the found plugin record.
 * @return Found plugin record or NULL if not found.
 */
static void *
lyplg_record_find(const struct ly_ctx *ctx, enum LYPLG type, const char *module, const char *revision, const char *name, uint32_t *record_idx)
{
    uint32_t i = 0;
    struct lyplg_record *item;

    assert(module);
    assert(name);

    if (record_idx) {
        *record_idx = 0;
    }

    while ((item = plugins_iter(ctx, type, &i)) != NULL) {
        if (!strcmp(item->module, module) && !strcmp(item->name, name)) {
            if (item->revision && revision && strcmp(item->revision, revision)) {
                continue;
            } else if (!revision && item->revision) {
                continue;
            }

            if (record_idx) {
                /* return the record idx, -1 because the iterator is already increased */
                *record_idx = i - 1;
            }

            return item;
        }
    }

    return NULL;
}

/**
 * @brief Find the plugin of the given @p type.
 *
 * @param[in] ctx Context for which the plugin record is searched for, NULL for built-in plugins.
 * @param[in] type Type of the plugin record to find.
 * @param[in] module Module name of the plugin.
 * @param[in] revision Revision of the @p module.
 * @param[in] name Name of the plugin.
 * @return Reference to the callbacks plugin structure. Use ::LYSC_GET_TYPE_PLG()
 * or ::LYSC_GET_EXT_PLG() on the returned value to get the actual plugin. 0 if not found.
 */
static uintptr_t
lyplg_plugin_find(const struct ly_ctx *ctx, enum LYPLG type, const char *module, const char *revision, const char *name)
{
    struct lyplg_type_record *record = NULL;
    uint32_t record_idx = 0, static_plugin_count;

    if (ctx) {
        /* try to find context specific plugin */
        record = lyplg_record_find(ctx, type, module, revision, name, &record_idx);
        if (record) {
            /* plugin found in the context, hence it is dynamic so return the ptr to it */
            return (uintptr_t)&record->plugin;
        }
    }

    /* try to find shared plugin */
    record = lyplg_record_find(NULL, type, module, revision, name, &record_idx);
    if (!record) {
        /* not found */
        return 0;
    }

    static_plugin_count = (type == LYPLG_TYPE) ? ly_static_type_plugins_count : ly_static_ext_plugins_count;
    if (record_idx < static_plugin_count) {
        /* static plugin, return an index with an offset of +1 in order to keep 0 as an invalid index (a NULL ptr) */
        return record_idx + 1;
    } else {
        /* dynamic plugin, return the pointer to it */
        return (uintptr_t)&record->plugin;
    }
}

uintptr_t
lyplg_type_plugin_find(const struct ly_ctx *ctx, const char *module, const char *revision, const char *name)
{
    return lyplg_plugin_find(ctx, LYPLG_TYPE, module, revision, name);
}

uintptr_t
lyplg_ext_plugin_find(const struct ly_ctx *ctx, const char *module, const char *revision, const char *name)
{
    return lyplg_plugin_find(ctx, LYPLG_EXTENSION, module, revision, name);
}

/**
 * @brief Insert the provided extension plugin records into the internal set of extension plugins for use by libyang.
 *
 * @param[in] ctx The context to which the plugin should be associated with. If NULL, the plugin is considered to be shared
 * between all existing contexts.
 * @param[in] type The type of plugins records
 * @param[in] recs An array of plugin records provided by the plugin implementation. The array must be terminated by a zeroed
 * record.
 * @return LY_SUCCESS in case of success
 * @return LY_EINVAL for invalid information in @p recs.
 * @return LY_EMEM in case of memory allocation failure.
 */
static LY_ERR
plugins_insert(struct ly_ctx *ctx, enum LYPLG type, const void *recs)
{
    struct ly_set *plugins;

    if (!recs) {
        return LY_SUCCESS;
    }

    if (type == LYPLG_EXTENSION) {
        const struct lyplg_ext_record *rec = (const struct lyplg_ext_record *)recs;

        plugins = ctx ? &ctx->plugins_extensions : &ly_plugins_extensions;

        for (uint32_t i = 0; rec[i].name; i++) {
            LY_CHECK_RET(ly_set_add(plugins, (void *)&rec[i], 0, NULL));
        }
    } else { /* LYPLG_TYPE */
        const struct lyplg_type_record *rec = (const struct lyplg_type_record *)recs;

        plugins = ctx ? &ctx->plugins_types : &ly_plugins_types;

        for (uint32_t i = 0; rec[i].name; i++) {
            LY_CHECK_RET(ly_set_add(plugins, (void *)&rec[i], 0, NULL));
        }
    }

    return LY_SUCCESS;
}

#ifndef STATIC

static void
lyplg_close_cb(void *handle)
{
    dlclose(handle);
}

static void
lyplg_clean_(void)
{
    if (--context_refcount) {
        /* there is still some other context, do not remove the plugins */
        return;
    }

    ly_set_erase(&ly_plugins_types, NULL);
    ly_set_erase(&ly_plugins_extensions, NULL);
    ly_set_erase(&plugins_handlers, lyplg_close_cb);
    ly_static_type_plugins_count = ly_static_ext_plugins_count = 0;
}

#endif

void
lyplg_clean(void)
{
#ifndef STATIC
    pthread_mutex_lock(&plugins_guard);
    lyplg_clean_();
    pthread_mutex_unlock(&plugins_guard);
#endif
}

#ifndef STATIC

/**
 * @brief Just a variadic data to cover extension and type plugins by a single ::plugins_load() function.
 *
 * The values are taken from ::LY_PLUGINS_EXTENSIONS and ::LYPLG_TYPES macros.
 */
static const struct {
    const char *id;          /**< string identifier: type/extension */
    const char *apiver_var;  /**< expected variable name holding API version value */
    const char *plugins_var; /**< expected variable name holding plugin records */
    const char *envdir;      /**< environment variable containing directory with the plugins */
    const char *dir;         /**< default directory with the plugins (has less priority than envdir) */
    uint32_t apiver;         /**< expected API version */
} plugins_load_info[] = {
    {   /* LYPLG_TYPE */
        .id = "type",
        .apiver_var = "plugins_types_apiver__",
        .plugins_var = "plugins_types__",
        .envdir = "LIBYANG_TYPES_PLUGINS_DIR",
        .dir = LYPLG_TYPE_DIR,
        .apiver = LYPLG_TYPE_API_VERSION
    }, {/* LYPLG_EXTENSION */
        .id = "extension",
        .apiver_var = "plugins_extensions_apiver__",
        .plugins_var = "plugins_extensions__",
        .envdir = "LIBYANG_EXTENSIONS_PLUGINS_DIR",
        .dir = LYPLG_EXT_DIR,
        .apiver = LYPLG_EXT_API_VERSION
    }
};

/**
 * @brief Get the expected plugin objects from the loaded dynamic object and add the defined plugins into the lists of
 * available extensions/types plugins.
 *
 * @param[in] dlhandler Loaded dynamic library handler.
 * @param[in] pathname Path of the loaded library for logging.
 * @param[in] type Type of the plugins to get from the dynamic library. Note that a single library can hold both types
 * and extensions plugins implementations, so this function should be called twice (once for each plugin type) with
 * different @p type values
 * @return LY_ERR values.
 */
static LY_ERR
plugins_load(void *dlhandler, const char *pathname, enum LYPLG type)
{
    const void *plugins;
    uint32_t *version;

    /* type plugin */
    version = dlsym(dlhandler, plugins_load_info[type].apiver_var);
    if (version) {
        /* check version ... */
        if (*version != plugins_load_info[type].apiver) {
            LOGERR(NULL, LY_EINVAL, "Processing user %s plugin \"%s\" failed, wrong API version - %d expected, %d found.",
                    plugins_load_info[type].id, pathname, plugins_load_info[type].apiver, *version);
            return LY_EINVAL;
        }

        /* ... get types plugins information ... */
        if (!(plugins = dlsym(dlhandler, plugins_load_info[type].plugins_var))) {
            char *errstr = dlerror();

            LOGERR(NULL, LY_EINVAL, "Processing user %s plugin \"%s\" failed, missing %s plugins information (%s).",
                    plugins_load_info[type].id, pathname, plugins_load_info[type].id, errstr);
            return LY_EINVAL;
        }

        /* ... and load all the types plugins */
        LY_CHECK_RET(plugins_insert(NULL, type, plugins));
    }

    return LY_SUCCESS;
}

static LY_ERR
plugins_load_module(const char *pathname)
{
    LY_ERR ret = LY_SUCCESS;
    void *dlhandler;
    uint32_t types_count = 0, extensions_count = 0;

    dlerror();    /* Clear any existing error */

    dlhandler = dlopen(pathname, RTLD_NOW);
    if (!dlhandler) {
        LOGERR(NULL, LY_ESYS, "Loading \"%s\" as a plugin failed (%s).", pathname, dlerror());
        return LY_ESYS;
    }

    if (ly_set_contains(&plugins_handlers, dlhandler, NULL)) {
        /* the plugin is already loaded */
        LOGVRB("Plugin \"%s\" already loaded.", pathname);

        /* keep the correct refcount */
        dlclose(dlhandler);
        return LY_SUCCESS;
    }

    /* remember the current plugins lists for recovery */
    types_count = ly_plugins_types.count;
    extensions_count = ly_plugins_extensions.count;

    /* type plugin */
    ret = plugins_load(dlhandler, pathname, LYPLG_TYPE);
    LY_CHECK_GOTO(ret, error);

    /* extension plugin */
    ret = plugins_load(dlhandler, pathname, LYPLG_EXTENSION);
    LY_CHECK_GOTO(ret, error);

    /* remember the dynamic plugin */
    ret = ly_set_add(&plugins_handlers, dlhandler, 1, NULL);
    LY_CHECK_GOTO(ret, error);

    return LY_SUCCESS;

error:
    dlclose(dlhandler);

    /* revert changes in the lists */
    while (ly_plugins_types.count > types_count) {
        ly_set_rm_index(&ly_plugins_types, ly_plugins_types.count - 1, NULL);
    }
    while (ly_plugins_extensions.count > extensions_count) {
        ly_set_rm_index(&ly_plugins_extensions, ly_plugins_extensions.count - 1, NULL);
    }

    return ret;
}

static LY_ERR
plugins_insert_dir(enum LYPLG type)
{
    LY_ERR ret = LY_SUCCESS;
    const char *pluginsdir;
    DIR *dir;
    ly_bool default_dir = 0;

    /* try to get the plugins directory from environment variable */
    pluginsdir = getenv(plugins_load_info[type].envdir);
    if (!pluginsdir) {
        /* remember that we are going to a default dir and do not print warning if the directory doesn't exist */
        default_dir = 1;
        pluginsdir = plugins_load_info[type].dir;
    }

    dir = opendir(pluginsdir);
    if (!dir) {
        /* no directory (or no access to it), no extension plugins */
        if (!default_dir || (errno != ENOENT)) {
            LOGWRN(NULL, "Failed to open libyang %s plugins directory \"%s\" (%s).", plugins_load_info[type].id,
                    pluginsdir, strerror(errno));
        }
    } else {
        struct dirent *file;

        while ((file = readdir(dir))) {
            size_t len;
            char pathname[PATH_MAX];

            /* required format of the filename is *LYPLG_SUFFIX */
            len = strlen(file->d_name);
            if ((len < LYPLG_SUFFIX_LEN + 1) || strcmp(&file->d_name[len - LYPLG_SUFFIX_LEN], LYPLG_SUFFIX)) {
                continue;
            }

            /* and construct the filepath */
            snprintf(pathname, PATH_MAX, "%s/%s", pluginsdir, file->d_name);

            ret = plugins_load_module(pathname);
            if (ret) {
                break;
            }
        }
        closedir(dir);
    }

    return ret;
}

#endif

LY_ERR
lyplg_init(ly_bool builtin_type_plugins_only, ly_bool static_plugins_only)
{
    LY_ERR ret;

    pthread_mutex_lock(&plugins_guard);
    /* let only the first context to initiate plugins, but let others wait for finishing the initiation */
    if (context_refcount++) {
        /* already initiated */
        pthread_mutex_unlock(&plugins_guard);
        return LY_SUCCESS;
    }

    /* internal types */
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_binary), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_bits), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_boolean), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_decimal64), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_empty), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_enumeration), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_identityref), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_instanceid), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_integer), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_leafref), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_string), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_union), error);

    /* metadata and lyds_tree, which requires them */
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_EXTENSION, plugins_metadata), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_lyds_tree), error);

    if (!builtin_type_plugins_only) {
        /* yang */
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_instanceid_keys), error);

        /* ietf-inet-types */
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_ipv4_address), error);
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_ipv4_address_no_zone), error);
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_ipv6_address), error);
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_ipv6_address_no_zone), error);
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_ipv4_prefix), error);
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_ipv6_prefix), error);

        /* ietf-yang-types */
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_date_and_time), error);
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_hex_string), error);
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_xpath10), error);

        /* libnetconf2-netconf-server */
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_time_period), error);

        /* ietf-netconf-acm */
        LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_TYPE, plugins_node_instanceid), error);
    }

    /* internal extensions */
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_EXTENSION, plugins_nacm), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_EXTENSION, plugins_yangdata), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_EXTENSION, plugins_schema_mount), error);
    LY_CHECK_GOTO(ret = plugins_insert(NULL, LYPLG_EXTENSION, plugins_structure), error);

    /* the global plugin sets contain only static plugins at this point, so assign to the counters here.
     * the counters are used to determine whether a plugin is static or not */
    ly_static_type_plugins_count = ly_plugins_types.count;
    ly_static_ext_plugins_count = ly_plugins_extensions.count;

#ifndef STATIC
    if (!static_plugins_only) {
        /* external types */
        LY_CHECK_GOTO(ret = plugins_insert_dir(LYPLG_TYPE), error);

        /* external extensions */
        LY_CHECK_GOTO(ret = plugins_insert_dir(LYPLG_EXTENSION), error);
    }
#endif

    /* initiation done, wake-up possibly waiting threads creating another contexts */
    pthread_mutex_unlock(&plugins_guard);

    return LY_SUCCESS;

error:
    /* initiation was not successful - cleanup (and let others to try) */
#ifndef STATIC
    lyplg_clean_();
#endif
    pthread_mutex_unlock(&plugins_guard);

    if (ret == LY_EINVAL) {
        /* all the plugins here are internal, invalid record actually means an internal libyang error */
        ret = LY_EINT;
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyplg_add(const char *pathname)
{
#ifdef STATIC
    (void)pathname;

    LOGERR(NULL, LY_EINVAL, "Plugins are not supported in statically built library.");
    return LY_EINVAL;
#elif defined (_WIN32)
    (void)pathname;

    LOGERR(NULL, LY_EINVAL, "Plugins are not (yet) supported on Windows.");
    return LY_EINVAL;
#else
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, pathname, LY_EINVAL);

    /* works only in case a context exists */
    pthread_mutex_lock(&plugins_guard);
    if (!context_refcount) {
        /* no context */
        pthread_mutex_unlock(&plugins_guard);
        LOGERR(NULL, LY_EDENIED, "To add a plugin, at least one context must exists.");
        return LY_EDENIED;
    }

    ret = plugins_load_module(pathname);

    pthread_mutex_unlock(&plugins_guard);

    return ret;
#endif
}

/**
 * @brief Manually load an extension plugins from memory
 *
 * Note, that a plugin can be loaded only if there is at least one context. The loaded plugins are connected with the
 * existence of a context. When all the contexts are destroyed, all the plugins are unloaded.
 *
 * @param[in] ctx The context to which the plugin should be associated with. If NULL, the plugin is considered to be shared
 * between all existing contexts.
 * @param[in] version The version of plugin records.
 * @param[in] type The type of plugins records.
 * @param[in] recs An array of plugin records provided by the plugin implementation. The array must be terminated by a zeroed
 * record.
 *
 * @return LY_SUCCESS if the plugins with compatible version were successfully loaded.
 * @return LY_EDENIED in case there is no context and the plugin cannot be loaded.
 * @return LY_EINVAL when recs is NULL or the plugin contains invalid content for this libyang version.
 */
static LY_ERR
lyplg_add_plugin(struct ly_ctx *ctx, uint32_t version, enum LYPLG type, const void *recs)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t cur_ver = (type == LYPLG_TYPE) ? LYPLG_TYPE_API_VERSION : LYPLG_EXT_API_VERSION;

    LY_CHECK_ARG_RET(NULL, recs, LY_EINVAL);

    if (version != cur_ver) {
        LOGERR(ctx, LY_EINVAL, "Adding user %s plugin failed, wrong API version - %" PRIu32 " expected, %" PRIu32 " found.",
                (type == LYPLG_TYPE) ? "type" : "extension", cur_ver, version);
        return LY_EINVAL;
    }

    /* works only in case a context exists */
    pthread_mutex_lock(&plugins_guard);
    if (!context_refcount) {
        /* no context */
        pthread_mutex_unlock(&plugins_guard);
        LOGERR(NULL, LY_EDENIED, "To add a plugin, at least one context must exists.");
        return LY_EDENIED;
    }

    ret = plugins_insert(ctx, type, recs);
    pthread_mutex_unlock(&plugins_guard);

    return ret;
}

LIBYANG_API_DEF LY_ERR
lyplg_add_extension_plugin(struct ly_ctx *ctx, uint32_t version, const struct lyplg_ext_record *recs)
{
    return lyplg_add_plugin(ctx, version, LYPLG_EXTENSION, recs);
}

LIBYANG_API_DEF LY_ERR
lyplg_add_type_plugin(struct ly_ctx *ctx, uint32_t version, const struct lyplg_type_record *recs)
{
    return lyplg_add_plugin(ctx, version, LYPLG_TYPE, recs);
}
