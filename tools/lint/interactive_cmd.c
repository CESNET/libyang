/**
 * @file interactive_cmd.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief libyang's yanglint tool general commands
 *
 * Copyright (c) 2024-2024 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "cmd.h"

/* Also keep enum COMMAND_I_INDEX updated. */
COMMAND interactive_cmd[] = {
    {
        "cli", cmd_cli_opt, cmd_cli_dep, cmd_cli_exec, NULL, cmd_cli_help, NULL,
        "Settings for the command line interface", "sh"
    },
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};
