/**
 * @file common.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common internal definitions for libyang
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_COMMON_H_
#define LY_COMMON_H_

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"

#include "context.h"
#include "dict.h"
#include "hash_table.h"
#include "log.h"
#include "set.h"

struct ly_ctx;

#if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
# define THREAD_LOCAL _Thread_local
#elif defined __GNUC__ || \
      defined __SUNPRO_C || \
      defined __xlC__
# define THREAD_LOCAL __thread
#else
# error "Cannot define THREAD_LOCAL"
#endif

#define GETMACRO1(_1, NAME, ...) NAME
#define GETMACRO2(_1, _2, NAME, ...) NAME
#define GETMACRO3(_1, _2, _3, NAME, ...) NAME
#define GETMACRO4(_1, _2, _3, _4, NAME, ...) NAME
#define GETMACRO5(_1, _2, _3, _4, _5, NAME, ...) NAME

/*
 * If the compiler supports attribute to mark objects as hidden, mark all
 * objects as hidden and export only objects explicitly marked to be part of
 * the public API.
 */
#define API __attribute__((visibility("default")))


/******************************************************************************
 * Compatibility functions
 *****************************************************************************/

#ifndef HAVE_GET_CURRENT_DIR_NAME
/**
 * @brief Return a malloc'd string containing the current directory name.
 */
char *get_current_dir_name(void);
#endif

#ifndef HAVE_STRNSTR
/**
 * @brief Find the first occurrence of find in s, where the search is limited to the
 * first slen characters of s.
 */
char *strnstr(const char *s, const char *find, size_t slen);
#endif


/******************************************************************************
 * Logger
 *****************************************************************************/

enum LY_VLOG_ELEM {
    LY_VLOG_NONE = 0,
    LY_VLOG_LINE, /* line number (uint64_t*) */
    LY_VLOG_LYSC, /* struct lysc_node* */
    LY_VLOG_LYD,  /* struct lyd_node* */
    LY_VLOG_STR,  /* const char* */
    LY_VLOG_PREV  /* use exact same previous path */
};

extern THREAD_LOCAL enum int_log_opts log_opt;
extern volatile uint8_t ly_log_level;
extern volatile uint8_t ly_log_opts;

/**
 * @brief Set error-app-tag to the last error record in the context.
 * @param[in] ctx libyang context where the error records are present.
 * @param[in] apptag The error-app-tag value to store.
 */
void ly_err_last_set_apptag(const struct ly_ctx *ctx, const char *apptag);

/**
 * @brief Print a log message and store it into the context (if provided).
 *
 * @param[in] ctx libyang context to store the error record. If not provided, the error is just printed.
 * @param[in] level Log message level (error, warning, etc.)
 * @param[in] no Error type code.
 * @param[in] format Format string to print.
 */
void ly_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, const char *format, ...);

/**
 * @brief Print Validation error and store it into the context (if provided).
 *
 * @param[in] ctx libyang context to store the error record. If not provided, the error is just printed.
 * @param[in] elem_type Type of the data in @p elem variable.
 * @param[in] elem Object to provide more information about the place where the error appeared.
 * @param[in] code Validation error code.
 * @param[in] format Format string to print.
 */
void ly_vlog(const struct ly_ctx *ctx, enum LY_VLOG_ELEM elem_type, const void *elem, LY_VECODE code, const char *format, ...);

#define LOGERR(ctx, errno, str, args...) ly_log(ctx, LY_LLERR, errno, str, ##args)
#define LOGWRN(ctx, str, ...) ly_log(ctx, LY_LLWRN, 0, str, ##__VA_ARGS__)
#define LOGVRB(str, args...) ly_log(NULL, LY_LLVRB, 0, str, ##args)

#ifdef NDEBUG
#  define LOGDBG(dbg_group, str, args...)
#else
   void ly_log_dbg(int group, const char *format, ...);
#  define LOGDBG(dbg_group, str, args...) ly_log_dbg(dbg_group, str, ##args);
#endif

#define LOGMEM(CTX) LOGERR(CTX, LY_EMEM, "Memory allocation failed (%s()).", __func__)
#define LOGINT(CTX) LOGERR(CTX, LY_EINT, "Internal error (%s:%d).", __FILE__, __LINE__)
#define LOGARG(CTX, ARG) LOGERR(CTX, LY_EINVAL, "Invalid argument %s (%s()).", #ARG, __func__)
#define LOGVAL(CTX, ELEM_TYPE, ELEM, CODE, FORMAT...) ly_vlog(CTX, ELEM_TYPE, ELEM, CODE, ##FORMAT)

