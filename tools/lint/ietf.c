/**
 * @file ietf.c
 * @author Petr Hanzlik <Petr.Hanzlik@cesnet.cz>
 * @brief 'ietf' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "common.h"
#include "ietf.h"
#include "yl_opt.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* Global variable to track the return code of the IETF validation process. It is set to 1 if any IETF compliance issues are found during the checks. */
static int rc = 0;

#define IETF_RFC_VERSION "9907"

#define IETF_WARN(file_name, rfc, section, out, ...) \
    do { \
        ly_print(out, "%s: warning: RFC %s: %s: ", file_name, rfc, section); \
        ly_print(out, __VA_ARGS__); \
        ly_print(out, "\n"); \
    } while(0)

#define IETF_ERR(file_name, rfc, section, out, ...) \
    do { \
        ly_print(out, "%s: error: RFC %s: %s: ", file_name, rfc, section); \
        ly_print(out, __VA_ARGS__); \
        ly_print(out, "\n"); \
    } while(0)

/**
 * @brief Convert a file path to its base name.
 *
 * @param[in] filepath File path to convert.
 * @return Base name of the file, or NULL if the input is invalid.
 */
const char *
get_file_name(const char *filepath)
{
    const char *filename;

    if (!filepath) {
        return NULL;
    }

    filename = strrchr(filepath, '/');

    if (filename) {
        return filename + 1;
    }
    return filepath;
}

/**
 * @brief Checks if a given text matches a specified regular expression pattern.
 *
 * @param[in] text Target string to search within. If NULL, the function returns 0.
 * @param[in] pattern Pattern to match against.
 * @return 1 if the pattern is successfully found within the text.
 * @return 0 if the pattern is not found, if a compilation error occurs, or if invalid NULL arguments are passed.
 */
static int
check_regex(const char *text, const char *pattern)
{
    if (!text || !pattern) {
        return 0;
    }

    return ly_pattern_match(NULL, pattern, text, 0, NULL) == 0;
}

/**
 * @brief Scans a description string for the presence of strict RFC 2119/8174 requirement keywords.
 *
 * @param[in] dsc Description text to evaluate.
 * @return 1 if at least one RFC 2119 keyword is successfully found in the text.
 * @return 0 if no keywords are found, if a compilation error occurs, or if NULL is passed.
 */
static int
check_words(const char *dsc)
{
    const char *pattern;

    /* Find a string that contains at least one of the strict RFC 2119 requirement words as a standalone word. */
    pattern = ".*(^|[^a-zA-Z])(MUST|MUST NOT|REQUIRED|SHALL|SHALL NOT|SHOULD|SHOULD NOT|RECOMMENDED|NOT RECOMMENDED|MAY|OPTIONAL)([^a-zA-Z]|$).*";

    if (!dsc) {
        return 0;
    }

    return ly_pattern_match(NULL, pattern, dsc, 0, NULL) == 0;
}

/**
 * @brief Helper to check a single XPath string for banned functions and axes.
 *
 * @param[in,out] out Libyang output handler where warnings and errors are printed.
 * @param[in] xpath XPath string to check.
 * @param[in] file_name Name of the file.
 * @param[in] node_name Name of the node.
 * @param[in] stmt Statement that contains the XPath string.
 */
static void
check_xpath_string(struct ly_out *out, const char *xpath, const char *file_name, const char *node_name, const char *stmt)
{
    const char *func_pattern;
    const char *axes_pattern;

    if (!xpath) {
        return;
    }

    /* W3C Regex for banned functions */
    func_pattern = "((.|\\n)*[^a-zA-Z0-9_-])?(position|last|id|namespace-uri|name|lang|local-name)\\s*\\((.|\\n)*";
    if (check_regex(xpath, func_pattern)) {
        IETF_WARN(file_name, IETF_RFC_VERSION, "4.6.2", out, "XPath function in \"%s\" statement on \"%s\" should not be used", stmt, node_name);
        rc = 1;
    }

    /* W3C Regex for banned axes */
    axes_pattern = "((.|\\n)*[^a-zA-Z0-9_-])?(preceding-sibling|following-sibling|preceding|following)\\s*::(.|\\n)*";
    if (check_regex(xpath, axes_pattern)) {
        IETF_WARN(file_name, IETF_RFC_VERSION, "4.6.3", out, "XPath axis in \"%s\" statement on \"%s\" should not be used", stmt, node_name);
        rc = 1;
    }
}

/**
 * @brief Helper to extract and check XPath strings from a node's 'when' and 'must' structures.
 *
 * @param[in,out] out Libyang output handler where warnings and errors are printed.
 * @param[in] when Pointer to the structure containing the 'when' condition.
 * @param[in] musts Array of structures containing 'must' conditions.
 * @param[in] file_name Name of the file being checked.
 * @param[in] node_name Name of the node being checked.
 */
