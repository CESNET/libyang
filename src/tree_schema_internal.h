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

#include <stdint.h>

#include "common.h"
#include "set.h"
#include "tree_schema.h"
#include "xml.h"

#define YIN_NS_URI "urn:ietf:params:xml:ns:yang:yin:1"

/**
 * @brief Check module version is at least 2 (YANG 1.1) because of the keyword presence.
 * Logs error message and returns LY_EVALID in case of module in YANG version 1.0.
 * @param[in] CTX yang parser context to get current module and for logging.
 * @param[in] KW keyword allowed only in YANG version 1.1 (or later) - for logging.
 * @param[in] PARENT parent statement where the KW is present - for logging.
 */
#define PARSER_CHECK_STMTVER2_RET(CTX, KW, PARENT) \
    if ((CTX)->mod_version < 2) {LOGVAL_PARSER((CTX), LY_VCODE_INCHILDSTMT2, KW, PARENT); return LY_EVALID;}

/* These 2 macros checks YANG's identifier grammar rule */
#define is_yangidentstartchar(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define is_yangidentchar(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || \
                              c == '_' || c == '-' || c == '.')

/* Macro to check YANG's yang-char grammar rule */
#define is_yangutf8char(c) ((c >= 0x20 && c <= 0xd7ff) || c == 0x09 || c == 0x0a || c == 0x0d || \
                            (c >= 0xe000 && c <= 0xfdcf)   || (c >= 0xfdf0 && c <= 0xfffd)   || \
                            (c >= 0x10000 && c <= 0x1fffd) || (c >= 0x20000 && c <= 0x2fffd) || \
                            (c >= 0x30000 && c <= 0x3fffd) || (c >= 0x40000 && c <= 0x2fffd) || \
                            (c >= 0x50000 && c <= 0x5fffd) || (c >= 0x60000 && c <= 0x6fffd) || \
                            (c >= 0x70000 && c <= 0x7fffd) || (c >= 0x80000 && c <= 0x8fffd) || \
                            (c >= 0x90000 && c <= 0x9fffd) || (c >= 0xa0000 && c <= 0xafffd) || \
                            (c >= 0xb0000 && c <= 0xbfffd) || (c >= 0xc0000 && c <= 0xcfffd) || \
                            (c >= 0xd0000 && c <= 0xdfffd) || (c >= 0xe0000 && c <= 0xefffd) || \
                            (c >= 0xf0000 && c <= 0xffffd) || (c >= 0x100000 && c <= 0x10fffd))

/**
 * @brief Try to find object with MEMBER string matching the IDENT in the given ARRAY.
 * Macro logs an error message and returns LY_EVALID in case of existence of a matching object.
 *
 * @param[in] CTX yang parser context for logging.
 * @param[in] ARRAY [sized array](@ref sizedarrays) of a generic objects with member named MEMBER to search.
 * @param[in] MEMBER Name of the member of the objects in the ARRAY to compare.
 * @param[in] STMT Name of the compared YANG statements for logging.
 * @param[in] IDENT String trying to find in the ARRAY's objects inside the MEMBER member.
 */
#define CHECK_UNIQUENESS(CTX, ARRAY, MEMBER, STMT, IDENT) \
    if (ARRAY) { \
        for (LY_ARRAY_COUNT_TYPE u_ = 0; u_ < LY_ARRAY_COUNT(ARRAY) - 1; ++u_) { \
            if (!strcmp((ARRAY)[u_].MEMBER, IDENT)) { \
                LOGVAL_PARSER(CTX, LY_VCODE_DUPIDENT, IDENT, STMT); \
                return LY_EVALID; \
            } \
        } \
    }

#define CHECK_NONEMPTY(CTX, VALUE_LEN, STMT) \
    if (!VALUE_LEN) { \
        LOGWRN(PARSER_CTX(CTX), "Empty argument of %s statement does not make sense.", STMT); \
    }

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

#define PARSER_CTX(CTX) (CTX)->format == LYS_IN_YANG ? ((struct lys_yang_parser_ctx *)CTX)->ctx : ((struct ly_ctx *)((struct lys_yin_parser_ctx *)CTX)->xmlctx->ctx)

#define LOGVAL_PARSER(CTX, ...) (CTX)->format == LYS_IN_YANG ? LOGVAL_YANG(CTX, __VA_ARGS__) : LOGVAL_YIN(CTX, __VA_ARGS__)

