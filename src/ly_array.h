/**
 * @file ly_array.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang sized array API.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_ARRAY_H_
#define LY_ARRAY_H_

#include <inttypes.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup arrays Sized Array API
 *
 * Generic macros for LY array handling that includes the information about its size.
 *
 * @{
 */

/**
 * @brief Type (i.e. size) of the [sized array](@ref sizedarrays)'s size counter.
 *
 * To print the value via a print format, use LYA_PRI_COUNT_T specifier.
 */
#define LYA_COUNT_T uint64_t

/**
 * @brief Printing format specifier macro for LYA_COUNT_T values.
 */
#define LYA_PRI_COUNT_T PRIu64

/**
 * @brief Helper macro to go through sized-arrays with a numeric iterator.
 *
 * Use with opening curly bracket (`{`).
 *
 * The item on the current INDEX in the ARRAY can be accessed in a standard C way as ARRAY[INDEX].
 *
 * @param[in] ARRAY Sized array to manipulate.
 * @param[out] INDEX Variable for the iterating index of the item being processed in each loop.
 */
#define LYA_FOR(ARRAY, INDEX) \
    for (INDEX = 0; \
         INDEX < LYA_COUNT(ARRAY); \
         ++INDEX)

/**
 * @brief Helper macro to go through sized-arrays with a pointer iterator.
 *
 * Use with opening curly bracket (`{`).
 *
 * @param[in] ARRAY Sized array to manipulate.
 * @param[out] ITER Iterating pointer to the item being processed in each loop.
 */
#define LYA_FOR_EACH(ARRAY, ITER) \
    for (ITER = ARRAY; \
         (ARRAY) && ((LYA_COUNT_T)(ITER - ARRAY) < LYA_COUNT(ARRAY)); \
         ++ITER)

/**
 * @brief Get the number of records in the ARRAY.
 *
 * @param[in] ARRAY Sized array to manipulate.
 */
#define LYA_COUNT(ARRAY) (ARRAY ? LYA_COUNT_(ARRAY) : 0)

/**
 * @brief Internal macro, do not use.
 */
#define LYA_COUNT_(ARRAY) (*((LYA_COUNT_T *)(ARRAY) - 1))

/**
 * @brief Allocate memory of a [sized array](@ref sizedarrays) or resize an existing array.
 *
 * Does not set the size information, it is supposed to be incremented via ::LYA_INCREMENT
 * when the items are filled.
 *
 * Any new memory is zeroed.
 *
 * @param[in,out] ARRAY Sized array to manipulate.
 * @param[in] COUNT New count (size) of the items in the array.
 * @param[in] EACTION Error action on memory allocation error, must include 'return' or 'goto' commands.
 */
#define LYA_PREALLOC(ARRAY, COUNT, EACTION) \
    { \
        LYA_COUNT_T orig_count = LYA_COUNT(ARRAY); \
        void *mem = realloc((ARRAY) ? (LYA_COUNT_T *)(ARRAY) - 1 : NULL, \
                sizeof(LYA_COUNT_T) + (orig_count + COUNT) * sizeof *(ARRAY)); \
        if (!mem) { \
            EACTION; \
        } \
        void *new_array = (LYA_COUNT_T *)mem + 1; \
        memcpy(&(ARRAY), &new_array, sizeof(ARRAY)); \
        LYA_COUNT_(ARRAY) = orig_count; \
        if ((COUNT) > orig_count) { \
            memset((ARRAY) + orig_count, 0, ((COUNT) - orig_count) * sizeof *(ARRAY)); \
        } \
    }

/**
 * @brief Allocate memory of a [sized array](@ref sizedarrays) or resize an existing array.
 *
 * Updates the size information unlike ::LYA_PREALLOC.
 *
 * Any new memory is zeroed.
 *
 * @param[in,out] ARRAY Sized array to manipulate.
 * @param[in] COUNT New count (size) of the items in the array.
 * @param[in] EACTION Error action on memory allocation error, must include 'return' or 'goto' commands.
 */