#define LOGMEM_RET(CTX) LOGMEM(CTX); return LY_EMEM
#define LOGINT_RET(CTX) LOGINT(CTX); return LY_EINT
#define LOGARG_RET(CTX) LOGARG(CTX); return LY_EINVAL

/*
 * Common code to check return value and perform appropriate action.
 */
#define LY_CHECK_GOTO(COND, GOTO) if ((COND)) {goto GOTO;}
#define LY_CHECK_ERR_GOTO(COND, ERR, GOTO) if ((COND)) {ERR; goto GOTO;}
#define LY_CHECK_RET1(RETVAL) {LY_ERR ret__ = RETVAL;if (ret__ != LY_SUCCESS) {return ret__;}}
#define LY_CHECK_RET2(COND, RETVAL) if ((COND)) {return RETVAL;}
#define LY_CHECK_RET(...) GETMACRO2(__VA_ARGS__, LY_CHECK_RET2, LY_CHECK_RET1)(__VA_ARGS__)
#define LY_CHECK_ERR_RET(COND, ERR, RETVAL) if ((COND)) {ERR; return RETVAL;}

#define LY_CHECK_ARG_GOTO1(CTX, ARG, GOTO) if (!(ARG)) {LOGARG(CTX, ARG);goto GOTO;}
#define LY_CHECK_ARG_GOTO2(CTX, ARG1, ARG2, GOTO) LY_CHECK_ARG_GOTO1(CTX, ARG1, GOTO);LY_CHECK_ARG_GOTO1(CTX, ARG2, GOTO)
#define LY_CHECK_ARG_GOTO3(CTX, ARG1, ARG2, ARG3, GOTO) LY_CHECK_ARG_GOTO2(CTX, ARG1, ARG2, GOTO);LY_CHECK_ARG_GOTO1(CTX, ARG3, GOTO)
#define LY_CHECK_ARG_GOTO4(CTX, ARG1, ARG2, ARG3, ARG4, GOTO) LY_CHECK_ARG_GOTO3(CTX, ARG1, ARG2, ARG3, GOTO);\
    LY_CHECK_ARG_GOTO1(CTX, ARG4, GOTO)
#define LY_CHECK_ARG_GOTO(CTX, ...) GETMACRO5(__VA_ARGS__, LY_CHECK_ARG_GOTO4, LY_CHECK_ARG_GOTO3, LY_CHECK_ARG_GOTO2, \
    LY_CHECK_ARG_GOTO1)(CTX, __VA_ARGS__)

#define LY_CHECK_ARG_RET1(CTX, ARG, RETVAL) if (!(ARG)) {LOGARG(CTX, ARG);return RETVAL;}
#define LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL) LY_CHECK_ARG_RET1(CTX, ARG1, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG2, RETVAL)
#define LY_CHECK_ARG_RET3(CTX, ARG1, ARG2, ARG3, RETVAL) LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG3, RETVAL)
#define LY_CHECK_ARG_RET4(CTX, ARG1, ARG2, ARG3, ARG4, RETVAL) LY_CHECK_ARG_RET3(CTX, ARG1, ARG2, ARG3, RETVAL);\
    LY_CHECK_ARG_RET1(CTX, ARG4, RETVAL)
#define LY_CHECK_ARG_RET(CTX, ...) GETMACRO5(__VA_ARGS__, LY_CHECK_ARG_RET4, LY_CHECK_ARG_RET3, LY_CHECK_ARG_RET2, LY_CHECK_ARG_RET1)\
    (CTX, __VA_ARGS__)

/* count sequence size for LY_VCODE_INCHILDSTMT validation error code */
size_t LY_VCODE_INSTREXP_len(const char *str);
/* default maximum characters to print in LY_VCODE_INCHILDSTMT */
#define LY_VCODE_INSTREXP_MAXLEN 20

