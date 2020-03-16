/**
 * @file set.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic set structure and manipulation routines.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_SET_H_
#define LY_SET_H_

#include <stdint.h>

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup lyset Generic sets
 *
 * Structure and functions to hold and manipulate with sets of nodes from schema or data trees.
 *
 * @{
 */

/**
 * @brief Structure to hold a set of (not necessary somehow connected) objects. Usually used for lyd_node,
 * ::lysp_node or ::lysc_node objects, but it is not limited to them. Caller is supposed to not mix the type of objects
 * added to the set and according to its knowledge about the set content, it can access objects via the members
 * of the set union.
 *
 * Until ly_set_rm() or ly_set_rm_index() is used, the set keeps the order of the inserted items as they
 * were added into the set, so the first added item is on array index 0.
 *
 * To free the structure, use ly_set_free() function, to manipulate with the structure, use other
 * ly_set_* functions.
 */
struct ly_set
{
    uint32_t size;                    /**< allocated size of the set array */
    uint32_t count;                   /**< number of elements in (used size of) the set array */
    void **objs;                      /**< set array of generic object pointers */
};

/**
 * @brief Option for ly_set_add() to allow duplicities in the ly_set structure so the
 * set is not used as a set, but as a list of (container for) items.
 */
#define LY_SET_OPT_USEASLIST 0x01

/**
 * @brief Create and initiate new ::ly_set structure.
 *
 * @return Created ::ly_set structure or NULL in case of error.
 */
struct ly_set *ly_set_new(void);

/**
 * @brief Duplicate the existing set.
 *
 * @param[in] set Original set to duplicate
 * @param[in] duplicator Optional pointer to function that duplicates the objects stored
 * in the original set. If not provided, the new set points to the exact same objects as
 * the original set.
 * @return Duplication of the original set.
 */
struct ly_set *ly_set_dup(const struct ly_set *set, void *(*duplicator)(void *obj));

/**
 * @brief Add an object into the set
 *
 * Since it is a set, the function checks for duplicity and if the
 * node is already in the set, the index of the previously added
 * node is returned.
 *
 * @param[in] set Set where the \p object will be added.
 * @param[in] object Object to be added into the \p set;
 * @param[in] options Options to change behavior of the function. Accepted options are:
 * - #LY_SET_OPT_USEASLIST - do not check for duplicities
 * @return -1 on failure, index of the \p node in the set on success
 */
int ly_set_add(struct ly_set *set, void *object, int options);

/**
 * @brief Add all objects from \p src to \p trg.
 *
 * Since it is a set, the function checks for duplicities.
 *
 * @param[in] trg Target (result) set.
 * @param[in] src Source set.
 * @param[in] options Options to change behavior of the function. Accepted options are:
 * - #LY_SET_OPT_USEASLIST - add without checking for duplicities
 * @param[in] duplicator Optional pointer to function that duplicates the objects being added
 * from \p src into \p trg set. If not provided, the \p trg set will point to the exact same
 * objects as the \p src set.
 * @return -1 on failure, number of objects added into \p trg on success.
 */
int ly_set_merge(struct ly_set *trg, struct ly_set *src, int options, void *(*duplicator)(void *obj));

/**
 * @brief Learn whether the set contains the specified object.
 *
 * @param[in] set Set to explore.
 * @param[in] object Object to be found in the set.
 * @return Index of the object in the set or -1 if the object is not present in the set.
 */
int ly_set_contains(const struct ly_set *set, void *object);

/**
 * @brief Remove all objects from the set, but keep the set container for further use.
 *
 * @param[in] set Set to clean.
 * @param[in] destructor Optional function to free the objects in the set.
 */
void ly_set_clean(struct ly_set *set, void (*destructor)(void *obj));

/**
 * @brief Remove an object from the set.
 *
 * Note that after removing the object from a set, indexes of other objects in the set can change
 * (the last object is placed instead of the removed object).
 *
 * @param[in] set Set from which the \p node will be removed.
 * @param[in] object The object to be removed from the \p set.
 * @param[in] destructor Optional function to free the objects being removed.
 * @return LY_ERR return value.
 */
LY_ERR ly_set_rm(struct ly_set *set, void *object, void (*destructor)(void *obj));

/**
 * @brief Remove an object on the specific set index.
 *
 * Note that after removing the object from a set, indexes of other nodes in the set can change
 * (the last object is placed instead of the removed object).
 *
 * @param[in] set Set from which a node will be removed.
 * @param[in] index Index of the object to remove in the \p set.
 * @param[in] destructor Optional function to free the objects being removed.
 * @return LY_ERR return value.
 */
LY_ERR ly_set_rm_index(struct ly_set *set, uint32_t index, void (*destructor)(void *obj));

/**
 * @brief Free the ::ly_set data. If the destructor is not provided, it frees only the set structure
 * content, not the referred data.
 *
 * @param[in] set The set to be freed.
 * @param[in] destructor Optional function to free the objects in the set.
 */
void ly_set_free(struct ly_set *set, void (*destructor)(void *obj));

/**
 * @brief Alternative to the ly_set_free() for static ::ly_set objects - in contrast to ly_set_free()
 * it does not free the provided ::ly_set object.
 *
 * @param[in] set The set to be erased.
 * @param[in] destructor Optional function to free the objects in the set.
 */
void ly_set_erase(struct ly_set *set, void (*destructor)(void *obj));

/** @} lyset */

#ifdef __cplusplus
}
#endif

#endif /* LY_SET_H_ */
