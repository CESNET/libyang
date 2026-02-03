/**
 * @file printer_tree.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief RFC tree printer for libyang data structure
 *
 * Copyright (c) 2015 - 2026 CESNET, z.s.p.o.
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
#include "ly_common.h"
#include "out_internal.h"
#include "plugins_exts.h"
#include "plugins_types.h"
#include "printer_internal.h"
#include "printer_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**********************************************************************
 * ly_out callback
 *********************************************************************/

/**
 * @brief List of available actions.
 */
typedef enum {
    PT_PRINT = 0,  /**< Normal behavior. It just prints. */
    PT_CHAR_COUNT  /**< Characters will be counted instead of printing. */
} pt_ly_out_clb_arg_flag;

/**
 * @brief Structure is passed as 'writeclb' argument
 * to the ::ly_out_new_clb().
 */
struct ly_out_clb_arg {
    pt_ly_out_clb_arg_flag mode;   /**< flag specifying which action to take. */
    struct ly_out *out;             /**< The ly_out pointer delivered to the printer tree module via the main interface. */
    size_t counter;                 /**< Counter of printed characters. */
    LY_ERR last_error;              /**< The last error that occurred. If no error has occurred, it will be ::LY_SUCCESS. */
};

/**
 * @brief Initialize struct ly_out_clb_arg with default settings.
 */
#define PT_INIT_LY_OUT_CLB_ARG(MODE, OUT, COUNTER, LAST_ERROR) \
    (struct ly_out_clb_arg) { \
        .mode = MODE, .out = OUT, \
        .counter = COUNTER, .last_error = LAST_ERROR \
    }

/**********************************************************************
 * Indent
 *********************************************************************/

/** Constant to indicate the need to break a line. */
#define PT_LINEBREAK -1

/**
 * @brief Constants which are defined in the RFC or are observable
 * from the pyang tool.
 */
typedef enum {
    PT_INDENT_EMPTY = 0,               /**< If the node is a case node, there is no space before the \<name\>. */
    PT_INDENT_LONG_LINE_BREAK = 2,     /**< The new line should be indented so that it starts below \<name\> with
                                             a whitespace offset of at least two characters. */
    PT_INDENT_LINE_BEGIN = 2,          /**< Indent below the keyword (module, augment ...).  */
    PT_INDENT_BTW_SIBLINGS = 2,        /**< Indent between | and | characters. */
    PT_INDENT_BEFORE_KEYS = 1,         /**< "..."___\<keys\>. */
    PT_INDENT_BEFORE_TYPE = 4,         /**< "..."___\<type\>, but if mark is set then indent == 3. */
    PT_INDENT_BEFORE_IFFEATURES = 1    /**< "..."___\<iffeatures\>. */
} pt_cnf_indent;

/**
 * @brief Type of indent in node.
 */
typedef enum {
    PT_INDENT_IN_NODE_NORMAL = 0,  /**< Node fits on one line. */
    PT_INDENT_IN_NODE_DIVIDED,     /**< The node must be split into multiple rows. */
    PT_INDENT_IN_NODE_FAILED       /**< Cannot be crammed into one line. The condition for the maximum line length is violated. */
} pt_indent_in_node_type;

/**
 * @brief Records the alignment between the individual
 * elements of the node.
 *
 * @see pt_default_indent_in_node, pt_try_normal_indent_in_node
 */
struct pt_indent_in_node {
    pt_indent_in_node_type type;   /**< Type of indent in node. */
    int16_t btw_name_opts;          /**< Indent between node name and \<opts\>. */
    int16_t btw_opts_type;          /**< Indent between \<opts\> and \<type\>. */
    int16_t btw_type_iffeatures;    /**< Indent between type and features. Ignored if \<type\> missing. */
};

/**
 * @brief Type of wrappers to be printed.
 */
typedef enum {
    PT_WRAPPER_TOP = 0,    /**< Related to the module. */
    PT_WRAPPER_BODY        /**< Related to e.g. Augmentations or Groupings */
} pt_wrapper_type;

/**
 * @brief For resolving sibling symbol ('|') placement.
 *
 * Bit indicates where the sibling symbol must be printed.
 * This place is in multiples of ::PT_INDENT_BTW_SIBLINGS.
 *
 * @see PT_INIT_WRAPPER_TOP, PT_INIT_WRAPPER_BODY,
 * pt_wrapper_set_mark, pt_wrapper_set_shift,
 * pt_wrapper_if_last_sibling, pt_wrapper_eq, pt_print_wrapper
 */
struct pt_wrapper {
    pt_wrapper_type type;  /**< Location of the wrapper. */
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
#define PT_INIT_WRAPPER_TOP \
    (struct pt_wrapper) { \
        .type = PT_WRAPPER_TOP, .actual_pos = 0, .bit_marks1 = 0 \
    }

/**
 * @brief Get wrapper related to subsection
 * e.g. Augmenations or Groupings.
 *
 * @code
 * module: <module-name>
 *   +--<node>
 *
 *   augment <target-node>:
 *     +--<node>
 * @endcode
 */
#define PT_INIT_WRAPPER_BODY \
    (struct pt_wrapper) { \
        .type = PT_WRAPPER_BODY, .actual_pos = 0, .bit_marks1 = 0 \
    }

/**
 * @brief Structure that only groups wrapper and indent in node.
 */
struct pt_indent {
    struct pt_wrapper wrapper;         /**< Coded "  |  |  " sequence. */
    struct pt_indent_in_node in_node;  /**< Indent in node. */
};

/**
 * @brief Initialize struct pt_indent by parameters.
 */
#define PT_INIT_INDENT(WRAPPER, INDENT_IN_NODE) \
    (struct pt_indent){ \
        .wrapper = WRAPPER, .in_node = INDENT_IN_NODE \
    }

/**********************************************************************
 * node
 *********************************************************************/

#define PT_FLAGS_TYPE_EMPTY "--"
#define PT_FLAGS_TYPE_RW "rw"
#define PT_FLAGS_TYPE_RO "ro"
#define PT_FLAGS_TYPE_RPC_INPUT_PARAMS "-w"
#define PT_FLAGS_TYPE_USES_OF_GROUPING "-u"
#define PT_FLAGS_TYPE_RPC "-x"
#define PT_FLAGS_TYPE_NOTIF "-n"
#define PT_FLAGS_TYPE_EXT ""
#define PT_FLAGS_TYPE_MOUNT_POINT "mp"
#define PT_NODE_NAME_PREFIX_CHOICE "("
#define PT_NODE_NAME_PREFIX_CASE ":("
#define PT_NODE_NAME_TRIPLE_DOT "..."
#define PT_STATUS_CURRENT "+"
#define PT_STATUS_DEPRECATED "x"
#define PT_STATUS_OBSOLETE "o"

/**
 * @brief Type of the node.
 *
 * Used mainly to complete the correct \<opts\> next to or
 * around the \<name\>.
 */
typedef enum {
    PT_NODE_ELSE = 0,          /**< For some node which does not require special treatment. \<name\> */
    PT_NODE_CASE,              /**< For case node. :(\<name\>) */
    PT_NODE_CHOICE,            /**< For choice node. (\<name\>) */
    PT_NODE_TRIPLE_DOT         /**< For collapsed sibling nodes and their children. Special case which doesn't belong here very well. */
} pt_node_type;

#define PT_NODE_OPTIONAL "?"            /**< For an optional leaf, anydata, or anyxml. \<name\>? */
#define PT_NODE_CONTAINER "!"           /**< For a presence container. \<name\>! */
#define PT_NODE_LISTLEAFLIST "*"        /**< For a leaf-list or list. \<name\>* */
#define PT_NODE_MOUNTED "/"             /**< For a top-level data node in a mounted module */
#define PT_NODE_MOUNTED_PARENT_REF "@"  /**< for a top-level data node of a module identified
                                             in a mount point parent reference */

/**
 * @brief Type of node and his name.
 *
 * @see PT_EMPTY_NODE_NAME, PT_NODE_NAME_IS_EMPTY,
 * pt_print_node_name, pt_mark_is_used, pt_print_opts_keys
 */
struct pt_node_name {
    pt_node_type type;         /**< Type of the node relevant for printing. */
    ly_bool keys;               /**< Set to 1 if [\<keys\>] are to be printed. Valid for some types only. */
    const char *module_prefix;  /**< If the node is augmented into the tree from another module,
                                     so this is the prefix of that module. */
    const char *str;            /**< Name of the node. */
    const char *opts;           /**< The \<opts\> symbol. */
};

/**
 * @brief Create struct pt_node_name as empty.
 */
#define PT_EMPTY_NODE_NAME \
    (struct pt_node_name) { \
        .type = PT_NODE_ELSE, .keys = 0, .module_prefix = NULL, .str = NULL, .opts = NULL \
    }

/**
 * @brief Check if struct pt_node_name is empty.
 */
#define PT_NODE_NAME_IS_EMPTY(NODE_NAME) \
    !NODE_NAME.str

/**
 * @brief Type of the \<type\>
 */
typedef enum {
    PT_TYPE_NAME = 0,  /**< Type is just a name that does not require special treatment. */
    PT_TYPE_TARGET,    /**< Should have a form "-> TARGET", where TARGET is the leafref path. */
    PT_TYPE_LEAFREF,   /**< leafref abbreviation when ::PT_TYPE_TARGET is too long */
    PT_TYPE_EMPTY      /**< Type is not used at all. */
} pt_lf_type_id;

/**
 * @brief \<type\> in the \<node\>.
 *
 * @see PT_EMPTY_LF_TYPE, PT_LF_TYPE_IS_EMPTY, pt_print_type
 */
struct pt_lf_type {
    pt_lf_type_id type; /**< Type of the \<type\>. */
    const char *str;    /**< Path or name of the type. */
};

/**
 * @brief Create empty struct pt_type.
 */
#define PT_EMPTY_LF_TYPE \
    (struct pt_lf_type) {.type = PT_TYPE_EMPTY, .str = NULL}

/**
 * @brief Check if struct pt_type is empty.
 */
#define PT_LF_TYPE_IS_EMPTY(LF_TYPE) \
    LF_TYPE.type == PT_TYPE_EMPTY

/**
 * @brief Initialize struct pt_type by parameters.
 */
#define PT_INIT_LF_TYPE(LF_TYPE, STRING) \
    (struct pt_lf_type) {.type = LF_TYPE, .str = STRING}

/**
 * @brief If-feature type.
 */
typedef enum {
    PT_IFF_NON_PRESENT = 0,    /**< iffeatures are not present. */
    PT_IFF_PRESENT             /**< iffeatures are present and will be printed. */
} pt_iffeatures_type;

/**
 * @brief \<if-features\>.
 */
struct pt_iffeatures {
    pt_iffeatures_type type;   /**< Type of iffeature. */
};

/**
 * @brief Create empty iffeatures.
 */
#define PT_EMPTY_IFFEATURES \
    (struct pt_iffeatures) {.type = PT_IFF_NON_PRESENT}

/**
 * @brief Check if iffeatures is empty.
 *
 * @param[in] IFF_TYPE value from pt_iffeatures.type.
 * @return 1 if is empty.
 */
#define PT_EMPTY_IFFEATURES_IS_EMPTY(IFF_TYPE) \
    (IFF_TYPE == PT_IFF_NON_PRESENT)

/**
 * @brief \<node\> data for printing.
 *
 * It contains RFC's:
 * \<status\>--\<flags\> \<name\>\<opts\> \<type\> \<if-features\>.
 * Item \<opts\> is moved to part struct pt_node_name.
 * For printing [\<keys\>] and if-features is required special
 * functions which prints them.
 *
 * @see PT_EMPTY_NODE, pt_node_is_empty, pt_node_body_is_empty,
 * pt_print_node_up_to_name, pt_print_divided_node_up_to_name,
 * pt_print_node
 */
struct pt_node {
    const char *status;                 /**< \<status\>. */
    const char *flags;                  /**< \<flags\>. */
    struct pt_node_name name;          /**< \<node\> with \<opts\> mark or [\<keys\>]. */
    struct pt_lf_type type;               /**< \<type\> contains the name of the type or type for leafref. */
    struct pt_iffeatures iffeatures;   /**< \<if-features\>. */
    ly_bool last_one;                   /**< Information about whether the node is the last. */
};

/**
 * @brief Create struct pt_node as empty.
 */
#define PT_EMPTY_NODE \
    (struct pt_node) { \
        .status = NULL, \
        .flags = NULL, \
        .name = PT_EMPTY_NODE_NAME, \
        .type = PT_EMPTY_LF_TYPE, \
        .iffeatures = PT_EMPTY_IFFEATURES, \
        .last_one = 1 \
    }

/**********************************************************************
 * parent cache
 *********************************************************************/

/**
 * @brief Types of nodes that have some effect on their children.
 */
typedef enum {
    PT_ANCESTOR_ELSE = 0,      /**< Everything not listed. */
    PT_ANCESTOR_RPC_INPUT,     /**< ::LYS_INPUT */
    PT_ANCESTOR_RPC_OUTPUT,    /**< ::LYS_OUTPUT */
    PT_ANCESTOR_NOTIF          /**< ::LYS_NOTIF */
} pt_parent_type;

/**
 *
 * @brief Cached information when browsing the lysp tree downwards.
 *
 * Structure should contain the resolved flags of the parent.
 * It prevent frequent retrieval of information from the all parents.
 * Only the function jumping on the child (next_child(...)) deal with
 * the structure when the pointer to the current node moves
 * down in the lysp tree.
 *
 * @see PT_EMPTY_PARENT_CACHE, pt_parent_cache_for_child
 */
struct pt_parent_cache {
    pt_parent_type ancestor; /**< Some types of nodes have a special effect on their children. */
    uint16_t lys_status;        /**< Inherited status CURR, DEPRC, OBSLT. */
    uint16_t lys_config;        /**< Inherited config W or R. */
    const struct lysp_node_list *last_list; /**< The last ::LYS_LIST passed. */
};

/**
 * @brief Return pt_parent_cache filled with default values.
 */
#define PT_EMPTY_PARENT_CACHE \
    (struct pt_parent_cache) { \
        .ancestor = PT_ANCESTOR_ELSE, .lys_status = LYS_STATUS_CURR, \
        .lys_config = LYS_CONFIG_W, .last_list = NULL \
    }

/**********************************************************************
 * statement
 *********************************************************************/

#define PT_KEYWORD_MODULE "module"
#define PT_KEYWORD_SUBMODULE "submodule"
#define PT_KEYWORD_AUGMENT "augment"
#define PT_KEYWORD_RPC "rpcs"
#define PT_KEYWORD_NOTIF "notifications"
#define PT_KEYWORD_GROUPING "grouping"

/**
 * @brief Main sign of the tree nodes.
 *
 * @see PT_EMPTY_KEYWORD_STMT, PT_KEYWORD_STMT_IS_EMPTY
 * pt_print_keyword_stmt_begin, pt_print_keyword_stmt_str,
 * pt_print_keyword_stmt_end, pt_print_keyword_stmt
 */
struct pt_keyword_stmt {
    const char *section_name;   /**< String containing section name. */
    const char *argument;       /**< Name or path located behind section name. */
    ly_bool has_node;           /**< Flag if section has any nodes. */
};

/**
 * @brief Create struct pt_keyword_stmt as empty.
 */
#define PT_EMPTY_KEYWORD_STMT \
    (struct pt_keyword_stmt) {.section_name = NULL, .argument = NULL, .has_node = 0}

/**
 * @brief The name of the section to which the node belongs.
 */
typedef enum {
    PT_SECT_MODULE = 0,    /**< The node belongs to the "module: <module_name>:" label. */
    PT_SECT_AUGMENT,       /**< The node belongs to some "augment <target-node>:" label. */
    PT_SECT_RPCS,          /**< The node belongs to the "rpcs:" label. */
    PT_SECT_NOTIF,         /**< The node belongs to the "notifications:" label. */
    PT_SECT_GROUPING,      /**< The node belongs to some "grouping <grouping-name>:" label. */
    PT_SECT_PLUG_DATA      /**< The node belongs to some plugin section. */
} pt_current_section;

/**********************************************************************
 * extension
 *********************************************************************/

/**
 * @brief Type of extension to print.
 */
typedef enum {
    PT_EXT_GENERIC,
    PT_EXT_SCHEMA_MOUNT,        /**< schema-mount subtree '\<node_name\>/', RFC 8340 */
    PT_EXT_SCHEMA_MOUNT_REF     /**< schema-mount parent reference subtree '\<node_name\>@' , RFC 8340 */
} pt_extension_type;

/**
 * @brief Extension schema.
 */
struct pt_ext_tree_schema {
    pt_extension_type ext;  /**< extension type */
    ly_bool compiled;       /**< Flag if it is a compiled schema. */

