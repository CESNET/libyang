/**
 * @file printer_tree.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief RFC tree printer for libyang data structure
 *
 * Copyright (c) 2015 - 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include "compat.h"
#include "out_internal.h"
#include "printer_internal.h"
#include "xpath.h"

/******************************************************************************
 * Declarations start
 *****************************************************************************/

/*
 *          +---------+    +---------+    +---------+
 *   output |   trp   |    |   trb   |    |   tro   |
 *      <---+  Print  +<---+  Browse +<-->+  Obtain |
 *          |         |    |         |    |         |
 *          +---------+    +----+----+    +---------+
 *                              ^
 *                              |
 *                         +----+----+
 *                         |   trm   |
 *                         | Manager |
 *                         |         |
 *                         +----+----+
 *                              ^
 *                              | input
 *                              +
 *
 * Glossary:
 * trt - type
 * trp - functions for Printing
 * trb - functions for Browse the tree
 * tro - functions for Obtaining information from libyang
 * trm - Main functions, Manager
 * trg - General functions
 *
 * - Manager functions (trm) are able to print individual sections of the YANG tree diagram
 *   (eg module, notifications, rpcs ...) and they call Browse functions (trb).
 * - Browse functions contain a general algorithm (Preorder DFS) for traversing the tree.
 *   They call the Obtain functions (tro) to get information about the node
 *   or eg to get a sibling or child for the current node.
 *   This obtained information is passed on to the Print functions (trp) for printing.
 *   Gap offsets before the node type are also calculated in the Browse functions.
 * - Print functions (trp) take care of the printing itself.
 *   They can also split one node into multiple lines if the node does not fit on one line.
 *
 * For future adjustments:
 *  it is assumed that the changes are likely to take place mainly for tro functions
 *  because they are the only ones dependent on libyang implementation.
 *  In special cases, changes will also need to be made to the trp functions
 *  if a special algorithm is needed to print (right now this is prepared for printing list's keys
 *  and if-features).
 */

/**
 * @brief List of available actions.
 */
typedef enum {
    TRD_PRINT = 0,
    TRD_CHAR_COUNT
} trt_ly_out_clb_arg_flag;

/**
 * @brief Specific argument to be passed to the ly_write_clb.
 */
struct ly_out_clb_arg {
    trt_ly_out_clb_arg_flag mode;       /**< flag specifying which action to take. */
    struct ly_out *out;                 /**< The ly_out pointer delivered to the printer tree module via the main interface. */
    size_t counter;                     /**< Counter of printed characters. */
    LY_ERR last_error;                  /**< The last error that occurred. If no error has occurred, it will be LY_SUCCESS. */
};

/**
 * @brief Initialize struct ly_out_clb_arg with default settings.
 */
#define TRP_INIT_LY_OUT_CLB_ARG(MODE, OUT, COUNTER, LAST_ERROR) \
    (struct ly_out_clb_arg){.mode = MODE, .out = OUT, .counter = COUNTER, .last_error = LAST_ERROR}

/******************************************************************************
 * Print getters
 *****************************************************************************/

/**
 * @brief Callback functions that prints special cases.
 *
 * It just groups together tree context with trt_fp_print.
 */
struct trt_cf_print {
    const struct trt_tree_ctx *ctx;                                 /**< Context of libyang tree. */
    void (*pf)(const struct trt_tree_ctx *, struct ly_out *);       /**< Pointing to function which printing list's keys or features. */
};

/**
 * @brief Callback functions for printing special cases.
 *
 * Functions with the suffix 'trp' can print most of the text on output, just by setting the pointer to the string.
 * But in some cases, it's not that simple, because its entire string is fragmented in memory.
 * For example, for printing list's keys or if-features.
 * However, this depends on how the libyang library is implemented.
 * This implementation of the printer_tree module goes through a lysp tree, but if it goes through a lysc tree,
 * these special cases would be different.
 * Functions must print including spaces or delimiters between names.
 */
struct trt_fp_print {
    void (*print_features_names)(const struct trt_tree_ctx *, struct ly_out *);   /**< Print list of features without {}? wrapper. */
    void (*print_keys)(const struct trt_tree_ctx *, struct ly_out *);             /**< Print list's keys without [] wrapper. */
};

/**
 * @brief Package which only groups getter function.
 */
struct trt_pck_print {
    const struct trt_tree_ctx *tree_ctx;    /**< Context of libyang tree. */
    struct trt_fp_print fps;                /**< Print function. */
};

/**
 * @brief Initialize struct trt_pck_print by parameters.
 */
#define TRP_INIT_PCK_PRINT(TREE_CTX, FP_PRINT) \
    (struct trt_pck_print){.tree_ctx = TREE_CTX, .fps = FP_PRINT}

/******************************************************************************
 * Indent
 *****************************************************************************/

/**
 * @brief Constants which are defined in the RFC or are observable from the pyang tool.
 */
typedef enum {
    TRD_INDENT_EMPTY = 0,               /**< If the node is a case node, there is no space before the \<name\>. */
    TRD_INDENT_LONG_LINE_BREAK = 2,     /**< The new line should be indented so that it starts below \<name\> with a whitespace offset of at least two characters. */
    TRD_INDENT_LINE_BEGIN = 2,          /**< Indent below the keyword (module, augment ...).  */
    TRD_INDENT_BTW_SIBLINGS = 2,        /**< Indent between | and | characters. */
    TRD_INDENT_BEFORE_KEYS = 1,         /**< "..."___\<keys\>. */
    TRD_INDENT_BEFORE_TYPE = 4,         /**< "..."___\<type\>, but if mark is set then indent == 3. */
    TRD_INDENT_BEFORE_IFFEATURES = 1    /**< "..."___\<iffeatures\>. */
} trt_cnf_indent;

/**
 * @brief Type of indent in node.
 */
typedef enum {
    TRD_INDENT_IN_NODE_NORMAL = 0,  /**< Node fits on one line. */
    TRD_INDENT_IN_NODE_DIVIDED,     /**< The node must be split into multiple rows. */
    TRD_INDENT_IN_NODE_FAILED       /**< Cannot be crammed into one line. The condition for the maximum line length is violated. */
} trt_indent_in_node_type;

/** Constant to indicate the need to break a line. */
#define TRD_LINEBREAK -1

/**
 * @brief Records the alignment between the individual elements of the node.
 *
 * See trp_indent_in_node_are_eq, trp_indent_in_node_place_break.
 */
struct trt_indent_in_node {
    trt_indent_in_node_type type;   /**< Type of indent in node. */
    int16_t btw_name_opts;          /**< Indent between node name and opts. */
    int16_t btw_opts_type;          /**< Indent between opts and type. */
    int16_t btw_type_iffeatures;    /**< Indent between type and features. Ignored if \<type\> missing. */
};

/**
 * @brief Type of wrappers to be printed.
 */
typedef enum {
    TRD_WRAPPER_TOP = 0,    /**< Related to the module. */
    TRD_WRAPPER_BODY        /**< Related to e.g. Augmentations or Groupings */
} trd_wrapper_type;

/**
 * @brief For resolving sibling symbol ('|') placement.
 *
 * Bit indicates where the sibling symbol must be printed.
 * This place is in multiples of TRD_INDENT_BTW_SIBLINGS.
 *
 * See: TRP_INIT_WRAPPER_TOP, TRP_INIT_WRAPPER_BODY, trp_wrapper_set_mark,
 * trp_wrapper_set_shift, trp_wrapper_if_last_sibling, trp_wrapper_eq,
 * trp_print_wrapper
 */
struct trt_wrapper {
    trd_wrapper_type type;  /**< Location of the wrapper. */
    uint64_t bit_marks1;    /**< The set bits indicate where the '|' character is to be printed.
                                 It follows that the maximum immersion of the printable node is 64. */
    uint32_t actual_pos;    /**< Actual position in bit_marks. */
};

/**
 * @brief Get wrapper related to the module section.
 *
 * @code
 * module: <module-name>
 *   +--<node>
 *   |
 * @endcode
 */
#define TRP_INIT_WRAPPER_TOP \
    (struct trt_wrapper) {.type = TRD_WRAPPER_TOP, .actual_pos = 0, .bit_marks1 = 0}

/**
 * @brief Get wrapper related to subsection e.g. Augmenations or Groupings.
 *
 * @code
 * module: <module-name>
 *   +--<node>
 *
 *   augment <target-node>:
 *     +--<node>
 * @endcode
 */
#define TRP_INIT_WRAPPER_BODY \
    (struct trt_wrapper) {.type = TRD_WRAPPER_BODY, .actual_pos = 0, .bit_marks1 = 0}

/**
 * @brief Package which only groups wrapper and indent in node.
 */
struct trt_pck_indent {
    struct trt_wrapper wrapper;         /**< Coded "  |  |  " sequence. */
    struct trt_indent_in_node in_node;  /**< Indent in node. */
};

/**
 * @brief Initialize struct trt_pck_indent by parameters.
 */
#define TRP_INIT_PCK_INDENT(WRAPPER, INDENT_IN_NODE) \
    (struct trt_pck_indent){.wrapper = WRAPPER, .in_node = INDENT_IN_NODE}

/******************************************************************************
 * status
 *****************************************************************************/

/**
 * @brief Status of the node.
 *
 * See: trp_print_status
 */
typedef enum {
    TRD_STATUS_TYPE_EMPTY = 0,
    TRD_STATUS_TYPE_CURRENT,
    TRD_STATUS_TYPE_DEPRECATED,
    TRD_STATUS_TYPE_OBSOLETE
} trt_status_type;

/******************************************************************************
 * flags
 *****************************************************************************/

/**
 * @brief Flag of the node.
 *
 * See: trp_print_flags, trp_get_flags_strlen
 */
typedef enum {
    TRD_FLAGS_TYPE_EMPTY = 0,
    TRD_FLAGS_TYPE_RW,                  /**< rw */
    TRD_FLAGS_TYPE_RO,                  /**< ro */
    TRD_FLAGS_TYPE_RPC_INPUT_PARAMS,    /**< -w */
    TRD_FLAGS_TYPE_USES_OF_GROUPING,    /**< -u */
    TRD_FLAGS_TYPE_RPC,                 /**< -x */
    TRD_FLAGS_TYPE_NOTIF,               /**< -n */
    TRD_FLAGS_TYPE_MOUNT_POINT          /**< mp */
} trt_flags_type;

/******************************************************************************
 * node_name and opts
 *****************************************************************************/

#define TRD_NODE_NAME_PREFIX_CHOICE "("
#define TRD_NODE_NAME_PREFIX_CASE ":("
#define TRD_NODE_NAME_TRIPLE_DOT "..."

/**
 * @brief Type of the node.
 *
 * Used mainly to complete the correct \<opts\> next to or around the \<name\>.
 */
typedef enum {
    TRD_NODE_ELSE = 0,              /**< For some node which does not require special treatment. \<name\> */
    TRD_NODE_CASE,                  /**< For case node. :(\<name\>) */
    TRD_NODE_CHOICE,                /**< For choice node. (\<name\>) */
    TRD_NODE_OPTIONAL_CHOICE,       /**< For choice node with optional mark. (\<name\>)? */
    TRD_NODE_OPTIONAL,              /**< For an optional leaf, anydata, or anyxml. \<name\>? */
    TRD_NODE_CONTAINER,             /**< For a presence container. \<name\>! */
    TRD_NODE_LISTLEAFLIST,          /**< For a leaf-list or list (without keys). \<name\>* */
    TRD_NODE_KEYS,                  /**< For a list's keys. \<name\>* [\<keys\>] */
    TRD_NODE_TOP_LEVEL1,            /**< For a top-level data node in a mounted module. \<name\>/ */
    TRD_NODE_TOP_LEVEL2,            /**< For a top-level data node of a module identified in a mount point parent reference. \<name\>@ */
    TRD_NODE_TRIPLE_DOT             /**< For collapsed sibling nodes and their children. Special case which doesn't belong here very well. */
} trt_node_type;

/**
 * @brief Type of node and his name.
 *
 * See: TRP_EMPTY_NODE_NAME, TRP_NODE_NAME_IS_EMPTY,
 * trp_print_node_name, trp_mark_is_used, trp_print_opts_keys
 */
struct trt_node_name {
    trt_node_type type;         /**< Type of the node relevant for printing. */
    const char *module_prefix;  /**< Prefix defined in the module where the node is defined. */
    const char *str;            /**< Name of the node. */
};

/**
 * @brief Create struct trt_node_name as empty.
 */
#define TRP_EMPTY_NODE_NAME \
    (struct trt_node_name){.type = TRD_NODE_ELSE, .module_prefix = NULL, .str = NULL}

/**
 * @brief Check if struct trt_node_name is empty.
 */
#define TRP_NODE_NAME_IS_EMPTY(NODE_NAME) \
    !NODE_NAME.str

/**< Every opts mark has a length of one. */
#define TRD_OPTS_MARK_LENGTH 1

/******************************************************************************
 * type
 *****************************************************************************/

/**
 * @brief Type of the \<type\>
 */
typedef enum {
    TRD_TYPE_NAME = 0,  /**< Type is just a name that does not require special treatment. */
    TRD_TYPE_TARGET,    /**< Should have a form "-> TARGET", where TARGET is the leafref path. */
    TRD_TYPE_LEAFREF,   /**< This type is set automatically by the 'trp' algorithm. So set type as TRD_TYPE_TARGET. */
    TRD_TYPE_EMPTY      /**< Type is not used at all. */
} trt_type_type;

/**
 * @brief \<type\> in the \<node\>.
 *
 * See: TRP_EMPTY_TRT_TYPE, TRP_TRT_TYPE_IS_EMPTY, trp_print_type
 */
struct trt_type {
    trt_type_type type; /**< Type of the \<type\>. */
    const char *str;    /**< Path or name of the type. */
};

/**
 * @brief Create empty struct trt_type.
 */
#define TRP_EMPTY_TRT_TYPE \
    (struct trt_type) {.type = TRD_TYPE_EMPTY, .str = NULL}