#define LY_VCODE_INCHAR         LYVE_SYNTAX, "Invalid character 0x%x."
#define LY_VCODE_INSTREXP       LYVE_SYNTAX, "Invalid character sequence \"%.*s\", expected %s."
#define LY_VCODE_EOF            LYVE_SYNTAX, "Unexpected end-of-input."
#define LY_VCODE_NTERM          LYVE_SYNTAX, "%s not terminated."
#define LY_VCODE_NSUPP          LYVE_SYNTAX, "%s not supported."
#define LY_VCODE_MOD_SUBOMD     LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\"."
#define LY_VCODE_TRAILING_MOD   LYVE_SYNTAX, "Trailing garbage \"%.*s%s\" after module, expected end-of-input."
#define LY_VCODE_TRAILING_SUBMOD LYVE_SYNTAX, "Trailing garbage \"%.*s%s\" after submodule, expected end-of-input."

#define LY_VCODE_INVAL_MINMAX   LYVE_SEMANTICS, "Invalid combination of min-elements and max-elements: min value %u is bigger than the max value %u."
#define LY_VCODE_CIRC_WHEN      LYVE_SEMANTICS, "When condition of \"%s\" includes a self-reference (referenced by when of \"%s\")."
#define LY_VCODE_DUMMY_WHEN     LYVE_SEMANTICS, "When condition of \"%s\" is accessing its own conditional node."

#define LY_VCODE_INSTMT         LYVE_SYNTAX_YANG, "Invalid keyword \"%s\"."
#define LY_VCODE_INCHILDSTMT    LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s\"."
#define LY_VCODE_INCHILDSTMT2   LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s\" - the statement is allowed only in YANG 1.1 modules."
#define LY_VCODE_INCHILDSTMSCOMB LYVE_SYNTAX_YANG, "Invalid combination of keywords \"%s\" and \"%s\" as substatements of \"%s\"."
#define LY_VCODE_DUPSTMT        LYVE_SYNTAX_YANG, "Duplicate keyword \"%s\"."
#define LY_VCODE_DUPIDENT       LYVE_SYNTAX_YANG, "Duplicate identifier \"%s\" of %s statement."
#define LY_VCODE_INVAL          LYVE_SYNTAX_YANG, "Invalid value \"%.*s\" of \"%s\"."
#define LY_VCODE_MISSTMT        LYVE_SYNTAX_YANG, "Missing mandatory keyword \"%s\" as a child of \"%s\"."
#define LY_VCODE_MISSCHILDSTMT  LYVE_SYNTAX_YANG, "Missing %s substatement for %s%s."
#define LY_VCODE_INORD          LYVE_SYNTAX_YANG, "Invalid keyword \"%s\", it cannot appear after \"%s\"."
#define LY_VCODE_OOB            LYVE_SYNTAX_YANG, "Value \"%.*s\" is out of \"%s\" bounds."
#define LY_VCODE_INDEV          LYVE_SYNTAX_YANG, "Deviate \"%s\" does not support keyword \"%s\"."
#define LY_VCODE_INREGEXP       LYVE_SYNTAX_YANG, "Regular expression \"%s\" is not valid (\"%s\": %s)."

#define LY_VCODE_INSUBELEM2     LYVE_SYNTAX_YIN, "Invalid sub-elemnt \"%s\" of \"%s\" element - this sub-element is allowed only in modules with version 1.1 or newer."
#define LY_VCODE_INVAL_YIN      LYVE_SYNTAX_YIN, "Invalid value \"%s\" of \"%s\" attribute in \"%s\" element."
#define LY_VCODE_UNEXP_SUBELEM  LYVE_SYNTAX_YIN, "Unexpected sub-element \"%.*s\" of \"%s\" element."
#define LY_VCODE_INDEV_YIN      LYVE_SYNTAX_YIN, "Deviate of this type doesn't allow \"%s\" as it's sub-element."
#define LY_VCODE_INORDER_YIN    LYVE_SYNTAX_YIN, "Invalid order of %s\'s sub-elements \"%s\" can't appear after \"%s\"."
#define LY_VCODE_OOB_YIN        LYVE_SYNTAX_YIN, "Value \"%s\" of \"%s\" attribute in \"%s\" element is out of bounds."
#define LY_VCODE_INCHILDSTMSCOMB_YIN LYVE_SYNTAX_YIN, "Invalid combination of sub-elemnts \"%s\" and \"%s\" in \"%s\" element."
#define LY_VCODE_DUP_ATTR       LYVE_SYNTAX_YIN, "Duplicit definition of \"%s\" attribute in \"%s\" element."
#define LY_VCODE_UNEXP_ATTR     LYVE_SYNTAX_YIN, "Unexpected attribute \"%.*s\" of \"%s\" element."
#define LY_VCODE_MAND_SUBELEM   LYVE_SYNTAX_YIN, "Missing mandatory sub-element \"%s\" of \"%s\" element."
#define LY_VCODE_FIRT_SUBELEM   LYVE_SYNTAX_YIN, "Sub-element \"%s\" of \"%s\" element must be defined as it's first sub-element."
#define LY_VCODE_NAME_COL       LYVE_SYNTAX_YIN, "Name collision between module and submodule of name \"%s\"."
#define LY_VCODE_SUBELEM_REDEF  LYVE_SYNTAX_YIN, "Redefinition of \"%s\" sub-element in \"%s\" element."

