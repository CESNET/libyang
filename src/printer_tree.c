/**
 * @file printer_tree.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief RFC tree printer for libyang data structure
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "log.h"
#include "out_internal.h"
#include "parser_data.h"
#include "plugins_types.h"
#include "printer_internal.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
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
 */

/* Glossary:
 * trt - type
 * trp - functions for Printing
 * trb - functions for Browse the tree
 * tro - functions for Obtaining information from libyang
 * trm - Main functions, Manager
 * trg - General functions
 * tmp - functions that will be removed
 */

struct trt_tree_ctx;
struct trt_getters;
struct trt_fp_modify_ctx;
struct trt_fp_read;
struct trt_fp_crawl;
struct trt_fp_print;

/**
 * @brief Pointer to print function.
 *
 * Variadic arguments are expected to be of the type char*.
 * This pointer does not necessarily points to print function.
 * It is also used in this module for character counting that to be printed.
 * Because it contains a void parameter, it can perform any operation instead of printing.
 *
 * @param[in,out] out struct ly_out* or pointer to integer for counting the number of printed characters.
 * @param[in] arg_count number of arguments in va_list.
 * @param[in] ap variadic argument list from <stdarg.h>.
 */
typedef void (*trt_print_func)(void *out, int arg_count, va_list ap);

/**
 * @brief Customizable structure for printing.
 *
 * Structures trt_printing is used for printing by ly_out or for character counting.
 * See trp_cnt_linebreak_reset, trp_cnt_linebreak_increment
 */
struct trt_printing
{
    void *out;              /**< Pointer to output data. It is ly_out* for printing or uint32_t* for character counting. */
    trt_print_func pf;      /**< Callback function for printing (trg_print_by_ly_print) or character counting (trp_injected_strlen). */
    uint32_t cnt_linebreak; /**< Counter of printed line breaks. */
};

/**
 * @brief Callback functions that prints special cases.
 *
 * It just groups together tree context with trt_fp_print.
 */
struct trt_cf_print
{
    const struct trt_tree_ctx *ctx;                                 /**< Context of libyang tree. */
    void (*pf)(const struct trt_tree_ctx *, struct trt_printing *); /**< Pointing to function which printing list's keys or features. */
};

/******************************************************************************
 * Print getters
 *****************************************************************************/

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
struct trt_fp_print
{
    void (*print_features_names)(const struct trt_tree_ctx *, struct trt_printing *);   /**< Print list of features without {}? wrapper. */
    void (*print_keys)(const struct trt_tree_ctx *, struct trt_printing *);             /**< Print list's keys without [] wrapper. */
};

/**
 * @brief Package which only groups getter function.
 */
struct trt_pck_print
{
    const struct trt_tree_ctx *tree_ctx;    /**< Context of libyang tree. */
    struct trt_fp_print fps;                /**< Print function. */
};

/******************************************************************************
 * Indent
 *****************************************************************************/

/**
 * @brief Constants which are defined in the RFC or are observable from the pyang tool.
 */
typedef enum
{
    TRD_INDENT_EMPTY = 0,               /**< If the node is a case node, there is no space before the <name>. */
    TRD_INDENT_LONG_LINE_BREAK = 2,     /**< The new line should be indented so that it starts below <name> with a whitespace offset of at least two characters. */
    TRD_INDENT_LINE_BEGIN = 2,          /**< Indent below the keyword (module, augment ...).  */
    TRD_INDENT_BTW_SIBLINGS = 2,        /**< Indent between | and | characters. */
    TRD_INDENT_BEFORE_KEYS = 1,         /**< <x>___<keys>. */
    TRD_INDENT_BEFORE_TYPE = 4,         /**< <x>___<type>, but if mark is set then indent == 3. */
    TRD_INDENT_BEFORE_IFFEATURES = 1    /**< <x>___<iffeatures>. */
} trt_cnf_indent;

/**
 * @brief Type of indent in node.
 */
typedef enum
{
    TRD_INDENT_IN_NODE_NORMAL = 0,  /**< Node fits on one line. */
    TRD_INDENT_IN_NODE_DIVIDED,     /**< The node must be split into multiple rows. */
    TRD_INDENT_IN_NODE_FAILED       /**< Cannot be crammed into one line. The condition for the maximum line length is violated. */
} trt_indent_in_node_type;

/** Constant to indicate the need to break a line. */
static const int16_t trd_linebreak = -1;

/**
 * @brief Records the alignment between the individual elements of the node.
 *
 * See trp_indent_in_node_are_eq, trp_indent_in_node_place_break.
 */
struct trt_indent_in_node
{
    trt_indent_in_node_type type;   /**< Type of indent in node. */
    int16_t btw_name_opts;          /**< Indent between node name and opts. */
    int16_t btw_opts_type;          /**< Indent between opts and type. */
    int16_t btw_type_iffeatures;    /**< Indent between type and features. Ignored if <type> missing. */
};

/**
 * @brief Type of wrappers to be printed.
 */
typedef enum
{
    TRD_WRAPPER_TOP = 0,    /**< Related to the module. */
    TRD_WRAPPER_BODY        /**< Related to e.g. Augmentations or Groupings */
} trd_wrapper_type;

/**
 * @brief For resolving sibling symbol ('|') placement.
 *
 * Bit indicates where the sibling symbol must be printed.
 * This place is in multiples of TRD_INDENT_BTW_SIBLINGS.
 *
 * See: trp_init_wrapper_top, trp_init_wrapper_body, trp_wrapper_set_mark,
 * trp_wrapper_set_shift, trp_wrapper_if_last_sibling, trp_wrapper_eq,
 * trp_print_wrapper
 */
struct trt_wrapper
{
    trd_wrapper_type type;  /**< Location of the wrapper. */
    uint64_t bit_marks1;    /**< The set bits indicate where the '|' character is to be printed. */
    uint32_t actual_pos;    /**< Actual position in bit_marks. */
};

/**
 * @brief Package which only groups wrapper and indent in node.
 */
struct trt_pck_indent
{
    struct trt_wrapper wrapper;         /**< Coded "  |  |  " sequence. */
    struct trt_indent_in_node in_node;  /**< Indent in node. */
};

/******************************************************************************
 * status
 *****************************************************************************/

/**
 * @brief Status of the node.
 *
 * See: trp_print_status
 */