#define LOGVAL_YANG(CTX, ...) LOGVAL(PARSER_CTX(CTX), ((struct lys_yang_parser_ctx *)CTX)->pos_type, \
                                     ((struct lys_yang_parser_ctx *)CTX)->pos_type == LY_VLOG_LINE ? \
                                        (void *)&((struct lys_yang_parser_ctx *)CTX)->line : \
                                        (void *)((struct lys_yang_parser_ctx *)CTX)->path, __VA_ARGS__)

#define LOGVAL_YIN(CTX, ...) LOGVAL(PARSER_CTX(CTX), LY_VLOG_LINE, \
                                     &((struct lys_yin_parser_ctx *)CTX)->xmlctx->line, __VA_ARGS__)

struct lys_parser_ctx {
    LYS_INFORMAT format;            /**< parser format */
    struct ly_set tpdfs_nodes;      /**< set of typedef nodes */
    struct ly_set grps_nodes;       /**< set of grouping nodes */
    struct lys_module *main_mod;    /**< main module (belongs-to module for submodules) */
    uint8_t mod_version;            /**< module's version */
};

/**
 * @brief Internal context for yang schema parser.
 */
struct lys_yang_parser_ctx {
    LYS_INFORMAT format;        /**< parser format */
    struct ly_set tpdfs_nodes;  /**< set of typedef nodes */
    struct ly_set grps_nodes;   /**< set of grouping nodes */
    struct lys_module *main_mod;    /**< main module (belongs-to module for submodules) */
    uint8_t mod_version;        /**< module's version */
    enum LY_VLOG_ELEM pos_type; /**< */
    struct ly_ctx *ctx;         /**< context of then yang schemas */
    union {
        uint64_t line;              /**< line number */
        const char *path;           /**< path */
    };
    uint64_t indent;            /**< current position on the line for YANG indentation */
};

/**
 * @brief free lys parser context.
 */
void yang_parser_ctx_free(struct lys_yang_parser_ctx *ctx);

/**
 * @brief Internal context for yin schema parser.
 */
struct lys_yin_parser_ctx {
    LYS_INFORMAT format;           /**< parser format */
    struct ly_set tpdfs_nodes;     /**< set of typedef nodes */
    struct ly_set grps_nodes;      /**< set of grouping nodes */
    struct lys_module *main_mod;    /**< main module (belongs-to module for submodules) */
    uint8_t mod_version;           /**< module's version */
    struct lyxml_ctx *xmlctx;      /**< context for xml parser */
};

/**
 * @brief free yin parser context
 *
 * @param[in] ctx Context to free.
 */
void yin_parser_ctx_free(struct lys_yin_parser_ctx *ctx);

struct lysc_incomplete_dflt {
    struct lyd_value *dflt;
    struct lys_module *dflt_mod;
    struct lysc_node *context_node;
};

/**
 * @brief internal context for compilation
 */
struct lysc_ctx {
    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lys_module *mod_def; /**< context module for the definitions of the nodes being currently
                                     processed - groupings are supposed to be evaluated in place where
                                     defined, but its content instances are supposed to be placed into
                                     the target module (mod) */
    struct ly_set groupings;    /**< stack for groupings circular check */
    struct ly_set xpath;        /**< to validate leafref's targets */
    struct ly_set leafrefs;     /**< when/must to check */
    struct ly_set dflts;        /**< set of incomplete default values */
    struct ly_set tpdf_chain;
    uint16_t path_len;
    int options;                /**< various @ref scflags. */
#define LYSC_CTX_BUFSIZE 4078
    char path[LYSC_CTX_BUFSIZE];
};

/**
 * @brief Check that \p c is valid UTF8 code point for YANG string.
 *
 * @param[in] ctx parser context for logging.
 * @param[in] c UTF8 code point of a character to check.
 * @return LY_ERR values.
 */
LY_ERR lysp_check_stringchar(struct lys_parser_ctx *ctx, unsigned int c);

/**
 * @brief Check that \p c is valid UTF8 code point for YANG identifier.
 *
 * @param[in] ctx parser context for logging.
 * @param[in] c UTF8 code point of a character to check.
 * @param[in] first Flag to check the first character of an identifier, which is more restricted.
 * @param[in,out] prefix Storage for internally used flag in case of possible prefixed identifiers:
 * 0 - colon not yet found (no prefix)
 * 1 - \p c is the colon character
 * 2 - prefix already processed, now processing the identifier
 *
 * If the identifier cannot be prefixed, NULL is expected.
 * @return LY_ERR values.
 */
