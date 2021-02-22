#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

int LLVMFuzzerTestOneInput(const char *data, size_t size) {
    struct ly_ctx *ctx = NULL;

    char filename[256];
    sprintf(filename, "/tmp/libfuzzer.%d", getpid());

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return 0;
    }
    fwrite(data, size, 1, fp);
    fclose(fp);

    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        return 0;
    }
    lyxml_parse_path(ctx, filename, LYS_IN_YANG);
    ly_ctx_clean(ctx, NULL);
    ly_ctx_destroy(ctx, NULL);

    unlink(filename);
    return 0;
}
