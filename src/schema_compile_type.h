/**
 * @file schema_compile_type.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Header for schema compilation of type.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_SCHEMA_COMPILE_TYPE_H_
#define LY_SCHEMA_COMPILE_TYPE_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "tree_schema.h"

struct ly_set;
struct lysc_ctx;

/**
 * @brief Compile the parsed range restriction.
 *
 * @param[in] ctx Compile context.
 * @param[in] range_p Parsed range structure to compile.
 * @param[in] basetype Base YANG built-in type of the node with the range restriction.
 * @param[in] length_restr Flag to distinguish between range and length restrictions. Only for logging.
 * @param[in] frdigits The fraction-digits value in case of LY_TYPE_DEC64 basetype.
 * @param[in] base_range Range restriction of the type from which the current type is derived. The current
 * range restriction must be more restrictive than the base_range.
 * @param[in,out] range Pointer to the created current range structure.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_type_range(struct lysc_ctx *ctx, const struct lysp_restr *range_p, LY_DATA_TYPE basetype,
        ly_bool length_restr, uint8_t frdigits, struct lysc_range *base_range, struct lysc_range **range);

/**
 * @brief Compile parsed pattern restriction in conjunction with the patterns from base type.
 *
 * @param[in] ctx Compile context.
 * @param[in] patterns_p Array of parsed patterns from the current type to compile.
 * @param[in] type_pmod Parsed module of the type with @p patterns_p used to check for OC 'regexp-posix' extension.
 * @param[in] base_patterns Compiled patterns from the type from which the current type is derived.
 * Patterns from the base type are inherited to have all the patterns that have to match at one place.
 * @param[out] patterns Pointer to the storage for the patterns of the current type.
 * @return LY_ERR LY_SUCCESS, LY_EMEM, LY_EVALID.
 */
LY_ERR lys_compile_type_patterns(struct lysc_ctx *ctx, const struct lysp_restr *patterns_p,
        const struct lysp_module *type_pmod, struct lysc_pattern **base_patterns, struct lysc_pattern ***patterns);

/**
 * @brief Compile parsed type's enum structures (for enumeration and bits types).
 *
 * @param[in] ctx Compile context.
 * @param[in] enums_p Array of the parsed enum structures to compile.
 * @param[in] basetype Base YANG built-in type from which the current type is derived. Only LY_TYPE_ENUM and LY_TYPE_BITS are expected.
 * @param[in] base_enums Array of the compiled enums information from the (latest) base type to check if the current enums are compatible.
 * @param[out] bitenums Newly created array of the compiled bitenums information for the current type.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile_type_enums(struct lysc_ctx *ctx, const struct lysp_type_enum *enums_p, LY_DATA_TYPE basetype,
        struct lysc_type_bitenum_item *base_enums, struct lysc_type_bitenum_item **bitenums);

/**
 * @brief Find the typedef and basetype of a parsed type.
 *
 * @param[in] ctx Compile context.
 * @param[in] context_pnode Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] context_flags Flags of the context node or the referencing typedef.
 * @param[in] context_name Name of the context node or referencing typedef for logging.
 * @param[in] type_p Parsed type to compile.
 * @param[out] units Storage for inheriting units value from the typedefs the current type derives from.
 * @param[out] dflt Default value for the type.
 * @param[in,out] tpdf_chain Empty set of typedefs, is added to.
 * @param[out] basetype Basetype of the final typedef.
 * @return LY_SUCCESS on success;
 * @return LY_ENOTFOUND if a referenced typedef was not found;
 * @return LY_ERR value on error.
 */
LY_ERR lys_compile_type_find_tpdf(struct lysc_ctx *ctx, const struct lysp_node *context_pnode, uint16_t context_flags,
        const char *context_name, const struct lysp_type *type_p, const char **units, struct lysp_qname **dflt,
        struct ly_set *tpdf_chain, LY_DATA_TYPE *basetype);

/**
 * @brief Compile information about the leaf/leaf-list's type.
 *
 * @param[in] ctx Compile context.
 * @param[in] context_pnode Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] context_flags Flags of the context node or the referencing typedef.
 * @param[in] context_name Name of the context node or referencing typedef for logging.
 * @param[in] type_p Parsed type to compile.
 * @param[out] type Newly created (or reused with increased refcount) type structure with the filled information about the type.
 * @param[out] units Storage for inheriting units value from the typedefs the current type derives from.
 * @param[out] dflt Default value for the type.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_type(struct lysc_ctx *ctx, struct lysp_node *context_pnode, uint16_t context_flags,
        const char *context_name, const struct lysp_type *type_p, struct lysc_type **type, const char **units,
        struct lysp_qname **dflt);

#endif /* LY_SCHEMA_COMPILE_TYPE_H_ */
