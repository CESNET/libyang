/**
 * @file log.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang logger implementation
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

#define _BSD_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>

#include "common.h"

volatile uint8_t ly_log_level = LY_LLERR;

API void
ly_verb(LY_LOG_LEVEL level)
{
    ly_log_level = level;
}

static void
log_vprintf(LY_LOG_LEVEL level, const char *format, va_list args)
{
#define PRV_MSG_SIZE 4096
    char prv_msg[PRV_MSG_SIZE];

    vsnprintf(prv_msg, PRV_MSG_SIZE - 1, format, args);
    prv_msg[PRV_MSG_SIZE - 1] = '\0';
    fprintf(stderr, "libyang[%d]: %s\n", level, prv_msg);
#undef PRV_MSG_SIZE
}

void
ly_log(LY_LOG_LEVEL level, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    log_vprintf(level, format, ap);
    va_end(ap);
}

const char *ly_errs[] = {
/* LYE_XML_MISS */     "Missing %s \"%s\".",
/* LYE_XML_INVAL */    "Invalid %s.",
/* LYE_XML_INCHAR */   "Encountered invalid character sequence \"%.10s\".",

/* LYE_EOF */          "Unexpected end of input data.",
/* LYE_INSTMT */       "Invalid keyword \"%s\".",
/* LYE_INID */         "Invalid identifier \"%s\" (%s).",
/* LYE_INDATE */       "Invalid date format of \"%s\", \"YYYY-MM-DD\" expected.",
/* LYE_INARG */        "Invalid value \"%s\" of \"%s\".",
/* LYE_MISSSTMT1 */    "Missing keyword \"%s\".",
/* LYE_MISSSTMT2 */    "Missing keyword \"%s\" as child to \"%s\".",
/* LYE_MISSARG */      "Missing argument \"%s\" to keyword \"%s\".",
/* LYE_TOOMANY */      "Too many instances of \"%s\" in \"%s\".",
/* LYE_DUPID */        "Duplicated %s identifier \"%s\".",
/* LYE_DUPLEAFLIST */  "Instances of \"%s\" leaf-list are not unique (\"%s\").",
/* LYE_DUPLIST */      "Instances of \"%s\" list are not unique.",
/* LYE_ENUM_DUPVAL */  "The value \"%d\" of \"%s\" enum has already been assigned to another enum value.",
/* LYE_ENUM_DUPNAME */ "The enum name \"%s\" has already been assigned to another enum.",
/* LYE_ENUM_WS */      "The enum name \"%s\" includes invalid leading or trailing whitespaces.",
/* LYE_BITS_DUPVAL */  "The position \"%d\" of \"%s\" bits has already been used to another named bit.",
/* LYE_BITS_DUPNAME */ "The bit name \"%s\" has already been assigned to another bit.",
/* LYE_INMOD */        "Module name in \"%s\" refers to an unknown module.",
/* LYE_INMOD_LEN */    "Module name \"%.*s\" refers to an unknown module.",
/* LYE_KEY_NLEAF */    "Key \"%s\" is not a leaf.",
/* LYE_KEY_TYPE */     "Key \"%s\" must not be the built-in type \"empty\".",
/* LYE_KEY_CONFIG */   "The \"config\" value of the \"%s\" key differs from its list config value.",
/* LYE_KEY_MISS */     "Leaf \"%s\" defined as key in a list not found.",
/* LYE_KEY_DUP */      "Key identifier \"%s\" is not unique.",
/* LYE_INREGEX */      "Regular expression \"%s\" is not valid (%s).",
/* LYE_INRESOLV */     "Failed to resolve %s \"%s\".",

/* LYE_NORESOLV */     "No resolvents found for \"%s\".",
/* LYE_INELEM */       "Unknown element \"%s\".",
/* LYE_INELEM_LEN */   "Unknown element \"%.*s\".",
/* LYE_MISSELEM */     "Missing required element \"%s\" in \"%s\".",
/* LYE_INVAL */        "Invalid value \"%s\" in \"%s\" element.",
/* LYE_INATTR */       "Invalid attribute \"%s\" in \"%s\" element.",
/* LYE_MISSATTR */     "Missing attribute \"%s\" in \"%s\" element.",
/* LYE_OORVAL */       "Value \"%s\" is out of range or length.",
/* LYE_INCHAR */       "Unexpected character(s) '%c' (%.15s).",
/* LYE_INPRED */       "Predicate resolution failed on \"%s\".",
/* LYE_MCASEDATA */    "Data for more than one case branch of \"%s\" choice present.",
/* LYE_NOCOND */       "%s condition \"%s\" not satisfied.",

/* LYE_XPATH_INTOK */  "Unexpected XPath token %s (%.15s).",
/* LYE_XPATH_EOF */    "Unexpected XPath expression end.",
/* LYE_XPATH_INOP_1 */ "Cannot apply XPath operation %s on %s.",
/* LYE_XPATH_INOP_2 */ "Cannot apply XPath operation %s on %s and %s.",
/* LYE_XPATH_INCTX */  "Invalid context type %s in %s.",
/* LYE_XPATH_INARGCOUNT */ "Invalid number of arguments (%d) for the XPath function %s.",
/* LYE_XPATH_INARGTYPE */ "Wrong type of argument #%d (%s) for the XPath function %s."
};

void
ly_vlog(enum LY_ERR code, uint32_t line, ...)
{
    va_list ap;
    const char *fmt;

    if (line == UINT_MAX) {
        return;
    }

    ly_errno = LY_EVALID;
    if (line) {
        fprintf(stderr, "libyang[%d]: Parser fails around the line %u.\n", LY_LLERR, line);
    }

    if (code == LYE_LINE) {
        return;
    }

    va_start(ap, line);
    if (code == LYE_SPEC) {
        fmt = va_arg(ap, char *);
        log_vprintf(LY_LLERR, fmt, ap);
    } else {
        log_vprintf(LY_LLERR, ly_errs[code], ap);
    }
    va_end(ap);
}
