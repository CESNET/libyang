/**
 * @file utils.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Utilities and generic types of libyang.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_UTILS_H_
#define LY_UTILS_H_

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ly_ctx;
struct timespec;

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
typedef enum {
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
    LY_TYPE_INT64  /**< 64-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
} LY_DATA_TYPE;
#define LY_DATA_TYPE_COUNT 20 /**< Number of different types */

/**
 * @brief Stringfield YANG built-in data types
 */
extern const char *ly_data_type2str[LY_DATA_TYPE_COUNT];

/**
 * @brief All kinds of supported value formats and prefix mappings to modules.
 */
typedef enum {
    LY_VALUE_CANON,           /**< canonical value, prefix mapping is type-specific */
    LY_VALUE_SCHEMA,          /**< YANG schema value, prefixes map to YANG import prefixes */
    LY_VALUE_SCHEMA_RESOLVED, /**< resolved YANG schema value, prefixes map to module structures directly */
    LY_VALUE_XML,             /**< XML data value, prefixes map to XML namespace prefixes */
    LY_VALUE_JSON,            /**< JSON data value, prefixes map to module names */
    LY_VALUE_CBOR,            /**< CBOR data value, prefixes map to module names (same as JSON) */
    LY_VALUE_LYB,             /**< LYB data binary value, prefix mapping is type-specific (but usually like JSON) */
    LY_VALUE_STR_NS           /**< any data format value, prefixes map to XML namespace prefixes */
} LY_VALUE_FORMAT;

/**
 * @brief Check a string matches an XML Schema regex used in YANG.
 *
 * @param[in] ctx Optional context for storing errors.
 * @param[in] pattern Regular expression pattern to use.
 * @param[in] string String to match.
 * @param[in] str_len Length of @p string, may be 0 if string is 0-terminated.
 * @param[in,out] pat_comp Optional pointer to pattern code. If set and NULL, it is returned. If set and non-NULL, it is
 * used directly for matching instead of compiling @p pattern. Free it using ::ly_pattern_free().
 * @return LY_SUCCESS on a match;
 * @return LY_ENOT if the string does not match;
 * @return LY_ERR on error.
 */
LIBYANG_API_DECL LY_ERR ly_pattern_match(const struct ly_ctx *ctx, const char *pattern, const char *string,
        uint32_t str_len, void **pat_comp);

/**
 * @brief Compile an XML Schema regex pattern prior to matching.
 *
 * @param[in] ctx Optional context for storing errors.
 * @param[in] pattern Regular expression pattern to use.
 * @param[out] pat_comp Compiled @p pattern to be used by ::ly_pattern_match(). Free it using ::ly_pattern_free().
 * @return LY_SUCCESS on success;
 * @return LY_ERR on error.
 */
LIBYANG_API_DECL LY_ERR ly_pattern_compile(const struct ly_ctx *ctx, const char *pattern, void **pat_comp);

/**
 * @brief Free a compiled XML Schema regex pattern.
 *
 * @param[in] pat_comp Compiled pattern to free.
 */
LIBYANG_API_DECL void ly_pattern_free(void *pat_comp);

/**
 * @brief Get current timezone (including DST setting) UTC (GMT) time offset in seconds.
 *
 * @return Timezone shift in seconds.
 */
LIBYANG_API_DECL int ly_time_tz_offset(void);

/**
 * @brief Get UTC (GMT) timezone offset in seconds at a specific timestamp (including DST setting).
 *
 * @param[in] time Timestamp to get the offset at.
 * @return Timezone shift in seconds.
 */
LIBYANG_API_DECL int ly_time_tz_offset_at(time_t time);

/**
 * @brief Convert date-and-time from string to UNIX timestamp and fractions of a second.
 *
 * @param[in] value Valid string date-and-time value, the string may continue after the value (be longer).
 * @param[out] time UNIX timestamp.
 * @param[out] fractions_s Optional fractions of a second, set to NULL if none.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR ly_time_str2time(const char *value, time_t *time, char **fractions_s);

/**
 * @brief Convert UNIX timestamp and fractions of a second into canonical date-and-time string value.
 *
 * @param[in] time UNIX timestamp.
 * @param[in] fractions_s Fractions of a second, if any.
 * @param[out] str String date-and-time value in the local timezone.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR ly_time_time2str(time_t time, const char *fractions_s, char **str);

/**
 * @brief Convert date-and-time from string to timespec.
 *
 * @param[in] value Valid string date-and-time value, the string may continue after the value (be longer).
 * @param[out] ts Timespec.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR ly_time_str2ts(const char *value, struct timespec *ts);

/**
 * @brief Convert timespec into date-and-time string value.
 *
 * @param[in] ts Timespec.
 * @param[out] str String date-and-time value in the local timezone.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR ly_time_ts2str(const struct timespec *ts, char **str);

#ifdef __cplusplus
}
#endif

#endif /* LY_UTILS_H_ */
