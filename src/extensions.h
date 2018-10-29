/**
 * @file extesnions.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang support for YANG extensions implementation.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_EXTENSIONS_H_
#define LY_EXTENSIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup extensions YANG Extensions
 *
 * @{
 */

/**
 * @brief Extension instance structure parent enumeration
 */
typedef enum {
    LYEXT_PAR_MODULE, /**< ::lys_module or ::lys_submodule */
    LYEXT_PAR_NODE, /**< ::lys_node (and the derived structures) */
    LYEXT_PAR_TPDF, /**< ::lys_tpdf */
    LYEXT_PAR_TYPE, /**< ::lys_type */
    LYEXT_PAR_TYPE_BIT, /**< ::lys_type_bit */
    LYEXT_PAR_TYPE_ENUM, /**< ::lys_type_enum */
    LYEXT_PAR_FEATURE, /**< ::lys_feature */
    LYEXT_PAR_RESTR, /**< ::lys_restr - YANG's must, range, length and pattern statements */
    LYEXT_PAR_WHEN, /**< ::lys_when */
    LYEXT_PAR_IDENT, /**< ::lys_ident */
    LYEXT_PAR_EXT, /**< ::lys_ext */
    LYEXT_PAR_EXTINST, /**< ::lys_ext_instance */
    LYEXT_PAR_REFINE, /**< ::lys_refine */
    LYEXT_PAR_DEVIATION, /**< ::lys_deviation */
    LYEXT_PAR_DEVIATE, /**< ::lys_deviate */
    LYEXT_PAR_IMPORT, /**< ::lys_import */
    LYEXT_PAR_INCLUDE,           /**< ::lysp_include */
    LYEXT_PAR_REVISION,          /**< ::lysc_revision */
} LYEXT_PARENT;

/**
 * @brief Enum of substatements in which extension instances can appear.
 */
typedef enum {
    LYEXT_SUBSTMT_SELF = 0,      /**< extension of the structure itself, not substatement's */
    LYEXT_SUBSTMT_ARGUMENT,      /**< extension of the argument statement, can appear in lys_ext */
    LYEXT_SUBSTMT_BASE,          /**< extension of the base statement, can appear (repeatedly) in lys_type and lys_ident */
    LYEXT_SUBSTMT_BELONGSTO,     /**< extension of the belongs-to statement, can appear in lys_submodule */
    LYEXT_SUBSTMT_CONTACT,       /**< extension of the contact statement, can appear in lys_module */
    LYEXT_SUBSTMT_DEFAULT,       /**< extension of the default statement, can appear in lys_node_leaf, lys_node_leaflist,
                                      lys_node_choice and lys_deviate */
    LYEXT_SUBSTMT_DESCRIPTION,   /**< extension of the description statement, can appear in lys_module, lys_submodule,
                                      lys_node, lys_import, lys_include, lys_ext, lys_feature, lys_tpdf, lys_restr,
                                      lys_ident, lys_deviation, lys_type_enum, lys_type_bit, lys_when and lys_revision */
    LYEXT_SUBSTMT_ERRTAG,        /**< extension of the error-app-tag statement, can appear in lys_restr */
    LYEXT_SUBSTMT_ERRMSG,        /**< extension of the error-message statement, can appear in lys_restr */
    LYEXT_SUBSTMT_KEY,           /**< extension of the key statement, can appear in lys_node_list */
    LYEXT_SUBSTMT_NAMESPACE,     /**< extension of the namespace statement, can appear in lys_module */
    LYEXT_SUBSTMT_ORGANIZATION,  /**< extension of the organization statement, can appear in lys_module and lys_submodule */
    LYEXT_SUBSTMT_PATH,          /**< extension of the path statement, can appear in lys_type */
    LYEXT_SUBSTMT_PREFIX,        /**< extension of the prefix statement, can appear in lys_module, lys_submodule (for
                                      belongs-to's prefix) and lys_import */
    LYEXT_SUBSTMT_PRESENCE,      /**< extension of the presence statement, can appear in lys_node_container */
    LYEXT_SUBSTMT_REFERENCE,     /**< extension of the reference statement, can appear in lys_module, lys_submodule,
                                      lys_node, lys_import, lys_include, lys_revision, lys_tpdf, lys_restr, lys_ident,
                                      lys_ext, lys_feature, lys_deviation, lys_type_enum, lys_type_bit and lys_when */
    LYEXT_SUBSTMT_REVISIONDATE,  /**< extension of the revision-date statement, can appear in lys_import and lys_include */
    LYEXT_SUBSTMT_UNITS,         /**< extension of the units statement, can appear in lys_tpdf, lys_node_leaf,
                                      lys_node_leaflist and lys_deviate */
    LYEXT_SUBSTMT_VALUE,         /**< extension of the value statement, can appear in lys_type_enum */
    LYEXT_SUBSTMT_VERSION,       /**< extension of the yang-version statement, can appear in lys_module and lys_submodule */
    LYEXT_SUBSTMT_MODIFIER,      /**< extension of the modifier statement, can appear in lys_restr */
    LYEXT_SUBSTMT_REQINSTANCE,   /**< extension of the require-instance statement, can appear in lys_type */
    LYEXT_SUBSTMT_YINELEM,       /**< extension of the yin-element statement, can appear in lys_ext */
    LYEXT_SUBSTMT_CONFIG,        /**< extension of the config statement, can appear in lys_node and lys_deviate */
    LYEXT_SUBSTMT_MANDATORY,     /**< extension of the mandatory statement, can appear in lys_node_leaf, lys_node_choice,
                                      lys_node_anydata and lys_deviate */
    LYEXT_SUBSTMT_ORDEREDBY,     /**< extension of the ordered-by statement, can appear in lys_node_list and lys_node_leaflist */
    LYEXT_SUBSTMT_STATUS,        /**< extension of the status statement, can appear in lys_tpdf, lys_node, lys_ident,
                                      lys_ext, lys_feature, lys_type_enum and lys_type_bit */
    LYEXT_SUBSTMT_FRACDIGITS,    /**< extension of the fraction-digits statement, can appear in lys_type */
    LYEXT_SUBSTMT_MAX,           /**< extension of the max-elements statement, can appear in lys_node_list,
                                      lys_node_leaflist and lys_deviate */
    LYEXT_SUBSTMT_MIN,           /**< extension of the min-elements statement, can appear in lys_node_list,
                                      lys_node_leaflist and lys_deviate */
    LYEXT_SUBSTMT_POSITION,      /**< extension of the position statement, can appear in lys_type_bit */
    LYEXT_SUBSTMT_UNIQUE,        /**< extension of the unique statement, can appear in lys_node_list and lys_deviate */
    LYEXT_SUBSTMT_IFFEATURE,     /**< extension of the if-feature statement */
} LYEXT_SUBSTMT;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_SCHEMA_H_ */
