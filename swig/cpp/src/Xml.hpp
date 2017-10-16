/**
 * @file Xml.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Class implementation for libyang C header xml.h.
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef XML_H
#define XML_H

#include <iostream>
#include <memory>
#include <exception>
#include <vector>

#include "Internal.hpp"

extern "C" {
#include "../../../src/libyang.h"
#include "../../../src/xml.h"
}

using namespace std;

/* defined */
class Xml_Ns;
class Xml_Attr;
class Xml_Elem;

/* used */
class Data_Node;
class Context;

class Xml_Ns
{
public:
    Xml_Ns(const struct lyxml_ns *ns, S_Deleter deleter);
    ~Xml_Ns();
    LYXML_ATTR_TYPE type() {return _ns->type;};
    S_Xml_Ns next();
    //struct lyxml_elem *parent;       /**< parent node of the attribute */
    const char *prefix() {return _ns->prefix;};
    const char *value() {return _ns->value;};

private:
    struct lyxml_ns *_ns;
    S_Deleter _deleter;
};

class Xml_Attr
{
public:
    Xml_Attr(struct lyxml_attr *attr, S_Deleter deleter);
    ~Xml_Attr();
    LYXML_ATTR_TYPE type() {return _attr->type;};
    S_Xml_Attr next();
    S_Xml_Ns ns();
    const char *name() {return _attr->name;};
    const char *value() {return _attr->value;};

private:
    struct lyxml_attr *_attr;
    S_Deleter _deleter;
};

class Xml_Elem
{
public:
    Xml_Elem(S_Context context, struct lyxml_elem *elem, S_Deleter deleter);
    ~Xml_Elem();
    char flags() {return _elem->flags;};
    S_Xml_Elem parent();
    S_Xml_Attr attr();
    S_Xml_Elem child();
    S_Xml_Elem next();
    S_Xml_Elem prev();
    const char *name() {return _elem->name;};
    S_Xml_Ns ns();
    const char *content() {return _elem->content;};

    /* methods */
    const char *get_attr(const char *name, const char *ns = NULL);
    S_Xml_Ns get_ns(const char *prefix);
    S_String print_mem(int options);
    //int lyxml_print_fd(int fd, const struct lyxml_elem *elem, int options);
    //int lyxml_print_file(FILE * stream, const struct lyxml_elem *elem, int options);

    /* emulate TREE macro's */
    std::vector<S_Xml_Elem> *tree_for();
    std::vector<S_Xml_Elem> *tree_dfs();

    /* TODO
    struct lyxml_elem *lyxml_dup(struct ly_ctx *ctx, struct lyxml_elem *root);
    struct lyxml_elem *lyxml_parse_mem(struct ly_ctx *ctx, const char *data, int options);
    struct lyxml_elem *lyxml_parse_path(struct ly_ctx *ctx, const char *filename, int options);
    */

    friend Data_Node;
    friend Context;

private:
    S_Context _context;
    struct lyxml_elem *_elem;
    S_Deleter _deleter;
};

#endif