    union {
        const struct lysc_node *ctree;  /**< Compiled schema. */
        const struct lysp_node *ptree;  /**< Parsed schema. */
    };
};

/**
 * @brief Schema mount structure.
 */
struct pt_ext_schema_mount {
    const struct lysc_node *mount_point;    /**< mount-point (--mp node) */
    struct pt_ext_tree_schema *schemas;     /**< Parsed or compiled schemas ([sized array](@ref sizedarrays)) */
    struct ly_set *parent_refs;             /**< Set of lysc nodes matching parent-reference XPaths. */
    ly_bool mp_has_normal_node;             /**< Set to 1 if mount-point node has other node besides schema-mount subtrees. */
};

/**
 * @brief Context for plugin extension.
 */
struct pt_extension {
    struct pt_ext_tree_schema *schema;                          /**< Current schema to print. */
    struct pt_ext_schema_mount *schema_mount;                   /**< Schema-mount data. */
};

/**********************************************************************
 * printer tree context
 *********************************************************************/

/**
 * @brief Main structure for browsing the libyang tree
 */
struct pt_tree_ctx {
    ly_bool lysc_tree;                              /**< The lysc nodes are used for browsing through the tree.
                                                         It is assumed that once set, it does not change.
                                                         If it is true then pt_tree_ctx.pn and
                                                         pt_tree_ctx.tpn are not used.
                                                         If it is false then pt_tree_ctx.cn is not used. */
    pt_current_section section;                     /**< To which section pn points. */
    const struct lysp_module *pmod;                 /**< Parsed YANG schema tree. */
    const struct lysc_module *cmod;                 /**< Compiled YANG schema tree. */
    const struct lysp_node *pn;                     /**< Actual pointer to parsed node. */
    const struct lysc_node *cn;                     /**< Actual pointer to compiled node. */
    LY_ERR last_error;                              /**< Error value during printing. */

    struct pt_extension plugin_ctx;              /**< Context for extension. */

    struct ly_out *out;     /**< Handler to printing. */
    size_t max_line_length; /**< The maximum number of characters that can be
                               printed on one line, including the last. */
};

/**
 * @brief Check if lysp node is available from
 * the current compiled node.
 *
 * Use only if pt_tree_ctx.lysc_tree is set to true.
 */
#define PT_TREE_CTX_LYSP_NODE_PRESENT(CN) \
    (CN->priv)

/**
 * @brief Get lysp_node from pt_tree_ctx.cn.
 *
 * Use only if :PT_TREE_CTX_LYSP_NODE_PRESENT returns true
 * for that node.
 */
#define PT_TREE_CTX_GET_LYSP_NODE(CN) \
    ((const struct lysp_node *)CN->priv)

#define PT_LAST_ARRAY_ITEM(ARR, ITEM) \
    (&ARR[LY_ARRAY_COUNT(ARR) - 1] == ITEM)

#define PT_LAST_SCHEMA_MOUNT(PT_EXTENSION) \
    (PT_EXTENSION.schema ? \
    PT_LAST_ARRAY_ITEM(PT_EXTENSION.schema_mount->schemas, PT_EXTENSION.schema): \
    1)

#define PT_LAST_SCHEMA(PT_EXTENSION) \
    (PT_EXTENSION.schema_mount ? PT_LAST_SCHEMA_MOUNT(PT_EXTENSION) : 1)

/**********************************************************************
 * String functions
 *********************************************************************/

/**
 * @brief Pointer is not NULL and does not point to an empty string.
 * @param[in] str Pointer to string to be checked.
 * @return 1 if str pointing to non empty string otherwise 0.
 */
static ly_bool
pt_charptr_has_data(const char *str)
{
    return (str) && (str[0] != '\0');
}

/**
 * @brief Check if @p word in @p src is present where words are
 * delimited by @p delim.
 * @param[in] src Source where words are separated by @p delim.
 * @param[in] word Word to be searched.
 * @param[in] delim Delimiter between @p words in @p src.
 * @return 1 if src contains @p word otherwise 0.
 */
static ly_bool
pt_word_is_present(const char *src, const char *word, char delim)
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
        return pt_word_is_present(src, word, delim);
    } else {
        return 0;
    }
}

/**********************************************************************
 * '|' wrapper
 *********************************************************************/

/**
 * @brief Setting space character because node is last sibling.
 * @param[in] wr Wrapper over which the shift operation
 * is to be performed.
 * @return New shifted wrapper.
 */
static struct pt_wrapper
pt_wrapper_set_shift(struct pt_wrapper wr)
{
    assert(wr.actual_pos < 64);
    /* +--<node>
     *    +--<node>
     */
    wr.actual_pos++;
    return wr;
}

/**
 * @brief Setting '|' symbol because node is divided or
 * it is not last sibling.
 * @param[in] wr Source of wrapper.
 * @return New wrapper which is marked at actual position and shifted.
 */
static struct pt_wrapper
pt_wrapper_set_mark(struct pt_wrapper wr)
{
    assert(wr.actual_pos < 64);
    wr.bit_marks1 |= 1U << wr.actual_pos;
    return pt_wrapper_set_shift(wr);
}

/**
 * @brief Setting ' ' symbol if node is last sibling otherwise set '|'.
 * @param[in] wr Current wrapper.
 * @param[in] last_one If set to 1 then the node is the last
 * and has no more siblings.
 * @return New wrapper for the actual node.
 */
static struct pt_wrapper
pt_wrapper_if_last_sibling(struct pt_wrapper wr, ly_bool last_one)
{
    return last_one ? pt_wrapper_set_shift(wr) : pt_wrapper_set_mark(wr);
}

/**
 * @brief Test if the wrappers are equivalent.
 * @param[in] first First wrapper.
 * @param[in] second Second wrapper.
 * @return 1 if the wrappers are equivalent otherwise 0.
 */
static ly_bool
pt_wrapper_eq(struct pt_wrapper first, struct pt_wrapper second)
{
    const ly_bool a = first.type == second.type;
    const ly_bool b = first.bit_marks1 == second.bit_marks1;
    const ly_bool c = first.actual_pos == second.actual_pos;

    return a && b && c;
}

/**
 * @brief Print "  |  " sequence on line.
 * @param[in] wr Wrapper to be printed.
 * @param[in,out] out Output handler.
 */
static void
pt_print_wrapper(struct pt_wrapper wr, struct ly_out *out)
{
    uint32_t lb;

    switch (wr.type) {
    case PT_WRAPPER_TOP:
        lb = PT_INDENT_LINE_BEGIN;
        break;
    case PT_WRAPPER_BODY:
        lb = PT_INDENT_LINE_BEGIN * 2;
        break;
    default:
        assert(0);
    }

    ly_print_(out, "%*c", lb, ' ');

    if (pt_wrapper_eq(wr, PT_INIT_WRAPPER_TOP)) {
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
            ly_print_(out, "%*c", PT_INDENT_BTW_SIBLINGS, ' ');
        }
    }
}

/**********************************************************************
 * Alignment of \<type\> for all siblings
 *********************************************************************/

/**
 * @brief Count gap from \<flags\> to \<opts\>.
 * @param[in] node Node in which the calculation is made.
 * @return gap from '--' to \<opts\>.
 */
static uint32_t
pt_gap_to_opts(const struct pt_node *node)
{
    uint32_t len = 0;

    if (node->name.keys) {
        return 0;
    }

    assert(node->flags);
    len += strlen(node->flags);
    /* space between flags and name */
    len += 1;

    switch (node->name.type) {
    case PT_NODE_CASE:
        /* ':' is already counted. Plus parentheses. */
        len += 2;
        break;
    case PT_NODE_CHOICE:
        /* Plus parentheses. */
        len += 2;
        break;
    default:
        break;
    }

    if (node->name.module_prefix) {
        /* prefix_name and ':' */
        len += strlen(node->name.module_prefix) + 1;
    }
    if (node->name.str) {
        len += strlen(node->name.str);
    }
    if (node->name.opts) {
        len += strlen(node->name.opts);
    }

    return len;
}

/**
 * @brief Count gap from \<flags\> to \<type\>.
 * @param[in] node Node in which the calculation is made.
 * @return gap from '--' to \<type\>.
 */
static uint32_t
pt_gap_to_type(const struct pt_node *node)
{
    uint32_t len, opts_len;

    if (node->name.keys) {
        return 0;
    }

    len = pt_gap_to_opts(node);
    /* Gap between opts and type. */
    opts_len = 0;
    opts_len += node->name.opts ? strlen(node->name.opts) : 0;
    if (opts_len >= PT_INDENT_BEFORE_TYPE) {
        /* At least one space should be there. */
        len += 1;
    } else if (node->name.opts) {
        len += PT_INDENT_BEFORE_TYPE - opts_len;
    } else {
        len += PT_INDENT_BEFORE_TYPE;
    }

    return len;
}

/**
 * @brief Calculate the pt_indent_in_node.btw_opts_type indent size
 * for a particular node.
 * @param[in] node Node for which the btw_opts_type is get.
 * @param[in] max_gap_before_type Maximum value of btw_opts_type
 * that it can have.
 * @return Indent between \<opts\> and \<type\> for node.
 */
static int16_t
pt_calc_btw_opts_type(const struct pt_node *node, int16_t max_gap_before_type)
{
    uint32_t to_opts_len;

    to_opts_len = pt_gap_to_opts(node);
    if (to_opts_len == 0) {
        return 1;
    } else {
        return max_gap_before_type - to_opts_len;
    }
}

/**
 * @brief Calculate the wrapper about how deep in the tree the node is.
 * @param[in] wr_in Wrapper to use as a starting point.
 * @param[in] node Node from which to count.
 * @return wrapper for @p node.
 */
static struct pt_wrapper
pt_count_depth(const struct pt_wrapper *wr_in, const struct lysc_node *node)
{
    struct pt_wrapper wr = wr_in ? *wr_in : PT_INIT_WRAPPER_TOP;
    const struct lysc_node *parent;

    if (!node) {
        return wr;
    }

    for (parent = node->parent; parent; parent = parent->parent) {
        wr = pt_wrapper_set_shift(wr);
    }

    return wr;
}

static struct pt_node pt_modi_first_sibling(struct pt_parent_cache ca, struct pt_tree_ctx *tc);

static ly_bool pt_node_is_empty(const struct pt_node *node);

static struct pt_node pt_modi_next_sibling(struct pt_parent_cache ca, struct pt_tree_ctx *tc);

/**
 * @brief For all siblings find maximal space from '--' to \<type\>.
 *
 * Side-effect -> Current node is set to the first sibling.
 *
 * @param[in] ca Contains inherited data from ancestors.
 * @param[in,out] tc Tree context.
 * @return max space.
 */
static uint32_t
pt_max_gap_to_type(struct pt_parent_cache ca, struct pt_tree_ctx *tc)
{
    struct pt_node node;
    int32_t maxlen, len;

    maxlen = 0;
    for (node = pt_modi_first_sibling(ca, tc);
            !pt_node_is_empty(&node);
            node = pt_modi_next_sibling(ca, tc)) {
        len = pt_gap_to_type(&node);
        maxlen = maxlen < len ? len : maxlen;
    }
    pt_modi_first_sibling(ca, tc);

    return maxlen;
}

/**
 * @brief Find out if it is possible to unify
 * the alignment before \<type\>.
 *
 * The goal is for all node siblings to have the same alignment
 * for \<type\> as if they were in a column. All siblings who cannot
 * adapt because they do not fit on the line at all are ignored.
 * Side-effect -> Current node is set to the first sibling.
 *
 * @param[in] ca Contains inherited data from ancestors.
 * @param[in,out] tc Tree context.
 * @return positive number indicating the maximum number of spaces
 * before \<type\> if the length of the flags, node name and opts is 0. To calculate
 * the pt_indent_in_node.btw_opts_type indent size for a particular
 * node, use the ::pt_calc_btw_opts_type().
*/
static uint32_t
pt_try_unified_indent(struct pt_parent_cache ca, struct pt_tree_ctx tc)
{
    return pt_max_gap_to_type(ca, &tc);
}

/**********************************************************************
 * Splitting a node into one or more lines.
 *********************************************************************/

/**
 * @brief Check if mark (?, !, *, /, @) is implicitly contained in
 * struct pt_node_name.
 * @param[in] node_name Structure containing the 'mark'.
 * @return 1 if contain otherwise 0.
 */
static ly_bool
pt_mark_is_used(struct pt_node_name node_name)
{
    if (PT_NODE_NAME_IS_EMPTY(node_name)) {
        return 0;
    } else if (node_name.keys) {
        return 0;
    }

    switch (node_name.type) {
    case PT_NODE_ELSE:
    case PT_NODE_CASE:
        return 0;
    default:
        return node_name.opts ? 1 : 0;
    }
}

/**
 * @brief Get default indent in node based on node values.
 * @param[in] node Node to process.
 * @return Default indent in node assuming that the node
 * will not be divided.
 */
static struct pt_indent_in_node
pt_default_indent_in_node(const struct pt_node *node)
{
    struct pt_indent_in_node ret;
    uint32_t opts_len = 0;

    ret.type = PT_INDENT_IN_NODE_NORMAL;

    /* btw_name_opts */
    ret.btw_name_opts = node->name.keys ? PT_INDENT_BEFORE_KEYS : 0;

    /* btw_opts_type */
    if (!(PT_LF_TYPE_IS_EMPTY(node->type))) {
        if (pt_mark_is_used(node->name)) {
            opts_len += node->name.opts ? strlen(node->name.opts) : 0;
            ret.btw_opts_type = PT_INDENT_BEFORE_TYPE > opts_len ? 1 : PT_INDENT_BEFORE_TYPE - opts_len;
        } else {
            ret.btw_opts_type = PT_INDENT_BEFORE_TYPE;
        }
    } else {
        ret.btw_opts_type = 0;
    }

    /* btw_type_iffeatures */
    ret.btw_type_iffeatures = node->iffeatures.type == PT_IFF_PRESENT ? PT_INDENT_BEFORE_IFFEATURES : 0;

    return ret;
}

/**
 * @brief Setting linebreaks in pt_indent_in_node.
 *
 * The order where the linebreak tag can be placed is from the end.
 *
 * @param[in] indent Indent containing alignment lengths
 * or already linebreak marks.
 * @return indent with a newly placed linebreak tag.
 * @return .type set to PT_INDENT_IN_NODE_FAILED if it is not possible
 * to place a more linebreaks.
 */
static struct pt_indent_in_node
pt_indent_in_node_place_break(struct pt_indent_in_node indent)
{
    /* somewhere must be set a line break in node */
    struct pt_indent_in_node ret = indent;

    /* gradually break the node from the end */
    if ((indent.btw_type_iffeatures != PT_LINEBREAK) && (indent.btw_type_iffeatures != 0)) {
        ret.btw_type_iffeatures = PT_LINEBREAK;
    } else if ((indent.btw_opts_type != PT_LINEBREAK) && (indent.btw_opts_type != 0)) {
        ret.btw_opts_type = PT_LINEBREAK;
    } else if ((indent.btw_name_opts != PT_LINEBREAK) && (indent.btw_name_opts != 0)) {
        /* set line break between name and opts */
        ret.btw_name_opts = PT_LINEBREAK;
    } else {
        /* it is not possible to place a more line breaks,
         * unfortunately the max_line_length constraint is violated
         */
        ret.type = PT_INDENT_IN_NODE_FAILED;
    }
    return ret;
}

/**
 * @brief Set the first half of the node based on the linebreak mark.
 *
 * Items in the second half of the node will be empty.
 *
 * @param[in] indent Indentation in the node.
 * @param[out] node Set where the first half of the \<node\> ends.
 */
static void
pt_first_half_node(const struct pt_indent_in_node *indent, struct pt_node *node)
{
    if (indent->btw_name_opts == PT_LINEBREAK) {
        node->type = PT_EMPTY_LF_TYPE;
        node->iffeatures = PT_EMPTY_IFFEATURES;
    } else if (indent->btw_opts_type == PT_LINEBREAK) {
        node->type = PT_EMPTY_LF_TYPE;
        node->iffeatures = PT_EMPTY_IFFEATURES;
    } else if (indent->btw_type_iffeatures == PT_LINEBREAK) {
        node->iffeatures = PT_EMPTY_IFFEATURES;
    }
}

/**
 * @brief Set the second half of the node based on the linebreak mark.
 *
 * Items in the first half of the node will be empty.
 * Indentations belonging to the first node will be reset to zero.
 *
 * @param[in] indent Indentation in the node.
 * @param[out] node Set where the second half of the \<node\>.
 */
static void
pt_second_half_node(struct pt_indent_in_node *indent, struct pt_node *node)
{
    if (indent->btw_name_opts < 0) {
        /* Logically, the information up to token <opts> should
         * be deleted, but the pt_print_node function needs it to
         * create the correct indent.
         */
        indent->btw_name_opts = 0;
        indent->btw_opts_type = PT_LF_TYPE_IS_EMPTY(node->type) ? 0 : PT_INDENT_BEFORE_TYPE;
        indent->btw_type_iffeatures = node->iffeatures.type == PT_IFF_NON_PRESENT ? 0 : PT_INDENT_BEFORE_IFFEATURES;
    } else if (indent->btw_opts_type == PT_LINEBREAK) {
        indent->btw_name_opts = 0;
        indent->btw_opts_type = 0;
        indent->btw_type_iffeatures = node->iffeatures.type == PT_IFF_NON_PRESENT ? 0 : PT_INDENT_BEFORE_IFFEATURES;
    } else if (indent->btw_type_iffeatures == PT_LINEBREAK) {
        node->type = PT_EMPTY_LF_TYPE;
        indent->btw_name_opts = 0;
        indent->btw_opts_type = 0;
        indent->btw_type_iffeatures = 0;
    }
}

