/**
 * @file tree_schema_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal functions for YANG schema trees.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_SCHEMA_INTERNAL_H_
#define LY_TREE_SCHEMA_INTERNAL_H_

#define LOGVAL_YANG(CTX, ...) LOGVAL((CTX)->ctx, LY_VLOG_LINE, &(CTX)->line, __VA_ARGS__)

/**
 * @brief List of YANG statement groups - the (sub)module's substatements
 */
enum yang_module_stmt {
    Y_MOD_MODULE_HEADER,
    Y_MOD_LINKAGE,
    Y_MOD_META,
    Y_MOD_REVISION,
    Y_MOD_BODY
};

/**
 * @brief Types of arguments of YANG statements
 */
enum yang_arg {
    Y_IDENTIF_ARG,        /**< YANG "identifier-arg-str" rule */
    Y_PREF_IDENTIF_ARG,   /**< YANG "identifier-ref-arg-str" rule */
    Y_STR_ARG,            /**< YANG "string" rule */
    Y_MAYBE_STR_ARG       /**< optional YANG "string" rule */
};

/**
 * @brief internal context for schema parsers
 */
struct ly_parser_ctx {
    struct ly_ctx *ctx;
    struct lysp_module *mod;
    uint64_t line;      /* line number */
    uint64_t indent;    /* current position on the line for YANG indentation */
};

/**
 * @brief Check the currently present prefixes in the module for collision with the new one.
 *
 * @param[in] ctx yang parser context.
 * @param[in] module Schema tree to check.
 * @param[in] value Newly added prefix value (including its location to distinguish collision with itself).
 * @return LY_EEXIST when prefix is already used in the module, LY_SUCCESS otherwise
 */
LY_ERR lysp_check_prefix(struct ly_parser_ctx *ctx, struct lysp_module *module, const char **value);

/**
 * @brief Check date string (4DIGIT "-" 2DIGIT "-" 2DIGIT)
 *
 * @param[in] ctx Context to store log message.
 * @param[in] date Date string to check (non-necessarily terminated by \0)
 * @param[in] date_len Length of the date string, 10 expected.
 * @param[in] stmt Statement name for error message.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_date(struct ly_ctx *ctx, const char *date, int date_len, const char *stmt);

/**
 * @brief Just move the newest revision into the first position, does not sort the rest
 * @param[in] revs Sized-array of the revisions in a printable schema tree.
 */
void lysp_sort_revisions(struct lysp_revision *revs);

/**
 * @brief Find and parse module of the given name.
 *
 * @param[in] ctx libyang context.
 * @param[in] name Name of the module to load.
 * @param[in] revison Optional revision of the module to load. If NULL, the newest revision is loaded.
 * @param[in] implement Flag if the loaded module is supposed to be marked as implemented.
 * @param[in] require_parsed Flag to require parsed module structure in case the module is already in the context,
 * but only the compiled structure is available.
 * @param[out] mod Parsed module structure.
 * @return LY_ERR value.
 */
LY_ERR lysp_load_module(struct ly_ctx *ctx, const char *name, const char *revision, int implement, int require_parsed, struct lys_module **mod);

/**
 * @brief Parse included submodule into the simply parsed YANG module.
 *
 * @param[in] ctx libyang context
 * @param[in] mod Module including a submodule.
 * @param[in,out] inc Include structure holding all available information about the include statement, the parsed
 * submodule is stored into this structure.
 * @return LY_ERR value.
 */
LY_ERR lysp_load_submodule(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_include *inc);

/**
 * @brief Find the module referenced by prefix in the provided parsed mod.
 *
 * @param[in] mod Schema module where the prefix was used.
 * @param[in] prefix Prefix used to reference a module.
 * @param[in] len Length of the prefix since it is not necessary NULL-terminated.
 * @return Pointer to the module or NULL if the module is not found.
 */
struct lys_module *lysp_module_find_prefix(struct lysp_module *mod, const char *prefix, size_t len);

