/**
 * @file cmd_sid.c
 * @author Petr Hanzlik <Petr.Hanzlik@cesnet.cz>
 * @brief Processing of .sid files in the libyang's yanglint tool.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "cmd.h"

#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

/* processes the positional arguments specific for a .sid file operation (defined below) */
static int process_sid_args(const char *op, int *argc, char **argv, struct yl_opt *yo);

void
cmd_sid_help(void)
{
    printf("Usage: sid [-o OUTFILE] [-f FORMAT] <operation> [arguments]\n"
            "                  Generate, update or extend a .sid file of a loaded schema module (RFC 9595).\n\n"
            "  Operations:\n"
            "    generate EP:SIZE [published] <module-name>[@revision]\n"
            "                  Generate a new .sid file for the module.\n"
            "    update <prev.sid> [published] <module-name>[@revision]\n"
            "                  Update an existing .sid file for the module.\n"
            "    range-add EP:SIZE <prev.sid>\n"
            "                  Add a new assignment range to an existing .sid file.\n\n"
            "  EP (Entry Point) is the first SID of the new assignment range, SIZE the\n"
            "  number of SIDs to assign (must not be 0).\n"
            "  Without the 'published' keyword, the status is 'unpublished'.\n"
            "  -o OUTFILE, --output=OUTFILE\n"
            "                  Write the output to OUTFILE instead of stdout.\n"
            "  -f FORMAT, --format=FORMAT\n"
            "                  Output data format (json (default) or xml).\n"
            "  -h, --help     Display this help message.\n");
}

int
cmd_sid_dep(struct yl_opt *yo, int posc)
{
    if (yo->sid_range && yo->sid_prev_path) {
        /* interactive range-add */
        if (posc) {
            YLMSG_E("Unexpected module name argument(s) for the SID range-add operation.\n");
            return 1;
        }
    } else if (yo->interactive) {
        if (!posc) {
            /* generate or update needs the target module */
            YLMSG_E("Missing the module name to process.\n");
            return 1;
        } else if (posc > 1) {
            YLMSG_E("Only a single module name argument is allowed.\n");
            return 1;
        }
    }

    return 0;
}