static void pt_print_line(const struct pt_node *node, struct pt_tree_ctx *tc,
        struct pt_indent indent);

/**
 * @brief Get the correct alignment for the node.
 *
 * This function is recursively called itself. It's like a backend
 * function for a function ::pt_try_normal_indent_in_node().
 *
 * @param[in] tc Tree context.
 * @param[in] node Node to print.
 * @param[out] indent Indentation data.
 * @param[in,out] cnt Counting number of characters to print.
 */
static void
pt_try_normal_indent_in_node_(struct pt_tree_ctx *tc, struct pt_node *node,
        struct pt_indent *indent, size_t *cnt)
{
    pt_print_line(node, tc, PT_INIT_INDENT(indent->wrapper, indent->in_node));

    if (*cnt <= tc->max_line_length) {
        /* success */
        return;
    } else {
        indent->in_node = pt_indent_in_node_place_break(indent->in_node);
        if (indent->in_node.type != PT_INDENT_IN_NODE_FAILED) {
            /* erase information in node due to line break */
            pt_first_half_node(&indent->in_node, node);
            /* check if line fits, recursive call */
            *cnt = 0;
            pt_try_normal_indent_in_node_(tc, node, indent, cnt);
            /* make sure that the result will be with the status divided
             * or eventually with status failed */
            indent->in_node.type = indent->in_node.type == PT_INDENT_IN_NODE_FAILED ?
                    PT_INDENT_IN_NODE_FAILED : PT_INDENT_IN_NODE_DIVIDED;
        }
        return;
    }
}

/**
 * @brief Get the correct alignment for the node.
 * @param[in] node Node to process.
 * @param[in] tc Tree context.
 * @param[out] indent Indentation data. If the node does not fit in the line,
 * some indent variable has negative value as a line break sign and therefore
 * ::PT_INDENT_IN_NODE_DIVIDED is set. If the node fits into the line,
 * all indent variables values has non-negative number and therefore
 * ::PT_INDENT_IN_NODE_NORMAL is set. If the node does not fit into the line,
 * all indent variables has negative or zero values, function failed
 * and therefore ::PT_INDENT_IN_NODE_FAILED is set.
 */
static void
pt_try_normal_indent_in_node(struct pt_node *node, struct pt_tree_ctx *tc,
        struct pt_indent *indent)
{
    struct ly_out_clb_arg *data;

    /* set ly_out to counting characters */
    data = tc->out->method.clb.arg;

    data->counter = 0;
    data->mode = PT_CHAR_COUNT;
    pt_try_normal_indent_in_node_(tc, node, indent, &data->counter);
    data->mode = PT_PRINT;
}

/**********************************************************************
 * Print keyword
 *********************************************************************/

/**
 * @brief Print keyword based on pt_keyword_stmt.type.
 * @param[in] ks Section keyword to print.
 * @param[in,out] out Output handler.
 */
static void
pt_print_keyword_stmt_begin(const struct pt_keyword_stmt *ks, struct ly_out *out)
{
    if (!strcmp(ks->section_name, PT_KEYWORD_MODULE) ||
            !strcmp(ks->section_name, PT_KEYWORD_SUBMODULE)) {
        ly_print_(out, "%s: ", ks->section_name);
        return;
    }

    ly_print_(out, "%*c", PT_INDENT_LINE_BEGIN, ' ');
    if (ks->argument) {
        ly_print_(out, "%s ", ks->section_name);
    } else {
        ly_print_(out, "%s", ks->section_name);
    }
}

/**
 * @brief Print pt_keyword_stmt.str which is string of name or path.
 * @param[in] ks Section keyword to print.
 * @param[in,out] out Output handler.
 */
static void
pt_print_keyword_stmt_str(const struct pt_keyword_stmt *ks, struct ly_out *out)
{
    if ((!ks->argument) || (ks->argument[0] == '\0')) {
        return;
    }

    ly_print_(out, "%s", ks->argument);
}

/**
 * @brief Print separator based on pt_keyword_stmt.type
 * @param[in] ks Section keyword to print.
 * @param[in,out] out Output handler.
 */
static void
pt_print_keyword_stmt_end(const struct pt_keyword_stmt *ks, struct ly_out *out)
{
    if (!strcmp(ks->section_name, PT_KEYWORD_MODULE) || !strcmp(ks->section_name, PT_KEYWORD_SUBMODULE)) {
        return;
    } else if (ks->has_node) {
        ly_print_(out, ":");
    }
}

/**
 * @brief Print entire struct pt_keyword_stmt structure.
 *
 * Print section like module <module-name>, augment <target-node>, rpcs:, notification:...
 *
 * @param[in] ks Section keyword to print.
 * @param[in,out] once Flag for extra newline if keyword is printed for the first time.
 * Set to NULL if no newline is desired.
 * @param[in,out] out Printing handler.
 */
static void
pt_print_keyword_stmt(const struct pt_keyword_stmt *ks,
        ly_bool *once, struct ly_out *out)
{
    if (once && *once) {
        ly_print_(out, "\n");
        ly_print_(out, "\n");
        *once = 0;
    } else if (once) {
        ly_print_(out, "\n");
    }
    assert(ks->section_name);
    pt_print_keyword_stmt_begin(ks, out);
    pt_print_keyword_stmt_str(ks, out);
    pt_print_keyword_stmt_end(ks, out);
}

/**********************************************************************
 * Print node
 *********************************************************************/

/**
 * @brief Write callback for ::ly_out_new_clb().
 *
 * @param[in] user_data Type of struct ly_out_clb_arg.
 * @param[in] buf Contains input characters
 * @param[in] count Number of characters in @p buf.
 * @return Number of printed bytes.
 * @return Negative value in case of error.
 */