/**
 * @brief Check if struct trt_type is empty.
 */
#define TRP_TRT_TYPE_IS_EMPTY(TYPE_OF_TYPE) \
    TYPE_OF_TYPE.type == TRD_TYPE_EMPTY

/**
 * @brief Initialize struct trt_type by parameters.
 */
#define TRP_INIT_TRT_TYPE(TYPE_OF_TYPE, STRING) \
    (struct trt_type) {.type = TYPE_OF_TYPE, .str = STRING}

/******************************************************************************
 * node
 *****************************************************************************/

/**
 * @brief \<node\> data for printing.
 *
 * It contains RFC's: \<status\>--\<flags\> \<name\>\<opts\> \<type\> \<if-features\>.
 * Item \<opts\> is moved to part struct trt_node_name.
 * For printing [\<keys\>] and if-features is required special functions which prints them.
 *
 * See: TRP_EMPTY_NODE, trp_node_is_empty, trp_node_body_is_empty, trp_print_node_up_to_name,
 * trp_print_divided_node_up_to_name, trp_print_node
 */
struct trt_node {
    trt_status_type status;             /**< \<status\>. */
    trt_flags_type flags;               /**< \<flags\>. */
    struct trt_node_name name;          /**< \<node\> with \<opts\> mark or [\<keys\>]. */
    struct trt_type type;               /**< \<type\> contains the name of the type or type for leafref. */
    ly_bool iffeatures;                 /**< \<if-features\>. Value 1 means that iffeatures are present and will be printed by print_features_names callback. */
    ly_bool last_one;                   /**< Information about whether the node is the last. */
};

/**
 * @brief Create struct trt_node as empty.
 */
#define TRP_EMPTY_NODE \
    (struct trt_node) {.status = TRD_STATUS_TYPE_EMPTY, .flags = TRD_FLAGS_TYPE_EMPTY, \
        .name = TRP_EMPTY_NODE_NAME, .type = TRP_EMPTY_TRT_TYPE, .iffeatures = 0, .last_one = 1}

/**
 * @brief Package which only groups indent and node.
 */
struct trt_pair_indent_node {
    struct trt_indent_in_node indent;
    struct trt_node node;
};

/**
 * @brief Initialize struct trt_pair_indent_node by parameters.
 */
#define TRP_INIT_PAIR_INDENT_NODE(INDENT_IN_NODE, NODE) \
    (struct trt_pair_indent_node){.indent = INDENT_IN_NODE, .node = NODE}

/******************************************************************************
 * statement
 *****************************************************************************/

#define TRD_TOP_KEYWORD_MODULE "module"
#define TRD_TOP_KEYWORD_SUBMODULE "submodule"

#define TRD_BODY_KEYWORD_AUGMENT "augment"
#define TRD_BODY_KEYWORD_RPC "rpcs"
#define TRD_BODY_KEYWORD_NOTIF "notifications"
#define TRD_BODY_KEYWORD_GROUPING "grouping"
#define TRD_BODY_KEYWORD_YANG_DATA "yang-data"

/**
 * @brief Type of the trt_keyword.
 */
typedef enum {
    TRD_KEYWORD_EMPTY = 0,
    TRD_KEYWORD_MODULE,
    TRD_KEYWORD_SUBMODULE,
    TRD_KEYWORD_AUGMENT,
    TRD_KEYWORD_RPC,
    TRD_KEYWORD_NOTIF,
    TRD_KEYWORD_GROUPING,
    TRD_KEYWORD_YANG_DATA
} trt_keyword_type;

/**
 * @brief Main sign of the tree nodes.
 *
 * See: TRP_EMPTY_KEYWORD_STMT, TRP_KEYWORD_STMT_IS_EMPTY
 * trt_print_keyword_stmt_begin, trt_print_keyword_stmt_str,
 * trt_print_keyword_stmt_end, trp_print_keyword_stmt
 * trp_keyword_type_strlen
 *
 */
struct trt_keyword_stmt {
    trt_keyword_type type;      /**< String containing some of the top or body keyword. */
    const char *str;            /**< Name or path, it determines the type. */
};

/**
 * @brief Create struct trt_keyword_stmt as empty.
 */
#define TRP_EMPTY_KEYWORD_STMT \
    (struct trt_keyword_stmt) {.type = TRD_KEYWORD_EMPTY, .str = NULL}

/**
 * @brief Check if struct trt_keyword_stmt is empty.
 */
#define TRP_KEYWORD_STMT_IS_EMPTY(KEYWORD_TYPE) \
    KEYWORD_TYPE.type == TRD_KEYWORD_EMPTY

/**
 * @brief Initialize struct trt_keyword_stmt by parameters.
 */
#define TRP_INIT_KEYWORD_STMT(KEYWORD_TYPE, STRING) \
    (struct trt_keyword_stmt) {.type = KEYWORD_TYPE, .str = STRING}

/******************************************************************************
 * Modify getters
 *****************************************************************************/

struct trt_parent_cache;

/**
 * @brief Functions that change the state of the tree_ctx structure.
 *
 * The 'tro' functions are set here, which provide data for the 'trp' printing functions
 * and are also called from the 'trb' browsing functions when walking through a tree.
 * These callback functions need to be checked or reformulated
 * if changes to the libyang library affect the printing tree.
 * For all, if the value cannot be returned,
 * its empty version obtained by relevant TRP_EMPTY macro is returned.
 */
struct trt_fp_modify_ctx {
    ly_bool (*parent)(struct trt_tree_ctx *);                                           /**< Jump to parent node. Return true if parent exists. */
    void (*first_sibling)(struct trt_tree_ctx *);                                       /**< Jump on the first of the siblings. */
    struct trt_node (*next_sibling)(struct trt_parent_cache, struct trt_tree_ctx *);    /**< Jump to next sibling of the current node. */
    struct trt_node (*next_child)(struct trt_parent_cache, struct trt_tree_ctx *);      /**< Jump to the child of the current node. */
    struct trt_keyword_stmt (*next_augment)(struct trt_tree_ctx *);                     /**< Jump to the augment section. */
    struct trt_keyword_stmt (*get_rpcs)(struct trt_tree_ctx *);                         /**< Jump to the rpcs section. */
    struct trt_keyword_stmt (*get_notifications)(struct trt_tree_ctx *);                /**< Jump to the notifications section. */
    struct trt_keyword_stmt (*next_grouping)(struct trt_tree_ctx *);                    /**< Jump to the grouping section. */
    struct trt_keyword_stmt (*next_yang_data)(struct trt_tree_ctx *);                   /**< Jump to the yang-data section. */
};

/******************************************************************************
 * Read getters
 *****************************************************************************/

/**
 * @brief Functions that do not change the state of the tree_structure.
 *
 * For details see trt_fp_modify_ctx.
 */
struct trt_fp_read {
    struct trt_keyword_stmt (*module_name)(const struct trt_tree_ctx *);                        /**< Get name of the module. */
    struct trt_node (*node)(struct trt_parent_cache, const struct trt_tree_ctx *);              /**< Get current node. */
    ly_bool (*if_sibling_exists)(const struct trt_tree_ctx *);  /**< Check if node's sibling exists. */
};

/******************************************************************************
 * All getters
 *****************************************************************************/

/**
 * @brief A set of all necessary functions that must be provided for the printer.
 */
struct trt_fp_all {
    struct trt_fp_modify_ctx modify;    /**< Function pointers which modify state of trt_tree_ctx. */
    struct trt_fp_read read;            /**< Function pointers which only reads state of trt_tree_ctx. */
    struct trt_fp_print print;          /**< Functions pointers for printing special items in node. */
};

/******************************************************************************
 * Printer context
 *****************************************************************************/

/**
 * @brief Main structure for trp component (printer part).
 */
struct trt_printer_ctx {
    struct ly_out *out;     /**< Handler to printing. */
    struct trt_fp_all fp;   /**< 'tro' functions callbacks. */
    size_t max_line_length; /**< The maximum number of characters that can be
                               printed on one line, including the last. */
};

/******************************************************************************
 * Tro functions
 *****************************************************************************/

/**
 * @brief The name of the section to which the node belongs.
 */
typedef enum {
    TRD_SECT_MODULE = 0,    /**< The node belongs to the "module: <module_name>:" label. */
    TRD_SECT_AUGMENT,       /**< The node belongs to some "augment <target-node>:" label. */
    TRD_SECT_RPCS,          /**< The node belongs to the "rpcs:" label. */
    TRD_SECT_NOTIF,         /**< The node belongs to the "notifications:" label. */
    TRD_SECT_GROUPING,      /**< The node belongs to some "grouping <grouping-name>:" label. */
    TRD_SECT_YANG_DATA      /**< The node belongs to some "yang-data <yang-data-name>:" label. */
} trt_actual_section;

/**
 * @brief Types of nodes that have some effect on their children.
 */
typedef enum {
    TRD_ANCESTOR_ELSE = 0,
    TRD_ANCESTOR_RPC_INPUT,
    TRD_ANCESTOR_RPC_OUTPUT,
    TRD_ANCESTOR_NOTIF
} trt_ancestor_type;

/**
 * @brief Saved information when browsing the tree downwards.
 *
 * This structure helps prevent frequent retrieval of information from the tree.
 * Browsing functions (trb) are designed to preserve this structures during their recursive calls.
 * Browsing functions (trb) do not interfere in any way with this data.
 * This structure is used by Obtaining functions (tro) which, thanks to this structure, can return a node with the correct data.
 * The word parent is in the name, because this data refers to the last parent and at the same time the states of its ancestors data.
 * Only the function jumping on the child (next_child(...)) creates this structure,
 * because the pointer to the current node moves down the tree.
 * It's like passing the genetic code to children.
 * Some data must be inherited and there are two approaches to this problem.
 * Either it will always be determined which inheritance states belong to the current node
 * (which can lead to regular travel to the root node) or the inheritance states will be stored during the recursive calls.
 * So the problem was solved by the second option.
 * Why does the structure contain this data? Because it walks through the lysp tree.
 * In the future, this data may change if another type of tree (such as the lysc tree) is traversed.
 *
 * See: TRO_EMPTY_PARENT_CACHE, tro_parent_cache_for_child
 */
struct trt_parent_cache {
    trt_ancestor_type ancestor;             /**< Some types of nodes have a special effect on their children. */
    uint16_t lys_status;                    /**< Inherited status CURR, DEPRC, OBSLT. */
    uint16_t lys_config;                    /**< Inherited config W or R. */
    const struct lysp_node_list *last_list; /**< The last LYS_LIST passed. */
};

/**
 * @brief Return trt_parent_cache filled with default values.
 */
#define TRP_EMPTY_PARENT_CACHE \
    (struct trt_parent_cache) {.ancestor = TRD_ANCESTOR_ELSE, .lys_status = LYS_STATUS_CURR, \
        .lys_config = LYS_CONFIG_W, .last_list = NULL}

/**
 * @brief Main structure for browsing the libyang tree
 */
struct trt_tree_ctx {
    trt_actual_section section;         /**< To which section pn points. */
    const struct lys_module *module;    /**< Schema tree structures. */
    const struct lysp_node *pn;         /**< Actual pointer to parsed node. */
    const struct lysp_node *tpn;        /**< Pointer to actual top-node. */
};

/**
 * @brief Used for updating trt_tree_ctx
 */
struct trt_tree_ctx_node_patch {
    const struct lysp_node *pn;         /**< Actual pointer to parsed node. */
    const struct lysp_node *tpn;        /**< Pointer to actual top-node. */
};

/**
 * @brief Initialize struct trt_keyword_stmt by parameters.
 */
#define TRP_INIT_TREE_CTX_NODE_PATCH(PN, TPN) \
    (struct trt_tree_ctx_node_patch){.pn = PN, .tpn = TPN}

/** Getter function for tro_lysp_node_charptr function. */
typedef const char *(*trt_get_charptr_func)(const struct lysp_node *pn);

/******************************************************************************
 * Definition of the general Trg functions
 *****************************************************************************/

/**
 * @brief Print a substring but limited to the maximum length.
 * @param[in] str is pointer to source.
 * @param[in] len is number of characters to be printed.
 * @param[in,out] out is output handler.
 * @return str parameter shifted by len.
 */
static const char *
trg_print_substr(const char *str, size_t len, struct ly_out *out)
{
    for (size_t i = 0; i < len; i++) {
        ly_print_(out, "%c", str[0]);
        str++;
    }
    return str;
}

/**
 * @brief Pointer is not NULL and does not point to an empty string.
 * @param[in] str is pointer to string to be checked.
 * @return 1 if str pointing to non empty string otherwise 0.
 */
static ly_bool
trg_charptr_has_data(const char *str)
{
    return (str) && (str[0] != '\0');
}

/**
 * @brief Check if 'word' in 'src' is present where words are delimited by 'delim'.
 * @param[in] src is source where words are separated by delim.
 * @param[in] word to be searched.
 * @param[in] delim is delimiter between words in src.
 * @return 1 if src contains word otherwise 0.
 */
static ly_bool
trg_word_is_present(const char *src, const char *word, char delim)
{
    const char *hit;

    if ((!src) || (src[0] == '\0') || (!word)) {
        return 0;
    }

    hit = strstr(src, word);

    if (hit) {
        /* word was founded at the begin of src
         * OR it match somewhere after delim
         */
        if ((hit == src) || (hit[-1] == delim)) {
            /* end of word was founded at the end of src
             * OR end of word was match somewhere before delim
             */
            char delim_or_end = (hit + strlen(word))[0];
            if ((delim_or_end == '\0') || (delim_or_end == delim)) {
                return 1;
            }
        }
        /* after -> hit is just substr and it's not the whole word */
        /* jump to the next word */
        for ( ; (src[0] != '\0') && (src[0] != delim); src++) {}
        /* skip delim */
        src = src[0] == '\0' ? src : src + 1;
        /* continue with searching */
        return trg_word_is_present(src, word, delim);
    } else {
        return 0;
    }
}

/******************************************************************************
 * Definition of printer functions
 *****************************************************************************/