LY_ERR lysp_check_identifierchar(struct lys_parser_ctx *ctx, unsigned int c, int first, int *prefix);

/**
 * @brief Internal structure for lys_get_prefix().
 */
struct lys_get_prefix_data {
    const struct lys_module *context_mod;
    struct ly_set prefixes;
};

/**
 * @brief Schema mapping of YANG modules to prefixes in values.
 *
 * Implementation of ly_get_prefix_clb. Inverse function to lys_resolve_prefix.
 *
 * In this case the @p mod is searched in the list of imports and the import's prefix
 * (not the module's itself) prefix is returned.
 */
const char *lys_get_prefix(const struct lys_module *mod, void *private);

/**
 * @brief Schema mapping of prefix in values to YANG modules (imports).
 *
 * Implementation of ly_resolve_prefix_clb. Inverse function to lys_get_prefix().
 *
 * In this case the @p prefix is searched in the list of imports' prefixes (not the prefixes of the imported modules themselves).
 */
const struct lys_module *lys_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *private);

/**
 * @brief Check the currently present prefixes in the module for collision with the new one.
 *
 * @param[in] ctx Context for logging.
 * @param[in] imports List of current imports of the module to check prefix collision.
 * @param[in] module_prefix Prefix of the module to check collision.
 * @param[in] value Newly added prefix value (including its location to distinguish collision with itself).
 * @return LY_EEXIST when prefix is already used in the module, LY_SUCCESS otherwise
 */
LY_ERR lysp_check_prefix(struct lys_parser_ctx *ctx, struct lysp_import *imports, const char *module_prefix, const char **value);

/**
 * @brief Check date string (4DIGIT "-" 2DIGIT "-" 2DIGIT)
 *
 * @param[in] ctx Optional context for logging.
 * @param[in] date Date string to check (non-necessarily terminated by \0)
 * @param[in] date_len Length of the date string, 10 expected.
 * @param[in] stmt Statement name for error message.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_date(struct lys_parser_ctx *ctx, const char *date, int date_len, const char *stmt);

/**
 * @brief Check names of typedefs in the parsed module to detect collisions.
 *
 * @param[in] ctx Parser context for logging and to maintain tpdfs_nodes
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_typedefs(struct lys_parser_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Finalize some of the structures in case they are stored in sized array,
 * which can be possibly reallocated and some other data may point to them.
 *
 * Update parent pointers in the nodes inside grouping/augment/RPC/Notification, which could be reallocated.
 *
 * @param[in] mod Parsed module to be updated.
 * @return LY_ERR value (currently only LY_SUCCESS, but it can change in future).
 */
LY_ERR
lysp_parse_finalize_reallocated(struct lys_parser_ctx *ctx, struct lysp_grp *groupings, struct lysp_augment *augments,
                                struct lysp_action *actions, struct lysp_notif *notifs);

/**
 * @brief Just move the newest revision into the first position, does not sort the rest
 * @param[in] revs Sized-array of the revisions in a printable schema tree.
 */
void lysp_sort_revisions(struct lysp_revision *revs);

/**
 * @brief Find type specified type definition.
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
 * @brief Validate enum name.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] name String to check.
 * @param[in] name_len Length of name.
 *
 * @return LY_ERR values
 */
LY_ERR lysp_check_enum_name(struct lys_parser_ctx *ctx, const char *name, size_t name_len);

/**
 * @brief Find and parse module of the given name.
 *
 * @param[in] ctx libyang context.
 * @param[in] name Name of the module to load.
 * @param[in] revison Optional revision of the module to load. If NULL, the newest revision is loaded.
 * @param[in] implement Flag if the loaded module is supposed to be marked as implemented. If revision is NULL and implement flag set,
 * the implemented module in the context is returned despite it might not be of the latest revision, because in this case the module
 * of the latest revision can not be made implemented.
 * @param[in] require_parsed Flag to require parsed module structure in case the module is already in the context,
 * but only the compiled structure is available.
 * @param[out] mod Parsed module structure.
 * @return LY_ERR value.
 */
LY_ERR lysp_load_module(struct ly_ctx *ctx, const char *name, const char *revision, int implement, int require_parsed, struct lys_module **mod);

