/**
 * @file dict.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang dictionary
 *
 * Copyright (c) 2015-2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_DICT_H_
#define LY_DICT_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* dummy context structure */
struct ly_ctx;

/**
 * @defgroup dict Dictionary
 * @{
 *
 * Publicly visible functions and values of the libyang dictionary. They provide
 * access to the strings stored in the libyang context.
 */

/**
 * @brief Insert string into dictionary. If the string is already present,
 * only a reference counter is incremented and no memory allocation is
 * performed.
 *
 * @param[in] ctx libyang context handler
 * @param[in] value String to be stored in the dictionary. If NULL, function does nothing.
 * @param[in] len Number of bytes to store. The value is not required to be
 * NULL terminated string, the len parameter says number of bytes stored in
 * dictionary. The specified number of bytes is duplicated and terminating NULL
 * byte is added automatically. If \p len is 0, it is count automatically using strlen().
 * @return pointer to the string stored in the dictionary, NULL if \p value was NULL.
 */
const char *lydict_insert(const struct ly_ctx *ctx, const char *value, size_t len);

/**
 * @brief Insert string into dictionary - zerocopy version. If the string is
 * already present, only a reference counter is incremented and no memory
 * allocation is performed. This insert function variant avoids duplication of
 * specified value - it is inserted into the dictionary directly.
 *
 * @param[in] ctx libyang context handler
 * @param[in] value NULL-terminated string to be stored in the dictionary. If
 * the string is not present in dictionary, the pointer is directly used by the
 * dictionary. Otherwise, the reference counter is incremented and the value is
 * freed. So, after calling the function, caller is supposed to not use the
 * value address anymore. If NULL, function does nothing.
 * @return pointer to the string stored in the dictionary, NULL if \p value was NULL.
 */
const char *lydict_insert_zc(const struct ly_ctx *ctx, char *value);

/**
 * @brief Remove specified string from the dictionary. It decrement reference
 * counter for the string and if it is zero, the string itself is freed.
 *
 * @param[in] ctx libyang context handler
 * @param[in] value String to be freed. Note, that not only the string itself
 * must match the stored value, but also the address is being compared and the
 * counter is decremented only if it matches. If NULL, function does nothing.
 */
void lydict_remove(const struct ly_ctx *ctx, const char *value);

/** @} dict */

#ifdef __cplusplus
}
#endif

#endif /* LY_DICT_H_ */
