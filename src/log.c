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