/**
 * @brief Parse included submodule into the simply parsed YANG module.
 *
 * @param[in] pctx main parser context
 * @param[in,out] inc Include structure holding all available information about the include statement, the parsed
 * submodule is stored into this structure.
 * @return LY_ERR value.
 */
LY_ERR lysp_load_submodule(struct lys_parser_ctx *pctx, struct lysp_include *inc);

/**
 * @brief Compile printable schema into a validated schema linking all the references.
 *
 * @param[in, out] mod Pointer to the schema structure holding pointers to both schema structure types. The ::lys_module#parsed
 * member is used as input and ::lys_module#compiled is used to hold the result of the compilation.
 * If the compilation fails, the whole module is removed from context, freed and @p mod is set to NULL!
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile(struct lys_module **mod, int options);

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
 * @param[in] flags Config flag to distinguish input (LYS_CONFIG_W) and output (LYS_CONFIG_R) data in case of RPC/action node.
 * @return Address of the node's child member if any, NULL otherwise.
 */
struct lysc_node **lysc_node_children_p(const struct lysc_node *node, uint16_t flags);

/**
 * @brief Get address of a node's notifs pointer if any.
 *
 * Decides the node's type and in case it has a notifs array, returns its address.
 * @param[in] node Node to check.
 * @return Address of the node's notifs member if any, NULL otherwise.
 */
struct lysc_notif **lysc_node_notifs_p(struct lysc_node *node);

/**
 * @brief Get address of a node's actions pointer if any.
 *
 * Decides the node's type and in case it has a actions array, returns its address.
 * @param[in] node Node to check.
 * @return Address of the node's actions member if any, NULL otherwise.
 */
struct lysc_action **lysc_node_actions_p(struct lysc_node *node);

/**
 * @brief Iterate over the specified type of the extension instances
 *
 * @param[in] ext ([Sized array](@ref sizedarrays)) of extensions to explore
 * @param[in] index Index in the \p ext array where to start searching (first call with 0, the consequent calls with
 *            the returned index increased by 1 (until the iteration is not terminated by returning LY_ARRAY_COUNT(ext).
 * @param[in] substmt Type of the extension (its belongins to the specific substatement) to iterate, use
 *            #LYEXT_SUBSTMT_ALL to go through all the extensions in the array
 * @result index in the ext array, LY_ARRAY_COUNT(ext) value if not present.
 */
LY_ARRAY_COUNT_TYPE lysp_ext_instance_iter(struct lysp_ext_instance *ext, LY_ARRAY_COUNT_TYPE index, LYEXT_SUBSTMT substmt);

/**
 * @brief Get the covering schema module structure for the given parsed module structure.
 * @param[in] ctx libyang context to search.
 * @param[in] mod Parsed schema structure.
 * @return Corresponding lys_module structure for the given parsed schema structure.
 */
struct lys_module *lysp_find_module(struct ly_ctx *ctx, const struct lysp_module *mod);

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
 * @brief Find the node according to the given descendant/absolute schema nodeid.
 * Used in unique, refine and augment statements.
 *
 * @param[in] ctx Compile context
 * @param[in] nodeid Descendant-schema-nodeid (according to the YANG grammar)
 * @param[in] nodeid_len Length of the given nodeid, if it is not NULL-terminated string.
 * @param[in] context_node Node where the nodeid is specified to correctly resolve prefixes and to start searching.
 * If no context node is provided, the nodeid is actually expected to be the absolute schema node .
 * @param[in] context_module Explicit module to resolve prefixes in @nodeid.
 * @param[in] nodetype Optional (can be 0) restriction for target's nodetype. If target exists, but does not match
 * the given nodetype, LY_EDENIED is returned (and target is provided), but no error message is printed.
 * The value can be even an ORed value to allow multiple nodetypes.
 * @param[in] implement Flag if the modules mentioned in the nodeid are supposed to be made implemented.
 * @param[out] target Found target node if any. In case of RPC/action input/output node, LYS_RPC or LYS_ACTION node is actually returned
 * since the input/output has not a standalone node structure and it is part of ::lysc_action which is better compatible with ::lysc_node.
 * @param[out] result_flag Output parameter to announce if the schema nodeid goes through the action's input/output or a Notification.
 * The LYSC_OPT_RPC_INPUT, LYSC_OPT_RPC_OUTPUT and LYSC_OPT_NOTIFICATION are used as flags.
 * @return LY_ERR values - LY_ENOTFOUND, LY_EVALID, LY_EDENIED or LY_SUCCESS.
 */
