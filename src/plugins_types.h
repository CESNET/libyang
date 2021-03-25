/**
 * @file plugins_types.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief API for (user) types plugins
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_TYPES_H_
#define LY_PLUGINS_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "tree.h"

#include "tree_edit.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_ctx;
struct ly_path;
struct lyd_node;
struct lyd_value;
struct lys_module;
struct lys_glob_unres;
struct lysc_ident;
struct lysc_node;
struct lysc_pattern;
struct lysc_prefix;
struct lysc_range;
struct lysc_type;
struct lysc_type_leafref;
struct lysp_module;

/**
 * @defgroup types Plugins - Types
 * @{
 *
 * Structures and functions to for libyang plugins implementing specific YANG types defined in YANG schemas
 */

/**
 * @page howtoPluginsTypes
 *
 * YANG allows schemas to define new data types via *typedef* statement or even in leaf's/leaf-list's *type* statements.
 * Such types are derived (directly or indirectly) from a set of [YANG built-in types](https://tools.ietf.org/html/rfc7950#section-4.2.4).
 * libyang implements all handling of the data values of the YANG types via plugins. Internally, plugins for the built-in types
 * and several others are implemented. Type plugin is supposed to
 * - validate (and canonize) data value according to the type's restrictions,
 * - store it as lyd_value,
 * - print it,
 * - compare two values (lyd_value) of the same type,
 * - duplicate data in lyd_value and
 * - free the connected data from lyd_value.
 *
 * All these functions are provided to libyang via a set of callback functions specified as ::lyplg_type.
 * All the callbacks are supposed to do not log directly via libyang logger. Instead, they return LY_ERR value and
 * ::ly_err_item error structure(s) describing the detected error(s) (helper functions ::ly_err_new() and ::ly_err_free()
 * are available).
 *
 * The main functionality is provided via ::ly_type_store_clb callback responsible for canonizing and storing
 * provided string representation of the value in specified format (XML and JSON supported). Valid value is stored in
 * ::lyd_value structure - its union allows to store data as one of the predefined type or in a custom form behind
 * the ptr member (void *) of ::lyd_value structure. The callback is also responsible for storing canonized string
 * representation of the value as ::lyd_value.canonical. If the type does not define canonical representation, the original
 * representation is stored. In case there are any differences between the representation in specific input types, the plugin
 * is supposed to store the value in JSON representation - typically, the difference is in prefix representation and JSON
 * format use directly the module names as prefixes. Optionally, in case the type requires some validation referencing other
 * entities in the data tree, the ::ly_type_validate_clb can be implemented. The stored value can be printed into the
 * required format via ::ly_type_print_clb implementation.
 */

/*
 * @brief Create and fill error structure.
 *
 * Helper function for various plugin functions to generate error information structure.
 *
 * @param[in, out] err Pointer to store a new error structure filled according to the input parameters. If the storage
 * already contains error information, the new record is appended into the errors list.
 * @param[in] ecode Code of the error to fill. In case LY_SUCCESS value, nothing is done and LY_SUCCESS is returned.
 * @param[in] vecode Validity error code in case of LY_EVALID error code.
 * @param[in] path Path to the node causing the error.
 * @param[in] apptag Error-app-tag value.
 * @param[in] err_msg error formating message.
 * @return The given @p ecode value if the @p err is successfully created. The structure can be freed using ::ly_err_free()
 * or passed back from callback into libyang.
 * @return LY_EMEM If there is not enough memory for allocating error record, the @p err is not touched in that case.
 * @return LY_SUCCESS if @p ecode is LY_SUCCESS, the @p err is not touched in this case.
 */
LY_ERR ly_err_new(struct ly_err_item **err, LY_ERR ecode, LY_VECODE vecode, char *path, char *apptag, const char *err_msg, ...);

/**
 * @brief Destructor for the error records created with ::ly_err_new().
 *
 * Compatible with the free(), so usable as a generic callback.
 *
 * @param[in] ptr Error record (::ly_err_item, the void pointer is here only for compatibility with a generic free()
 * function) to free. With the record, also all the records (if any) connected after this one are freed.
 */
