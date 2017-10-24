#include <stdlib.h>

#include "../../src/libyang.h"

int
main(int argc, char **argv)
{
    int i;
    struct ly_ctx *ctx;
    struct lyd_node *data;

    if (argc < 3) {
        return 1;
    }

    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        return 1;
    }

    for (i = 1; i < argc - 1; ++i) {
        if (!lys_parse_path(ctx, argv[i], LYS_YANG)) {
            ly_ctx_destroy(ctx, NULL);
            return 1;
        }
    }

    data = lyd_parse_path(ctx, argv[argc - 1], LYD_XML, LYD_OPT_STRICT | LYD_OPT_DATA_NO_YANGLIB);
    if (!data) {
        ly_ctx_destroy(ctx, NULL);
        return 1;
    }

    lyd_free_withsiblings(data);
    ly_ctx_destroy(ctx, NULL);
    return 0;
}