LY_ERR lys_resolve_schema_nodeid(struct lysc_ctx *ctx, const char *nodeid, size_t nodeid_len, const struct lysc_node *context_node,
                                 const struct lys_module *context_module, int nodetype, int implement,
                                 const struct lysc_node **target, uint16_t *result_flag);

/**
 * @brief Find the module referenced by prefix in the provided mod.
 *
 * Reverse function to lys_prefix_find_module().
 *
 * @param[in] mod Schema module where the prefix was used.
 * @param[in] prefix Prefix used to reference a module.
 * @param[in] len Length of the prefix since it is not necessary NULL-terminated.
 * @return Pointer to the module or NULL if the module is not found.
 */
struct lys_module *lys_module_find_prefix(const struct lys_module *mod, const char *prefix, size_t len);

/**
 * @brief Find the prefix used to referenced the import module in the provided mod.
 *
 * Reverse function to lys_module_find_prefix().
 *
 * Note that original prefixes are present only in the parsed schema. In case it is not available
 * (only compiled schema available), the own prefix of the import module is returned instead.
 *
 * @param[in] mod Schema module where the import module was used.
 * @param[in] import Module referenced in mod.
 * @return Prefix of the import module.
 */
const char *lys_prefix_find_module(const struct lys_module *mod, const struct lys_module *import);

/**
 * @brief Stringify YANG built-in type.
 * @param[in] basetype Built-in type ID to stringify.
 * @return Constant string with the name of the built-in type.
 */
const char *lys_datatype2str(LY_DATA_TYPE basetype);

typedef LY_ERR (*lys_custom_check)(const struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod,
                                   void *check_data);

/**
 * @brief Parse module from a string.
 *
 * The modules are added into the context and the latest_revision flag is updated.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] in Input structure.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] implement Flag if the schema is supposed to be marked as implemented.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @param[out] module Parsed module.
 * @return LY_ERR value.
 */
LY_ERR lys_parse_mem_module(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, int implement,
                            lys_custom_check custom_check, void *check_data, struct lys_module **module);

/**
 * @brief Parse submodule from a string.
 *
 * The latest_revision flag of submodule is updated.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] in Input structure.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] main_ctx Parser context of the main module.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @param[out] submodule Parsed submodule.
 * @return LY_ERR value.
 */
LY_ERR lys_parse_mem_submodule(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format,
                               struct lys_parser_ctx *main_ctx, lys_custom_check custom_check,
                               void *check_data, struct lysp_submodule **submodule);

/**
 * @brief Fill filepath value if available in input handler @p in
 *
 * @param[in] ctx Context with dictionary where the filepath value will be stored.
 * @param[in] in Input handler to examine (filepath is not available for all the input types).
 * @param[out] filepath Address of the variable where the filepath is stored.
 */
void lys_parser_fill_filepath(struct ly_ctx *ctx, struct ly_in *in, const char **filepath);

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
 * @param[in] main_name Main module name in case of loading submodule.
 * @param[in] required Module is required so error (even if the input file not found) are important. If 0, there is some
 * backup and it is actually ok if the input data are not found. However, parser reports errors even in this case.
 * @param[out] result Parsed YANG schema tree of the requested module (struct lys_module*) or submodule (struct lysp_submodule*).
 * If it is a module, it is already in the context!
 * @return LY_ERR value, in case of LY_SUCCESS, the \arg result is always provided.
 */
LY_ERR lys_module_localfile(struct ly_ctx *ctx, const char *name, const char *revision, int implement,
                            struct lys_parser_ctx *main_ctx, const char *main_name, int required, void **result);

/**
 * @brief Compile information from the identity statement
 *
 * The backlinks to the identities derived from this one are supposed to be filled later via lys_compile_identity_bases().
 *
 * @param[in] ctx_sc Compile context - alternative to the combination of @p ctx and @p module.
 * @param[in] ctx libyang context.
 * @param[in] module Module of the features.
 * @param[in] identities_p Array of the parsed identity definitions to precompile.
 * @param[in,out] identities Pointer to the storage of the (pre)compiled identities array where the new identities are
 * supposed to be added. The storage is supposed to be initiated to NULL when the first parsed identities are going
 * to be processed.
 * @return LY_ERR value.
 */