static ssize_t
pt_ly_out_clb_func(void *user_data, const void *buf, size_t count)
{
    LY_ERR erc = LY_SUCCESS;
    struct ly_out_clb_arg *data = (struct ly_out_clb_arg *)user_data;

    switch (data->mode) {
    case PT_PRINT:
        erc = ly_write_(data->out, buf, count);
        break;
    case PT_CHAR_COUNT:
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
 * @brief Check if struct pt_node is empty.
 * @param[in] node Node to test.
 * @return 1 if node is considered empty otherwise 0.
 */
static ly_bool
pt_node_is_empty(const struct pt_node *node)
{
    const ly_bool a = PT_EMPTY_IFFEATURES_IS_EMPTY(node->iffeatures.type);
    const ly_bool b = PT_LF_TYPE_IS_EMPTY(node->type);
    const ly_bool c = PT_NODE_NAME_IS_EMPTY(node->name);
    const ly_bool d = node->flags == NULL;
    const ly_bool e = node->status == NULL;

    return a && b && c && d && e;
}

/**
 * @brief Check if [\<keys\>], \<type\> and
 * \<iffeatures\> are empty/not_set.
 * @param[in] node Node to test.
 * @return 1 if node has no \<keys\> \<type\> or \<iffeatures\>
 * otherwise 0.
 */
static ly_bool
pt_node_body_is_empty(const struct pt_node *node)
{
    const ly_bool a = PT_EMPTY_IFFEATURES_IS_EMPTY(node->iffeatures.type);
    const ly_bool b = PT_LF_TYPE_IS_EMPTY(node->type);
    const ly_bool c = !node->name.keys;

    return a && b && c;
}

/**
 * @brief Print entire struct pt_node_name structure.
 * @param[in] node_name Node name to print.
 * @param[in,out] out Output handler.
 */
static void
pt_print_node_name(struct pt_node_name node_name, struct ly_out *out)
{
    const char *mod_prefix;
    const char *colon;
    const char pt_node_name_suffix_choice[] = ")";
    const char pt_node_name_suffix_case[] = ")";

    assert(!PT_NODE_NAME_IS_EMPTY(node_name));

    if (node_name.module_prefix) {
        mod_prefix = node_name.module_prefix;
        colon = ":";
    } else {
        mod_prefix = "";
        colon = "";
    }

    switch (node_name.type) {
    case PT_NODE_ELSE:
        ly_print_(out, "%s%s%s", mod_prefix, colon, node_name.str);
        break;
    case PT_NODE_CASE:
        ly_print_(out, "%s%s%s%s%s", PT_NODE_NAME_PREFIX_CASE, mod_prefix, colon, node_name.str, pt_node_name_suffix_case);
        break;
    case PT_NODE_CHOICE:
        ly_print_(out, "%s%s%s%s%s", PT_NODE_NAME_PREFIX_CHOICE,  mod_prefix, colon, node_name.str, pt_node_name_suffix_choice);
        break;
    case PT_NODE_TRIPLE_DOT:
        ly_print_(out, "%s", PT_NODE_NAME_TRIPLE_DOT);
        break;
    default:
        break;
    }

    if (node_name.opts) {
        ly_print_(out, "%s", node_name.opts);
    }
}

/**
 * @brief Transformation of the Schema nodes flags to
 * Tree diagram \<status\>.
 * @param[in] flags Node's flags obtained from the tree.
 * @return Status string.
 */
static char *
pt_flags2status(uint16_t flags)
{
    return flags & LYS_STATUS_OBSLT ? PT_STATUS_OBSOLETE :
           flags & LYS_STATUS_DEPRC ? PT_STATUS_DEPRECATED :
           PT_STATUS_CURRENT;
}

/**
 * @brief Transformation of the Schema nodes flags to Tree diagram
 * \<flags\> but more specifically 'ro' or 'rw'.
 * @param[in] flags Node's flags obtained from the tree.
 * @return Flags string.
 */
static char *
pt_flags2config(uint16_t flags)
{
    return flags & LYS_CONFIG_R ? PT_FLAGS_TYPE_RO :
           flags & LYS_CONFIG_W ? PT_FLAGS_TYPE_RW :
           PT_FLAGS_TYPE_EMPTY;
}

/**
 * @brief Print current list's keys.
 * @param[in] tc Tree context.
 */
static void
pt_print_keys(const struct pt_tree_ctx *tc)
{
    const struct lysp_node_list *list;

    if (tc->lysc_tree) {
        assert(PT_TREE_CTX_LYSP_NODE_PRESENT(tc->cn));
        list = (const struct lysp_node_list *)PT_TREE_CTX_GET_LYSP_NODE(tc->cn);
    } else {
        list = (const struct lysp_node_list *)tc->pn;
    }
    assert(list->nodetype & LYS_LIST);

    if (pt_charptr_has_data(list->key)) {
        ly_print_(tc->out, "%s", list->key);
    }
}

/**
 * @brief Print opts keys.
 * @param[in] node_name Contains type of the node with his name.
 * @param[in] btw_name_opts Number of spaces between name and [keys].
 * @param[in] tc Tree context.
 */
static void
pt_print_opts_keys(struct pt_node_name node_name, int16_t btw_name_opts,
        struct pt_tree_ctx *tc)
{
    if (!node_name.keys) {
        return;
    }

    /* <name><mark>___<keys>*/
    if (btw_name_opts > 0) {
        ly_print_(tc->out, "%*c", btw_name_opts, ' ');
    }
    ly_print_(tc->out, "[");
    pt_print_keys(tc);
    ly_print_(tc->out, "]");
}

/**
 * @brief Print entire struct pt_type structure.
 * @param[in] type Item to print.
 * @param[in,out] out Output handler.
 */
static void
pt_print_type(struct pt_lf_type type, struct ly_out *out)
{
    if (PT_LF_TYPE_IS_EMPTY(type)) {
        return;
    }

    switch (type.type) {
    case PT_TYPE_NAME:
        ly_print_(out, "%s", type.str);
        break;
    case PT_TYPE_TARGET:
        ly_print_(out, "-> %s", type.str);
        break;
    case PT_TYPE_LEAFREF:
        ly_print_(out, "leafref");
    default:
        break;
    }
}

/**
 * @brief Print current node's iffeatures.
 * @param[in] tc Tree context.
 */
static void
pt_print_features_names(const struct pt_tree_ctx *tc)
{
    const struct lysp_qname *iffs;

    if (tc->lysc_tree) {
        assert(PT_TREE_CTX_LYSP_NODE_PRESENT(tc->cn));
        iffs = PT_TREE_CTX_GET_LYSP_NODE(tc->cn)->iffeatures;
    } else {
        iffs = tc->pn->iffeatures;
    }
    LY_ARRAY_COUNT_TYPE i;

    LY_ARRAY_FOR(iffs, i) {
        if (i == 0) {
            ly_print_(tc->out, "%s", iffs[i].str);
        } else {
            ly_print_(tc->out, ",%s", iffs[i].str);
        }
    }

}

/**
 * @brief Print all iffeatures of node
 * @param[in] iff Iffeatures to print.
 * @param[in] tc Tree context.
 */
static void
pt_print_iffeatures(struct pt_iffeatures iff, struct pt_tree_ctx *tc)
{
    if (iff.type == PT_IFF_PRESENT) {
        ly_print_(tc->out, "{");
        pt_print_features_names(tc);
        ly_print_(tc->out, "}?");
    }
}

/**
 * @brief Print just \<status\>--\<flags\> \<name\> with opts mark.
 * @param[in] node Node name to print.
 * @param[in] out Output handler.
 */
static void
pt_print_node_up_to_name(const struct pt_node *node, struct ly_out *out)
{
    if (node->name.type == PT_NODE_TRIPLE_DOT) {
        pt_print_node_name(node->name, out);
        return;
    }
    /* <status>--<flags> */
    ly_print_(out, "%s", node->status);
    ly_print_(out, "--");
    /* If the node is a case node, there is no space before the <name>
     * also case node has no flags.
     */
    if (node->flags && (node->name.type != PT_NODE_CASE)) {
        ly_print_(out, "%s", node->flags);
        ly_print_(out, " ");
    }
    /* <name> */
    pt_print_node_name(node->name, out);
}

/**
 * @brief Print alignment (spaces) instead of
 * \<status\>--\<flags\> \<name\> for divided node.
 * @param[in] node Node to print.
 * @param[in] out Output handler.
 */
static void
pt_print_divided_node_up_to_name(const struct pt_node *node, struct ly_out *out)
{
    uint32_t space = strlen(node->flags);

    if (node->name.type == PT_NODE_CASE) {
        /* :(<name> */
        space += strlen(PT_NODE_NAME_PREFIX_CASE);
    } else if (node->name.type == PT_NODE_CHOICE) {
        /* (<name> */
        space += strlen(PT_NODE_NAME_PREFIX_CHOICE);
    } else {
        /* _<name> */
        space += strlen(" ");
    }

    /* <name>
     * __
     */
    space += PT_INDENT_LONG_LINE_BREAK;

    ly_print_(out, "%*c", space, ' ');
}

/**
 * @brief Print struct pt_node structure.
 * @param[in] node Node to print.
 * @param[in] tc Tree context.
 * @param[in] indent Indent in node.
 */
static void
pt_print_node(const struct pt_node *node, struct pt_tree_ctx *tc,
        struct pt_indent_in_node indent)
{
    ly_bool triple_dot;
    ly_bool divided;

    assert(!pt_node_is_empty(node));

    /* <status>--<flags> <name><opts> <type> <if-features> */
    triple_dot = node->name.type == PT_NODE_TRIPLE_DOT;
    divided = indent.type == PT_INDENT_IN_NODE_DIVIDED;

    if (triple_dot) {
        pt_print_node_name(node->name, tc->out);
        return;
    } else if (!divided) {
        pt_print_node_up_to_name(node, tc->out);
    } else {
        pt_print_divided_node_up_to_name(node, tc->out);
    }

    /* <opts> */
    /* <name>___<opts>*/
    pt_print_opts_keys(node->name, indent.btw_name_opts, tc);

    /* <opts>__<type> */
    if (indent.btw_opts_type > 0) {
        ly_print_(tc->out, "%*c", indent.btw_opts_type, ' ');
    }

    /* <type> */
    pt_print_type(node->type, tc->out);

    /* <type>__<iffeatures> */
    if (indent.btw_type_iffeatures > 0) {
        ly_print_(tc->out, "%*c", indent.btw_type_iffeatures, ' ');
    }

    /* <iffeatures> */
    pt_print_iffeatures(node->iffeatures, tc);
}

/**
 * @brief Printing one line including wrapper and node
 * which can be incomplete (divided).
 * @param[in] node Node to process.
 * @param[in] tc Tree context.
 * @param[in] indent Contains wrapper and indent in node numbers.
 */
static void
pt_print_line(const struct pt_node *node, struct pt_tree_ctx *tc,
        struct pt_indent indent)
{
    pt_print_wrapper(indent.wrapper, tc->out);
    pt_print_node(node, tc, indent.in_node);
}

/**
 * @brief Printing one line including wrapper and
 * \<status\>--\<flags\> \<name\>\<option_mark\>.
 * @param[in] node Node to process.
 * @param[in] wr Wrapper for printing indentation before node.
 * @param[in] out Output handler.
 */
static void
pt_print_line_up_to_node_name(const struct pt_node *node, struct pt_wrapper wr, struct ly_out *out)
{
    pt_print_wrapper(wr, out);
    pt_print_node_up_to_name(node, out);
}

/**
 * @brief Check that indent in node can be considered as equivalent.
 * @param[in] first First indent in node.
 * @param[in] second Second indent in node.
 * @return 1 if indents are equivalent otherwise 0.
 */
static ly_bool
pt_indent_in_node_are_eq(struct pt_indent_in_node first, struct pt_indent_in_node second)
{
    const ly_bool a = first.type == second.type;
    const ly_bool b = first.btw_name_opts == second.btw_name_opts;
    const ly_bool c = first.btw_opts_type == second.btw_opts_type;
    const ly_bool d = first.btw_type_iffeatures == second.btw_type_iffeatures;

    return a && b && c && d;
}

/**
 * @brief Auxiliary function for ::pt_print_entire_node()
 * that prints split nodes.
 * @param[in] node_p Node representation.
 * @param[in] tc Tree context.
 * @param[in] indent_p Contains wrapper and indent in node numbers.
 */
static void
pt_print_divided_node(const struct pt_node *node_p, struct pt_tree_ctx *tc,
        struct pt_indent indent_p)
{
    struct pt_node node;
    struct pt_indent indent;
    ly_bool entire_node_was_printed;

    indent = indent_p;
    node = *node_p;
    pt_try_normal_indent_in_node(&node, tc, &indent);

    if (indent.in_node.type == PT_INDENT_IN_NODE_FAILED) {
        /* nothing can be done, continue as usual */
        indent.in_node.type = PT_INDENT_IN_NODE_DIVIDED;
    }

    pt_print_line(&node, tc, PT_INIT_INDENT(indent_p.wrapper, indent.in_node));
    entire_node_was_printed = pt_indent_in_node_are_eq(indent_p.in_node, indent.in_node);

    if (!entire_node_was_printed) {
        ly_print_(tc->out, "\n");
        /* continue with second half node */
        node = *node_p;
        pt_second_half_node(&indent.in_node, &node);
        /* continue with printing node */
        pt_print_divided_node(&node, tc, PT_INIT_INDENT(indent.wrapper, indent.in_node));
    } else {
        return;
    }
}

/**
 * @brief Printing of the wrapper and the whole node,
 * which can be divided into several lines.
 * @param[in] node_p Node to print.
 * @param[in] tc Tree context.
 * @param[in] indent_p Contains wrapper and indent in node numbers.
 */
static void
pt_print_entire_node_(const struct pt_node *node_p, struct pt_tree_ctx *tc,
        struct pt_indent indent_p)
{
    struct pt_wrapper wr;
    struct pt_node node_try;
    struct pt_indent indent_try;
    struct pt_indent indent_merge;

    /* check if normal indent is possible */
    indent_try = indent_p;
    node_try = *node_p;
    pt_try_normal_indent_in_node(&node_try, tc, &indent_try);

    if (indent_try.in_node.type == PT_INDENT_IN_NODE_NORMAL) {
        /* node fits to one line */
        pt_print_line(node_p, tc, indent_p);
    } else if (indent_try.in_node.type == PT_INDENT_IN_NODE_DIVIDED) {
        /* node will be divided */
        /* print first half */
        indent_merge = PT_INIT_INDENT(indent_p.wrapper, indent_try.in_node);
        /* pretend that this is normal node */
        indent_merge.in_node.type = PT_INDENT_IN_NODE_NORMAL;

        pt_print_line(&node_try, tc, indent_merge);
        ly_print_(tc->out, "\n");

        /* continue with second half on new line */
        node_try = *node_p;
        pt_second_half_node(&indent_try.in_node, &node_try);
        wr = pt_wrapper_if_last_sibling(indent_p.wrapper, node_try.last_one);
        indent_merge = PT_INIT_INDENT(wr, indent_try.in_node);

        pt_print_divided_node(&node_try, tc, indent_merge);
    } else if (indent_try.in_node.type == PT_INDENT_IN_NODE_FAILED) {
        /* node name is too long */
        pt_print_line_up_to_node_name(node_p, indent_p.wrapper, tc->out);

        if (pt_node_body_is_empty(node_p)) {
            return;
        }
        ly_print_(tc->out, "\n");

        node_try = *node_p;
        pt_second_half_node(&indent_try.in_node, &node_try);
        indent_try.in_node.type = PT_INDENT_IN_NODE_DIVIDED;
        wr = pt_wrapper_if_last_sibling(indent_p.wrapper, node_try.last_one);
        indent_merge = PT_INIT_INDENT(wr, indent_try.in_node);

        pt_print_divided_node(&node_try, tc, indent_merge);
    }
}

/**
 * @brief Check if leafref target must be change to string 'leafref'
 * because his target string is too long.
 * @param[in] node Containing leafref target.
 * @param[in] wr Wrapper for printing indentation before node.
 * @param[in] mll Max line length.
 * @param[in] out Output handler.
 * @return true if leafref must be changed to string 'leafref'.
 */
static ly_bool
pt_leafref_target_is_too_long(const struct pt_node *node, struct pt_wrapper wr,
        size_t mll, struct ly_out *out)
{
    size_t type_len;
    struct ly_out_clb_arg *data;

    if (node->type.type != PT_TYPE_TARGET) {
        return 0;
    }

    /* set ly_out to counting characters */
    data = out->method.clb.arg;

    data->counter = 0;
    data->mode = PT_CHAR_COUNT;
    /* count number of printed bytes */
    pt_print_wrapper(wr, out);
    ly_print_(out, "%*c", PT_INDENT_BTW_SIBLINGS, ' ');
    pt_print_divided_node_up_to_name(node, out);
    data->mode = PT_PRINT;
    type_len = strlen(node->type.str);

    return data->counter + type_len > mll;
}

/**
 * @brief Print node.
 *
 * This function is wrapper for ::pt_print_entire_node().
 * But difference is that take @p max_gap_before_type which will be
 * used to set the unified alignment.
 *
 * @param[in] node Node to print.
 * @param[in] max_gap_before_type Number of indent before \<type\>.
 * @param[in] wr Wrapper for printing indentation before node.
 * @param[in] tc Tree context.
 */
static void
pt_print_entire_node(struct pt_node node, uint32_t max_gap_before_type, struct pt_wrapper wr,
        struct pt_tree_ctx *tc)
{
    struct pt_indent_in_node indent = pt_default_indent_in_node(&node);

    if ((max_gap_before_type > 0) && (node.type.type != PT_TYPE_EMPTY)) {
        /* print actual node with unified indent */
        indent.btw_opts_type = pt_calc_btw_opts_type(&node, max_gap_before_type);
    }
    if (pt_leafref_target_is_too_long(&node, wr, tc->max_line_length, tc->out)) {
        node.type.type = PT_TYPE_LEAFREF;
    }
    /* after -> print actual node with default indent */
    pt_print_entire_node_(&node, tc, PT_INIT_INDENT(wr, indent));
}

/**
 * @brief Create implicit "case" node as parent of @p node.
 * @param[in] node Child of implicit case node.
 * @param[out] case_node Created case node.
 */
static void
pt_create_implicit_case_node(const struct pt_node *node, struct pt_node *case_node)
{
    case_node->status = node->status;
    case_node->flags = PT_FLAGS_TYPE_EMPTY;
    case_node->name.type = PT_NODE_CASE;
    case_node->name.keys = node->name.keys;
    case_node->name.module_prefix = node->name.module_prefix;
    case_node->name.str = node->name.str;
    case_node->name.opts = node->name.opts;
    case_node->type = PT_EMPTY_LF_TYPE;
    case_node->iffeatures = PT_EMPTY_IFFEATURES;
    case_node->last_one = node->last_one;
}

static const void *pt_next_sibling(const void *node, struct pt_tree_ctx *tc, ly_bool update);

/**
 * @brief Print implicit case node and his subtree.
 * @param[in] node Child of implicit case.
 * @param[in] wr Wrapper for printing indentation before node.
 * @param[in] tc Tree context. Its settings should be the same as
 * before the function call.
 * @return new indentation wrapper for @p node.
 */
static struct pt_wrapper
pt_print_implicit_node(const struct pt_node *node, struct pt_wrapper wr,
        struct pt_tree_ctx *tc)
{
    const void *sibl;
    struct pt_node case_node;
    struct pt_wrapper wr_case_child;

    pt_create_implicit_case_node(node, &case_node);
    ly_print_(tc->out, "\n");
    pt_print_entire_node(case_node, 0, wr, tc);
    ly_print_(tc->out, "\n");
    sibl = tc->lysc_tree ? pt_next_sibling(tc->cn, tc, 0) : pt_next_sibling(tc->pn, tc, 0);
    wr_case_child = sibl ?
            pt_wrapper_set_mark(wr) : pt_wrapper_set_shift(wr);
    return wr_case_child;
}

/**
 * @brief Check if there is no case statement under the choice statement.
 *
 * It can return true only if the Parsed schema tree
 * is used for browsing.
 *
 * @param[in] tc Tree context.
 * @return 1 if implicit case statement is present otherwise 0.
 */
static ly_bool
pt_need_implicit_node_case(struct pt_tree_ctx *tc)
{
    return !tc->lysc_tree && tc->pn->parent &&
           (tc->pn->parent->nodetype & LYS_CHOICE) &&
           (tc->pn->nodetype & (LYS_ANYDATA | LYS_CHOICE | LYS_CONTAINER |
           LYS_LEAF | LYS_LEAFLIST));
}

/**********************************************************************
 * Primitive getters
 *********************************************************************/

static const void *
pt_get_node(const struct pt_tree_ctx *tc)
{
    return tc->lysc_tree ?
           (const void *)tc->cn :
           (const void *)tc->pn;
}

static const void *
pt_get_child(const struct pt_tree_ctx *tc)
{
    if (!pt_get_node(tc)) {
        return NULL;
    }

    if (tc->lysc_tree) {
        return lysc_node_child(tc->cn);
    } else {
        return lysp_node_child(tc->pn);
    }
}

static uint16_t
pt_nodetype(const struct pt_tree_ctx *tc, const void *node)
{
    return tc->lysc_tree ?
           ((const struct lysc_node *)node)->nodetype :
           ((const struct lysp_node *)node)->nodetype;
}

static uint16_t
pt_lysp_flags(const struct pt_tree_ctx *tc, const void *node)
{
    const struct lysc_node *cn;

    if (!tc->lysc_tree) {
        return ((const struct lysp_node *)node)->flags;
    }

    cn = (const struct lysc_node *)node;
    if (PT_TREE_CTX_LYSP_NODE_PRESENT(cn)) {
        return PT_TREE_CTX_GET_LYSP_NODE(cn)->flags;
    } else {
        return 0;
    }
}

static const void *
pt_next(const struct pt_tree_ctx *tc, const void *node)
{
    return tc->lysc_tree ?
           (const void *)((const struct lysc_node *)node)->next :
           (const void *)((const struct lysp_node *)node)->next;
}

static const void *
pt_parent(const struct pt_tree_ctx *tc, const void *node)
{
    return tc->lysc_tree ?
           (const void *)((const struct lysc_node *)node)->parent :
           (const void *)((const struct lysp_node *)node)->parent;
}

static const void *
pt_child(const struct pt_tree_ctx *tc, const void *node)
{
    return tc->lysc_tree ?
           (const void *)lysc_node_child(node) :
           (const void *)lysp_node_child(node);
}

static const void *
pt_actions(const struct pt_tree_ctx *tc, const void *node)
{
    return tc->lysc_tree ?
           (const void *)lysc_node_actions(node) :
           (const void *)lysp_node_actions(node);
}

static const void *
pt_action_input(const struct pt_tree_ctx *tc, const void *node)
{
    return tc->lysc_tree ?
           (const void *)&((const struct lysc_node_action *)node)->input :
           (const void *)&((const struct lysp_node_action *)node)->input;
}

static const void *
pt_action_output(const struct pt_tree_ctx *tc, const void *node)
{
    return tc->lysc_tree ?
           (const void *)&((const struct lysc_node_action *)node)->output :
           (const void *)&((const struct lysp_node_action *)node)->output;
}

static const void *
pt_notifs(const struct pt_tree_ctx *tc, const void *node)
{
    return tc->lysc_tree ?
           (const void *)lysc_node_notifs(node) :
           (const void *)lysp_node_notifs(node);
}

/**********************************************************************
 * Read functions.
 *********************************************************************/

static ly_bool pt_schema_mount_skip(const struct lysc_node *node, const struct ly_set *refs);

/**
 * @brief Determine whether the node should be ignored and not printed.
 * @param[in] node Node to test.
 * @param[in] tc Current tree context.
 * @return 1 if node should be ignored otherwise 0.
 */
static ly_bool
pt_ignore_node(const void *node, const struct pt_tree_ctx *tc)
{
    struct pt_extension plugin_ctx;

    plugin_ctx = tc->plugin_ctx;

    if (node && plugin_ctx.schema_mount && (plugin_ctx.schema->ext == PT_EXT_SCHEMA_MOUNT_REF) &&
            pt_schema_mount_skip(node, plugin_ctx.schema_mount->parent_refs)) {
        return 1;
    } else if (node && (pt_lysp_flags(tc, node) & LYS_INTERNAL)) {
        return 1;
    } else {
        return 0;
    }
}

static void pt_ext_set_next_schema_mount(struct pt_tree_ctx *tc);

/**
 * @brief Get next sibling of the current node.
 *
 * This is a general algorithm that is able to
 * work with lysp_node or lysc_node.
 *
 * @param[in] node Node lysp_node or lysc_node.
 * @param[in] tc Current tree context.
 * @param[in] update Set to 1 if the transition to siblings should be
 * reflected directly in @p tc. Parameter originally added for schema-mount.
 * @return next sibling node.
 */
static const void *
pt_next_sibling(const void *node, struct pt_tree_ctx *tc, ly_bool update)
{
    const void *tmp, *parent, *sibl;
    struct pt_tree_ctx tc_copy;

    assert(node);

    if (pt_nodetype(tc, node) & (LYS_RPC | LYS_ACTION)) {
        if ((tmp = pt_next(tc, node))) {
            /* next action exists */
            sibl = tmp;
        } else if ((parent = pt_parent(tc, node))) {
            /* maybe if notif exists as sibling */
            sibl = pt_notifs(tc, parent);
        } else {
            sibl = NULL;
        }
    } else if (pt_nodetype(tc, node) & LYS_INPUT) {
        if ((parent = pt_parent(tc, node))) {
            /* if output action has data */
            if (pt_child(tc, pt_action_output(tc, parent))) {
                /* then next sibling is output action */
                sibl = pt_action_output(tc, parent);
            } else {
                /* input action cannot have siblings other
                 * than output action.
                 */
                sibl = NULL;
            }
        } else {
            /* there is no way how to get output action */
            sibl = NULL;
        }
    } else if (pt_nodetype(tc, node) & LYS_OUTPUT) {
        /* output action cannot have siblings */
        sibl = NULL;
    } else if (pt_nodetype(tc, node) & LYS_NOTIF) {
        /* must have as a sibling only notif */
        sibl = pt_next(tc, node);
    } else {
        /* for rest of nodes */
        if ((tmp = pt_next(tc, node))) {
            /* some sibling exists */
            sibl = tmp;
        } else if ((parent = pt_parent(tc, node))) {
            /* Action and notif are siblings too.
             * They can be reached through parent.
             */
            if ((tmp = pt_actions(tc, parent))) {
                /* next sibling is action */
                sibl = tmp;
            } else if ((tmp = pt_notifs(tc, parent))) {
                /* next sibling is notif */
                sibl = tmp;
            } else {
                /* sibling not exists */
                sibl = NULL;
            }
        } else {
            /* sibling not exists */
            sibl = NULL;
        }
    }

    if (pt_ignore_node(sibl, tc)) {
        return pt_next_sibling(sibl, tc, update);
    }

    if (!sibl && !pt_parent(tc, node) && tc->plugin_ctx.schema_mount &&
            !PT_LAST_SCHEMA_MOUNT(tc->plugin_ctx)) {
        tc_copy = *tc;
        pt_ext_set_next_schema_mount(tc);
        if (tc->lysc_tree) {
            sibl = (const void *)tc->plugin_ctx.schema->ctree;
        } else {
            sibl = (const void *)tc->plugin_ctx.schema->ptree;
        }
        if (pt_ignore_node(sibl, tc)) {
            sibl = pt_next_sibling(sibl, tc, update);
        }
        if (!update) {
            *tc = tc_copy;
        }
    }

    return sibl;
}

/**
 * @brief Get child of the current node.
 *
 * This is a general algorithm that is able to
 * work with lysp_node or lysc_node.
 *
 * @param[in] node Node lysp_node or lysc_node.
 * @param[in] tc Current tree context.
 * @param[in] update Set to 1 if the transition to child should be
 * reflected directly in @p tc. Parameter originally added for schema-mount.
 * @return child node.
 */
static const void *
pt_next_child(const void *node, struct pt_tree_ctx *tc, ly_bool update)
{
    const void *tmp, *child;

    assert(node);

    if (tc->plugin_ctx.schema_mount && (tc->plugin_ctx.schema_mount->mount_point == node)) {
        child = tc->plugin_ctx.schema_mount->schemas->ctree;
        if (update) {
            tc->plugin_ctx.schema = tc->plugin_ctx.schema_mount->schemas;
        }
    } else if (pt_nodetype(tc, node) & (LYS_ACTION | LYS_RPC)) {
        if (pt_child(tc, pt_action_input(tc, node))) {
            /* go to LYS_INPUT */
            child = pt_action_input(tc, node);
        } else if (pt_child(tc, pt_action_output(tc, node))) {
            /* go to LYS_OUTPUT */
            child = pt_action_output(tc, node);
        } else {
            /* input action and output action have no data */
            child = NULL;
        }
    } else {
        if ((tmp = pt_child(tc, node))) {
            child = tmp;
        } else {
            /* current node can't have children or has no children */
            /* but maybe has some actions or notifs */
            if ((tmp = pt_actions(tc, node))) {
                child = tmp;
            } else if ((tmp = pt_notifs(tc, node))) {
                child = tmp;
            } else {
                child = NULL;
            }
        }
    }

    if (pt_ignore_node(child, tc)) {
        return pt_next_sibling(child, tc, update);
    }

    return child;
}

/**
 * @brief Check if parent of the current node is the last
 * of his siblings.
 *
 * To maintain stability use this function only if the current node is
 * the first of the siblings.
 * Side-effect -> current node is set to the first sibling
 * if node has a parent otherwise no side-effect.
 *
 * @param[in,out] tc Tree context.
 * @return 1 if parent is last sibling otherwise 0.
 */
static ly_bool
pt_node_is_last_sibling(struct pt_tree_ctx *tc)
{
    const void *sibl;
    const void *parent;

    sibl = tc->lysc_tree ? pt_next_sibling(tc->cn, tc, 0) : pt_next_sibling(tc->pn, tc, 0);
    parent = tc->lysc_tree ? pt_parent(tc, tc->cn) : pt_parent(tc, tc->pn);

    if (parent) {
        return !sibl;
    } else if (tc->plugin_ctx.schema_mount) {
        return !sibl && !tc->plugin_ctx.schema_mount->mp_has_normal_node;
    } else {
        return !sibl && PT_LAST_SCHEMA(tc->plugin_ctx);
    }
}

/**
 * @brief Get new pt_parent_cache if we apply the transfer
 * to the child node in the tree.
 * @param[in] ca Parent cache for current node.
 * @param[in] tc Contains current tree node.
 * @return Cache for the current node.
 */
static struct pt_parent_cache
pt_parent_cache_for_child(struct pt_parent_cache ca, const struct pt_tree_ctx *tc)
{
    struct pt_parent_cache ret = PT_EMPTY_PARENT_CACHE;

    if (!tc->lysc_tree) {
        const struct lysp_node *pn = tc->pn;

        ret.ancestor =
                pn->nodetype & (LYS_INPUT) ? PT_ANCESTOR_RPC_INPUT :
                pn->nodetype & (LYS_OUTPUT) ? PT_ANCESTOR_RPC_OUTPUT :
                pn->nodetype & (LYS_NOTIF) ? PT_ANCESTOR_NOTIF :
                ca.ancestor;

        ret.lys_status =
                pn->flags & (LYS_STATUS_CURR | LYS_STATUS_DEPRC | LYS_STATUS_OBSLT) ? pn->flags :
                ca.lys_status;

        ret.lys_config =
                ca.ancestor == PT_ANCESTOR_RPC_INPUT ? 0 : /* because <flags> will be -w */
                ca.ancestor == PT_ANCESTOR_RPC_OUTPUT ? LYS_CONFIG_R :
                pn->flags & (LYS_CONFIG_R | LYS_CONFIG_W) ? pn->flags :
                ca.lys_config;

        ret.last_list =
                pn->nodetype & (LYS_LIST) ? (struct lysp_node_list *)pn :
                ca.last_list;
    }

    return ret;
}

/**
 * @brief Get name of the module.
 * @param[in] tc Context of the tree.
 * @return Filled keyword structure.
 */
static struct pt_keyword_stmt
pt_read_module_name(const struct pt_tree_ctx *tc)
{
    assert(tc);
    struct pt_keyword_stmt module = PT_EMPTY_KEYWORD_STMT;

    module.section_name = !tc->lysc_tree && tc->pmod->is_submod ?
            PT_KEYWORD_SUBMODULE :
            PT_KEYWORD_MODULE;

    module.argument = !tc->lysc_tree ?
            LYSP_MODULE_NAME(tc->pmod) :
            tc->cmod->mod->name;

    module.has_node = pt_get_node(tc) ? 1 : 0;

    return module;
}

static struct pt_node pt_pnode_read(struct pt_parent_cache ca, struct pt_tree_ctx *tc);

static struct pt_node pt_cnode_read(struct pt_tree_ctx *tc);

/**
 * @brief Transformation of current lysp_node to struct pt_node.
 * @param[in] ca Contains stored important data
 * when browsing the tree downwards.
 * @param[in] tc Context of the tree.
 * @return the read node.
 */
static struct pt_node
pt_read_node(struct pt_parent_cache ca, struct pt_tree_ctx *tc)
{
    if (tc->lysc_tree) {
        return pt_cnode_read(tc);
    } else {
        return pt_pnode_read(ca, tc);
    }
}

/**********************************************************************
 * Definition of lysp_node reading functions
 *********************************************************************/

/**
 * @brief Check if list statement has keys.
 * @param[in] pn Pointer to the list.
 * @return 1 if has keys, otherwise 0.
 */
static ly_bool
pt_pnode_list_has_keys(const struct lysp_node *pn)
{
    return pt_charptr_has_data(((const struct lysp_node_list *)pn)->key);
}

/**
 * @brief Check if it contains at least one feature.
 * @param[in] pn Current node.
 * @return 1 if has if-features, otherwise 0.
 */
static ly_bool
pt_pnode_has_iffeature(const struct lysp_node *pn)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lysp_qname *iffs;

    ly_bool ret = 0;

    iffs = pn->iffeatures;
    LY_ARRAY_FOR(iffs, u) {
        ret = 1;
        break;
    }
    return ret;
}

/**
 * @brief Find out if leaf is also the key in last list.
 * @param[in] pn Pointer to leaf.
 * @param[in] ca_last_list Pointer to last visited list.
 * Obtained from pt_parent_cache.
 * @return 1 if leaf is also the key, otherwise 0.
 */
static ly_bool
pt_pnode_leaf_is_key(const struct lysp_node *pn, const struct lysp_node_list *ca_last_list)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *)pn;
    const struct lysp_node_list *list = ca_last_list;

    if (!list) {
        return 0;
    }
    return pt_charptr_has_data(list->key) ?
           pt_word_is_present(list->key, leaf->name, ' ') : 0;
}

