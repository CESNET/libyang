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
 * @defgroup plugintypevalidateopts Options for type plugin validation callback.
 * Options applicable to ly_type_validate_clb().
 * @{
 */
#define LY_TYPE_VALIDATE_CANONIZE 0x01 /**< Canonize the given value and store it (insert into the context's dictionary)
                                            as the value's canonized string */
#define LY_TYPE_VALIDATE_DYNAMIC  0x02 /**< Flag for the dynamically allocated string value, in this case the value is supposed to be freed
                                            or directly used to insert into the node's value structure in case of canonization.
                                            In any case, the caller of the callback does not free the provided string value after calling
                                            the ly_type_validate_clb() with this option */

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
 * @brief Callback to validate the given @p value according to the given @p type. Optionaly, it can be requested to canonize the value.
 *
 * Note that the \p value string is not necessarily zero-terminated. The provided \p value_len is always correct.
 *
 * @param[in] ctx libyang Context
 * @param[in] type Type of the value being canonized.
 * @param[in] value Lexical representation of the value to be validated (and canonized).
 * @param[in] value_len Length of the given \p value.
 * @param[in] options [Type validation options ](@ref plugintypevalidateopts).
 * @param[out] canonized If LY_TYPE_VALIDATE_CANONIZE option set, the canonized string stored in the @p ctx dictionary is returned via this parameter.
 * @param[out] err Optionally provided error information in case of failure. If not provided to the caller, a generic error message is prepared instead.
 * The error structure can be created by ly_err_new().
 * @return LY_SUCCESS on success
 * @return LY_ERR value if an error occurred and the value could not be canonized following the type's rules.
 */
typedef LY_ERR (*ly_type_validate_clb)(struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                                       const char **canonized, struct ly_err_item **err);

/**
 * @brief Callback for storing user type values.
 *
 * This callback should overwrite the value stored in \p value using some custom encoding. Be careful,
 * if the type is #LY_TYPE_BITS, the bits must be freed before overwritting the union value.
 *
 * @param[in] ctx libyang ctx to enable correct manipulation with values that are in the dictionary.
 * @param[in] type Type of the value being stored.
 * @param[in] value_str Canonized string value to be stored.
 * @param[in,out] value Value structure to store the data in the type plugin specific way.
 * @param[out] err Optionally provided error information in case of failure. If not provided to the caller, a generic error message is prepared instead.
 * The error structure can be created by ly_err_new().
 * @return LY_SUCCESS on success
 * @return LY_ERR value if an error occurred and the value could not be stored for any reason.
 */
typedef LY_ERR (*ly_type_store_clb)(struct ly_ctx *ctx, struct lysc_type *type, const char *value_str, struct lyd_value *value, struct ly_err_item **err);

/**
 * @brief Hold type-specific functions for various operations with the data values.
 *
 * libyang includes set of plugins for all the built-in types. They are, by default, inherited to the derived types.
 * However, if the user type plugin for the specific type is loaded, the plugin can provide it's own functions.
 * The built-in types plugins and are public, so even the user type plugins can use them to do part of their own functionality.
 */
struct lysc_type_plugin {
    LY_DATA_TYPE type;               /**< implemented type, use LY_TYPE_UNKNOWN for derived data types */
    ly_type_validate_clb validate;   /**< function to validate and canonize given value */
    ly_type_store_clb store;         /**< function to store the value in the type-specific way */
    uint32_t flags;                  /**< [type flags ](@ref plugintypeflags). */
};

/**
 * @brief List of type plugins for built-in types.
 */
extern struct lysc_type_plugin ly_builtin_type_plugins[LY_DATA_TYPE_COUNT];

/**
 * @brief Data type validator for a range/length-restricted values.
 *
 * @param[in] ctx libyang context
 * @param[in] range Range (length) restriction information.
 * @param[in] value Value to check. In case of basetypes using unsigned integer values, the value is actually cast to uint64_t.
 * @param[out] err Optionally provided error information in case of failure. If not provided to the caller, a generic error message is prepared instead.
 * The error structure can be created by ly_err_new().
 * @return LY_ERR value according to the result of the validation.
 */
LY_ERR ly_type_validate_range(LY_DATA_TYPE basetype, struct lysc_range *range, int64_t value, struct ly_err_item **err);


#endif /* LY_PLUGINS_TYPES_H_ */