void ly_err_free(void *ptr);

/**
 * @brief Check that the type is suitable for the parser's hints (if any) in the specified format
 *
 * Use only in implementations of ::ly_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] hints Bitmap of [value hints](@ref lydvalhints) of all the allowed value types provided by parsers
 *            to ::ly_type_store_clb.
 * @param[in] value Lexical representation of the value to be stored.
 * @param[in] value_len Length (number of bytes) of the given \p value.
 * @param[in] type Expected base type of the @p value by the caller.
 * @param[out] base Pointer to store the numeric base for parsing numeric values using strtol()/strtoll() function.
 * Returned (and required) only for numeric @p type values.
 * @param[out] err Pointer to store error information in case of failure.
 * @return LY_ERR value
 */
LY_ERR ly_type_check_hints(uint32_t hints, const char *value, size_t value_len, LY_DATA_TYPE type, int *base,
        struct ly_err_item **err);

/**
 * @brief Get the corresponding module for the identity value.
 *
 * Use only in implementations of ::ly_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] ctx libyang context.
 * @param[in] ctx_node Schema node where the value is instantiated to determine the module in case of unprefixed value
 * in specific @p format.
 * @param[in] prefix Prefix to resolve - identified beginning of a prefix in ::ly_type_store_clb's value parameter.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] format Format of the prefix (::ly_type_store_clb's format parameter).
 * @param[in] prefix_data Format-specific data (::ly_type_store_clb's prefix_data parameter).
 * @return Resolved prefix module,
 * @return NULL otherwise.
 */
const struct lys_module *ly_type_identity_module(const struct ly_ctx *ctx, const struct lysc_node *ctx_node,
        const char *prefix, size_t prefix_len, LY_PREFIX_FORMAT format, const void *prefix_data);

/**
 * @brief Implement a module (just like ::lys_set_implemented()), but keep maintaining unresolved items.
 *
 * Use only in implementations of ::ly_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] mod Module to implement.
 * @param[in] features Array of features to enable.
 * @param[in,out] unres Global unres to add to.
 * @return LY_ERR value.
 */
LY_ERR ly_type_make_implemented(struct lys_module *mod, const char **features, struct lys_glob_unres *unres);

/**
 * @brief Get format-specific prefix for a module.
 *
 * Use only in implementations of ::ly_type_print_clb which provide all the necessary parameters for this function.
 *
 * @param[in] mod Module whose prefix to get - the module somehow connected with the value to print.
 * @param[in] format Format of the prefix (::ly_type_print_clb's format parameter).
 * @param[in] prefix_data Format-specific data (::ly_type_print_clb's prefix_data parameter).
 * @return Module's prefix to print.
 * @return NULL on error.
 */
const char *ly_type_get_prefix(const struct lys_module *mod, LY_PREFIX_FORMAT format, void *prefix_data);

/**
 * @brief Store used prefixes in a string into an internal libyang structure used in ::lyd_value.
 *
 * Use only in implementations of ::ly_type_store_clb which provide all the necessary parameters for this function.
 *
 * If @p prefix_data_p are non-NULL, they are treated as valid according to the @p format_p and new possible
 * prefixes are simply added. This way it is possible to store prefix data for several strings together.
 *
 * @param[in] ctx libyang context.
 * @param[in] value Value string to be parsed.
 * @param[in] value_len Length of the @p value string.
 * @param[in] format Format of the prefixes in the value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in,out] format_p Resulting format of the prefixes.
 * @param[in,out] prefix_data_p Resulting prefix data for the value in format @p format_p.
 * @return LY_ERR value.
 */
LY_ERR ly_type_prefix_data_new(const struct ly_ctx *ctx, const char *value, size_t value_len, LY_PREFIX_FORMAT format,
        const void *prefix_data, LY_PREFIX_FORMAT *format_p, void **prefix_data_p);
/*
 * @brief Duplicate prefix data.
 *
 * Use only in implementations of ::ly_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] ctx libyang context.
 * @param[in] format Format of the prefixes in the value.
 * @param[in] orig Prefix data to duplicate.
 * @param[out] dup Duplicated prefix data.
 * @return LY_ERR value.
 */
