/**
 * @file tree_edit.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang generic macros and functions to modify YANG schema or data trees. Intended for internal use and libyang
 * plugins.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_EDIT_H_
#define LY_TREE_EDIT_H_

#include <stdlib.h>

#include "log.h"
#include "tree.h"

#ifndef LOGMEM
#define LOGMEM(CTX)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wrapper for realloc() call. The only difference is that if it fails to
 * allocate the requested memory, the original memory is freed as well.
 *
 * @param[in] ptr Memory to reallocate.
 * @param[in] size New size of the memory block.
 *
 * @return Pointer to the new memory, NULL on error.
 */
void *ly_realloc(void *ptr, size_t size);

/**
 * @defgroup trees_edit Trees - modification
 * @ingroup trees
 *
 * Generic macros, functions, etc. to modify [schema](@ref schematree) and [data](@ref datatree) trees.
 * @{
 */

/** @} trees_edit */

/**
 * @brief (Re-)Allocation of a ([sized array](@ref sizedarrays)).
 *
 * Increases the size information.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to allocate/resize. The size of the allocated
 * space is counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[out] NEW_ITEM Returning pointer to the newly allocated record in the ARRAY.
 * @param[in] RETVAL Return value for the case of error (memory allocation failure).
 */
#define LY_ARRAY_NEW_RET(CTX, ARRAY, NEW_ITEM, RETVAL) \
        if (!(ARRAY)) { \
            ARRAY = malloc(sizeof(LY_ARRAY_COUNT_TYPE) + sizeof *(ARRAY)); \
            *((LY_ARRAY_COUNT_TYPE*)(ARRAY)) = 1; \
        } else { \
            ++(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)); \
            ARRAY = ly_realloc(((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1), sizeof(LY_ARRAY_COUNT_TYPE) + (*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1) * sizeof *(ARRAY))); \
            if (!(ARRAY)) { \
                LOGMEM(CTX); \
                return RETVAL; \
            } \
        } \
        ARRAY = (void*)((LY_ARRAY_COUNT_TYPE*)(ARRAY) + 1); \
        (NEW_ITEM) = &(ARRAY)[*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1) - 1]; \
        memset(NEW_ITEM, 0, sizeof *(NEW_ITEM))

/**
 * @brief (Re-)Allocation of a ([sized array](@ref sizedarrays)).
 *
 * Increases the size information.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to allocate/resize. The size of the allocated
 * space is counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[out] NEW_ITEM Returning pointer to the newly allocated record in the ARRAY.
 * @param[out] RET Variable to store error code.
 * @param[in] GOTO Label to go in case of error (memory allocation failure).
 */
#define LY_ARRAY_NEW_GOTO(CTX, ARRAY, NEW_ITEM, RET, GOTO) \
        if (!(ARRAY)) { \
            ARRAY = malloc(sizeof(LY_ARRAY_COUNT_TYPE) + sizeof *(ARRAY)); \
            *((LY_ARRAY_COUNT_TYPE*)(ARRAY)) = 1; \
        } else { \
            ++(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)); \
            ARRAY = ly_realloc(((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1), sizeof(LY_ARRAY_COUNT_TYPE) + (*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1) * sizeof *(ARRAY))); \
            if (!(ARRAY)) { \
                RET = LY_EMEM; \
                LOGMEM(CTX); \
                goto GOTO; \
            } \
        } \
        ARRAY = (void*)((LY_ARRAY_COUNT_TYPE*)(ARRAY) + 1); \
        (NEW_ITEM) = &(ARRAY)[*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1) - 1]; \
        memset(NEW_ITEM, 0, sizeof *(NEW_ITEM))

/**
 * @brief Allocate a ([sized array](@ref sizedarrays)) for the specified number of items.
 * If the ARRAY already exists, it is resized (space for SIZE items is added and zeroed).
 *
 * Does not set the size information, it is supposed to be incremented via ::LY_ARRAY_INCREMENT
 * when the items are filled.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to create.
 * @param[in] SIZE Number of the new items the array is supposed to hold. The size of the allocated
 * space is then counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[in] RETVAL Return value for the case of error (memory allocation failure).
 */
#define LY_ARRAY_CREATE_RET(CTX, ARRAY, SIZE, RETVAL) \
        if (ARRAY) { \
            ARRAY = ly_realloc(((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1), sizeof(LY_ARRAY_COUNT_TYPE) + ((*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1) + (SIZE)) * sizeof *(ARRAY))); \
            if (!(ARRAY)) { \
                LOGMEM(CTX); \
                return RETVAL; \
            } \
            ARRAY = (void*)((LY_ARRAY_COUNT_TYPE*)(ARRAY) + 1); \
            memset(&(ARRAY)[*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)], 0, (SIZE) * sizeof *(ARRAY)); \
        } else { \
            ARRAY = calloc(1, sizeof(LY_ARRAY_COUNT_TYPE) + (SIZE) * sizeof *(ARRAY)); \
            if (!(ARRAY)) { \
                LOGMEM(CTX); \
                return RETVAL; \
            } \
            ARRAY = (void*)((LY_ARRAY_COUNT_TYPE*)(ARRAY) + 1); \
        }

/**
 * @brief Allocate a ([sized array](@ref sizedarrays)) for the specified number of items.
 * If the ARRAY already exists, it is resized (space for SIZE items is added).
 *
 * Does not set the count information, it is supposed to be incremented via ::LY_ARRAY_INCREMENT
 * when the items are filled.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to create.
 * @param[in] SIZE Number of the new items the array is supposed to hold. The size of the allocated
 * space is then counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[out] RET Variable to store error code.
 * @param[in] GOTO Label to go in case of error (memory allocation failure).
 */
