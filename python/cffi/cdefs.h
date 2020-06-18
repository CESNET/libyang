/*
 * Copyright (c) 2018-2019 Robin Jarry
 * SPDX-License-Identifier: MIT
 */

struct ly_ctx;

#define LY_CTX_ALLIMPLEMENTED ...
#define LY_CTX_TRUSTED ...
#define LY_CTX_NOYANGLIBRARY ...
#define LY_CTX_DISABLE_SEARCHDIRS ...
#define LY_CTX_DISABLE_SEARCHDIR_CWD ...
#define LY_CTX_PREFER_SEARCHDIRS ...

struct ly_ctx *ly_ctx_new(const char *, int);
int ly_ctx_set_searchdir(struct ly_ctx *, const char *);
void ly_ctx_destroy(struct ly_ctx *, void *);

typedef enum {
    LY_SUCCESS,
    ...
} LY_ERR;

typedef enum {
    LYVE_SUCCESS,
    LYVE_PATH_EXISTS,
    ...
} LY_VECODE;

typedef enum {
    LY_LLERR,
    LY_LLWRN,
    LY_LLVRB,
    LY_LLDBG,
    ...
} LY_LOG_LEVEL;

struct ly_err_item {
    char *msg;
    char *path;
    char *apptag;
    struct ly_err_item *next;
    ...;
};

#define LY_LOLOG ...
#define LY_LOSTORE ...
#define LY_LOSTORE_LAST ...
int ly_log_options(int);

LY_LOG_LEVEL ly_verb(LY_LOG_LEVEL);
extern "Python" void lypy_log_cb(LY_LOG_LEVEL, const char *, const char *);
void ly_set_log_clb(void (*)(LY_LOG_LEVEL, const char *, const char *), int);
struct ly_err_item *ly_err_first(const struct ly_ctx *);
void ly_err_clean(struct ly_ctx *, struct ly_err_item *);
LY_VECODE ly_vecode(const struct ly_ctx *);

struct lys_module {
    const char *name;
    const char *prefix;
    const char *dsc;
    const char *filepath;
    uint8_t rev_size;
    uint8_t features_size;
    struct lys_revision *rev;
    struct lys_feature *features;
    ...;
};

#define LY_REV_SIZE 11
struct lys_revision {
    char date[LY_REV_SIZE];
    uint8_t ext_size;
    struct lys_ext_instance **ext;
    const char *dsc;
    const char *ref;
};

#define LYS_FENABLED ...
struct lys_feature {
    const char *name;
    const char *dsc;
    const char *ref;
    uint16_t flags;
    uint8_t iffeature_size;
    struct lys_iffeature *iffeature;
    struct lys_module *module;
    ...;
};

#define LYS_IFF_NOT ...
#define LYS_IFF_AND ...
#define LYS_IFF_OR ...
#define LYS_IFF_F ...
struct lys_iffeature {
    uint8_t *expr;
    struct lys_feature **features;
    ...;
};

int lys_features_enable(const struct lys_module *, const char *);
int lys_features_disable(const struct lys_module *, const char *);
int lys_features_state(const struct lys_module *, const char *);

struct lys_ext {
    const char *name;
    struct lys_module *module;
    ...;
};

struct lys_ext_instance {
    struct lys_ext *def;
    const char *arg_value;
    ...;
};

struct lys_restr {
    const char *expr;
    ...;
};

typedef enum {
    LY_TYPE_DER,
    LY_TYPE_BINARY,
    LY_TYPE_BITS,
    LY_TYPE_BOOL,
    LY_TYPE_DEC64,
    LY_TYPE_EMPTY,
    LY_TYPE_ENUM,
    LY_TYPE_IDENT,
    LY_TYPE_INST,
    LY_TYPE_LEAFREF,
    LY_TYPE_STRING,
    LY_TYPE_UNION,
    LY_TYPE_INT8,
    LY_TYPE_UINT8,
    LY_TYPE_INT16,
    LY_TYPE_UINT16,
    LY_TYPE_INT32,
    LY_TYPE_UINT32,
    LY_TYPE_INT64,
    LY_TYPE_UINT64,
    LY_TYPE_UNKNOWN,
    ...
} LY_DATA_TYPE;

struct lys_type_info_binary {
    struct lys_restr *length;
};

struct lys_type_bit {
    const char *name;
    const char *dsc;
    uint32_t pos;
    ...;
};

struct lys_type_info_bits {
    struct lys_type_bit *bit;
    unsigned int count;
};

struct lys_type_info_dec64 {
    struct lys_restr *range;
    ...;
};

struct lys_type_enum {
    const char *name;
    const char *dsc;
    ...;
};

struct lys_type_info_enums {
    struct lys_type_enum *enm;
    unsigned int count;
};