static void
check_node_xpath(struct ly_out *out, const struct lysp_when *when, const struct lysp_restr *musts, const char *file_name, const char *node_name)
{
    LY_ARRAY_COUNT_TYPE i;

    /* Check 'when' condition */
    if (when && when->cond) {
        check_xpath_string(out, when->cond, file_name, node_name, "when");
    }

    /* Check all 'must' conditions */
    if (musts) {
        LY_ARRAY_FOR(musts, i) {
            if (musts[i].arg.str) {
                check_xpath_string(out, musts[i].arg.str, file_name, node_name, "must");
            }
        }
    }
}

/**
 * @brief Collapses all consecutive whitespace (newlines, tabs, spaces) into a single space.
 * @param[in] input Input string.
 * @return Normalized string.
 */
static char *
normalize_whitespace(const char *input)
{
    size_t i, j = 0, len;
    int in_whitespace = 0;

    if (!input) {
        return NULL;
    }

    len = strlen(input);
    char *normalized = malloc(len + 1);

    if (!normalized) {
        return NULL;
    }

    for (i = 0; i < len; i++) {
        if (isspace((unsigned char)input[i])) {
            if (!in_whitespace) {
                normalized[j++] = ' ';
                in_whitespace = 1;
            }
        } else {
            normalized[j++] = input[i];
            in_whitespace = 0;
        }
    }
    normalized[j] = '\0';
    return normalized;
}

/**
 * @brief Traverses the compiled schema tree to enforce IETF-specific validation rules.
 *
 * @param[in,out] out Libyang output handler where warnings and errors are printed.
 * @param[in] data First top-level node of the compiled data tree to be inspected.
 * @param[in] file_name Name of the file being evaluated (used to prefix the error messages).
 */
static void
check_nodes_ietf(struct ly_out *out, const struct lysc_node *data, const char *file_name)
{
    const struct lysc_node *root, *elem;
    char node_path[256];
    int is_mandatory;

    LY_LIST_FOR(data, root) {
        is_mandatory = 0;

        if ((root->flags & LYS_MAND_TRUE) && !(root->flags & LYS_CONFIG_R)) {
            is_mandatory = 1;
        }

        if (is_mandatory) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.10", out, "top-level node \"%s\" must not be mandatory", root->name);
            rc = 1;
        }

        LYSC_TREE_DFS_BEGIN(root, elem) {
            lysc_path(elem, LYSC_PATH_LOG, node_path, sizeof(node_path));

            if (elem->nodetype == LYS_CONTAINER) {
                struct lysc_node_container *cont = (struct lysc_node_container *)elem;

                if (cont->actions) {
                    check_nodes_ietf(out, (const struct lysc_node *)cont->actions, file_name);
                }
                if (cont->notifs) {
                    check_nodes_ietf(out, (const struct lysc_node *)cont->notifs, file_name);
                }
            } else if (elem->nodetype == LYS_LIST) {
                struct lysc_node_list *list = (struct lysc_node_list *)elem;

                if (list->actions) {
                    check_nodes_ietf(out, (const struct lysc_node *)list->actions, file_name);
                }
                if (list->notifs) {
                    check_nodes_ietf(out, (const struct lysc_node *)list->notifs, file_name);
                }
            }

            switch (elem->nodetype) {
            case LYS_CONTAINER:
            case LYS_LIST:
            case LYS_LEAF:
            case LYS_ANYXML:
            case LYS_ANYDATA:
            case LYS_LEAFLIST:
            case LYS_CHOICE:
            case LYS_RPC:
                if (!elem->dsc) {
                    IETF_ERR(file_name, IETF_RFC_VERSION, "4.14", out, "statement \"%s\" must have a \"description\" substatement", lys_nodetype2str(elem->nodetype));
                    rc = 1;
                }
                break;
            case LYS_NOTIF:
                if (!elem->dsc) {
                    IETF_ERR(file_name, IETF_RFC_VERSION, "4.14,4.16", out, "statement \"%s\" must have a \"description\" substatement", lys_nodetype2str(elem->nodetype));
                    rc = 1;
                }
                break;
            default:
                break;
            }

            LYSC_TREE_DFS_END(root, elem);
        }
    }
}

/**
 * @brief Helper function to validate locally scoped typedefs and groupings inside parsed data nodes.
 *
 * @param[in,out] out Libyang output handler for printing errors.
 * @param[in] file_name Name of the parsed file for error prefixes.
 * @param[in] typedefs Array of parsed typedefs to check.
 * @param[in] groupings Linked list of parsed groupings to check.
 * @param[in] node_type_str String literal representing the node type (e.g., "container", "list").
 * @param[in] node_name String name of the parent node to provide context.
 * @param[in,out] found_2119 Pointer to an integer flag that tracks if RFC 2119 keywords are used.
 */
