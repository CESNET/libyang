/**
 * @file extensions.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YANG extensions routines implementation
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "extensions.h"
#include "extensions_config.h"
#include "libyang.h"

/* internal structures storing the extension plugins */
struct lyext_plugin_list *ext_plugins = NULL;
unsigned int ext_plugins_count = 0; /* size of the ext_plugins array */
unsigned int ext_plugins_ref = 0;   /* number of contexts that may reference the ext_plugins */
struct ly_set dlhandlers = {0, 0, {NULL}};
pthread_mutex_t ext_lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief reference counter for the ext_plugins, it actually counts number of contexts
 */
unsigned int ext_plugins_references = 0;

API int
lyext_clean_plugins(void)
{
    unsigned int u;

    if (ext_plugins_ref) {
        /* there is a context that may refer to the plugins, so we cannot remove them */
        return EXIT_FAILURE;
    }

    if (!ext_plugins_count) {
        /* no plugin loaded - nothing to do */
        return EXIT_SUCCESS;
    }

    /* lock the extension plugins list */
    pthread_mutex_lock(&ext_lock);

    /* clean the list */
    free(ext_plugins);
    ext_plugins = NULL;
    ext_plugins_count = 0;

    /* close the dl handlers */
    for (u = 0; u < dlhandlers.number; u++) {
        dlclose(dlhandlers.set.g[u]);
    }
    free(dlhandlers.set.g);
    dlhandlers.set.g = NULL;
    dlhandlers.size = 0;
    dlhandlers.number = 0;

    /* unlock the global structures */
    pthread_mutex_unlock(&ext_lock);

    return EXIT_SUCCESS;
}

API void
lyext_load_plugins(void)
{
    DIR* dir;
    struct dirent *file;
    size_t len;
    char *str;
    char name[NAME_MAX];
    void *dlhandler;
    struct lyext_plugin_list *plugin, *p;
    unsigned int u, v;
    const char *pluginsdir;

    /* try to get the plugins directory from environment variable */
    pluginsdir = getenv("LIBYANG_EXTENSIONS_PLUGINS_DIR");
    if (!pluginsdir) {
        pluginsdir = LYEXT_PLUGINS_DIR;
    }

    dir = opendir(pluginsdir);
    if (!dir) {
        /* no directory (or no access to it), no plugins */
        LOGWRN("libyang extensions plugins directory \"%s\" does not exist.", pluginsdir);
        return;
    }

    /* lock the extension plugins list */
    pthread_mutex_lock(&ext_lock);

    while ((file = readdir(dir))) {
        if (file->d_type != DT_REG && file->d_type == DT_LNK) {
            /* other files than regular and symbolic links are ignored */
            continue;
        }

        /* required format of the filename is *.so */
        len = strlen(file->d_name);
        if (len < 4 || strcmp(&file->d_name[len - 3], ".so")) {
            continue;
        }

        /* store the name without the suffix */
        memcpy(name, file->d_name, len - 3);
        name[len - 3] = '\0';

        /* and construct the filepath */
        asprintf(&str, "%s/%s", pluginsdir, file->d_name);

        /* load the plugin - first, try if it is already loaded... */
        dlhandler = dlopen(str, RTLD_NOW | RTLD_NOLOAD);
        dlerror();    /* Clear any existing error */
        if (dlhandler) {
            /* the plugin is already loaded */
            LOGVRB("Extension plugin \"%s\" already loaded.", str);
            free(str);
            continue;
        }

        /* ... and if not, load it */
        dlhandler = dlopen(str, RTLD_NOW);
        if (!dlhandler) {
            LOGERR(LY_ESYS, "Loading \"%s\" as an extension plugin failed (%s).", str, dlerror());
            free(str);
            continue;
        }
        LOGVRB("Extension plugin \"%s\" successfully loaded.", str);
        free(str);
        dlerror();    /* Clear any existing error */

        /* get the plugin data */
        plugin = dlsym(dlhandler, name);
        str = dlerror();
        if (str) {
            LOGERR(LY_ESYS, "Processing \"%s\" extension plugin failed, missing plugin list object (%s).", name, str);
            dlclose(dlhandler);
            continue;
        }

        /* check extension implementations for collisions */
        for(u = 0; plugin[u].name; u++) {
            for (v = 0; v < ext_plugins_count; v++) {
                if (!strcmp(plugin[u].name, ext_plugins[v].name) &&
                        !strcmp(plugin[u].module, ext_plugins[v].module) &&
                        (!plugin[u].revision || !ext_plugins[v].revision || !strcmp(plugin[u].revision, ext_plugins[v].revision))) {
                    LOGERR(LY_ESYS, "Processing \"%s\" extension plugin failed,"
                           "implementation collision for extension %s from module %s%s%s.",
                           name, plugin[u].name, plugin[u].module, plugin[u].revision ? "@" : "",
                           plugin[u].revision ? plugin[u].revision : "");
                    dlclose(dlhandler);
                    continue;
                }
            }
        }

        /* add the new plugins, we have number of new plugins as u */
        p = realloc(ext_plugins, (ext_plugins_count + u) * sizeof *ext_plugins);
        if (!p) {
            LOGMEM;
            dlclose(dlhandler);
            closedir(dir);

            /* unlock the global structures */
            pthread_mutex_unlock(&ext_lock);

            return;
        }
        ext_plugins = p;
        for( ; u; u--) {
            memcpy(&ext_plugins[ext_plugins_count], &plugin[u - 1], sizeof *plugin);
            ext_plugins_count++;
        }

        /* keep the handler */
        ly_set_add(&dlhandlers, dlhandler, LY_SET_OPT_USEASLIST);
    }

    closedir(dir);

    /* unlock the global structures */
    pthread_mutex_unlock(&ext_lock);
}

