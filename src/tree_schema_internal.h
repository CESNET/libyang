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

struct lysc_ctx;
struct lys_glob_unres;

#define LY_YANG_SUFFIX ".yang"
#define LY_YANG_SUFFIX_LEN 5
#define LY_YIN_SUFFIX ".yin"
#define LY_YIN_SUFFIX_LEN 4

#define YIN_NS_URI "urn:ietf:params:xml:ns:yang:yin:1"

#define LY_PCRE2_MSG_LIMIT 256

/**
 * @brief Check module version is at least 2 (YANG 1.1) because of the keyword presence.
 * Logs error message and returns LY_EVALID in case of module in YANG version 1.0.
 * @param[in] CTX yang parser context to get current module and for logging.
 * @param[in] KW keyword allowed only in YANG version 1.1 (or later) - for logging.
 * @param[in] PARENT parent statement where the KW is present - for logging.
 */
#define PARSER_CHECK_STMTVER2_RET(CTX, KW, PARENT) \
    if ((CTX)->parsed_mod->version < LYS_VERSION_1_1) {LOGVAL_PARSER((CTX), LY_VCODE_INCHILDSTMT2, KW, PARENT); return LY_EVALID;}

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

/*
 * Additional YANG constants
 */
#define Y_TAB_SPACES         8  /**< number of spaces instead of tab character */
#define LY_TYPE_DEC64_FD_MAX 18 /**< Maximal value of decimal64's fraction-digits */

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

#define PARSER_CTX(CTX) ((CTX)->parsed_mod->mod->ctx)

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
    struct lysp_module *parsed_mod; /**< (sub)module being parsed */
    struct lys_glob_unres *unres;   /**< global unres structure */
};

/**
 * @brief Internal context for yang schema parser.
 */
struct lys_yang_parser_ctx {
    LYS_INFORMAT format;            /**< parser format */
    struct ly_set tpdfs_nodes;      /**< set of typedef nodes */
    struct ly_set grps_nodes;       /**< set of grouping nodes */
    struct lysp_module *parsed_mod; /**< (sub)module being parsed */
    struct lys_glob_unres *unres;   /**< global unres structure */
    enum LY_VLOG_ELEM pos_type;     /**< */
    struct ly_in *in;               /**< input handler for the parser */
    union {
        uint64_t line;              /**< line number */
        const char *path;           /**< path */
    };
    uint64_t indent;                /**< current position on the line for YANG indentation */
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
    struct lysp_module *parsed_mod;/**< (sub)module being parsed */
    struct lys_glob_unres *unres;   /**< global unres structure */
    struct lyxml_ctx *xmlctx;      /**< context for xml parser */
};

/**
 * @brief free yin parser context
 *
 * @param[in] ctx Context to free.
 */
void yin_parser_ctx_free(struct lys_yin_parser_ctx *ctx);

/**
 * @brief Check that \p c is valid UTF8 code point for YANG string.
 *
 * @param[in] ctx parser context for logging.
 * @param[in] c UTF8 code point of a character to check.
 * @return LY_ERR values.
 */
LY_ERR lysp_check_stringchar(struct lys_parser_ctx *ctx, uint32_t c);

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
LY_ERR lysp_check_identifierchar(struct lys_parser_ctx *ctx, uint32_t c, ly_bool first, uint8_t *prefix);

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
LY_ERR lysp_check_date(struct lys_parser_ctx *ctx, const char *date, uint8_t date_len, const char *stmt);

/**
 * @brief Check names of typedefs in the parsed module to detect collisions.
 *
 * @param[in] ctx Parser context for logging and to maintain tpdfs_nodes
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_typedefs(struct lys_parser_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Check names of features in the parsed module and submodules to detect collisions.
 *
 * @param[in] ctx Parser context.
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_features(struct lys_parser_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Check names of identities in the parsed module and submodules to detect collisions.
 *
 * @param[in] ctx Parser context.
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_identities(struct lys_parser_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Finalize some of the structures in case they are stored in sized array,
 * which can be possibly reallocated and some other data may point to them.
 *
 * Update parent pointers in the nodes inside grouping/augment/RPC/Notification, which could be reallocated.
 *
 * @param[in] mod Parsed module to be updated.
 * @return LY_ERR value (currently only LY_SUCCESS, but it can change in future).
 */
