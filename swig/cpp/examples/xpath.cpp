/**
 * @file xpath.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Example of the libyang C++ bindings
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <Libyang.hpp>
#include <Tree_Data.hpp>
#include <Tree_Schema.hpp>

using namespace std;

int main() {

    S_Context ctx = nullptr;
    try {
        ctx = S_Context(new Context("/etc/sysrepo/yang"));
    } catch( const std::exception& e ) {
        cout << e.what() << endl;
        return -1;
    }

    auto module = ctx->load_module("turing-machine", nullptr);
    if (!module) {
        printf("module not loaded\n");
        return -1;
    }

    S_Data_Node node = nullptr;
    try {
        node = ctx->parse_data_path("/etc/sysrepo/data/turing-machine.startup", LYD_XML, LYD_OPT_CONFIG);
    } catch( const std::exception& e ) {
        cout << e.what() << endl;
        return -1;
    }

    auto node_set = node->find_path("/turing-machine:turing-machine/transition-function/delta[label='left summand']/*");\
    if (!node_set) {
        printf("could not find data for xpath\n");
        return -1;
    }

    std::vector<S_Data_Node>::iterator data_set;
    auto list = std::shared_ptr<std::vector<S_Data_Node>>(node_set->data());
    for(data_set = list->begin() ; data_set != list->end() ; ++data_set) {
        cout << "name: " << (*data_set)->schema()->name() << " type: " << (*data_set)->schema()->nodetype() << " path: " << (*data_set)->path() << endl;
    }

    return 0;
}
