/**
 * @file main.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/times.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "../../src/libyang.h"

void usage(const char *progname)
{
	fprintf(stdout, "Usage: %s [-h] [-v level] [-p dir] [-f format [-o file]] file.yin\n\n", progname);
	fprintf(stdout, "  -h, --help             Print this text.\n");
	fprintf(stdout, "  -f, --format yang      Print data model in given format.\n");
	fprintf(stdout, "  -o, --output file      Print output to the specified file.\n");
	fprintf(stdout, "  -p, --path dir         Search path for data models.\n");
	fprintf(stdout, "  -v, --verbose level    Set verbosity level (0-3).\n");
	fprintf(stdout, "  file.yin               Input file in YIN format.\n\n");
}

int main(int argc, char *argv[])
{
	int c;
	int ret = EXIT_SUCCESS;
	FILE *output = NULL;
	int fd;
	struct ly_module *model;
	struct ly_ctx *ctx;
	struct stat sb;
	char *addr;
	char *searchpath = NULL;
	LY_MOUTFORMAT out_format = LY_OUT_YANG;

	int opt_i;
	struct option opt[] = {
			{ "format",  required_argument, 0, 'f' },
			{ "help",    no_argument,       0, 'h' },
			{ "output",  required_argument, 0, 'o' },
			{ "path",    required_argument, 0, 'p' },
			{ "verbose", required_argument, 0, 'v' },
			{ 0, 0, 0, 0 } };

	while ((c = getopt_long(argc, argv, "f:ho:p:v:", opt, &opt_i)) != -1) {
		switch (c) {
		case 'f':
			if (strcmp(optarg, "yang") && strcmp(optarg, "tree")) {
				fprintf(stderr, "Output format \"%s\" not supported.\n", optarg);
				return EXIT_FAILURE;
			}
			if (strcmp(optarg, "tree") == 0) {
				out_format = LY_OUT_TREE;
			}

			if (!output) {
				output = stdout;
			}
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'o':
			output = fopen(optarg, "w");
			if (!output) {
				fprintf(stderr, "Unable to use output file (%s).\n", strerror(errno));
				return EXIT_FAILURE;
			}
			break;
		case 'p':
			searchpath = optarg;
			break;
		case 'v':
			ly_verb(atoi(optarg));
			break;
		default: /* '?' */
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (optind != argc - 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	fd = open(argv[optind], O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Opening input file failed (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}
	fstat(fd, &sb);
	addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);


	/* libyang */
	ctx = ly_ctx_new(searchpath);
	model = ly_module_read(ctx, addr, LY_IN_YIN);
	if (!model) {
		fprintf(stderr, "Parsing data model failed.\n");
		ret = EXIT_FAILURE;
		goto cleanup;
	}

	if (output) {
		ly_model_print(output, model, out_format);
	}


cleanup:
	ly_ctx_destroy(ctx);
	munmap(addr, sb.st_size);
	close(fd);

	if (output && output != stdout) {
		fclose(output);
	}

	return ret;
}