/**
 * @brief Write callback for ly_out_new_clb function.
 *
 * @param[in] user_data is type of struct ly_out_clb_arg*.
 * @param[in] buf contains input characters
 * @param[in] count is number of characters in buf.
 * @return Number of printed bytes.
 * @return Negative value in case of error.
 */
static ssize_t
trp_ly_out_clb_func(void *user_data, const void *buf, size_t count)
{
    LY_ERR erc = LY_SUCCESS;
    struct ly_out_clb_arg *data = (struct ly_out_clb_arg *)user_data;

    switch (data->mode) {
    case TRD_PRINT:
        erc = ly_write_(data->out, buf, count);
        break;
    case TRD_CHAR_COUNT:
        data->counter = data->counter + count;
        break;
    default:
        break;
    }

    if (erc != LY_SUCCESS) {
        data->last_error = erc;
        return -1;
    } else {
        return count;
    }
}

/**
 * @brief Check that indent in node can be considered as equivalent.
 * @param[in] first is the first indent in node.
 * @param[in] second is the second indent in node.
 * @return 1 if indents are equivalent otherwise 0.
 */
static ly_bool
trp_indent_in_node_are_eq(struct trt_indent_in_node first, struct trt_indent_in_node second)
{
    const ly_bool a = first.type == second.type;
    const ly_bool b = first.btw_name_opts == second.btw_name_opts;
    const ly_bool c = first.btw_opts_type == second.btw_opts_type;
    const ly_bool d = first.btw_type_iffeatures == second.btw_type_iffeatures;

    return a && b && c && d;
}

/**
 * @brief Setting ' ' symbol because node is last sibling.
 * @param[in] wr is wrapper over which the shift operation is to be performed.
 * @return New shifted wrapper.
 */
static struct trt_wrapper
trp_wrapper_set_shift(struct trt_wrapper wr)
{
    assert(wr.actual_pos < 64);
    /* +--<node>
     *    +--<node>
     */
    wr.actual_pos++;
    return wr;
}

/**
 * @brief Setting '|' symbol because node is divided or it is not last sibling.
 * @param[in] wr is source of wrapper.
 * @return New wrapper which is marked at actual position and shifted.
 */
static struct trt_wrapper
trp_wrapper_set_mark(struct trt_wrapper wr)
{
    assert(wr.actual_pos < 64);
    wr.bit_marks1 |= 1U << wr.actual_pos;
    return trp_wrapper_set_shift(wr);
}

/**
 * @brief Setting ' ' symbol if node is last sibling otherwise set '|'.
 * @param[in] wr is actual wrapper.
 * @param[in] last_one is flag. Value 1 saying if the node is the last and has no more siblings.
 * @return New wrapper for the actual node.
 */
static struct trt_wrapper
trp_wrapper_if_last_sibling(struct trt_wrapper wr, ly_bool last_one)
{
    return last_one ? trp_wrapper_set_shift(wr) : trp_wrapper_set_mark(wr);
}

/**
 * @brief Test if the wrappers are equivalent.
 * @param[in] first is the first wrapper.
 * @param[in] second is the second wrapper.
 * @return 1 if the wrappers are equivalent otherwise 0.
 */
static ly_bool
trp_wrapper_eq(struct trt_wrapper first, struct trt_wrapper second)
{
    const ly_bool a = first.type == second.type;
    const ly_bool b = first.bit_marks1 == second.bit_marks1;
    const ly_bool c = first.actual_pos == second.actual_pos;

    return a && b && c;
}

/**
 * @brief Print "  |  " sequence on line.
 * @param[in] wr is wrapper to be printed.
 * @param[in,out] out is output handler.
 */
static void
trp_print_wrapper(struct trt_wrapper wr, struct ly_out *out)
{
    uint32_t lb;

    if (wr.type == TRD_WRAPPER_TOP) {
        lb = TRD_INDENT_LINE_BEGIN;
    } else if (wr.type == TRD_WRAPPER_BODY) {
        lb = TRD_INDENT_LINE_BEGIN * 2;
    } else {
        lb = TRD_INDENT_LINE_BEGIN;
    }

    ly_print_(out, "%*c", lb, ' ');

    if (trp_wrapper_eq(wr, TRP_INIT_WRAPPER_TOP)) {
        return;
    }

    for (uint32_t i = 0; i < wr.actual_pos; i++) {
        /** Test if the bit on the index is set. */
        if ((wr.bit_marks1 >> i) & 1U) {
            ly_print_(out, "|");
        } else {
            ly_print_(out, " ");
        }

        if (i != wr.actual_pos) {
            ly_print_(out, "%*c", TRD_INDENT_BTW_SIBLINGS, ' ');
        }
    }
}

/**
 * @brief Check if struct trt_node is empty.
 * @param[in] node is item to test.
 * @return 1 if node is considered empty otherwise 0.
 */
static ly_bool
trp_node_is_empty(struct trt_node node)
{
    const ly_bool a = !node.iffeatures;
    const ly_bool b = TRP_TRT_TYPE_IS_EMPTY(node.type);
    const ly_bool c = TRP_NODE_NAME_IS_EMPTY(node.name);
    const ly_bool d = node.flags == TRD_FLAGS_TYPE_EMPTY;
    const ly_bool e = node.status == TRD_STATUS_TYPE_EMPTY;

    return a && b && c && d && e;
}

/**
 * @brief Check if [\<keys\>], \<type\> and \<iffeatures\> are empty/not_set.
 * @param[in] node is item to test.
 * @return 1 if node has no \<keys\> \<type\> or \<iffeatures\> otherwise 0.
 */
static ly_bool
trp_node_body_is_empty(struct trt_node node)
{
    const ly_bool a = !node.iffeatures;
    const ly_bool b = TRP_TRT_TYPE_IS_EMPTY(node.type);
    const ly_bool c = node.name.type != TRD_NODE_KEYS;

    return a && b && c;
}

/**
 * @brief Print \<status\> of the node.
 * @param[in] status_type is type of status.
 * @param[in,out] out is output handler.
 */
static void
trp_print_status(trt_status_type status_type, struct ly_out *out)
{
    switch (status_type) {
    case TRD_STATUS_TYPE_CURRENT:
        ly_print_(out, "%c", '+');
        break;
    case TRD_STATUS_TYPE_DEPRECATED:
        ly_print_(out, "%c", 'x');
        break;
    case TRD_STATUS_TYPE_OBSOLETE:
        ly_print_(out, "%c", 'o');
        break;
    default:
        break;
    }
}

/**
 * @brief Print \<flags\>.
 * @param[in] flags_type is type of \<flags\>.
 * @param[in,out] out is output handler.
 */
static void
trp_print_flags(trt_flags_type flags_type, struct ly_out *out)
{
    switch (flags_type) {
    case TRD_FLAGS_TYPE_RW:
        ly_print_(out, "%s", "rw");
        break;
    case TRD_FLAGS_TYPE_RO:
        ly_print_(out, "%s", "ro");
        break;
    case TRD_FLAGS_TYPE_RPC_INPUT_PARAMS:
        ly_print_(out, "%s", "-w");
        break;
    case TRD_FLAGS_TYPE_USES_OF_GROUPING:
        ly_print_(out, "%s", "-u");
        break;
    case TRD_FLAGS_TYPE_RPC:
        ly_print_(out, "%s", "-x");
        break;
    case TRD_FLAGS_TYPE_NOTIF:
        ly_print_(out, "%s", "-n");
        break;
    case TRD_FLAGS_TYPE_MOUNT_POINT:
        ly_print_(out, "%s", "mp");
        break;
    default:
        break;
    }
}

/**
 * @brief Get size of the \<flags\>.
 * @param[in] flags_type is type of \<flags\>.
 * @return 0 if flags_type is not set otherwise 2.
 */
static size_t
trp_get_flags_strlen(trt_flags_type flags_type)
{
    return flags_type == TRD_FLAGS_TYPE_EMPTY ? 0 : 2;
}

/**
 * @brief Print entire struct trt_node_name structure.
 * @param[in] node_name is item to print.
 * @param[in,out] out is output handler.
 */
static void
trp_print_node_name(struct trt_node_name node_name, struct ly_out *out)
{
    const char *mod_prefix;
    const char *colon;
    const char trd_node_name_suffix_choice[] = ")";
    const char trd_node_name_suffix_case[] = ")";
    const char trd_opts_optional[] = "?";        /**< For an optional leaf, choice, anydata, or anyxml. */
    const char trd_opts_container[] = "!";       /**< For a presence container. */
    const char trd_opts_list[] = "*";            /**< For a leaf-list or list. */
    const char trd_opts_slash[] = "/";           /**< For a top-level data node in a mounted module. */
    const char trd_opts_at_sign[] = "@";         /**< For a top-level data node of a module identified in a mount point parent reference. */

    if (TRP_NODE_NAME_IS_EMPTY(node_name)) {
        return;
    }

    if (node_name.module_prefix) {
        mod_prefix = node_name.module_prefix;
        colon = ":";
    } else {
        mod_prefix = "";
        colon = "";
    }

    switch (node_name.type) {
    case TRD_NODE_ELSE:
        ly_print_(out, "%s%s%s", mod_prefix, colon, node_name.str);
        break;
    case TRD_NODE_CASE:
        ly_print_(out, "%s%s%s%s%s", TRD_NODE_NAME_PREFIX_CASE, mod_prefix, colon, node_name.str, trd_node_name_suffix_case);
        break;
    case TRD_NODE_CHOICE:
        ly_print_(out, "%s%s%s%s%s", TRD_NODE_NAME_PREFIX_CHOICE,  mod_prefix, colon, node_name.str, trd_node_name_suffix_choice);
        break;
    case TRD_NODE_OPTIONAL_CHOICE:
        ly_print_(out, "%s%s%s%s%s%s", TRD_NODE_NAME_PREFIX_CHOICE,  mod_prefix, colon, node_name.str, trd_node_name_suffix_choice, trd_opts_optional);
        break;
    case TRD_NODE_OPTIONAL:
        ly_print_(out, "%s%s%s%s", mod_prefix, colon, node_name.str, trd_opts_optional);
        break;
    case TRD_NODE_CONTAINER:
        ly_print_(out, "%s%s%s%s", mod_prefix, colon, node_name.str, trd_opts_container);
        break;
    case TRD_NODE_LISTLEAFLIST:
        ly_print_(out, "%s%s%s%s", mod_prefix, colon, node_name.str, trd_opts_list);
        break;
    case TRD_NODE_KEYS:
        ly_print_(out, "%s%s%s%s", mod_prefix, colon, node_name.str, trd_opts_list);
        break;
    case TRD_NODE_TOP_LEVEL1:
        ly_print_(out, "%s%s%s%s", mod_prefix, colon, node_name.str, trd_opts_slash);
        break;
    case TRD_NODE_TOP_LEVEL2:
        ly_print_(out, "%s%s%s%s", mod_prefix, colon, node_name.str, trd_opts_at_sign);
        break;
    case TRD_NODE_TRIPLE_DOT:
        ly_print_(out, "%s", TRD_NODE_NAME_TRIPLE_DOT);
        break;
    default:
        break;
    }
}

/**
 * @brief Check if mark (?, !, *, /, @) is implicitly contained in struct trt_node_name.
 * @param[in] node_name is structure containing the 'mark'.
 * @return 1 if contain otherwise 0.
 */
static ly_bool
trp_mark_is_used(struct trt_node_name node_name)
{
    if (TRP_NODE_NAME_IS_EMPTY(node_name)) {
        return 0;
    }

    switch (node_name.type) {
    case TRD_NODE_ELSE:
    case TRD_NODE_CASE:
    case TRD_NODE_KEYS:
        return 0;
    default:
        return 1;
    }
}

/**
 * @brief Print opts keys.
 * @param[in] node_name contains type of the node with his name.
 * @param[in] btw_name_opts is number of spaces between name and [keys].
 * @param[in] cf is basically a pointer to the function that prints the keys.
 * @param[in,out] out is output handler.
 */
static void
trp_print_opts_keys(struct trt_node_name node_name, int16_t btw_name_opts, struct trt_cf_print cf, struct ly_out *out)
{
    if (node_name.type != TRD_NODE_KEYS) {
        return;
    }

    /* <name><mark>___<keys>*/
    if (btw_name_opts > 0) {
        ly_print_(out, "%*c", btw_name_opts, ' ');
    }
    ly_print_(out, "[");
    cf.pf(cf.ctx, out);
    ly_print_(out, "]");
}

/**
 * @brief Print entire struct trt_type structure.
 * @param[in] type is item to print.
 * @param[in,out] out is output handler.
 */
static void
trp_print_type(struct trt_type type, struct ly_out *out)
{
    if (TRP_TRT_TYPE_IS_EMPTY(type)) {
        return;
    }

    switch (type.type) {
    case TRD_TYPE_NAME:
        ly_print_(out, "%s", type.str);
        break;
    case TRD_TYPE_TARGET:
        ly_print_(out, "-> %s", type.str);
        break;
    case TRD_TYPE_LEAFREF:
        ly_print_(out, "leafref");
    default:
        break;
    }
}

/**
 * @brief Print all iffeatures of node
 *
 * @param[in] iffeature_flag contains if if-features is present.
 * @param[in] cf is basically a pointer to the function that prints the list of features.
 * @param[in,out] out is output handler.
 */
static void
trp_print_iffeatures(ly_bool iffeature_flag, struct trt_cf_print cf, struct ly_out *out)
{
    if (iffeature_flag) {
        ly_print_(out, "{");
        cf.pf(cf.ctx, out);
        ly_print_(out, "}?");
    }
}

/**
 * @brief Print just \<status\>--\<flags\> \<name\> with opts mark.
 * @param[in] node contains items to print.
 * @param[in] out is output handler.
 */
