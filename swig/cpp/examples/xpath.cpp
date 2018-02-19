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
        auto module = ctx->load_module("turing-machine", nullptr);

        auto node = ctx->parse_data_path("/etc/sysrepo/data/turing-machine.startup", LYD_XML, LYD_OPT_CONFIG);

        auto node_set = node->find_path("/turing-machine:turing-machine/transition--function/delta[label='left summand']/*");\
        if (!node_set) {
            cout << "could not find data for xpath" << endl;
            return -1;
        }

        auto list = std::shared_ptr<std::vector<S_Data_Node>>(node_set->data());
        for(auto data_set = list->begin() ; data_set != list->end() ; ++data_set) {
            cout << "name: " << (*data_set)->schema()->name() << " type: " << (*data_set)->schema()->nodetype() << " path: " << (*data_set)->path() << endl;
        }
    } catch( const std::exception& e ) {
        cout << "test" << endl;
        cout << e.what() << endl;
        auto errors = std::shared_ptr<std::vector<S_Error>>(get_ly_errors(ctx));
        for(auto error = errors->begin() ; error != errors->end() ; ++error) {
            cout << "err: " << (*error)->err() << endl;
            cout << "vecode: " << (*error)->vecode() << endl;
            cout << "errmsg: " << (*error)->errmsg() << endl;
            cout << "errpath: " << (*error)->errpath() << endl;
            cout << "errapptag: " << (*error)->errapptag() << endl;
        }
        return -1;
    }

    return 0;
}
