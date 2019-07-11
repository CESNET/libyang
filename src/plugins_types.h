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
#include "tree_schema.h"
#include "tree_data.h"

/**
 * @brief Helper function for various plugin functions to generate error information structure.
 *
 * @param[in] level Error level of the error.
 * @param[in] code Code of the error.
 * @param[in] vecode Validity error code in case of LY_EVALID error code.
 * @param[in] msg Error message.
 * @param[in] path Path to the node causing the error.
 * @param[in] apptag Error-app-tag value.
 * @return NULL in case of memory allocation failure.
 * @return Created error information structure that can be freed using ly_err_free().
 */
struct ly_err_item *ly_err_new(LY_LOG_LEVEL level, LY_ERR code, LY_VECODE vecode, char *msg, char *path, char *apptag);

/**
 * @brief Destructor for the error records created with ly_err_new().
 *
 * Compatible with the free(), so usable as a generic callback.
 *
 * @param[in] ptr Error record to free. With the record, also all the records connected after this one are freed.
 */
void ly_err_free(void *ptr);

/**
 * @defgroup plugintypeopts Options for type plugin callbacks. The same set of the options is passed to all the type's callbacks used together.
 *
 * Options applicable to ly_type_validate_clb() and ly_typestore_clb.
 * @{
 */
#define LY_TYPE_OPTS_CANONIZE     0x01 /**< Canonize the given value and store it (insert into the context's dictionary)
                                            as the value's canonized string */
#define LY_TYPE_OPTS_DYNAMIC      0x02 /**< Flag for the dynamically allocated string value, in this case the value is supposed to be freed
                                            or directly inserted into the context's dictionary (e.g. in case of canonization).
                                            In any case, the caller of the callback does not free the provided string value after calling
                                            the type's callbacks with this option */
#define LY_TYPE_OPTS_STORE        0x04 /**< Flag announcing calling of ly_type_store_clb() */
#define LY_TYPE_OPTS_SCHEMA       0x08 /**< Flag for the value used in schema instead of the data tree */
#define LY_TYPE_OPTS_INCOMPLETE_DATA 0x10 /**< Flag for the case the data trees are not yet complete. In this case the plagin should do what it
                                            can (e.g. store the canonical/auxiliary value if it is requested) and in the case of need to use
                                            data trees (checking require-instance), it returns LY_EINCOMPLETE.
                                            Caller is supposed to call such validation callback again later with complete data trees. */
#define LY_TYPE_OPTS_SECOND_CALL 0x20  /**< Flag for the second call of the callback when the first call returns LY_EINCOMPLETE,
                                            other options should be the same as for the first call. */

/**
 * @}
 */

/**
 * @defgroup plugintypeflags Various flags for type plugins usage.
 * Options applicable to ly_type_validate_clb().
 * @{
 */
#define LY_TYPE_FLAG_PREFIXES 0x01     /**< The value contains prefixes, so when printing XML,
                                            get the namespaces connected with the prefixes and print them. */
/**
 * @}
 */

/**
 * @brief Callback to validate, canonize and store (optionally, according to the given @p options) the given @p value according to the given @p type.
 *
 * Note that the \p value string is not necessarily zero-terminated. The provided \p value_len is always correct.
 *
 * @param[in] ctx libyang Context
 * @param[in] type Type of the value being canonized.
 * @param[in] value Lexical representation of the value to be validated (and canonized).
 *            It is never NULL, empty string is represented as "" with zero @p value_len.
 * @param[in] value_len Length of the given \p value.
 * @param[in] options [Type plugin options ](@ref plugintypeopts).
 *
 * @param[in] get_prefix Parser-specific getter to resolve prefixes used in the value strings.
 * @param[in] parser Parser's data for @p get_prefix
 * @param[in] format Input format of the data.
 * @param[in] context_node The @p value's node for the case that the require-instance restriction is supposed to be resolved. This argument is of
 *            lys_node (in case LY_TYPE_OPTS_INCOMPLETE_DATA set in @p options) or lyd_node structure.
 * @param[in] trees ([Sized array](@ref sizedarrays)) of external data trees (e.g. when validating RPC/Notification) where the required data
 *            instance can be placed.
 *
 * @param[in,out] storage If LY_TYPE_OPTS_STORE option set, the parsed data are stored into this structure in the type's specific way.
 *             If the @p canonized differs from the storage's canonized member, the canonized value is also stored here despite the
 *             LY_TYPE_OPTS_CANONIZE option.
 * @param[out] canonized If LY_TYPE_OPTS_CANONIZE option set, the canonized string stored in the @p ctx dictionary is returned via this parameter.
 *             This is usable mainly in the case the LY_TYPE_OPTS_STORE option is not set and the canonized value is needed outside the lyd_value
 *             structure, otherwise the canonized value is stored in the @p storage automatically.
 * @param[out] err Optionally provided error information in case of failure. If not provided to the caller, a generic error message is prepared instead.
 * The error structure can be created by ly_err_new().
 * @return LY_SUCCESS on success
 * @return LY_EINCOMPLETE in case the option included LY_TYPE_OPTS_INCOMPLETE_DATA flag and the data @p trees are needed to finish the validation.
 * @return LY_ERR value if an error occurred and the value could not be canonized following the type's rules.
 */