static void
check_inner_defs(struct ly_out *out, const char *file_name, struct lysp_tpdf *typedefs, struct lysp_node_grp *groupings, const char *node_type_str, const char *node_name, int *found_2119)
{
    LY_ARRAY_COUNT_TYPE i;
    struct lysp_node_grp *grp;

    LY_ARRAY_FOR(typedefs, i) {
        if (strlen(typedefs[i].name) > 64) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", typedefs[i].name);
            rc = 1;
        }

        if (!typedefs[i].dsc) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.13,4.14", out,
                    "statement \"typedef\" inside %s \"%s\" must have a \"description\" substatement",
                    node_type_str, node_name);
            rc = 1;
        } else if (check_words(typedefs[i].dsc)) {
            *found_2119 = 1;
        }
    }

    LY_LIST_FOR(groupings, grp) {
        if (strlen(grp->name) > 64) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", grp->name);
            rc = 1;
        }

        if (!grp->dsc) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.14", out,
                    "statement \"grouping\" inside %s \"%s\" must have a \"description\" substatement",
                    node_type_str, node_name);
            rc = 1;
        } else if (check_words(grp->dsc)) {
            *found_2119 = 1;
        }
    }
}

/**
 * @brief Recursively traverses and validates the parsed schema tree for IETF compliance.
 *
 * @param[in,out] out Libyang output handler where warnings and errors are printed.
 * @param[in] node Starting node of the parsed data tree (or sibling list) to inspect.
 * @param[in] file_name Name of the file being evaluated, used to prefix the log messages.
 * @param[in,out] found_2119 Pointer to an integer flag used to record whether any node in this tree uses RFC 2119 keywords.
 */
