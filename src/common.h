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

#include "libyang.h"
#include "config.h"

#if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
# define THREAD_LOCAL _Thread_local
#elif defined __GNUC__ || \
      defined __SUNPRO_C || \
      defined __xlC__
# define THREAD_LOCAL __thread
#else
# error "Cannot define THREAD_LOCAL"
#endif

#define GETMACRO2(_1, _2, NAME, ...) NAME
#define GETMACRO3(_1, _2, _3, NAME, ...) NAME
#define GETMACRO4(_1, _2, _3, _4, NAME, ...) NAME

/*
 * logger
 */

/* internal logging options */
enum int_log_opts {
    ILO_LOG = 0, /* log normally */
    ILO_STORE,   /* only store any messages, they will be processed higher on stack */
    ILO_IGNORE,  /* completely ignore messages */
    ILO_ERR2WRN, /* change errors to warnings */
};

extern THREAD_LOCAL enum int_log_opts log_opt;
extern volatile uint8_t ly_log_level;
extern volatile uint8_t ly_log_opts;

void ly_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, const char *format, ...);

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

/*
 * Common code to check return value and perform appropriate action.
 */
#define LY_CHECK_GOTO(COND, GOTO) if (COND) {goto GOTO;}
#define LY_CHECK_ERR_GOTO(COND, ERR, GOTO) if (COND) {ERR; goto GOTO;}
#define LY_CHECK_RET(COND, RETVAL) if (COND) {return RETVAL;}
#define LY_CHECK_ERR_RET(COND, ERR, RETVAL) if (COND) {ERR; return RETVAL;}

#define LY_CHECK_ARG_GOTO1(CTX, ARG, GOTO) if (!ARG) {LOGARG(CTX, ARG);goto GOTO;}
#define LY_CHECK_ARG_GOTO2(CTX, ARG1, ARG2, GOTO) LY_CHECK_ARG_GOTO1(CTX, ARG1, GOTO);LY_CHECK_ARG_GOTO1(CTX, ARG2, GOTO)
#define LY_CHECK_ARG_GOTO3(CTX, ARG1, ARG2, ARG3, GOTO) LY_CHECK_ARG_GOTO2(CTX, ARG1, ARG2, GOTO);LY_CHECK_ARG_GOTO1(CTX, ARG3, GOTO)
#define LY_CHECK_ARG_GOTO(CTX, ...) GETMACRO4(__VA_ARGS__, LY_CHECK_ARG_GOTO3, LY_CHECK_ARG_GOTO2, LY_CHECK_ARG_GOTO1)(CTX, __VA_ARGS__)

#define LY_CHECK_ARG_RET1(CTX, ARG, RETVAL) if (!ARG) {LOGARG(CTX, ARG);return RETVAL;}
#define LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL) LY_CHECK_ARG_RET1(CTX, ARG1, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG2, RETVAL)
#define LY_CHECK_ARG_RET3(CTX, ARG1, ARG2, ARG3, RETVAL) LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG3, RETVAL)
#define LY_CHECK_ARG_RET(CTX, ...) GETMACRO4(__VA_ARGS__, LY_CHECK_ARG_RET3, LY_CHECK_ARG_RET2, LY_CHECK_ARG_RET1)(CTX, __VA_ARGS__)

/*
 * If the compiler supports attribute to mark objects as hidden, mark all
 * objects as hidden and export only objects explicitly marked to be part of
 * the public API.
 */
#define API __attribute__((visibility("default")))

#endif /* LY_COMMON_H_ */