#define LY_VCODE_XP_EOE         LYVE_XPATH, "Unterminated string delimited with %c (%.15s)."
#define LY_VCODE_XP_INEXPR      LYVE_XPATH, "Invalid character number %u of expression \'%s\'."
#define LY_VCODE_XP_EOF         LYVE_XPATH, "Unexpected XPath expression end."
#define LY_VCODE_XP_INTOK       LYVE_XPATH, "Unexpected XPath token %s (%.15s)."
#define LY_VCODE_XP_INFUNC      LYVE_XPATH, "Unknown XPath function \"%.*s\"."
#define LY_VCODE_XP_INARGCOUNT  LYVE_XPATH, "Invalid number of arguments (%d) for the XPath function %.*s."
#define LY_VCODE_XP_INARGTYPE   LYVE_XPATH, "Wrong type of argument #%d (%s) for the XPath function %s."
#define LY_VCODE_XP_INCTX       LYVE_XPATH, "Invalid context type %s in %s."
#define LY_VCODE_XP_INOP_1      LYVE_XPATH, "Cannot apply XPath operation %s on %s."
#define LY_VCODE_XP_INOP_2      LYVE_XPATH, "Cannot apply XPath operation %s on %s and %s."
#define LY_VCODE_XP_INMOD       LYVE_XPATH, "Unknown module \"%.*s\"."

#define LY_VCODE_DEV_NODETYPE   LYVE_REFERENCE, "Invalid deviation of %s node - it is not possible to %s \"%s\" property."
#define LY_VCODE_DEV_NOT_PRESENT LYVE_REFERENCE, "Invalid deviation %s \"%s\" property \"%s\" which is not present."

#define LY_VCODE_NOWHEN         LYVE_DATA, "When condition \"%s\" not satisfied."

/******************************************************************************
 * Context
 *****************************************************************************/

/**
 * @brief Context of the YANG schemas
 */
struct ly_ctx {
    struct dict_table dict;           /**< dictionary to effectively store strings used in the context related structures */
    struct ly_set search_paths;       /**< set of directories where to search for schema's imports/includes */
    struct ly_set list;               /**< set of YANG schemas */
    ly_module_imp_clb imp_clb;        /**< Optional callback for retrieving missing included or imported models in a custom way. */
    void *imp_clb_data;               /**< Optional private data for imp_clb() */
    uint16_t module_set_id;           /**< ID of the current set of schemas */
    uint16_t flags;                   /**< context settings, see @ref contextoptions. */
    pthread_key_t errlist_key;        /**< key for the thread-specific list of errors related to the context */
};

/**
 * @defgroup contextflags Context flags
 * @ingroup context
 *
 * Internal context flags.
 *
 * Note that the flags 0x00FF are reserved for @ref contextoptions.
 * @{
 */

#define LY_CTX_CHANGED_TREE 0x8000    /**< Deviation changed tree of a module(s) in the context, it is necessary to recompile
                                           leafref paths, default values and must/when expressions to check that they are still valid */

/**@} contextflags */

/**
 * @brief Try to find submodule in the context. Submodules are present only in the parsed (lysp_) schema trees, if only
 * the compiled versions of the schemas are present, the submodule cannot be returned even if it was used to compile
 * some of the currently present schemas.
 *
 * @param[in] ctx Context where to search
 * @param[in] module Name of the module where the submodule is supposed to belongs-to. If NULL, the module name is not checked.
 * @param[in] submodule Name of the submodule to find.
 * @param[in] revision Optional revision of the submodule to find. If not specified, the latest revision is returned.
 * @return Pointer to the specified submodule if it is present in the context.
 */
