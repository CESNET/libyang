/**
 * @file process_tree.cpp
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
        auto err = Error();
        cout << "err: " << err.err() << endl;
        cout << "vecode: " << err.vecode() << endl;
        cout << "errmsg: " << err.errmsg() << endl;
        cout << "errpath: " << err.errpath() << endl;
        cout << "errapptag: " << err.errapptag() << endl;
        return -1;
    }

    auto module = ctx->get_module("turing-machine");
    if (module) {
        cout << module->name() << endl;
    } else {
        module = ctx->load_module("turing-machine");
    }

    S_Data_Node node = nullptr;
    try {
        node = ctx->parse_data_path("/etc/sysrepo/data/turing-machine.startup", LYD_XML, LYD_OPT_CONFIG);
    } catch( const std::exception& e ) {
        cout << e.what() << endl;
    }

    if (!node) {
        cout << "parse_path did not return any nodes" << endl;
    } else {
        cout << "tree_dfs\n" << endl;
        auto data_list = std::shared_ptr<std::vector<S_Data_Node>>(node->tree_dfs());
        if (data_list) {
            std::vector<S_Data_Node>::iterator elem;
            for(elem = data_list->begin() ; elem != data_list->end() ; ++elem) {
                cout << "name: " << (*elem)->schema()->name() << " type: " << (*elem)->schema()->nodetype() << endl;
            }
        }

        cout << "\nChild of " << node->schema()->name() << " is: " << node->child()->schema()->name() << "\n" << endl;

        cout << "tree_for\n" << endl;

        data_list = std::shared_ptr<std::vector<S_Data_Node>>(node->child()->child()->tree_dfs());
        if (data_list) {
            std::vector<S_Data_Node>::iterator elem;
            for(elem = data_list->begin() ; elem != data_list->end() ; ++elem) {
                cout << "child of " << node->child()->schema()->name() << " is: " << (*elem)->schema()->name() << " type: " << (*elem)->schema()->nodetype() << endl;
            }
        }

        cout << "\n schema tree_dfs\n" << endl;
        auto schema_list = std::shared_ptr<std::vector<S_Schema_Node>>(node->schema()->tree_dfs());
        if (schema_list) {
            std::vector<S_Schema_Node>::iterator elem;
            for(elem = schema_list->begin() ; elem != schema_list->end() ; ++elem) {
                cout << "schema name " << (*elem)->name() << " type " << (*elem)->nodetype() << endl;
            }
        }
    }

    return 0;
}
