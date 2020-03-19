#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "libyang.h"

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
	struct ly_ctx *ctx = NULL;
	static bool log = false;
	LY_ERR err;

	if (!log) {
		ly_log_options(0);
		log = true;
	}

	err = ly_ctx_new(NULL, 0, &ctx);
	if (err != LY_SUCCESS) {
		fprintf(stderr, "Failed to create context\n");
		exit(EXIT_FAILURE);
	}

	lys_parse_mem(ctx, buf, LYS_IN_YANG);
	ly_ctx_destroy(ctx, NULL);
	return 0;
}
