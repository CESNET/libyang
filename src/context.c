/**
 * @file context.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief context implementation for libyang
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
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "common.h"
#include "context.h"
#include "dict.h"

API struct ly_ctx *ly_ctx_new(const char *search_dir)
{
	struct stat st;
	struct ly_ctx *ctx;
	char *cwd;

	ctx = calloc(1, sizeof *ctx);
	if (!ctx) {
		ly_errno = LY_EFATAL;
		return NULL;
	}

	/* dictionary */
	lydict_init(&ctx->dict);

	/* models list */
	ctx->models.list = calloc(16, sizeof *ctx->models.list);
	ctx->models.used = 0;
	ctx->models.size = 16;
	if (search_dir) {
		if (stat(search_dir, &st) == -1) {
			LY_ERR(LY_ESYS,
			       "Unable to get information about search directory (%s)",
				   strerror(errno));
			ly_ctx_destroy(ctx);
			return NULL;
		}
		if (!S_ISDIR(st.st_mode)) {
			LY_ERR(LY_EINVAL,
			       "Search directory parameter does not specify a directory");
			ly_ctx_destroy(ctx);
			return NULL;
		}

		if (search_dir) {
			cwd = get_current_dir_name();
			chdir(search_dir);
			ctx->models.search_path = get_current_dir_name();
			chdir(cwd);
			free(cwd);
		}
	}

	return ctx;
}


API void ly_ctx_destroy(struct ly_ctx *ctx)
{
	int i;

	if (!ctx) {
		ly_errno = LY_EINVAL;
		return;
	}

	/* models list */
	if (ctx->models.used) {
		for (i = 0; i < ctx->models.size; i++) {
			if (ctx->models.list[i]) {
				ly_model_free(ctx->models.list[i]);
			}
		}
	}
	free(ctx->models.search_path);
	free(ctx->models.list);

	/* dictionary */
	lydict_clean(&ctx->dict);

	free(ctx);
}

API struct ly_module *ly_ctx_get_model(struct ly_ctx *ctx, const char *name,
                                      const char *revision)
{
	int i, len, flen;
	char *cwd;
	int fd;
	DIR *dir;
	struct dirent *file;
	struct ly_module *m;
	LY_MFORMAT format;

	if (!ctx || !name) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	for (i = 0; i < ctx->models.size; i++) {
		m = ctx->models.list[i];
		if (!m || strcmp(name, m->name)) {
			continue;
		}

		if (!revision || (m->rev_size && !strcmp(revision, m->rev[0].date))) {
			return m;
		}
	}
	m = NULL;

	/* not found in context, try to get it from the search directory */
	if (!ctx->models.search_path) {
		if (revision) {
			LY_ERR(LY_EVALID, "Unknown data model \"%s\", revision \"%s\" (search path not specified)", name, revision);
		} else {
			LY_ERR(LY_EVALID, "Unknown data model \"%s\" (search path not specified)", name);
		}
		return NULL;
	}

	len = strlen(name);
	cwd = get_current_dir_name();
	dir = opendir(ctx->models.search_path);
	if (!dir) {
		fprintf(stderr, "%s \n %s \n (%s)", cwd, ctx->models.search_path, strerror(errno));
		return (NULL);
	}
	chdir(ctx->models.search_path);
	while ((file = readdir(dir))) {
		if (strncmp(name, file->d_name, len)) {
			continue;
		}

		flen = strlen(file->d_name);
		if (revision && flen > len + 5) {
			/* check revision from the filename */
			/* TODO */
		}

		/* get type according to filename suffix */
		if (!strcmp(&file->d_name[flen-4], ".yin")) {
			format = LY_YIN;
		} else if (!strcmp(&file->d_name[flen-5], ".yang")) {
			format = LY_YANG;
		} else {
			continue;
		}

		/* open the file */
		fd = open(file->d_name, O_RDONLY);
		m = ly_model_read_fd(ctx, fd, format);
		close(fd);

		if (m) {
			break;
		}
	}

	chdir(cwd);
	free(cwd);
	closedir(dir);

	if (!m) {
		LY_ERR(LY_EVALID, "Data model \"%s\" not found (search path is \"%s\")", ctx->models.search_path);
	}

	return m;
}
