/**
 * @file common.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common internal definitions for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#ifndef LY_COMMON_H_
#define LY_COMMON_H_

#include <stdint.h>

#include "libyang.h"

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

/*
 * If the compiler supports attribute to mark objects as hidden, mark all
 * objects as hidden and export only objects explicitly marked to be part of
 * the public API.
 */
#define API __attribute__((visibility("default")))

/*
 * logger
 */
extern volatile uint8_t ly_log_level;
void ly_log(LY_LOG_LEVEL level, const char *format, ...);

#define LOGERR(errno, str, args...)                                 \
	ly_errno = errno;                                               \
	ly_log(LY_LLERR, str, ##args)

#define LOGWRN(str, args...)                                        \
	if (ly_log_level >= LY_LLWRN) {                                 \
		ly_log(LY_LLWRN, str, ##args);                              \
	}

#define LOGVRB(str, args...)                                        \
	if (ly_log_level >= LY_LLVRB) {                                 \
		ly_log(LY_LLVRB, str, ##args);                              \
	}

#ifdef NDEBUG
#define LOGDBG(str, args...)
#else
#define LOGDBG(str, args...)                                        \
	if (ly_log_level >= LY_LLDBG) {                                 \
		ly_log(LY_LLDBG, str, ##args);                              \
	}
#endif

#define LOGMEM LOGERR(LY_EMEM, "Memory allocation failed (%s())", __func__)

enum LY_VERR {
    VE_SPEC = -1,

    VE_XML_MISS,
    VE_XML_INVAL,

    VE_EOF,
    VE_INSTMT,
    VE_INID,
    VE_INDATE,
    VE_INARG,
    VE_MISSSTMT1,
    VE_MISSSTMT2,
    VE_MISSARG,
    VE_TOOMANY,
    VE_DUPID,
    VE_ENUM_DUPVAL,
    VE_ENUM_DUPNAME,
    VE_ENUM_WS,
    VE_BITS_DUPVAL,
    VE_BITS_DUPNAME,
    VE_INPREFIX,
    VE_KEY_NLEAF,
    VE_KEY_TYPE,
    VE_KEY_CONFIG,
    VE_KEY_MISS,
    VE_KEY_DUP,
    VE_INREGEX,

    DE_INELEM,
    DE_INVAL,
    DE_OORVAL
};
void ly_vlog(enum LY_VERR code, unsigned int line, ...);

#define LOGVAL(code, line, args...) ly_vlog(code, line, ##args)

#ifdef NDEBUG
#    define LOGLINE(node) 0
#else
#    define LOGLINE(node) node->line
#endif

char *strnchr(const char *s, int c, unsigned int len);

const char *strnodetype(LY_NODE_TYPE type);

#endif /* LY_COMMON_H_ */
