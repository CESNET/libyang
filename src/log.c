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

volatile uint8_t ly_verb_level = LY_VERB_ERR;

API void ly_verbosity(LY_VERB_LEVEL level)
{
	ly_verb_level = level;
}

static void log_vprintf(LY_VERB_LEVEL level, const char *format, va_list args)
{
#define PRV_MSG_SIZE 4096
	char prv_msg[PRV_MSG_SIZE];

	vsnprintf(prv_msg, PRV_MSG_SIZE - 1, format, args);
	prv_msg[PRV_MSG_SIZE - 1] = '\0';
	fprintf(stderr, "libyang[%d]: %s\n", level, prv_msg);
#undef PRV_MSG_SIZE
}

void ly_log(LY_VERB_LEVEL level, int errno_, const char *format, ...)
{
	va_list ap;

	if (level == LY_VERB_ERR) {
		ly_errno = errno_;
	}

	va_start(ap, format);
	log_vprintf(level, format, ap);
	va_end(ap);
}

struct {
	LY_VERB_LEVEL level; /* verbose level */
	const char *fmt;     /* format string to be printed */
} verrs[] = {
		{LY_VERB_ERR,    /* LY_VERR_UNEXP_STMT */
		"Unexpected keyword \"%s\"."},
		{LY_VERB_ERR,    /* LY_VERR_MISS_STMT1 */
		"Missing keyword \"%s\"."},
		{LY_VERB_ERR,    /* LY_VERR_MISS_STMT2 */
		"Missing keyword \"%s\" as child to \"%s\"."},
		{LY_VERB_ERR,    /* LY_VERR_MISS_ARG */
		"Missing argument \"%s\" to keyword \"%s\"."},
		{LY_VERB_ERR,    /* LY_VERR_TOOMANY */
		"Too many instances of \"%s\" in \"%s\"."},
		{LY_VERB_ERR,    /* LY_VERR_UNEXP_ARG */
		"Unexpected value \"%s\" of \"%s\"."},
		{LY_VERB_ERR,    /* LY_VERR_BAD_RESTR */
		"Restriction \"%s\" not allowed for this base type."},
		{LY_VERB_ERR,    /* LY_VERR_ENUM_DUP_VAL */
		"The value \"%d\" of \"%s\" enum has already been assigned to another enum value."},
		{LY_VERB_ERR,    /* LY_VERR_ENUM_DUP_NAME */
		"The enum name \"%s\" has already been assigned to another enum."},
		{LY_VERB_ERR,    /* LY_VERR_ENUM_WS */
		"The enum name \"%s\" includes invalid leading or trailing whitespaces."},
		{LY_VERB_ERR,    /* LY_VERR_UNEXP_PREFIX */
		"Prefix in \"%s\" refers to an unknown module."}
};

void ly_verr(enum LY_VERR code, ...)
{
	va_list ap;

	if (verrs[code].level == LY_VERB_ERR) {
		ly_errno = LY_EVALID;
	}

	va_start(ap, code);
	log_vprintf(verrs[code].level, verrs[code].fmt, ap);
	va_end(ap);
}
