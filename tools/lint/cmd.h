/**
 * @file cmd.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool commands header
 *
 * Copyright (c) 2015-2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "libyang.h"

/**
 * @brief command information
 */
typedef struct {
    char *name;                                      /* User printable name of the function. */

    void (*func)(struct ly_ctx **ctx, const char *); /* Function to call to do the command. */
    void (*help_func)(void);                         /* Display command help. */
    void (*free_func)(void);                         /* Freeing global variables allocated by the command. */
    char *helpstring;                                /* Documentation for this function. */
    char *optstring;                                 /* Option characters used in function getopt_long. */
} COMMAND;

/**
 * @brief The list of available commands.
 */
extern COMMAND commands[];

/**
 * @brief Index for global variable ::commands.
 */
enum COMMAND_INDEX {
    CMD_HELP = 0,
    CMD_ADD,
    CMD_LOAD,
    CMD_PRINT,
    CMD_DATA,
    CMD_LIST,
    CMD_FEATURE,
    CMD_SEARCHPATH,
    CMD_EXTDATA,
    CMD_CLEAR,
    CMD_VERB,
#ifndef NDEBUG
    CMD_DEBUG,
#endif
};

/**
 * @brief For each cmd, call the COMMAND.free_func in the variable 'commands'.
 */
void cmd_free(void);

/* cmd_add.c */
void cmd_add(struct ly_ctx **ctx, const char *cmdline);
void cmd_add_help(void);

/* cmd_clear.c */
void cmd_clear(struct ly_ctx **ctx, const char *cmdline);
void cmd_clear_help(void);

/* cmd_data.c */
void cmd_data(struct ly_ctx **ctx, const char *cmdline);
void cmd_data_help(void);

/* cmd_list.c */
void cmd_list(struct ly_ctx **ctx, const char *cmdline);
void cmd_list_help(void);

/* cmd_feature.c */
void cmd_feature(struct ly_ctx **ctx, const char *cmdline);
void cmd_feature_help(void);

/* cmd_load.c */
void cmd_load(struct ly_ctx **ctx, const char *cmdline);
void cmd_load_help(void);

/* cmd_print.c */
void cmd_print(struct ly_ctx **ctx, const char *cmdline);
void cmd_print_help(void);

/* cmd_searchpath.c */
void cmd_searchpath(struct ly_ctx **ctx, const char *cmdline);
void cmd_searchpath_help(void);

/* cmd_extdata.c */
void cmd_extdata(struct ly_ctx **ctx, const char *cmdline);
void cmd_extdata_help(void);
void cmd_extdata_free(void);

#endif /* COMMANDS_H_ */