static void
trp_print_node_up_to_name(struct trt_node node, struct ly_out *out)
{
    if (node.name.type == TRD_NODE_TRIPLE_DOT) {
        trp_print_node_name(node.name, out);
        return;
    }
    /* <status>--<flags> */
    trp_print_status(node.status, out);
    ly_print_(out, "--");
    /* If the node is a case node, there is no space before the <name> */
    /* also case node has no flags. */
    if (node.name.type != TRD_NODE_CASE) {
        trp_print_flags(node.flags, out);
        ly_print_(out, " ");
    }
    /* <name> */
    trp_print_node_name(node.name, out);
}

/**
 * @brief Print alignment (spaces) instead of \<status\>--\<flags\> \<name\> for divided node.
 * @param[in] node contains items to print.
 * @param[in] out is output handler.
 */
static void
trp_print_divided_node_up_to_name(struct trt_node node, struct ly_out *out)
{
    uint32_t space = trp_get_flags_strlen(node.flags);

    if (node.name.type == TRD_NODE_CASE) {
        /* :(<name> */
        space += strlen(TRD_NODE_NAME_PREFIX_CASE);
    } else if (node.name.type == TRD_NODE_CHOICE) {
        /* (<name> */
        space += strlen(TRD_NODE_NAME_PREFIX_CHOICE);
    } else {
        /* _<name> */
        space += strlen(" ");
    }

    /* <name>
     * __
     */
    space += TRD_INDENT_LONG_LINE_BREAK;

    ly_print_(out, "%*c", space, ' ');
}

/**
 * @brief Print struct trt_node structure.
 * @param[in] node is item to print.
 * @param[in] pck package of functions for printing [\<keys\>] and \<iffeatures\>.
 * @param[in] indent is the indent in node.
 * @param[in,out] out is output handler.
 */
static void
trp_print_node(struct trt_node node, struct trt_pck_print pck, struct trt_indent_in_node indent, struct ly_out *out)
{
    ly_bool triple_dot;
    ly_bool divided;
    struct trt_cf_print cf_print_keys;
    struct trt_cf_print cf_print_iffeatures;

    if (trp_node_is_empty(node)) {
        return;
    }

    /* <status>--<flags> <name><opts> <type> <if-features> */
    triple_dot = node.name.type == TRD_NODE_TRIPLE_DOT;
    divided = indent.type == TRD_INDENT_IN_NODE_DIVIDED;

    if (triple_dot) {
        trp_print_node_name(node.name, out);
        return;
    } else if (!divided) {
        trp_print_node_up_to_name(node, out);
    } else {
        trp_print_divided_node_up_to_name(node, out);
    }

    /* <opts> */
    /* <name>___<opts>*/
    cf_print_keys.ctx = pck.tree_ctx;
    cf_print_keys.pf = pck.fps.print_keys;

    trp_print_opts_keys(node.name, indent.btw_name_opts, cf_print_keys, out);

    /* <opts>__<type> */
    if (indent.btw_opts_type > 0) {
        ly_print_(out, "%*c", indent.btw_opts_type, ' ');
    }

    /* <type> */
    trp_print_type(node.type, out);

    /* <type>__<iffeatures> */
    if (indent.btw_type_iffeatures > 0) {
        ly_print_(out, "%*c", indent.btw_type_iffeatures, ' ');
    }

    /* <iffeatures> */
    cf_print_iffeatures.ctx = pck.tree_ctx;
    cf_print_iffeatures.pf = pck.fps.print_features_names;

    trp_print_iffeatures(node.iffeatures, cf_print_iffeatures, out);
}

/**
 * @brief Print .keyword based on .type.
 * @param[in] ks is keyword statement to print.
 * @param[in,out] out is output handler
 */
static void
trt_print_keyword_stmt_begin(struct trt_keyword_stmt ks, struct ly_out *out)
{
    switch (ks.type) {
    case TRD_KEYWORD_MODULE:
        ly_print_(out, "%s: ", TRD_TOP_KEYWORD_MODULE);
        return;
    case TRD_KEYWORD_SUBMODULE:
        ly_print_(out, "%s: ", TRD_TOP_KEYWORD_SUBMODULE);
        return;
    default:
        ly_print_(out, "%*c", TRD_INDENT_LINE_BEGIN, ' ');
        switch (ks.type) {
        case TRD_KEYWORD_AUGMENT:
            ly_print_(out, "%s ", TRD_BODY_KEYWORD_AUGMENT);
            break;
        case TRD_KEYWORD_RPC:
            ly_print_(out, "%s", TRD_BODY_KEYWORD_RPC);
            break;
        case TRD_KEYWORD_NOTIF:
            ly_print_(out, "%s", TRD_BODY_KEYWORD_NOTIF);
            break;
        case TRD_KEYWORD_GROUPING:
            ly_print_(out, "%s ", TRD_BODY_KEYWORD_GROUPING);
            break;
        case TRD_KEYWORD_YANG_DATA:
            ly_print_(out, "%s ", TRD_BODY_KEYWORD_YANG_DATA);
            break;
        default:
            break;
        }
        break;
    }
}

/**
 * @brief Get string length of stored keyword.
 * @param[in] type is type of the keyword statement.
 * @return length of the keyword statement name.
 */
static size_t
trp_keyword_type_strlen(trt_keyword_type type)
{
    switch (type) {
    case TRD_KEYWORD_MODULE:
        return sizeof(TRD_TOP_KEYWORD_MODULE) - 1;
    case TRD_KEYWORD_SUBMODULE:
        return sizeof(TRD_TOP_KEYWORD_SUBMODULE) - 1;
    case TRD_KEYWORD_AUGMENT:
        return sizeof(TRD_BODY_KEYWORD_AUGMENT) - 1;
    case TRD_KEYWORD_RPC:
        return sizeof(TRD_BODY_KEYWORD_RPC) - 1;
    case TRD_KEYWORD_NOTIF:
        return sizeof(TRD_BODY_KEYWORD_NOTIF) - 1;
    case TRD_KEYWORD_GROUPING:
        return sizeof(TRD_BODY_KEYWORD_GROUPING) - 1;
    case TRD_KEYWORD_YANG_DATA:
        return sizeof(TRD_BODY_KEYWORD_YANG_DATA) - 1;
    default:
        return 0;
    }
}

/**
 * @brief Print .str which is string of name or path.
 * @param[in] ks is keyword statement structure.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 */
static void
trt_print_keyword_stmt_str(struct trt_keyword_stmt ks, size_t mll, struct ly_out *out)
{
    uint32_t ind_initial;
    uint32_t ind_divided;
    /* flag if path must be splitted to more lines */
    ly_bool linebreak_was_set;
    /* flag if at least one subpath was printed */
    ly_bool subpath_printed;
    /* the sum of the sizes of the substrings on the current line */
    uint32_t how_far;
    /* pointer to start of the subpath */
    const char *sub_ptr;
    /* size of subpath from sub_ptr */
    size_t sub_len;

    if ((!ks.str) || (ks.str[0] == '\0')) {
        return;
    }

    /* module name cannot be splitted */
    if ((ks.type == TRD_KEYWORD_MODULE) || (ks.type == TRD_KEYWORD_SUBMODULE)) {
        ly_print_(out, "%s", ks.str);
        return;
    }

    /* after -> for trd_keyword_stmt_body do */

    /* set begin indentation */
    ind_initial = TRD_INDENT_LINE_BEGIN + trp_keyword_type_strlen(ks.type) + 1;
    ind_divided = ind_initial + TRD_INDENT_LONG_LINE_BREAK;
    linebreak_was_set = 0;
    subpath_printed = 0;
    how_far = 0;
    sub_ptr = ks.str;
    sub_len = 0;

    while (sub_ptr[0] != '\0') {
        uint32_t ind;
        /* skip slash */
        const char *tmp = sub_ptr[0] == '/' ? sub_ptr + 1 : sub_ptr;
        /* get position of the end of substr */
        tmp = strchr(tmp, '/');
        /* set correct size if this is a last substring */
        sub_len = !tmp ? strlen(sub_ptr) : (size_t)(tmp - sub_ptr);
        /* actualize sum of the substring's sizes on the current line */
        how_far += sub_len;
        /* correction due to colon character if it this is last substring */
        how_far = *(sub_ptr + sub_len) == '\0' ? how_far + 1 : how_far;
        /* choose indentation which depends on
         * whether the string is printed on multiple lines or not
         */
        ind = linebreak_was_set ? ind_divided : ind_initial;
        if (ind + how_far <= mll) {
            /* printing before max line length */
            sub_ptr = trg_print_substr(sub_ptr, sub_len, out);
            subpath_printed = 1;
        } else {
            /* printing on new line */
            if (subpath_printed == 0) {
                /* first subpath is too long but print it at first line anyway */
                sub_ptr = trg_print_substr(sub_ptr, sub_len, out);
                subpath_printed = 1;
                continue;
            }
            ly_print_(out, "\n");
            ly_print_(out, "%*c", ind_divided, ' ');
            linebreak_was_set = 1;
            sub_ptr = trg_print_substr(sub_ptr, sub_len, out);
            how_far = sub_len;
            subpath_printed = 1;
        }
    }
}

/**
 * @brief Print separator based on .type.
 * @param[in] ks is keyword statement structure.
 * @param[in,out] out is output handler.
 */
static void
trt_print_keyword_stmt_end(struct trt_keyword_stmt ks, struct ly_out *out)
{
    if ((ks.type != TRD_KEYWORD_MODULE) && (ks.type != TRD_KEYWORD_SUBMODULE)) {
        ly_print_(out, ":");
    }
}

/**
 * @brief Print entire struct trt_keyword_stmt structure.
 * @param[in] ks is item to print.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 */
static void
trp_print_keyword_stmt(struct trt_keyword_stmt ks, size_t mll, struct ly_out *out)
{
    if (TRP_KEYWORD_STMT_IS_EMPTY(ks)) {
        return;
    }
    trt_print_keyword_stmt_begin(ks, out);
    trt_print_keyword_stmt_str(ks, mll, out);
    trt_print_keyword_stmt_end(ks, out);
}

/******************************************************************************
 * Main trp functions
 *****************************************************************************/

/**
 * @brief Printing one line including wrapper and node which can be incomplete (divided).
 * @param[in] node is \<node\> representation.
 * @param[in] pck contains special printing functions callback.
 * @param[in] indent contains wrapper and indent in node numbers.
 * @param[in,out] out is output handler.
 */
static void
trp_print_line(struct trt_node node, struct trt_pck_print pck, struct trt_pck_indent indent, struct ly_out *out)
{
    trp_print_wrapper(indent.wrapper, out);
    trp_print_node(node, pck, indent.in_node, out);
}

/**
 * @brief Printing one line including wrapper and \<status\>--\<flags\> \<name\>\<option_mark\>.
 * @param[in] node is \<node\> representation.
 * @param[in] wr is wrapper for printing indentation before node.
 * @param[in] out is output handler.
 */
static void
trp_print_line_up_to_node_name(struct trt_node node, struct trt_wrapper wr, struct ly_out *out)
{
    trp_print_wrapper(wr, out);
    trp_print_node_up_to_name(node, out);
}

/**
 * @brief Check if leafref target must be change to string 'leafref' because his target string is too long.
 * @param[in] node containing leafref target.
 * @param[in] wr is wrapper for printing indentation before node.
 * @param[in] mll is max line length.
 * @param[in] out is output handler.
 * @return true if leafref must be changed to string 'leafref'.
 */
static ly_bool
trp_leafref_target_is_too_long(struct trt_node node, struct trt_wrapper wr, size_t mll, struct ly_out *out)
{
    struct ly_out_clb_arg *data;

    if (node.type.type != TRD_TYPE_TARGET) {
        return 0;
    }

    /* set ly_out to counting characters */
    data = out->method.clb.arg;

    data->counter = 0;
    data->mode = TRD_CHAR_COUNT;
    /* count number of printed bytes */
    trp_print_wrapper(wr, out);
    ly_print_(out, "%*c", TRD_INDENT_BTW_SIBLINGS, ' ');
    trp_print_divided_node_up_to_name(node, out);
    data->mode = TRD_PRINT;

    return data->counter + strlen(node.type.str) > mll;
}

/**
 * @brief Get default indent in node based on node values.
 * @param[in] node is \<node\> representation.
 * @return Default indent in node assuming that the node will not be divided.
 */
static struct trt_indent_in_node
trp_default_indent_in_node(struct trt_node node)
{
    struct trt_indent_in_node ret;

    ret.type = TRD_INDENT_IN_NODE_NORMAL;

    /* btw_name_opts */
    ret.btw_name_opts = node.name.type == TRD_NODE_KEYS ? TRD_INDENT_BEFORE_KEYS : 0;

    /* btw_opts_type */
    if (!(TRP_TRT_TYPE_IS_EMPTY(node.type))) {
        ret.btw_opts_type = trp_mark_is_used(node.name) ?
                TRD_INDENT_BEFORE_TYPE - TRD_OPTS_MARK_LENGTH :
                TRD_INDENT_BEFORE_TYPE;
    } else {
        ret.btw_opts_type = 0;
    }

    /* btw_type_iffeatures */
    ret.btw_type_iffeatures = node.iffeatures ? TRD_INDENT_BEFORE_IFFEATURES : 0;

    return ret;
}

/**
 * @brief Setting linebreaks in trt_indent_in_node.
 *
 * The order where the linebreak tag can be placed is from the end.
 *
 * @param[in] indent containing alignment lengths or already linebreak marks.
 * @return indent with a newly placed linebreak tag.
 * @return .type set to TRD_INDENT_IN_NODE_FAILED if it is not possible to place a more linebreaks.
 */
static struct trt_indent_in_node
trp_indent_in_node_place_break(struct trt_indent_in_node indent)
{
    /* somewhere must be set a line break in node */
    struct trt_indent_in_node ret = indent;

    /* gradually break the node from the end */
    if ((indent.btw_type_iffeatures != TRD_LINEBREAK) && (indent.btw_type_iffeatures != 0)) {
        ret.btw_type_iffeatures = TRD_LINEBREAK;
    } else if ((indent.btw_opts_type != TRD_LINEBREAK) && (indent.btw_opts_type != 0)) {
        ret.btw_opts_type = TRD_LINEBREAK;
    } else if ((indent.btw_name_opts != TRD_LINEBREAK) && (indent.btw_name_opts != 0)) {
        /* set line break between name and opts */
        ret.btw_name_opts = TRD_LINEBREAK;
    } else {
        /* it is not possible to place a more line breaks,
         * unfortunately the max_line_length constraint is violated
         */
        ret.type = TRD_INDENT_IN_NODE_FAILED;
    }
    return ret;
}

