#include <stdbool.h>
#include <stdint.h>

#include "libyang.h"
#include "tests_config.h"

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
    struct ly_ctx *ctx = NULL;
    struct lyd_node *tree = NULL;
    static bool log = false;
    const char *schema =
            "module fuzz-lyb {namespace urn:tests:fuzz-lyb;prefix fl;"
            "container c {leaf s {type string;} leaf u16 {type uint16;}"
            "leaf-list bits {type bits {bit zero; bit one;}}}}";

    if (!log) {
        ly_log_options(0);
        log = true;
    }

    if (ly_ctx_new(TESTS_SRC "/../modules", 0, &ctx) != LY_SUCCESS) {
        return 0;
    }

    if (lys_parse_mem(ctx, schema, LYS_IN_YANG, NULL) != LY_SUCCESS) {
        goto cleanup;
    }

    if (len <= UINT32_MAX) {
        lyd_parse_data_mem_len(ctx, (const char *)buf, (uint32_t)len, LYD_LYB, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT,
                &tree);
    }

cleanup:
    lyd_free_all(tree);
    ly_ctx_destroy(ctx);
    return 0;
}
