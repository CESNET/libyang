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

#include "common.h"

volatile uint8_t ly_log_level = LY_LLERR;

API void ly_verb(LY_LOG_LEVEL level)
{
	ly_log_level = level;
}

static void log_vprintf(LY_LOG_LEVEL level, const char *format, va_list args)
{
#define PRV_MSG_SIZE 4096
	char prv_msg[PRV_MSG_SIZE];

	vsnprintf(prv_msg, PRV_MSG_SIZE - 1, format, args);
	prv_msg[PRV_MSG_SIZE - 1] = '\0';
	fprintf(stderr, "libyang[%d]: %s\n", level, prv_msg);
#undef PRV_MSG_SIZE
}

void ly_log(LY_LOG_LEVEL level, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	log_vprintf(level, format, ap);
	va_end(ap);
}

const char *verrs[] = {
/* VE_XML_MISS */     "Missing %s \"%s\".",
/* VE_XML_INVAL */    "Invalid %s.",
/* VE_EOF */          "Unexpected end of input data.",
/* VE_INSTMT */       "Invalid keyword \"%s\".",
/* VE_INID */         "Invalid identifier \"%s\" (%s).",
/* VE_INDATE */       "Invalid date format of \"%s\", \"YYYY-MM-DD\" expected.",
/* VE_INARG */        "Invalid value \"%s\" of \"%s\".",
/* VE_MISSSTMT1 */    "Missing keyword \"%s\".",
/* VE_MISSSTMT2 */    "Missing keyword \"%s\" as child to \"%s\".",
/* VE_MISSARG */      "Missing argument \"%s\" to keyword \"%s\".",
/* VE_TOOMANY */      "Too many instances of \"%s\" in \"%s\".",
/* VE_DUPID */        "Duplicated %s identifier \"%s\".",
/* VE_ENUM_DUPVAL */  "The value \"%d\" of \"%s\" enum has already been assigned to another enum value.",
/* VE_ENUM_DUPNAME */ "The enum name \"%s\" has already been assigned to another enum.",
/* VE_ENUM_WS */      "The enum name \"%s\" includes invalid leading or trailing whitespaces.",
/* VE_INPREFIX */     "Prefix in \"%s\" refers to an unknown module.",
/* VE_KEY_NLEAF */    "Key \"%s\" in \"%s\" list is not a leaf.",
/* VE_KEY_TYPE */     "Key \"%s\" in \"%s\" list must not be the built-in type \"empty\".",
/* VE_KEY_CONFIG */   "The \"config\" value of the \"%s\" key differs from its \"%s\" list config value.",
/* VE_KEY_MISS */     "Leaf \"%s\" defined as key in a list not found.",
/* VE_KEY_DUP */      "Key identifier \"%s\" in the \"%s\" list is not unique."
};

void ly_vlog(enum LY_VERR code, unsigned int line, ...)
{
	va_list ap;
	const char *fmt;

	ly_errno = LY_EVALID;
	if (line) {
		fprintf(stderr, "libyang[%d]: Parser fails around line %u.\n", LY_LLERR, line);
	}

	va_start(ap, line);
	if (code == VE_SPEC) {
		fmt = va_arg(ap, char *);
		log_vprintf(LY_LLERR, fmt, ap);
	} else {
		log_vprintf(LY_LLERR, verrs[code], ap);
	}
	va_end(ap);
}