API LYEXT_TYPE
lys_ext_instance_type(struct lys_ext_instance *ext)
{
    if (!ext) {
        return LYEXT_ERR;
    }

    if (!ext->def->plugin) {
        /* default extension type - LY_EXT_FLAG */
        return 0;
    } else {
        return ext->def->plugin->type;
    }
}

struct lyext_plugin *
ext_get_plugin(const char *name, const char *module, const char *revision)
{
    unsigned int u;

    assert(name);
    assert(module);

    for (u = 0; u < ext_plugins_count; u++) {
        if (!strcmp(name, ext_plugins[u].name) &&
                !strcmp(module, ext_plugins[u].module) &&
                (!ext_plugins[u].revision || !strcmp(revision, ext_plugins[u].revision))) {
            /* we have the match */
            return ext_plugins[u].plugin;
        }
    }

    /* plugin not found */
    return NULL;
}

API int
lys_ext_instance_presence(struct lys_ext *def, struct lys_ext_instance **ext, uint8_t ext_size)
{
    uint8_t index;

    if (!def || (ext_size && !ext)) {
        ly_errno = LY_EINVAL;
        return -1;
    }

    /* search for the extension instance */
    for (index = 0; index < ext_size; index++) {
        if (ext[index]->def == def) {
            return index;
        }
    }

    /* not found */
    return -1;
}

API void *
lys_ext_complex_get_substmt(LY_STMT stmt, struct lys_ext_instance_complex *ext, struct lyext_substmt **info)
{
    int i;

    if (!ext || !ext->def || !ext->def->plugin || ext->def->plugin->type != LYEXT_COMPLEX) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (!ext->substmt) {
        /* no substatement defined in the plugin */
        if (info) {
            *info = NULL;
        }
        return NULL;
    }

    /* search the substatements defined by the plugin */
    for (i = 0; ext->substmt[i].stmt; i++) {
        if (ext->substmt[i].stmt == stmt) {
            if (info) {
                *info = &ext->substmt[i];
            }
            break;
        }
    }

    if (ext->substmt[i].stmt) {
        return &ext->content[ext->substmt[i].offset];
    } else {
        return NULL;
    }
}