/**
 * @brief Get the first half of the node based on the linebreak mark.
 *
 * Items in the second half of the node will be empty.
 *
 * @param[in] node the whole \<node\> to be split.
 * @param[in] indent contains information in which part of the \<node\> the first half ends.
 * @return first half of the node, indent is unchanged.
 */
static struct trt_pair_indent_node
trp_first_half_node(struct trt_node node, struct trt_indent_in_node indent)
{
    struct trt_pair_indent_node ret = TRP_INIT_PAIR_INDENT_NODE(indent, node);

    if (indent.btw_name_opts == TRD_LINEBREAK) {
        ret.node.name.type = node.name.type == TRD_NODE_KEYS ? TRD_NODE_LISTLEAFLIST : node.name.type;
        ret.node.type = TRP_EMPTY_TRT_TYPE;
        ret.node.iffeatures = 0;
    } else if (indent.btw_opts_type == TRD_LINEBREAK) {
        ret.node.type = TRP_EMPTY_TRT_TYPE;
        ret.node.iffeatures = 0;
    } else if (indent.btw_type_iffeatures == TRD_LINEBREAK) {
        ret.node.iffeatures = 0;
    }

    return ret;
}

/**
 * @brief Get the second half of the node based on the linebreak mark.
 *
 * Items in the first half of the node will be empty.
 * Indentations belonging to the first node will be reset to zero.
 *
 * @param[in] node the whole \<node\> to be split.
 * @param[in] indent contains information in which part of the \<node\> the second half starts.
 * @return second half of the node, indent is newly set.
 */
static struct trt_pair_indent_node
trp_second_half_node(struct trt_node node, struct trt_indent_in_node indent)
{
    struct trt_pair_indent_node ret = TRP_INIT_PAIR_INDENT_NODE(indent, node);

    if (indent.btw_name_opts < 0) {
        /* Logically, the information up to token <opts> should be deleted,
         * but the the trp_print_node function needs it to create
         * the correct indent.
         */
        ret.indent.btw_name_opts = 0;
        ret.indent.btw_opts_type = TRP_TRT_TYPE_IS_EMPTY(node.type) ? 0 : TRD_INDENT_BEFORE_TYPE;
        ret.indent.btw_type_iffeatures = !node.iffeatures ? 0 : TRD_INDENT_BEFORE_IFFEATURES;
    } else if (indent.btw_opts_type == TRD_LINEBREAK) {
        ret.node.name.type = node.name.type == TRD_NODE_KEYS ? TRD_NODE_LISTLEAFLIST : node.name.type;
        ret.indent.btw_name_opts = 0;
        ret.indent.btw_opts_type = 0;
        ret.indent.btw_type_iffeatures = !node.iffeatures ? 0 : TRD_INDENT_BEFORE_IFFEATURES;
    } else if (indent.btw_type_iffeatures == TRD_LINEBREAK) {
        ret.node.name.type = node.name.type == TRD_NODE_KEYS ? TRD_NODE_LISTLEAFLIST : node.name.type;
        ret.node.type = TRP_EMPTY_TRT_TYPE;
        ret.indent.btw_name_opts = 0;
        ret.indent.btw_opts_type = 0;
        ret.indent.btw_type_iffeatures = 0;
    }
    return ret;
}

/**
 * @brief Get the correct alignment for the node.
 *
 * This function is recursively called itself.
 * It's like a backend function for a function trp_try_normal_indent_in_node.
 *
 * @param[in] node is \<node\> representation.
 * @param[in] pck contains speciall callback functions for printing.
 * @param[in] indent contains wrapper and indent in node numbers.
 * @param[in] mll is max line length.
 * @param[in,out] cnt counting number of characters to print.
 * @param[in,out] out is output handler.
 * @return pair of node and indentation numbers of that node.
 */
static struct trt_pair_indent_node
trp_try_normal_indent_in_node_(struct trt_node node, struct trt_pck_print pck, struct trt_pck_indent indent, size_t mll, size_t *cnt, struct ly_out *out)
{
    struct trt_pair_indent_node ret = TRP_INIT_PAIR_INDENT_NODE(indent.in_node, node);

    trp_print_line(node, pck, indent, out);

    if (*cnt <= mll) {
        /* success */
        return ret;
    } else {
        ret.indent = trp_indent_in_node_place_break(ret.indent);
        if (ret.indent.type != TRD_INDENT_IN_NODE_FAILED) {
            /* erase information in node due to line break */
            ret = trp_first_half_node(node, ret.indent);
            /* check if line fits, recursive call */
            *cnt = 0;
            ret = trp_try_normal_indent_in_node_(ret.node, pck, TRP_INIT_PCK_INDENT(indent.wrapper, ret.indent), mll, cnt, out);
            /* make sure that the result will be with the status divided
             * or eventually with status failed */
            ret.indent.type = ret.indent.type == TRD_INDENT_IN_NODE_FAILED ? TRD_INDENT_IN_NODE_FAILED : TRD_INDENT_IN_NODE_DIVIDED;
        }
        return ret;
    }
}

/**
 * @brief Get the correct alignment for the node.
 *
 * @param[in] node is \<node\> representation.
 * @param[in] pck contains speciall callback functions for printing.
 * @param[in] indent contains wrapper and indent in node numbers.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 * @return .type == TRD_INDENT_IN_NODE_DIVIDED - the node does not fit in the line, some indent variable has negative value as a line break sign.
 * @return .type == TRD_INDENT_IN_NODE_NORMAL - the node fits into the line, all indent variables values has non-negative number.
 * @return .type == TRD_INDENT_IN_NODE_FAILED - the node does not fit into the line, all indent variables has negative or zero values, function failed.
 */
static struct trt_pair_indent_node
trp_try_normal_indent_in_node(struct trt_node node, struct trt_pck_print pck, struct trt_pck_indent indent, size_t mll, struct ly_out *out)
{
    struct trt_pair_indent_node ret = TRP_INIT_PAIR_INDENT_NODE(indent.in_node, node);
    struct ly_out_clb_arg *data;

    /* set ly_out to counting characters */
    data = out->method.clb.arg;

    data->counter = 0;
    data->mode = TRD_CHAR_COUNT;
    ret = trp_try_normal_indent_in_node_(node, pck, indent, mll, &data->counter, out);
    data->mode = TRD_PRINT;

    return ret;
}

/**
 * @brief Auxiliary function for trp_print_entire_node that prints split nodes.
 * @param[in] node is node representation.
 * @param[in] ppck contains speciall callback functions for printing.
 * @param[in] ipck contains wrapper and indent in node numbers.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 */
static void
trp_print_divided_node(struct trt_node node, struct trt_pck_print ppck, struct trt_pck_indent ipck, size_t mll, struct ly_out *out)
{
    ly_bool entire_node_was_printed;
    struct trt_pair_indent_node ind_node = trp_try_normal_indent_in_node(node, ppck, ipck, mll, out);

    if (ind_node.indent.type == TRD_INDENT_IN_NODE_FAILED) {
        /* nothing can be done, continue as usual */
        ind_node.indent.type = TRD_INDENT_IN_NODE_DIVIDED;
    }

    trp_print_line(ind_node.node, ppck, TRP_INIT_PCK_INDENT(ipck.wrapper, ind_node.indent), out);
    entire_node_was_printed = trp_indent_in_node_are_eq(ipck.in_node, ind_node.indent);

    if (!entire_node_was_printed) {
        ly_print_(out, "\n");
        /* continue with second half node */
        ind_node = trp_second_half_node(node, ind_node.indent);
        /* continue with printing node */
        trp_print_divided_node(ind_node.node, ppck, TRP_INIT_PCK_INDENT(ipck.wrapper, ind_node.indent), mll, out);
    } else {
        return;
    }
}

/**
 * @brief Printing of the wrapper and the whole node, which can be divided into several lines.
 * @param[in] node is node representation.
 * @param[in] ppck contains speciall callback functions for printing.
 * @param[in] ipck contains wrapper and indent in node numbers.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 */
static void
trp_print_entire_node(struct trt_node node, struct trt_pck_print ppck, struct trt_pck_indent ipck, size_t mll, struct ly_out *out)
{
    struct trt_pair_indent_node ind_node1;
    struct trt_pair_indent_node ind_node2;
    struct trt_pck_indent tmp;

    if (trp_leafref_target_is_too_long(node, ipck.wrapper, mll, out)) {
        node.type.type = TRD_TYPE_LEAFREF;
    }

    /* check if normal indent is possible */
    ind_node1 = trp_try_normal_indent_in_node(node, ppck, ipck, mll, out);

    if (ind_node1.indent.type == TRD_INDENT_IN_NODE_NORMAL) {
        /* node fits to one line */
        trp_print_line(node, ppck, ipck, out);
    } else if (ind_node1.indent.type == TRD_INDENT_IN_NODE_DIVIDED) {
        /* node will be divided */
        /* print first half */
        tmp = TRP_INIT_PCK_INDENT(ipck.wrapper, ind_node1.indent);
        /* pretend that this is normal node */
        tmp.in_node.type = TRD_INDENT_IN_NODE_NORMAL;

        trp_print_line(ind_node1.node, ppck, tmp, out);
        ly_print_(out, "\n");

        /* continue with second half on new line */
        ind_node2 = trp_second_half_node(node, ind_node1.indent);
        tmp = TRP_INIT_PCK_INDENT(trp_wrapper_if_last_sibling(ipck.wrapper, node.last_one), ind_node2.indent);

        trp_print_divided_node(ind_node2.node, ppck, tmp, mll, out);
    } else if (ind_node1.indent.type == TRD_INDENT_IN_NODE_FAILED) {
        /* node name is too long */
        trp_print_line_up_to_node_name(node, ipck.wrapper, out);

        if (trp_node_body_is_empty(node)) {
            return;
        } else {
            ly_print_(out, "\n");

            ind_node2 = trp_second_half_node(node, ind_node1.indent);
            ind_node2.indent.type = TRD_INDENT_IN_NODE_DIVIDED;
            tmp = TRP_INIT_PCK_INDENT(trp_wrapper_if_last_sibling(ipck.wrapper, node.last_one), ind_node2.indent);

            trp_print_divided_node(ind_node2.node, ppck, tmp, mll, out);
        }

    }
}

/******************************************************************************
 * Definition of Tro reading functions
 *****************************************************************************/

/**
 * @brief Get new trt_parent_cache if we apply the transfer to the child node in the tree.
 * @param[in] ca is parent cache for current node.
 * @param[in] pn is pointer to the current tree node.
 * @return Cache for the current node.
 */
static struct trt_parent_cache
tro_parent_cache_for_child(struct trt_parent_cache ca, const struct lysp_node *pn)
{
    struct trt_parent_cache ret;

    ret.ancestor =
            pn->nodetype & (LYS_INPUT) ? TRD_ANCESTOR_RPC_INPUT :
            pn->nodetype & (LYS_OUTPUT) ? TRD_ANCESTOR_RPC_OUTPUT :
            pn->nodetype & (LYS_NOTIF) ? TRD_ANCESTOR_NOTIF :
            ca.ancestor;

    ret.lys_status =
            pn->flags & (LYS_STATUS_CURR | LYS_STATUS_DEPRC | LYS_STATUS_OBSLT) ? pn->flags :
            ca.lys_status;

    ret.lys_config =
            ca.ancestor == TRD_ANCESTOR_RPC_INPUT ? 0 :     /* because <flags> will be -w */
            ca.ancestor == TRD_ANCESTOR_RPC_OUTPUT ? LYS_CONFIG_R :
            pn->flags & (LYS_CONFIG_R | LYS_CONFIG_W) ? pn->flags :
            ca.lys_config;

    ret.last_list =
            pn->nodetype & (LYS_LIST) ? (struct lysp_node_list *)pn :
            ca.last_list;

    return ret;
}

/**
 * @brief Read next sibling of the current node.
 * @param[in] origin_tc is context of the tree.
 * @return A patch structure that has prepared pointers for updating struct trt_tree_ctx.
 *  If sibling exists then .pn points to him, otherwise is set to NULL.
 *  The .tpn points to its sibling if it exists and if .pn points to the same node as .tpn,
 *  otherwise .tpn value from origin_tc is copied.
 */
static struct trt_tree_ctx_node_patch
tro_read_next_sibling(const struct trt_tree_ctx *origin_tc)
{
    assert(origin_tc && origin_tc->pn);

    struct trt_tree_ctx_node_patch tc = TRP_INIT_TREE_CTX_NODE_PATCH(origin_tc->pn, origin_tc->tpn);

    if (tc.pn->nodetype & (LYS_RPC | LYS_ACTION)) {
        if (tc.tpn == tc.pn) {
            /* just go to the top-node's sibling */
            tc.pn = tc.pn->next;
            tc.tpn = tc.pn;
        } else {
            /* try to go to the notif node as sibling */
            if (!tc.pn->next) {
                tc.pn = (const struct lysp_node *)lysp_node_notifs(tc.pn->parent);
            } else {
                tc.pn = tc.pn->next;
            }
        }
    } else if (tc.pn->nodetype & LYS_INPUT) {
        const struct lysp_node_action *parent = (struct lysp_node_action *)tc.pn->parent;
        /* if output action has data */
        if (parent->output.child) {
            /* then next sibling is output action */
            tc.pn = &parent->output.node;
        } else {
            /* else input action has no sibling */
            tc.pn = NULL;
        }
        /* if current node is output action */
    } else if (tc.pn->nodetype & LYS_OUTPUT) {
        /* then next sibling does not exist */
        tc.pn = NULL;
        /* if current node is notification */
    } else if (tc.pn->nodetype & LYS_NOTIF) {
        if (tc.tpn == tc.pn) {
            tc.pn = tc.pn->next;
            tc.tpn = tc.pn;
        } else {
            tc.pn = tc.pn->next;
        }
    } else {
        /* else actual node is some node with 'next' element */
        if (tc.tpn == tc.pn) {
            tc.tpn = tc.pn->next;
        }
        tc.pn = tc.pn->next;
    }

    return tc;
}