struct lys_type_info_num {
    struct lys_restr *range;
};

struct lys_type_info_lref {
    const char *path;
    struct lys_node_leaf* target;
    int8_t req;
};

struct lys_type_info_str {
    struct lys_restr *length;
    struct lys_restr *patterns;
    unsigned int pat_count;
    ...;
};

struct lys_type_info_union {
    struct lys_type *types;
    unsigned int count;
    int has_ptr_type;
};

union lys_type_info {
    struct lys_type_info_binary binary;
    struct lys_type_info_bits bits;
    struct lys_type_info_dec64 dec64;
    struct lys_type_info_enums enums;
    struct lys_type_info_num num;
    struct lys_type_info_lref lref;
    struct lys_type_info_str str;
    struct lys_type_info_union uni;
    ...;
};

struct lys_type {
    LY_DATA_TYPE base;
    uint8_t value_flags;
    uint8_t ext_size;
    struct lys_ext_instance **ext;
    struct lys_tpdf *der;
    struct lys_tpdf *parent;
    union lys_type_info info;
    ...;
};

struct lys_tpdf {
    const char *name;
    const char *dsc;
    uint8_t ext_size;
    struct lys_ext_instance **ext;
    const char *units;
    struct lys_type type;
    const char *dflt;
    ...;
};

typedef enum lys_nodetype {
    LYS_UNKNOWN,
    LYS_CONTAINER,
    LYS_CHOICE,
    LYS_LEAF,
    LYS_LEAFLIST,
    LYS_LIST,
    LYS_ANYXML,
    LYS_CASE,
    LYS_NOTIF,
    LYS_RPC,
    LYS_INPUT,
    LYS_OUTPUT,
    LYS_GROUPING,
    LYS_USES,
    LYS_AUGMENT,
    LYS_ACTION,
    LYS_ANYDATA,
    LYS_EXT,
    ...
} LYS_NODE;

#define LYS_CONFIG_W ...
#define LYS_CONFIG_R ...
#define LYS_CONFIG_SET ...
#define LYS_USERORDERED ...
#define LYS_MAND_TRUE ...
#define LYS_STATUS_DEPRC ...
#define LYS_STATUS_OBSLT ...

struct lys_node {
    const char *name;
    const char *dsc;
    uint16_t flags;
    uint8_t ext_size;
    uint8_t iffeature_size;
    struct lys_ext_instance **ext;
    struct lys_iffeature *iffeature;
    LYS_NODE nodetype;
    ...;
};

struct lys_node_container {
    uint8_t must_size;
    struct lys_restr *must;
    const char *presence;
    ...;
};

struct lys_node_leaf {
    uint8_t must_size;
    struct lys_restr *must;
    struct lys_type type;
    const char *units;
    const char *dflt;
    ...;
};

struct lys_node_leaflist {
    uint8_t must_size;
    struct lys_restr *must;
    struct lys_type type;
    const char *units;
    uint32_t min;
    uint32_t max;
    uint8_t dflt_size;
    const char **dflt;
    ...;
};

struct lys_node_list {
    uint8_t must_size;
    struct lys_restr *must;
    uint8_t keys_size;
    struct lys_node_leaf **keys;
    uint32_t min;
    uint32_t max;
    ...;
};

union ly_set_set {
    struct lys_node **s;
    ...;
};

struct ly_set {
    unsigned int size;
    unsigned int number;
    union ly_set_set set;
};

const struct lys_module *ly_ctx_load_module(struct ly_ctx *, const char *, const char *);
const struct lys_module *ly_ctx_get_module_iter(const struct ly_ctx *, uint32_t *);
const struct lys_module *ly_ctx_get_module(const struct ly_ctx *, const char *, const char *, int);
struct ly_set *ly_ctx_find_path(struct ly_ctx *, const char *);
void ly_set_free(struct ly_set *set);
const struct lys_node_list *lys_is_key(const struct lys_node_leaf *, uint8_t *);

#define LYS_GETNEXT_WITHCHOICE ...
#define LYS_GETNEXT_WITHCASE ...
#define LYS_GETNEXT_WITHGROUPING ...
#define LYS_GETNEXT_WITHINOUT ...
#define LYS_GETNEXT_WITHUSES ...
#define LYS_GETNEXT_INTOUSES ...
#define LYS_GETNEXT_INTONPCONT ...
#define LYS_GETNEXT_PARENTUSES ...
#define LYS_GETNEXT_NOSTATECHECK ...

const struct lys_node *lys_getnext(const struct lys_node *, const struct lys_node *, const struct lys_module *, int);
char *lys_data_path(const struct lys_node *);
char *lys_path(const struct lys_node *, int);
char *lys_data_path_pattern(const struct lys_node *, const char *);
struct lys_module *lys_node_module(const struct lys_node *);
struct lys_module *lys_main_module(const struct lys_module *);
struct lys_node *lys_parent(const struct lys_node *);