static void
check_parsed_tree_ietf(struct ly_out *out, const struct lysp_node *node, const char *file_name, int *found_2119)
{
    const struct lysp_node *elem;
    LY_ARRAY_COUNT_TYPE i;

    LY_LIST_FOR(node, elem) {
        if (strlen(elem->name) > 64) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", elem->name);
            rc = 1;
        }

        if (elem->dsc && check_words(elem->dsc)) {
            *found_2119 = 1;
        }

        if (elem->flags & LYS_CONFIG_W) {
            IETF_WARN(file_name, IETF_RFC_VERSION, "4.4", out, "statement \"config\" is given with its default value \"true\"");
            rc = 1;
        }

        if (elem->flags & LYS_STATUS_CURR) {
            IETF_WARN(file_name, IETF_RFC_VERSION, "4.4", out, "statement \"status\" is given with its default value \"current\"");
            rc = 1;
        }

        if (elem->flags & LYS_MAND_FALSE) {
            IETF_WARN(file_name, IETF_RFC_VERSION, "4.4", out, "statement \"mandatory\" is given with its default value \"false\"");
            rc = 1;
        }

        if (elem->flags & LYS_ORDBY_SYSTEM) {
            IETF_WARN(file_name, IETF_RFC_VERSION, "4.4", out, "statement \"ordered-by\" is given with its default value \"system\"");
            rc = 1;
        }

        switch (elem->nodetype) {
        case LYS_CONTAINER: {
            const struct lysp_node_container *cont = (struct lysp_node_container *)elem;

            if ((elem->flags & LYS_CONFIG_R) && cont->musts) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.5", out, "constraints ('must' statements) on state data SHOULD be avoided (leaf \"%s\")", cont->name);
                rc = 1;
            }

            check_node_xpath(out, cont->when, cont->musts, file_name, elem->name);
            check_inner_defs(out, file_name, cont->typedefs, cont->groupings, "container", elem->name, found_2119);

            if (cont->child) {
                check_parsed_tree_ietf(out, cont->child, file_name, found_2119);
            }
            if (cont->actions) {
                check_parsed_tree_ietf(out, (const struct lysp_node *)cont->actions, file_name, found_2119);
            }
            if (cont->notifs) {
                check_parsed_tree_ietf(out, (const struct lysp_node *)cont->notifs, file_name, found_2119);
            }
            break;
        }
        case LYS_LIST: {
            const struct lysp_node_list *list = (struct lysp_node_list *)elem;

            check_node_xpath(out, list->when, list->musts, file_name, elem->name);
            check_inner_defs(out, file_name, list->typedefs, list->groupings, "list", elem->name, found_2119);

            if ((elem->flags & LYS_SET_MIN) && (list->min == 0)) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.4", out, "statement \"min-elements\" is given with its default value \"0\"");
                rc = 1;
            }

            if ((elem->flags & LYS_SET_MAX) && (list->max == 0)) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.4", out, "statement \"max-elements\" is given with its default value \"unbounded\"");
                rc = 1;
            }

            if ((elem->flags & LYS_CONFIG_R) && list->musts) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.5", out, "constraints ('must' statements) on state data should be avoided (leaf \"%s\")", list->name);
                rc = 1;
            }

            if (list->child) {
                check_parsed_tree_ietf(out, list->child, file_name, found_2119);
            }
            if (list->actions) {
                check_parsed_tree_ietf(out, (const struct lysp_node *)list->actions, file_name, found_2119);
            }
            if (list->notifs) {
                check_parsed_tree_ietf(out, (const struct lysp_node *)list->notifs, file_name, found_2119);
            }
            break;
        }
        case LYS_LEAFLIST: {
            const struct lysp_node_leaflist *llist = (struct lysp_node_leaflist *)elem;

            check_node_xpath(out, llist->when, llist->musts, file_name, elem->name);

            if (!strcmp(llist->type.name, "empty")) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.11.5", out, "the type \"empty\" should not be used for a leaf-list (\"%s\")", llist->name);
                rc = 1;
            }

            if ((elem->flags & LYS_CONFIG_R) && llist->musts) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.5", out, "constraints ('must' statements) on state data should be avoided (leaf \"%s\")", llist->name);
                rc = 1;
            }

            if ((elem->flags & LYS_SET_MIN) && (llist->min == 0)) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.4", out, "statement \"min-elements\" is given with its default value \"0\"");
                rc = 1;
            }

            if ((elem->flags & LYS_SET_MAX) && (llist->max == 0)) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.4", out, "statement \"max-elements\" is given with its default value \"unbounded\"");
                rc = 1;
            }

            if (llist->type.enums) {
                LY_ARRAY_FOR(llist->type.enums, i) {
                    if (strlen(llist->type.enums[i].name) > 64) {
                        IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", llist->type.enums[i].name);
                        rc = 1;
                    }

                    if (!llist->type.enums[i].dsc) {
                        IETF_WARN(file_name, IETF_RFC_VERSION, "4.11.3", out, "statement \"enum\" should have a \"description\" substatement");
                        rc = 1;
                    }
                }
            }
            if (llist->type.bits) {
                LY_ARRAY_FOR(llist->type.bits, i) {
                    if (strlen(llist->type.bits[i].name) > 64) {
                        IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", llist->type.bits[i].name);
                        rc = 1;
                    }

                    if (!llist->type.bits[i].dsc) {
                        IETF_WARN(file_name, IETF_RFC_VERSION, "4.11.3", out, "statement \"bit\" should have a \"description\" substatement");
                        rc = 1;
                    }
                }
            }
            break;
        }
        case LYS_GROUPING: {
            const struct lysp_node_grp *grp = (struct lysp_node_grp *)elem;

            check_inner_defs(out, file_name, grp->typedefs, grp->groupings, "grouping", elem->name, found_2119);
            if (grp->child) {
                check_parsed_tree_ietf(out, grp->child, file_name, found_2119);
            }
            break;
        }
        case LYS_RPC:
        case LYS_ACTION: {
            const struct lysp_node_action *act = (struct lysp_node_action *)elem;
            const char *type_str = (elem->nodetype == LYS_RPC) ? "rpc" : "action";

            check_inner_defs(out, file_name, act->input.typedefs, act->input.groupings, type_str, elem->name, found_2119);

            if (act->input.typedefs || act->input.groupings) {
                check_inner_defs(out, file_name, act->input.typedefs, act->input.groupings, "input of", elem->name, found_2119);
            }
            if (act->input.child) {
                check_parsed_tree_ietf(out, act->input.child, file_name, found_2119);
            }

            if (act->output.typedefs || act->output.groupings) {
                check_inner_defs(out, file_name, act->output.typedefs, act->output.groupings, "output of", elem->name, found_2119);
            }
            if (act->output.child) {
                check_parsed_tree_ietf(out, act->output.child, file_name, found_2119);
            }
            break;
        }
        case LYS_NOTIF: {
            const struct lysp_node_notif *notif = (struct lysp_node_notif *)elem;

            check_inner_defs(out, file_name, notif->typedefs, notif->groupings, "notification", elem->name, found_2119);
            if (notif->child) {
                check_parsed_tree_ietf(out, notif->child, file_name, found_2119);
            }
            break;
        }
        case LYS_CHOICE: {
            const struct lysp_node_choice *choice = (struct lysp_node_choice *)elem;

            check_node_xpath(out, choice->when, NULL, file_name, elem->name);

            if (choice->child) {
                check_parsed_tree_ietf(out, choice->child, file_name, found_2119);
            }
            break;
        }
        case LYS_CASE: {
            const struct lysp_node_case *cas = (struct lysp_node_case *)elem;

            check_node_xpath(out, cas->when, NULL, file_name, elem->name);

            if (cas->child) {
                check_parsed_tree_ietf(out, cas->child, file_name, found_2119);
            }
            break;
        }
        case LYS_LEAF: {
            const struct lysp_node_leaf *leaf = (struct lysp_node_leaf *)elem;
            const struct lysp_node_list *parent_list = (struct lysp_node_list *)elem->parent;
            const struct lysp_node *parent_node = elem->parent;
            LY_ARRAY_COUNT_TYPE leaf_iff_count;
            LY_ARRAY_COUNT_TYPE list_iff_count;
            int is_key = 0;

            check_node_xpath(out, leaf->when, leaf->musts, file_name, elem->name);

            if (elem->parent && (elem->parent->nodetype == LYS_LIST)) {
                if (parent_list->key) {
                    if (!strcmp(parent_list->key, elem->name)) {
                        is_key = 1;
                    }
                }
            }

            if ((elem->flags & LYS_CONFIG_R) && leaf->musts) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.5", out, "constraints ('must' statements) on state data should be avoided (leaf \"%s\")", leaf->name);
                rc = 1;
            }

            if (is_key) {
                if (leaf->when) {
                    IETF_WARN(file_name, IETF_RFC_VERSION, "4.5", out, "key leaf \"%s\" should not have a \"when\" statement", leaf->name);
                    rc = 1;
                }

                leaf_iff_count = elem->iffeatures ? LY_ARRAY_COUNT(elem->iffeatures) : 0;
                list_iff_count = parent_node->iffeatures ? LY_ARRAY_COUNT(parent_node->iffeatures) : 0;

                if (leaf_iff_count != list_iff_count) {
                    IETF_ERR(file_name, IETF_RFC_VERSION, "4.5", out, "key leaf \"%s\" must have the exact same \"if-feature\" statements as its parent list \"%s\"", leaf->name, parent_node->name);
                    rc = 1;
                } else {
                    for (LY_ARRAY_COUNT_TYPE i = 0; i < list_iff_count; i++) {
                        if (strcmp(elem->iffeatures[i].str, parent_node->iffeatures[i].str)) {
                            IETF_ERR(file_name, IETF_RFC_VERSION, "4.5", out, "key leaf \"%s\" must have the exact same \"if-feature\" statements as its parent list \"%s\"", leaf->name, parent_node->name);
                            rc = 1;
                            break;
                        }
                    }
                }

                if (!strcmp(leaf->type.name, "empty")) {
                    IETF_WARN(file_name, IETF_RFC_VERSION, "4.11.5", out, "the type \"empty\" should not be used for a key leaf \"%s\"", leaf->name);
                    rc = 1;
                }
            }

            if (!strcmp(leaf->type.name, "empty")) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.11.5", out, "in \"%s\", the \"boolean\" data type should be used instead of the \"empty\" data type", leaf->name);
            }

            if (leaf->type.enums) {
                LY_ARRAY_FOR(leaf->type.enums, i) {
                    if (strlen(leaf->type.enums[i].name) > 64) {
                        IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", leaf->type.enums[i].name);
                        rc = 1;
                    }

                    if (!leaf->type.enums[i].dsc) {
                        IETF_WARN(file_name, IETF_RFC_VERSION, "4.11.3", out, "statement \"enum\" should have a \"description\" substatement");
                        rc = 1;
                    }
                }
            }
            if (leaf->type.bits) {
                LY_ARRAY_FOR(leaf->type.bits, i) {
                    if (strlen(leaf->type.bits[i].name) > 64) {
                        IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", leaf->type.bits[i].name);
                        rc = 1;
                    }

                    if (!leaf->type.bits[i].dsc) {
                        IETF_WARN(file_name, IETF_RFC_VERSION, "4.11.3", out, "statement \"bit\" should have a \"description\" substatement");
                        rc = 1;
                    }
                }
            }
            break;
        }
        case LYS_USES: {
            const struct lysp_node_uses *uses = (struct lysp_node_uses *)elem;
            const struct lysp_node_augment *aug;

            LY_LIST_FOR(uses->augments, aug) {
                if (!aug->dsc) {
                    IETF_ERR(file_name, IETF_RFC_VERSION, "4.14", out, "statement \"augment\" inside uses \"%s\" must have a \"description\" substatement", uses->name);
                    rc = 1;
                }

                if (aug->child) {
                    check_parsed_tree_ietf(out, aug->child, file_name, found_2119);
                }
            }
            break;
        }
        case LYS_ANYXML:
        case LYS_ANYDATA: {
            const struct lysp_node_anydata *anydata = (struct lysp_node_anydata *)elem;

            check_node_xpath(out, anydata->when, anydata->musts, file_name, elem->name);
            break;
        }

        default:
            break;
        }
    }
}