struct lysp_submodule *ly_ctx_get_submodule(const struct ly_ctx *ctx, const char *module, const char *submodule, const char *revision);

/******************************************************************************
 * Parsers
 *****************************************************************************/

/* list of the YANG statements strings */
extern const char *const ly_stmt_list[];
#define ly_stmt2str(STMT) ly_stmt_list[STMT]

/* list of the extensions' substatements strings */
extern const char *const lyext_substmt_list[];
#define lyext_substmt2str(STMT) lyext_substmt_list[STMT]

/* list of the deviate modifications strings */
extern const char *const ly_devmod_list[];
#define ly_devmod2str(TYPE) ly_devmod_list[TYPE]

/******************************************************************************
 * Generic useful functions.
 *****************************************************************************/

#define FREE_STRING(CTX, STRING) if (STRING) {lydict_remove(CTX, STRING);}

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
 * @brief Just like strchr() function except limit the number of examined characters.
 *
 * @param[in] s String to search in.
 * @param[in] c Character to search for.
 * @param[in] len Limit the search to this number of characters in @p s.
 * @return Pointer to first @p c occurence in @p s, NULL if not found in first @p len characters.
 */
char *ly_strnchr(const char *s, int c, unsigned int len);

/**
 * @brief Compare NULL-terminated @p refstr with @str_len bytes from @p str.
 *
 * @param[in] refstr NULL-terminated string which must match @str_len bytes from @str followed by NULL-byte.
 * @param[in] str String to compare
 * @param[in] str_len Number of bytes to take into comparison from @p str.
 * @return An integer less than, equal to, or greater than zero if @p refstr matches,
 * respectively, to be less than, to match, or be greater than @p str.
 */
int ly_strncmp(const char *refstr, const char *str, size_t str_len);

/**
 * @brief Get UTF8 code point of the next character in the input string.
 *
 * @param[in,out] input Input string to process, updated according to the processed/read data.
 * @param[out] utf8_char UTF8 code point of the next character.
 * @param[out] bytes_read Number of bytes used to encode the read utf8_char.
 * @return LY_ERR value
 */
LY_ERR ly_getutf8(const char **input, unsigned int *utf8_char, size_t *bytes_read);

/**
 * @brief Get number of characters in the @p str, taking multibyte characters into account.
 * @param[in] str String to examine.
 * @param[in] bytes Number of valid bytes that are supposed to be taken into account in @p str.
 * This parameter is useful mainly for non NULL-terminated strings. In case of NULL-terminated
 * string, strlen() can be used.
 * @return Number of characters in (possibly) multibyte characters string.
 */
size_t ly_utf8len(const char *str, size_t bytes);

/**
 * @brief Parse signed integer with possible limitation.
 * @param[in] val_str String value containing signed integer, note that
 * nothing else than whitespaces are expected after the value itself.
 * @param[in] val_len Length of the @p val_str string.
 * @param[in] min Limitation for the value which must not be lower than min.
 * @param[in] max Limitation for the value which must not be higher than max.
 * @param[in] base Numeric base for parsing:
 *        0 - to accept decimal, octal, hexadecimal (e.g. in default value)
 *       10 - to accept only decimal (e.g. data instance value)
 * @param[out] ret Resulting value.
 * @return LY_ERR value:
 * LY_EDENIED - the value breaks the limits,
 * LY_EVALID - string contains invalid value,
 * LY_SUCCESS - successful parsing.
 */
LY_ERR ly_parse_int(const char *val_str, size_t val_len, int64_t min, int64_t max, int base, int64_t *ret);

/**
 * @brief Parse unsigned integer with possible limitation.
 * @param[in] val_str String value containing unsigned integer, note that
 * nothing else than whitespaces are expected after the value itself.
 * @param[in] val_len Length of the @p val_str string.
 * @param[in] max Limitation for the value which must not be higher than max.
 * @param[in] base Numeric base for parsing:
 *        0 - to accept decimal, octal, hexadecimal (e.g. in default value)
 *       10 - to accept only decimal (e.g. data instance value)
 * @param[out] ret Resulting value.
 * @return LY_ERR value:
 * LY_EDENIED - the value breaks the limits,
 * LY_EVALID - string contains invalid value,
 * LY_SUCCESS - successful parsing.
 */