#define LY_ARRAY_CREATE_GOTO(CTX, ARRAY, SIZE, RET, GOTO) \
        if (ARRAY) { \
            ARRAY = ly_realloc(((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1), sizeof(LY_ARRAY_COUNT_TYPE) + ((LY_ARRAY_COUNT(ARRAY) + (SIZE)) * sizeof *(ARRAY))); \
            if (!(ARRAY)) { \
                RET = LY_EMEM; \
                LOGMEM(CTX); \
                goto GOTO; \
            } \
            ARRAY = (void*)((LY_ARRAY_COUNT_TYPE*)(ARRAY) + 1); \
            memset(&(ARRAY)[*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)], 0, (SIZE) * sizeof *(ARRAY)); \
        } else { \
            ARRAY = calloc(1, sizeof(LY_ARRAY_COUNT_TYPE) + (SIZE) * sizeof *(ARRAY)); \
            if (!(ARRAY)) { \
                RET = LY_EMEM; \
                LOGMEM(CTX); \
                goto GOTO; \
            } \
            ARRAY = (void*)((LY_ARRAY_COUNT_TYPE*)(ARRAY) + 1); \
        }

/**
 * @brief Increment the items counter in a ([sized array](@ref sizedarrays)).
 *
 * Does not change the allocated memory used by the ARRAY. To do so, use LY_ARRAY_CREATE_RET,
 * LY_ARRAY_CREATE_GOTO or LY_ARRAY_RESIZE_ERR_RET.
 *
 * @param[in] ARRAY Pointer to the array to affect.
 */
#define LY_ARRAY_INCREMENT(ARRAY) \
        ++(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1))

/**
 * @brief Decrement the items counter in a ([sized array](@ref sizedarrays)).
 *
 * Does not change the allocated memory used by the ARRAY. To do so, use LY_ARRAY_CREATE_RET,
 * LY_ARRAY_CREATE_GOTO or LY_ARRAY_RESIZE_ERR_RET.
 *
 * @param[in] ARRAY Pointer to the array to affect.
 */
#define LY_ARRAY_DECREMENT(ARRAY) \
        --(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1))

/**
 * @brief Decrement the items counter in a ([sized array](@ref sizedarrays)) and free the whole array
 * in case it was decremented to 0.
 *
 * @param[in] ARRAY Pointer to the array to affect.
 */
#define LY_ARRAY_DECREMENT_FREE(ARRAY) \
        --(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)); \
        if (!LY_ARRAY_COUNT(ARRAY)) { \
            LY_ARRAY_FREE(ARRAY); \
            (ARRAY) = NULL; \
        }

/**
 * @brief Free the space allocated for the ([sized array](@ref sizedarrays)).
 *
 * The items inside the array are not freed.
 *
 * @param[in] ARRAY A ([sized array](@ref sizedarrays)) to be freed.
 */
#define LY_ARRAY_FREE(ARRAY) \
        if (ARRAY){free((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1);}

/**
 * @brief Insert item into linked list.
 *
 * @param[in,out] LIST Linked list to add to.
 * @param[in] NEW_ITEM New item, that will be appended to the list, must be already allocated.
 * @param[in] LINKER name of structuin member that is used to connect items together.
 */
#define LY_LIST_INSERT(LIST, NEW_ITEM, LINKER)\
    if (!(*LIST)) { \
        *LIST = (__typeof__(*(LIST)))NEW_ITEM; \
    } else { \
        do { \
            __typeof__(*(LIST)) iterator; \
            for (iterator = *(LIST); iterator->LINKER; iterator = iterator->LINKER) {} \
            iterator->LINKER = (__typeof__(*(LIST)))NEW_ITEM; \
        } while (0); \
    }

/**
 * @brief Allocate and insert new item into linked list, return in case of error.
 *
 * @param[in] CTX used for logging.
 * @param[in,out] LIST Linked list to add to.
 * @param[out] NEW_ITEM New item that is appended to the list.
 * @param[in] LINKER name of structure member that is used to connect items together.
 * @param[in] RETVAL Return value for the case of error (memory allocation failure).
 */
#define LY_LIST_NEW_RET(CTX, LIST, NEW_ITEM, LINKER, RETVAL) \
    NEW_ITEM = calloc(1, sizeof *NEW_ITEM); \
    if (!(NEW_ITEM)) { \
        LOGMEM(CTX); \
        return RETVAL; \
    } \
    LY_LIST_INSERT(LIST, NEW_ITEM, LINKER)

/**
 * @brief Allocate and insert new item into linked list, goto specified label in case of error.
 *
 * @param[in] CTX used for logging.
 * @param[in,out] LIST Linked list to add to.
 * @param[out] NEW_ITEM New item that is appended to the list.
 * @param[in] LINKER name of structure member that is used to connect items together.
 * @param[in] RET variable to store returned error type.
 * @param[in] LABEL label to goto in case of error.
 */
#define LY_LIST_NEW_GOTO(CTX, LIST, NEW_ITEM, LINKER, RET, LABEL) \
    NEW_ITEM = calloc(1, sizeof *NEW_ITEM); \
    if (!(NEW_ITEM)) { \
        RET = LY_EMEM; \
        LOGMEM(CTX); \
        goto LABEL; \
    } \
    LY_LIST_INSERT(LIST, NEW_ITEM, LINKER)

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_EDIT_H_ */