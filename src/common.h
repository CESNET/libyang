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

/* internal logging options */
enum int_log_opts {
    ILO_LOG = 0, /* log normally */
    ILO_STORE,   /* only store any messages, they will be processed higher on stack */
    ILO_IGNORE,  /* completely ignore messages */
    ILO_ERR2WRN, /* change errors to warnings */
};

enum LY_VLOG_ELEM {
    LY_VLOG_NONE = 0,
    LY_VLOG_LINE,/* line number */
    LY_VLOG_LYS, /* struct lysc_node* */
    LY_VLOG_LYD, /* struct lyd_node* */
    LY_VLOG_STR, /* const char* */
    LY_VLOG_PREV /* use exact same previous path */
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
#define LY_CHECK_ARG_GOTO(CTX, ...) GETMACRO4(__VA_ARGS__, LY_CHECK_ARG_GOTO3, LY_CHECK_ARG_GOTO2, LY_CHECK_ARG_GOTO1)(CTX, __VA_ARGS__)

#define LY_CHECK_ARG_RET1(CTX, ARG, RETVAL) if (!(ARG)) {LOGARG(CTX, ARG);return RETVAL;}
#define LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL) LY_CHECK_ARG_RET1(CTX, ARG1, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG2, RETVAL)
#define LY_CHECK_ARG_RET3(CTX, ARG1, ARG2, ARG3, RETVAL) LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG3, RETVAL)
#define LY_CHECK_ARG_RET(CTX, ...) GETMACRO4(__VA_ARGS__, LY_CHECK_ARG_RET3, LY_CHECK_ARG_RET2, LY_CHECK_ARG_RET1)(CTX, __VA_ARGS__)

/* count sequence size for LY_VCODE_INCHILDSTMT validation error code */
size_t LY_VCODE_INSTREXP_len(const char *str);
/* default maximum characters to print in LY_VCODE_INCHILDSTMT */
#define LY_VCODE_INSTREXP_MAXLEN 20

#define LY_VCODE_INCHAR      LYVE_SYNTAX, "Invalid character 0x%x."
#define LY_VCODE_INSTREXP    LYVE_SYNTAX, "Invalid character sequence \"%.*s\", expected %s."
#define LY_VCODE_EOF         LYVE_SYNTAX, "Unexpected end-of-input."
#define LY_VCODE_NTERM       LYVE_SYNTAX, "%s not terminated."
#define LY_VCODE_NSUPP       LYVE_SYNTAX, "%s not supported."
#define LY_VCODE_INSTMT      LYVE_SYNTAX_YANG, "Invalid keyword \"%s\"."
#define LY_VCODE_INCHILDSTMT LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s\"."
#define LY_VCODE_INCHILDSTMT2 LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s\" - the statement is allowed only in YANG 1.1 modules."
#define LY_VCODE_INCHILDSTMSCOMB LYVE_SYNTAX_YANG, "Invalid combination of keywords \"%s\" and \"%s\" as substatements of \"%s\"."
#define LY_VCODE_DUPSTMT     LYVE_SYNTAX_YANG, "Duplicate keyword \"%s\"."
#define LY_VCODE_DUPIDENT    LYVE_SYNTAX_YANG, "Duplicate identifier \"%s\" of %s statement."
#define LY_VCODE_INVAL       LYVE_SYNTAX_YANG, "Invalid value \"%.*s\" of \"%s\"."
#define LY_VCODE_MISSTMT     LYVE_SYNTAX_YANG, "Missing mandatory keyword \"%s\" as a child of \"%s\"."
#define LY_VCODE_MISSCHILDSTMT LYVE_SYNTAX_YANG, "Missing %s substatement for %s%s."
#define LY_VCODE_INORD       LYVE_SYNTAX_YANG, "Invalid keyword \"%s\", it cannot appear after \"%s\"."
#define LY_VCODE_OOB         LYVE_SYNTAX_YANG, "Value \"%.*s\" is out of \"%s\" bounds."
#define LY_VCODE_INDEV       LYVE_SYNTAX_YANG, "Deviate \"%s\" does not support keyword \"%s\"."
#define LY_VCODE_INREGEXP    LYVE_SYNTAX_YANG, "Regular expression \"%s\" is not valid (\"%s\": %s)."
#define LY_VCODE_XP_EOE      LYVE_XPATH, "Unterminated string delimited with %c (%.15s)."
#define LY_VCODE_XP_INEXPR   LYVE_XPATH, "Invalid character number %u of expression \'%s\'."
#define LY_VCODE_DEV_NODETYPE LYVE_REFERENCE, "Invalid deviation (%s) of %s node - it is not possible to %s \"%s\" property."
#define LY_VCODE_DEV_NOT_PRESENT LYVE_REFERENCE, "Invalid deviation (%s) %s \"%s\" property \"%s\" which is not present."

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

enum yang_keyword {
    YANG_NONE = 0,
    YANG_ACTION,
    YANG_ANYDATA,
    YANG_ANYXML,
    YANG_ARGUMENT,
    YANG_AUGMENT,
    YANG_BASE,
    YANG_BELONGS_TO,
    YANG_BIT,
    YANG_CASE,
    YANG_CHOICE,
    YANG_CONFIG,
    YANG_CONTACT,
    YANG_CONTAINER,
    YANG_DEFAULT,
    YANG_DESCRIPTION,
    YANG_DEVIATE,
    YANG_DEVIATION,
    YANG_ENUM,
    YANG_ERROR_APP_TAG,
    YANG_ERROR_MESSAGE,
    YANG_EXTENSION,
    YANG_FEATURE,
    YANG_FRACTION_DIGITS,
    YANG_GROUPING,
    YANG_IDENTITY,
    YANG_IF_FEATURE,
    YANG_IMPORT,
    YANG_INCLUDE,
    YANG_INPUT,
    YANG_KEY,
    YANG_LEAF,
    YANG_LEAF_LIST,
    YANG_LENGTH,
    YANG_LIST,
    YANG_MANDATORY,
    YANG_MAX_ELEMENTS,
    YANG_MIN_ELEMENTS,
    YANG_MODIFIER,
    YANG_MODULE,
    YANG_MUST,
    YANG_NAMESPACE,
    YANG_NOTIFICATION,
    YANG_ORDERED_BY,
    YANG_ORGANIZATION,
    YANG_OUTPUT,
    YANG_PATH,
    YANG_PATTERN,
    YANG_POSITION,
    YANG_PREFIX,
    YANG_PRESENCE,
    YANG_RANGE,
    YANG_REFERENCE,
    YANG_REFINE,
    YANG_REQUIRE_INSTANCE,
    YANG_REVISION,
    YANG_REVISION_DATE,
    YANG_RPC,
    YANG_STATUS,
    YANG_SUBMODULE,
    YANG_TYPE,
    YANG_TYPEDEF,
    YANG_UNIQUE,
    YANG_UNITS,
    YANG_USES,
    YANG_VALUE,
    YANG_WHEN,
    YANG_YANG_VERSION,
    YANG_YIN_ELEMENT,

    YANG_SEMICOLON,
    YANG_LEFT_BRACE,
    YANG_RIGHT_BRACE,
    YANG_CUSTOM
};

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
 * @brief Get UTF8 code point of the next character in the input string.
 *
 * @param[in,out] input Input string to process, updated according to the processed/read data.
 * @param[out] utf8_char UTF8 code point of the next character.
 * @param[out] bytes_read Number of bytes used to encode the read utf8_char.
 * @return LY_ERR value
 */
LY_ERR ly_getutf8(const char **input, unsigned int *utf8_char, size_t *bytes_read);

/**
 * @brief Parse signed integer with possible limitation.
 * @param[in] val_str String value containing signed integer, note that
 * nothing else than whitespaces are expected after the value itself.
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
LY_ERR ly_parse_int(const char *val_str, int64_t min, int64_t max, int base, int64_t *ret);

/**
 * @brief Parse unsigned integer with possible limitation.
 * @param[in] val_str String value containing unsigned integer, note that
 * nothing else than whitespaces are expected after the value itself.
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
LY_ERR ly_parse_uint(const char *val_str, uint64_t max, int base, uint64_t *ret);

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
#endif /* LY_COMMON_H_ */
