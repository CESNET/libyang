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

/* These 2 macros checks YANG's identifier grammar rule */
#define is_yangidentstartchar(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define is_yangidentchar(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || \
        c == '_' || c == '-' || c == '.')

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
    Y_PREF_IDENTIF_ARG,   /**< YANG "identifier-ref-arg-str" or node-identifier rule */
    Y_STR_ARG,            /**< YANG "string" rule */
    Y_MAYBE_STR_ARG       /**< optional YANG "string" rule */
};

/**
 * @brief internal context for schema parsers
 */
struct ly_parser_ctx {
    struct ly_ctx *ctx;
    struct ly_set tpdfs_nodes;
    struct ly_set grps_nodes;
    uint64_t line;      /**< line number */
    uint64_t indent;    /**< current position on the line for YANG indentation */
    uint8_t mod_version; /**< module's version */
};

/**
 * @brief internal context for compilation
 */
struct lysc_ctx {
    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lys_module *mod_def; /* context module for the definitions of the nodes being currently
                                   processed - groupings are supposed to be evaluated in place where
                                   defined, but its content instances are supposed to be placed into
                                   the target module (mod) */
    struct ly_set groupings;    /* stack for groupings circular check */
    struct ly_set unres;        /* to validate leafref's target and xpath of when/must */
    uint16_t path_len;
#define LYSC_CTX_BUFSIZE 4078
    char path[LYSC_CTX_BUFSIZE];
};

/**
 * @brief Check the currently present prefixes in the module for collision with the new one.
 *
 * @param[in] ctx Context for logging.
 * @param[in] imports List of current imports of the module to check prefix collision.
 * @param[in] module_prefix Prefix of the module to check collision.
 * @param[in] value Newly added prefix value (including its location to distinguish collision with itself).
 * @return LY_EEXIST when prefix is already used in the module, LY_SUCCESS otherwise
 */
LY_ERR lysp_check_prefix(struct ly_parser_ctx *ctx, struct lysp_import *imports, const char *module_prefix, const char **value);

/**
 * @brief Check date string (4DIGIT "-" 2DIGIT "-" 2DIGIT)
 *
 * @param[in] ctx Optional context for logging.
 * @param[in] date Date string to check (non-necessarily terminated by \0)
 * @param[in] date_len Length of the date string, 10 expected.
 * @param[in] stmt Statement name for error message.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_date(struct ly_parser_ctx *ctx, const char *date, int date_len, const char *stmt);

/**
 * @brief Check names of typedefs in the parsed module to detect collisions.
 *
 * @param[in] ctx Parser context for logging and to maintain tpdfs_nodes
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_typedefs(struct ly_parser_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Just move the newest revision into the first position, does not sort the rest
 * @param[in] revs Sized-array of the revisions in a printable schema tree.
 */
void lysp_sort_revisions(struct lysp_revision *revs);

/**
 * @brief Find type specified type definition
 *
 * @param[in] id Name of the type including possible prefix. Module where the prefix is being searched is start_module.
 * @param[in] start_node Context node where the type is being instantiated to be able to search typedefs in parents.
 * @param[in] start_module Module where the type is being instantiated for search for typedefs.
 * @param[out] type Built-in type identifier of the id. If #LY_TYPE_UNKNOWN, tpdf is expected to contain found YANG schema typedef statement.
 * @param[out] tpdf Found type definition.
 * @param[out] node Node where the found typedef is defined, NULL in case of a top-level typedef.
 * @param[out] module Module where the found typedef is being defined, NULL in case of built-in YANG types.
 */
LY_ERR lysp_type_find(const char *id, struct lysp_node *start_node, struct lysp_module *start_module,
                      LY_DATA_TYPE *type, const struct lysp_tpdf **tpdf, struct lysp_node **node, struct lysp_module **module);

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
 * @param[in] ctx parser context
 * @param[in] mod Module including a submodule.
 * @param[in,out] inc Include structure holding all available information about the include statement, the parsed
 * submodule is stored into this structure.
 * @return LY_ERR value.
 */
LY_ERR lysp_load_submodule(struct ly_parser_ctx *ctx, struct lysp_module *mod, struct lysp_include *inc);

/**
 * @brief Get address of a node's actions list if any.
 *
 * Decides the node's type and in case it has an actions list, returns its address.
 * @param[in] node Node to check.
 * @return Address of the node's actions member if any, NULL otherwise.
 */
struct lysp_action **lysp_node_actions_p(struct lysp_node *node);

/**
 * @brief Get address of a node's notifications list if any.
 *
 * Decides the node's type and in case it has a notifications list, returns its address.
 * @param[in] node Node to check.
 * @return Address of the node's notifs member if any, NULL otherwise.
 */
struct lysp_notif **lysp_node_notifs_p(struct lysp_node *node);

/**
 * @brief Get address of a node's child pointer if any.
 *
 * Decides the node's type and in case it has a children list, returns its address.
 * @param[in] node Node to check.
 * @return Address of the node's child member if any, NULL otherwise.
 */
