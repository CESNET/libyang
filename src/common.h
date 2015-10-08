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
#include "resolve.h"

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

#define LOGMEM LOGERR(LY_EMEM, "Memory allocation failed (%s()).", __func__)

#define LOGINT LOGERR(LY_EINT, "Internal error (%s:%d).", __FILE__, __LINE__)

enum LY_ERR {
    LYE_SPEC = -2,
    LYE_LINE = -1,

    LYE_XML_MISS,
    LYE_XML_INVAL,
    LYE_XML_INCHAR,

    LYE_EOF,
    LYE_INSTMT,
    LYE_INID,
    LYE_INDATE,
    LYE_INARG,
    LYE_MISSSTMT1,
    LYE_MISSSTMT2,
    LYE_MISSARG,
    LYE_TOOMANY,
    LYE_DUPID,
    LYE_DUPLEAFLIST,
    LYE_DUPLIST,
    LYE_ENUM_DUPVAL,
    LYE_ENUM_DUPNAME,
    LYE_ENUM_WS,
    LYE_BITS_DUPVAL,
    LYE_BITS_DUPNAME,
    LYE_INPREF,
    LYE_INPREF_LEN,
    LYE_KEY_NLEAF,
    LYE_KEY_TYPE,
    LYE_KEY_CONFIG,
    LYE_KEY_MISS,
    LYE_KEY_DUP,
    LYE_INREGEX,
    LYE_INRESOLV,

    LYE_NORESOLV,
    LYE_INELEM,
    LYE_INELEM_LEN,
    LYE_MISSELEM,
    LYE_INVAL,
    LYE_INATTR,
    LYE_MISSATTR,
    LYE_OORVAL,
    LYE_INCHAR,
    LYE_INPRED,
    LYE_MCASEDATA,
    LYE_NOCOND,

    LYE_XPATH_INTOK,
    LYE_XPATH_EOF,
    LYE_XPATH_INOP_1,
    LYE_XPATH_INOP_2,
    LYE_XPATH_INCTX,
    LYE_XPATH_INARGCOUNT,
    LYE_XPATH_INARGTYPE
};
void ly_vlog(enum LY_ERR code, unsigned int line, ...);

#define LOGVAL(code, line, args...) ly_vlog(code, line, ##args)

#ifdef NDEBUG
#    define LOGLINE(node) 0
#    define LOGLINE_IDX(node, idx) 0
#else
#    define LOGLINE(node) (node)->line
#    define LOGLINE_IDX(node, idx) (node)->line[idx]
#endif

/**
 * @brief Basic functionality like strpbrk(3). However, it searches string \p s
 *        backwards up to most \p s_len characters.
 *
 * @param[in] s String to search backwards.
 * @param[in] accept String of characters that are searched for.
 * @param[in] s_len Backward length of \p s.
 *
 * @return Pointer to the first backward occurence of a character from
 *         \p accept or \p s - \p s_len if not found.
 */
const char *strpbrk_backwards(const char *s, const char *accept, unsigned int s_len);

char *strnchr(const char *s, int c, unsigned int len);

const char *strnodetype(LYS_NODE type);

#endif /* LY_COMMON_H_ */