LY_ERR lysp_parse_finalize_reallocated(struct lys_parser_ctx *ctx, struct lysp_grp *groupings, struct lysp_augment *augments,
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
 * @param[in] features All the features to enable if implementing the module.
 * @param[in] unres Global unres structure for all newly implemented modules.
 * @param[out] mod Parsed module structure.
 * @return LY_ERR value.
 */
LY_ERR lysp_load_module(struct ly_ctx *ctx, const char *name, const char *revision, ly_bool implement,
        const char **features, struct lys_glob_unres *unres, struct lys_module **mod);

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
 * @brief Free a parsed restriction.
 *
 * @param[in] ctx libyang context.
 * @param[in] restr Restriction to free.
 */
void lysp_restr_free(struct ly_ctx *ctx, struct lysp_restr *restr);

/**
 * @brief Free a parsed qualified name.
 *
 * @param[in] ctx libyang context.
 * @param[in] qname Qualified name to free.
 */
void lysp_qname_free(struct ly_ctx *ctx, struct lysp_qname *qname);

/**
 * @brief Free a parsed node.
 *
 * @param[in] ctx libyang context.
 * @param[in] node Node to free.
 */
void lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node);

/**
 * @brief Free a parsed input/output node.
 *
 * @param[in] ctx libyang context.
 * @param[in] inout Input/output to free.
 */
void lysp_action_inout_free(struct ly_ctx *ctx, struct lysp_action_inout *inout);

/**
 * @brief Free a parsed action node.
 *
 * @param[in] ctx libyang context.
 * @param[in] action Action to free.
 */
void lysp_action_free(struct ly_ctx *ctx, struct lysp_action *action);

/**
 * @brief Free a parsed notification node.
 *
 * @param[in] ctx libyang context.
 * @param[in] notif Notification to free.
 */
void lysp_notif_free(struct ly_ctx *ctx, struct lysp_notif *notif);

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
 * @brief Stringify YANG built-in type.
 * @param[in] basetype Built-in type ID to stringify.
 * @return Constant string with the name of the built-in type.
 */
const char *lys_datatype2str(LY_DATA_TYPE basetype);

/**
 * @brief Implement a module (just like ::lys_set_implemented()), can be called recursively.
 *
 * @param[in] mod Module to implement.
 * @param[in] features Array of features to enable.
 * @param[in,out] unres Global unres to add to.
 * @return LY_ERR value.
 */
LY_ERR lys_set_implemented_r(struct lys_module *mod, const char **features, struct lys_glob_unres *unres);

typedef LY_ERR (*lys_custom_check)(const struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod,
        void *check_data);

/**
 * @brief Create a new module.
 *
 * It is parsed, opionally compiled, added into the context, and the latest_revision flag is updated.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] in Input structure.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] implement Flag if the schema is supposed to be marked as implemented and compiled.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @param[in] features Array of features to enable ended with NULL. NULL for all features disabled and '*' for all enabled.
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @param[out] module Created module.
 * @return LY_ERR value.
 */
LY_ERR lys_create_module(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, ly_bool implement,
        lys_custom_check custom_check, void *check_data, const char **features, struct lys_glob_unres *unres,
        struct lys_module **module);

/**
 * @brief Parse submodule.
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
LY_ERR lys_parse_submodule(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format,
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
 * @param[in] features Array of enabled features ended with NULL.
 * @param[in] implement Flag if the (sub)module is supposed to be marked as implemented.
 * @param[in] main_ctx Parser context of the main module in case of loading submodule.
 * @param[in] main_name Main module name in case of loading submodule.
 * @param[in] required Module is required so error (even if the input file not found) are important. If 0, there is some
 * backup and it is actually ok if the input data are not found. However, parser reports errors even in this case.
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @param[out] result Parsed YANG schema tree of the requested module (struct lys_module*) or submodule (struct lysp_submodule*).
 * If it is a module, it is already in the context!
 * @return LY_ERR value, in case of LY_SUCCESS, the \arg result is always provided.
 */