LY_ERR ly_type_prefix_data_dup(const struct ly_ctx *ctx, LY_PREFIX_FORMAT format, const void *orig, void **dup);

/**
 * @brief Free internal prefix data.
 *
 * Use only in implementations of ::ly_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] format Format of the prefixes.
 * @param[in] prefix_data Format-specific data to free.
 */
void ly_type_prefix_data_free(LY_PREFIX_FORMAT format, void *prefix_data);

/**
 * @brief Helper function to create internal schema path representation for instance-identifier value representation.
 *
 * Use only in implementations of ::ly_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] ctx libyang Context
 * @param[in] value Lexical representation of the value to be stored.
 * @param[in] value_len Length (number of bytes) of the given \p value.
 * @param[in] options [Type plugin store options](@ref plugintypestoreopts).
 * @param[in] format Input format of the value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_type_store_resolve_prefix).
 * @param[in] ctx_node The @p value schema context node.
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @param[out] path Pointer to store the created structure representing the schema path from the @p value.
 * @param[out] err Pointer to store the error information provided in case of failure.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value on error.
 */
LY_ERR ly_type_lypath_new(const struct ly_ctx *ctx, const char *value, size_t value_len, uint32_t options,
        LY_PREFIX_FORMAT format, void *prefix_data, const struct lysc_node *ctx_node,
        struct lys_glob_unres *unres, struct ly_path **path, struct ly_err_item **err);

/**
 * @brief Free ly_path structure used by instanceid value representation.
 *
 * The ly_path representation can be created by ::ly_type_lypath_new().
 *
 * @param[in] ctx libyang context.
 * @param[in] path The structure ([sized array](@ref sizedarrays)) to free.
 */
void ly_type_lypath_free(const struct ly_ctx *ctx, struct ly_path *path);

/**
 * @defgroup plugintypestoreopts Type store callback options.
 *
 * Options applicable to ::ly_type_store_clb().
 *
 * @{
 */
#define LY_TYPE_STORE_DYNAMIC   0x01 /**< String value was dynamically allocated and is supposed to be freed or
                                          directly inserted into the context's dictionary (e.g. in case of canonization).
                                          In any case, the caller of the callback does not free the provided string
                                          value after calling the type's store callback with this option */
#define LY_TYPE_STORE_IMPLEMENT 0x02 /**< If a foreign module is needed to be implemented to successfully instantiate
                                          the value, make the module implemented. */
/** @} plugintypestoreopts */

/**
 * @brief Callback to store and canonize the given @p value according to the given @p type.
 *
 * Value must always be correctly stored meaning all the other type callbacks (such as print or compare)
 * must function as expected.
 *
 * Note that the \p value string is not necessarily zero-terminated. The provided \p value_len is always correct.
 * All store function have to free dynamically allocated value in case they dont used it.
 * Also, in case of any error store function have to call free on dynamically allocated value.
 *
 * @param[in] ctx libyang Context
 * @param[in] type Type of the value being stored.
 * @param[in] value Lexical representation of the value to be stored.
 *            It is never NULL, empty string is represented as "" with zero @p value_len.
 * @param[in] value_len Length (number of bytes) of the given \p value.
 * @param[in] options [Type plugin store options](@ref plugintypestoreopts).
 * @param[in] format Input format of the value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_type_store_resolve_prefix).
 * @param[in] hints Bitmap of [value hints](@ref lydvalhints) of all the allowed value types.
 * @param[in] ctx_node The @p value schema context node.
 * @param[out] storage Storage for the value in the type's specific encoding. All the members should be filled by the plugin.
 * @param[in,out] unres Global unres structure for newly implemented modules. Set only if ::LY_TYPE_STORE_IMPLEMENT is used.
 * @param[out] err Optionally provided error information in case of failure. If not provided to the caller, a generic
 *             error message is prepared instead. The error structure can be created by ::ly_err_new().
 * @return LY_SUCCESS on success,
 * @return LY_EINCOMPLETE in case the ::ly_type_validate_clb should be called to finish value validation in data,
 * @return LY_ERR value on error.
 */
