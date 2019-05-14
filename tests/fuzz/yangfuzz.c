#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "invalid usage\n");
        exit(EXIT_FAILURE);
    }


    struct ly_ctx *ctx = NULL;
	LY_ERR err;
    while (__AFL_LOOP(100)) {
		err = ly_ctx_new(NULL, 0, &ctx);
		if (err != LY_SUCCESS) {
			fprintf(stderr, "Failed to create context\n");
			exit(EXIT_FAILURE);
		}

        lys_parse_path(ctx, argv[1], LYS_IN_YANG);
    	ly_ctx_destroy(ctx, NULL);
    }
}