/**
 * @brief Validates IETF-mandated boilerplate and top-level descriptions for a parsed module or submodule.
 *
 * @param[in] name Name of the module or submodule.
 * @param[in] filepath File path of the module (used to prefix error messages).
 * @param[in] dsc Top-level `description` text of the module/submodule.
 * @param[in] contact Top-level `contact` text.
 * @param[in] org Top-level `organization` text.
 * @param[in] revs Pointer to the parsed `revision` array.
 * @param[in] type Integer flag indicating whether the file is a module (0) or submodule (non-zero).
 * @param[in] exts Array of top-level parsed `extension` statements.
 * @param[in] feats Array of top-level parsed `feature` statements.
 * @param[in] idents Array of top-level parsed `identity` statements.
 * @param[in] augments Linked list of top-level parsed `augment` statements.
 * @param[in] tpdfs Array of top-level parsed `typedef` statements.
 * @param[in] grps Linked list of top-level parsed `grouping` statements.
 * @param[in,out] out Libyang output handler where warnings and errors are printed.
 * @param[in,out] found_2119 Pointer to a flag tracking if RFC 2119 keywords were found anywhere in this specific file.
 */
static void
check_parsed_boilerplate(const char *name, const char *filepath, const char *dsc, const char *contact, const char *org, struct lysp_revision *revs, int type, struct lysp_ext *exts,
        struct lysp_feature *feats, struct lysp_ident *idents, struct lysp_node_augment *augments, struct lysp_tpdf *tpdfs, struct lysp_node_grp *grps, struct ly_out *out, int *found_2119)
{
    const char *file_name = filepath ? get_file_name(filepath) : name;
    const char *type_name = type ? "submodule" : "module";
    const char *rfc8174_target, *tlp, *rfc_start, *tlp_part2;
    char *norm_dsc, *endptr;
    struct lysp_node_grp *grp;
    struct lysp_node_augment *aug;
    LY_ARRAY_COUNT_TYPE i;
    int rfc_valid = 0, tlp_valid = 0, is_simplified;
    long year;

