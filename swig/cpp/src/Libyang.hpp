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

class Context
{
public:
    Context(struct ly_ctx *ctx, S_Deleter deleter);
    explicit Context(const char *search_dir = nullptr, int options = 0);
    Context(const char *search_dir, const char *path, LYD_FORMAT format, int options = 0);
    Context(const char *search_dir, LYD_FORMAT format, const char *data, int options = 0);
    ~Context();
    int set_searchdir(const char *search_dir);
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
    std::vector<S_Schema_Node> *data_instantiables(int options);
    S_Set find_path(const char *schema_path);
    void clean();

    /* functions */
    S_Data_Node parse_data_mem(const char *data, LYD_FORMAT format, int options = 0);
    S_Data_Node parse_data_fd(int fd, LYD_FORMAT format, int options = 0);
    S_Data_Node parse_data_path(const char *path, LYD_FORMAT format, int options = 0);
    S_Data_Node parse_data_xml(S_Xml_Elem elem, int options = 0);
    S_Module parse_module_mem(const char *data, LYS_INFORMAT format);
    S_Module parse_module_fd(int fd, LYS_INFORMAT format);
    S_Module parse_module_path(const char *path, LYS_INFORMAT format);

    friend std::vector<S_Error> *get_ly_errors(S_Context context);
    friend Data_Node;
    friend Deleter;
    friend Error;

private:
    struct ly_ctx *ctx;
    S_Deleter deleter;
};

S_Context create_new_Context(struct ly_ctx *ctx);

class Error
{
public:
    Error(struct ly_err_item *eitem);
    ~Error() {};
    LY_ERR err() throw() {return eitem->no;};
    LY_VECODE vecode() throw() {return eitem->vecode;};
    const char *errmsg() const throw() {return eitem->msg ? eitem->msg : "";};
    const char *errpath() const throw() {return eitem->path ? eitem->path : "";};
    const char *errapptag() const throw() {return eitem->apptag ? eitem->path : "";};
private:
	struct ly_err_item *eitem;
};

std::vector<S_Error> *get_ly_errors(S_Context context);
int set_log_options(int options);
LY_LOG_LEVEL set_log_verbosity(LY_LOG_LEVEL level);

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
