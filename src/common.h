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

#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "log.h"
#include "tree_schema.h"

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

void ly_err_free(void *ptr);
void ly_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, const char *format, ...);
void ly_vlog(const struct ly_ctx *ctx, enum LY_VLOG_ELEM elem_type, const void *elem, LY_VECODE code, const char *format, ...);

#define LOGERR(ctx, errno, str, args...) ly_log(ctx, LY_LLERR, errno, str, ##args)
#define LOGWRN(ctx, str, args...) ly_log(ctx, LY_LLWRN, 0, str, ##args)
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
#define LY_CHECK_GOTO(COND, GOTO) if (COND) {goto GOTO;}
#define LY_CHECK_ERR_GOTO(COND, ERR, GOTO) if (COND) {ERR; goto GOTO;}
#define LY_CHECK_RET1(RETVAL) if (RETVAL != LY_SUCCESS) {return RETVAL;}
#define LY_CHECK_RET2(COND, RETVAL) if (COND) {return RETVAL;}
#define LY_CHECK_RET(...) GETMACRO2(__VA_ARGS__, LY_CHECK_RET2, LY_CHECK_RET1)(__VA_ARGS__)
#define LY_CHECK_ERR_RET(COND, ERR, RETVAL) if (COND) {ERR; return RETVAL;}

#define LY_CHECK_ARG_GOTO1(CTX, ARG, GOTO) if (!ARG) {LOGARG(CTX, ARG);goto GOTO;}
#define LY_CHECK_ARG_GOTO2(CTX, ARG1, ARG2, GOTO) LY_CHECK_ARG_GOTO1(CTX, ARG1, GOTO);LY_CHECK_ARG_GOTO1(CTX, ARG2, GOTO)
#define LY_CHECK_ARG_GOTO3(CTX, ARG1, ARG2, ARG3, GOTO) LY_CHECK_ARG_GOTO2(CTX, ARG1, ARG2, GOTO);LY_CHECK_ARG_GOTO1(CTX, ARG3, GOTO)
#define LY_CHECK_ARG_GOTO(CTX, ...) GETMACRO4(__VA_ARGS__, LY_CHECK_ARG_GOTO3, LY_CHECK_ARG_GOTO2, LY_CHECK_ARG_GOTO1)(CTX, __VA_ARGS__)

#define LY_CHECK_ARG_RET1(CTX, ARG, RETVAL) if (!ARG) {LOGARG(CTX, ARG);return RETVAL;}
#define LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL) LY_CHECK_ARG_RET1(CTX, ARG1, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG2, RETVAL)
#define LY_CHECK_ARG_RET3(CTX, ARG1, ARG2, ARG3, RETVAL) LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG3, RETVAL)
#define LY_CHECK_ARG_RET(CTX, ...) GETMACRO4(__VA_ARGS__, LY_CHECK_ARG_RET3, LY_CHECK_ARG_RET2, LY_CHECK_ARG_RET1)(CTX, __VA_ARGS__)

/* count sequence size for LY_VCODE_INCHILDSTMT validation error code */
size_t LY_VCODE_INSTREXP_len(const char *str);
/* default maximum characters to print in LY_VCODE_INCHILDSTMT */
#define LY_VCODE_INSTREXP_MAXLEN 20

#define LY_VCODE_INCHAR      LYVE_SYNTAX, "Invalid character 0x%x."
#define LY_VCODE_INSTREXP    LYVE_SYNTAX, "Invalid character sequence \"%.*s\", expected %s."
#define LY_VCODE_EOF         LYVE_SYNTAX, "Unexpected end-of-file."
#define LY_VCODE_NTERM       LYVE_SYNTAX, "%s not terminated."
#define LY_VCODE_NSUPP       LYVE_SYNTAX, "%s not supported."
#define LY_VCODE_INSTMT      LYVE_SYNTAX_YANG, "Invalid keyword \"%s\"."
#define LY_VCODE_INCHILDSTMT LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s\"."
#define LY_VCODE_DUPSTMT     LYVE_SYNTAX_YANG, "Duplicate keyword \"%s\"."
#define LY_VCODE_INVAL       LYVE_SYNTAX_YANG, "Invalid value \"%.*s\" of \"%s\"."
#define LY_VCODE_MISSTMT     LYVE_SYNTAX_YANG, "Missing mandatory keyword \"%s\" as a child of \"%s\"."
#define LY_VCODE_INORD       LYVE_SYNTAX_YANG, "Invalid keyword \"%s\", it cannot appear after \"%s\"."
#define LY_VCODE_OOB         LYVE_SYNTAX_YANG, "Value \"%.*s\" is out of \"%s\" bounds."
#define LY_VCODE_INDEV       LYVE_SYNTAX_YANG, "Deviate \"%s\" does not support keyword \"%s\"."

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
 * @brief Check date string (4DIGIT "-" 2DIGIT "-" 2DIGIT)
 *
 * @param[in] ctx Context to store log message.
 * @param[in] date Date string to check (non-necessarily terminated by \0)
 * @param[in] date_len Length of the date string, 10 expected.
 * @param[in] stmt Statement name for error message.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_date(struct ly_ctx *ctx, const char *date, int date_len, const char *stmt);

/*
 * Macros to work with sized-arrays.
 *
 * There is a 32b unsigned size (number of items) of the array at its beginning.
 *
 */
#define LYSP_ARRAY_NEW_RET(CTX, ARRAY, NEW_ITEM, RETVAL) \
        if (!(ARRAY)) { \
            ARRAY = malloc(sizeof(uint32_t) + sizeof *(ARRAY)); \
            *((uint32_t*)(ARRAY)) = 1; \
        } else { \
            ++(*((uint32_t*)(ARRAY))); \
            ARRAY = ly_realloc(ARRAY, sizeof(uint32_t) + (*((uint32_t*)(ARRAY)) * sizeof *(ARRAY))); \
            LY_CHECK_ERR_RET(!(ARRAY), LOGMEM(CTX), RETVAL); \
        } \
        (NEW_ITEM) = (void*)((uint32_t*)((ARRAY) + *((uint32_t*)(ARRAY)) - 1) + 1); \
        memset(NEW_ITEM, 0, sizeof *(NEW_ITEM));

#endif /* LY_COMMON_H_ */