#define LYA_NEW(ARRAY, COUNT, EACTION) \
        LYA_PREALLOC(ARRAY, COUNT, EACTION); \
        LYA_COUNT_(ARRAY) = (COUNT)

/**
 * @brief Allocate memory for a new item in a [sized array](@ref sizedarrays).
 *
 * Updates the size information.
 *
 * New item is zeroed.
 *
 * @param[in,out] ARRAY Sized array to manipulate.
 * @param[in] EACTION Error action on memory allocation error, must include 'return' or 'goto' commands.
 */
#define LYA_ADD(ARRAY, EACTION) \
        LYA_PREALLOC(ARRAY, LYA_COUNT(ARRAY) + 1, EACTION); \
        LYA_INCREMENT(ARRAY)

/**
 * @brief Allocate memory for a new item in a [sized array](@ref sizedarrays).
 *
 * Updates the size information.
 *
 * New item is zeroed.
 *
 * @param[in,out] ARRAY Sized array to manipulate.
 * @param[out] NEW_ITEM Pointer to the new item.
 * @param[in] EACTION Error action on memory allocation error, must include 'return' or 'goto' commands.
 */
#define LYA_ADD_ITEM(ARRAY, NEW_ITEM, EACTION) \
        LYA_ADD(ARRAY, EACTION); \
        (NEW_ITEM) = &(ARRAY)[LYA_COUNT_(ARRAY) - 1]

/**
 * @brief Increment the items counter of a [sized array](@ref sizedarrays).
 *
 * Does not change the allocated memory used by the ARRAY. To do so, use ::LYA_PREALLOC.
 *
 * @param[in,out] ARRAY Sized array to manipulate.
 */
#define LYA_INCREMENT(ARRAY) \
        ++LYA_COUNT_(ARRAY)

/**
 * @brief Decrement the items counter of a [sized array](@ref sizedarrays).
 *
 * Does not change the allocated memory used by the ARRAY. To do so, use ::LYA_PREALLOC.
 *
 * @param[in,out] ARRAY Sized array to manipulate.
 */
#define LYA_DECREMENT(ARRAY) \
        --LYA_COUNT_(ARRAY)

/**
 * @brief Decrement the items counter of a [sized array](@ref sizedarrays). Free and zero
 * the whole array in case the count was decremented to 0.
 *
 * @param[in,out] ARRAY Sized array to manipulate.
 */
#define LYA_DECREMENT_FREE(ARRAY) \
        LYA_DECREMENT(ARRAY); \
        if (!LYA_COUNT(ARRAY)) { \
            LYA_FREE(ARRAY); \
            (ARRAY) = NULL; \
        }

/**
 * @brief Free the space allocated for a [sized array](@ref sizedarrays).
 *
 * The items inside the array are not freed.
 *
 * @param[in,out] ARRAY Sized array to manipulate.
 */
#define LYA_FREE(ARRAY) \
        if (ARRAY) { \
            free((LYA_COUNT_T *)(ARRAY) - 1); \
        }

/**
 * @brief Remove a specific value from a [sized array](@ref sizedarrays).
 *
 * @param[in,out] ARRAY Sized array to manipulate.
 * @param[in] VALUE Value to remove, only the first occurence is removed.
 */
#define LYA_REMOVE_VALUE(ARRAY, VALUE) \
    { \
        LYA_COUNT_T u; \
        LYA_FOR(ARRAY, u) { \
            if ((ARRAY)[u] == VALUE) { \
                if (u < LYA_COUNT(ARRAY) - 1) { \
                    memmove(&((ARRAY)[u]), &((ARRAY)[LYA_COUNT(ARRAY) - 1]), sizeof *(ARRAY)); \
                } \
                LYA_DECREMENT(ARRAY); \
                break; \
            } \
        } \
    }

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LY_ARRAY_H_ */