/**
 * @brief Check if container's type is presence.
 * @param[in] pn Pointer to container.
 * @return 1 if container has presence statement, otherwise 0.
 */
static ly_bool
pt_pnode_container_has_presence(const struct lysp_node *pn)
{
    return pt_charptr_has_data(((struct lysp_node_container *)pn)->presence);
}

/**
 * @brief Resolve \<status\> of the current node.
 * @param[in] nodetype Node's type obtained from the tree.
 * @param[in] flags Node's flags obtained from the tree.
 * @param[in] ca_lys_status Inherited status obtained from pt_parent_cache.
 * @return The status type.
 */
static char *
pt_pnode_resolve_status(uint16_t nodetype, uint16_t flags, uint16_t ca_lys_status)
{
    if (nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        /* LYS_INPUT and LYS_OUTPUT is special case */
        return pt_flags2status(ca_lys_status);
        /* if ancestor's status is deprc or obslt
         * and also node's status is not set
         */
    } else if ((ca_lys_status & (LYS_STATUS_DEPRC | LYS_STATUS_OBSLT)) && !(flags & (LYS_STATUS_CURR | LYS_STATUS_DEPRC | LYS_STATUS_OBSLT))) {
        /* get ancestor's status */
        return pt_flags2status(ca_lys_status);
    } else {
        /* else get node's status */
        return pt_flags2status(flags);
    }
}

static ly_bool pt_ext_is_present(struct pt_tree_ctx *tc, const char *ext_name);

/**
 * @brief Resolve \<flags\> of the current node.
 * @param[in] tc Current tree context.
 * @param[in] ca_ancestor Ancestor type obtained from pt_parent_cache.
 * @param[in] ca_lys_config Inherited config item obtained from pt_parent_cache.
 * @return The flags type.
 */
static const char *
pt_pnode_resolve_flags(struct pt_tree_ctx *tc, pt_parent_type ca_ancestor,
        uint16_t ca_lys_config)
{
    const struct lysp_node *pn;

    pn = tc->pn;

    if (pt_ext_is_present(tc, "mount-point")) {
        return PT_FLAGS_TYPE_MOUNT_POINT;
    } else if (pn->nodetype & LYS_USES) {
        return PT_FLAGS_TYPE_USES_OF_GROUPING;
    } else if (tc->plugin_ctx.schema && (tc->plugin_ctx.schema->ext == PT_EXT_GENERIC)) {
        return PT_FLAGS_TYPE_EXT;
    } else if ((pn->nodetype & LYS_INPUT) || (ca_ancestor == PT_ANCESTOR_RPC_INPUT)) {
        return PT_FLAGS_TYPE_RPC_INPUT_PARAMS;
    } else if ((pn->nodetype & LYS_OUTPUT) || (ca_ancestor == PT_ANCESTOR_RPC_OUTPUT)) {
        return PT_FLAGS_TYPE_RO;
    } else if (ca_ancestor == PT_ANCESTOR_NOTIF) {
        return PT_FLAGS_TYPE_RO;
    } else if (pn->nodetype & LYS_NOTIF) {
        return PT_FLAGS_TYPE_NOTIF;
    } else if (pn->nodetype & (LYS_RPC | LYS_ACTION)) {
        return PT_FLAGS_TYPE_RPC;
    } else if (!(pn->flags & (LYS_CONFIG_R | LYS_CONFIG_W))) {
        /* config is not set. Look at ancestor's config */
        return pt_flags2config(ca_lys_config);
    } else {
        return pt_flags2config(pn->flags);
    }
}

/**
 * @brief Resolve node type of the current node.
 * @param[in] tc Current tree context.
 * @param[in] ca_last_list Pointer to the last visited list. Obtained from the pt_parent_cache.
 * @param[out] type Resolved type of node.
 * @param[out] opts Resolved opts of node.
 */
static void
pt_pnode_resolve_opts(struct pt_tree_ctx *tc, const struct lysp_node_list *ca_last_list,
        pt_node_type *type, const char **opts)
{
    if (tc->plugin_ctx.schema && !tc->pn->parent &&
            (tc->plugin_ctx.schema->ext == PT_EXT_SCHEMA_MOUNT)) {
        *opts = PT_NODE_MOUNTED;
    } else if (tc->plugin_ctx.schema && !tc->pn->parent &&
            (tc->plugin_ctx.schema->ext == PT_EXT_SCHEMA_MOUNT_REF)) {
        *opts = PT_NODE_MOUNTED_PARENT_REF;
    } else if (tc->pn->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        *type = PT_NODE_ELSE;
    } else if (tc->pn->nodetype & LYS_CASE) {
        *type = PT_NODE_CASE;
    } else if ((tc->pn->nodetype & LYS_CHOICE) && !(tc->pn->flags & LYS_MAND_TRUE)) {
        *type = PT_NODE_CHOICE;
        *opts = PT_NODE_OPTIONAL;
    } else if (tc->pn->nodetype & LYS_CHOICE) {
        *type = PT_NODE_CHOICE;
    } else if ((tc->pn->nodetype & LYS_CONTAINER) && (pt_pnode_container_has_presence(tc->pn))) {
        *opts = PT_NODE_CONTAINER;
    } else if (tc->pn->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        *opts = PT_NODE_LISTLEAFLIST;
    } else if ((tc->pn->nodetype & (LYS_ANYDATA | LYS_ANYXML)) && !(tc->pn->flags & LYS_MAND_TRUE)) {
        *opts = PT_NODE_OPTIONAL;
    } else if ((tc->pn->nodetype & LYS_LEAF) && !(tc->pn->flags & LYS_MAND_TRUE) && (!pt_pnode_leaf_is_key(tc->pn, ca_last_list))) {
        *opts = PT_NODE_OPTIONAL;
    } else {
        *type = PT_NODE_ELSE;
    }
}

/**
 * @brief Resolve \<type\> of the current node.
 * @param[in] pn Current node.
 * @return Resolved type.
 */
static struct pt_lf_type
pt_pnode_resolve_type(const struct lysp_node *pn)
{
    const char *str = NULL;
    const struct lysp_node_leaf *leaf;
    const struct lysp_node_leaflist *list;

    if (!pn) {
        return PT_EMPTY_LF_TYPE;
    } else if (pn->nodetype & LYS_LEAFLIST) {
        list = (const struct lysp_node_leaflist *)pn;
        str = list->type.path ? list->type.path->expr : NULL;
        if (pt_charptr_has_data(str)) {
            return PT_INIT_LF_TYPE(PT_TYPE_TARGET, str);
        }
        str = list->type.name;
        if (pt_charptr_has_data(str)) {
            return PT_INIT_LF_TYPE(PT_TYPE_NAME, str);
        } else {
            return PT_EMPTY_LF_TYPE;
        }
    } else if (pn->nodetype & LYS_LEAF) {
        leaf = (const struct lysp_node_leaf *)pn;
        str = leaf->type.path ? leaf->type.path->expr : NULL;
        if (pt_charptr_has_data(str)) {
            return PT_INIT_LF_TYPE(PT_TYPE_TARGET, str);
        }
        str = leaf->type.name;
        if (pt_charptr_has_data(str)) {
            return PT_INIT_LF_TYPE(PT_TYPE_NAME, str);
        } else {
            return PT_EMPTY_LF_TYPE;
        }
    } else if (pn->nodetype == LYS_ANYDATA) {
        return PT_INIT_LF_TYPE(PT_TYPE_NAME, "anydata");
    } else if (pn->nodetype & LYS_ANYXML) {
        return PT_INIT_LF_TYPE(PT_TYPE_NAME, "anyxml");
    } else {
        return PT_EMPTY_LF_TYPE;
    }
}

/**
 * @brief Resolve iffeatures.
 * @param[in] pn Current parsed node.
 * @return Resolved iffeatures.
 */
static struct pt_iffeatures
pt_pnode_resolve_iffeatures(const struct lysp_node *pn)
{
    struct pt_iffeatures iff;

    if (pn && pt_pnode_has_iffeature(pn)) {
        iff.type = PT_IFF_PRESENT;
    } else {
        iff.type = PT_IFF_NON_PRESENT;
    }

    return iff;
}

/**
 * @brief Transformation of current lysp_node to struct pt_node.
 * @param[in] ca Contains stored important data
 * when browsing the tree downwards.
 * @param[in] tc Context of the tree.
 * @return the read node.
 */
static struct pt_node
pt_pnode_read(struct pt_parent_cache ca, struct pt_tree_ctx *tc)
{
    const struct lysp_node *pn;
    struct pt_node ret;

    assert(tc && tc->pn && tc->pn->nodetype != LYS_UNKNOWN);

    pn = tc->pn;
    ret = PT_EMPTY_NODE;

    /* <status> */
    ret.status = pt_pnode_resolve_status(pn->nodetype, pn->flags, ca.lys_status);

    /* <flags> */
    ret.flags = pt_pnode_resolve_flags(tc, ca.ancestor, ca.lys_config);

    /* set type of the node */
    pt_pnode_resolve_opts(tc, ca.last_list, &ret.name.type, &ret.name.opts);
    ret.name.keys = (tc->pn->nodetype & LYS_LIST) && pt_pnode_list_has_keys(tc->pn);

    /* The parsed tree is not compiled, so no node can be augmented
     * from another module. This means that nodes from the parsed tree
     * will never have the prefix.
     */
    ret.name.module_prefix = NULL;

    /* set node's name */
    ret.name.str = pn->name;

    /* <type> */
    ret.type = pt_pnode_resolve_type(pn);

    /* <iffeature> */
    ret.iffeatures = pt_pnode_resolve_iffeatures(pn);

    ret.last_one = !pt_next_sibling(pn, tc, 0);

    return ret;
}

/**********************************************************************
 * Definition of lysc_node reading functions
 *********************************************************************/

/**
 * @brief Resolve \<flags\> of the current node.
 *
 * Use this function only if pt_tree_ctx.lysc_tree is true.
 *
 * @param[in] tc Current tree context.
 * @return The flags type.
 */
static const char *
pt_cnode_resolve_flags(struct pt_tree_ctx *tc)
{
    const struct lysc_node *cn;

    cn = tc->cn;

    if (pt_ext_is_present(tc, "mount-point")) {
        return PT_FLAGS_TYPE_MOUNT_POINT;
    } else if (tc->plugin_ctx.schema && (tc->plugin_ctx.schema->ext == PT_EXT_GENERIC)) {
        return PT_FLAGS_TYPE_EXT;
    } else if ((cn->nodetype & LYS_INPUT) || (cn->flags & LYS_IS_INPUT)) {
        return PT_FLAGS_TYPE_RPC_INPUT_PARAMS;
    } else if ((cn->nodetype & LYS_OUTPUT) || (cn->flags & LYS_IS_OUTPUT)) {
        return PT_FLAGS_TYPE_RO;
    } else if (cn->nodetype & LYS_NOTIF) {
        return PT_FLAGS_TYPE_NOTIF;
    } else if (cn->nodetype & (LYS_RPC | LYS_ACTION)) {
        return PT_FLAGS_TYPE_RPC;
    } else {
        return pt_flags2config(cn->flags);
    }
}

