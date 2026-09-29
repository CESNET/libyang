#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "libyang.h"
#include "tests_config.h"

static const char mounted_module[] =
        "module fuzz-sm-leaf {"
        "namespace \"urn:tests:fuzz-sm-leaf\";"
        "prefix fsl;"
        "leaf lfl {type uint16;}"
        "leaf txt {type string;}"
        "}";

static const char mount_module[] =
        "module fuzz-sm {"
        "yang-version 1.1;"
        "namespace \"urn:tests:fuzz-sm\";"
        "prefix fsm;"
        "import ietf-yang-schema-mount {prefix sm;}"
        "container root {sm:mount-point \"root\";}"
        "}";

static const char ext_data_xml[] =
        "<yang-library xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module-set><name>fuzz-set</name><module><name>fuzz-sm-leaf</name>"
        "<namespace>urn:tests:fuzz-sm-leaf</namespace></module></module-set>"
        "<content-id>1</content-id></yang-library>"
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module-set-id>1</module-set-id></modules-state>"
        "<schema-mounts xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-schema-mount\">"
        "<mount-point><module>fuzz-sm</module><label>root</label><inline/></mount-point>"
        "</schema-mounts>";

static const char mount_module_dir_template[] = "/tmp/libyang-sm-fuzz.XXXXXX";
static char mount_module_dir[sizeof mount_module_dir_template];
static ly_bool mount_module_dir_ready;

static void
remove_mount_module_dir(void)
{
    char path[sizeof mount_module_dir + sizeof "/fuzz-sm-leaf.yang"];

    if (!mount_module_dir_ready) {
        return;
    }

    snprintf(path, sizeof path, "%s/fuzz-sm-leaf.yang", mount_module_dir);
    unlink(path);
    rmdir(mount_module_dir);
    mount_module_dir[0] = 0;
    mount_module_dir_ready = 0;
}

static LY_ERR
prepare_mount_module_dir(void)
{
    char path[sizeof mount_module_dir + sizeof "/fuzz-sm-leaf.yang"];
    FILE *f;

    if (mount_module_dir_ready) {
        return LY_SUCCESS;
    }

    memcpy(mount_module_dir, mount_module_dir_template, sizeof mount_module_dir);
    if (!mkdtemp(mount_module_dir)) {
        mount_module_dir[0] = 0;
        return LY_ESYS;
    }
    mount_module_dir_ready = 1;
    atexit(remove_mount_module_dir);

    snprintf(path, sizeof path, "%s/fuzz-sm-leaf.yang", mount_module_dir);
    f = fopen(path, "w");
    if (!f) {
        remove_mount_module_dir();
        return LY_ESYS;
    }

    if (fputs(mounted_module, f) == EOF) {
        fclose(f);
        remove_mount_module_dir();
        return LY_ESYS;
    }
    if (fclose(f) == EOF) {
        remove_mount_module_dir();
        return LY_ESYS;
    }

    return LY_SUCCESS;
}

static LY_ERR
fuzz_ext_data_clb(const struct lysc_ext_instance *ext, const struct lyd_node *parent, void *user_data,
        void **ext_data, ly_bool *ext_data_free)
{
    struct lyd_node *data = NULL;
    static ly_bool recursive_call;
    LY_ERR ret = LY_SUCCESS;

    (void)ext;
    (void)parent;

    *ext_data = NULL;
    *ext_data_free = 0;

    if (recursive_call) {
        return LY_SUCCESS;
    }

    recursive_call = 1;
    ret = lyd_parse_data_mem(user_data, ext_data_xml, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &data);
    recursive_call = 0;
    if (ret) {
        lyd_free_all(data);
        return ret;
    }

    *ext_data = data;
    *ext_data_free = 1;
    return LY_SUCCESS;
}

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
    struct ly_ctx *ctx = NULL;
    struct lyd_node *tree = NULL;
    char *data = NULL;
    static bool log = false;

    if (!log) {
        ly_log_options(0);
        log = true;
    }

    if (prepare_mount_module_dir() != LY_SUCCESS) {
        return 0;
    }

    if (ly_ctx_new(TESTS_SRC "/../modules", 0, &ctx) != LY_SUCCESS) {
        return 0;
    }
    if (ly_ctx_set_searchdir(ctx, mount_module_dir) != LY_SUCCESS) {
        goto cleanup;
    }
    if (lys_parse_mem(ctx, mount_module, LYS_IN_YANG, NULL) != LY_SUCCESS) {
        goto cleanup;
    }

    ly_ctx_set_ext_data_clb(ctx, fuzz_ext_data_clb, ctx);

    data = malloc(len + 1);
    if (!data) {
        goto cleanup;
    }
    memcpy(data, buf, len);
    data[len] = 0;

    lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT | LYD_VALIDATE_MULTI_ERROR, &tree);

cleanup:
    lyd_free_all(tree);
    free(data);
    ly_ctx_destroy(ctx);
    return 0;
}
