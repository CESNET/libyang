/*
 * mreadline.c
 * Author Radek Krejci <rkrejci@cesnet.cz>
 * Author Michal Vasko <mvasko@cesnet.cz>
 *
 * Modification of the libreadline functions for NETCONF client.
 *
 * Copyright (C) 2015 CESNET, z.s.p.o.
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
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is, and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include "commands.h"

extern struct model_hint *model_hints;
volatile int multiline = 0;

/* Generator function for command completion.  STATE lets us know whether
 to start from scratch; without any state (i.e. STATE == 0), then we
 start at the top of the list. */
char *
cmd_generator(const char *text, int state)
{
	static int list_index, len;
	char *name;

	/* If this is a new word to complete, initialize now.  This includes
	 saving the length of TEXT for efficiency, and initializing the index
	 variable to 0. */
	if (!state) {
		list_index = 0;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the command list. */
	while ((name = commands[list_index].name) != NULL) {
		list_index++;

		if (strncmp(name, text, len) == 0) {
			return strdup(name);
		}
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

char *
model_generator(const char *text, int state)
{
	static int len;
	static struct model_hint *cur, *prev;

	if (model_hints == NULL) {
		return NULL;
	}

	/* If this is a new word to complete, initialize now.  This includes
	 saving the length of TEXT for efficiency, and initializing the index
	 variable to 0. */
	if (!state) {
		cur = model_hints;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the command list. */
	while (cur != NULL) {
		prev = cur;
		cur = cur->next;

		if (strncmp(prev->hint, text, len) == 0) {
			return strdup(prev->hint);
		}
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

/**
 * \brief Attempt to complete available program commands.
 *
 * Attempt to complete on the contents of #text. #start and #end bound the
 * region of rl_line_buffer that contains the word to complete. #text is the
 * word to complete.  We can use the entire contents of rl_line_buffer in case
 * we want to do some simple parsing.
 *
 * \return The array of matches, or NULL if there aren't any.
 */
char **
cmd_completion(const char *text, int start, int end)
{
	char **matches;

	matches = (char **)NULL;

	/* If this word is at the start of the line, then it is a command
	 to complete.  If it is the command remove, print or feature,
	 complete model name. Otherwise it is the name of a file in the current
	 directory. */
	if (start == 0) {
		matches = rl_completion_matches(text, cmd_generator);
	} else if (strcmp(rl_line_buffer, "remove ") == 0 || strcmp(rl_line_buffer, "print ") == 0 || strcmp(rl_line_buffer, "feature ") == 0 ||
			(rl_line_buffer[end-1] != ' ' && (strncmp(rl_line_buffer, "remove ", 7) == 0 || strncmp(rl_line_buffer, "print ", 6) == 0 ||
			strncmp(rl_line_buffer, "feature ", 8) == 0))) {
		matches = rl_completion_matches(text, model_generator);
	} else if (strcmp(rl_line_buffer, "help ") == 0 || (rl_line_buffer[end-1] != ' ' && strncmp(rl_line_buffer, "help ", 5) == 0)) {
		matches = rl_completion_matches(text, cmd_generator);
	}

	return matches;
}

int
my_bind_cr(int UNUSED(count), int UNUSED(key))
{
	if (multiline == 0) {
		rl_point = rl_end;
		rl_redisplay();
		rl_done = 1;
	}
	printf("\n");

	return 0;
}

int
my_bind_esc(int UNUSED(count), int UNUSED(key))
{
	if (multiline == 1) {
		rl_point = rl_end;
		rl_redisplay();
		rl_done = 1;
		printf("\n");
	}
	return 0;
}

/**
 * \brief Tell the GNU Readline library how to complete commands.
 *
 * We want to try to complete on command names if this is the first word in the
 * line, or on filenames if not.
 */
void
initialize_readline(void)
{
	/* Allow conditional parsing of the ~/.inputrc file. */
	rl_readline_name = "dev-datastores";

	/* Tell the completer that we want a crack first. */
	rl_attempted_completion_function = cmd_completion;

	rl_bind_key('\n', my_bind_cr);
	rl_bind_key('\r', my_bind_cr);
	rl_bind_key('D' - 64, my_bind_esc); /* Ctrl-D */
}

char *
mreadline(const char *prompt)
{
	char *ret;

	multiline = 1; /* enable multiline editing */
	ret = readline(prompt);
	multiline = 0; /* disable multiline editing */

	return ret;
}