LY_ERR lys_module_localfile(struct ly_ctx *ctx, const char *name, const char *revision, const char **features,
        ly_bool implement, struct lys_parser_ctx *main_ctx, const char *main_name, ly_bool required,
        struct lys_glob_unres *unres, void **result);

/**
 * @brief Get the @ref ifftokens from the given position in the 2bits array
 * (libyang format of the if-feature expression).
 * @param[in] list The 2bits array with the compiled if-feature expression.
 * @param[in] pos Position (0-based) to specify from which position get the operator.
 */
uint8_t lysc_iff_getop(uint8_t *list, size_t pos);

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
 * @brief Free the printable YANG schema tree structure. Works for both modules and submodules.
 *
 * @param[in] module Printable YANG schema tree structure to free.
 */
void lysp_module_free(struct lysp_module *module);

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
 * @brief Free a parsed node.
 *
 * @param[in] ctx libyang context.
 * @param[in] node Node to free.
 */
void lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node);

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
 * @brief Free the compiled identity structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] ident Compiled identity structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_ident_free(struct ly_ctx *ctx, struct lysc_ident *ident);

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
 * @param[in] node Compiled node structure to be freed.
 * @param[in] unlink Whether to first unlink the node before freeing.
 */
void lysc_node_free(struct ly_ctx *ctx, struct lysc_node *node, ly_bool unlink);

/**
 * @brief Free the compiled container node structure.
 *
 * Only the container-specific members are freed, for generic node free function,
 * use ::lysc_node_free().
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
 * @brief Learn whether a node is inside an operation output.
 *
 * @param[in] schema Schema node to examine.
 * @return Boolean value whether the node is under an operation output or not.
 */
ly_bool lysc_is_output(const struct lysc_node *schema);

/**
 * @brief Get format-specific prefix for a module.
 *
 * For type plugins available as ::ly_type_print_get_prefix().
 *
 * @param[in] mod Module whose prefix to get.
 * @param[in] format Format of the prefix.
 * @param[in] prefix_data Format-specific data:
 *      LY_PREF_SCHEMA          - const struct lysp_module * (module used for resolving imports to prefixes)
 *      LY_PREF_SCHEMA_RESOLVED - struct lyd_value_prefix * (sized array of pairs: prefix - module)
 *      LY_PREF_XML             - struct ly_set * (set of all returned modules as ::struct lys_module)
 *      LY_PREF_JSON            - NULL
 * @return Module prefix to print.
 * @return NULL on error.
 */
const char *ly_get_prefix(const struct lys_module *mod, LY_PREFIX_FORMAT format, void *prefix_data);

/**
 * @brief Resolve format-specific prefixes to modules.
 *
 * For type plugins available as ::ly_type_store_resolve_prefix().
 *
 * @param[in] ctx libyang context.
 * @param[in] prefix Prefix to resolve.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] format Format of the prefix.
 * @param[in] prefix_data Format-specific data:
 *      LY_PREF_SCHEMA          - const struct lysp_module * (module used for resolving prefixes from imports)
 *      LY_PREF_SCHEMA_RESOLVED - struct lyd_value_prefix * (sized array of pairs: prefix - module)
 *      LY_PREF_XML             - const struct ly_set * (set with defined namespaces stored as ::lyxml_ns)
 *      LY_PREF_JSON            - NULL
 * @return Resolved prefix module,
 * @return NULL otherwise.
 */
const struct lys_module *ly_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len,
        LY_PREFIX_FORMAT format, void *prefix_data);

#endif /* LY_TREE_SCHEMA_INTERNAL_H_ */