LY_ERR ly_parse_uint(const char *val_str, size_t val_len, uint64_t max, int base, uint64_t *ret);

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
LY_ERR ly_parse_nodeid(const char **id, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len);

/**
 * @brief parse instance-identifier's predicate, supports key-predicate, leaf-list-predicate and pos rules from YANG ABNF Grammar.
 *
 * @param[in, out] pred Predicate string (including the leading '[') to parse. The string is updated according to what was parsed
 * (even for error case, so it can be used to determine which substring caused failure).
 * @param[in] limit Limiting length of the @p pred. Function expects NULL terminated string which is not overread.
 * The limit value is not checked with each character, so it can be overread and the failure is detected later.
 * @param[in] format Input format of the data containing the @p pred.
 * @param[out] prefix Start of the node-identifier's prefix if any, NULL in case of pos or leaf-list-predicate rules.
 * @param[out] prefix_len Length of the parsed @p prefix.
 * @param[out] id Start of the node-identifier's identifier string, NULL in case of pos rule, "." in case of leaf-list-predicate rule.
 * @param[out] id_len Length of the parsed @p id.
 * @param[out] value Start of the quoted-string (without quotation marks), not NULL in case of success.
 * @param[out] value_len Length of the parsed @p value.
 * @param[out] errmsg Error message string in case of error.
 * @return LY_SUCCESS in case a complete predicate was parsed.
 * @return LY_EVALID in case of invalid predicate form.
 * @return LY_EINVAL in case of reaching @p limit when parsing @p pred.
 */
LY_ERR ly_parse_instance_predicate(const char **pred, size_t limit, LYD_FORMAT format,
                                   const char **prefix, size_t *prefix_len, const char **id, size_t *id_len,
                                   const char **value, size_t *value_len, const char **errmsg);

/**
 * @brief ly_clb_get_prefix implementation for JSON. For its simplicity, this implementation is used
 * internally for various purposes.
 *
 * Implemented in printer_json.c
 */
const char *json_print_get_prefix(const struct lys_module *mod, void *private);

/**
 * @brief mmap(2) wrapper to map input files into memory to unify parsing.
 *
 * The address space is allocate only for reading.
 *
 * @param[in] ctx libyang context for logging
 * @param[in] fd Open file descriptor of a file to map.
 * @param[out] length Allocated size.
 * @param[out] addr Address where the file is mapped.
 * @return LY_ERR value.
 */
LY_ERR ly_mmap(struct ly_ctx *ctx, int fd, size_t *length, void **addr);

/**
 * @brief munmap(2) wrapper to free the memory mapped by ly_mmap()
 *
 * @param[in] addr Address where the input file is mapped.
 * @param[in] length Allocated size of the address space.
 * @return LY_ERR value.
 */
LY_ERR ly_munmap(void *addr, size_t length);

/**
 * @brief Concatenate formating string to the @p dest.
 *
 * @param[in, out] dest String to be concatenated by @p format.
 *                 Note that the input string can be reallocated during concatenation.
 * @param[in] format Formating string (as for printf) which is supposed to be added after @p dest.
 * @return LY_SUCCESS or LY_EMEM.
 */
LY_ERR ly_strcat(char **dest, const char *format, ...);

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
            ARRAY = malloc(sizeof(uint32_t) + sizeof *(ARRAY)); \
            *((uint32_t*)(ARRAY)) = 1; \
        } else { \
            ++(*((uint32_t*)(ARRAY) - 1)); \
            ARRAY = ly_realloc(((uint32_t*)(ARRAY) - 1), sizeof(uint32_t) + (*((uint32_t*)(ARRAY) - 1) * sizeof *(ARRAY))); \
            LY_CHECK_ERR_RET(!(ARRAY), LOGMEM(CTX), RETVAL); \
        } \
        ARRAY = (void*)((uint32_t*)(ARRAY) + 1); \
        (NEW_ITEM) = &(ARRAY)[*((uint32_t*)(ARRAY) - 1) - 1]; \
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
            ARRAY = malloc(sizeof(uint32_t) + sizeof *(ARRAY)); \
            *((uint32_t*)(ARRAY)) = 1; \
        } else { \
            ++(*((uint32_t*)(ARRAY) - 1)); \
            ARRAY = ly_realloc(((uint32_t*)(ARRAY) - 1), sizeof(uint32_t) + (*((uint32_t*)(ARRAY) - 1) * sizeof *(ARRAY))); \
            LY_CHECK_ERR_GOTO(!(ARRAY), LOGMEM(CTX); RET = LY_EMEM, GOTO); \
        } \
        ARRAY = (void*)((uint32_t*)(ARRAY) + 1); \
        (NEW_ITEM) = &(ARRAY)[*((uint32_t*)(ARRAY) - 1) - 1]; \
        memset(NEW_ITEM, 0, sizeof *(NEW_ITEM))

