/**
 * @file Libyang.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Class implementation for libyang C header libyang.h.
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LIBYANG_H
#define LIBYANG_H

#include <iostream>
#include <memory>
#include <exception>
#include <vector>

#include "Internal.hpp"

extern "C" {
#include "libyang.h"
}

/* defined */
class Context;
class Error;
class Set;

/* used */
class Module;
class Submodule;
class Data_Node;
class Schema_Node;
class Schema_Node;
class Xml_Elem;
class Deleter;

class Error
{
/* add custom deleter for Context class */
public:
    Error() {
        libyang_err = ly_errno;
        libyang_vecode = ly_vecode;
        libyang_errmsg = ly_errmsg();
        libyang_errpath = ly_errpath();
        libyang_errapptag = ly_errapptag();
    };
    ~Error() {};
    LY_ERR err() throw() {return libyang_err;};
    LY_VECODE vecode() throw() {return libyang_vecode;};
    const char *errmsg() const throw() {return libyang_errmsg;};
    const char *errpath() const throw() {return libyang_errpath;};
    const char *errapptag() const throw() {return libyang_errapptag;};
private:
    LY_ERR libyang_err;
    LY_VECODE libyang_vecode;
    const char *libyang_errmsg;
    const char *libyang_errpath;
    const char *libyang_errapptag;
};

class Context
{
public:
    Context(struct ly_ctx *ctx, S_Deleter deleter);
    explicit Context(const char *search_dir = nullptr, int options = 0);
    Context(const char *search_dir, const char *path, LYD_FORMAT format, int options = 0);
    Context(const char *search_dir, LYD_FORMAT format, const char *data, int options = 0);
    ~Context();
    int set_searchdir(const char *search_dir) {return ly_ctx_set_searchdir(ctx, search_dir);};
    void unset_searchdirs(int idx) {return ly_ctx_unset_searchdirs(ctx, idx);};
    std::vector<std::string> *get_searchdirs();
    void set_allimplemented() {return ly_ctx_set_allimplemented(ctx);};
    void unset_allimplemented() {return ly_ctx_unset_allimplemented(ctx);};
    S_Data_Node info();
    std::vector<S_Module> *get_module_iter();
    std::vector<S_Module> *get_disabled_module_iter();
    S_Module get_module(const char *name, const char *revision = nullptr, int implemented = 0);
    S_Module get_module_older(S_Module module);
    S_Module load_module(const char *name, const char *revision = nullptr);
    S_Module get_module_by_ns(const char *ns, const char *revision = nullptr, int implemented = 0);
    S_Submodule get_submodule(const char *module, const char *revision = nullptr, const char *submodule = nullptr, const char *sub_revision = nullptr);
    S_Submodule get_submodule2(S_Module main_module, const char *submodule = nullptr);
    S_Schema_Node get_node(S_Schema_Node start, const char *data_path, int output = 0);
    void clean();

    /* functions */
    S_Data_Node parse_mem(const char *data, LYD_FORMAT format, int options = 0);
    S_Data_Node parse_fd(int fd, LYD_FORMAT format, int options = 0);
    S_Data_Node parse_data_path(const char *path, LYD_FORMAT format, int options = 0);
    S_Data_Node parse_xml(S_Xml_Elem elem, int options = 0);
    S_Module parse_path(const char *path, LYS_INFORMAT format);

    friend Data_Node;
    friend Deleter;

private:
    struct ly_ctx *ctx;
    S_Deleter deleter;
};

class Set
{
public:
    Set(struct ly_set *set, S_Deleter);
    Set();
    ~Set();
    unsigned int size() {return set->size;};
    unsigned int number() {return set->number;};
    std::vector<S_Data_Node> *data();
    std::vector<S_Schema_Node> *schema();

    /* functions */
    S_Set dup();
    int add(S_Data_Node node, int options = 0);
    int add(S_Schema_Node node, int options = 0);
    int contains(S_Data_Node node);
    int contains(S_Schema_Node node);
    int clean();
    int rm(S_Data_Node node);
    int rm(S_Schema_Node node);
    int rm_index(unsigned int index);

private:
    struct ly_set *set;
    S_Deleter deleter;
};

#endif
