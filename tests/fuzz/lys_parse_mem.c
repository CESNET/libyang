#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "libyang.h"

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
    struct ly_ctx *ctx = NULL;
    static bool log = false;
    char *data = NULL;
    LY_ERR err;

    if (!log) {
        ly_log_options(0);
        log = true;
    }

    err = ly_ctx_new(LY_SRC_DIR "/modules", 0, &ctx);
    if (err != LY_SUCCESS) {
        return 0;
    }

    data = malloc(len + 1);
    if (data == NULL) {
        ly_ctx_destroy(ctx);
        return 0;
    }

    memcpy(data, buf, len);
    data[len] = 0;

    lys_parse_mem(ctx, data, LYS_IN_YANG, NULL);
    ly_ctx_destroy(ctx);
    free(data);
    return 0;
}