LY_ERR lys_identity_precompile(struct lysc_ctx *ctx_sc, struct ly_ctx *ctx, struct lys_module *module,
                               struct lysp_ident *identities_p, struct lysc_ident **identities);

/**
 * @brief Create pre-compiled features array.
 *
 * Features are compiled in two steps to allow forward references between them via their if-feature statements.
 * In case of not implemented schemas, the precompiled list of features is stored in lys_module structure and
 * the compilation is not finished (if-feature and extensions are missing) and all the features are permanently
 * disabled without a chance to change it. The list is used as target for any if-feature statement in any
 * implemented module to get valid data to evaluate its result. The compilation is finished via
 * lys_feature_precompile_finish() in implemented modules. In case a not implemented module becomes implemented,
 * the precompiled list is reused to finish the compilation to preserve pointers already used in various compiled
 * if-feature structures.
 *
 * @param[in] ctx_sc Compile context - alternative to the combination of @p ctx and @p module.
 * @param[in] ctx libyang context.
 * @param[in] module Module of the features.
 * @param[in] features_p Array of the parsed features definitions to precompile.
 * @param[in,out] features Pointer to the storage of the (pre)compiled features array where the new features are
 * supposed to be added. The storage is supposed to be initiated to NULL when the first parsed features are going
 * to be processed.
 * @return LY_ERR value.
 */
LY_ERR lys_feature_precompile(struct lysc_ctx *ctx_sc, struct ly_ctx *ctx, struct lys_module *module,
                              struct lysp_feature *features_p, struct lysc_feature **features);

/**
 * @brief Get the @ref ifftokens from the given position in the 2bits array
 * (libyang format of the if-feature expression).
 * @param[in] list The 2bits array with the compiled if-feature expression.
 * @param[in] pos Position (0-based) to specify from which position get the operator.
 */
uint8_t lysc_iff_getop(uint8_t *list, int pos);

/**
 * @brief Checks pattern syntax.
 *
 * @param[in] ctx Context.
 * @param[in] log_path Path for logging errors.
 * @param[in] pattern Pattern to check.
 * @param[in,out] pcre2_code Compiled PCRE2 pattern. If NULL, the compiled information used to validate pattern are freed.
 * @return LY_ERR value - LY_SUCCESS, LY_EMEM, LY_EVALID.
 */
LY_ERR lys_compile_type_pattern_check(struct ly_ctx *ctx, const char *log_path, const char *pattern, pcre2_code **code);

/**
 * @brief Macro to free [sized array](@ref sizedarrays) of items using the provided free function. The ARRAY itself is also freed,
 * but the memory is not sanitized.
 */
#define FREE_ARRAY(CTX, ARRAY, FUNC) {LY_ARRAY_COUNT_TYPE c__; LY_ARRAY_FOR(ARRAY, c__){FUNC(CTX, &(ARRAY)[c__]);}LY_ARRAY_FREE(ARRAY);}

/**
 * @brief Macro to free the specified MEMBER of a structure using the provided free function. The memory is not sanitized.
 */
#define FREE_MEMBER(CTX, MEMBER, FUNC) if (MEMBER) {FUNC(CTX, MEMBER);free(MEMBER);}

/**
 * @brief Macro to free [sized array](@ref sizedarrays) of strings stored in the context's dictionary. The ARRAY itself is also freed,
 * but the memory is not sanitized.
 */
#define FREE_STRINGS(CTX, ARRAY) {LY_ARRAY_COUNT_TYPE c__; LY_ARRAY_FOR(ARRAY, c__){FREE_STRING(CTX, ARRAY[c__]);}LY_ARRAY_FREE(ARRAY);}

/**
 * @brief Free the parsed submodule structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] submod Parsed schema submodule structure to free.
 */
void lysp_submodule_free(struct ly_ctx *ctx, struct lysp_submodule *submod);

/**
 * @brief Free the parsed type structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in] type Parsed schema type structure to free. Note that the type itself is not freed.
 */
void lysp_type_free(struct ly_ctx *ctx, struct lysp_type *type);

/**
 * @brief Free the parsed extension instance structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in] type Parsed extension instance structure to free. Note that the instance itself is not freed.
 */
void lysp_ext_instance_free(struct ly_ctx *ctx, struct lysp_ext_instance *ext);

