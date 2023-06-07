/**
 * @file yl_opt.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Settings options for the libyang context.
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <strings.h>

#include "in.h" /* ly_in_free */

#include "common.h"
#include "yl_opt.h"

struct cmdline_file *
fill_cmdline_file(struct ly_set *set, struct ly_in *in, const char *path, LYD_FORMAT format)
{
    struct cmdline_file *rec;

    rec = malloc(sizeof *rec);
    if (!rec) {
        YLMSG_E("Allocating memory for data file information failed.\n");
        return NULL;
    }
    rec->in = in;
    rec->path = path;
    rec->format = format;

    if (set && ly_set_add(set, rec, 1, NULL)) {
        free(rec);
        YLMSG_E("Storing data file information failed.\n");
        return NULL;
    }

    return rec;
}

void
free_cmdline_file_items(struct cmdline_file *rec)
{
    if (rec && rec->in) {
        ly_in_free(rec->in, 1);
    }
}

void
free_cmdline_file(void *cmdline_file)
{
    struct cmdline_file *rec = (struct cmdline_file *)cmdline_file;

    if (rec) {
        free_cmdline_file_items(rec);
        free(rec);
    }
}

void
yl_opt_erase(struct yl_opt *yo)
{
    ly_bool interactive;

    interactive = yo->interactive;

    /* data */
    ly_set_erase(&yo->data_inputs, free_cmdline_file);
    ly_in_free(yo->data_operational.in, 1);
    ly_set_erase(&yo->data_xpath, NULL);

    /* schema */
    ly_set_erase(&yo->schema_features, free_features);
    ly_set_erase(&yo->schema_modules, NULL);
    free(yo->features_output);

    /* context */
    free(yo->searchpaths);

    /* --reply-rpc */
    ly_in_free(yo->reply_rpc.in, 1);

    ly_out_free(yo->out, NULL, yo->out_stdout ? 0 : 1);

    free_cmdline(yo->argv);

    *yo = (const struct yl_opt) {
        0
    };
    yo->interactive = interactive;
}

void
free_cmdline(char *argv[])
{
    if (argv) {
        free(argv[0]);
        free(argv);
    }
}

int
parse_cmdline(const char *cmdline, int *argc_p, char **argv_p[])
{
    int count;
    char **vector;
    char *ptr;
    char qmark = 0;

    assert(cmdline);
    assert(argc_p);
    assert(argv_p);

    /* init */
    optind = 0; /* reinitialize getopt() */
    count = 1;
    vector = malloc((count + 1) * sizeof *vector);
    vector[0] = strdup(cmdline);

    /* command name */
    strtok(vector[0], " ");

    /* arguments */
    while ((ptr = strtok(NULL, " "))) {
        size_t len;
        void *r;

        len = strlen(ptr);

        if (qmark) {
            /* still in quotated text */
            /* remove NULL termination of the previous token since it is not a token,
             * but a part of the quotation string */
            ptr[-1] = ' ';

            if ((ptr[len - 1] == qmark) && (ptr[len - 2] != '\\')) {
                /* end of quotation */
                qmark = 0;
                /* shorten the argument by the terminating quotation mark */
                ptr[len - 1] = '\0';
            }
            continue;
        }

        /* another token in cmdline */
        ++count;
        r = realloc(vector, (count + 1) * sizeof *vector);
        if (!r) {
            YLMSG_E("Memory allocation failed (%s:%d, %s).\n", __FILE__, __LINE__, strerror(errno));
            free(vector);
            return -1;
        }
        vector = r;
        vector[count - 1] = ptr;

        if ((ptr[0] == '"') || (ptr[0] == '\'')) {
            /* remember the quotation mark to identify end of quotation */
            qmark = ptr[0];

            /* move the remembered argument after the quotation mark */
            ++vector[count - 1];

            /* check if the quotation is terminated within this token */
            if ((ptr[len - 1] == qmark) && (ptr[len - 2] != '\\')) {
                /* end of quotation */
                qmark = 0;
                /* shorten the argument by the terminating quotation mark */
                ptr[len - 1] = '\0';
            }
        }
    }
    vector[count] = NULL;

    *argc_p = count;
    *argv_p = vector;

    return 0;
}
