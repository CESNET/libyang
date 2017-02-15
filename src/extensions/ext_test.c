/**
 * @file metadata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension test plugin - special plugin for internal cmocka tests of extensions implementation.
 * The extension definition can be found in libyang/tests/schema/yang/files/ext-def.yang
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
int libyang_ext_test_position(const void * UNUSED(parent), LYEXT_PAR UNUSED(parent_type),
                              LYEXT_SUBSTMT UNUSED(substmt_type))
{
    /* allow extension instance anywhere */
    return 0;
}

struct lyext_substmt libyang_ext_test_substmt[] = {
    {LY_STMT_ARGUMENT,      0,                       LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_BASE,          1 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_BELONGSTO,     2 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char*[2] */
    {LY_STMT_CONTACT,       4 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_DEFAULT,       5 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_DESCRIPTION,   6 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_ERRTAG,        7 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_ERRMSG,        8 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_KEY,           9 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_NAMESPACE,    10 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_ORGANIZATION, 11 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_PATH,         12 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_PREFIX,       13 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_PRESENCE,     14 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_REFERENCE,    15 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_REVISIONDATE, 16 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_UNITS,        17 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_VALUE,        18 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_VERSION,      19 * sizeof(const char*), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_MODIFIER,     20 * sizeof(const char*), LY_STMT_CARD_OPT}, /* uint8_t */
    {LY_STMT_REQINSTANCE,  20 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* uint8_t */
    {LY_STMT_YINELEM,      20 * sizeof(const char*) + 2 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* uint8_t */
    {LY_STMT_CONFIG,       20 * sizeof(const char*) + 3 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared uint16_t */
    {LY_STMT_MANDATORY,    20 * sizeof(const char*) + 3 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared uint16_t */
    {LY_STMT_ORDEREDBY,    20 * sizeof(const char*) + 3 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared uint16_t */
    {LY_STMT_STATUS,       20 * sizeof(const char*) + 3 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared uint16_t */
    {LY_STMT_DIGITS,       20 * sizeof(const char*) + 3 * sizeof(uint8_t) + sizeof(uint16_t), LY_STMT_CARD_OPT}, /* uint8_t */
    {LY_STMT_MAX,          20 * sizeof(const char*) + 4 * sizeof(uint8_t) + sizeof(uint16_t), LY_STMT_CARD_OPT}, /* uint32_t* */
    {LY_STMT_MIN,          20 * sizeof(const char*) + 4 * sizeof(uint8_t) +
                           sizeof(uint16_t) + 1 * sizeof(uint32_t*), LY_STMT_CARD_OPT}, /* uint32_t* */
    {LY_STMT_POSITION,     20 * sizeof(const char*) + 4 * sizeof(uint8_t) +
                           sizeof(uint16_t) + 2 * sizeof(uint32_t*), LY_STMT_CARD_OPT}, /* uint32_t* */
/* compress the offset calculation */
    {LY_STMT_UNIQUE,       23 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_unique* */
    {LY_STMT_MODULE,       24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_module* */
    {LY_STMT_ACTION,       25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_ANYDATA,      25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_ANYXML,       25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_CASE,         25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_CHOICE,       25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_CONTAINER,    25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_GROUPING,     25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_INPUT,        25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_LEAF,         25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_LEAFLIST,     25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_LIST,         25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_NOTIFICATION, 25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_OUTPUT,       25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_USES,         25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
/*    {LY_STMT_TYPEDEF,      47 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_TYPE,         48 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_BIT,          49 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_ENUM,         40 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_REFINE,       51 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_AUGMENT,      52 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_DEVIATE,      53 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_DEVIATION,    54 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_EXTENSION,    55 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_FEATURE,      56 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_IDENTITY,     57 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_IFFEATURE,    58 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_IMPORT,       59 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_INCLUDE,      60 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_LENGTH,       61 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_MUST,         62 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_PATTERN,      63 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_RANGE,        64 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_WHEN,         65 * sizeof(void *), LY_STMT_CARD_OPT},
    {LY_STMT_REVISION,     66 * sizeof(void *), LY_STMT_CARD_OPT},
*/    {0, 0, 0} /* terminating item */
};

struct lyext_substmt libyang_ext_test_substmt_arrays[] = {
    {LY_STMT_ARGUMENT,      0,                       LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_BASE,          1 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_BELONGSTO,     2 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char**[2] */
    {LY_STMT_CONTACT,       4 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_DEFAULT,       5 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_DESCRIPTION,   6 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_ERRTAG,        7 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_ERRMSG,        8 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_KEY,           9 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_NAMESPACE,    10 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_ORGANIZATION, 11 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_PATH,         12 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_PREFIX,       13 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_PRESENCE,     14 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_REFERENCE,    15 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_REVISIONDATE, 16 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_UNITS,        17 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_VALUE,        18 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_VERSION,      19 * sizeof(const char*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_DIGITS,       20 * sizeof(const char*), LY_STMT_CARD_ANY}, /* uint8_t* */
    {LY_STMT_MAX,          20 * sizeof(const char*) + sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* uint32_t* */
    {LY_STMT_MIN,          20 * sizeof(const char*) + sizeof(uint8_t*) +
                            1 * sizeof(uint32_t*), LY_STMT_CARD_ANY}, /* uint32_t* */
    {LY_STMT_POSITION,     20 * sizeof(const char*) + sizeof(uint8_t*) +
                            2 * sizeof(uint32_t*), LY_STMT_CARD_ANY}, /* uint32_t* */
/* compress the offset calculation */
    {LY_STMT_UNIQUE,       24 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_unique** */
    {LY_STMT_MODULE,       25 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_module** */
    {LY_STMT_ACTION,       26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_ANYDATA,      26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_ANYXML,       26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_CASE,         26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_CHOICE,       26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_CONTAINER,    26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_GROUPING,     26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_INPUT,        26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_LEAF,         26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_LEAFLIST,     26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_LIST,         26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_NOTIFICATION, 26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_OUTPUT,       26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_USES,         26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
/*    {LY_STMT_TYPEDEF,      47 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_TYPE,         48 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_BIT,          49 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_ENUM,         40 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_REFINE,       51 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_AUGMENT,      52 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_DEVIATE,      53 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_DEVIATION,    54 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_EXTENSION,    55 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_FEATURE,      56 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_IDENTITY,     57 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_IFFEATURE,    58 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_IMPORT,       59 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_INCLUDE,      60 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_LENGTH,       61 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_MUST,         62 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_PATTERN,      63 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_RANGE,        64 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_WHEN,         65 * sizeof(void *), LY_STMT_CARD_ANY},
    {LY_STMT_REVISION,     66 * sizeof(void *), LY_STMT_CARD_ANY},
*/    {0, 0, 0} /* terminating item */
};

/**
 * @brief Plugin structure with cardinalities up to 1
 */
struct lyext_plugin_complex libyang_ext_test_p = {
    .type = LYEXT_COMPLEX,
    .flags = 0,
    .check_position = &libyang_ext_test_position,
    .check_result = NULL,
    .check_inherit = NULL,

    /* specification of allowed substatements of the extension instance */
    .substmt = libyang_ext_test_substmt,

    /* final size of the extension instance structure with the space for storing the substatements */
    .instance_size = 67 * sizeof(void*)
};

/**
 * @brief Plugin structure with cardinalities higher than 1
 */
struct lyext_plugin_complex libyang_ext_test_arrays_p = {
    .type = LYEXT_COMPLEX,
    .flags = 0,
    .check_position = &libyang_ext_test_position,
    .check_result = NULL,
    .check_inherit = NULL,

    /* specification of allowed substatements of the extension instance */
    .substmt = libyang_ext_test_substmt_arrays,

    /* final size of the extension instance structure with the space for storing the substatements */
    .instance_size = 67 * sizeof(void*)
};

/**
 * @brief list of all extension plugins implemented here
 *
 * MANDATORY object for all libyang extension plugins, the name must match the <name>.so
 */
struct lyext_plugin_list libyang_ext_test[] = {
    {"ext-def", "2017-01-18", "complex", (struct lyext_plugin*)&libyang_ext_test_p},
    {"ext-def", "2017-01-18", "complex-arrays", (struct lyext_plugin*)&libyang_ext_test_arrays_p},
    {NULL, NULL, NULL, NULL} /* terminating item */
};