int
cmd_sid_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    const char *op;
    struct option options[] = {
        {"output", required_argument, NULL, 'o'},
        {"format", required_argument, NULL, 'f'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    optind = 0;
    while ((opt = getopt_long(argc, yo->argv, commands[CMD_SID].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'o': /* --output */
            if (yo->out) {
                ly_out_free(yo->out, NULL, 0);
                yo->out = NULL;
            }
            if (ly_out_new_filepath(optarg, &yo->out)) {
                YLMSG_E("Unable to open output file %s.\n", optarg);
                return 1;
            }
            break;
        case 'f': /* --format */
            if (yo->data_out_format) {
                YLMSG_E("The output data format can be specified only once.\n");
                return 1;
            }
            if (yl_opt_update_data_out_format(optarg, yo)) {
                YLMSG_E("Invalid output data format \"%s\".\n", optarg);
                return 1;
            }
            break;
        case 'h': /* --help */
            cmd_sid_help();
            return 1;
        default:
            YLMSG_E("Unknown option.\n");
            return 1;
        }
    }

    /* the first positional argument must be the operation */
    if (optind >= argc) {
        YLMSG_E("Missing the SID operation (generate, update or range-add).\n");
        return 1;
    }
    if (strcmp(yo->argv[optind], "generate") && strcmp(yo->argv[optind], "update") &&
            strcmp(yo->argv[optind], "range-add")) {
        YLMSG_E("Unknown SID operation \"%s\" (use generate, update or range-add).\n", yo->argv[optind]);
        return 1;
    }
    op = yo->argv[optind];
    optind++;

    /* parse the SID-specific positional arguments */
    if (process_sid_args(op, &argc, yo->argv, yo)) {
        return 1;
    }

    *posc = argc - optind;
    *posv = &yo->argv[optind];

    return 0;
}

/**
 * @brief Parse a positional argument in the EP:SIZE SID assignment range format.
 *
 * @param[in] sid_range Argument to parse.
 * @param[out] entry_point Parsed entry point.
 * @param[out] range_size Parsed range size.
 * @return 0 if @p sid_range is a valid EP:SIZE range, 1 otherwise.
 */
static int
parse_sid_range(const char *sid_range, uint64_t *entry_point, uint64_t *range_size)
{
    char *end;
    unsigned long long ep, len;

    errno = 0;
    ep = strtoull(sid_range, &end, 10);
    if ((end == sid_range) || errno || (*end != ':')) {
        return 1;
    }
    end++;

    errno = 0;
    len = strtoull(end, &end, 10);
    if (!end || *end || errno) {
        return 1;
    }

    *entry_point = (uint64_t)ep;
    *range_size = (uint64_t)len;
    return 0;
}

/**
 * @brief Process the positional arguments of the interactive 'sid' command (an optional
 * EP:SIZE range, an optional 'published' keyword, and the previous .sid file for the
 * 'update'/'range-add' operations) and store them into the SID fields of ::yl_opt.
 *
 * @param[in] op The sid command operation ("generate", "update", or "range-add").
 * @param[in,out] argc Number of parsed arguments, updated to the remaining ones.
 * @param[in,out] argv Parsed arguments, updated to the remaining ones.
 * @param[in,out] yo yanglint options to store the SID values into.
 * @return 0 on success, 1 on error (message already printed).
 */
static int
process_sid_args(const char *op, int *argc, char **argv, struct yl_opt *yo)
{
    int i, j;
    uint64_t ep, size;
    size_t len;

    for (i = optind, j = optind; i < *argc; i++) {
        /* the EP:SIZE assignment range */
        if (!parse_sid_range(argv[i], &ep, &size)) {
            if (!strcmp(op, "update")) {
                YLMSG_E("The EP:SIZE range is not a valid argument for the SID update operation.");
                return 1;
            }
            if (yo->sid_range) {
                YLMSG_E("The EP:SIZE range can be specified only once.");
                return 1;
            }
            if (!size) {
                YLMSG_E("The SID range size must not be zero.");
                return 1;
            }
            yo->sid_range = strdup(argv[i]);
            if (!yo->sid_range) {
                YLMSG_E("Memory allocation failed.");
                return 1;
            }
            continue;
        }

        /* the 'published' status keyword */
        if (!strcmp(argv[i], "published")) {
            if (!strcmp(op, "range-add")) {
                YLMSG_E("The 'published' status is not a valid argument for the SID range-add operation.");
                return 1;
            }
            if (yo->sid_publish) {
                YLMSG_E("The 'published' status can be specified only once.");
                return 1;
            }
            yo->sid_publish = 1;
            continue;
        }

        /* the .sid file */
        len = strlen(argv[i]);
        if ((len > 4) && !strcmp(argv[i] + len - 4, ".sid")) {
            if (!strcmp(op, "generate")) {
                YLMSG_E("A .sid file is not a valid argument for the SID generate operation.");
                return 1;
            }
            if (yo->sid_prev_path) {
                YLMSG_E("Only a single previous .sid file can be specified.");
                return 1;
            }
            yo->sid_prev_path = strdup(argv[i]);
            if (!yo->sid_prev_path) {
                YLMSG_E("Memory allocation failed.");
                return 1;
            }
            continue;
        }

        /* keep all the other arguments for further processing */
        argv[j] = argv[i];
        j++;
    }
    argv[j] = NULL;
    *argc = j;

    /* check the mandatory arguments */
    if (strcmp(op, "update") && !yo->sid_range) {
        YLMSG_E("Missing the SID assignment range EP:SIZE.");
        return 1;
    }
    if (strcmp(op, "generate") && !yo->sid_prev_path) {
        YLMSG_E("Missing the previous .sid file to process.");
        return 1;
    }

    return 0;
}

/**
 * @brief Check the EP:SIZE SID assignment range option and parse it.
 *
 * @param[in] yo yanglint options with the range string (must be set).
 * @param[out] ep Parsed entry point.
 * @param[out] size Parsed range size.
 * @return 0 on success, 1 on failure with an error message printed.
 */
static int
sid_check_range(const struct yl_opt *yo, uint64_t *ep, uint64_t *size)
{
    if (parse_sid_range(yo->sid_range, ep, size) || !*size) {
        YLMSG_E("Bad format of the SID range \"%s\", expected EP:SIZE with a nonzero SIZE.", yo->sid_range);
        return 1;
    }
    return 0;
}

/**
 * @brief Print the resulting .sid file data tree in the requested data format (JSON by default)
 * to the prepared output.
 *
 * @param[in,out] yo yanglint options with the prepared output handler.
 * @param[in] sid_file The .sid file data tree to print.
 * @return 0 on success, 1 on failure.
 */
static int
sid_print(struct yl_opt *yo, struct lyd_node *sid_file)
{
    int rc = 0;
    uint8_t out_alloc = 0;
    LYD_FORMAT format = LYD_JSON;

    if (yo->data_out_format) {
        format = yo->data_out_format;
    }

    if (!yo->out) {
        if (ly_out_new_file(stdout, &yo->out)) {
            YLMSG_E("Unable to allocate output handler.");
            return 1;
        }
        out_alloc = 1;
    }

    if (lyd_print_all(yo->out, sid_file, format, 0)) {
        YLMSG_E("Printing the .sid file failed.");
        rc = 1;
    } else {
        ly_print_flush(yo->out);
    }

    if (out_alloc) {
        ly_out_free(yo->out, NULL, 0);
        yo->out = NULL;
    }

    return rc;
}

int
cmd_sid_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    int rc = 1;
    const struct lys_module *mod = NULL;
    struct lyd_node *prev = NULL, *sid_file = NULL;
    char *name = NULL, *revision;
    uint64_t ep = 0, size = 0;
    LYS_SID_FILE_STATUS status = yo->sid_publish ? LYS_SID_FILE_PUBLISHED : LYS_SID_FILE_UNPUBLISHED;

    /* make sure the ietf-sid-file module is implemented in the context */
    if (!ly_ctx_get_module_implemented(*ctx, "ietf-sid-file") &&
            !ly_ctx_load_module(*ctx, "ietf-sid-file", NULL, NULL)) {
        YLMSG_E("Unable to load the ietf-sid-file module required for SID processing.");
        return 1;
    }

    if (yo->sid_range && sid_check_range(yo, &ep, &size)) {
        return 1;
    }

    /* generate and update always have the target module, the standalone range-add does not */
    if (posv) {
        name = strdup(posv);
        if (!name) {
            YLMSG_E("Memory allocation failed.");
            return 1;
        }
        revision = strchr(name, '@');
        if (revision) {
            revision[0] = '\0';
            revision++;
        }

        mod = revision ?
                ly_ctx_get_module(*ctx, name, revision) :
                ly_ctx_get_module_latest(*ctx, name);

        if (!mod) {
            YLMSG_E("Module \"%s\" not loaded.", name);
            goto cleanup;
        }
        if (!mod->compiled && lys_set_implemented((struct lys_module *)mod, NULL)) {
            YLMSG_E("Implementing module \"%s\" failed.", mod->name);
            goto cleanup;
        }
    }

    if (yo->sid_prev_path) {
        /* parse the previous .sid file */
        if (lyd_parse_data_path(*ctx, yo->sid_prev_path, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &prev)) {
            YLMSG_E("Failed to parse the previous .sid file \"%s\".", yo->sid_prev_path);
            goto cleanup;
        }
    }

    if (yo->sid_range && yo->sid_prev_path) {
        /* add the new assignment range first */
        if (lys_sid_range_add(prev, ep, size)) {
            YLMSG_E("Adding a range into the .sid file \"%s\" failed.", yo->sid_prev_path);
            goto cleanup;
        }
        if (yo->interactive) {
            /* standalone range-add prints the previous file with the added range */
            sid_file = prev;
            prev = NULL;
        }
    }

    if (!sid_file) {
        if (prev) {
            /* update */
            if (lys_sid_update(mod, prev, status, NULL, &sid_file)) {
                YLMSG_E("Updating the .sid file of module \"%s\" failed.", mod->name);
                goto cleanup;
            }
        } else {
            /* generation */
            if (lys_sid_gen(mod, ep, size, status, NULL, &sid_file)) {
                YLMSG_E("Generating the .sid file for module \"%s\" failed.", mod->name);
                goto cleanup;
            }
        }
    }

    rc = sid_print(yo, sid_file);

cleanup:
    free(name);
    lyd_free_all(prev);
    lyd_free_all(sid_file);
    return rc;
}