    if (strncmp(name, "ietf-", 5) && strncmp(name, "iana-", 5)) {
        IETF_WARN(file_name, IETF_RFC_VERSION, "4.1", out, "the module name should start with one of the strings \"ietf-\" or \"iana-\"");
        rc = 1;
    }

    if (strlen(name) > 64) {
        IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", name);
        rc = 1;
    }

    if (!contact) {
        IETF_ERR(file_name, IETF_RFC_VERSION, "4.8", out, "statement \"%s\" must have a \"contact\" substatement", type_name);
        rc = 1;
    }

    if (!org) {
        IETF_ERR(file_name, IETF_RFC_VERSION, "4.8", out, "statement \"%s\" must have a \"organization\" substatement", type_name);
        rc = 1;
    }

    if (!revs) {
        IETF_ERR(file_name, IETF_RFC_VERSION, "4.8", out, "statement \"%s\" must have a \"revision\" substatement", type_name);
        rc = 1;
    } else {
        LY_ARRAY_FOR(revs, i) {
            if (!revs[i].ref) {
                IETF_ERR(file_name, IETF_RFC_VERSION, "4.8", out, "statement \"revision\" %s must have a \"reference\" substatement", revs[i].date);
                rc = 1;
            }
            if (!revs[i].dsc) {
                IETF_WARN(file_name, IETF_RFC_VERSION, "4.8", out, "statement \"revision\" %s should have a \"description\" substatement summarizing changes", revs[i].date);
                rc = 1;
            }
        }
    }

    if (!dsc) {
        IETF_ERR(file_name, IETF_RFC_VERSION, "4.8", out, "statement \"%s\" must have a \"description\" substatement", type_name);
        rc = 1;
        return;
    }

    if (check_words(dsc)) {
        *found_2119 = 1;
    }

    // Normalize whitespace for strict boilerplate matching
    norm_dsc = normalize_whitespace(dsc);
    if (!norm_dsc) {
        ly_print(out, "%s: error: Memory allocation failed during description normalization.\n", file_name);
        rc = 1;
        return;
    }

    tlp = strstr(norm_dsc, "Copyright (c) ");
    if (tlp) {
        tlp += 14; // Skip past "Copyright (c) "

        year = strtol(tlp, &endptr, 10);

        if (tlp != endptr) {
            tlp = endptr; // Move past the year
            tlp_part2 =
                    " IETF Trust and the persons identified as authors of the code. All rights reserved. Redistribution and use in source and binary forms, with or without modification, is permitted pursuant to, and subject to the license terms contained in, the ";

            if (strncmp(tlp, tlp_part2, strlen(tlp_part2)) == 0) {
                tlp += strlen(tlp_part2);
                is_simplified = 0;

                if (strncmp(tlp, "Revised", 7) == 0) {
                    tlp += 7;
                } else if (strncmp(tlp, "Simplified", 10) == 0) {
                    is_simplified = 1;
                    tlp += 10;
                }

                // If we found Revised or Simplified, check the rest of the string
                if (strncmp(tlp, " BSD License set forth in Section 4.c of the IETF Trust's Legal Provisions Relating to IETF Documents (http", 107) == 0) {
                    tlp += 107;
                    if (*tlp == 's') {
                        tlp++;              // allow https or http

                    }
                    if (strncmp(tlp, "://trustee.ietf.org/license-info).", 34) == 0) {
                        tlp_valid = 1;
                        // Throw error if they used "Simplified" in 2022 or later
                        if ((year >= 2022) && is_simplified) {
                            IETF_WARN(file_name, IETF_RFC_VERSION, "3.1", out, "The IETF Trust Copyright statement uses 'Simplified' instead of 'Revised' BSD License for a module >= 2022");
                            rc = 1;
                        }
                    }
                }
            }
        }
    }

