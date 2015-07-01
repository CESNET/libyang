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