/**
 * @brief Resolve node type of the current node.
 *
 * Use this function only if pt_tree_ctx.lysc_tree is true.
 *
 * @param[in] tc Current tree context.
 * @param[out] type Resolved type of node.
 * @param[out] opts Resolved opts.
 */
static void
pt_cnode_resolve_opts(struct pt_tree_ctx *tc, pt_node_type *type, const char **opts)
{
    if (tc->plugin_ctx.schema && !tc->cn->parent &&
            (tc->plugin_ctx.schema->ext == PT_EXT_SCHEMA_MOUNT)) {
        *opts = PT_NODE_MOUNTED;
    } else if (tc->plugin_ctx.schema && !tc->cn->parent &&
            (tc->plugin_ctx.schema->ext == PT_EXT_SCHEMA_MOUNT_REF)) {
        *opts = PT_NODE_MOUNTED_PARENT_REF;
    } else if (tc->cn->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        *type = PT_NODE_ELSE;
    } else if (tc->cn->nodetype & LYS_CASE) {
        *type = PT_NODE_CASE;
    } else if ((tc->cn->nodetype & LYS_CHOICE) && !(tc->cn->flags & LYS_MAND_TRUE)) {
        *type = PT_NODE_CHOICE;
        *opts = PT_NODE_OPTIONAL;
    } else if (tc->cn->nodetype & LYS_CHOICE) {
        *type = PT_NODE_CHOICE;
    } else if ((tc->cn->nodetype & LYS_CONTAINER) && (tc->cn->flags & LYS_PRESENCE)) {
        *opts = PT_NODE_CONTAINER;
    } else if (tc->cn->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        *opts = PT_NODE_LISTLEAFLIST;
    } else if ((tc->cn->nodetype & (LYS_ANYDATA | LYS_ANYXML)) && !(tc->cn->flags & LYS_MAND_TRUE)) {
        *opts = PT_NODE_OPTIONAL;
    } else if ((tc->cn->nodetype & LYS_LEAF) && !(tc->cn->flags & (LYS_MAND_TRUE | LYS_KEY))) {
        *opts = PT_NODE_OPTIONAL;
    } else {
        *type = PT_NODE_ELSE;
    }
}

/**
 * @brief Resolve prefix (\<prefix\>:\<name\>) of node that has been
 * placed from another module via an augment statement.
 * @param[in] cn Current compiled node.
 * @param[in] current_compiled_module Module whose nodes are
 * currently being printed.
 * @return Prefix of foreign module or NULL.
 */
static const char *
pt_cnode_resolve_prefix(const struct lysc_node *cn, const struct lysc_module *current_compiled_module)
{
    const struct lys_module *node_module;
    const char *ret = NULL;

    node_module = cn->module;
    if (!node_module || !current_compiled_module) {
        return NULL;
    } else if (node_module->compiled != current_compiled_module) {
        ret = node_module->prefix;
    }

    return ret;
}

/**
 * @brief Transformation of current lysc_node to struct pt_node.
 * @param[in] tc Context of the tree.
 * @return the read node.
 */
static struct pt_node
pt_cnode_read(struct pt_tree_ctx *tc)
{
    const struct lysc_node *cn;
    struct pt_node ret;

    assert(tc && tc->cn);

    cn = tc->cn;
    ret = PT_EMPTY_NODE;

    /* <status> */
    ret.status = pt_flags2status(cn->flags);

    /* <flags> */
    ret.flags = pt_cnode_resolve_flags(tc);

    /* set type of the node */
    pt_cnode_resolve_opts(tc, &ret.name.type, &ret.name.opts);
    ret.name.keys = (cn->nodetype & LYS_LIST) && !(cn->flags & LYS_KEYLESS);

    /* <prefix> */
    ret.name.module_prefix = pt_cnode_resolve_prefix(cn, tc->cmod);

    /* set node's name */
    ret.name.str = cn->name;

    /* <type> */
    ret.type = pt_pnode_resolve_type(PT_TREE_CTX_GET_LYSP_NODE(cn));

    /* <iffeature> */
    ret.iffeatures = pt_pnode_resolve_iffeatures(PT_TREE_CTX_GET_LYSP_NODE(cn));

    ret.last_one = !pt_next_sibling(cn, tc, 0);

    return ret;
}

/**********************************************************************
 * Traversing the nodes.
 *********************************************************************/

/**
 * @brief Change the pointer to the current node to its next sibling
 * only if exists.
 * @param[in] ca Contains inherited data from ancestors.
 * @param[in,out] tc Tree context.
 * Contains pointer to the current node.
 * @return Non-empty \<node\> representation if sibling exists.
 * The @p tc is modified.
 * @return Empty \<node\> representation.
 * The @p tc is not modified.
 */
static struct pt_node
pt_modi_next_sibling(struct pt_parent_cache ca, struct pt_tree_ctx *tc)
{
    const void *node, *sibl;

    node = pt_get_node(tc);
    sibl = pt_next_sibling(node, tc, 1);

    /* if next sibling exists */
    if (!sibl) {
        return PT_EMPTY_NODE;
    }

    if (tc->plugin_ctx.schema) {
        tc->lysc_tree = tc->plugin_ctx.schema->compiled;
    }

    /* update pt_tree_ctx */
    if (tc->lysc_tree) {
        tc->cn = sibl;
        return pt_cnode_read(tc);
    } else {
        tc->pn = sibl;
        return pt_pnode_read(ca, tc);
    }
}

/**
 * @brief Change the current node pointer to its child
 * but only if exists.
 * @param[in] ca Contains inherited data from ancestors.
 * @param[in,out] tc Context of the tree.
 * Contains pointer to the current node.
 * @return Non-empty \<node\> representation of the current
 * node's child. The @p tc is modified.
 * @return Empty \<node\> representation if child don't exists.
 * The @p tc is not modified.
 */
static struct pt_node
pt_modi_next_child(struct pt_parent_cache ca, struct pt_tree_ctx *tc)
{
    const void *node, *child;

    node = pt_get_node(tc);
    child = pt_next_child(node, tc, 1);

    if (!child) {
        return PT_EMPTY_NODE;
    }

    if (tc->plugin_ctx.schema) {
        tc->lysc_tree = tc->plugin_ctx.schema->compiled;
    }

    if (tc->lysc_tree) {
        tc->cn = child;
        return pt_cnode_read(tc);
    } else {
        tc->pn = child;
        return pt_pnode_read(ca, tc);
    }
}

/**
 * @brief Reset current node to child.
 * @param[in,out] tc Contains current node.
 */
static void
pt_modi_next_child2(struct pt_tree_ctx *tc)
{
    const void *node = pt_get_child(tc);

    if (tc->lysc_tree) {
        tc->cn = node;
    } else {
        tc->pn = node;
    }
}

/**
 * @brief Change current node pointer to its parent
 * but only if parent exists.
 * @param[in,out] tc Tree context.
 * Contains pointer to the current node.
 * @return 1 if the node had parents and the change was successful.
 * @return 0 if the node did not have parents.
 * The pointer to the current node did not change.
 */
static ly_bool
pt_modi_parent(struct pt_tree_ctx *tc)
{
    if (tc->lysc_tree) {
        assert(tc && tc->cn);
        /* If no parent exists, stay in actual node. */
        if (tc->cn->parent) {
            tc->cn = tc->cn->parent;
            return 1;
        } else {
            return 0;
        }
    } else {
        assert(tc && tc->pn);
        /* If no parent exists, stay in actual node. */
        if (!tc->pn->parent) {
            return 0;
        }
        if (((tc->section == PT_SECT_AUGMENT) || (tc->section == PT_SECT_GROUPING)) &&
                !tc->pn->parent->parent) {
            /* augment and grouping themselves are nodes, stay in subtree */
            return 0;
        }
        tc->pn = tc->pn->parent;
        return 1;
    }
}

/**
 * @copydoc ::pt_pnode_modi_first_sibling()
 */
static struct pt_node
pt_cnode_modi_first_sibling(struct pt_parent_cache ca, struct pt_tree_ctx *tc)
{
    struct pt_node node;

    assert(tc && tc->cn);

    if (pt_modi_parent(tc)) {
        node = pt_modi_next_child(ca, tc);
    } else {
        /* current node is top-node */
        switch (tc->section) {
        case PT_SECT_MODULE:
            tc->cn = tc->cn->module->compiled->data;
            break;
        case PT_SECT_RPCS:
            tc->cn = (const struct lysc_node *)tc->cmod->rpcs;
            break;
        case PT_SECT_NOTIF:
            tc->cn = (const struct lysc_node *)tc->cmod->notifs;
            break;
        case PT_SECT_PLUG_DATA:
            if ((tc->plugin_ctx.schema->ext == PT_EXT_SCHEMA_MOUNT) ||
                    (tc->plugin_ctx.schema->ext == PT_EXT_SCHEMA_MOUNT_REF)) {
                tc->cn = tc->plugin_ctx.schema_mount->schemas->ctree;
                tc->plugin_ctx.schema = tc->plugin_ctx.schema_mount->schemas;
            } else {
                tc->cn = tc->plugin_ctx.schema->ctree;
            }
            break;
        default:
            assert(0);
        }
        if (pt_ignore_node(tc->cn, tc)) {
            node = pt_modi_next_sibling(ca, tc);
        } else {
            node = pt_cnode_read(tc);
        }
    }

    return node;
}

/**
 * @brief Change the current node pointer to the first child of node's
 * parent. If current node is already first sibling/child then nothing
 * will change.
 * @param[in] ca Settings of parent.
 * @param[in,out] tc Tree context.
 * @return node for printing.
 */
static struct pt_node
pt_pnode_modi_first_sibling(struct pt_parent_cache ca, struct pt_tree_ctx *tc)
{
    struct pt_node node;

    assert(tc && tc->pn);

    if (pt_modi_parent(tc)) {
        node = pt_modi_next_child(ca, tc);
    } else {
        /* current node is top-node */
        switch (tc->section) {
        case PT_SECT_MODULE:
            tc->pn = tc->pmod->data;
            break;
        case PT_SECT_AUGMENT:
            tc->pn = lysp_node_child(tc->pn->parent);
            break;
        case PT_SECT_RPCS:
            tc->pn = (const struct lysp_node *)tc->pmod->rpcs;
            break;
        case PT_SECT_NOTIF:
            tc->pn = (const struct lysp_node *)tc->pmod->notifs;
            break;
        case PT_SECT_GROUPING:
            tc->pn = lysp_node_child(tc->pn->parent);
            break;
        case PT_SECT_PLUG_DATA:
            if ((tc->plugin_ctx.schema->ext == PT_EXT_SCHEMA_MOUNT) ||
                    (tc->plugin_ctx.schema->ext == PT_EXT_SCHEMA_MOUNT_REF)) {
                tc->pn = tc->plugin_ctx.schema_mount->schemas->ptree;
                tc->plugin_ctx.schema = tc->plugin_ctx.schema_mount->schemas;
            } else {
                tc->pn = tc->plugin_ctx.schema->ptree;
            }
            break;
        default:
            assert(0);
        }
        if (pt_ignore_node(tc->pn, tc)) {
            node = pt_modi_next_sibling(ca, tc);
        } else {
            node = pt_pnode_read(ca, tc);
        }
    }

    return node;
}

/**
 * @brief Change the current node pointer to the first child of node's
 * parent. If current node is already first sibling/child then nothing
 * will change.
 * @param[in] ca Settings of parent.
 * @param[in,out] tc Tree context.
 * @return node for printing.
 */
static struct pt_node
pt_modi_first_sibling(struct pt_parent_cache ca, struct pt_tree_ctx *tc)
{
    return tc->lysc_tree ? pt_cnode_modi_first_sibling(ca, tc) : pt_pnode_modi_first_sibling(ca, tc);
}

/**
 * @brief Get rpcs section if exists.
 * @param[in,out] tc Tree context.
 * @return Section representation if it exists. The @p tc is modified
 * and his pointer points to the first node in rpcs section.
 * @return Empty section representation otherwise.
 */
static struct pt_keyword_stmt
pt_modi_get_rpcs(struct pt_tree_ctx *tc)
{
    assert(tc);
    const void *actions;
    struct pt_keyword_stmt rpc = PT_EMPTY_KEYWORD_STMT;

    if (tc->lysc_tree) {
        actions = tc->cmod->rpcs;
        if (actions) {
            tc->cn = actions;
        }
    } else {
        actions = tc->pmod->rpcs;
        if (actions) {
            tc->pn = actions;
        }
    }

    if (actions) {
        tc->section = PT_SECT_RPCS;
        rpc.section_name = PT_KEYWORD_RPC;
        rpc.has_node = pt_get_node(tc) ? 1 : 0;
    }

    return rpc;
}

/**
 * @brief Get notification section if exists
 * @param[in,out] tc Tree context.
 * @return Section representation if it exists.
 * The @p tc is modified and his pointer points to the
 * first node in notification section.
 * @return Empty section representation otherwise.
 */
static struct pt_keyword_stmt
pt_modi_get_notifications(struct pt_tree_ctx *tc)
{
    assert(tc);
    const void *notifs;
    struct pt_keyword_stmt notif = PT_EMPTY_KEYWORD_STMT;

    if (tc->lysc_tree) {
        notifs = tc->cmod->notifs;
        if (notifs) {
            tc->cn = notifs;
        }
    } else {
        notifs = tc->pmod->notifs;
        if (notifs) {
            tc->pn = notifs;
        }
    }

    if (notifs) {
        tc->section = PT_SECT_NOTIF;
        notif.section_name = PT_KEYWORD_NOTIF;
        notif.has_node = pt_get_node(tc) ? 1 : 0;
    }

    return notif;
}

/**
 * @brief Get next (or first) augment section if exists.
 * @param[in,out] tc Tree context. It is modified and his current
 * node is set to the lysp_node_augment.
 * @param[in] index Index of augments to get.
 * @return Section's representation if (next augment) section exists.
 * @return Empty section structure otherwise.
 */
static struct pt_keyword_stmt
pt_modi_get_augment(struct pt_tree_ctx *tc, uint32_t index)
{
    assert(tc);
    const struct lysp_node_augment *augs, *elem;
    struct pt_keyword_stmt aug = PT_EMPTY_KEYWORD_STMT;
    uint32_t i;

    i = 0;
    augs = tc->pmod->augments;
    LY_LIST_FOR(augs, elem) {
        if (i == index) {
            tc->pn = &elem->node;
            aug.section_name = PT_KEYWORD_AUGMENT;
            aug.argument = elem->nodeid;
            aug.has_node = pt_get_child(tc) ? 1 : 0;
            return aug;
        }
        i++;
    }

    return PT_EMPTY_KEYWORD_STMT;
}

/**
 * @brief Get next (or first) grouping section if exists
 * @param[in,out] tc Tree context. It is modified and his current
 * node is set to the lysp_node_grp.
 * @param[in] index Index of grouping to get.
 * @return The next (or first) section representation if it exists.
 * @return Empty section representation otherwise.
 */
static struct pt_keyword_stmt
pt_modi_get_grouping(struct pt_tree_ctx *tc, uint32_t index)
{
    assert(tc);
    const struct lysp_node_grp *grps, *elem;
    struct pt_keyword_stmt group = PT_EMPTY_KEYWORD_STMT;
    uint32_t i;

    i = 0;
    grps = tc->pmod->groupings;
    LY_LIST_FOR(grps, elem) {
        if (i == index) {
            tc->pn = &elem->node;
            group.section_name = PT_KEYWORD_GROUPING;
            group.argument = elem->name;
            group.has_node = pt_get_child(tc) ? 1 : 0;
            return group;
        }
        i++;
    }

    return PT_EMPTY_KEYWORD_STMT;
}

/**********************************************************************
 * Extension functions
 *********************************************************************/

/**
 * @brief Set printer tree context to the next schema.
 * @param[in,out] tc Printer tree schema.
 */
static void
pt_ext_set_next_schema_mount(struct pt_tree_ctx *tc)
{
    LY_ARRAY_COUNT_TYPE i;
    struct pt_ext_tree_schema *schemas;
    ly_bool set_next = 0;

    assert(!PT_LAST_SCHEMA_MOUNT(tc->plugin_ctx));

    schemas = tc->plugin_ctx.schema_mount->schemas;
    LY_ARRAY_FOR(schemas, i) {
        if (set_next) {
            tc->plugin_ctx.schema = &schemas[i];
            tc->lysc_tree = tc->plugin_ctx.schema->compiled;
            break;
        } else if (&schemas[i] == tc->plugin_ctx.schema) {
            set_next = 1;
        }
    }

    assert(set_next);
}

/**
 * @brief Check if parent-stmt is valid for printing extension.
 * @param[in] lysc_tree Set to 1 if ext is from compiled tree.
 * @param[in] ext Extension to check.
 * @return 1 if extension is valid.
 */