    if (!tlp_valid) {
        IETF_WARN(file_name, IETF_RFC_VERSION, "3.1", out, "The IETF Trust Copyright statement seems to be missing or is not correct");
        rc = 1;
    }

    if (!strncmp(name, "ietf-", 5)) {
        rfc_start = strstr(norm_dsc, "This version of this YANG module is part of RFC ");
        if (!rfc_start) {
            rfc_start = strstr(norm_dsc, "This version of this YANG submodule is part of RFC ");
        }

        if (rfc_start) {
            // As long as the ending declaration exists anywhere after the start, it's valid
            if (strstr(rfc_start, "; see the RFC itself for full legal notices.")) {
                rfc_valid = 1;
            }
        }

        if (!rfc_valid) {
            IETF_WARN(file_name, IETF_RFC_VERSION, "Appendix B", out, "The text about which RFC this module is part of seems to be missing or is not correct");
            rc = 1;
        }
    }

    LY_ARRAY_FOR(exts, i) {
        if (exts[i].flags & LYS_YINELEM_FALSE) {
            IETF_WARN(file_name, IETF_RFC_VERSION, "4.4", out, "statement \"yin-element\" is given with its default value \"false\"");
            rc = 1;
        }

        if (!exts[i].dsc) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.14", out, "statement \"%s\" must have a \"description\" substatement", "extension");
            rc = 1;
        } else if (check_words(exts[i].dsc)) {
            *found_2119 = 1;
        }
    }

    LY_ARRAY_FOR(feats, i) {
        if (strlen(feats[i].name) > 64) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", feats[i].name);
            rc = 1;
        }

        if (!feats[i].dsc) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.14", out, "statement \"%s\" must have a \"description\" substatement", "feature");
            rc = 1;
        } else if (check_words(feats[i].dsc)) {
            *found_2119 = 1;
        }
    }

    LY_ARRAY_FOR(idents, i) {
        if (strlen(idents[i].name) > 64) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", idents[i].name);
            rc = 1;
        }

        if (!idents[i].dsc) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.14", out, "statement \"%s\" must have a \"description\" substatement", "identity");
            rc = 1;
        } else if (check_words(idents[i].dsc)) {
            *found_2119 = 1;
        }
    }

    LY_LIST_FOR(augments, aug) {
        if (!aug->dsc) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.14", out, "statement \"augment\" must have a \"description\" substatement");
            rc = 1;
        } else if (check_words(aug->dsc)) {
            *found_2119 = 1;
        }

        if (aug->child) {
            check_parsed_tree_ietf(out, aug->child, file_name, found_2119);
        }
    }

    LY_ARRAY_FOR(tpdfs, i) {
        if (strlen(tpdfs[i].name) > 64) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", tpdfs[i].name);
            rc = 1;
        }

        if (!tpdfs[i].dsc) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.13,4.14", out, "statement \"typedef\" must have a \"description\" substatement");
            rc = 1;
        } else if (check_words(tpdfs[i].dsc)) {
            *found_2119 = 1;
        }
    }

    LY_LIST_FOR(grps, grp) {
        if (strlen(grp->name) > 64) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.3", out, "identifier %s exceeds 64 characters", grp->name);
            rc = 1;
        }

        if (!grp->dsc) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.14", out, "statement \"grouping\" must have a \"description\" substatement");
            rc = 1;
        } else if (check_words(grp->dsc)) {
            *found_2119 = 1;
        }
    }

    if (*found_2119) {
        rfc8174_target =
                "The key words 'MUST', 'MUST NOT', 'REQUIRED', 'SHALL', 'SHALL NOT', 'SHOULD', 'SHOULD NOT', 'RECOMMENDED', 'NOT RECOMMENDED', 'MAY', and 'OPTIONAL' in this document are to be interpreted as described in BCP 14 (RFC 2119) (RFC 8174) when, and only when, they appear in all capitals, as shown here.";

        if (!strstr(norm_dsc, rfc8174_target)) {
            ly_print(out, "%s: warning: %s\n", file_name, "the module seems to use RFC 2119 keywords, but the required text from RFC 8174 is not found or is not correct");
            rc = 1;
        }
    }

    // Free the normalized string to prevent memory leaks
    free(norm_dsc);
}

/**
 * @brief Orchestrator for IETF RFC 9907 compliance validation on a YANG module.

 * @param[in] mod Libyang module structure containing both the parsed schema
 * @param[in,out] out Libyang output handler where compliance warnings and errors are printed.
 */
