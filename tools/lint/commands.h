/**
 * @file main.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool commands header
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

#ifndef COMMANDS_H_
#define COMMANDS_H_

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#include <stdlib.h>

#include "../../src/libyang.h"

#define PROMPT "> "

struct model_hint {
	char* hint;
	struct model_hint* next;
};

typedef struct {
	char *name; /* User printable name of the function. */
	int (*func)(const char*); /* Function to call to do the command. */
	void (*help_func)(void); /* Display command help. */
	char *helpstring; /* Documentation for this function. */
} COMMAND;

extern COMMAND commands[];

#endif /* COMMANDS_H_ */