struct lysp_node **lysp_node_children_p(struct lysp_node *node);

/**
 * @brief Get address of a node's child pointer if any.
 *
 * Decides the node's type and in case it has a children list, returns its address.
 * @param[in] node Node to check.
 * @return Address of the node's child member if any, NULL otherwise.
 */
struct lysc_node **lysc_node_children_p(const struct lysc_node *node);

/**
 * @brief Get the covering schema module structure for the given parsed module structure.
 * @param[in] ctx libyang context to search.
 * @param[in] mod Parsed schema structure.
 * @return Corresponding lys_module structure for the given parsed schema structure.
 */
struct lys_module *lysp_find_module(struct ly_ctx *ctx, const struct lysp_module *mod);

/**
 * @brief Find the module referenced by prefix in the provided parsed mod.
 *
 * @param[in] mod Schema module where the prefix was used.
 * @param[in] prefix Prefix used to reference a module.
 * @param[in] len Length of the prefix since it is not necessary NULL-terminated.
 * @return Pointer to the module or NULL if the module is not found.
 */
struct lysp_module *lysp_module_find_prefix(const struct lysp_module *mod, const char *prefix, size_t len);

/**
 * @brief Find the module referenced by prefix in the provided compiled mod.
 *
 * @param[in] mod Schema module where the prefix was used.
 * @param[in] prefix Prefix used to reference a module.
 * @param[in] len Length of the prefix since it is not necessary NULL-terminated.
 * @return Pointer to the module or NULL if the module is not found.
 */
struct lysc_module *lysc_module_find_prefix(const struct lysc_module *mod, const char *prefix, size_t len);

/**
 * @brief Check statement's status for invalid combination.
 *
 * The modX parameters are used just to determine if both flags are in the same module,
 * so any of the schema module structure can be used, but both modules must be provided
 * in the same type.
 *
 * @param[in] ctx Compile context for logging.
 * @param[in] flags1 Flags of the referencing node.
 * @param[in] mod1 Module of the referencing node,
 * @param[in] name1 Schema node name of the referencing node.
 * @param[in] flags2 Flags of the referenced node.
 * @param[in] mod2 Module of the referenced node,
 * @param[in] name2 Schema node name of the referenced node.
 * @return LY_ERR value
 */
LY_ERR lysc_check_status(struct lysc_ctx *ctx,
                         uint16_t flags1, void *mod1, const char *name1,
                         uint16_t flags2, void *mod2, const char *name2);

/**
 * @brief Parse a node-identifier.
 *
 * node-identifier     = [prefix ":"] identifier
 *
 * @param[in, out] id Identifier to parse. When returned, it points to the first character which is not part of the identifier.
 * @param[out] prefix Node's prefix, NULL if there is not any.
 * @param[out] prefix_len Length of the node's prefix, 0 if there is not any.
 * @param[out] name Node's name.
 * @param[out] nam_len Length of the node's name.
 * @return LY_ERR value: LY_SUCCESS or LY_EINVAL in case of invalid character in the id.
 */
LY_ERR lys_parse_nodeid(const char **id, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len);

/**
 * @brief Find the node according to the given descendant schema node id.
 * Used in unique, refine and uses's augment statements
 *
 * @param[in] ctx Compile context
 * @param[in] nodeid Descendant-schema-nodeid (according to the YANG grammar)
 * @param[in] nodeid_len Length of the given nodeid, if it is not NULL-terminated string.
 * @param[in] context_node Node where the nodeid is specified to correctly resolve prefixes and to start searching.
 * @param[in] nodetype Optional (can be 0) restriction for target's nodetype. If target exists, but does not match
 * the given nodetype, LY_EDENIED is returned, but no error message is printed. The value can be even an ORed value to allow
 * multiple nodetypes.
 * @param[out] target Found target node if any.
 * @return LY_ERR values - LY_ENOTFOUND, LY_EVALID, LY_EDENIED or LY_SUCCESS.
 */
LY_ERR lys_resolve_descendant_schema_nodeid(struct lysc_ctx *ctx, const char *nodeid, size_t nodeid_len, const struct lysc_node *context_node,
                                            int nodetype, const struct lysc_node **target);

/**
 * @brief Find the module referenced by prefix in the provided mod.
 *
 * @param[in] mod Schema module where the prefix was used.
 * @param[in] prefix Prefix used to reference a module.
 * @param[in] len Length of the prefix since it is not necessary NULL-terminated.
 * @return Pointer to the module or NULL if the module is not found.
 */
struct lys_module *lys_module_find_prefix(const struct lys_module *mod, const char *prefix, size_t len);

/**
 * @brief Stringify schema nodetype.
 * @param[in] nodetype Nodetype to stringify.
 * @return Constant string with the name of the node's type.
 */
const char *lys_nodetype2str(uint16_t nodetype);