typedef enum {
    LYS_IN_UNKNOWN,
    LYS_IN_YANG,
    LYS_IN_YIN,
    ...
} LYS_INFORMAT;

const struct lys_module *lys_parse_mem(struct ly_ctx *, const char *, LYS_INFORMAT);
const struct lys_module *lys_parse_fd(struct ly_ctx *, int, LYS_INFORMAT);

typedef enum {
    LYS_OUT_UNKNOWN,
    LYS_OUT_YANG,
    LYS_OUT_YIN,
    LYS_OUT_TREE,
    LYS_OUT_INFO,
    LYS_OUT_JSON,
    ...
} LYS_OUTFORMAT;

int lys_print_mem(char **, const struct lys_module *, LYS_OUTFORMAT, const char *, int, int);
int lys_print_fd(int, const struct lys_module *, LYS_OUTFORMAT, const char *, int, int);

typedef enum {
    LYD_XML,
    LYD_JSON,
    LYD_LYB,
    ...
} LYD_FORMAT;

typedef enum {
    ...
} LYD_ANYDATA_VALUETYPE;

#define LYP_WITHSIBLINGS ...
#define LYP_FORMAT ...
#define LYP_WD_TRIM ...
#define LYP_WD_ALL ...
#define LYP_KEEPEMPTYCONT ...

#define LYD_ANYDATA_CONSTSTRING ...

#define LYD_PATH_OPT_UPDATE ...
#define LYD_PATH_OPT_NOPARENTRET ...
#define LYD_PATH_OPT_OUTPUT ...

#define LYD_OPT_DATA ...
#define LYD_OPT_CONFIG ...
#define LYD_OPT_GET ...
#define LYD_OPT_STRICT ...
#define LYD_OPT_TRUSTED ...
#define LYD_OPT_DATA_NO_YANGLIB ...
#define LYD_OPT_RPC ...
#define LYD_OPT_RPCREPLY ...
#define LYD_OPT_EXPLICIT ...
#define LYD_OPT_DESTRUCT ...
#define LYD_OPT_NOSIBLINGS ...

typedef union lyd_value_u {
    int8_t bln;
    struct lyd_node *leafref;
    ...;
} lyd_val;

struct lyd_node {
    struct lys_node *schema;
    struct lyd_node *next;
    struct lyd_node *child;
    struct lyd_node *parent;
    ...;
};

struct lyd_node_leaf_list {
    struct lys_node *schema;
    struct lyd_node *next;
    struct lyd_node *parent;
    const char *value_str;
    lyd_val value;
    LY_DATA_TYPE value_type;
    ...;
};

struct ly_set *lyd_find_instance(const struct lyd_node *, const struct lys_node *);
struct ly_set *lyd_find_path(const struct lyd_node *, const char *);
struct lyd_node *lyd_new(struct lyd_node *, const struct lys_module *, const char *);
struct lyd_node *lyd_new_leaf(struct lyd_node *, const struct lys_module *, const char *, const char *);
struct lyd_node *lyd_new_output(struct lyd_node *, const struct lys_module *, const char *);
struct lyd_node *lyd_new_output_leaf(struct lyd_node *, const struct lys_module *, const char *, const char *);
struct lyd_node *lyd_new_path(struct lyd_node *, const struct ly_ctx *, const char *, void *, LYD_ANYDATA_VALUETYPE, int);
struct lyd_node *lyd_first_sibling(struct lyd_node *);
char *lyd_path(const struct lyd_node *);
struct lys_module *lyd_node_module(struct lyd_node *);
struct lyd_node *lyd_parse_mem(struct ly_ctx *, const char *, LYD_FORMAT, int, ...);
struct lyd_node *lyd_parse_fd(struct ly_ctx *, int, LYD_FORMAT, int, ...);
int lyd_print_fd(int, const struct lyd_node *, LYD_FORMAT, int);
int lyd_print_mem(char **, const struct lyd_node *, LYD_FORMAT, int);
int lyd_node_should_print(const struct lyd_node *, int);
double lyd_dec64_to_double(const struct lyd_node *);
void lyd_free(struct lyd_node *);
void lyd_free_withsiblings(struct lyd_node *);
int lyd_validate(struct lyd_node **, int, void *);
int lyd_merge(struct lyd_node *, const struct lyd_node *, int);

/* from libc, needed to free allocated strings */
void free(void *);

/* extra functions */
uint8_t lypy_module_implemented(const struct lys_module *);
LY_ERR lypy_get_errno(void);
void lypy_set_errno(LY_ERR);
