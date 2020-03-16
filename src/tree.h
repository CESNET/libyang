/**
 * @file tree.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang geenric macros and functions to work with YANG schema or data trees.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_H_
#define LY_TREE_H_

#include "tree_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Macro selector for other LY_ARRAY_* macros, do not use directly!
 */
#define LY_ARRAY_SELECT(_1, _2, NAME, ...) NAME

/**
 * @brief Helper macro to go through sized-arrays with a pointer iterator.
 *
 * Use with opening curly bracket (`{`).
 *
 * @param[in] ARRAY Array to go through
 * @param[in] TYPE Type of the records in the ARRAY
 * @param[out] ITER Iterating pointer to the item being processed in each loop
 */
#define LY_ARRAY_FOR_ITER(ARRAY, TYPE, ITER) \
    for (ITER = ARRAY; \
         (ARRAY) && ((void*)ITER - (void*)ARRAY)/(sizeof(TYPE)) < (*((uint32_t*)(ARRAY) - 1)); \
         ITER = (void*)((TYPE*)ITER + 1))

/**
 * @brief Helper macro to go through sized-arrays with a numeric iterator.
 *
 * Use with opening curly bracket (`{`).
 *
 * To access an item with the INDEX value, use always LY_ARRAY_INDEX macro!
 *
 * @param[in] ARRAY Array to go through
 * @param[out] INDEX Iterating index of the item being processed in each loop
 */
#define LY_ARRAY_FOR_INDEX(ARRAY, INDEX) \
    for (INDEX = 0; \
         ARRAY && INDEX < (*((uint32_t*)(ARRAY) - 1)); \
         ++INDEX)

/**
 * @defgroup schematree Schema Tree
 * @{
 *
 * Data structures and functions to manipulate and access schema tree.
 */

/**
 * @brief Get a number of records in the ARRAY.
 *
 * Does not check if array exists!
 */
#define LY_ARRAY_SIZE(ARRAY) (*((uint32_t*)(ARRAY) - 1))

/**
 * @brief Sized-array iterator (for-loop).
 *
 * Use with opening curly bracket (`{`).
 *
 * There are 2 variants:
 *
 *     LY_ARRAY_FOR(ARRAY, TYPE, ITER)
 *
 * Where ARRAY is a sized-array to go through, TYPE is the type of the items in the ARRAY and ITER is a pointer variable
 * providing the items of the ARRAY in the loops. This functionality is provided by LY_ARRAY_FOR_ITER macro
 *
 *     LY_ARRAY_FOR(ARRAY, INDEX)
 *
 * The ARRAY is again a sized-array to go through, the INDEX is a variable (unsigned integer) for storing iterating ARRAY's index
 * to access the items of ARRAY in the loops. This functionality is provided by LY_ARRAY_FOR_INDEX macro.
 */
#define LY_ARRAY_FOR(ARRAY, ...) LY_ARRAY_SELECT(__VA_ARGS__, LY_ARRAY_FOR_ITER, LY_ARRAY_FOR_INDEX)(ARRAY, __VA_ARGS__)

/**
 * @brief Macro to iterate via all sibling elements without affecting the list itself
 *
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type.
 *
 * Use with opening curly bracket (`{`). All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param ELEM Iterator.
 */
#define LY_LIST_FOR(START, ELEM) \
    for ((ELEM) = (START); \
         (ELEM); \
         (ELEM) = (ELEM)->next)

/**
 * @brief Macro to iterate via all sibling elements allowing to modify the list itself (e.g. removing elements)
 *
 * Use with opening curly bracket (`{`). All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param NEXT Temporary storage to allow removing of the current iterator content.
 * @param ELEM Iterator.
 */
#define LY_LIST_FOR_SAFE(START, NEXT, ELEM) \
    for ((ELEM) = (START); \
         (ELEM) ? (NEXT = (ELEM)->next, 1) : 0; \
         (ELEM) = (NEXT))

/**
 * @brief YANG built-in types
 */
typedef enum
{
    LY_TYPE_UNKNOWN = 0, /**< Unknown type */
    LY_TYPE_BINARY, /**< Any binary data ([RFC 6020 sec 9.8](http://tools.ietf.org/html/rfc6020#section-9.8)) */
    LY_TYPE_UINT8, /**< 8-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT16, /**< 16-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT32, /**< 32-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT64, /**< 64-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_STRING, /**< Human-readable string ([RFC 6020 sec 9.4](http://tools.ietf.org/html/rfc6020#section-9.4)) */
    LY_TYPE_BITS, /**< A set of bits or flags ([RFC 6020 sec 9.7](http://tools.ietf.org/html/rfc6020#section-9.7)) */
    LY_TYPE_BOOL, /**< "true" or "false" ([RFC 6020 sec 9.5](http://tools.ietf.org/html/rfc6020#section-9.5)) */
    LY_TYPE_DEC64, /**< 64-bit signed decimal number ([RFC 6020 sec 9.3](http://tools.ietf.org/html/rfc6020#section-9.3))*/
    LY_TYPE_EMPTY, /**< A leaf that does not have any value ([RFC 6020 sec 9.11](http://tools.ietf.org/html/rfc6020#section-9.11)) */
    LY_TYPE_ENUM, /**< Enumerated strings ([RFC 6020 sec 9.6](http://tools.ietf.org/html/rfc6020#section-9.6)) */
    LY_TYPE_IDENT, /**< A reference to an abstract identity ([RFC 6020 sec 9.10](http://tools.ietf.org/html/rfc6020#section-9.10)) */
    LY_TYPE_INST, /**< References a data tree node ([RFC 6020 sec 9.13](http://tools.ietf.org/html/rfc6020#section-9.13)) */
    LY_TYPE_LEAFREF, /**< A reference to a leaf instance ([RFC 6020 sec 9.9](http://tools.ietf.org/html/rfc6020#section-9.9))*/
    LY_TYPE_UNION, /**< Choice of member types ([RFC 6020 sec 9.12](http://tools.ietf.org/html/rfc6020#section-9.12)) */
    LY_TYPE_INT8, /**< 8-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT16, /**< 16-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT32, /**< 32-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT64, /**< 64-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
} LY_DATA_TYPE;
#define LY_DATA_TYPE_COUNT 20 /**< Number of different types */

/**
 * @brief Stringified YANG built-in data types
 */
extern const char* ly_data_type2str[LY_DATA_TYPE_COUNT];

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_H_ */