/**
 * @param[in,out] exts [sized array](@ref sizedarrays) For extension instances in case of statements that do not store extension instances in their own list.
 */
LY_ERR lysp_stmt_parse(struct lysc_ctx *ctx, const struct lysp_stmt *stmt, enum ly_stmt kw, void **result, struct lysp_ext_instance **exts);

/**
 * @brief Free the compiled type structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] type Compiled type structure to be freed. The structure has refcount, so it is freed only in case the value is decreased to 0.
 */
void lysc_type_free(struct ly_ctx *ctx, struct lysc_type *type);

/**
 * @brief Free the compiled if-feature structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] iff Compiled if-feature structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_iffeature_free(struct ly_ctx *ctx, struct lysc_iffeature *iff);

/**
 * @brief Free the compiled must structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] must Compiled must structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_must_free(struct ly_ctx *ctx, struct lysc_must *must);

/**
 * @brief Free the data inside compiled input/output structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] inout Compiled inout structure to be cleaned.
 * Since the structure is part of the RPC/action structure, it is not freed itself.
 */
void lysc_action_inout_free(struct ly_ctx *ctx, struct lysc_action_inout *inout);

/**
 * @brief Free the data inside compiled RPC/action structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] action Compiled action structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_action_free(struct ly_ctx *ctx, struct lysc_action *action);

/**
 * @brief Free the items inside the compiled Notification structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] action Compiled Notification structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_notif_free(struct ly_ctx *ctx, struct lysc_notif *notif);

/**
 * @brief Free the compiled extension instance structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] ext Compiled extension instance structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_ext_instance_free(struct ly_ctx *ctx, struct lysc_ext_instance *ext);

/**
 * @brief Free the compiled node structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] node Compiled node structure to be freed.
 */
void lysc_node_free(struct ly_ctx *ctx, struct lysc_node *node);

/**
 * @brief Free the compiled container node structure.
 *
 * Only the container-specific members are freed, for generic node free function,
 * use lysc_node_free().
 *
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] node Compiled container node structure to be freed.
 */
void lysc_node_container_free(struct ly_ctx *ctx, struct lysc_node_container *node);

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
 * @brief Make the specific module implemented, use the provided value as flag.
 *
 * @param[in] mod Module to make implemented. It is not an error to provide already implemented module, it just does nothing.
 * @param[in] implemented Flag value for the ::lys_module#implemented item.
 * @return LY_SUCCESS or LY_EDENIED in case the context contains some other revision of the
 * same module which is already implemented.
 */
LY_ERR lys_set_implemented_internal(struct lys_module *mod, uint8_t implemented);

/**
 * @brief match yang keyword
 *
 * @param[in] ctx yang parser context for logging, can be NULL if keyword is from YIN data.
 * @param[in,out] in Input structure, is updated.
 * @return yang_keyword values.
 */
enum ly_stmt lysp_match_kw(struct lys_yang_parser_ctx *ctx, struct ly_in *in);

/**
 * @brief Generate path of the given node in the requested format.
 *
 * @param[in] node Schema path of this node will be generated.
 * @param[in] parent Build relative path only until this parent is found. If NULL, the full absolute path is printed.
 * @param[in] pathtype Format of the path to generate.
 * @param[in,out] buffer Prepared buffer of the @p buflen length to store the generated path.
 *                If NULL, memory for the complete path is allocated.
 * @param[in] buflen Size of the provided @p buffer.
 * @return NULL in case of memory allocation error, path of the node otherwise.
 * In case the @p buffer is NULL, the returned string is dynamically allocated and caller is responsible to free it.
 */
char *lysc_path_until(const struct lysc_node *node, const struct lysc_node *parent, LYSC_PATH_TYPE pathtype, char *buffer,
                      size_t buflen);

/**
 * @brief Get schema parent that can be instantiated in data. In other words, skip any choice or case nodes.
 *
 * @param[in] schema Schema node to get the parent for.
 * @return Parent, NULL if top-level (in data).
 */
const struct lysc_node *lysc_data_parent(const struct lysc_node *schema);

/**
 * @brief Learn whether a node is in an operation output.
 *
 * @param[in] schema Schema node to examine.
 * @return non-zero is the node is in output,
 * @return 0 if it is not.
 */
int lysc_is_output(const struct lysc_node *schema);

#endif /* LY_TREE_SCHEMA_INTERNAL_H_ */
