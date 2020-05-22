/**
 * @file parser_schema.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema parsers for libyang
 *
 * Copyright (c) 2015-2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PARSER_SCHEMA_H_
#define LY_PARSER_SCHEMA_H_

#ifdef __cplusplus
extern "C" {
#endif

struct ly_in;

/**
 * @addtogroup schematree
 * @{
 */

/**
 * @brief Schema input formats accepted by libyang [parser functions](@ref howtoschemasparsers).
 */
typedef enum {
    LYS_IN_UNKNOWN = 0,  /**< unknown format, used as return value in case of error */
    LYS_IN_YANG = 1,     /**< YANG schema input format */
    LYS_IN_YIN = 3       /**< YIN schema input format */
} LYS_INFORMAT;

/**
 * @brief Load a schema into the specified context.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] in The input handle to provide the dumped data model in the specified format.
 * @param[in] format Format of the schema to parse.
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format);

/**
 * @brief Load a schema into the specified context.
 *
 * This function is comsidered for a simple use, if you have a complex usecase,
 * consider use of lys_parse() with a standalone input handler.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified
 * format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format);

/**
 * @brief Read a schema from file descriptor into the specified context.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * This function is comsidered for a simple use, if you have a complex usecase,
 * consider use of lys_parse() with a standalone input handler.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the schema
 *            in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format);

/**
 * @brief Load a schema into the specified context from a file.
 *
 * This function is comsidered for a simple use, if you have a complex usecase,
 * consider use of lys_parse() with a standalone input handler.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] path Path to the file with the model in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format);

/**
 * @brief Search for the schema file in the specified searchpaths.
 *
 * @param[in] searchpaths NULL-terminated array of paths to be searched (recursively). Current working
 * directory is searched automatically (but non-recursively if not in the provided list). Caller can use
 * result of the ly_ctx_get_searchdirs().
 * @param[in] cwd Flag to implicitly search also in the current working directory (non-recursively).
 * @param[in] name Name of the schema to find.
 * @param[in] revision Revision of the schema to find. If NULL, the newest found schema filepath is returned.
 * @param[out] localfile Mandatory output variable containing absolute path of the found schema. If no schema
 * complying the provided restriction is found, NULL is set.
 * @param[out] format Optional output variable containing expected format of the schema document according to the
 * file suffix.
 * @return LY_ERR value (LY_SUCCESS is returned even if the file is not found, then the *localfile is NULL).
 */
LY_ERR lys_search_localfile(const char * const *searchpaths, int cwd, const char *name, const char *revision, char **localfile, LYS_INFORMAT *format);

/** @} schematree */

#ifdef __cplusplus
}
#endif

#endif /* LY_PARSER_SCHEMA_H_ */