static ly_bool
pt_ext_parent_is_valid(ly_bool lysc_tree, void *ext)
{
    enum ly_stmt parent_stmt;

    if (lysc_tree) {
        parent_stmt = ((struct lysc_ext_instance *)ext)->parent_stmt;
    } else {
        parent_stmt = ((struct lysp_ext_instance *)ext)->parent_stmt;
    }
    if ((parent_stmt & LY_STMT_OP_MASK) || (parent_stmt & LY_STMT_DATA_NODE_MASK) ||
            (parent_stmt & LY_STMT_SUBMODULE) || parent_stmt & LY_STMT_MODULE) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Move extension iterator to the next position.
 * @param[in] lysc_tree Set to 1 if exts is from compiled tree.
 * @param[in] exts Current array of extensions.
 * @param[in,out] i State of iterator.
 * @return Pointer to the first/next extension.
 */
static void *
pt_ext_iter_next(ly_bool lysc_tree, void *exts, LY_ARRAY_COUNT_TYPE *i)
{
    void *ext = NULL;
    struct lysc_ext_instance *ce;
    struct lysp_ext_instance *pe;

    if (!exts) {
        return NULL;
    }

    if (lysc_tree) {
        ce = exts;
        while (*i < LY_ARRAY_COUNT(ce)) {
            if (ce->def->plugin_ref && pt_ext_parent_is_valid(1, &ce[*i])) {
                ext = &ce[*i];
                break;
            }
            ++(*i);
        }
    } else {
        pe = exts;
        while (*i < LY_ARRAY_COUNT(pe)) {
            if (pt_ext_parent_is_valid(0, &pe[*i])) {
                ext = &pe[*i];
                break;
            }
            ++(*i);
        }
    }
    ++(*i);

    return ext;
}

/**
 * @brief Iterate over extensions in module or node.
 * @param[in] tc Contains current node.
 * @param[in] ext_name Extension name to find.
 * @param[in] from_module Set to 1 if extensions in the module
 * sould be searched otherwise it will search in the node.
 * @param[in,out] i State of iterator.
 * @return First/next extension or NULL.
 */
static void *
pt_ext_iter(const struct pt_tree_ctx *tc, const char *ext_name,
        ly_bool from_module, LY_ARRAY_COUNT_TYPE *i)
{
    struct lysp_ext_instance *ext_pars;
    struct lysc_ext_instance *ext_comp;
    void *ext = NULL;
    const char *name = "";

    do {
        if (tc->lysc_tree) {
            ext_comp = from_module ? tc->cmod->exts : tc->cn->exts;
            ext_comp = ext = pt_ext_iter_next(1, ext_comp, i);
            if (ext_comp) {
                name = ext_comp->def->name;
                assert(name);
            }
        } else {
            ext_pars = from_module ? tc->pmod->exts : tc->pn->exts;
            ext_pars = ext = pt_ext_iter_next(0, ext_pars, i);
            if (ext_pars) {
                name = strchr(ext_pars->name, ':') + 1;
                assert(name);
            }
        }
    } while (ext && ext_name && strcmp(name, ext_name));

    return ext;
}

/**
 * @brief Check if printer_tree can use node extension.
 * @param[in] tc Context with current node.
 * @param[in] ext_name Extension name to find.
 * @return 1 if some extension for printer_tree is valid.
 */
static ly_bool
pt_ext_is_present(struct pt_tree_ctx *tc, const char *ext_name)
{
    uint64_t i = 0;

    if (pt_ext_iter(tc, ext_name, 0, &i)) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Determine whether the node should be ignored.
 * @param[in] node Node to check.
 * @param[in] refs Set of lysc nodes matching parent-reference XPaths.
 * @return 1 to ignore otherwise 0.
 */
static ly_bool
pt_schema_mount_skip(const struct lysc_node *node, const struct ly_set *refs)
{
    uint32_t i;
    const struct lysc_module *mod;
    struct lysc_node *ref, *iter;
    ly_bool skip;

    mod = node->module->compiled;

    /* Assume the @p node will be skipped. */
    skip = 1;
    for (i = 0; (i < refs->count) && skip; i++) {
        ref = refs->snodes[i];
        if (ref->module->compiled != mod) {
            /* parent-reference points to different module */
            continue;
        }

        for (iter = ref; iter; iter = iter->parent) {
            if (iter == node) {
                /* @p node is not skipped because it is parent-rererence node or his parent */
                skip = 0;
                break;
            }
        }
    }

    return skip;
}

/**
 * @brief Add lysc schema to the list.
 * @param[in] ctx Destination where to add.
 * @param[in] nodes Schema to add.
 * @param[in] parent_ref Set to 1 if schema is referenced by schema-mount parent-reference.
 * @return 1 to ignore otherwise 0.
 */
static LY_ERR
pt_ext_sprinter_ctree_add_nodes(const struct pt_ext_schema_mount *ctx, struct lysc_node *nodes, ly_bool parent_ref)
{
    struct pt_ext_tree_schema *new;

    LY_CHECK_ARG_RET(NULL, ctx, LY_EINVAL);

    if (!nodes) {
        return LY_SUCCESS;
    }

    LY_ARRAY_NEW_RET(NULL, ((struct pt_ext_schema_mount *)ctx)->schemas, new, LY_EMEM);
    new->compiled = 1;
    new->ctree = nodes;
    new->ext = parent_ref ? PT_EXT_SCHEMA_MOUNT_REF : PT_EXT_SCHEMA_MOUNT;

    return LY_SUCCESS;
}

/**
 * @brief Add lysp schema to the list.
 * @param[in] ctx Destination where to add.
 * @param[in] nodes Schema to add.
 * @param[in] parent_ref Set to 1 if schema is referenced by schema-mount parent-reference.
 * @return LY_ERR value.
 */
static LY_ERR
pt_ext_sprinter_ptree_add_nodes(const struct pt_ext_schema_mount *ctx, struct lysp_node *nodes, ly_bool parent_ref)
{
    struct pt_ext_tree_schema *new;

    LY_CHECK_ARG_RET(NULL, ctx, LY_EINVAL);

    if (!nodes) {
        return LY_SUCCESS;
    }

    LY_ARRAY_NEW_RET(NULL, ((struct pt_ext_schema_mount *)ctx)->schemas, new, LY_EMEM);
    new->compiled = 0;
    new->ptree = nodes;
    new->ext = parent_ref ? PT_EXT_SCHEMA_MOUNT_REF : PT_EXT_SCHEMA_MOUNT;

    return LY_SUCCESS;
}

/**
 * @brief Initialize plugin context.
 * @param[in] compiled Set to 1 if @p ext is lysc structure.
 * @param[in] ext Current processed extension.
 * @param[out] schema_mount Filled schema_mount structure.
 * @return LY_ERR value.
 */
static LY_ERR
pt_create_mount_point(ly_bool compiled, void *ext, struct pt_ext_schema_mount *schema_mount)
{
    LY_ERR rc = LY_SUCCESS;
    const struct ly_ctx *ext_ctx = NULL;
    const struct lys_module *mod;
    struct ly_set *refs = NULL;
    struct lysc_node *tree1, *tree2;
    uint32_t i, j;
    ly_bool from_parent_ref, is_first;

    if (!compiled) {
        return LY_SUCCESS;
    }

    if (lyplg_ext_schema_mount_get_ctx(ext, NULL, &ext_ctx)) {
        /* Void mount point */
        return LY_SUCCESS;
    }

    rc = lyplg_ext_schema_mount_get_parent_ref(ext, NULL, &refs);
    LY_CHECK_GOTO(rc, cleanup);

    /* build new list of modules to print. This list will omit internal
     * modules, modules with no nodes (e.g., iana-if-types) and modules
     * that were loaded as the result of a parent-reference.
     */
    i = ly_ctx_internal_modules_count(ext_ctx);
    while ((mod = ly_ctx_get_module_iter(ext_ctx, &i))) {
        from_parent_ref = 0;

        for (j = 0; refs && j < refs->count; j++) {
            if (!strcmp(mod->ns, refs->snodes[j]->module->ns)) {
                from_parent_ref = 1;
                break;
            }
        }
        if (from_parent_ref) {
            /* Modules loaded as the result of a parent-reference are added later. */
            continue;
        }

        /* Add data nodes, rpcs and notifications. */
        if ((ext_ctx->opts & LY_CTX_SET_PRIV_PARSED) && mod->compiled) {
            /* For compiled module. */
            rc = pt_ext_sprinter_ctree_add_nodes(schema_mount, mod->compiled->data, 0);
            LY_CHECK_GOTO(rc, cleanup);
            if (mod->compiled->rpcs) {
                rc = pt_ext_sprinter_ctree_add_nodes(schema_mount, &mod->compiled->rpcs->node, 0);
            }
            LY_CHECK_GOTO(rc, cleanup);
            if (mod->compiled->notifs) {
                rc = pt_ext_sprinter_ctree_add_nodes(schema_mount, &mod->compiled->notifs->node, 0);
            }
            LY_CHECK_GOTO(rc, cleanup);
        } else {
            /* For parsed module. */
            rc = pt_ext_sprinter_ptree_add_nodes(schema_mount, mod->parsed->data, 0);
            LY_CHECK_GOTO(rc, cleanup);
            if (mod->parsed->rpcs) {
                rc = pt_ext_sprinter_ptree_add_nodes(schema_mount, &mod->parsed->rpcs->node, 0);
            }
            LY_CHECK_GOTO(rc, cleanup);
            if (mod->parsed->notifs) {
                rc = pt_ext_sprinter_ptree_add_nodes(schema_mount, &mod->parsed->notifs->node, 0);
            }
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

    /* Add modules loaded as the result of a parent-reference. */
    for (i = 0; refs && (i < refs->count); i++) {
        tree1 = refs->snodes[i]->module->compiled->data;

        /* Add data nodes from the module only once. */
        is_first = 1;
        for (j = 0; j < i; j++) {
            tree2 = refs->snodes[j]->module->compiled->data;
            if (tree1 == tree2) {
                is_first = 0;
                break;
            }
        }
        if (is_first) {
            /* Add all data nodes but unavailable nodes are skipped in the callback. */
            rc = pt_ext_sprinter_ctree_add_nodes(schema_mount, tree1, 1);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

    schema_mount->parent_refs = refs;

cleanup:
    if (rc) {
        ly_set_free(refs, NULL);
    }

    return rc;
}

static void pt_print_subtree(struct pt_node *node, uint32_t max_gap_before_type, struct pt_wrapper wr,
        struct pt_parent_cache ca, ly_bool print_root, struct pt_tree_ctx *tc);

/**
 * @brief For every extension instance print all schemas.
 * @param[in] wr Iindentation wrapper for node.
 * @param[in] ca Parent settings.
 * @param[in] tc Tree context.
 */
static void
pt_print_schema_mount(struct pt_wrapper wr, struct pt_parent_cache ca,
        struct pt_tree_ctx tc)
{
    LY_ERR rc;
    LY_ARRAY_COUNT_TYPE i;
    void *ext;
    struct pt_ext_schema_mount schema_mount = {0};
    struct pt_node node;

    if (!tc.lysc_tree) {
        return;
    }

    if (pt_next_child(pt_get_node(&tc), &tc, 0)) {
        schema_mount.mp_has_normal_node = 1;
    }

    tc.section = PT_SECT_PLUG_DATA;
    tc.pmod = NULL;
    tc.cmod = NULL;

    /* load children of mount-point */
    i = 0;
    while ((ext = pt_ext_iter(&tc, "mount-point", 0, &i))) {
        rc = pt_create_mount_point(tc.lysc_tree, ext, &schema_mount);
        LY_CHECK_ERR_GOTO(rc, tc.last_error = rc, end);

        if (!schema_mount.schemas) {
            /* mount-point not used */
            goto end;
        }
        schema_mount.mount_point = tc.cn;
        tc.plugin_ctx.schema_mount = &schema_mount;
        tc.plugin_ctx.schema = schema_mount.schemas;
        tc.lysc_tree = tc.plugin_ctx.schema->compiled;

        /* print schema-mount and parent-reference schemas */
        node = PT_EMPTY_NODE;
        pt_print_subtree(&node, 0, wr, ca, 0, &tc);
        /* then continue printing the remaining children of the mount-point node */

        /* only one mount_point is allowed */
        break;
    }

end:
    LY_ARRAY_FREE(schema_mount.schemas);
    if (schema_mount.parent_refs) {
        ly_set_free(schema_mount.parent_refs, NULL);
    }
    return;
}

/**********************************************************************
 * Print subtree
 *********************************************************************/

/**
 * @brief Print all parent nodes of @p node and the @p node itself.
 *
 * Side-effect -> pt_tree_ctx.cn will be set to @p node.
 *
 * @param[in] node Node to process.
 * @param[in] wr_in Indentation before node.
 * @param[in,out] tc Context of tree printer.
 */
static void
pt_print_parents(const struct lysc_node *node, struct pt_wrapper *wr_in, struct pt_tree_ctx *tc)
{
    uint32_t max_gap_before_type;
    struct pt_wrapper wr;
    struct pt_node print_node;

    assert(tc && tc->section == PT_SECT_MODULE);

    /* stop recursion */
    if (!node) {
        return;
    }
    pt_print_parents(node->parent, wr_in, tc);

    /* setup for printing */
    tc->cn = node;
    wr = pt_count_depth(wr_in, node);

    /* print node */
    ly_print_(tc->out, "\n");
    print_node = pt_read_node(PT_EMPTY_PARENT_CACHE, tc);
    /* siblings do not print, so the node is always considered the last */
    print_node.last_one = 1;
    max_gap_before_type = pt_max_gap_to_type(PT_EMPTY_PARENT_CACHE, tc);
    tc->cn = node;
    pt_print_entire_node(print_node, max_gap_before_type, wr, tc);
}

static void pt_print_subtree(struct pt_node *node, uint32_t max_gap_before_type, struct pt_wrapper wr,
        struct pt_parent_cache ca, ly_bool print_root, struct pt_tree_ctx *tc);

/**
 * @brief Print node and its siblings.
 * @param[in] node Node to print.
 * @param[in] wr Indentation on line.
 * @param[in] ca Contains inherited data from ancestors.
 * @param[in,out] tc Current tree context.
 */
static void
pt_print_siblings(struct pt_node *node, struct pt_wrapper wr,
        struct pt_parent_cache ca, struct pt_tree_ctx *tc)
{
    uint32_t max_gap_before_type;

    max_gap_before_type = pt_try_unified_indent(ca, *tc);
    do {
        if (pt_need_implicit_node_case(tc)) {
            struct pt_wrapper wr_case_child;

            wr_case_child = pt_print_implicit_node(node, wr, tc);
            pt_print_subtree(node, max_gap_before_type, wr_case_child, ca, 1, tc);
        } else {
            ly_print_(tc->out, "\n");
            pt_print_subtree(node, max_gap_before_type, wr, ca, 1, tc);
        }
        /* go to the actual node's sibling */
        *node = pt_modi_next_sibling(ca, tc);
    } while (!pt_node_is_empty(node));
}

/**
 * @brief Print the subtree.
 *
 * The current node is expected to be the root of the subtree.
 * Before root node is no linebreak printing. This must be addressed by
 * the caller. Root node will also be printed. Behind last printed node
 * is no linebreak.
 *
 * @param[in,out] node Current processed node used as iterator.
 * @param[in] max_gap_before_type Result from
 * ::pt_try_unified_indent() function for root node.
 * Set parameter to 0 if distance does not matter.
 * @param[in] wr Wrapper saying how deep in the whole tree
 * is the root of the subtree.
 * @param[in] ca Parent cache from root's parent.
 * If root is top-level node, insert ::PT_EMPTY_PARENT_CACHE.
 * @param[in] print_root Set to 1 for printing root node
 * and in that case @p max_gag_before_type is ignored.
 * @param[in,out] tc Context of tree printer.
 */
static void
pt_print_subtree(struct pt_node *node, uint32_t max_gap_before_type, struct pt_wrapper wr,
        struct pt_parent_cache ca, ly_bool print_root, struct pt_tree_ctx *tc)
{
    /* Print root node. */
    if (print_root && !pt_node_is_empty(node)) {
        pt_print_entire_node(*node, max_gap_before_type, wr, tc);
        if (pt_ext_is_present(tc, "mount-point")) {
            pt_print_schema_mount(wr, ca, *tc);
            /* continue with printing normal (not mounted) nodes */
        }
    }

    /* if node is last sibling, then do not add '|' to wrapper */
    wr = pt_node_is_last_sibling(tc) ?
            pt_wrapper_set_shift(wr) : pt_wrapper_set_mark(wr);

    /* go to the child */
    ca = pt_parent_cache_for_child(ca, tc);
    *node = pt_modi_next_child(ca, tc);
    if (pt_node_is_empty(node)) {
        return;
    }
    pt_print_siblings(node, wr, ca, tc);
    /* get back from child node to root node */
    pt_modi_parent(tc);
}

/**
 * @brief Print all siblings and their children.
 *
 * This function is suitable for printing top-level nodes that
 * do not have ancestors. Function call ::pt_print_subtree_nodes()
 * for all top-level siblings. Use this function after 'module' keyword
 * or 'augment' and so. The nodes may not be exactly top-level in the
 * tree, but the function considers them that way.
 *
 * @param[in] wr Wrapper saying how deeply the top-level nodes are
 * immersed in the tree.
 * @param[in,out] tc Tree context.
 */
static void
pt_print_trees(struct pt_wrapper wr, struct pt_tree_ctx *tc)
{
    struct pt_parent_cache ca;
    struct pt_node node;
    uint32_t max_gap_before_type;

    if (!pt_get_node(tc)) {
        return;
    }

    ca = PT_EMPTY_PARENT_CACHE;
    max_gap_before_type = pt_try_unified_indent(ca, *tc);

    if (!tc->lysc_tree) {
        if ((tc->section == PT_SECT_GROUPING) && !tc->pn->parent->parent) {
            ca.lys_config = 0x0;
        }
    }

    for (node = pt_modi_first_sibling(ca, tc);
            !pt_node_is_empty(&node);
            node = pt_modi_next_sibling(ca, tc)) {
        ly_print_(tc->out, "\n");
        pt_print_subtree(&node, max_gap_before_type, wr, ca, 1, tc);
    }
}

/**********************************************************************
 * Print sections
 *********************************************************************/

/**
 * @brief Settings if lysp_node are used for browsing through the tree.
 * @param[in] module YANG schema tree structure representing
 * YANG module.
 * @param[in] out Output handler.
 * @param[in] max_line_length Maximum line length limit
 * that should not be exceeded.
 * @param[in,out] tc Will be adapted to lysp_tree.
 */
static void
pt_lysp_tree_ctx(const struct lys_module *module, struct ly_out *out, size_t max_line_length,
        struct pt_tree_ctx *tc)
{
    *tc = (struct pt_tree_ctx) {
        .lysc_tree = 0,
        .section = PT_SECT_MODULE,
        .pmod = module->parsed,
        .cmod = NULL,
        .pn = module->parsed ? module->parsed->data : NULL,
        .cn = NULL,
        .last_error = 0,
        .plugin_ctx = {
            .schema_mount = NULL,
            .schema = NULL,
        },
        .out = out,
        .max_line_length = max_line_length
    };
}

/**
 * @brief Settings if lysc_node are used for browsing through the tree.
 *
 * Pointers to current nodes will be set to module data.
 *
 * @param[in] module YANG schema tree structure representing
 * YANG module.
 * @param[in] out Output handler.
 * @param[in] max_line_length Maximum line length limit
 * that should not be exceeded.
 * @param[in,out] tc Will be adapted to lysc_tree.
 */
static void
pt_lysc_tree_ctx(const struct lys_module *module, struct ly_out *out, size_t max_line_length,
        struct pt_tree_ctx *tc)
{
    *tc = (struct pt_tree_ctx) {
        .lysc_tree = 1,
        .section = PT_SECT_MODULE,
        .pmod = module->parsed,
        .cmod = module->compiled,
        .pn = NULL,
        .cn = module->compiled->data,
        .last_error = 0,
        .plugin_ctx = {
            .schema_mount = NULL,
            .schema = NULL,
        },
        .out = out,
        .max_line_length = max_line_length
    };
}

/**
 * @brief Reset settings to browsing through the lysp tree.
 * @param[in,out] tc Resets to lysp browsing.
 */
static void
pt_reset_to_lysp_tree_ctx(struct pt_tree_ctx *tc)
{
    LY_ERR erc;

    erc = tc->last_error;
    pt_lysp_tree_ctx(tc->pmod->mod, tc->out, tc->max_line_length, tc);
    tc->last_error = erc;
}

/**
 * @brief Check if augment's target node is located on the current module.
 * @param[in] pn Examined augment.
 * @param[in] pmod Current module.
 * @return 1 if nodeid refers to the local node, otherwise 0.
 */
static ly_bool
pt_nodeid_target_is_local(const struct lysp_node_augment *pn, const struct lysp_module *pmod)
{
    const char *id, *prefix, *name;
    uint32_t prefix_len, name_len;
    const struct lys_module *mod;
    ly_bool ret = 0;

    if (pn == NULL) {
        return ret;
    }

    id = pn->nodeid;
    if (!id) {
        return ret;
    }
    /* only absolute-schema-nodeid is taken into account */
    assert(id[0] == '/');
    ++id;

    ly_parse_nodeid(&id, &prefix, &prefix_len, &name, &name_len);
    if (prefix) {
        mod = ly_resolve_prefix(pmod->mod->ctx, prefix, prefix_len, LY_VALUE_SCHEMA, pmod);
        ret = mod ? (mod->parsed == pmod) : 0;
    } else {
        ret = 1;
    }

    return ret;
}

/**
 * @brief Print 'module' keyword, its name and all nodes.
 * @param[in,out] tc Tree context.
 */
static void
pt_print_module_section(struct pt_tree_ctx *tc)
{
    struct pt_keyword_stmt module = PT_EMPTY_KEYWORD_STMT;

    module = pt_read_module_name(tc);
    pt_print_keyword_stmt(&module, NULL, tc->out);
    pt_print_trees(PT_INIT_WRAPPER_TOP, tc);
}

/**
 * @brief For all augment sections: print 'augment' keyword,
 * its target node and all nodes.
 * @param[in,out] tc Tree context.
 */
static void
pt_print_augmentations(struct pt_tree_ctx tc)
{
    uint32_t i;
    ly_bool once = 1;
    ly_bool origin_was_lysc_tree = 0;
    struct pt_keyword_stmt aug = PT_EMPTY_KEYWORD_STMT;

    if (tc.lysc_tree) {
        origin_was_lysc_tree = 1;
        pt_reset_to_lysp_tree_ctx(&tc);
    }

    for (i = 0; ((aug = pt_modi_get_augment(&tc, i))).section_name; i++) {
        tc.section = PT_SECT_AUGMENT;
        if (origin_was_lysc_tree) {
            /* if lysc tree is used, then only augments targeting
             * another module are printed
             */
            if (pt_nodeid_target_is_local((const struct lysp_node_augment *)tc.pn, tc.pmod)) {
                continue;
            }
        }

        pt_print_keyword_stmt(&aug, &once, tc.out);
        pt_modi_next_child2(&tc);
        pt_print_trees(PT_INIT_WRAPPER_BODY, &tc);
    }
}

/**
 * @brief For rpcs section: print 'rpcs' keyword and all its nodes.
 * @param[in,out] tc Tree context.
 */
static void
pt_print_rpcs(struct pt_tree_ctx *tc)
{
    ly_bool once = 1;
    struct pt_keyword_stmt rpc = PT_EMPTY_KEYWORD_STMT;

    rpc = pt_modi_get_rpcs(tc);
    if (rpc.section_name) {
        pt_print_keyword_stmt(&rpc, &once, tc->out);
        pt_print_trees(PT_INIT_WRAPPER_BODY, tc);
    }
}

/**
 * @brief For notifications section: print 'notifications' keyword
 * and all its nodes.
 * @param[in,out] tc Tree context.
 */
static void
pt_print_notifications(struct pt_tree_ctx *tc)
{
    ly_bool once = 1;
    struct pt_keyword_stmt notifs = PT_EMPTY_KEYWORD_STMT;

    notifs = pt_modi_get_notifications(tc);
    if (notifs.section_name) {
        pt_print_keyword_stmt(&notifs, &once, tc->out);
        pt_print_trees(PT_INIT_WRAPPER_BODY, tc);
    }
}

/**
 * @brief For all grouping sections: print 'grouping' keyword, its name
 * and all nodes.
 * @param[in,out] tc Tree context.
 */
static void
pt_print_groupings(struct pt_tree_ctx *tc)
{
    uint32_t i;
    ly_bool once = 1;
    struct pt_keyword_stmt group = PT_EMPTY_KEYWORD_STMT;

    if (tc->lysc_tree) {
        return;
    }

    for (i = 0; ((group = pt_modi_get_grouping(tc, i))).section_name; i++) {
        tc->section = PT_SECT_GROUPING;
        pt_print_keyword_stmt(&group, &once, tc->out);
        pt_modi_next_child2(tc);
        pt_print_trees(PT_INIT_WRAPPER_BODY, tc);
    }
}

/**
 * @brief Read extension and get schema pointer.
 * @param[in] ext Extension to read.
 * @param[in] stmt_mask First stmt which match will be returned.
 * @return pointer to lysp node.
 */
static void *
pt_ext_parsed_read_storage(struct lysp_ext_instance *ext, int stmt_mask)
{
    LY_ARRAY_COUNT_TYPE i;
    enum ly_stmt stmt;
    void *substmts, **storage_p, *node = NULL;

    substmts = (void *)((struct lysp_ext_instance *)ext)->substmts;

    LY_ARRAY_FOR(substmts, i) {
        stmt = ((struct lysp_ext_instance *)ext)->substmts[i].stmt;
        storage_p = ((struct lysp_ext_instance *)ext)->substmts[i].storage_p;

        if (storage_p && (stmt & stmt_mask)) {
            return *storage_p;
        }
    }

    return node;
}

/**
 * @brief Read extension and get pointer to schema.
 * @param[in] ext Extension to read.
 * @param[in,out] compiled For input set to 1 if @p ext is compiled instance.
 * For output is set to 1 if returned pointer is from compiled instance.
 * @param[out] ks Section name to fill.
 * @return pointer to lysp or lysc node.
 */
static void *
pt_ext_read(void *ext, ly_bool *compiled, struct pt_keyword_stmt *ks)
{
    struct lysc_ext_instance *ext_comp;
    struct lysp_ext_instance *ext_pars;
    const char *name;
    void *schema;

    if (!*compiled) {
        ext_pars = ext;
        ks->argument = ext_pars->argument;
        name = strchr(ext_pars->name, ':') + 1;
        ks->section_name = name;
        if (!strcmp(ks->section_name, "augment-structure")) {
            schema = pt_ext_parsed_read_storage(ext_pars, LY_STMT_AUGMENT);
            schema = ((struct lysp_node_augment *)schema)->child;
        } else {
            schema = pt_ext_parsed_read_storage(ext_pars, LY_STMT_DATA_NODE_MASK);
        }
        *compiled = 0;
        return schema;
    }

    /* for compiled extension instance */
    ext_comp = ext;
    ks->argument = ext_comp->argument;
    ks->section_name = ext_comp->def->name;

    /* search in lysc_ext_instance */
    lyplg_ext_get_storage(ext, LY_STMT_DATA_NODE_MASK, sizeof schema, (const void **)&schema);
    *compiled = 1;
    if (schema) {
        return schema;
    }

    /* no data nodes lysc_ext_instance, so search in lysp_ext_instance */
    *compiled = 0;
    if (!strcmp(ks->section_name, "augment-structure")) {
        lyplg_ext_parsed_get_storage(ext_comp, LY_STMT_AUGMENT, sizeof schema, (const void **)&schema);
        schema = ((struct lysp_node_augment *)schema)->child;
    } else {
        lyplg_ext_parsed_get_storage(ext_comp, LY_STMT_DATA_NODE_MASK, sizeof schema, (const void **)&schema);
    }

    return schema;
}

/**
 * @brief Print top-level extension instances.
 * @param[in] tc Tree context.
 */
static void
pt_print_extensions(struct pt_tree_ctx tc)
{
    ly_bool once = 1;
    LY_ARRAY_COUNT_TYPE i = 0;
    struct pt_keyword_stmt ks = PT_EMPTY_KEYWORD_STMT;
    struct pt_node node;
    void *schema;
    void *ext;
    struct pt_ext_tree_schema ext_schema;
    ly_bool origin_lysc_tree = tc.lysc_tree;

    tc.section = PT_SECT_PLUG_DATA;
    tc.plugin_ctx.schema = &ext_schema;
    tc.plugin_ctx.schema->ext = PT_EXT_GENERIC;

    while ((ext = pt_ext_iter(&tc, NULL, 1, &i))) {
        tc.lysc_tree = origin_lysc_tree;

        schema = pt_ext_read(ext, &tc.lysc_tree, &ks);
        if (!strcmp(ks.section_name, "mount-point") ||
                !strcmp(ks.section_name, "annotation")) {
            /* extension ignored */
            continue;
        }
        if (tc.lysc_tree) {
            tc.cn = schema;
            tc.plugin_ctx.schema->ctree = tc.cn;
            tc.plugin_ctx.schema->compiled = 1;
            ks.has_node = tc.cn ? 1 : 0;
        } else {
            tc.pn = schema;
            tc.plugin_ctx.schema->ptree = tc.pn;
            tc.plugin_ctx.schema->compiled = 0;
            ks.has_node = tc.pn ? 1 : 0;
        }
        pt_print_keyword_stmt(&ks, &once, tc.out);
        if (!ks.has_node) {
            /* no subtree to print */
            continue;
        }
        /* print subtree */
        node = pt_modi_first_sibling(PT_EMPTY_PARENT_CACHE, &tc);
        pt_print_siblings(&node, PT_INIT_WRAPPER_BODY, PT_EMPTY_PARENT_CACHE, &tc);

        tc.lysc_tree = origin_lysc_tree;
    }
}

/**
 * @brief Print sections module, augment, rpcs, notifications,
 * grouping, yang-data.
 * @param[in,out] tc Tree context.
 */
static void
pt_print_sections(struct pt_tree_ctx *tc)
{
    pt_print_module_section(tc);
    pt_print_augmentations(*tc);
    pt_print_rpcs(tc);
    pt_print_notifications(tc);
    pt_print_groupings(tc);
    pt_print_extensions(*tc);
    ly_print_(tc->out, "\n");
}

static LY_ERR
pt_print_check_error(struct ly_out_clb_arg *out, struct pt_tree_ctx *tc)
{
    if (out->last_error) {
        return out->last_error;
    } else if (tc->last_error) {
        return tc->last_error;
    } else {
        return LY_SUCCESS;
    }
}

/**********************************************************************
 * Definition of module interface
 *********************************************************************/

LY_ERR
tree_print_module(struct ly_out *out, const struct lys_module *module, uint32_t UNUSED(options), size_t line_length)
{
    struct pt_tree_ctx tc;
    struct ly_out *new_out;
    LY_ERR erc;
    struct ly_out_clb_arg clb_arg = PT_INIT_LY_OUT_CLB_ARG(PT_PRINT, out, 0, LY_SUCCESS);

    LY_CHECK_ARG_RET3(module->ctx, out, module, module->parsed, LY_EINVAL);

    if ((erc = ly_out_new_clb(&pt_ly_out_clb_func, &clb_arg, &new_out))) {
        return erc;
    }

    line_length = line_length == 0 ? SIZE_MAX : line_length;
    if ((module->ctx->opts & LY_CTX_SET_PRIV_PARSED) && module->compiled) {
        pt_lysc_tree_ctx(module, new_out, line_length, &tc);
    } else {
        pt_lysp_tree_ctx(module, new_out, line_length, &tc);
    }

    pt_print_sections(&tc);
    erc = pt_print_check_error(&clb_arg, &tc);

    ly_out_free(new_out, NULL, 1);

    return erc;
}

LY_ERR
tree_print_compiled_node(struct ly_out *out, const struct lysc_node *node, uint32_t options, size_t line_length)
{
    struct pt_tree_ctx tc;
    struct ly_out *new_out;
    struct pt_wrapper wr;
    LY_ERR erc;
    struct ly_out_clb_arg clb_arg = PT_INIT_LY_OUT_CLB_ARG(PT_PRINT, out, 0, LY_SUCCESS);
    struct pt_keyword_stmt module = PT_EMPTY_KEYWORD_STMT;

    assert(out && node);

    if (!(node->module->ctx->opts & LY_CTX_SET_PRIV_PARSED)) {
        return LY_EINVAL;
    }

    if ((erc = ly_out_new_clb(&pt_ly_out_clb_func, &clb_arg, &new_out))) {
        return erc;
    }

    line_length = line_length == 0 ? SIZE_MAX : line_length;
    pt_lysc_tree_ctx(node->module, new_out, line_length, &tc);

    module = pt_read_module_name(&tc);
    pt_print_keyword_stmt(&module, NULL, tc.out);
    pt_print_parents(node, NULL, &tc);

    if (!(options & LYS_PRINT_NO_SUBSTMT)) {
        tc.cn = lysc_node_child(node);
        wr = pt_count_depth(NULL, tc.cn);
        pt_print_trees(wr, &tc);
    }
    ly_print_(out, "\n");

    erc = pt_print_check_error(&clb_arg, &tc);
    ly_out_free(new_out, NULL, 1);

    return erc;
}

LY_ERR
tree_print_parsed_submodule(struct ly_out *out, const struct lysp_submodule *submodp, uint32_t UNUSED(options),
        size_t line_length)
{
    struct pt_tree_ctx tc;
    struct ly_out *new_out;
    LY_ERR erc;
    struct ly_out_clb_arg clb_arg = PT_INIT_LY_OUT_CLB_ARG(PT_PRINT, out, 0, LY_SUCCESS);

    assert(submodp);
    LY_CHECK_ARG_RET(submodp->mod->ctx, out, LY_EINVAL);

    if ((erc = ly_out_new_clb(&pt_ly_out_clb_func, &clb_arg, &new_out))) {
        return erc;
    }

    line_length = line_length == 0 ? SIZE_MAX : line_length;
    pt_lysp_tree_ctx(submodp->mod, new_out, line_length, &tc);
    tc.pmod = (struct lysp_module *)submodp;
    tc.pn = submodp->data;

    pt_print_sections(&tc);
    erc = pt_print_check_error(&clb_arg, &tc);

    ly_out_free(new_out, NULL, 1);

    return erc;
}
