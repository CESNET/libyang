/**
 * @file extensions.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang support for YANG extension implementations.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_EXTENSIONS_H_
#define LY_EXTENSIONS_H_

#include "tree_schema.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    LYS_STRUCT_MODULE = 1,           /**< for struct lys_module (or the derived struct lys_submodule) */
    LYS_STRUCT_NODE = 2              /**< for struct lys_node (and all the derived structures) */
} LYS_STRUCT;

/**
 * @brief Callback to check that the extension can be instantiated inside the provided node
 *
 * @param[in] type The type of the structure provided as \p parent.
 * @param[in] parent The parent of the instantiated extension.
 * @return 0 - yes
 *         1 - no
 *         2 - ignore / skip without an error
 */
typedef int (*lyext_check_position_clb)(LYS_STRUCT type, const void *parent);

/**
 * @brief Callback to check that the extension can be instantiated inside the provided node
 *
 * @param[in] type The type of the structure provided as \p parent.
 * @param[in] parent The parent of the instantiated extension.
 * @return 0 - yes
 *         1 - no
 *         2 - ignore / skip without an error
 */
typedef int (*lyext_check_value_clb)(const char *value);

/**
 * @brief Extension instance parent enumeration
 */
typedef enum {
    LYEXT_PAR_MODULE,              /**< ::lys_module or ::lys_submodule */
    LYEXT_PAR_NODE,                /**< ::lys_node (and the derived types */
    LYEXT_PAR_TPDF,                /**< ::lys_tpdf */
    LYEXT_PAR_TYPE,                /**< ::lys_type */
    LYEXT_PAR_FEATURE,             /**< ::lys_feature */
    LYEXT_PAR_IDENT,               /**< ::lys_ident */
    LYEXT_PAR_EXT,                 /**< ::lys_ext */
    LYEXT_PAR_EXTINST,             /**< ::lys_ext_instance */
    LYEXT_PAR_REFINE,              /**< ::lys_refine */
    LYEXT_PAR_DEVIATION,           /**< ::lys_deviation */
    LYEXT_PAR_IMPORT,              /**< ::lys_import */
    LYEXT_PAR_INCLUDE              /**< ::lys_include */
} LYEXT_PAR;

/* extension types */
typedef enum {
    LYEXT_ERR = -1,                /**< error value when #LYEXT_TYPE is expected as return value of a function */
    LYEXT_FLAG = 0                 /**< simple extension with no substatements;
                                        instances are stored directly as ::lys_ext_instance and no cast is needed;
                                        plugins are expected directly as ::lys_ext_plugin and no cast is done */
} LYEXT_TYPE;

/**
 * @brief Generic extension instance structure
 *
 * The structure can be cast to another lys_ext_instance_* structure according to the extension type
 * that can be get via lys_ext_type() function. Check the #LYEXT_TYPE values to get know the specific mapping
 * between the extension type and lys_ext_instance_* structures.
 */
struct lys_ext_instance {
    struct lys_ext *def;             /**< definition of the instantiated extension,
                                          according to the type in the extension's plugin structure, the
                                          structure can be cast to the more specific structure */
    const char *arg_value;           /**< value of the instance's argument, if defined */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    uint8_t ext_size;                /**< number of elements in #ext array */
};


struct lys_ext_plugin {
    LYEXT_TYPE type;                 /**< type of the extension, according to it the structure will be casted */
    lyext_check_position_clb *check_position; /**< callbcak for testing that the extension can be instantiated
                                          under the provided parent. Mandatory callback. */
    lyext_check_value_clb *check_value; /**< callback for testing if the argument value of the extension instance
                                          is valid. Mandatory if the extension has the argument. */
};

#ifdef __cplusplus
}
#endif

#endif /* LY_EXTENSIONS_H_ */