/**
 * @brief Find the module referenced by prefix in the provided compiled mod.
 *
 * @param[in] mod Schema module where the prefix was used.
 * @param[in] prefix Prefix used to reference a module.
 * @param[in] len Length of the prefix since it is not necessary NULL-terminated.
 * @return Pointer to the module or NULL if the module is not found.
 */
struct lys_module *lysc_module_find_prefix(struct lysc_module *mod, const char *prefix, size_t len);

/**
 * @brief Find the module referenced by prefix in the provided mod.
 *
 * @param[in] mod Schema module where the prefix was used.
 * @param[in] prefix Prefix used to reference a module.
 * @param[in] len Length of the prefix since it is not necessary NULL-terminated.
 * @return Pointer to the module or NULL if the module is not found.
 */
struct lys_module *lys_module_find_prefix(struct lys_module *mod, const char *prefix, size_t len);

/**
 * @brief Parse YANG module and submodule from a string.
 *
 * In contrast to public lys_parse_mem(), also submodules can be parsed here. However,
 * while the modules are added into the context, submodules not. The latest_revision
 * flag is updated in both cases.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified
 * format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] revision If a specific revision is required, it is checked before the parsed
 * schema is accepted.
 * @param[in] implement Flag if the schema is supposed to be marked as implemented.
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_mem_(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, const char *revision, int implement);

/**
 * @brief Parse YANG module and submodule from a file descriptor.
 *
 * In contrast to public lys_parse_mem(), also submodules can be parsed here. However,
 * while the modules are added into the context, submodules not. The latest_revision
 * flag is updated in both cases.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the schema
 *            in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] revision If a specific revision is required, it is checked before the parsed
 * schema is accepted.
 * @param[in] implement Flag if the schema is supposed to be marked as implemented.
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_fd_(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, const char *revision, int implement);

/**
 * @brief Parse YANG module and submodule from a file descriptor.
 *
 * In contrast to public lys_parse_mem(), also submodules can be parsed here. However,
 * while the modules are added into the context, submodules not. The latest_revision
 * flag is updated in both cases.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @brief REad a schema into the specified context from a file.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] path Path to the file with the model in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] revision If a specific revision is required, it is checked before the parsed
 * schema is accepted.
 * @param[in] implement Flag if the schema is supposed to be marked as implemented.
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_path_(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format, const char *revision, int implement);

/**
 * @brief Load the (sub)module into the context.
 *
 * This function does not check the presence of the (sub)module in context, it should be done before calling this function.
 *
 * module_name and submodule_name are alternatives - only one of the
 *
 * @param[in] ctx libyang context where to work.
 * @param[in] name Name of the (sub)module to load.
 * @param[in] revision Optional revision of the (sub)module to load, if NULL the newest revision is being loaded.
 * @param[in] implement Flag if the (sub)module is supposed to be marked as implemented.
 * @param[out] result Parsed YANG schema tree of the requested module. If it is a module, it is already in the context!
 * @return LY_ERR value, in case of LY_SUCCESS, the \arg result is always provided.
 */
LY_ERR lys_module_localfile(struct ly_ctx *ctx, const char *name, const char *revision, int implement, struct lys_module **result);

/**
 * @brief Make the module implemented.
 * Does not check for collision in context, it must be done before calling the function, this is a simple switch.
 * @param[in] mod Module to make implemented.
 */
void lys_module_implement(struct lys_module *mod);

/**
 * @brief Free the schema structure. It just frees, it does not remove the schema from its context.
 * @param[in,out] module Schema module structure to free.
 * @param[in] private_destructor Function to remove private data from the compiled schema tree.
 */
void lys_module_free(struct lys_module *module, void (*private_destructor)(const struct lysc_node *node, void *priv));

/**
 * @brief
 */
LY_ERR yang_parse(struct ly_ctx *ctx, const char *data, struct lysp_module **mod_p);

#endif /* LY_TREE_SCHEMA_INTERNAL_H_ */