typedef LY_ERR (*ly_type_store_clb)(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value,
        size_t value_len, uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Callback to validate the stored value in data.
 *
 * This callback is optional for types that can only be validated in a data tree. It must be called and succeed
 * in case the ::ly_type_store_clb callback returned ::LY_EINCOMPLETE for the value to be valid. However, this
 * callback can be called even in other cases (such as separate/repeated validation).
 *
 * @param[in] ctx libyang Context
 * @param[in] type Original type of the value (not necessarily the stored one) being validated.
 * @param[in] ctx_node The value data context node for validation.
 * @param[in] tree External data tree (e.g. when validating RPC/Notification) with possibly referenced data.
 * @param[in,out] storage Storage of the value successfully filled by ::ly_type_store_clb. May be modified.
 * @param[out] err Optionally provided error information in case of failure. If not provided to the caller, a generic
 *             error message is prepared instead. The error structure can be created by ::ly_err_new().
 * @return LY_SUCCESS on success,
 * @return LY_ERR value on error.
 */
typedef LY_ERR (*ly_type_validate_clb)(const struct ly_ctx *ctx, const struct lysc_type *type,
        const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err);

/**
 * @brief Callback for comparing 2 values of the same type.
 *
 * In case the value types (::lyd_value.realtype) are different, ::LY_ENOT must always be returned.
 * It can be assumed that the same libyang context (dictionary) was used for storing both values.
 *
 * @param[in] val1 First value to compare.
 * @param[in] val2 Second value to compare.
 * @return LY_SUCCESS if values are same (according to the type's definition of being same).
 * @return LY_ENOT if values differ.
 */
typedef LY_ERR (*ly_type_compare_clb)(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Callback to getting the canonical value of the data stored in @p value.
 *
 * @param[in] value Value to print.
 * @param[in] format Format in which the data are supposed to be printed.
 *            Only 2 formats are currently implemented: LYD_XML and LYD_JSON.
 * @param[in] prefix_data Format-specific data for getting any prefixes (see ::ly_type_print_get_prefix()).
 * @param[out] dynamic Flag if the returned string is dynamically allocated. In such a case the caller is responsible
 *            for freeing it.
 * @return String with the value of @p value in specified @p format. According to the returned @p dynamic flag, caller
 *         can be responsible for freeing allocated memory.
 * @return NULL in case of error.
 */
typedef const char *(*ly_type_print_clb)(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data,
        ly_bool *dynamic);

/**
 * @brief Callback to duplicate data in data structure. Note that callback is even responsible for
 * duplicating ::lyd_value.canonical.
 *
 * @param[in] ctx libyang context of the @p dup. Note that the context of @p original and @p dup might not be the same.
 * @param[in] original Original data structure to be duplicated.
 * @param[in,out] dup Prepared data structure to be filled with the duplicated data of @p original.
 * @return LY_SUCCESS after successful duplication.
 * @return LY_ERR value on error.
 */
typedef LY_ERR (*ly_type_dup_clb)(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup);

/**
 * @brief Callback for freeing the user type values stored by ::ly_type_store_clb.
 *
 * Note that this callback is responsible also for freeing the canonized member in the @p value.
 *
 * @param[in] ctx libyang ctx to enable correct manipulation with values that are in the dictionary.
 * @param[in,out] value Value structure to free the data stored there by the plugin's ::ly_type_store_clb callback
 */
typedef void (*ly_type_free_clb)(const struct ly_ctx *ctx, struct lyd_value *value);

/**
 * @brief Hold type-specific functions for various operations with the data values.
 *
 * libyang includes set of plugins for all the built-in types. They are, by default, inherited to the derived types.
 * However, if the user type plugin for the specific type is loaded, the plugin can provide it's own functions.
 * The built-in types plugin callbacks are public, so even the user type plugins can use them to do part of their own
 * functionality.
 */
struct lyplg_type {
    LY_DATA_TYPE type;               /**< implemented type, use LY_TYPE_UNKNOWN for derived data types */
    ly_type_store_clb store;         /**< store and canonize the value in the type-specific way */
    ly_type_validate_clb validate;   /**< optional, validate the value in the type-specific way in data */
    ly_type_compare_clb compare;     /**< comparison callback to compare 2 values of the same type */
    ly_type_print_clb print;         /**< printer callback to get string representing the value */
    ly_type_dup_clb duplicate;       /**< data duplication callback */
    ly_type_free_clb free;           /**< optional function to free the type-spceific way stored value */
    const char *id;                  /**< Plugin identification (mainly for distinguish incompatible versions when used by external tools) */
};

/**
 * @brief List of type plugins for built-in types.
 *
 * TODO hide behind some plugin getter
 */
extern struct lyplg_type ly_builtin_type_plugins[LY_DATA_TYPE_COUNT];

/**
 * @brief Generic simple comparison callback checking the canonical value.
 * Implementation of the ::ly_type_compare_clb.
 */
LY_ERR ly_type_compare_simple(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Generic simple printer callback of the canonized value.
 * Implementation of the ::ly_type_print_clb.
 */
const char *ly_type_print_simple(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, ly_bool *dynamic);

/**
 * @brief Generic simple duplication callback.
 * Implementation of the ::ly_type_dup_clb.
 */
LY_ERR ly_type_dup_simple(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup);

/**
 * @brief Generic cleanup callback freeing only the canonized value in ::lyd_value.canonical.
 * Simple implementation of the ::ly_type_free_clb.
 */
void ly_type_free_simple(const struct ly_ctx *ctx, struct lyd_value *value);

/*
 * Binary built-in type functions
 */

/**
 * @brief Validate, canonize and store value of the YANG built-in binary type.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_binary(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/*
 * Bits built-in type functions
 */

/**
 * @brief Validate, canonize and store value of the YANG built-in bits type.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_bits(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Duplication callback of the bits values.
 * Implementation of the ly_type_dup_clb.
 */
LY_ERR ly_type_dup_bits(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup);

/**
 * @brief Free value of the YANG built-in bits type.
 * Implementation of the ly_type_free_clb.
 */
void ly_type_free_bits(const struct ly_ctx *ctx, struct lyd_value *value);

/*
 * Boolean built-in type functions
 */

/**
 * @brief Validate and store value of the YANG built-in boolean type.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_boolean(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/*
 * Decimal64 built-in type functions
 */

/**
 * @brief Validate, canonize and store value of the YANG built-in decimal64 types.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_decimal64(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/*
 * Decimal64 built-in type functions
 */

/**
 * @brief Validate and store value of the YANG built-in empty type.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_empty(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Comparison callback for built-in empty type.
 * Implementation of the ly_type_compare_clb.
 */
LY_ERR ly_type_compare_empty(const struct lyd_value *val1, const struct lyd_value *val2);

/*
 * Enumeration built-in type functions
 */

/**
 * @brief Validate, canonize and store value of the YANG built-in enumeration type.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_enum(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/*
 * Identityref built-in type functions
 */

/**
 * @brief Validate, canonize and store value of the YANG built-in identiytref type.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_identityref(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Comparison callback for built-in identityref type.
 * Implementation of the ly_type_compare_clb.
 */
LY_ERR ly_type_compare_identityref(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Printer callback printing identityref value.
 * Implementation of the ly_type_print_clb.
 */
const char *ly_type_print_identityref(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data,
        ly_bool *dynamic);

/*
 * Instance-identifier built-in type functions
 */

/**
 * @brief Validate and store value of the YANG built-in instance-identifier type.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_instanceid(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Comparison callback checking the instance-identifier value.
 * Implementation of the ly_type_compare_clb.
 */
LY_ERR ly_type_compare_instanceid(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Printer callback printing the instance-identifier value.
 * Implementation of the ly_type_print_clb.
 */
const char *ly_type_print_instanceid(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data,
        ly_bool *dynamic);

/**
 * @brief Duplication callback of the instance-identifier values.
 * Implementation of the ly_type_dup_clb.
 */
LY_ERR ly_type_dup_instanceid(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup);

/**
 * @brief Validate value of the YANG built-in instance-identifier type.
 * Implementation of the ly_type_validate_clb.
 */
LY_ERR ly_type_validate_instanceid(const struct ly_ctx *ctx, const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err);

/**
 * @brief Free value of the YANG built-in instance-identifier types.
 * Implementation of the ly_type_free_clb.
 */
void ly_type_free_instanceid(const struct ly_ctx *ctx, struct lyd_value *value);

/*
 * Integer built-in types functions
 */

/**
 * @brief Validate, canonize and store value of the YANG built-in signed integer types.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_int(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Validate and canonize value of the YANG built-in unsigned integer types.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_uint(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/*
 * Leafref built-in type functions
 */

/**
 * @brief Validate, canonize and store value of the YANG built-in leafref type.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_leafref(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Comparison callback checking the leafref value.
 * Implementation of the ly_type_compare_clb.
 */
LY_ERR ly_type_compare_leafref(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Printer callback printing the leafref value.
 * Implementation of the ly_type_print_clb.
 */
const char *ly_type_print_leafref(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data,
        ly_bool *dynamic);

/**
 * @brief Duplication callback of the leafref values.
 * Implementation of the ly_type_dup_clb.
 */
LY_ERR ly_type_dup_leafref(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup);

/**
 * @brief Validate value of the YANG built-in leafref type.
 * Implementation of the ly_type_validate_clb.
 */
LY_ERR ly_type_validate_leafref(const struct ly_ctx *ctx, const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err);

/**
 * @brief Free value of the YANG built-in leafref types.
 * Implementation of the ly_type_free_clb.
 */
void ly_type_free_leafref(const struct ly_ctx *ctx, struct lyd_value *value);

/*
 * String built-in type functions
 */

/**
 * @brief Validate and store value of the YANG built-in string type.
 * Implementation of the ::ly_type_store_clb.
 */
LY_ERR ly_type_store_string(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/*
 * Union built-in type functions
 */

/**
 * @brief Validate, canonize and store value of the YANG built-in union type.
 * Implementation of the ly_type_store_clb.
 */
LY_ERR ly_type_store_union(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Comparison callback checking the union value.
 * Implementation of the ly_type_compare_clb.
 */
LY_ERR ly_type_compare_union(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Printer callback printing the union value.
 * Implementation of the ly_type_print_clb.
 */
const char *ly_type_print_union(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data,
        ly_bool *dynamic);

/**
 * @brief Duplication callback of the union values.
 * Implementation of the ly_type_dup_clb.
 */
LY_ERR ly_type_dup_union(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup);

/**
 * @brief Validate value of the YANG built-in union type.
 * Implementation of the ly_type_validate_clb.
 */
LY_ERR ly_type_validate_union(const struct ly_ctx *ctx, const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err);

/**
 * @brief Free value of the YANG built-in union types.
 * Implementation of the ly_type_free_clb.
 */
void ly_type_free_union(const struct ly_ctx *ctx, struct lyd_value *value);

/*
 * Other supporting functions
 */

/**
 * @brief Unsigned integer value parser and validator.
 *
 * @param[in] datatype Type of the integer for logging.
 * @param[in] base Base of the integer's lexical representation. In case of built-in types, data must be represented in decimal format (base 10),
 * but default values in schemas can be represented also as hexadecimal or octal values (base 0).
 * @param[in] min Lower bound of the type.
 * @param[in] max Upper bound of the type.
 * @param[in] value Value string to parse.
 * @param[in] value_len Length of the @p value (mandatory parameter).
 * @param[out] ret Parsed integer value (optional).
 * @param[out] err Error information in case of failure. The error structure can be freed by ::ly_err_free().
 * @return LY_ERR value according to the result of the parsing and validation.
 */
LY_ERR ly_type_parse_int(const char *datatype, int base, int64_t min, int64_t max, const char *value, size_t value_len,
        int64_t *ret, struct ly_err_item **err);

/**
 * @brief Unsigned integer value parser and validator.
 *
 * @param[in] datatype Type of the unsigned integer for logging.
 * @param[in] base Base of the integer's lexical representation. In case of built-in types, data must be represented in decimal format (base 10),
 * but default values in schemas can be represented also as hexadecimal or octal values (base 0).
 * @param[in] max Upper bound of the type.
 * @param[in] value Value string to parse.
 * @param[in] value_len Length of the @p value (mandatory parameter).
 * @param[out] ret Parsed unsigned integer value (optional).
 * @param[out] err Error information in case of failure. The error structure can be freed by ::ly_err_free().
 * @return LY_ERR value according to the result of the parsing and validation.
 */
LY_ERR ly_type_parse_uint(const char *datatype, int base, uint64_t max, const char *value, size_t value_len,
        uint64_t *ret, struct ly_err_item **err);

/**
 * @brief Convert a string with a decimal64 value into libyang representation:
 * ret = value * 10^fraction-digits
 *
 * @param[in] fraction_digits Fraction-digits of the decimal64 type.
 * @param[in] value Value string to parse.
 * @param[in] value_len Length of the @p value (mandatory parameter).
 * @param[out] ret Parsed decimal64 value representing original value * 10^fraction-digits (optional).
 * @param[out] err Error information in case of failure. The error structure can be freed by ::ly_err_free().
 * @return LY_ERR value according to the result of the parsing and validation.
 */
LY_ERR ly_type_parse_dec64(uint8_t fraction_digits, const char *value, size_t value_len, int64_t *ret, struct ly_err_item **err);

/**
 * @brief Decide if the @p derived identity is derived from (based on) the @p base identity.
 *
 * @param[in] base Expected base identity.
 * @param[in] derived Expected derived identity.
 * @return LY_SUCCESS if @p derived IS based on the @p base identity.
 * @return LY_ENOTFOUND if @p derived IS NOT not based on the @p base identity.
 */
LY_ERR ly_type_identity_isderived(struct lysc_ident *base, struct lysc_ident *derived);

/**
 * @brief Data type validator for a range/length-restricted values.
 *
 * @param[in] basetype Base built-in type of the type with the range specified to get know if the @p range structure represents range or length restriction.
 * @param[in] range Range (length) restriction information.
 * @param[in] value Value to check. In case of basetypes using unsigned integer values, the value is actually cast to uint64_t.
 * @param[in] strval String representation of the @p value for error logging.
 * @param[out] err Error information in case of failure. The error structure can be freed by ::ly_err_free().
 * @return LY_ERR value according to the result of the validation.
 */
LY_ERR ly_type_validate_range(LY_DATA_TYPE basetype, struct lysc_range *range, int64_t value, const char *strval,
        struct ly_err_item **err);

/**
 * @brief Data type validator for pattern-restricted string values.
 *
 * @param[in] patterns ([Sized array](@ref sizedarrays)) of the compiled list of pointers to the pattern restrictions.
 * The array can be found in the ::lysc_type_str.patterns structure.
 * @param[in] str String to validate.
 * @param[in] str_len Length (number of bytes) of the string to validate (mandatory).
 * @param[out] err Error information in case of failure or non-matching @p str. The error structure can be freed by ::ly_err_free().
 * @return LY_SUCCESS when @p matches all the patterns.
 * @return LY_EVALID when @p does not match any of the patterns.
 * @return LY_ESYS in case of PCRE2 error.
 */
LY_ERR ly_type_validate_patterns(struct lysc_pattern **patterns, const char *str, size_t str_len, struct ly_err_item **err);

/**
 * @brief Find leafref target in data.
 *
 * @param[in] lref Leafref type.
 * @param[in] node Context node.
 * @param[in] value Target value.
 * @param[in] tree Full data tree to search in.
 * @param[out] target Optional found target.
 * @param[out] errmsg Error message in case of error.
 * @return LY_ERR value.
 */
LY_ERR ly_type_resolve_leafref(const struct lysc_type_leafref *lref, const struct lyd_node *node, struct lyd_value *value,
        const struct lyd_node *tree, struct lyd_node **target, char **errmsg);

/** @} types */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_TYPES_H_ */