typedef LY_ERR (*ly_type_store_clb)(struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                                    ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format,
                                    const void *context_node, struct lyd_node **trees,
                                    struct lyd_value *storage, const char **canonized, struct ly_err_item **err);

/**
 * @brief Callback for comparing 2 values of the same type.
 *
 * Caller is responsible to provide values of the SAME type.
 *
 * @param[in] val1 First value to compare.
 * @param[in] val2 Second value to compare.
 * @return LY_SUCCESS if values are same (according to the type's definition of being same).
 * @return LY_EVALID if values differ.
 */
typedef LY_ERR (*ly_type_compare_clb)(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Callback for freeing the user type values stored by ly_type_store_clb().
 *
 * Note that this callback is responsible also for freeing the canonized member in the @p value.
 *
 * @param[in] ctx libyang ctx to enable correct manipulation with values that are in the dictionary.
 * @param[in] type Type of the stored value.
 * @param[in,out] value Value structure to free the data stored there by the plugin's ly_type_store_clb() callback
 */
typedef void (*ly_type_free_clb)(struct ly_ctx *ctx, struct lysc_type *type, struct lyd_value *value);

/**
 * @brief Hold type-specific functions for various operations with the data values.
 *
 * libyang includes set of plugins for all the built-in types. They are, by default, inherited to the derived types.
 * However, if the user type plugin for the specific type is loaded, the plugin can provide it's own functions.
 * The built-in types plugins and are public, so even the user type plugins can use them to do part of their own functionality.
 */
struct lysc_type_plugin {
    LY_DATA_TYPE type;               /**< implemented type, use LY_TYPE_UNKNOWN for derived data types */
    ly_type_store_clb store;         /**< function to validate, canonize and store (according to the options) the value in the type-specific way */
    ly_type_compare_clb compare;     /**< comparison callback to compare 2 values of the same type */
    ly_type_free_clb free;           /**< optional function to free the type-spceific way stored value */
    uint32_t flags;                  /**< [type flags ](@ref plugintypeflags). */
};

/**
 * @brief List of type plugins for built-in types.
 */
extern struct lysc_type_plugin ly_builtin_type_plugins[LY_DATA_TYPE_COUNT];

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
 * @param[out] err Error information in case of failure. The error structure can be freed by ly_err_free().
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
 * @param[out] err Error information in case of failure. The error structure can be freed by ly_err_free().
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
 * @param[out] err Error information in case of failure. The error structure can be freed by ly_err_free().
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
 * @param[in] canonized Canonized @p value for error logging.
 * @param[out] err Error information in case of failure. The error structure can be freed by ly_err_free().
 * @return LY_ERR value according to the result of the validation.
 */
LY_ERR ly_type_validate_range(LY_DATA_TYPE basetype, struct lysc_range *range, int64_t value, const char *canonized, struct ly_err_item **err);

/**
 * @brief Data type validator for pattern-restricted string values.
 *
 * @param[in] patterns ([Sized array](@ref sizedarrays)) of the compiled list of pointers to the pattern restrictions.
 * The array can be found in the lysc_type_str::patterns structure.
 * @param[in] str String to validate.
 * @param[in] str_len Length of the string to validate (mandatory).
 * @param[out] err Error information in case of failure or non-matching @p str. The error structure can be freed by ly_err_free().
 * @return LY_SUCCESS when @p matches all the patterns.
 * @return LY_EVALID when @p does not match any of the patterns.
 * @return LY_ESYS in case of PCRE2 error.
 */
LY_ERR ly_type_validate_patterns(struct lysc_pattern **patterns, const char *str, size_t str_len, struct ly_err_item **err);

/**
 * @brief Helper function for type validation callbacks to prepare list of all possible prefixes used in the value string.
 *
 * @param[in] ctx libyang context.
 * @param[in] value Value string to be parsed.
 * @param[in] value_len Length of the @p value string.
 * @param[in] get_prefix Parser-specific getter to resolve prefixes used in the value strings.
 * @param[in] parser Parser's data for @p get_prefix.
 * @return Created [sized array](@ref sizedarrays) of prefix mappings, NULL in case of error.
 */
struct lyd_value_prefix *ly_type_get_prefixes(struct ly_ctx *ctx, const char *value, size_t value_len,
                                              ly_clb_resolve_prefix get_prefix, void *parser);

#endif /* LY_PLUGINS_TYPES_H_ */
