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
typedef int (*ly_ext_check_position_clb)(LYS_STRUCT type, const void *parent);

/**
 * @brief Callback to check that the extension can be instantiated inside the provided node
 *
 * @param[in] type The type of the structure provided as \p parent.
 * @param[in] parent The parent of the instantiated extension.
 * @return 0 - yes
 *         1 - no
 *         2 - ignore / skip without an error
 */
typedef int (*ly_ext_check_value_clb)(const char *value);

/**
 * @brief Extension instance parent enumeration
 */
typedef enum {
    LYS_EXT_PAR_MODULE,              /**< ::lys_module or ::lys_submodule */
    LYS_EXT_PAR_NODE,                /**< ::lys_node (and the derived types */
    LYS_EXT_PAR_TPDF,                /**< ::lys_tpdf */
    LYS_EXT_PAR_TYPE,                /**< ::lys_type */
    LYS_EXT_PAR_FEATURE,             /**< ::lys_feature */
    LYS_EXT_PAR_IDENT,               /**< ::lys_ident */
    LYS_EXT_PAR_EXT,                 /**< ::lys_ext */
    LYS_EXT_PAR_EXTINST,             /**< ::lys_extension_instance */
    LYS_EXT_PAR_REFINE,              /**< ::lys_refine */
    LYS_EXT_PAR_DEVIATION,           /**< ::lys_deviation */
    LYS_EXT_PAR_IMPORT,              /**< ::lys_import */
    LYS_EXT_PAR_INCLUDE              /**< ::lys_include */
} LYS_EXT_PAR;

/* extension types */
typedef enum {
    LY_EXT_FLAG = 1                  /**< simple extension with no substatements */
} LY_EXT_TYPE;

struct lys_ext_instance {
    struct lys_ext *def;             /**< definition of the instantiated extension,
                                          according to the type in the extension's plugin structure, the
                                          structure can be cast to the more specific structure */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    uint8_t ext_size;                /**< number of elements in #ext array */
};

struct lys_ext_instance_flag {
    struct lys_ext *def;
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    uint8_t ext_size;                /**< number of elements in #ext array */

    /* flag specific part */
    const char *arg_value;           /**< value of the instance's argument */
};

struct lys_ext_plugin {
    LY_EXT_TYPE type;                /**< type of the extension, according to it the structure will be casted */
    ly_ext_check_position_clb *check_position; /**< callbcak for testing that the extension can be instantiated
                                          under the provided parent. Mandatory callback. */
    ly_ext_check_value_clb *check_value; /**< callback for testing if the argument value of the extension instance
                                          is valid. Mandatory if the extension has the argument. */
};

/**
 * @brief Extension plugin structure for the LY_EXT_FLAG type of extension is actually the same as the base
 * ::lys_ext_plugin structure.
 */
#define lys_ext_plugin_flag lys_ext_plugin

#ifdef __cplusplus
}
#endif

#endif /* LY_EXTENSIONS_H_ */
