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

#include "plugins.h"
#include "plugins_internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <string.h>

#include "common.h"
#include "plugins_exts.h"
#include "plugins_types.h"

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
 * internal extension plugins records
 */
extern struct lyplg_ext_record plugins_metadata[];
extern struct lyplg_ext_record plugins_nacm[];
extern struct lyplg_ext_record plugins_yangdata[];

static pthread_mutex_t plugins_guard = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Counter for currently present contexts able to refer to the loaded plugins.
 *
 * Plugins are shared among all the created contexts. They are loaded with the creation of the very first context and
 * unloaded with the destroy of the last context. Therefore, to reload the list of plugins, all the contexts must be
 * destroyed and with the creation of a first new context after that, the plugins will be reloaded.
 */
static uint32_t context_refcount = 0;

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

static struct ly_set plugins_types = {0};
static struct ly_set plugins_extensions = {0};

/**
 * @brief Iterate over list of loaded plugins of the given @p type.
 *
 * @param[in] type Type of the plugins to iterate.
 * @param[in,out] index The iterator - set to 0 for the first call.
 * @return The plugin records, NULL if no more record is available.
 */
static struct lyplg_record *
plugins_iter(enum LYPLG type, uint32_t *index)
{
    struct ly_set *plugins;

    assert(index);

    if (type == LYPLG_EXTENSION) {
        plugins = &plugins_extensions;
    } else {
        plugins = &plugins_types;
    }

    if (*index == plugins->count) {
        return NULL;
    }

    *index += 1;
    return plugins->objs[*index - 1];
}

void *
lyplg_find(enum LYPLG type, const char *module, const char *revision, const char *name)
{
    uint32_t i = 0;
    struct lyplg_record *item;

    assert(module);
    assert(name);

    while ((item = plugins_iter(type, &i)) != NULL) {
        if (!strcmp(item->module, module) && !strcmp(item->name, name)) {
            if (item->revision && revision && strcmp(item->revision, revision)) {
                continue;
            } else if (!revision && item->revision) {
                continue;
            }

            return &item->plugin;
        }
    }

    return NULL;
}

/**
 * @brief Insert the provided extension plugin records into the internal set of extension plugins for use by libyang.
 *
 * @param[in] recs An array of plugin records provided by the plugin implementation. The array must be terminated by a zeroed
 * record.
 * @return LY_SUCCESS in case of success
 * @return LY_EINVAL for invalid information in @p recs.
 * @return LY_EMEM in case of memory allocation failure.
 */
static LY_ERR
plugins_insert(enum LYPLG type, const void *recs)
{
    if (!recs) {
        return LY_SUCCESS;
    }

    if (type == LYPLG_EXTENSION) {
        const struct lyplg_ext_record *rec = (const struct lyplg_ext_record *)recs;

        for (uint32_t i = 0; rec[i].name; i++) {
            LY_CHECK_RET(ly_set_add(&plugins_extensions, (void *)&rec[i], 0, NULL));
        }
    } else { /* LY_PLUGIN_TYPE */
        const struct lyplg_type_record *rec = (const struct lyplg_type_record *)recs;

        for (uint32_t i = 0; rec[i].name; i++) {
            LY_CHECK_RET(ly_set_add(&plugins_types, (void *)&rec[i], 0, NULL));
        }
    }

    return LY_SUCCESS;
}

static void
lyplg_clean_(void)
{
    if (--context_refcount) {
        /* there is still some other context, do not remove the plugins */
        return;
    }

    ly_set_erase(&plugins_types, NULL);
    ly_set_erase(&plugins_extensions, NULL);
}

void
lyplg_clean(void)
{
    pthread_mutex_lock(&plugins_guard);
    lyplg_clean_();
    pthread_mutex_unlock(&plugins_guard);
}

LY_ERR
lyplg_init(void)
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
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_binary), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_bits), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_boolean), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_decimal64), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_empty), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_enumeration), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_identityref), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_instanceid), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_integer), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_leafref), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_string), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_union), error);

    /* internal extensions */
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_EXTENSION, plugins_metadata), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_EXTENSION, plugins_nacm), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_EXTENSION, plugins_yangdata), error);

    /* initiation done, wake-up possibly waiting threads creating another contexts */
    pthread_mutex_unlock(&plugins_guard);

    return LY_SUCCESS;

error:
    /* initiation was not successful - cleanup (and let others to try) */
    lyplg_clean_();
    pthread_mutex_unlock(&plugins_guard);

    if (ret == LY_EINVAL) {
        /* all the plugins here are internal, invalid record actually means an internal libyang error */
        ret = LY_EINT;
    }
    return ret;
}