/**
 * @brief Parse YANG module from a string.
 *
 * The modules are added into the context and the latest_revision flag is updated.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified
 * format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] implement Flag if the schema is supposed to be marked as implemented.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_mem_module(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, int implement,
                                        LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *check_data),
                                        void *check_data);

/**
 * @brief Parse YANG submodule from a string.
 *
 * The latest_revision flag of submodule is updated.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified
 * format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] main_ctx Parser context of the main module.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @return Pointer to the data model structure or NULL on error.
 */
struct lysp_submodule *lys_parse_mem_submodule(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, struct ly_parser_ctx *main_ctx,
                                               LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *check_data),
                                               void *check_data);

/**
 * @brief Parse YANG module or submodule from a file descriptor.
 *
 * The modules are added into the context, submodules not. The latest_revision flag is updated in both cases.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the schema
 *            in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] implement Flag if the schema is supposed to be marked as implemented.
 * @param[in] main_ctx Parser context of the main module in case of parsing submodule. This flag decides if the module
 * or submodule was expected to be parsed.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @return Pointer to the data model structure or NULL on error.
 */
void *lys_parse_fd_(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, int implement, struct ly_parser_ctx *main_ctx,
                    LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data),
                    void *check_data);

/**
 * @brief Parse YANG module from a file descriptor.
 *
 * The modules are added into the context. The latest_revision flag is updated.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the schema
 *            in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] implement Flag if the schema is supposed to be marked as implemented.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_fd_module(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, int implement,
                                           LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *check_data),
                                           void *check_data);

/**
 * @brief Parse YANG submodule from a file descriptor.
 *
 * The latest_revision flag of submodules is updated.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the schema
 *            in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] main_ctx Parser context of the main module.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @return Pointer to the data model structure or NULL on error.
 */
struct lysp_submodule *lys_parse_fd_submodule(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, struct ly_parser_ctx *main_ctx,
                                              LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *check_data),
                                              void *check_data);

/**
 * @brief Parse YANG module from a filepath.
 *
 * The modules are added into the context. The latest_revision flag is updated.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] path Path to the file with the model in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] implement Flag if the schema is supposed to be marked as implemented.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_path_(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format, int implement,
                                   LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data),
                                   void *check_data);

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
 * @param[in] main_ctx Parser context of the main module in case of loading submodule.
 * @param[out] result Parsed YANG schema tree of the requested module (struct lys_module*) or submodule (struct lysp_submodule*).
 * If it is a module, it is already in the context!
 * @return LY_ERR value, in case of LY_SUCCESS, the \arg result is always provided.
 */
LY_ERR lys_module_localfile(struct ly_ctx *ctx, const char *name, const char *revision, int implement, struct ly_parser_ctx *main_ctx,
                            void **result);

/**
 * @brief Free the parsed submodule structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] submod Parsed schema submodule structure to free.
 */
void lysp_submodule_free(struct ly_ctx *ctx, struct lysp_submodule *submod);

/**
 * @brief Free the compiled type structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] type Compiled type structure to be freed. The structure has refcount, so it is freed only in case the value is decreased to 0.
 */
void lysc_type_free(struct ly_ctx *ctx, struct lysc_type *type);

/**
 * @brief Free the compiled node structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] node Compiled node structure to be freed.
 */
void lysc_node_free(struct ly_ctx *ctx, struct lysc_node *node);

/**
 * @brief Free the compiled schema structure.
 * @param[in,out] module Compiled schema module structure to free.
 * @param[in] private_destructor Function to remove private data from the compiled schema tree.
 */
void lysc_module_free(struct lysc_module *module, void (*private_destructor)(const struct lysc_node *node, void *priv));

/**
 * @brief Free the schema structure. It just frees, it does not remove the schema from its context.
 * @param[in,out] module Schema module structure to free.
 * @param[in] private_destructor Function to remove private data from the compiled schema tree.
 */
void lys_module_free(struct lys_module *module, void (*private_destructor)(const struct lysc_node *node, void *priv));

/**
 * @brief Parse submodule from YANG data.
 * @param[in] ctx Parser context.
 * @param[in] data Input data to be parsed.
 * @param[out] submod Pointer to the parsed submodule structure.
 * @return LY_ERR value - LY_SUCCESS, LY_EINVAL or LY_EVALID.
 */
LY_ERR yang_parse_submodule(struct ly_parser_ctx *ctx, const char *data, struct lysp_submodule **submod);

/**
 * @brief Parse module from YANG data.
 * @param[in] ctx Parser context.
 * @param[in] data Input data to be parsed.
 * @param[in, out] mod Prepared module structure where the parsed information, including the parsed
 * module structure, will be filled in.
 * @return LY_ERR value - LY_SUCCESS, LY_EINVAL or LY_EVALID.
 */
LY_ERR yang_parse_module(struct ly_parser_ctx *ctx, const char *data, struct lys_module *mod);

#endif /* LY_TREE_SCHEMA_INTERNAL_H_ */