typedef enum
{
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
 * See: trp_print_flags, trp_print_flags_strlen
 */
typedef enum
{
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

static const char trd_node_name_prefix_choice[] = "(";
static const char trd_node_name_prefix_case[] = ":(";
static const char trd_node_name_triple_dot[] = "...";

/**
 * @brief Type of the node.
 *
 * Used mainly to complete the correct <opts> next to or around the <name>.
 */
typedef enum
{
    TRD_NODE_ELSE = 0,              /**< For some node which does not require special treatment. <name> */
    TRD_NODE_CASE,                  /**< For case node. :(<name>) */
    TRD_NODE_CHOICE,                /**< For choice node. (<name>) */
    TRD_NODE_OPTIONAL_CHOICE,       /**< For choice node with optional mark. (<name>)? */
    TRD_NODE_OPTIONAL,              /**< For an optional leaf, anydata, or anyxml. <name>? */
    TRD_NODE_CONTAINER,             /**< For a presence container. <name>! */
    TRD_NODE_LISTLEAFLIST,          /**< For a leaf-list or list (without keys). <name>* */
    TRD_NODE_KEYS,                  /**< For a list's keys. <name>* [<keys>] */
    TRD_NODE_TOP_LEVEL1,            /**< For a top-level data node in a mounted module. <name>/ */
    TRD_NODE_TOP_LEVEL2,            /**< For a top-level data node of a module identified in a mount point parent reference. <name>@ */
    TRD_NODE_TRIPLE_DOT             /**< For collapsed sibling nodes and their children. Special case which doesn't belong here very well. */
} trt_node_type;

/**
 * @brief Type of node and his name.
 *
 * See: trp_empty_node_name, trp_node_name_is_empty,
 * trp_print_node_name, trp_mark_is_used
 * trp_opts_keys_are_set, trp_print_opts_keys
 */
struct trt_node_name
{
    trt_node_type type;         /**< Type of the node relevant for printing. */
    const char *module_prefix;  /**< Prefix defined in the module where the node is defined. */
    const char *str;            /**< Name of the node. */
};

static const size_t trd_opts_mark_length = 1;       /**< Every opts mark has a length of one. */

/******************************************************************************
 * type
 *****************************************************************************/

/**
 * @brief Type of the <type>
 */
typedef enum
{
    TRD_TYPE_NAME = 0,  /**< Type is just a name that does not require special treatment. */
    TRD_TYPE_TARGET,    /**< Should have a form "-> TARGET", where TARGET is the leafref path. */
    TRD_TYPE_LEAFREF,   /**< This type is set automatically by the 'trp' algorithm. So set type as TRD_TYPE_TARGET. */
    TRD_TYPE_EMPTY      /**< Type is not used at all. */
} trt_type_type;

/**
 * @brief <type> in the <node>.
 *
 * See: trp_empty_type, trp_type_is_empty, trp_print_type
 */
struct trt_type
{
    trt_type_type type; /**< Type of the <type>. */
    const char *str;    /**< Path or name of the type. */
};

/******************************************************************************
 * iffeatures
 *****************************************************************************/

/**
 * @brief List of features in node.
 *
 * The iffeature is just ly_bool because printing is not provided by the printer component (trp).
 *
 * See: trp_empty_iffeature, trp_iffeature_is_empty, trp_print_iffeatures
 */
struct trt_iffeature
{
    ly_bool present;
};

/******************************************************************************
 * node
 *****************************************************************************/

/**
 * @brief <node> data for printing.
 *
 * It contains RFC's: <status>--<flags> <name><opts> <type> <if-features>.
 * Item <opts> is moved to part struct trt_node_name.
 * For printing [<keys>] and if-features is required special functions which prints them.
 *
 * See: trp_empty_node, trp_node_is_empty, trp_node_body_is_empty, trp_print_node_up_to_name,
 * trp_print_divided_node_up_to_name, trp_print_node
 */
struct trt_node
{
    trt_status_type status;             /**< <status>. */
    trt_flags_type flags;               /**< <flags>. */
    struct trt_node_name name;          /**< <node> with <opts> mark or [<keys>]. */
    struct trt_type type;               /**< <type> is the name of the type for leafs and leaf-lists. */
    struct trt_iffeature iffeatures;    /**< <if-features>. Printing function required. */
    ly_bool last_one;                   /**< Information about whether the node is the last. */
};

/**
 * @brief Package which only groups indent and node.
 */
struct trt_pair_indent_node
{
    struct trt_indent_in_node indent;
    struct trt_node node;
};

/******************************************************************************
 * statement
 *****************************************************************************/

static const char trd_top_keyword_module[] = "module";
static const char trd_top_keyword_submodule[] = "submodule";

static const char trd_body_keyword_augment[] = "augment";
static const char trd_body_keyword_rpc[] = "rpcs";
static const char trd_body_keyword_notif[] = "notifications";
static const char trd_body_keyword_grouping[] = "grouping";
static const char trd_body_keyword_yang_data[] = "yang-data";

/**
 * @brief Type of the trt_keyword.
 */
typedef enum
{
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
 * See: trp_empty_keyword_stmt, trp_keyword_stmt_is_empty
 * trt_print_keyword_stmt_begin, trt_print_keyword_stmt_str,
 * trt_print_keyword_stmt_end, trp_print_keyword_stmt
 * trp_keyword_type_strlen
 *
 */
struct trt_keyword_stmt
{
    trt_keyword_type type;      /**< String containing some of the top or body keyword. */
    const char *str;            /**< Name or path, it determines the type. */
};

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
 * its empty version obtained by relevant trp_empty* function is returned.
 */
struct trt_fp_modify_ctx
{
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
struct trt_fp_read
{
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
struct trt_fp_all
{
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
struct trt_printer_ctx
{
    struct trt_printing print;  /**< Basically printing function itself. */
    struct trt_fp_all fp;       /**< 'tro' functions callbacks. */
    uint32_t max_line_length;   /**< The maximum number of characters that can be printed on one line, including the last. */
};

/******************************************************************************
 * Tro functions
 *****************************************************************************/

typedef enum
{
    TRD_SECT_MODULE = 0,
    TRD_SECT_AUGMENT,
    TRD_SECT_RPCS,
    TRD_SECT_NOTIF,
    TRD_SECT_GROUPING,
    TRD_SECT_YANG_DATA
} trt_actual_section;

/**
 * @brief Ancestors who have a strong influence on their children
 */
typedef enum
{
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
 * See: tro_empty_parent_cache, tro_parent_cache_for_child
 */
struct trt_parent_cache {
    trt_ancestor_type ancestor;             /**< Some types of nodes have a special effect on their children. */
    uint16_t lys_status;                    /**< Inherited status CURR, DEPRC, OBSLT. */
    uint16_t lys_config;                    /**< Inherited config W or R. */
    const struct lysp_node_list *last_list; /**< The last LYS_LIST passed. */
};

/** Setting the behavior of this entire printer_tree module. */
struct trt_options
{
    uint32_t max_linebreaks;    /**< Max linebreaks per section. Functionality is not implemented. */
    uint32_t *cnt_linebreak;    /**< Pointer to struct trt_printing.cnt_linebreak counter. Functionality is not implemented. */
};

/**
 * @brief Main structure for browsing the libyang tree
 */
struct trt_tree_ctx
{
    trt_actual_section section;         /**< To which section pn points. */
    int64_t index_within_section;       /**< For example in which 'grouping' we are right now. It has the number -1 if it is invalid. */
    int64_t index;                      /**< Some nodes contain an array of other nodes etc. list has actions. Number -1 means invalid value. */
    const struct lys_module *module;    /**< Schema tree structures. */
    const struct lysp_node *pn;         /**< Actual pointer to parsed node. */
    const struct lysp_node *tpn;        /**< Pointer to actual top-node. */
    struct trt_options opt;             /**< Options for printing. */
};

/** Getter function for tro_lysp_node_charptr function. */
typedef const char *(*trt_get_charptr_func)(const struct lysp_node *pn);

/******************************************************************************
 * Declarations end
 *****************************************************************************/
/******************************************************************************
 * Definitions start
 *****************************************************************************/
/******************************************************************************
 * Definition of the general Trg / Trp functions
 *****************************************************************************/

/** Set .cnt_linebreak to zero. */
static void
trp_cnt_linebreak_reset(struct trt_printing *p)
{
    p->cnt_linebreak = 0;
}

/** Increment .cnt_linebreak by one. */
static void
trp_cnt_linebreak_increment(struct trt_printing *p)
{
    p->cnt_linebreak++;
}

/**
 * @brief Print function.
 *
 * This is a special abstraction for printing characters on output.
 * It was created because in some places in the code you only need to know the number of characters to print
 * in which case nothing may be printed.
 * So just set the callback function for counting characters (trt_printing.pf)
 * and nothing else in the code needs to change, because other functions do not know the difference.
 * Variadic arguments are expected to be of type char*.
 *
 * @param[in] p is used for printing or counting characters.
 * @param[in] arg_count number of arguments in va_list.
 */
static void
trp_print(struct trt_printing *p, int arg_count, ...)
{
    va_list ap;

    va_start(ap, arg_count);
    p->pf(p->out, arg_count, ap);
    va_end(ap);
}

#define PRINT_N_TIMES_BUFFER_SIZE 16

/** Print character n times. */
static void
trg_print_n_times(int32_t n, char c, struct trt_printing *p)
{
    if (n <= 0) {
        return;
    }

    static char buffer[PRINT_N_TIMES_BUFFER_SIZE];
    const uint32_t buffer_size = PRINT_N_TIMES_BUFFER_SIZE;

    buffer[buffer_size - 1] = '\0';
    for (uint32_t i = 0; i < n / (buffer_size - 1); i++) {
        memset(&buffer[0], c, buffer_size - 1);
        trp_print(p, 1, &buffer[0]);
    }
    uint32_t rest = n % (buffer_size - 1);

    buffer[rest] = '\0';
    memset(&buffer[0], c, rest);
    trp_print(p, 1, &buffer[0]);
}

#undef PRINT_N_TIMES_BUFFER_SIZE

/** Using ly_print_ function for printing */
static void
trg_print_by_ly_print(void *out, int arg_count, va_list ap)
{
    if (arg_count <= 0) {
        return;
    }

    for (int i = 0; i < arg_count; i++) {
        char *item = va_arg(ap, char *);
        if (item) {
            ly_print_(out, "%s", item);
        }
    }
}

/** Get absolute value of integer. */
static uint32_t
trg_abs(int32_t a)
{
    return a < 0 ? a * (-1) : a;
}

/** Test if the bit on the index is set. */
static ly_bool
trg_test_bit(uint64_t number, uint32_t bit)
{
    return (number >> bit) & 1U;
}

/** Print trd_separator_linebreak. */
static void
trg_print_linebreak(struct trt_printing *p)
{
    trp_cnt_linebreak_increment(p);
    trp_print(p, 1, "\n");
}

/** Print a substring but limited to the maximum length. */
const char *
trg_print_substr(const char *str, size_t len, struct trt_printing *p)
{
    for (size_t i = 0; i < len; i++) {
        trg_print_n_times(1, str[0], p);
        str++;
    }
    return str;
}

/** Pointer is not NULL and does not point to an empty string. */
static ly_bool
trg_charptr_has_data(const char *ptr)
{
    return ptr != NULL && ptr[0] != '\0';
}

/** Check if 'word' in 'src' is present where words are delimited by 'delim'. */
static ly_bool
trg_word_is_present(const char *src, const char *word, char delim)
{
    if ((src == NULL) || (src[0] == '\0') || (word == NULL)) {
        return 0;
    }

    const char *hit = strstr(src, word);

    if (hit) {
        /* word was founded at the begin of src
         * OR it match somewhere after delim
         */
        if ((hit == src) || (*(hit - 1) == delim)) {
            /* end of word was founded at the end of src
             * OR end of word was match somewhere before delim
             */
            char delim_or_end = (hit + strlen(word))[0];
            if ((delim_or_end == '\0') || (delim_or_end == delim)) {
                return 1;
            }
        }
        /* else -> hit is just substr and it's not the whole word */
        /* jump to the next word */
        for ( ; src[0] != '\0' && src[0] != delim; src++) {}
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
 * @brief Counts the characters to be printed instead of printing.
 *
 * Used in trt_printing as trt_print_func.
 *
 * @param[in,out] out is pointer to uint32_t for counting of bytes.
 * @param[in] arg_count number of arguments of type char* in va_list.
 */
static void
trp_injected_strlen(void *out, int arg_count, va_list ap)
{
    uint32_t *cnt = (uint32_t *)out;

    for (int i = 0; i < arg_count; i++) {
        char *item = va_arg(ap, char *);
        if (item) {
            *cnt = *cnt + strlen(item);
        }
    }
}

/** Check that indent in node can be considered as equivalent. */
static ly_bool
trp_indent_in_node_are_eq(struct trt_indent_in_node f, struct trt_indent_in_node s)
{
    const ly_bool a = f.type == s.type;
    const ly_bool b = f.btw_name_opts == s.btw_name_opts;
    const ly_bool c = f.btw_opts_type == s.btw_opts_type;
    const ly_bool d = f.btw_type_iffeatures == s.btw_type_iffeatures;

    return a && b && c && d;
}

/** Get wrapper related to the module. */
static struct trt_wrapper
trp_init_wrapper_top()
{
    /* module: <module-name>
     *   +--<node>
     *   |
     */
    struct trt_wrapper wr;

    wr.type = TRD_WRAPPER_TOP;
    wr.actual_pos = 0;
    wr.bit_marks1 = 0;
    return wr;
}

/** Get wrapper related to e.g. Augmenations or Groupings. */
static struct trt_wrapper
trp_init_wrapper_body()
{
    /* module: <module-name>
     *   +--<node>
     *
     *   augment <target-node>:
     *     +--<node>
     */
    struct trt_wrapper wr;

    wr.type = TRD_WRAPPER_BODY;
    wr.actual_pos = 0;
    wr.bit_marks1 = 0;
    return wr;
}

/** Setting ' ' symbol because node is last sibling. */
static struct trt_wrapper
trp_wrapper_set_shift(struct trt_wrapper wr)
{
    /* +--<node>
     *    +--<node>
     */
    wr.actual_pos++;
    return wr;
}

/** Setting '|' symbol because node is divided or it is not last sibling. */
static struct trt_wrapper
trp_wrapper_set_mark(struct trt_wrapper wr)
{
    wr.bit_marks1 |= 1U << wr.actual_pos;
    return trp_wrapper_set_shift(wr);
}

/** Setting ' ' symbol if node is last sibling otherwise set '|'. */
static struct trt_wrapper
trp_wrapper_if_last_sibling(struct trt_wrapper wr, ly_bool last_one)
{
    return last_one ? trp_wrapper_set_shift(wr) : trp_wrapper_set_mark(wr);
}

/** Test if the wrappers are equivalent. */
static ly_bool
trp_wrapper_eq(struct trt_wrapper f, struct trt_wrapper s)
{
    const ly_bool a = f.type == s.type;
    const ly_bool b = f.bit_marks1 == s.bit_marks1;
    const ly_bool c = f.actual_pos == s.actual_pos;

    return a && b && c;
}

/** Print "  |  " sequence on line. */
static void
trp_print_wrapper(struct trt_wrapper wr, struct trt_printing *p)
{
    const char char_space = ' ';

    {
        uint32_t lb;
        if (wr.type == TRD_WRAPPER_TOP) {
            lb = TRD_INDENT_LINE_BEGIN;
        } else if (wr.type == TRD_WRAPPER_BODY) {
            lb = TRD_INDENT_LINE_BEGIN * 2;
        } else {
            lb = TRD_INDENT_LINE_BEGIN;
        }

        trg_print_n_times(lb, char_space, p);
    }

    if (trp_wrapper_eq(wr, trp_init_wrapper_top())) {
        return;
    }

    for (uint32_t i = 0; i < wr.actual_pos; i++) {
        if (trg_test_bit(wr.bit_marks1, i)) {
            trp_print(p, 1, "|");
        } else {
            trp_print(p, 1, " ");
        }

        if (i != wr.actual_pos) {
            trg_print_n_times(TRD_INDENT_BTW_SIBLINGS, char_space, p);
        }
    }
}

/** Create struct trt_node_name as empty. */
static struct trt_node_name
trp_empty_node_name()
{
    struct trt_node_name ret;

    ret.str = NULL;
    return ret;
}

/** Check if struct trt_node_name is empty. */
static ly_bool
trp_node_name_is_empty(struct trt_node_name node_name)
{
    return node_name.str == NULL;
}

/** Check if [<keys>] are present in node. */
static ly_bool
trp_opts_keys_are_set(struct trt_node_name node_name)
{
    return node_name.type == TRD_NODE_KEYS;
}

/** Create empty struct trt_type. */
static struct trt_type
trp_empty_type()
{
    struct trt_type ret;

    ret.type = TRD_TYPE_EMPTY;
    return ret;
}

/** Check if struct trt_type is empty. */
static ly_bool
trp_type_is_empty(struct trt_type type)
{
    return type.type == TRD_TYPE_EMPTY;
}

/** Create empty trt_iffeature and note the absence of features. */
static struct trt_iffeature
trp_empty_iffeature()
{
    return (struct trt_iffeature) {
               0
    };
}

/** Check if trt_iffeature is empty. */
static ly_bool
trp_iffeature_is_empty(struct trt_iffeature iffeature)
{
    return !iffeature.present;
}

/** Create struct trt_node as empty. */
static struct trt_node
trp_empty_node()
{
    struct trt_node ret =
    {
        TRD_STATUS_TYPE_EMPTY, TRD_FLAGS_TYPE_EMPTY,
        trp_empty_node_name(), trp_empty_type(),
        trp_empty_iffeature(), 1
    };

    return ret;
}

/** Check if struct trt_node is empty. */
static ly_bool
trp_node_is_empty(struct trt_node node)
{
    const ly_bool a = trp_iffeature_is_empty(node.iffeatures);
    const ly_bool b = trp_type_is_empty(node.type);
    const ly_bool c = trp_node_name_is_empty(node.name);
    const ly_bool d = node.flags == TRD_FLAGS_TYPE_EMPTY;
    const ly_bool e = node.status == TRD_STATUS_TYPE_EMPTY;

    return a && b && c && d && e;
}

/** Check if [<keys>], <type> and <iffeatures> are empty/not_set. */
static ly_bool
trp_node_body_is_empty(struct trt_node node)
{
    const ly_bool a = trp_iffeature_is_empty(node.iffeatures);
    const ly_bool b = trp_type_is_empty(node.type);
    const ly_bool c = !trp_opts_keys_are_set(node.name);

    return a && b && c;
}

/** Create struct trt_keyword_stmt as empty. */
static struct trt_keyword_stmt
trp_empty_keyword_stmt()
{
    return (struct trt_keyword_stmt) {
               TRD_KEYWORD_EMPTY, NULL
    };
}

/** Check if struct trt_keyword_stmt is empty. */
static ly_bool
trp_keyword_stmt_is_empty(struct trt_keyword_stmt ks)
{
    return ks.type == TRD_KEYWORD_EMPTY;
}

/** Print <status> of the node. */
static void
trp_print_status(trt_status_type a, struct trt_printing *p)
{
    switch (a) {
    case TRD_STATUS_TYPE_CURRENT:
        trp_print(p, 1, "+");
        break;
    case TRD_STATUS_TYPE_DEPRECATED:
        trp_print(p, 1, "x");
        break;
    case TRD_STATUS_TYPE_OBSOLETE:
        trp_print(p, 1, "o");
        break;
    default:
        break;
    }
}

/** Print <flags>. */
static void
trp_print_flags(trt_flags_type a, struct trt_printing *p)
{
    switch (a) {
    case TRD_FLAGS_TYPE_RW:
        trp_print(p, 1, "rw");
        break;
    case TRD_FLAGS_TYPE_RO:
        trp_print(p, 1, "ro");
        break;
    case TRD_FLAGS_TYPE_RPC_INPUT_PARAMS:
        trp_print(p, 1, "-w");
        break;
    case TRD_FLAGS_TYPE_USES_OF_GROUPING:
        trp_print(p, 1, "-u");
        break;
    case TRD_FLAGS_TYPE_RPC:
        trp_print(p, 1, "-x");
        break;
    case TRD_FLAGS_TYPE_NOTIF:
        trp_print(p, 1, "-n");
        break;
    case TRD_FLAGS_TYPE_MOUNT_POINT:
        trp_print(p, 1, "mp");
        break;
    default:
        break;
    }
}

/** Get size of the <flags>. */
static size_t
trp_print_flags_strlen(trt_flags_type a)
{
    return a == TRD_FLAGS_TYPE_EMPTY ? 0 : 2;
}

/** Print entire struct trt_node_name structure. */
static void
trp_print_node_name(struct trt_node_name a, struct trt_printing *p)
{
    if (trp_node_name_is_empty(a)) {
        return;
    }

    const char *colon = a.module_prefix == NULL || a.module_prefix[0] == '\0' ? "" : ":";
    const char trd_node_name_suffix_choice[] = ")";
    const char trd_node_name_suffix_case[] = ")";
    const char trd_opts_optional[] = "?";        /**< For an optional leaf, choice, anydata, or anyxml. */
    const char trd_opts_container[] = "!";       /**< For a presence container. */
    const char trd_opts_list[] = "*";            /**< For a leaf-list or list. */
    const char trd_opts_slash[] = "/";           /**< For a top-level data node in a mounted module. */
    const char trd_opts_at_sign[] = "@";         /**< For a top-level data node of a module identified in a mount point parent reference. */

    switch (a.type) {
    case TRD_NODE_ELSE:
        trp_print(p, 3, a.module_prefix, colon, a.str);
        break;
    case TRD_NODE_CASE:
        trp_print(p, 5, trd_node_name_prefix_case, a.module_prefix, colon, a.str, trd_node_name_suffix_case);
        break;
    case TRD_NODE_CHOICE:
        trp_print(p, 5, trd_node_name_prefix_choice,  a.module_prefix, colon, a.str, trd_node_name_suffix_choice);
        break;
    case TRD_NODE_OPTIONAL_CHOICE:
        trp_print(p, 6, trd_node_name_prefix_choice,  a.module_prefix, colon, a.str, trd_node_name_suffix_choice, trd_opts_optional);
        break;
    case TRD_NODE_OPTIONAL:
        trp_print(p, 4, a.module_prefix, colon, a.str, trd_opts_optional);
        break;
    case TRD_NODE_CONTAINER:
        trp_print(p, 4, a.module_prefix, colon, a.str, trd_opts_container);
        break;
    case TRD_NODE_LISTLEAFLIST:
        trp_print(p, 4, a.module_prefix, colon, a.str, trd_opts_list);
        break;
    case TRD_NODE_KEYS:
        trp_print(p, 4, a.module_prefix, colon, a.str, trd_opts_list);
        break;
    case TRD_NODE_TOP_LEVEL1:
        trp_print(p, 4, a.module_prefix, colon, a.str, trd_opts_slash);
        break;
    case TRD_NODE_TOP_LEVEL2:
        trp_print(p, 4, a.module_prefix, colon, a.str, trd_opts_at_sign);
        break;
    case TRD_NODE_TRIPLE_DOT:
        trp_print(p, 1, trd_node_name_triple_dot);
        break;
    default:
        break;
    }
}

/** Check if mark (?, !, *, /, @) is implicitly contained in struct trt_node_name. */
static ly_bool
trp_mark_is_used(struct trt_node_name a)
{
    if (trp_node_name_is_empty(a)) {
        return 0;
    }

    switch (a.type) {
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
 *
 * @param[in] name type of the node with his name.
 * @param[in] ind number of spaces between name and [keys].
 * @param[in] pf basically a pointer to the function that prints the keys.
 * @param[in,out] p basically a pointer to a function that handles the printing itself.
 */
static void
trp_print_opts_keys(struct trt_node_name a, int16_t btw_name_opts, struct trt_cf_print cf, struct trt_printing *p)
{
    if (!trp_opts_keys_are_set(a)) {
        return;
    }

    /* <name><mark>___<keys>*/
    trg_print_n_times(btw_name_opts, ' ', p);
    trp_print(p, 1, "[");
    cf.pf(cf.ctx, p);
    trp_print(p, 1, "]");
}

/** Print entire struct trt_type structure. */
static void
trp_print_type(struct trt_type a, struct trt_printing *p)
{
    if (trp_type_is_empty(a)) {
        return;
    }

    switch (a.type) {
    case TRD_TYPE_NAME:
        trp_print(p, 1, a.str);
        break;
    case TRD_TYPE_TARGET:
        trp_print(p, 2, "-> ", a.str);
        break;
    case TRD_TYPE_LEAFREF:
        trp_print(p, 1, "leafref");
    default:
        break;
    }
}

/**
 * @brief Print all iffeatures of node
 *
 * @param[in] i contains flag if if-features is present.
 * @param[in] pf basically a pointer to the function that prints the list of features.
 * @param[in,out] p basically a pointer to a function that handles the printing itself.
 */
static void
trp_print_iffeatures(struct trt_iffeature a, struct trt_cf_print cf, struct trt_printing *p)
{
    if (trp_iffeature_is_empty(a)) {
        return;
    }

    trp_print(p, 1, "{");
    cf.pf(cf.ctx, p);
    trp_print(p, 1, "}?");
}

/** Print just <status>--<flags> <name> with opts mark. */
static void
trp_print_node_up_to_name(struct trt_node a, struct trt_printing *p)
{
    if (a.name.type == TRD_NODE_TRIPLE_DOT) {
        trp_print_node_name(a.name, p);
        return;
    }
    /* <status>--<flags> */
    trp_print_status(a.status, p);
    trp_print(p, 1, "--");
    /* If the node is a case node, there is no space before the <name> */
    /* also case node has no flags. */
    if (a.name.type != TRD_NODE_CASE) {
        trp_print_flags(a.flags, p);
        trp_print(p, 1, " ");
    }
    /* <name> */
    trp_print_node_name(a.name, p);
}

/** Print alignment (spaces) instead of <status>--<flags> <name> for divided node. */
static void
trp_print_divided_node_up_to_name(struct trt_node a, struct trt_printing *p)
{
    uint32_t space = trp_print_flags_strlen(a.flags);

    if (a.name.type == TRD_NODE_CASE) {
        /* :(<name> */
        space += strlen(trd_node_name_prefix_case);
    } else if (a.name.type == TRD_NODE_CHOICE) {
        /* (<name> */
        space += strlen(trd_node_name_prefix_choice);
    } else {
        /* _<name> */
        space += strlen(" ");
    }

    /* <name>
     * __
     */
    space += TRD_INDENT_LONG_LINE_BREAK;

    trg_print_n_times(space, ' ', p);
}

/**
 * @brief Print struct trt_node structure.
 *
 * @param[in] n node structure for printing.
 * @param[in] ppck package of functions for printing [<keys>] and <iffeatures>.
 * @param[in] ind indent in node.
 * @param[in,out] p basically a pointer to a function that handles the printing itself.
 */
static void
trp_print_node(struct trt_node a, struct trt_pck_print pck, struct trt_indent_in_node ind, struct trt_printing *p)
{
    if (trp_node_is_empty(a)) {
        return;
    }

    /* <status>--<flags> <name><opts> <type> <if-features> */

    const ly_bool triple_dot = a.name.type == TRD_NODE_TRIPLE_DOT;
    const ly_bool divided = ind.type == TRD_INDENT_IN_NODE_DIVIDED;
    const char char_space = ' ';

    if (triple_dot) {
        trp_print_node_name(a.name, p);
        return;
    } else if (!divided) {
        trp_print_node_up_to_name(a, p);
    } else {
        trp_print_divided_node_up_to_name(a, p);
    }

    /* <opts> */
    /* <name>___<opts>*/
    struct trt_cf_print cf_print_keys = {pck.tree_ctx, pck.fps.print_keys};

    trp_print_opts_keys(a.name, ind.btw_name_opts, cf_print_keys, p);

    /* <opts>__<type> */
    trg_print_n_times(ind.btw_opts_type, char_space, p);

    /* <type> */
    trp_print_type(a.type, p);

    /* <type>__<iffeatures> */
    trg_print_n_times(ind.btw_type_iffeatures, char_space, p);

    /* <iffeatures> */
    struct trt_cf_print cf_print_iffeatures = {pck.tree_ctx, pck.fps.print_features_names};

    trp_print_iffeatures(a.iffeatures, cf_print_iffeatures, p);
}

/** Print .keyword based on .type. */
static void
trt_print_keyword_stmt_begin(struct trt_keyword_stmt a, struct trt_printing *p)
{
    switch (a.type) {
    case TRD_KEYWORD_MODULE:
        trp_print(p, 3, trd_top_keyword_module, ":", " ");
        return;
    case TRD_KEYWORD_SUBMODULE:
        trp_print(p, 3, trd_top_keyword_submodule, ":", " ");
        return;
    default:
        trg_print_n_times(TRD_INDENT_LINE_BEGIN, ' ', p);
        switch (a.type) {
        case TRD_KEYWORD_AUGMENT:
            trp_print(p, 2, trd_body_keyword_augment, " ");
            break;
        case TRD_KEYWORD_RPC:
            trp_print(p, 1, trd_body_keyword_rpc);
            break;
        case TRD_KEYWORD_NOTIF:
            trp_print(p, 1, trd_body_keyword_notif);
            break;
        case TRD_KEYWORD_GROUPING:
            trp_print(p, 2, trd_body_keyword_grouping, " ");
            break;
        case TRD_KEYWORD_YANG_DATA:
            trp_print(p, 2, trd_body_keyword_yang_data, " ");
            break;
        default:
            break;
        }
        break;
    }
}

/** Get string length of stored keyword. */
static size_t
trp_keyword_type_strlen(trt_keyword_type a)
{
    switch (a) {
    case TRD_KEYWORD_MODULE:
        return sizeof(trd_top_keyword_module) - 1;
    case TRD_KEYWORD_SUBMODULE:
        return sizeof(trd_top_keyword_submodule) - 1;
    case TRD_KEYWORD_AUGMENT:
        return sizeof(trd_body_keyword_augment) - 1;
    case TRD_KEYWORD_RPC:
        return sizeof(trd_body_keyword_rpc) - 1;
    case TRD_KEYWORD_NOTIF:
        return sizeof(trd_body_keyword_notif) - 1;
    case TRD_KEYWORD_GROUPING:
        return sizeof(trd_body_keyword_grouping) - 1;
    case TRD_KEYWORD_YANG_DATA:
        return sizeof(trd_body_keyword_yang_data) - 1;
    default:
        return 0;
    }
}

/** Print .str which is string of name or path. */
static void
trt_print_keyword_stmt_str(struct trt_keyword_stmt a, uint32_t mll, struct trt_printing *p)
{
    if ((a.str == NULL) || (a.str[0] == '\0')) {
        return;
    }

    /* module name cannot be splitted */
    if ((a.type == TRD_KEYWORD_MODULE) || (a.type == TRD_KEYWORD_SUBMODULE)) {
        trp_print(p, 1, a.str);
        return;
    }

    /* else for trd_keyword_stmt_body do */

    const char slash = '/';
    /* set begin indentation */
    const uint32_t ind_initial = TRD_INDENT_LINE_BEGIN + trp_keyword_type_strlen(a.type) + 1;
    const uint32_t ind_divided = ind_initial + TRD_INDENT_LONG_LINE_BREAK;
    /* flag if path must be splitted to more lines */
    ly_bool linebreak_was_set = 0;
    /* flag if at least one subpath was printed */
    ly_bool subpath_printed = 0;
    /* the sum of the sizes of the substrings on the current line */
    uint32_t how_far = 0;

    /* pointer to start of the subpath */
    const char *sub_ptr = a.str;
    /* size of subpath from sub_ptr */
    size_t sub_len = 0;

    while (sub_ptr[0] != '\0') {
        /* skip slash */
        const char *tmp = sub_ptr[0] == slash ? sub_ptr + 1 : sub_ptr;
        /* get position of the end of substr */
        tmp = strchr(tmp, slash);
        /* set correct size if this is a last substring */
        sub_len = tmp == NULL ? strlen(sub_ptr) : (size_t)(tmp - sub_ptr);
        /* actualize sum of the substring's sizes on the current line */
        how_far += sub_len;
        /* correction due to colon character if it this is last substring */
        how_far = *(sub_ptr + sub_len) == '\0' ? how_far + 1 : how_far;
        /* choose indentation which depends on
         * whether the string is printed on multiple lines or not
         */
        uint32_t ind = linebreak_was_set ? ind_divided : ind_initial;
        if (ind + how_far <= mll) {
            /* printing before max line length */
            sub_ptr = trg_print_substr(sub_ptr, sub_len, p);
            subpath_printed = 1;
        } else {
            /* printing on new line */
            if (subpath_printed == 0) {
                /* first subpath is too long but print it at first line anyway */
                sub_ptr = trg_print_substr(sub_ptr, sub_len, p);
                subpath_printed = 1;
                continue;
            }
            trg_print_linebreak(p);
            trg_print_n_times(ind_divided, ' ', p);
            linebreak_was_set = 1;
            sub_ptr = trg_print_substr(sub_ptr, sub_len, p);
            how_far = sub_len;
            subpath_printed = 1;
        }
    }
}

/** Print separator based on .type. */
static void
trt_print_keyword_stmt_end(struct trt_keyword_stmt a, struct trt_printing *p)
{
    if ((a.type != TRD_KEYWORD_MODULE) && (a.type != TRD_KEYWORD_SUBMODULE)) {
        trp_print(p, 1, ":");
    }
}

/** Print entire struct trt_keyword_stmt structure. */
static void
trp_print_keyword_stmt(struct trt_keyword_stmt a, uint32_t mll, struct trt_printing *p)
{
    if (trp_keyword_stmt_is_empty(a)) {
        return;
    }
    trt_print_keyword_stmt_begin(a, p);
    trt_print_keyword_stmt_str(a, mll, p);
    trt_print_keyword_stmt_end(a, p);
}

/******************************************************************************
 * Main trp functions
 *****************************************************************************/

/** Printing one line including wrapper and node which can be incomplete. */
static void
trp_print_line(struct trt_node node, struct trt_pck_print pck, struct trt_pck_indent ind, struct trt_printing *p)
{
    trp_print_wrapper(ind.wrapper, p);
    trp_print_node(node, pck, ind.in_node, p);
}

/** Printing one line including wrapper and <status>--<flags> <name><option_mark>. */
static void
trp_print_line_up_to_node_name(struct trt_node node, struct trt_wrapper wr, struct trt_printing *p)
{
    trp_print_wrapper(wr, p);
    trp_print_node_up_to_name(node, p);
}

/**
 * @brief Check if leafref target must be change to string 'leafref' because his target string is too long.
 *
 * @param[in] n node containing leafref target.
 * @param[in] wr for node immersion depth.
 * @param[in] mll max line length border.
 * @return true if must be change to string 'leafref'.
 */
static ly_bool
trp_leafref_target_is_too_long(struct trt_node node, struct trt_wrapper wr, uint32_t mll)
{
    if (node.type.type != TRD_TYPE_TARGET) {
        return 0;
    }

    uint32_t cnt = 0;
    /* inject print function with strlen */
    struct trt_printing func = {&cnt, trp_injected_strlen, 0};

    /* count number of printed bytes */
    trp_print_wrapper(wr, &func);
    trg_print_n_times(TRD_INDENT_BTW_SIBLINGS, ' ', &func);
    trp_print_divided_node_up_to_name(node, &func);

    return cnt + strlen(node.type.str) > mll;
}

/** Get default indent in node based on node values. */
static struct trt_indent_in_node
trp_default_indent_in_node(struct trt_node node)
{
    struct trt_indent_in_node ret;

    ret.type = TRD_INDENT_IN_NODE_NORMAL;

    /* btw_name_opts */
    ret.btw_name_opts = trp_opts_keys_are_set(node.name) ?
            TRD_INDENT_BEFORE_KEYS : 0;

    /* btw_opts_type */
    if (!trp_type_is_empty(node.type)) {
        ret.btw_opts_type = trp_mark_is_used(node.name) ?
                TRD_INDENT_BEFORE_TYPE - trd_opts_mark_length :
                TRD_INDENT_BEFORE_TYPE;
    } else {
        ret.btw_opts_type = 0;
    }

    /* btw_type_iffeatures */
    ret.btw_type_iffeatures = !trp_iffeature_is_empty(node.iffeatures) ?
            TRD_INDENT_BEFORE_IFFEATURES : 0;

    return ret;
}

/**
 * @brief Setting linebreaks in trt_indent_in_node.
 *
 * The order where the linebreak tag can be placed is from the end.
 *
 * @param[in] item containing alignment lengths or already linebreak marks.
 * @return with a newly placed linebreak tag.
 * @return .type = TRD_INDENT_IN_NODE_FAILED if it is not possible to place a more linebreaks.
 */
static struct trt_indent_in_node
trp_indent_in_node_place_break(struct trt_indent_in_node ind)
{
    /* somewhere must be set a line break in node */
    struct trt_indent_in_node ret = ind;

    /* gradually break the node from the end */
    if ((ind.btw_type_iffeatures != trd_linebreak) && (ind.btw_type_iffeatures != 0)) {
        ret.btw_type_iffeatures = trd_linebreak;
    } else if ((ind.btw_opts_type != trd_linebreak) && (ind.btw_opts_type != 0)) {
        ret.btw_opts_type = trd_linebreak;
    } else if ((ind.btw_name_opts != trd_linebreak) && (ind.btw_name_opts != 0)) {
        /* set line break between name and opts */
        ret.btw_name_opts = trd_linebreak;
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
 * @param[in] node the whole <node> to be split.
 * @param[in] ind contains information in which part of the <node> the first half ends.
 * @return first half of the node, indent is unchanged.
 */
static struct trt_pair_indent_node
trp_first_half_node(struct trt_node node, struct trt_indent_in_node ind)
{
    struct trt_pair_indent_node ret = {ind, node};

    if (ind.btw_name_opts == trd_linebreak) {
        ret.node.name.type = trp_opts_keys_are_set(node.name) ?
                TRD_NODE_LISTLEAFLIST : node.name.type;
        ret.node.type = trp_empty_type();
        ret.node.iffeatures = trp_empty_iffeature();
    } else if (ind.btw_opts_type == trd_linebreak) {
        ret.node.type = trp_empty_type();
        ret.node.iffeatures = trp_empty_iffeature();
    } else if (ind.btw_type_iffeatures == trd_linebreak) {
        ret.node.iffeatures = trp_empty_iffeature();
    }

    return ret;
}

/**
 * @brief Get the second half of the node based on the linebreak mark.
 *
 * Items in the first half of the node will be empty.
 * Indentations belonging to the first node will be reset to zero.
 *
 * @param[in] node the whole <node> to be split.
 * @param[in] ind contains information in which part of the <node> the second half starts.
 * @return second half of the node, indent is newly set.
 */
static struct trt_pair_indent_node
trp_second_half_node(struct trt_node node, struct trt_indent_in_node ind)
{
    struct trt_pair_indent_node ret = {ind, node};

    if (ind.btw_name_opts < 0) {
        /* Logically, the information up to token <opts> should be deleted,
         * but the the trp_print_node function needs it to create
         * the correct indent.
         */
        ret.indent.btw_name_opts = 0;
        ret.indent.btw_opts_type = trp_type_is_empty(node.type) ?
                0 : TRD_INDENT_BEFORE_TYPE;
        ret.indent.btw_type_iffeatures = trp_iffeature_is_empty(node.iffeatures) ?
                0 : TRD_INDENT_BEFORE_IFFEATURES;
    } else if (ind.btw_opts_type == trd_linebreak) {
        ret.node.name.type = trp_opts_keys_are_set(node.name) ?
                TRD_NODE_LISTLEAFLIST : node.name.type;
        ret.indent.btw_name_opts = 0;
        ret.indent.btw_opts_type = 0;
        ret.indent.btw_type_iffeatures = trp_iffeature_is_empty(node.iffeatures) ?
                0 : TRD_INDENT_BEFORE_IFFEATURES;
    } else if (ind.btw_type_iffeatures == trd_linebreak) {
        ret.node.name.type = trp_opts_keys_are_set(node.name) ?
                TRD_NODE_LISTLEAFLIST : node.name.type;
        ret.node.type = trp_empty_type();
        ret.indent.btw_name_opts = 0;
        ret.indent.btw_opts_type = 0;
        ret.indent.btw_type_iffeatures = 0;
    }
    return ret;
}

/**
 * @brief Get the correct alignment for the node.
 *
 * @return .type == TRD_INDENT_IN_NODE_DIVIDED - the node does not fit in the line, some indent variable has negative value as a line break sign.
 * @return .type == TRD_INDENT_IN_NODE_NORMAL - the node fits into the line, all indent variables values has non-negative number.
 * @return .type == TRD_INDENT_IN_NODE_FAILED - the node does not fit into the line, all indent variables has negative or zero values, function failed.
 */
static struct trt_pair_indent_node
trp_try_normal_indent_in_node(struct trt_node n, struct trt_pck_print p, struct trt_pck_indent ind, uint32_t mll)
{
    uint32_t cnt = 0;
    /* inject print function with strlen */
    struct trt_printing func = {&cnt, trp_injected_strlen, 0};

    /* count number of printed bytes */
    trp_print_line(n, p, ind, &func);

    struct trt_pair_indent_node ret = {ind.in_node, n};

    if (cnt <= mll) {
        /* success */
        return ret;
    } else {
        ret.indent = trp_indent_in_node_place_break(ret.indent);
        if (ret.indent.type != TRD_INDENT_IN_NODE_FAILED) {
            /* erase information in node due to line break */
            ret = trp_first_half_node(n, ret.indent);
            /* check if line fits, recursive call */
            ret = trp_try_normal_indent_in_node(ret.node, p, (struct trt_pck_indent) {ind.wrapper, ret.indent}, mll);
            /* make sure that the result will be with the status divided
             * or eventually with status failed */
            ret.indent.type = ret.indent.type == TRD_INDENT_IN_NODE_FAILED ?
                    TRD_INDENT_IN_NODE_FAILED : TRD_INDENT_IN_NODE_DIVIDED;
        }
        return ret;
    }
}

/** Auxiliary function for trp_print_entire_node that prints split nodes. */
static void
trp_print_divided_node(struct trt_node node, struct trt_pck_print ppck, struct trt_pck_indent ipck, uint32_t mll, struct trt_printing *p)
{
    struct trt_pair_indent_node ind_node = trp_try_normal_indent_in_node(node, ppck, ipck, mll);

    if (ind_node.indent.type == TRD_INDENT_IN_NODE_FAILED) {
        /* nothing can be done, continue as usual */
        ind_node.indent.type = TRD_INDENT_IN_NODE_DIVIDED;
    }

    trp_print_line(ind_node.node, ppck, (struct trt_pck_indent) {ipck.wrapper, ind_node.indent}, p);

    const ly_bool entire_node_was_printed = trp_indent_in_node_are_eq(ipck.in_node, ind_node.indent);

    if (!entire_node_was_printed) {
        trg_print_linebreak(p);
        /* continue with second half node */
        ind_node = trp_second_half_node(node, ind_node.indent);
        /* continue with printing node */
        trp_print_divided_node(ind_node.node, ppck, (struct trt_pck_indent) {ipck.wrapper, ind_node.indent}, mll, p);
    } else {
        return;
    }
}

/** Printing of the wrapper and the whole node, which can be divided into several lines. */
static void
trp_print_entire_node(struct trt_node node, struct trt_pck_print ppck, struct trt_pck_indent ipck, uint32_t mll, struct trt_printing *p)
{
    if (trp_leafref_target_is_too_long(node, ipck.wrapper, mll)) {
        node.type.type = TRD_TYPE_LEAFREF;
    }

    /* check if normal indent is possible */
    struct trt_pair_indent_node ind_node1 = trp_try_normal_indent_in_node(node, ppck, ipck, mll);

    if (ind_node1.indent.type == TRD_INDENT_IN_NODE_NORMAL) {
        /* node fits to one line */
        trp_print_line(node, ppck, ipck, p);
    } else if (ind_node1.indent.type == TRD_INDENT_IN_NODE_DIVIDED) {
        /* node will be divided */
        /* print first half */
        {
            struct trt_pck_indent tmp = {ipck.wrapper, ind_node1.indent};
            /* pretend that this is normal node */
            tmp.in_node.type = TRD_INDENT_IN_NODE_NORMAL;
            trp_print_line(ind_node1.node, ppck, tmp, p);
        }
        trg_print_linebreak(p);
        /* continue with second half on new line */
        {
            struct trt_pair_indent_node ind_node2 = trp_second_half_node(node, ind_node1.indent);
            struct trt_pck_indent tmp = {trp_wrapper_if_last_sibling(ipck.wrapper, node.last_one), ind_node2.indent};
            trp_print_divided_node(ind_node2.node, ppck, tmp, mll, p);
        }
    } else if (ind_node1.indent.type == TRD_INDENT_IN_NODE_FAILED) {
        /* node name is too long */
        trp_print_line_up_to_node_name(node, ipck.wrapper, p);
        if (trp_node_body_is_empty(node)) {
            return;
        } else {
            trg_print_linebreak(p);
            struct trt_pair_indent_node ind_node2 = trp_second_half_node(node, ind_node1.indent);
            ind_node2.indent.type = TRD_INDENT_IN_NODE_DIVIDED;
            struct trt_pck_indent tmp = {trp_wrapper_if_last_sibling(ipck.wrapper, node.last_one), ind_node2.indent};
            trp_print_divided_node(ind_node2.node, ppck, tmp, mll, p);
        }

    }
}

/******************************************************************************
 * Definition of Tro reading functions
 *****************************************************************************/

/** Return trt_parent_cache filled with default values. */
static struct trt_parent_cache
tro_empty_parent_cache()
{
    return (struct trt_parent_cache)
           {
               TRD_ANCESTOR_ELSE,
               LYS_STATUS_CURR,
               LYS_CONFIG_W,
               NULL
           };
}

/**
 * @brief Get new trt_parent_cache if we apply the transfer to the child.
 *
 * @param[in] ca is parent cache for current node.
 * @param[in] pn is current node data from lysp_tree.
 * @return cache for the current node.
 */
static struct trt_parent_cache
tro_parent_cache_for_child(struct trt_parent_cache ca, const struct lysp_node *pn)
{
    struct trt_parent_cache ret = {};

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

/** Find out if the current node has siblings. */
static ly_bool
tro_read_if_sibling_exists(const struct trt_tree_ctx *tc)
{
    /* this code is unfortunately very duplicated with the function tro_modi_next_sibling.
     * What's more, changes in the function tro_modi_next_sibling can be reflected in this function.
     */
    assert(tc != NULL && tc->pn != NULL && tc->module != NULL && tc->module->parsed != NULL);
    if (tc->pn->nodetype & (LYS_RPC | LYS_ACTION)) {
        if ((tc->section == TRD_SECT_RPCS) && (tc->tpn == tc->pn)) {
            const struct lysp_action *arr = tc->module->parsed->rpcs;
            return arr && tc->index_within_section + 1 < (int64_t)LY_ARRAY_COUNT(arr);
        } else {
            const struct lysp_action *arr_actions = lysp_node_actions(tc->pn->parent);
            const struct lysp_notif *arr_notifs = lysp_node_notifs(tc->pn->parent);
            int64_t actions_count = (int64_t) LY_ARRAY_COUNT(arr_actions);

            if ((actions_count > 0) && ((tc->index == -1) || (tc->index + 1 < actions_count))) {
                return 1;
            } else if (LY_ARRAY_COUNT(arr_notifs) > 0) {
                return 1;
            } else {
                return 0;
            }
        }
    } else if (tc->pn->nodetype & LYS_INPUT) {
        const struct lysp_action *parent = (struct lysp_action *)tc->pn->parent;
        const struct lysp_node *output_data = parent->output.data;
        /* if output action has data */
        return output_data ? 1 : 0;
    } else if (tc->pn->nodetype & LYS_OUTPUT) {
        return 0;
    } else if (tc->pn->nodetype & LYS_NOTIF) {
        if ((tc->section == TRD_SECT_NOTIF) && (tc->tpn == tc->pn)) {
            const struct lysp_notif *arr = tc->module->parsed->notifs;
            return arr && tc->index_within_section + 1 < (int64_t)LY_ARRAY_COUNT(arr);
        } else {
            const struct lysp_notif *arr_notifs = lysp_node_notifs(tc->pn->parent);
            int64_t notifs_count = (int64_t) LY_ARRAY_COUNT(arr_notifs);

            if ((notifs_count > 0) && ((tc->index == -1) || (tc->index + 1 < notifs_count))) {
                return 1;
            } else {
                return 0;
            }
        }
    } else {
        if (tc->pn->next != NULL) {
            return 1;
        } else {
            return 0;
        }
    }
}

/** Check if list statement has keys. */
static ly_bool
tro_lysp_list_has_keys(const struct lysp_node *pn)
{
    const struct lysp_node_list *list = (const struct lysp_node_list *) pn;

    return trg_charptr_has_data(list->key);
}

/** Check if it contains at least one feature. */
static struct trt_iffeature
tro_lysp_node_to_iffeature(const struct lysp_qname *iffs)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool ret = 0;

    LY_ARRAY_FOR(iffs, u) {
        ret = 1;
        break;
    }
    return (struct trt_iffeature) {
               ret
    };
}

/** Find out if leaf is also the key in last list. */
static ly_bool
tro_lysp_leaf_is_key(const struct lysp_node *pn, const struct lysp_node_list *ca_last_list)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *) pn;
    const struct lysp_node_list *list = ca_last_list;

    if (list == NULL) {
        return 0;
    }
    return trg_charptr_has_data(list->key) ?
           trg_word_is_present(list->key, leaf->name, ' ') : 0;
}

/** Check if container's type is presence. */
static ly_bool
tro_lysp_container_has_presence(const struct lysp_node *pn)
{
    return trg_charptr_has_data(((struct lysp_node_container *)pn)->presence);
}

/** Get leaflist's path without lysp_node type control. */
static const char *
tro_lysp_leaflist_refpath(const struct lysp_node *pn)
{
    const struct lysp_node_leaflist *list = (const struct lysp_node_leaflist *) pn;

    return list->type.path != NULL ? list->type.path->expr : NULL;
}

/** Get leaflist's type name without lysp_node type control. */
static const char *
tro_lysp_leaflist_type_name(const struct lysp_node *pn)
{
    const struct lysp_node_leaflist *list = (const struct lysp_node_leaflist *) pn;

    return list->type.name;
}

/** Get leaf's path without lysp_node type control. */
static const char *
tro_lysp_leaf_refpath(const struct lysp_node *pn)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *) pn;

    return leaf->type.path != NULL ? leaf->type.path->expr : NULL;
}

/** Get leaf's type name without lysp_node type control. */
static const char *
tro_lysp_leaf_type_name(const struct lysp_node *pn)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *) pn;

    return leaf->type.name;
}

/**
 * @brief Get pointer to data using node type specification and getter function.
 *
 * @param[in] flags is node type specification. If it is the correct node, the getter function is called.
 * @param[in] f is getter function which provides the desired char pointer from the structure.
 * @return NULL if node has wrong type or getter function return pointer to NULL.
 * @return pointer to desired char pointer obtained from the node.
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

/** Transformation of the lysp flags to Yang tree <status>. */
static trt_status_type
tro_lysp_flags2status(uint16_t flags)
{
    return flags & LYS_STATUS_OBSLT ? TRD_STATUS_TYPE_OBSOLETE :
           flags & LYS_STATUS_DEPRC ? TRD_STATUS_TYPE_DEPRECATED :
           TRD_STATUS_TYPE_CURRENT;
}

/** Transformation of the lysp flags to Yang tree <flags> but more specifically 'ro' or 'rw'. */
static trt_flags_type
tro_lysp_flags2config(uint16_t flags)
{
    return flags & LYS_CONFIG_R ?
           TRD_FLAGS_TYPE_RO : TRD_FLAGS_TYPE_RW;
}

/** Get name of the module. */
static struct trt_keyword_stmt
tro_read_module_name(const struct trt_tree_ctx *a)
{
    assert(a != NULL && a->module != NULL && a->module->name != NULL);
    return (struct trt_keyword_stmt)
           {
               TRD_KEYWORD_MODULE,
               a->module->name
           };
}

static trt_status_type
tro_resolve_status(uint16_t nodetype, uint16_t flags, uint16_t ca_lys_status)
{
    /* LYS_INPUT and LYS_OUTPUT is special case */
    if (nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        return tro_lysp_flags2status(ca_lys_status);
        /* if ancestor's status is deprc or obslt and also node's status is not set */
    } else if (ca_lys_status & (LYS_STATUS_DEPRC | LYS_STATUS_OBSLT) && !(flags & (LYS_STATUS_CURR | LYS_STATUS_DEPRC | LYS_STATUS_OBSLT))) {
        /* get ancestor's status */
        return tro_lysp_flags2status(ca_lys_status);
    } else {
        /* else get node's status */
        return tro_lysp_flags2status(flags);
    }
}

static trt_flags_type
tro_resolve_flags(uint16_t nodetype, uint16_t flags, trt_ancestor_type ca_ancestor, uint16_t ca_lys_config)
{
    if (nodetype & LYS_INPUT || (ca_ancestor == TRD_ANCESTOR_RPC_INPUT)) {
        return TRD_FLAGS_TYPE_RPC_INPUT_PARAMS;
    } else if (nodetype & LYS_OUTPUT || (ca_ancestor == TRD_ANCESTOR_RPC_OUTPUT)) {
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

static trt_node_type
tro_resolve_node_type(const struct lysp_node *pn, const struct lysp_node_list *ca_last_list)
{
    if (pn->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        return TRD_NODE_ELSE;
    } else if (pn->nodetype & LYS_CASE) {
        return TRD_NODE_CASE;
    } else if (pn->nodetype & LYS_CHOICE && !(pn->flags & LYS_MAND_TRUE)) {
        return TRD_NODE_OPTIONAL_CHOICE;
    } else if (pn->nodetype & LYS_CHOICE) {
        return TRD_NODE_CHOICE;
    } else if (pn->nodetype & LYS_CONTAINER && tro_lysp_container_has_presence(pn)) {
        return TRD_NODE_CONTAINER;
    } else if (pn->nodetype & LYS_LIST && tro_lysp_list_has_keys(pn)) {
        return TRD_NODE_KEYS;
    } else if (pn->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        return TRD_NODE_LISTLEAFLIST;
    } else if (pn->nodetype & (LYS_ANYDATA | LYS_ANYXML) && !(pn->flags & LYS_MAND_TRUE)) {
        return TRD_NODE_OPTIONAL;
    } else if (pn->nodetype & LYS_LEAF && !(pn->flags & LYS_MAND_TRUE) && !tro_lysp_leaf_is_key(pn, ca_last_list)) {
        return TRD_NODE_OPTIONAL;
    } else {
        return TRD_NODE_ELSE;
    }
}

/** Transformation of current lysp_node to struct trt_node. */
static struct trt_node
tro_read_node(struct trt_parent_cache ca, const struct trt_tree_ctx *tc)
{
    assert(tc != NULL && tc->pn != NULL);
    const struct lysp_node *pn = tc->pn;

    assert(pn->nodetype != LYS_UNKNOWN);

    struct trt_node ret = trp_empty_node();

    /* remember:
     * - LYS_INTPUT and LYS_OUTPUT lysp_node don't have
     *      flags, name and iffeatures element in their structure.
     */

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
    ret.name.str =
            pn->nodetype & (LYS_INPUT) ? "input" :
            pn->nodetype & (LYS_OUTPUT) ? "output" :
            pn->name;

    /* <type> */
    const char *tmp = NULL;

    if ((tmp = tro_lysp_node_charptr(LYS_LEAFLIST, tro_lysp_leaflist_refpath, pn))) {
        ret.type = (struct trt_type) {
            TRD_TYPE_TARGET, tmp
        };
    } else if ((tmp = tro_lysp_node_charptr(LYS_LEAFLIST, tro_lysp_leaflist_type_name, pn))) {
        ret.type = (struct trt_type) {
            TRD_TYPE_NAME, tmp
        };
    } else if ((tmp = tro_lysp_node_charptr(LYS_LEAF, tro_lysp_leaf_refpath, pn))) {
        ret.type = (struct trt_type) {
            TRD_TYPE_TARGET, tmp
        };
    } else if ((tmp = tro_lysp_node_charptr(LYS_LEAF, tro_lysp_leaf_type_name, pn))) {
        ret.type = (struct trt_type) {
            TRD_TYPE_NAME, tmp
        };
    } else if ((pn->nodetype & LYS_ANYDATA) == LYS_ANYDATA) {
        ret.type = (struct trt_type) {
            TRD_TYPE_NAME, "anydata"
        };
    } else if (pn->nodetype & LYS_ANYXML) {
        ret.type = (struct trt_type) {
            TRD_TYPE_NAME, "anyxml"
        };
    } else {
        ret.type = trp_empty_type();
    }

    /* <iffeature> */
    ret.iffeatures = pn->nodetype & (LYS_INPUT | LYS_OUTPUT) ?
            trp_empty_iffeature() : tro_lysp_node_to_iffeature(pn->iffeatures);

    ret.last_one = !tro_read_if_sibling_exists(tc);

    return ret;
}

/******************************************************************************
 * Modify Tro getters
 *****************************************************************************/

/**
 * @brief Change the pointer to the current node to its parent.
 * @return 1 if the node had parents and the change was successful
 * @return 0 if if the node did not have parents and the pointer to the current node did not change.
 */
static ly_bool
tro_modi_parent(struct trt_tree_ctx *tc)
{
    assert(tc != NULL && tc->pn != NULL);
    /* If no parent exists, stay in actual node. */
    if (tc->pn != tc->tpn) {
        tc->pn = tc->pn->parent;
        return 1;
    } else {
        return 0;
    }
}

/** Change the pointer to the current node to its child. */
static struct trt_node
tro_modi_next_child(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    assert(tc != NULL && tc->pn != NULL);

    struct trt_parent_cache new_ca = tro_parent_cache_for_child(ca, tc->pn);

    if (tc->pn->nodetype & (LYS_ACTION | LYS_RPC)) {
        const struct lysp_action *act = (struct lysp_action *)tc->pn;
        const struct lysp_node *input_data = act->input.data;
        const struct lysp_node *output_data = act->output.data;
        if (input_data) {
            /* go to LYS_INPUT */
            tc->pn = (const struct lysp_node *) &act->input;
            return tro_read_node(new_ca, tc);
        } else if (output_data) {
            /* go to LYS_OUTPUT */
            tc->pn = (const struct lysp_node *) &act->output;
            return tro_read_node(new_ca, tc);
        } else {
            /* input action and output action are not set */
            return trp_empty_node();
        }
    } else {
        const struct lysp_node *pn = lysp_node_children(tc->pn);
        if (pn != NULL) {
            tc->pn = pn;
            return tro_read_node(new_ca, tc);
        } else {
            /* current node can't have children or has no children */
            /* but maybe has some actions or notifs */
            const struct lysp_action *arr_actions = lysp_node_actions(tc->pn);
            const struct lysp_notif *arr_notifs = lysp_node_notifs(tc->pn);
            if (LY_ARRAY_COUNT(arr_actions) > 0) {
                tc->index = 0;
                tc->pn = (const struct lysp_node *)arr_actions;
                return tro_read_node(new_ca, tc);
            } else if (LY_ARRAY_COUNT(arr_notifs) > 0) {
                tc->index = 0;
                tc->pn = (const struct lysp_node *)arr_notifs;
                return tro_read_node(new_ca, tc);
            } else {
                return trp_empty_node();
            }
        }
    }
}

/** Change the pointer to the current node to its first sibling. */
static void
tro_modi_first_sibling(struct trt_tree_ctx *tc)
{
    assert(tc != NULL && tc->pn != NULL && tc->module != NULL && tc->module->parsed != NULL);

    if (tro_modi_parent(tc)) {
        tro_modi_next_child(tro_empty_parent_cache(), tc);
    } else {
        /* current node is top-node */

        struct lysp_module *pm = tc->module->parsed;

        switch (tc->section) {
        case TRD_SECT_MODULE:
            tc->pn = pm->data;
            break;
        case TRD_SECT_AUGMENT:
            tc->pn = pm->augments[tc->index_within_section].child;
            break;
        case TRD_SECT_RPCS:
            tc->index_within_section = 0;
            tc->pn = (struct lysp_node *) pm->rpcs;
            break;
        case TRD_SECT_NOTIF:
            tc->index_within_section = 0;
            tc->pn = (struct lysp_node *) pm->notifs;
            break;
        case TRD_SECT_GROUPING:
            tc->pn = pm->groupings[tc->index_within_section].data;
            break;
        case TRD_SECT_YANG_DATA:
            /*TODO: yang-data is not supported now */
            break;
        }

        /* update pointer to top-node */
        tc->tpn = tc->pn;
    }
}

#define NEXT_SIBLING_BY_PARSED_TREE() {\
    if(arr && tc->index_within_section + 1 < (int64_t)LY_ARRAY_COUNT(arr)) {\
        tc->index_within_section++;\
        tc->pn = (const struct lysp_node*) (&(arr[tc->index_within_section]));\
        tc->tpn = tc->pn;\
        return tro_read_node(ca, tc);\
    } else {\
        return trp_empty_node();\
    }\
}

/** Change the pointer to the current node to its next sibling. */
static struct trt_node
tro_modi_next_sibling(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    /* if you make changes in this function, please make sure to
     * make a change to function tro_read_if_sibling_exists as well,
     * as it is essentially the same.
     */
    assert(tc != NULL && tc->pn != NULL && tc->module != NULL && tc->module->parsed != NULL);

    if (tc->pn->nodetype & (LYS_RPC | LYS_ACTION)) {
        /* if current section is rpcs and current node is top-node */
        if ((tc->section == TRD_SECT_RPCS) && (tc->tpn == tc->pn)) {
            /* then next sibling is the next action located in parsed module */
            const struct lysp_action *arr = tc->module->parsed->rpcs;
            /* return next action or empty node */
            NEXT_SIBLING_BY_PARSED_TREE()
            /* else next sibling is located in parent */
        } else {
            const struct lysp_action *arr_actions = lysp_node_actions(tc->pn->parent);
            const struct lysp_notif *arr_notifs = lysp_node_notifs(tc->pn->parent);
            int64_t actions_count = (int64_t) LY_ARRAY_COUNT(arr_actions);

            if ((actions_count > 0) && ((tc->index == -1) || (tc->index + 1 < actions_count))) {
                /* return first or next action */
                tc->index++;
                tc->pn = (const struct lysp_node *) (&(arr_actions[tc->index]));
                return tro_read_node(ca, tc);
            } else if (LY_ARRAY_COUNT(arr_notifs) > 0) {
                /* return first notification */
                tc->index = 0;
                tc->pn = (const struct lysp_node *)arr_notifs;
                return tro_read_node(ca, tc);
            } else {
                tc->index = -1;
                /* return empty node because sibling does not exist */
                return trp_empty_node();
            }
        }
        /* if current node is input action */
    } else if (tc->pn->nodetype & LYS_INPUT) {
        const struct lysp_action *parent = (struct lysp_action *)tc->pn->parent;
        const struct lysp_node *output_data = parent->output.data;
        /* if output action has data */
        if (output_data) {
            /* then next sibling is output action */
            tc->pn = (struct lysp_node *)&parent->output;
            return tro_read_node(ca, tc);
        } else {
            /* else input action has no sibling */
            return trp_empty_node();
        }
        /* if current node is output action */
    } else if (tc->pn->nodetype & LYS_OUTPUT) {
        /* then next sibling does not exist */
        return trp_empty_node();
        /* if current node is notification */
    } else if (tc->pn->nodetype & LYS_NOTIF) {
        /* if current section is notifications and current node is top-node */
        if ((tc->section == TRD_SECT_NOTIF) && (tc->tpn == tc->pn)) {
            /* then next sibling is the next action located in parsed module */
            const struct lysp_notif *arr = tc->module->parsed->notifs;
            /* return next notification or empty node */
            NEXT_SIBLING_BY_PARSED_TREE()
        } else {
            /* else next sibling is located in parent */
            const struct lysp_notif *arr_notifs = lysp_node_notifs(tc->pn->parent);
            /* if next notification exists in parent node */
            int64_t notifs_count = (int64_t) LY_ARRAY_COUNT(arr_notifs);

            if ((notifs_count > 0) && ((tc->index == -1) || (tc->index + 1 < notifs_count))) {
                /* return first or next notification */
                tc->index++;
                tc->pn = (const struct lysp_node *) (&(arr_notifs[tc->index]));
                return tro_read_node(ca, tc);
            } else {
                tc->index = -1;
                /* return empty node because sibling does not exist */
                return trp_empty_node();
            }
        }
    } else {
        /* else actual node is some node with 'next' element */
        if (tc->pn->next != NULL) {
            /* if current node is top-node */
            if (tc->tpn == tc->pn) {
                /* shift pointer to top-node too */
                tc->tpn = tc->pn->next;
            }
            tc->pn = tc->pn->next;
            /* return next sibling by 'next' element */
            return tro_read_node(ca, tc);
        } else {
            /* actual node is last node */
            return trp_empty_node();
        }
    }
}

#undef NEXT_SIBLING_BY_PARSED_TREE

/** Get next augment section if exists. */
static struct trt_keyword_stmt
tro_modi_next_augment(struct trt_tree_ctx *tc)
{
    assert(tc != NULL && tc->module != NULL && tc->module->parsed != NULL);
    const struct lysp_augment *arr = tc->module->parsed->augments;

    tc->section = TRD_SECT_AUGMENT;
    if (arr && (tc->index_within_section + 1 < (int64_t)LY_ARRAY_COUNT(arr))) {
        tc->index_within_section++;
        const struct lysp_augment *item = &(arr[tc->index_within_section]);
        if (item->child) {
            tc->pn = item->child;
            tc->tpn = tc->pn;
            return (struct trt_keyword_stmt) {
                       TRD_KEYWORD_AUGMENT, item->nodeid
            };
        }
    }
    return trp_empty_keyword_stmt();
}

/** Get rpcs section if exists. */
static struct trt_keyword_stmt
tro_modi_get_rpcs(struct trt_tree_ctx *tc)
{
    assert(tc != NULL && tc->module != NULL && tc->module->parsed != NULL);
    const struct lysp_action *arr = tc->module->parsed->rpcs;

    if (arr == NULL) {
        return trp_empty_keyword_stmt();
    }

    tc->section = TRD_SECT_RPCS;
    tc->pn = (const struct lysp_node *)arr;
    tc->tpn = tc->pn;
    tc->index_within_section = 0;
    return (struct trt_keyword_stmt) {
               TRD_KEYWORD_RPC, NULL
    };
}

/** Get norification section if exists. */
static struct trt_keyword_stmt
tro_modi_get_notifications(struct trt_tree_ctx *tc)
{
    assert(tc != NULL && tc->module != NULL && tc->module->parsed != NULL);
    const struct lysp_notif *arr = tc->module->parsed->notifs;

    if (arr == NULL) {
        return trp_empty_keyword_stmt();
    }

    tc->section = TRD_SECT_NOTIF;
    tc->pn = (const struct lysp_node *)arr;
    tc->tpn = tc->pn;
    tc->index_within_section = 0;
    return (struct trt_keyword_stmt) {
               TRD_KEYWORD_NOTIF, NULL
    };
}

/** Get next grouping section if exists. */
static struct trt_keyword_stmt
tro_modi_next_grouping(struct trt_tree_ctx *tc)
{
    assert(tc != NULL && tc->module != NULL && tc->module->parsed != NULL);
    const struct lysp_grp *arr = tc->module->parsed->groupings;

    tc->section = TRD_SECT_GROUPING;
    if (arr && (tc->index_within_section + 1 < (int64_t)LY_ARRAY_COUNT(arr))) {
        tc->index_within_section++;
        const struct lysp_grp *item = &(arr[tc->index_within_section]);
        if (item->data) {
            tc->pn = item->data;
            tc->tpn = tc->pn;
            return (struct trt_keyword_stmt) {
                       TRD_KEYWORD_GROUPING, item->name
            };
        }
    }
    return trp_empty_keyword_stmt();
}

/** Get next yang-data section if exists. */
static struct trt_keyword_stmt
tro_modi_next_yang_data(struct trt_tree_ctx *tc)
{
    tc->section = TRD_SECT_YANG_DATA;
    /* TODO: yang-data is not supported */
    return trp_empty_keyword_stmt();
}

/******************************************************************************
 * Print Tro getters
 *****************************************************************************/

/** Print current node's iffeatures. */
static void
tro_print_features_names(const struct trt_tree_ctx *a, struct trt_printing *p)
{
    const struct lysp_qname *iffs = a->pn->iffeatures;

    LY_ARRAY_COUNT_TYPE i;

    LY_ARRAY_FOR(iffs, i) {
        if (i == 0) {
            trp_print(p, 1, iffs[i].str);
        } else {
            trp_print(p, 2, ",", iffs[i].str);
        }
    }

}

/** Print current list's keys. */
static void
tro_print_keys(const struct trt_tree_ctx *a, struct trt_printing *p)
{
    const struct lysp_node *pn = a->pn;

    if (pn->nodetype != LYS_LIST) {
        return;
    }
    struct lysp_node_list *list = (struct lysp_node_list *) pn;

    if (trg_charptr_has_data(list->key)) {
        trp_print(p, 1, list->key);
    }
}

/******************************************************************************
 * Definition of tree browsing functions
 *****************************************************************************/

/**
 * @brief Get size of node name.
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
           ((int32_t)(name_len + trd_opts_mark_length)) * (-1) :
           (int32_t)name_len;
}

/**
 * @brief Calculate the btw_opts_type indent size for a particular node.
 *
 * @param[in] name is the node for which we get btw_opts_type.
 * @param[in] max_len is the maximum value of btw_opts_type that it can have.
 * @return btw_opts_type for node.
 */
static int16_t
trb_calc_btw_opts_type(struct trt_node_name name, int16_t max_len4all)
{
    int32_t name_len = trb_strlen_of_name_and_mark(name);
    /* negative value indicate that in name is some opt mark */
    int16_t min_len = name_len < 0 ?
            TRD_INDENT_BEFORE_TYPE - trd_opts_mark_length :
            TRD_INDENT_BEFORE_TYPE;
    int16_t ret = trg_abs(max_len4all) - trg_abs(name_len);

    /* correction -> negative indicate that name is too long. */
    return ret < 0 ? min_len : ret;
}

/**
 * @brief Print node.
 *
 * This function is wrapper for trp_print_entire_node function.
 * But difference is that take max_gap_before_type parameter which will be used to set the unified alignment.
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
    /* else - print actual node with default indent */
    trp_print_entire_node(node, (struct trt_pck_print) {tc, pc->fp.print},
            (struct trt_pck_indent) {wr, ind},
            pc->max_line_length, &pc->print);
}

/**
 * @brief Check if parent of the current node is the last of his siblings.
 *
 * To mantain stability use this function only if the current node is the first of the siblings.
 * Side-effect -> current node is set to the first sibling if node has a parent otherwise no side-effect.
 */
static ly_bool
trb_parent_is_last_sibling(struct trt_fp_all fp, struct trt_tree_ctx *tc)
{
    if (fp.modify.parent(tc)) {
        ly_bool ret = fp.read.if_sibling_exists(tc);
        fp.modify.next_child(tro_empty_parent_cache(), tc);
        return !ret;
    } else {
        return !fp.read.if_sibling_exists(tc);
    }
}

/**
 * @brief Find sibling with the biggest node name and return that size.
 *
 * Side-effect -> Current node is set to the first sibling.
 * @return positive number lesser than upper_limit as a sign that only the node name is included in the size.
 * @return negative number whose absolute value is less than upper_limit and sign that node name and his opt mark is included in the size.
 */
static int32_t
trb_maxlen_node_name(struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    pc->fp.modify.first_sibling(tc);
    int32_t ret = 0;

    for (struct trt_node node = pc->fp.read.node(ca, tc);
            !trp_node_is_empty(node);
            node = pc->fp.modify.next_sibling(ca, tc)) {
        int32_t maxlen = trb_strlen_of_name_and_mark(node.name);
        ret = trg_abs(maxlen) > trg_abs(ret) ? maxlen : ret;
    }
    pc->fp.modify.first_sibling(tc);
    return ret;
}

/**
 * @brief Find maximal indent between <opts> and <type> for siblings.
 *
 * Side-effect -> Current node is set to the first sibling.
 * @return max btw_opts_type value for rest of the siblings
 */
static int16_t
trb_max_btw_opts_type4siblings(struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    int32_t maxlen_node_name = trb_maxlen_node_name(ca, pc, tc);
    int16_t ind_before_type = maxlen_node_name < 0 ?
            TRD_INDENT_BEFORE_TYPE - 1 : /* mark was present */
            TRD_INDENT_BEFORE_TYPE;

    return trg_abs(maxlen_node_name) + ind_before_type;
}

/**
 * @brief Find out if it is possible to unify the alignment before <type>.
 *
 * The goal is for all node siblings to have the same alignment for <type> as if they were in a column.
 * All siblings who cannot adapt because they do not fit on the line at all are ignored.
 * @return 0 if all siblings cannot fit on the line.
 * @return positive number indicating the maximum number of spaces before <type> if the length of the node name is 0.
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
 * Nodes are printed, including unified sibling node alignment (align <type> to column).
 * Side-effect -> current node is set to the last sibling.
 */
static void
trb_print_nodes(struct trt_wrapper wr, struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    /* if node is last sibling, then do not add '|' to wrapper */
    wr = trb_parent_is_last_sibling(pc->fp, tc) ?
            trp_wrapper_set_shift(wr) : trp_wrapper_set_mark(wr);

    /* try unified indentation in node */
    const uint32_t max_gap_before_type = trb_try_unified_indent(ca, pc, tc);

    ly_bool sibling_flag = 0;
    ly_bool child_flag = 0;

    /* print all siblings */
    do {
        /* print linebreak before printing actual node */
        trg_print_linebreak(&pc->print);
        /* print node */
        trb_print_entire_node(max_gap_before_type, wr, ca, pc, tc);

        struct trt_parent_cache new_ca = tro_parent_cache_for_child(ca, tc->pn);
        /* go to the actual node's child or stay in actual node */
        struct trt_node node = pc->fp.modify.next_child(ca, tc);
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
 * @param[in] max_gap_begore_type is result from trb_try_unified_indent function for root node. Set parameter to 0 if distance does not matter.
 * @param[in] wr is wrapper saying how deep in the whole tree is the root of the subtree.
 * @param[in] ca is parent_cache from root's parent. If root is top-level node, insert result of the tro_empty_parent_cache function.
 * @param[in,out] pc is pointer to the printer (trp) context.
 * @param[in,out] tc is pointer to the tree (tro) context.
 */
static void
trb_print_subtree_nodes(uint32_t max_gap_before_type, struct trt_wrapper wr, struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    trb_print_entire_node(max_gap_before_type, wr, ca, pc, tc);
    /* go to the actual node's child */
    struct trt_parent_cache new_ca = tro_parent_cache_for_child(ca, tc->pn);
    struct trt_node node = pc->fp.modify.next_child(ca, tc);

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
 */
static uint32_t
trb_get_number_of_siblings(struct trt_fp_modify_ctx fp, struct trt_tree_ctx *tc)
{
    /* including actual node */
    fp.first_sibling(tc);
    uint32_t ret = 1;
    struct trt_node node = trp_empty_node();

    while (!trp_node_is_empty(node = fp.next_sibling(tro_empty_parent_cache(), tc))) {
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
 */
static void
trb_print_family_tree(trd_wrapper_type wr_t, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_wrapper wr = wr_t == TRD_WRAPPER_TOP ?
            trp_init_wrapper_top() :
            trp_init_wrapper_body();

    uint32_t total_parents = trb_get_number_of_siblings(pc->fp.modify, tc);
    struct trt_parent_cache ca = tro_empty_parent_cache();
    uint32_t max_gap_before_type = trb_try_unified_indent(ca, pc, tc);

    for (uint32_t i = 0; i < total_parents; i++) {
        trg_print_linebreak(&pc->print);
        trb_print_subtree_nodes(max_gap_before_type, wr, ca, pc, tc);
        pc->fp.modify.next_sibling(ca, tc);
    }
}

/******************************************************************************
 * Definition of trm main functions
 *****************************************************************************/

/** General function to prevent repetitiveness code. */
static void
trm_print_body_section(struct trt_keyword_stmt ks, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    if (trp_keyword_stmt_is_empty(ks)) {
        return;
    }
    trp_print_keyword_stmt(ks, pc->max_line_length, &pc->print);
    trb_print_family_tree(TRD_WRAPPER_BODY, pc, tc);
}

/** Print 'module' keyword, its name and all nodes. */
static void
trm_print_module_section(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    trp_print_keyword_stmt(pc->fp.read.module_name(tc), pc->max_line_length, &pc->print);
    /* check if module section contains any data */
    if (tc->tpn != NULL) {
        trb_print_family_tree(TRD_WRAPPER_TOP, pc, tc);
    }
}

/** For all augment sections: print 'augment' keyword, its target node and all nodes. */
static void
trm_print_augmentations(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    tc->index_within_section = -1;
    ly_bool once = 1;

    for (struct trt_keyword_stmt ks = pc->fp.modify.next_augment(tc);
            !trp_keyword_stmt_is_empty(ks);
            ks = pc->fp.modify.next_augment(tc)) {
        if (once) {
            trg_print_linebreak(&pc->print);
            trg_print_linebreak(&pc->print);
            once = 0;
        } else {
            trg_print_linebreak(&pc->print);
        }
        trm_print_body_section(ks, pc, tc);
    }
}

/** For rpcs section: print 'rpcs' keyword and all its nodes. */
static void
trm_print_rpcs(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    tc->index_within_section = -1;
    struct trt_keyword_stmt rpc = pc->fp.modify.get_rpcs(tc);

    if (!trp_keyword_stmt_is_empty(rpc)) {
        trg_print_linebreak(&pc->print);
        trg_print_linebreak(&pc->print);
        trm_print_body_section(rpc, pc, tc);
    }
}

/** For notifications section: print 'notifications' keyword and all its nodes. */
static void
trm_print_notifications(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    tc->index_within_section = -1;
    struct trt_keyword_stmt notifs = pc->fp.modify.get_notifications(tc);

    if (!trp_keyword_stmt_is_empty(notifs)) {
        trg_print_linebreak(&pc->print);
        trg_print_linebreak(&pc->print);
        trm_print_body_section(notifs, pc, tc);
    }
}

/** For all grouping sections: print 'grouping' keyword, its name and all nodes. */
static void
trm_print_groupings(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    tc->index_within_section = -1;
    ly_bool once = 1;

    for (struct trt_keyword_stmt ks = pc->fp.modify.next_grouping(tc);
            !trp_keyword_stmt_is_empty(ks);
            ks = pc->fp.modify.next_grouping(tc)) {
        if (once) {
            trg_print_linebreak(&pc->print);
            trg_print_linebreak(&pc->print);
            once = 0;
        } else {
            trg_print_linebreak(&pc->print);
        }
        trm_print_body_section(ks, pc, tc);
    }
}

/** For all yang-data sections: print 'yang-data' keyword and all its nodes. */
static void
trm_print_yang_data(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    tc->index_within_section = -1;
    ly_bool once = 1;

    for (struct trt_keyword_stmt ks = pc->fp.modify.next_yang_data(tc);
            !trp_keyword_stmt_is_empty(ks);
            ks = pc->fp.modify.next_yang_data(tc)) {
        if (once) {
            trg_print_linebreak(&pc->print);
            trg_print_linebreak(&pc->print);
            once = 0;
        } else {
            trg_print_linebreak(&pc->print);
        }
        trm_print_body_section(ks, pc, tc);
    }
}

/** Print sections module, augment, rpcs, notifications, grouping, yang-data. */
static void
trm_print_sections(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    trm_print_module_section(pc, tc);
    trp_cnt_linebreak_reset(&pc->print);

    trm_print_augmentations(pc, tc);
    trp_cnt_linebreak_reset(&pc->print);

    trm_print_rpcs(pc, tc);
    trp_cnt_linebreak_reset(&pc->print);

    trm_print_notifications(pc, tc);
    trp_cnt_linebreak_reset(&pc->print);

    trm_print_groupings(pc, tc);
    trp_cnt_linebreak_reset(&pc->print);

    trm_print_yang_data(pc, tc);
    trp_cnt_linebreak_reset(&pc->print);

    trg_print_linebreak(&pc->print);
}

/**
 * @brief Get default settings for trt_printer_ctx.
 *
 * Get trt_printer_ctx containing all structure items correctly defined except for trt_printer_opts and max_line_length,
 * which are parameters of the printer tree module.
 */
static struct trt_printer_ctx
trm_default_printer_ctx(struct ly_out *out, uint32_t max_line_length)
{
    struct trt_printing printing = {
        out,
        trg_print_by_ly_print,
        0
    };

    struct trt_fp_modify_ctx modify_fp = {
        tro_modi_parent,
        tro_modi_first_sibling,
        tro_modi_next_sibling,
        tro_modi_next_child,
        tro_modi_next_augment,
        tro_modi_get_rpcs,
        tro_modi_get_notifications,
        tro_modi_next_grouping,
        tro_modi_next_yang_data
    };

    struct trt_fp_read read_fp = {
        tro_read_module_name,
        tro_read_node,
        tro_read_if_sibling_exists
    };

    struct trt_fp_print print_fp = {
        tro_print_features_names,
        tro_print_keys
    };

    return (struct trt_printer_ctx)
           {
               printing,
               {
                   modify_fp,
                   read_fp,
                   print_fp
               },
               max_line_length
           };
}

/**
 * @brief Get default settings for trt_tree_ctx.
 *
 * Pointers to current nodes will be set to module data.
 */
static struct trt_tree_ctx
trm_default_tree_ctx(const struct lys_module *module, struct trt_printer_ctx *pc)
{
    struct trt_options options = {
        0,
        &pc->print.cnt_linebreak,
    };

    return (struct trt_tree_ctx)
           {
               TRD_SECT_MODULE,
               -1,
               -1,
               module,
               module->parsed->data,
               module->parsed->data,
               options
           };
}

/******************************************************************************
 * Definition of module interface
 *****************************************************************************/

LY_ERR
tree_print_parsed_and_compiled_module(struct ly_out *out, const struct lys_module *module, uint32_t UNUSED(options), size_t line_length)
{
    line_length = line_length == 0 ? 72 : line_length;
    struct trt_printer_ctx pc = trm_default_printer_ctx(out, line_length);
    struct trt_tree_ctx tc = trm_default_tree_ctx(module, &pc);

    trm_print_sections(&pc, &tc);
    return LY_SUCCESS;
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

/******************************************************************************
 * Definitions end
 *****************************************************************************/