static void
check_ietf(const struct lys_module *mod, struct ly_out *out)
{
    const char *file_name = mod->filepath ? get_file_name(mod->filepath) : mod->name;
    const char *sub_file_name;
    char ns[256];
    LY_ARRAY_COUNT_TYPE i;
    int main_found_2119 = 0;
    struct lysp_submodule *sub;

    if (mod->parsed->data) {
        check_parsed_tree_ietf(out, mod->parsed->data, file_name, &main_found_2119);
    }
    if (mod->parsed->rpcs) {
        check_parsed_tree_ietf(out, (struct lysp_node *)mod->parsed->rpcs, file_name, &main_found_2119);
    }
    if (mod->parsed->notifs) {
        check_parsed_tree_ietf(out, (struct lysp_node *)mod->parsed->notifs, file_name, &main_found_2119);
    }

    check_parsed_boilerplate(mod->name, mod->filepath, mod->dsc, mod->contact, mod->org, mod->parsed->revs, mod->parsed->is_submod, mod->parsed->extensions, mod->parsed->features, mod->parsed->identities, mod->parsed->augments, mod->parsed->typedefs,
            mod->parsed->groupings, out, &main_found_2119);

    snprintf(ns, sizeof(ns), "urn:ietf:params:xml:ns:yang:%s", mod->name);
    if (strcmp(ns, mod->ns)) {
        IETF_WARN(file_name, IETF_RFC_VERSION, "4.9", out, "namespace value should be \"%s\"", ns);
        rc = 1;
    }

    if (mod->parsed->imports) {
        LY_ARRAY_FOR(mod->parsed->imports, i) {
            if (!strncmp(mod->parsed->imports[i].name, "ietf-", 5) || !strncmp(mod->parsed->imports[i].name, "iana-", 5)) {
                if (!mod->parsed->imports[i].ref) {
                    IETF_WARN(file_name, IETF_RFC_VERSION, "4.7", out, "statement \"import\" for stable module \"%s\" should have a \"reference\" substatement", mod->parsed->imports[i].name);
                    rc = 1;
                }
            }
        }
    }

    if (mod->parsed->deviations) {
        if (!strncmp(mod->name, "ietf-", 5) || !strncmp(mod->name, "iana-", 5)) {
            IETF_ERR(file_name, IETF_RFC_VERSION, "4.20", out, "the YANG \"deviation\" statement is not allowed to appear in IETF YANG modules");
            rc = 1;
        }
    }

    LY_ARRAY_FOR(mod->parsed->includes, i) {
        sub = mod->parsed->includes[i].submodule;
        sub_file_name = sub->filepath ? get_file_name(sub->filepath) : sub->name;
        int sub_found_2119 = 0;

        if (mod->parsed->revs && sub->revs) {
            if (strcmp(mod->parsed->revs[0].date, sub->revs[0].date) < 0) {
                IETF_ERR(file_name, IETF_RFC_VERSION, "4.7", out, "the module's revision %s is older than submodule %s's revision %s", mod->parsed->revs[0].date, sub->name, sub->revs[0].date);
                rc = 1;
            }
        }

        if (sub->data) {
            check_parsed_tree_ietf(out, sub->data, sub_file_name, &sub_found_2119);
        }
        if (sub->rpcs) {
            check_parsed_tree_ietf(out, (struct lysp_node *)sub->rpcs, sub_file_name, &sub_found_2119);
        }
        if (sub->notifs) {
            check_parsed_tree_ietf(out, (struct lysp_node *)sub->notifs, sub_file_name, &sub_found_2119);
        }

        check_parsed_boilerplate(sub->name, sub->filepath, sub->dsc, sub->contact, sub->org, sub->revs, sub->is_submod, sub->extensions, sub->features, sub->identities, sub->augments, sub->typedefs, sub->groupings, out, &sub_found_2119);
    }

    if (mod->compiled->data) {
        check_nodes_ietf(out, mod->compiled->data, file_name);
    }

    if (mod->compiled->rpcs) {
        check_nodes_ietf(out, (const struct lysc_node *)mod->compiled->rpcs, file_name);
    }

    if (mod->compiled->notifs) {
        check_nodes_ietf(out, (const struct lysc_node *)mod->compiled->notifs, file_name);
    }
}

int
yl_validate_ietf(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    const struct lys_module *mod;
    char *revision, *name;
    uint8_t out_alloc = 0;

    name = strdup(posv);
    if (!name) {
        YLMSG_E("Memory allocation failed.");
        return 1;
    }
    revision = strchr(name, '@');
    if (revision) {
        revision[0] = '\0';
        ++revision;
    }

    mod = revision ?
            ly_ctx_get_module(*ctx, name, revision) :
            ly_ctx_get_module_latest(*ctx, name);

    if (!mod || !mod->compiled || !mod->parsed) {
        YLMSG_E("Error: Module %s not loaded or failed to compile.", name);
        free(name);
        return 1;
    }

    if (!yo->out) {
        if (ly_out_new_file(stdout, &yo->out)) {
            YLMSG_E("Unable to allocate output handler.");
            free(name);
            return 1;
        }
        out_alloc = 1;
    }

    check_ietf(mod, yo->out);

    if (out_alloc) {
        ly_out_free(yo->out, NULL, 0);
        yo->out = NULL;
    }

    free(name);
    return rc;
}