/**
 * @brief Find out if the current node has siblings.
 * @param[in] tc is context of the tree.
 * @return 1 if sibling exists otherwise 0.
 */
static ly_bool
tro_read_if_sibling_exists(const struct trt_tree_ctx *tc)
{
    return tro_read_next_sibling(tc).pn != NULL;
}

/**
 * @brief Check if list statement has keys.
 * @param[in] pn is pointer to the list.
 * @return 1 if has keys, otherwise 0.
 */
static ly_bool
tro_lysp_list_has_keys(const struct lysp_node_list *pn)
{
    return trg_charptr_has_data(pn->key);
}

/**
 * @brief Check if it contains at least one feature.
 * @param[in] iffs is pointer to the if-features.
 * @return 1 if has if-features, otherwise 0.
 */
static ly_bool
tro_lysp_node_to_iffeature(const struct lysp_qname *iffs)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool ret = 0;

    LY_ARRAY_FOR(iffs, u) {
        ret = 1;
        break;
    }
    return ret;
}

/**
 * @brief Find out if leaf is also the key in last list.
 * @param[in] pn is pointer to leaf.
 * @param[in] ca_last_list is pointer to last visited list. Obtained from trt_parent_cache.
 * @return 1 if leaf is also the key, otherwise 0.
 */
static ly_bool
tro_lysp_leaf_is_key(const struct lysp_node *pn, const struct lysp_node_list *ca_last_list)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *)pn;
    const struct lysp_node_list *list = ca_last_list;

    if (!list) {
        return 0;
    }
    return trg_charptr_has_data(list->key) ?
           trg_word_is_present(list->key, leaf->name, ' ') : 0;
}

/**
 * @brief Check if container's type is presence.
 * @param[in] pn is pointer to container.
 * @return 1 if container has presence statement, otherwise 0.
 */
static ly_bool
tro_lysp_container_has_presence(const struct lysp_node *pn)
{
    return trg_charptr_has_data(((struct lysp_node_container *)pn)->presence);
}

/**
 * @brief Get leaflist's path without lysp_node type control.
 * @param[in] pn is pointer to the leaflist.
 */
static const char *
tro_lysp_leaflist_refpath(const struct lysp_node *pn)
{
    const struct lysp_node_leaflist *list = (const struct lysp_node_leaflist *)pn;

    return list->type.path ? list->type.path->expr : NULL;
}

/**
 * @brief Get leaflist's type name without lysp_node type control.
 * @param[in] pn is pointer to the leaflist.
 */
static const char *
tro_lysp_leaflist_type_name(const struct lysp_node *pn)
{
    const struct lysp_node_leaflist *list = (const struct lysp_node_leaflist *)pn;

    return list->type.name;
}

/**
 * @brief Get leaf's path without lysp_node type control.
 * @param[in] pn is pointer to the leaf node.
 */
static const char *
tro_lysp_leaf_refpath(const struct lysp_node *pn)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *)pn;

    return leaf->type.path ? leaf->type.path->expr : NULL;
}

/**
 * @brief Get leaf's type name without lysp_node type control.
 * @param[in] pn is pointer to the leaf's type name.
 */
static const char *
tro_lysp_leaf_type_name(const struct lysp_node *pn)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *)pn;

    return leaf->type.name;
}

/**
 * @brief Get pointer to data using node type specification and getter function.
 *
 * @param[in] flags is node type specification. If it is the correct node, the getter function is called.
 * @param[in] f is getter function which provides the desired char pointer from the structure.
 * @param[in] pn pointer to node.
 * @return NULL if node has wrong type or getter function return pointer to NULL.
 * @return Pointer to desired char pointer obtained from the node.
 */
static const char *
tro_lysp_node_charptr(uint16_t flags, trt_get_charptr_func f, const struct lysp_node *pn)
{
    if (pn->nodetype & flags) {
        const char *ret = f(pn);
        return trg_charptr_has_data(ret) ? ret : NULL;
    } else {
        return NULL;
    }
}

/**
 * @brief Transformation of the lysp flags to Yang tree \<status\>.
 * @param[in] flags is node's flags obtained from the tree.
 */
static trt_status_type
tro_lysp_flags2status(uint16_t flags)
{
    return flags & LYS_STATUS_OBSLT ? TRD_STATUS_TYPE_OBSOLETE :
           flags & LYS_STATUS_DEPRC ? TRD_STATUS_TYPE_DEPRECATED :
           TRD_STATUS_TYPE_CURRENT;
}

/**
 * @brief Transformation of the lysp flags to Yang tree \<flags\> but more specifically 'ro' or 'rw'.
 * @param[in] flags is node's flags obtained from the tree.
 */
static trt_flags_type
tro_lysp_flags2config(uint16_t flags)
{
    return flags & LYS_CONFIG_R ?
           TRD_FLAGS_TYPE_RO : TRD_FLAGS_TYPE_RW;
}

/**
 * @brief Get name of the module.
 * @param[in] tc is context of the tree.
 */
static struct trt_keyword_stmt
tro_read_module_name(const struct trt_tree_ctx *tc)
{
    assert(tc && tc->module && tc->module->name);
    return (struct trt_keyword_stmt) {
               .type = TRD_KEYWORD_MODULE, .str = tc->module->name
    };
}

/**
 * @brief Resolve \<status\> of the current node.
 * @param[in] nodetype is node's type obtained from the tree.
 * @param[in] flags is node's flags obtained from the tree.
 * @param[in] ca_lys_status is inherited status obtained from trt_parent_cache.
 * @return The status type.
 */
static trt_status_type
tro_resolve_status(uint16_t nodetype, uint16_t flags, uint16_t ca_lys_status)
{
    /* LYS_INPUT and LYS_OUTPUT is special case */
    if (nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        return tro_lysp_flags2status(ca_lys_status);
        /* if ancestor's status is deprc or obslt and also node's status is not set */
    } else if ((ca_lys_status & (LYS_STATUS_DEPRC | LYS_STATUS_OBSLT)) && !(flags & (LYS_STATUS_CURR | LYS_STATUS_DEPRC | LYS_STATUS_OBSLT))) {
        /* get ancestor's status */
        return tro_lysp_flags2status(ca_lys_status);
    } else {
        /* else get node's status */
        return tro_lysp_flags2status(flags);
    }
}

/**
 * @brief Resolve \<flags\> of the current node.
 * @param[in] nodetype is node's type obtained from the tree.
 * @param[in] flags is node's flags obtained from the tree.
 * @param[in] ca_ancestor is ancestor type obtained from trt_parent_cache.
 * @param[in] ca_lys_config is inherited config item obtained from trt_parent_cache.
 * @return The flags type.
 */
static trt_flags_type
tro_resolve_flags(uint16_t nodetype, uint16_t flags, trt_ancestor_type ca_ancestor, uint16_t ca_lys_config)
{
    if ((nodetype & LYS_INPUT) || (ca_ancestor == TRD_ANCESTOR_RPC_INPUT)) {
        return TRD_FLAGS_TYPE_RPC_INPUT_PARAMS;
    } else if ((nodetype & LYS_OUTPUT) || (ca_ancestor == TRD_ANCESTOR_RPC_OUTPUT)) {
        return TRD_FLAGS_TYPE_RO;
    } else if (ca_ancestor == TRD_ANCESTOR_NOTIF) {
        return TRD_FLAGS_TYPE_RO;
    } else if (nodetype & LYS_NOTIF) {
        return TRD_FLAGS_TYPE_NOTIF;
    } else if (nodetype & LYS_USES) {
        return TRD_FLAGS_TYPE_USES_OF_GROUPING;
    } else if (nodetype & (LYS_RPC | LYS_ACTION)) {
        return TRD_FLAGS_TYPE_RPC;
        /* if config is not set then look at ancestor's config and get his config */
    } else if (!(flags & (LYS_CONFIG_R | LYS_CONFIG_W))) {
        return tro_lysp_flags2config(ca_lys_config);
    } else {
        return tro_lysp_flags2config(flags);
    }
}

/**
 * @brief Resolve node type of the current node.
 * @param[in] pn is pointer to the current node in the tree.
 * @param[in] ca_last_list is pointer to the last visited list. Obtained from the trt_parent_cache.
 */
static trt_node_type
tro_resolve_node_type(const struct lysp_node *pn, const struct lysp_node_list *ca_last_list)
{
    if (pn->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        return TRD_NODE_ELSE;
    } else if (pn->nodetype & LYS_CASE) {
        return TRD_NODE_CASE;
    } else if ((pn->nodetype & LYS_CHOICE) && !(pn->flags & LYS_MAND_TRUE)) {
        return TRD_NODE_OPTIONAL_CHOICE;
    } else if (pn->nodetype & LYS_CHOICE) {
        return TRD_NODE_CHOICE;
    } else if ((pn->nodetype & LYS_CONTAINER) && (tro_lysp_container_has_presence(pn))) {
        return TRD_NODE_CONTAINER;
    } else if ((pn->nodetype & LYS_LIST) && (tro_lysp_list_has_keys((const struct lysp_node_list *)pn))) {
        return TRD_NODE_KEYS;
    } else if (pn->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        return TRD_NODE_LISTLEAFLIST;
    } else if ((pn->nodetype & (LYS_ANYDATA | LYS_ANYXML)) && !(pn->flags & LYS_MAND_TRUE)) {
        return TRD_NODE_OPTIONAL;
    } else if ((pn->nodetype & LYS_LEAF) && !(pn->flags & LYS_MAND_TRUE) && (!tro_lysp_leaf_is_key(pn, ca_last_list))) {
        return TRD_NODE_OPTIONAL;
    } else {
        return TRD_NODE_ELSE;
    }
}

/**
 * @brief Transformation of current lysp_node to struct trt_node.
 * @param[in] ca contains stored important data when browsing the tree downwards.
 * @param[in] tc is context of the tree.
 */
static struct trt_node
tro_read_node(struct trt_parent_cache ca, const struct trt_tree_ctx *tc)
{
    assert(tc && tc->pn && tc->pn->nodetype != LYS_UNKNOWN);
    const struct lysp_node *pn = tc->pn;
    struct trt_node ret = TRP_EMPTY_NODE;
    const char *tmp;

    /* <status> */
    ret.status = tro_resolve_status(pn->nodetype, pn->flags, ca.lys_status);

    /* TODO: TRD_FLAGS_TYPE_MOUNT_POINT aka "mp" is not supported right now. */
    /* <flags> */
    ret.flags = tro_resolve_flags(pn->nodetype, pn->flags, ca.ancestor, ca.lys_config);

    /* TODO: TRD_NODE_TOP_LEVEL1 aka '/' is not supported right now. */
    /* TODO: TRD_NODE_TOP_LEVEL2 aka '@' is not supported right now. */
    /* set type of the node */
    ret.name.type = tro_resolve_node_type(pn, ca.last_list);

    /* TODO: ret.name.module_prefix is not supported right now. */
    ret.name.module_prefix = NULL;

    /* set node's name */
    ret.name.str = pn->name;

    /* <type> */
    tmp = NULL;

    if ((tmp = tro_lysp_node_charptr(LYS_LEAFLIST, tro_lysp_leaflist_refpath, pn))) {
        ret.type = TRP_INIT_TRT_TYPE(TRD_TYPE_TARGET, tmp);
    } else if ((tmp = tro_lysp_node_charptr(LYS_LEAFLIST, tro_lysp_leaflist_type_name, pn))) {
        ret.type = TRP_INIT_TRT_TYPE(TRD_TYPE_NAME, tmp);
    } else if ((tmp = tro_lysp_node_charptr(LYS_LEAF, tro_lysp_leaf_refpath, pn))) {
        ret.type = TRP_INIT_TRT_TYPE(TRD_TYPE_TARGET, tmp);
    } else if ((tmp = tro_lysp_node_charptr(LYS_LEAF, tro_lysp_leaf_type_name, pn))) {
        ret.type = TRP_INIT_TRT_TYPE(TRD_TYPE_NAME, tmp);
    } else if ((pn->nodetype & LYS_ANYDATA) == LYS_ANYDATA) {
        ret.type = TRP_INIT_TRT_TYPE(TRD_TYPE_NAME, "anydata");
    } else if (pn->nodetype & LYS_ANYXML) {
        ret.type = TRP_INIT_TRT_TYPE(TRD_TYPE_NAME, "anyxml");
    } else {
        ret.type = TRP_EMPTY_TRT_TYPE;
    }

    /* <iffeature> */
    ret.iffeatures = tro_lysp_node_to_iffeature(pn->iffeatures);

    ret.last_one = !tro_read_if_sibling_exists(tc);

    return ret;
}

/******************************************************************************
 * Modify Tro getters
 *****************************************************************************/

/**
 * @brief Change current node pointer to its parent but only if parent exists.
 * @param[in,out] tc is tree context. Contains pointer to the current node.
 * @return 1 if the node had parents and the change was successful.
 * @return 0 if the node did not have parents. The pointer to the current node did not change.
 */
