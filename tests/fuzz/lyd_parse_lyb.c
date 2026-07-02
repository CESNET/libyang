#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "libyang.h"

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
    struct ly_ctx *ctx = NULL;
    struct lyd_node *tree = NULL;
    FILE *input = NULL;
    static bool log = false;
    const char *schema =
            "module fuzz-lyb {namespace urn:tests:fuzz-lyb;prefix fl;"
            "container c {leaf s {type string;} leaf u16 {type uint16;}"
            "leaf-list bits {type bits {bit zero; bit one;}}}}";

    if (!log) {
        ly_log_options(0);
        log = true;
    }

    if (ly_ctx_new(LY_SRC_DIR "/modules", 0, &ctx) != LY_SUCCESS) {
        return 0;
    }

    if (lys_parse_mem(ctx, schema, LYS_IN_YANG, NULL) != LY_SUCCESS) {
        goto cleanup;
    }

    input = tmpfile();
    if (!input) {
        goto cleanup;
    }

    if (len && (fwrite(buf, 1, len, input) != len)) {
        goto cleanup;
    }
    fflush(input);
    if (lseek(fileno(input), 0, SEEK_SET) == -1) {
        goto cleanup;
    }

    lyd_parse_data_fd(ctx, fileno(input), LYD_LYB, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree);

cleanup:
    lyd_free_all(tree);
    if (input) {
        fclose(input);
    }
    ly_ctx_destroy(ctx);
    return 0;
}
