/**
 * @file metadata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension plugin - YANG Metadata (annotations) (RFC 7952)
 *
 * Copyright (c) 2016-2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#include "../extensions.h"

/**
 * @brief Callback to check that the annotation can be instantiated inside the provided node
 *
 * @param[in] parent The parent of the instantiated extension.
 * @param[in] parent_type The type of the structure provided as \p parent.
 * @param[in] substmt_type libyang does not store all the extension instances in the structures where they are
 *                         instantiated in the module. In some cases (see #LYEXT_SUBSTMT) they are stored in parent
 *                         structure and marked with flag to know in which substatement of the parent the extension
 *                         was originally instantiated.
 * @return 0 - ok
 *         1 - error
 */
int annotation_position(const void * UNUSED(parent), LYEXT_PAR parent_type, LYEXT_SUBSTMT UNUSED(substmt_type))
{
    /* annotations can appear only at the top level of a YANG module or submodule */
    if (parent_type == LYEXT_PAR_MODULE) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * extension instance's content:
 * struct lys_type *type;
 * const char* dsc;
 * const char* ref;
 * const char* units;
 * struct lys_iffeature **iff;
 * uint16_t status;
 *
 * - placement in the structure is specified via offsets
 * - the order in lyext_substmt structure specified the canonical order in which the items are printed
 */
struct lyext_substmt annotation_substmt[] = {
    {LY_STMT_IFFEATURE, 4 * sizeof(void *),  LY_STMT_CARD_ANY},
    {LY_STMT_TYPE, 0, LY_STMT_CARD_MAND},
    {LY_STMT_UNITS, 3 * sizeof(void *),  LY_STMT_CARD_OPT},
    {LY_STMT_STATUS, 5 * sizeof(void *),  LY_STMT_CARD_OPT},
    {LY_STMT_DESCRIPTION, sizeof(void *),  LY_STMT_CARD_OPT},
    {LY_STMT_REFERENCE, 2 * sizeof(void *),  LY_STMT_CARD_OPT},
    {0, 0, 0} /* terminating item */
};

/**
 * @brief Plugin for the RFC 7952's annotation extension
 */
struct lyext_plugin_complex annotation = {
    .type = LYEXT_COMPLEX,
    .flags = 0,
    .check_position = &annotation_position,
    .check_result = NULL,
    .check_inherit = NULL,

    /* specification of allowed substatements of the extension instance */
    .substmt = annotation_substmt,

    /* final size of the extension instance structure with the space for storing the substatements */
    .instance_size = sizeof(struct lys_ext_instance) + sizeof(struct lys_type) +
                     3 * sizeof(char*) + sizeof(uint16_t) + sizeof(struct lys_iffeature*)
};

/**
 * @brief list of all extension plugins implemented here
 *
 * MANDATORY object for all libyang extension plugins, the name must match the <name>.so
 */
struct lyext_plugin_list metadata[] = {
    {"ietf-yang-metadata", "2016-08-05", "annotation", (struct lyext_plugin*)&annotation},
    {NULL, NULL, NULL, NULL} /* terminating item */
};