static ly_bool
tro_modi_parent(struct trt_tree_ctx *tc)
{
    assert(tc && tc->pn);
    /* If no parent exists, stay in actual node. */
    if (tc->pn != tc->tpn) {
        tc->pn = tc->pn->parent;
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Change the current node pointer to its child but only if exists.
 * @param[in] ca contains inherited data from ancestors.
 * @param[in,out] tc is context of the tree. Contains pointer to the current node.
 * @return Non-empty \<node\> representation of the current node's child. The tc parameter is modified.
 * @return Empty \<node\> representation if child don't exists. The tc parameter is not modified.
 */
static struct trt_node
tro_modi_next_child(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    assert(tc && tc->pn);

    struct trt_parent_cache new_ca = tro_parent_cache_for_child(ca, tc->pn);

    if (tc->pn->nodetype & (LYS_ACTION | LYS_RPC)) {
        const struct lysp_node_action *act = (const struct lysp_node_action *)tc->pn;
        if (act->input.child) {
            /* go to LYS_INPUT */
            tc->pn = &act->input.node;
            return tro_read_node(new_ca, tc);
        } else if (act->output.child) {
            /* go to LYS_OUTPUT */
            tc->pn = &act->output.node;
            return tro_read_node(new_ca, tc);
        } else {
            /* input action and output action are not set */
            return TRP_EMPTY_NODE;
        }
    } else {
        const struct lysp_node *pn = lysp_node_child(tc->pn);
        if (pn) {
            tc->pn = pn;
            return tro_read_node(new_ca, tc);
        } else {
            /* current node can't have children or has no children */
            /* but maybe has some actions or notifs */
            const struct lysp_node_action *actions = lysp_node_actions(tc->pn);
            const struct lysp_node_notif *notifs = lysp_node_notifs(tc->pn);

            if (actions) {
                tc->pn = (const struct lysp_node *)actions;
                return tro_read_node(new_ca, tc);
            } else if (notifs) {
                tc->pn = (const struct lysp_node *)notifs;
                return tro_read_node(new_ca, tc);
            } else {
                return TRP_EMPTY_NODE;
            }
        }
    }
}

/**
 * @brief Change the current node pointer to the first child of node's parent.
 * If current node is already first sibling/child then nothing will change.
 * @param[in,out] tc is tree context.
 */
static void
tro_modi_first_sibling(struct trt_tree_ctx *tc)
{
    assert(tc && tc->pn && tc->module && tc->module->parsed);

    if (tro_modi_parent(tc)) {
        tro_modi_next_child(TRP_EMPTY_PARENT_CACHE, tc);
    } else {
        /* current node is top-node */

        struct lysp_module *pm = tc->module->parsed;

        switch (tc->section) {
        case TRD_SECT_MODULE:
            tc->pn = pm->data;
            break;
        case TRD_SECT_AUGMENT:
            tc->pn = ((const struct lysp_node_augment *)tc->pn->parent)->child;
            break;
        case TRD_SECT_RPCS:
            tc->pn = (const struct lysp_node *)pm->rpcs;
            break;
        case TRD_SECT_NOTIF:
            tc->pn = (const struct lysp_node *)pm->notifs;
            break;
        case TRD_SECT_GROUPING:
            tc->pn = ((const struct lysp_node_grp *)tc->pn->parent)->child;
            break;
        case TRD_SECT_YANG_DATA:
            /*TODO: yang-data is not supported now */
            break;
        }

        /* update pointer to top-node */
        tc->tpn = tc->pn;
    }
}

/**
 * @brief Change the pointer to the current node to its next sibling only if exists.
 * @param[in] ca contains inherited data from ancestors.
 * @param[in,out] tc is tree context. Contains pointer to the current node.
 * @return Non-empty \<node\> representation if sibling exists. The tc is modified.
 * @return Empty \<node\> representation otherwise. The tc is not modified.
 */
static struct trt_node
tro_modi_next_sibling(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    struct trt_tree_ctx_node_patch patch = tro_read_next_sibling(tc);

    /* if next sibling exists */
    if (patch.pn) {
        /* update trt_tree_ctx */
        tc->pn = patch.pn;
        tc->tpn = patch.tpn;
        return tro_read_node(ca, tc);
    } else {
        return TRP_EMPTY_NODE;
    }
}

/**
 * @brief Get next (or first) augment section if exists.
 * @param[in,out] tc is tree context.
 * @return Section's representation if (next augment) section exists.
 *  The tc is modified and his pointer points to the first node in augment section.
 * @return Empty section structure otherwise.
 */
static struct trt_keyword_stmt
tro_modi_next_augment(struct trt_tree_ctx *tc)
{
    assert(tc && tc->module && tc->module->parsed);
    const struct lysp_node_augment *augs;

    /* if next_augment func was called for the first time */
    if (tc->section != TRD_SECT_AUGMENT) {
        tc->section = TRD_SECT_AUGMENT;
        augs = tc->module->parsed->augments;
    } else {
        /* get augment sibling from top-node pointer */
        augs = (const struct lysp_node_augment *)tc->tpn->parent->next;
    }

    if ((augs) && (augs->child)) {
        tc->pn = augs->child;
        tc->tpn = tc->pn;
        return TRP_INIT_KEYWORD_STMT(TRD_KEYWORD_AUGMENT, augs->nodeid);
    } else {
        return TRP_EMPTY_KEYWORD_STMT;
    }
}

/**
 * @brief Get rpcs section if exists.
 * @param[in,out] tc is tree context.
 * @return Section representation if it exists.
 *  The tc is modified and his pointer points to the first node in rpcs section.
 * @return Empty section representation otherwise.
 */
static struct trt_keyword_stmt
tro_modi_get_rpcs(struct trt_tree_ctx *tc)
{
    assert(tc && tc->module && tc->module->parsed);
    const struct lysp_node_action *actions = tc->module->parsed->rpcs;

    if (actions) {
        tc->section = TRD_SECT_RPCS;
        tc->pn = &actions->node;
        tc->tpn = tc->pn;
        return TRP_INIT_KEYWORD_STMT(TRD_KEYWORD_RPC, NULL);
    } else {
        return TRP_EMPTY_KEYWORD_STMT;
    }
}

/**
 * @brief Get notification section if exists
 * @param[in,out] tc is tree context.
 * @return Section representation if it exists.
 *  The tc is modified and his pointer points to the first node in notification section.
 * @return Empty section representation otherwise.
 */
static struct trt_keyword_stmt
tro_modi_get_notifications(struct trt_tree_ctx *tc)
{
    assert(tc && tc->module && tc->module->parsed);
    const struct lysp_node_notif *notifs = tc->module->parsed->notifs;

    if (notifs) {
        tc->section = TRD_SECT_NOTIF;
        tc->pn = &notifs->node;
        tc->tpn = tc->pn;
        return TRP_INIT_KEYWORD_STMT(TRD_KEYWORD_NOTIF, NULL);
    } else {
        return TRP_EMPTY_KEYWORD_STMT;
    }
}

/**
 * @brief Get next (or first) grouping section if exists
 * @param[in,out] tc is tree context.
 * @return The next (or first) section representation if it exists.
 *  The tc is modified and his pointer points to the first node in this grouping section.
 * @return Empty section representation otherwise.
 */
static struct trt_keyword_stmt
tro_modi_next_grouping(struct trt_tree_ctx *tc)
{
    assert(tc && tc->module && tc->module->parsed);
    const struct lysp_node_grp *grps;

    if (tc->section != TRD_SECT_GROUPING) {
        tc->section = TRD_SECT_GROUPING;
        grps = tc->module->parsed->groupings;
    } else {
        grps = (const struct lysp_node_grp *)tc->tpn->parent->next;
    }

    if ((grps) && (grps->child)) {
        tc->pn = grps->child;
        tc->tpn = tc->pn;
        return TRP_INIT_KEYWORD_STMT(TRD_KEYWORD_GROUPING, grps->name);
    } else {
        return TRP_EMPTY_KEYWORD_STMT;
    }
}

/**
 * @brief Get next yang-data section if exists.
 *
 * Not implemented.
 *
 * @param[in,out] tc is tree context.
 * @return Section representation if it exists.
 * @return Empty section representation otherwise.
 */
static struct trt_keyword_stmt
tro_modi_next_yang_data(struct trt_tree_ctx *tc)
{
    tc->section = TRD_SECT_YANG_DATA;
    /* TODO: yang-data is not supported */
    return TRP_EMPTY_KEYWORD_STMT;
}

/******************************************************************************
 * Print Tro getters
 *****************************************************************************/

/**
 * @brief Print current node's iffeatures.
 * @param[in] tc is tree context.
 * @param[in,out] out is output handler.
 */
static void
tro_print_features_names(const struct trt_tree_ctx *tc, struct ly_out *out)
{
    const struct lysp_qname *iffs = tc->pn->iffeatures;

    LY_ARRAY_COUNT_TYPE i;

    LY_ARRAY_FOR(iffs, i) {
        if (i == 0) {
            ly_print_(out, "%s", iffs[i].str);
        } else {
            ly_print_(out, ",%s", iffs[i].str);
        }
    }

}

/**
 * @brief Print current list's keys.
 *
 * Well, actually printing keys in the lysp_tree is trivial, because char* points to all keys.
 * However, special functions have been reserved for this, because in principle
 * the list of elements can have more implementations.
 *
 * @param[in] tc is tree context.
 * @param[in,out] out is output handler.
 */
static void
tro_print_keys(const struct trt_tree_ctx *tc, struct ly_out *out)
{
    const struct lysp_node *pn = tc->pn;
    const struct lysp_node_list *list;

    if (pn->nodetype != LYS_LIST) {
        return;
    }

    list = (const struct lysp_node_list *)pn;

    if (trg_charptr_has_data(list->key)) {
        ly_print_(out, "%s", list->key);
    }
}

/******************************************************************************
 * Definition of tree browsing functions
 *****************************************************************************/

/**
 * @brief Get size of node name.
 * @param[in] name contains name and mark.
 * @return positive value total size of the node name.
 * @return negative value as an indication that option mark is included in the total size.
 */
static int32_t
trb_strlen_of_name_and_mark(struct trt_node_name name)
{
    size_t name_len = strlen(name.str);

    if ((name.type == TRD_NODE_CHOICE) || (name.type == TRD_NODE_CASE)) {
        /* counting also parentheses */
        name_len += 2;
    }

    return trp_mark_is_used(name) ?
           ((int32_t)(name_len + TRD_OPTS_MARK_LENGTH)) * (-1) :
           (int32_t)name_len;
}

/**
 * @brief Calculate the btw_opts_type indent size for a particular node.
 * @param[in] name is the node for which we get btw_opts_type.
 * @param[in] max_len4all is the maximum value of btw_opts_type that it can have.
 * @return btw_opts_type for node.
 */
static int16_t
trb_calc_btw_opts_type(struct trt_node_name name, int16_t max_len4all)
{
    int32_t name_len;
    int16_t min_len;
    int16_t ret;

    name_len = trb_strlen_of_name_and_mark(name);

    /* negative value indicate that in name is some opt mark */
    min_len = name_len < 0 ?
            TRD_INDENT_BEFORE_TYPE - TRD_OPTS_MARK_LENGTH :
            TRD_INDENT_BEFORE_TYPE;
    ret = abs(max_len4all) - abs(name_len);

    /* correction -> negative indicate that name is too long. */
    return ret < 0 ? min_len : ret;
}

/**
 * @brief Print node.
 *
 * This function is wrapper for trp_print_entire_node function.
 * But difference is that take max_gap_before_type parameter which will be used to set the unified alignment.
 *
 * @param[in] max_gap_before_type is number of indent before \<type\>.
 * @param[in] wr is wrapper for printing indentation before node.
 * @param[in] ca contains inherited data from ancestors.
 * @param[in] pc contains mainly functions for printing.
 * @param[in] tc is tree context.
 */
static void
trb_print_entire_node(uint32_t max_gap_before_type, struct trt_wrapper wr, struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_node node = pc->fp.read.node(ca, tc);
    struct trt_indent_in_node ind = trp_default_indent_in_node(node);

    if ((max_gap_before_type > 0) && (node.type.type != TRD_TYPE_EMPTY)) {
        /* print actual node with unified indent */
        ind.btw_opts_type = trb_calc_btw_opts_type(node.name, max_gap_before_type);
    }
    /* after -> print actual node with default indent */
    trp_print_entire_node(node, TRP_INIT_PCK_PRINT(tc, pc->fp.print),
            TRP_INIT_PCK_INDENT(wr, ind), pc->max_line_length, pc->out);
}

/**
 * @brief Check if parent of the current node is the last of his siblings.
 *
 * To mantain stability use this function only if the current node is the first of the siblings.
 * Side-effect -> current node is set to the first sibling if node has a parent otherwise no side-effect.
 *
 * @param[in] fp contains all 'tro' callback functions.
 * @param[in,out] tc is tree context.
 * @return 1 if parent is last sibling otherwise 0.
 */
static ly_bool
trb_parent_is_last_sibling(struct trt_fp_all fp, struct trt_tree_ctx *tc)
{
    if (fp.modify.parent(tc)) {
        ly_bool ret = fp.read.if_sibling_exists(tc);
        fp.modify.next_child(TRP_EMPTY_PARENT_CACHE, tc);
        return !ret;
    } else {
        return !fp.read.if_sibling_exists(tc);
    }
}

/**
 * @brief Find sibling with the biggest node name and return that size.
 *
 * Side-effect -> Current node is set to the first sibling.
 *
 * @param[in] ca contains inherited data from ancestors.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is tree context.
 * @return positive number lesser than upper_limit as a sign that only the node name is included in the size.
 * @return negative number whose absolute value is less than upper_limit and sign that node name and his opt mark is included in the size.
 */
static int32_t
trb_maxlen_node_name(struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    int32_t ret = 0;

    pc->fp.modify.first_sibling(tc);

    for (struct trt_node node = pc->fp.read.node(ca, tc);
            !trp_node_is_empty(node);
            node = pc->fp.modify.next_sibling(ca, tc)) {
        int32_t maxlen = trb_strlen_of_name_and_mark(node.name);
        ret = abs(maxlen) > abs(ret) ? maxlen : ret;
    }
    pc->fp.modify.first_sibling(tc);
    return ret;
}

/**
 * @brief Find maximal indent between \<opts\> and \<type\> for siblings.
 *
 * Side-effect -> Current node is set to the first sibling.
 *
 * @param[in] ca contains inherited data from ancestors.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is tree context.
 * @return max btw_opts_type value for rest of the siblings
 */
static int16_t
trb_max_btw_opts_type4siblings(struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    int32_t maxlen_node_name = trb_maxlen_node_name(ca, pc, tc);
    int16_t ind_before_type = maxlen_node_name < 0 ?
            TRD_INDENT_BEFORE_TYPE - 1 : /* mark was present */
            TRD_INDENT_BEFORE_TYPE;

    return abs(maxlen_node_name) + ind_before_type;
}

/**
 * @brief Find out if it is possible to unify the alignment before \<type\>.
 *
 * The goal is for all node siblings to have the same alignment for \<type\> as if they were in a column.
 * All siblings who cannot adapt because they do not fit on the line at all are ignored.
 * Side-effect -> Current node is set to the first sibling.
 *
 * @param[in] ca contains inherited data from ancestors.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is tree context.
 * @return 0 if all siblings cannot fit on the line.
 * @return positive number indicating the maximum number of spaces before \<type\> if the length of the node name is 0.
 *  To calculate the btw_opts_type indent size for a particular node, use the trb_calc_btw_opts_type function.
*/
static uint32_t
trb_try_unified_indent(struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    return trb_max_btw_opts_type4siblings(ca, pc, tc);
}

/**
 * @brief For the current node: recursively print all of its child nodes and all of its siblings, including their children.
 *
 * This function is an auxiliary function for trb_print_subtree_nodes.
 * The parent of the current node is expected to exist.
 * Nodes are printed, including unified sibling node alignment (align \<type\> to column).
 * Side-effect -> current node is set to the last sibling.
 *
 * @param[in] wr is wrapper for printing identation before node.
 * @param[in] ca contains inherited data from ancestors.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is tree context.
 */
static void
trb_print_nodes(struct trt_wrapper wr, struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    uint32_t max_gap_before_type;
    ly_bool sibling_flag = 0;
    ly_bool child_flag = 0;

    /* if node is last sibling, then do not add '|' to wrapper */
    wr = trb_parent_is_last_sibling(pc->fp, tc) ?
            trp_wrapper_set_shift(wr) : trp_wrapper_set_mark(wr);

    /* try unified indentation in node */
    max_gap_before_type = trb_try_unified_indent(ca, pc, tc);

    /* print all siblings */
    do {
        struct trt_parent_cache new_ca;
        struct trt_node node;
        /* print linebreak before printing actual node */
        ly_print_(pc->out, "\n");
        /* print node */
        trb_print_entire_node(max_gap_before_type, wr, ca, pc, tc);

        new_ca = tro_parent_cache_for_child(ca, tc->pn);
        /* go to the actual node's child or stay in actual node */
        node = pc->fp.modify.next_child(ca, tc);
        child_flag = !trp_node_is_empty(node);

        if (child_flag) {
            /* print all childs - recursive call */
            trb_print_nodes(wr, new_ca, pc, tc);
            /* get back from child node to actual node */
            pc->fp.modify.parent(tc);
        }

        /* go to the actual node's sibling */
        node = pc->fp.modify.next_sibling(ca, tc);
        sibling_flag = !trp_node_is_empty(node);

        /* go to the next sibling or stay in actual node */
    } while (sibling_flag);
}

/**
 * @brief Print subtree of nodes.
 *
 * The current node is expected to be the root of the subtree.
 * Before root node is no linebreak printing. This must be addressed by the caller.
 * Root node will also be printed. Behind last printed node is no linebreak.
 *
 * @param[in] max_gap_before_type is result from trb_try_unified_indent function for root node. Set parameter to 0 if distance does not matter.
 * @param[in] wr is wrapper saying how deep in the whole tree is the root of the subtree.
 * @param[in] ca is parent_cache from root's parent. If root is top-level node, insert TRP_EMPTY_PARENT_CACHE.
 * @param[in] pc is pointer to the printer (trp) context.
 * @param[in,out] tc is pointer to the tree (tro) context.
 */
static void
trb_print_subtree_nodes(uint32_t max_gap_before_type, struct trt_wrapper wr, struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_parent_cache new_ca;
    struct trt_node node;

    trb_print_entire_node(max_gap_before_type, wr, ca, pc, tc);
    /* go to the actual node's child */
    new_ca = tro_parent_cache_for_child(ca, tc->pn);
    node = pc->fp.modify.next_child(ca, tc);

    if (!trp_node_is_empty(node)) {
        /* print root's nodes */
        trb_print_nodes(wr, new_ca, pc, tc);
        /* get back from child node to actual node */
        pc->fp.modify.parent(tc);
    }
}

/**
 * @brief Get number of siblings.
 *
 * Side-effect -> current node is set to the first sibling.
 *
 * @param[in] fp contains callback functions which modify tree context
 * @param[in,out] tc is the tree context.
 * @return Number of siblings of the current node.
 */
static uint32_t
trb_get_number_of_siblings(struct trt_fp_modify_ctx fp, struct trt_tree_ctx *tc)
{
    uint32_t ret = 1;
    struct trt_node node = TRP_EMPTY_NODE;

    /* including actual node */
    fp.first_sibling(tc);
    while (!trp_node_is_empty(node = fp.next_sibling(TRP_EMPTY_PARENT_CACHE, tc))) {
        ret++;
    }
    fp.first_sibling(tc);
    return ret;
}

/**
 * @brief Print all parents and their children.
 *
 * This function is suitable for printing top-level nodes that do not have ancestors.
 * Function call print_subtree_nodes for all top-level siblings.
 * Use this function after 'module' keyword or 'augment' and so.
 *
 * @param[in] wr_t is type of the wrapper.
 * @param[pc] pc contains mainly functions for printing.
 * @param[in,out] tc is tree context.
 */
static void
trb_print_family_tree(trd_wrapper_type wr_t, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_wrapper wr;
    struct trt_parent_cache ca;
    uint32_t total_parents;
    uint32_t max_gap_before_type;

    wr = wr_t == TRD_WRAPPER_TOP ? TRP_INIT_WRAPPER_TOP : TRP_INIT_WRAPPER_BODY;
    ca = TRP_EMPTY_PARENT_CACHE;
    total_parents = trb_get_number_of_siblings(pc->fp.modify, tc);
    max_gap_before_type = trb_try_unified_indent(ca, pc, tc);

    for (uint32_t i = 0; i < total_parents; i++) {
        ly_print_(pc->out, "\n");
        trb_print_subtree_nodes(max_gap_before_type, wr, ca, pc, tc);
        pc->fp.modify.next_sibling(ca, tc);
    }
}

/******************************************************************************
 * Definition of trm main functions
 *****************************************************************************/

/**
 * @brief General function to prevent repetitiveness code.
 * @param[in] ks is section representation.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_body_section(struct trt_keyword_stmt ks, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    if (TRP_KEYWORD_STMT_IS_EMPTY(ks)) {
        return;
    }
    trp_print_keyword_stmt(ks, pc->max_line_length, pc->out);
    trb_print_family_tree(TRD_WRAPPER_BODY, pc, tc);
}

/**
 * @brief Print 'module' keyword, its name and all nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_module_section(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    trp_print_keyword_stmt(pc->fp.read.module_name(tc), pc->max_line_length, pc->out);
    /* check if module section contains any data */
    if (tc->tpn) {
        trb_print_family_tree(TRD_WRAPPER_TOP, pc, tc);
    }
}

/**
 * @brief For all augment sections: print 'augment' keyword, its target node and all nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_augmentations(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    ly_bool once = 1;

    for (struct trt_keyword_stmt ks = pc->fp.modify.next_augment(tc);
            !(TRP_KEYWORD_STMT_IS_EMPTY(ks));
            ks = pc->fp.modify.next_augment(tc)) {
        if (once) {
            ly_print_(pc->out, "\n");
            ly_print_(pc->out, "\n");
            once = 0;
        } else {
            ly_print_(pc->out, "\n");
        }
        trm_print_body_section(ks, pc, tc);
    }
}

/**
 * @brief For rpcs section: print 'rpcs' keyword and all its nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_rpcs(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_keyword_stmt rpc = pc->fp.modify.get_rpcs(tc);

    if (!(TRP_KEYWORD_STMT_IS_EMPTY(rpc))) {
        ly_print_(pc->out, "\n");
        ly_print_(pc->out, "\n");
        trm_print_body_section(rpc, pc, tc);
    }
}

/**
 * @brief For notifications section: print 'notifications' keyword and all its nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_notifications(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_keyword_stmt notifs = pc->fp.modify.get_notifications(tc);

    if (!(TRP_KEYWORD_STMT_IS_EMPTY(notifs))) {
        ly_print_(pc->out, "\n");
        ly_print_(pc->out, "\n");
        trm_print_body_section(notifs, pc, tc);
    }
}

/**
 * @brief For all grouping sections: print 'grouping' keyword, its name and all nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_groupings(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    ly_bool once = 1;

    for (struct trt_keyword_stmt ks = pc->fp.modify.next_grouping(tc);
            !(TRP_KEYWORD_STMT_IS_EMPTY(ks));
            ks = pc->fp.modify.next_grouping(tc)) {
        if (once) {
            ly_print_(pc->out, "\n");
            ly_print_(pc->out, "\n");
            once = 0;
        } else {
            ly_print_(pc->out, "\n");
        }
        trm_print_body_section(ks, pc, tc);
    }
}

/**
 * @brief For all yang-data sections: print 'yang-data' keyword and all its nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_yang_data(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    ly_bool once = 1;

    for (struct trt_keyword_stmt ks = pc->fp.modify.next_yang_data(tc);
            !(TRP_KEYWORD_STMT_IS_EMPTY(ks));
            ks = pc->fp.modify.next_yang_data(tc)) {
        if (once) {
            ly_print_(pc->out, "\n");
            ly_print_(pc->out, "\n");
            once = 0;
        } else {
            ly_print_(pc->out, "\n");
        }
        trm_print_body_section(ks, pc, tc);
    }
}

/**
 * @brief Print sections module, augment, rpcs, notifications, grouping, yang-data.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_sections(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    trm_print_module_section(pc, tc);

    trm_print_augmentations(pc, tc);

    trm_print_rpcs(pc, tc);

    trm_print_notifications(pc, tc);

    trm_print_groupings(pc, tc);

    trm_print_yang_data(pc, tc);

    ly_print_(pc->out, "\n");
}

/**
 * @brief Set default settings for trt_printer_ctx.
 *
 * Fill trt_printer_ctx so that it will contain all items correctly defined
 * except for max_line_length which is parameters of the printer tree module.
 *
 * @param[in] out is output handler.
 * @param[in] max_line_length is the maximum line length limit that should not be exceeded.
 * @param[in,out] ctx fill structure with default values.
 */
static void
trm_default_printer_ctx(struct ly_out *out, size_t max_line_length, struct trt_printer_ctx *ctx)
{
    ctx->out = out;

    ctx->fp.modify = (struct trt_fp_modify_ctx) {
        .parent = tro_modi_parent,
        .first_sibling = tro_modi_first_sibling,
        .next_sibling = tro_modi_next_sibling,
        .next_child = tro_modi_next_child,
        .next_augment = tro_modi_next_augment,
        .get_rpcs = tro_modi_get_rpcs,
        .get_notifications = tro_modi_get_notifications,
        .next_grouping = tro_modi_next_grouping,
        .next_yang_data = tro_modi_next_yang_data
    };

    ctx->fp.read = (struct trt_fp_read) {
        .module_name = tro_read_module_name,
        .node = tro_read_node,
        .if_sibling_exists = tro_read_if_sibling_exists
    };

    ctx->fp.print = (struct trt_fp_print) {
        .print_features_names = tro_print_features_names,
        .print_keys = tro_print_keys
    };

    ctx->max_line_length = max_line_length;
}

/**
 * @brief Set default settings for trt_tree_ctx.
 *
 * Pointers to current nodes will be set to module data.
 *
 * @param[in] module is pointer to the YANG schema tree structures representing YANG module.
 * @param[in,out] tc fill structure with default values.
 */
static void
trm_default_tree_ctx(const struct lys_module *module, struct trt_tree_ctx *tc)
{
    tc->section = TRD_SECT_MODULE;
    tc->module = module;
    tc->pn = module->parsed->data;
    tc->tpn = module->parsed->data;
}

/******************************************************************************
 * Definition of module interface
 *****************************************************************************/

LY_ERR
tree_print_parsed_module(struct ly_out *out, const struct lys_module *module, uint32_t UNUSED(options), size_t line_length)
{
    struct trt_printer_ctx pc;
    struct trt_tree_ctx tc;
    struct ly_out *new_out;
    LY_ERR erc;
    struct ly_out_clb_arg clb_arg = TRP_INIT_LY_OUT_CLB_ARG(TRD_PRINT, out, 0, LY_SUCCESS);

    if ((erc = ly_out_new_clb(&trp_ly_out_clb_func, &clb_arg, &new_out))) {
        return erc;
    }

    line_length = line_length == 0 ? SIZE_MAX : line_length;
    trm_default_printer_ctx(new_out, line_length, &pc);
    trm_default_tree_ctx(module, &tc);

    trm_print_sections(&pc, &tc);

    ly_out_free(new_out, NULL, 1);

    return clb_arg.last_error;
}

LY_ERR
tree_print_submodule(struct ly_out *UNUSED(out), const struct lys_module *UNUSED(module), const struct lysp_submodule *UNUSED(submodp), uint32_t UNUSED(options), size_t UNUSED(line_length))
// LY_ERR tree_print_submodule(struct ly_out *out, const struct lys_module *module, const struct lysp_submodule *submodp, uint32_t options, size_t line_length)
{
    /** Not implemented right now. */
    return LY_SUCCESS;
}

LY_ERR
tree_print_compiled_node(struct ly_out *UNUSED(out), const struct lysc_node *UNUSED(node), uint32_t UNUSED(options), size_t UNUSED(line_length))
// LY_ERR tree_print_compiled_node(struct ly_out *out, const struct lysc_node *node, uint32_t options, size_t line_length)
{
    /** Not implemented right now. */
    return LY_SUCCESS;
}