/**
 * @brief Allocate a ([sized array](@ref sizedarrays)) for the specified number of items.
 * If the ARRAY already exists, it is resized (space for SIZE items is added).
 *
 * Does not set the size information, it is supposed to be incremented via ::LY_ARRAY_INCREMENT
 * when the items are filled.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to create.
 * @param[in] SIZE Number of items the array is supposed to hold. The size of the allocated
 * space is then counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[in] RETVAL Return value for the case of error (memory allocation failure).
 */
#define LY_ARRAY_CREATE_RET(CTX, ARRAY, SIZE, RETVAL) \
        if (ARRAY) { \
            ARRAY = ly_realloc(((uint32_t*)(ARRAY) - 1), sizeof(uint32_t) + ((*((uint32_t*)(ARRAY) - 1) + SIZE) * sizeof *(ARRAY))); \
            LY_CHECK_ERR_RET(!(ARRAY), LOGMEM(CTX), RETVAL); \
            ARRAY = (void*)((uint32_t*)(ARRAY) + 1); \
            memset(&(ARRAY)[*((uint32_t*)(ARRAY) - 1)], 0, SIZE * sizeof *(ARRAY)); \
        } else { \
            ARRAY = calloc(1, sizeof(uint32_t) + SIZE * sizeof *(ARRAY)); \
            LY_CHECK_ERR_RET(!(ARRAY), LOGMEM(CTX), RETVAL); \
            ARRAY = (void*)((uint32_t*)(ARRAY) + 1); \
        }

/**
 * @brief Allocate a ([sized array](@ref sizedarrays)) for the specified number of items.
 * If the ARRAY already exists, it is resized (space for SIZE items is added).
 *
 * Does not set the size information, it is supposed to be incremented via ::LY_ARRAY_INCREMENT
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
            ARRAY = ly_realloc(((uint32_t*)(ARRAY) - 1), sizeof(uint32_t) + ((*((uint32_t*)(ARRAY) - 1) + (SIZE)) * sizeof *(ARRAY))); \
            LY_CHECK_ERR_GOTO(!(ARRAY), LOGMEM(CTX); RET = LY_EMEM, GOTO); \
            ARRAY = (void*)((uint32_t*)(ARRAY) + 1); \
            memset(&(ARRAY)[*((uint32_t*)(ARRAY) - 1)], 0, (SIZE) * sizeof *(ARRAY)); \
        } else { \
            ARRAY = calloc(1, sizeof(uint32_t) + (SIZE) * sizeof *(ARRAY)); \
            LY_CHECK_ERR_GOTO(!(ARRAY), LOGMEM(CTX); RET = LY_EMEM, GOTO); \
            ARRAY = (void*)((uint32_t*)(ARRAY) + 1); \
        }

#define LY_ARRAY_INCREMENT(ARRAY) \
        ++(*((uint32_t*)(ARRAY) - 1))

#define LY_ARRAY_DECREMENT(ARRAY) \
        --(*((uint32_t*)(ARRAY) - 1))

/**
 * @brief Free the space allocated for the ([sized array](@ref sizedarrays)).
 *
 * The items inside the array are not freed.
 *
 * @param[in] ARRAY A ([sized array](@ref sizedarrays)) to be freed.
 */
#define LY_ARRAY_FREE(ARRAY) \
        if (ARRAY){free((uint32_t*)(ARRAY) - 1);}

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
            for (iterator = *(LIST); iterator->LINKER; iterator = iterator->LINKER); \
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
    LY_CHECK_ERR_RET(!(NEW_ITEM), LOGMEM(CTX), RETVAL); \
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
    LY_CHECK_ERR_GOTO(!(NEW_ITEM), RET = LY_EMEM; LOGMEM(CTX), LABEL); \
    LY_LIST_INSERT(LIST, NEW_ITEM, LINKER)

#endif /* LY_COMMON_H_ */
