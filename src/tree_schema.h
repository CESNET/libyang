/**
 * @file tree_schema.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang representation of YANG schema trees.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_SCHEMA_H_
#define LY_TREE_SCHEMA_H_

#include <stdint.h>

/**
 * @defgroup schematree Schema Tree
 * @{
 *
 * Data structures and functions to manipulate and access schema tree.
 */

#define LY_REV_SIZE 11   /**< revision data string length (including terminating NULL byte) */

#define LYS_UNKNOWN 0x0000,        /**< uninitalized unknown statement node */
#define LYS_CONTAINER 0x0001,      /**< container statement node */
#define LYS_CHOICE 0x0002,         /**< choice statement node */
#define LYS_LEAF 0x0004,           /**< leaf statement node */
#define LYS_LEAFLIST 0x0008,       /**< leaf-list statement node */
#define LYS_LIST 0x0010,           /**< list statement node */
#define LYS_ANYXML 0x0020,         /**< anyxml statement node */
#define LYS_CASE 0x0040,           /**< case statement node */
#define LYS_USES 0x0080,           /**< uses statement node */
#define LYS_AUGMENT 0x0100,        /**< augment statement node */
#define LYS_ANYDATA 0x0220,        /**< anydata statement node, in tests it can be used for both #LYS_ANYXML and #LYS_ANYDATA */

/**
 * @brief YANG import-stmt
 */
struct lysp_import {
    const char *prefix;              /**< prefix for the data from the imported schema (mandatory) */
    struct lysp_module *module;      /**< link to the imported module (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    char rev[LY_REV_SIZE];           /**< revision-date of the imported module */
};

/**
 * @brief YANG include-stmt
 */
struct lysp_include {
    struct lysp_module *submodule;   /**< link to the included submodule (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    char rev[LY_REV_SIZE];           /**< revision-date of the included submodule */
};

/**
 * @brief YANG extension-stmt
 */
struct lysp_ext {
    const char *name;                /**< extension name */
    const char *argument;            /**< argument name, NULL if not specified */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    uint16_t flags;                  /**< LYS_STATUS_* and LYS_YINELEM values (@ref snodeflags) */
};

/**
 * @brief Helper structure for generic storage of the extension instances content.
 */
struct lysp_stmt {
    const char *stmt;                /**< identifier of the statement */
    const char *arg;                 /**< statement's argument */
    struct lysp_stmt *next;          /**< link to the next statement */
    struct lysp_stmt *children;      /**< list of the statement's substatements (linked list) */
};

/**
 * @brief YANG extension instance
 */
struct lysp_ext_instance {
    const char *name;                /**< extension identifier, including possible prefix */
    const char *argument;            /**< optional value of the extension's argument */
    struct lysp_stmt *children;      /**< list of the extension's substatements (linked list) */
};

/**
 * @brief YANG feature-stmt
 */
struct lysp_feature {
    const char *name;                /**< feature name (mandatory) */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement  */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values allowed */
};

/**
 * @brief YANG identity-stmt
 */
struct lysp_ident {
    const char *name;                /**< identity name (mandatory), including possible prefix */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    const char **bases;              /**< list of base identifiers (NULL-terminated) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
};

/*
 * @brief Covers restrictions: range, length, pattern, must
 */
struct lysp_restr {
    const char *arg;                 /**< The restriction expression/value (mandatory);
                                          in case of pattern restriction, the first byte has a special meaning:
                                          0x06 (ACK) for regular match and 0x15 (NACK) for invert-match */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
};

/**
 * @brief Enumeration/Bit value definition
 */
struct lysp_type_enum {
    const char *name;                /**< name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    const char *value;               /**< enum's value or bit's position */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
};

/**
 * @brief YANG type-stmt
 *
 * Some of the items in the structure may be mandatory, but it is necessary to resolve the type's base type first
 */
struct lysp_type {
    const char *name;                /**< name of the type (mandatory) */
    struct lysp_restr *range;        /**< allowed values range - numerical, decimal64 */
    struct lysp_restr *length;       /**< allowed length of the value - string, binary */
    struct lysp_restr **patterns;    /**< list of patterns (NULL-terminated) - string */
    struct lysp_type_enum **enumerations; /**< list of enum-stmts (NULL-terminated) - enum */
    struct lysp_type_enum **bits;         /**< list of bit-stmts (NULL-terminated) - bits */
    const char *path;                /**< path - leafref */
    const char **bases;              /**< list of base identifiers (NULL-terminated) - identityref */
    struct lysp_type *types;         /**< list of sub-types (NULL-terminated) - union */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */

    uint8_t fraction_digits;         /**< number of fraction digits - decimal64 */
    uint8_t require_instance;        /**< require-instance flag - leafref, instance */
};

/**
 * @brief YANG typedef-stmt
 */
struct lysp_tpdf {
    const char *name;                /**< name of the newly defined type (mandatory) */
    const char *units;               /**< units of the newly defined type */
    const char *dflt;                /**< default value of the newly defined type */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    struct lysp_type type;           /**< base type from which the typedef is derived (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values allowed */
};

/**
 * @brief YANG grouping-stmt
 */
struct lysp_grp {
    const char *name;                /**< grouping name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_tpdf *typedefs;      /**< list of typedefs (NULL-terminated) */
    struct lysp_grp *groupings;      /**< list of groupings (NULL-terminated) */
    struct lysp_node *data;          /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions (NULL-terminated) */
    struct lysp_notif *notifications;/**< list of notifications (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
};

/**
 * @brief YANG when-stmt
 */
struct lysp_when {
    const char *cond;                /**< specified condition (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
};

/**
 * @brief YANG refine-stmt
 */
struct lysp_refine {
    const char *nodeid;              /**< target descendant schema nodeid (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
    const char *presence;            /**< presence description */
    const char **dflts;              /**< list of default values (NULL-terminated) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
};

/**
 * @brief YANG uses-augment-stmt and augment-stmt
 */
struct lysp_augment {
    const char *nodeid;              /**< target schema nodeid (mandatory) - absolute for global augments, descendant for uses's augments */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions (NULL-terminated) */
    struct lysp_notif *notifications;/**< list of notifications (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
};

/**
 * @defgroup deviatetypes Deviate types
 * @{
 */
#define LYS_DEV_NOT_SUPPORTED 1      /**< deviate type not-supported */
#define LYS_DEV_ADD 2                /**< deviate type add */
#define LYS_DEV_DELETE 3             /**< deviate type delete */
#define LYS_DEV_REPLACE 4            /**< deviate type replace */
/** @} */

/**
 * @brief Generic deviate structure to get type and cast to lysp_deviate_* structure
 */
struct lysp_deviate {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
};

struct lysp_deviate_add {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    const char *units;               /**< units of the leaf's type */
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
    const char **uniques;            /**< list of uniques specifications (NULL-terminated) */
    const char **dflts;              /**< list of default values (NULL-terminated) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_deviate_del {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    const char *units;               /**< units of the leaf's type */
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
    const char **uniques;            /**< list of uniques specifications (NULL-terminated) */
    const char **dflts;              /**< list of default values (NULL-terminated) */
};

struct lysp_deviate_rpl {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_type *type;          /**< type of the node (mandatory) */
    const char *units;               /**< units of the values */
    const char *dflt;                /**< default value */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_deviation {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_deviate* deviates;   /**< list of deviate specifications (linked list) */
};

/**
 * @brief Generic YANG data node
 */
struct lysp_node {
    uint16_t nodetype;               /**< type of the node (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
};

/**
 * @brief Extension structure of the lysp_node for YANG container
 */
struct lysp_node_container {
    uint16_t nodetype;               /**< LYS_CONTAINER */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */

    /* container */
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
    const char *presence;            /**< presence description */
    struct lysp_tpdf *typedefs;      /**< list of typedefs (NULL-terminated) */
    struct lysp_grp *groupings;      /**< list of groupings (NULL-terminated) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions (NULL-terminated) */
    struct lysp_notif *notifications;/**< list of notifications (NULL-terminated) */
};

struct lysp_node_leaf {
    uint16_t nodetype;               /**< LYS_LEAF */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */

    /* leaf */
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
    struct lysp_type type;           /**< type of the leaf node (mandatory) */
    const char *units;               /**< units of the leaf's type */
    const char *dflt;                /**< default value */
};

struct lysp_node_leaflist {
    uint16_t nodetype;               /**< LYS_LEAFLIST */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */

    /* leaf-list */
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
    struct lysp_type type;           /**< type of the leaf node (mandatory) */
    const char *units;               /**< units of the leaf's type */
    const char **dflts;              /**< list of default values (NULL-terminated) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_node_list {
    uint16_t nodetype;               /**< LYS_LIST */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */

    /* list */
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
    const char *key;                 /**< keys specification */
    struct lysp_tpdf *typedefs;      /**< list of typedefs (NULL-terminated) */
    struct lysp_grp *groupings;      /**< list of groupings (NULL-terminated) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions (NULL-terminated) */
    struct lysp_notif *notifications;/**< list of notifications (NULL-terminated) */
    const char **uniques;            /**< list of uniques specifications (NULL-terminated) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_node_choice {
    uint16_t nodetype;               /**< LYS_CHOICE */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */

    /* choice */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    const char* dflt;                /**< default case */
};

struct lysp_node_case {
    uint16_t nodetype;               /**< LYS_CASE */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */

    /* case */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
};

struct lysp_node_anydata {
    uint16_t nodetype;               /**< LYS_ANYXML || LYS_ANYDATA */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */

    /* anyxml/anydata */
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
};

struct lysp_node_uses {
    uint16_t nodetype;               /**< LYS_ANYXML || LYS_ANYDATA */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */

    /* uses */
    struct lysp_refine *refines;     /**< list of uses's refines (NULL-terminated) */
    struct lysp_augment *augments;   /**< list of uses's augment  (NULL-terminated) */
};

/**
 * @brief YANG input-stmt and output-stmt
 */
struct lysp_action_inout {
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs (NULL-terminated) */
    struct lysp_grp *groupings;      /**< list of groupings (NULL-terminated) */
    struct lysp_node *data;          /**< list of data nodes (linked list) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
};

/**
 * @brief YANG rpc-stmt and action-stmt
 */
struct lysp_action {
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs (NULL-terminated) */
    struct lysp_grp *groupings;      /**< list of groupings (NULL-terminated) */
    struct lysp_action_inout input;  /**< RPC's/Action's input */
    struct lysp_action_inout output; /**< RPC's/Action's output */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
};

/**
 * @brief YANG notification-stmt
 */
struct lysp_notif {
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    const char **iffeatures;         /**< list of if-feature expressions (NULL-terminated) */
    struct lysp_restr *musts;        /**< list of must restrictions (NULL-terminated) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs (NULL-terminated) */
    struct lysp_grp *groupings;      /**< list of groupings (NULL-terminated) */
    struct lysp_node *data;          /**< list of data nodes (linked list) */
    struct lysp_ext_instance **ext;  /**< list of the extension instances (NULL-terminated) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
};

/**
 * @brief supported YANG schema version values
 */
typedef enum LYS_VERSION {
    LYS_VERSION_UNDEF = 0,  /**< no specific version, YANG 1.0 as default */
    LYS_VERSION_1_0 = 1,    /**< YANG 1.0 */
    LYS_VERSION_1_1 = 2     /**< YANG 1.1 */
} LYS_VERSION;

/**
 * @brief Printable YANG schema tree structure representing YANG module.
 *
 * Simple structure corresponding to the YANG format. The schema is only syntactically validated.
 */
struct lysp_module {
    struct ly_ctx *ctx;              /**< libyang context of the module (mandatory) */
    const char *name;                /**< name of the module (mandatory) */
    const char *filepath;            /**< path, if the schema was read from a file, NULL in case of reading from memory */
    union {
        /* module */
        const char *ns;                /**< namespace of the module (module - type 0, mandatory) */
        /* submodule */
        struct lysp_module *belongsto; /**< belongs to parent module (submodule - type 1, mandatory) */
    };
    const char *prefix;              /**< prefix of the module (module - type 0, mandatory) */
    struct lysp_import *imports;     /**< list of imported modules (NULL-terminated) */
    struct lysp_include *includes;   /**< list of included submodules (NULL-terminated) */
    const char *org;                 /**< party/company responsible for the module */
    const char *contact;             /**< contact information for the module */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */
    struct lysp_revision *rev;       /**< list of the module revisions (NULL-terminated), the first revision
                                          in the list is always the last (newest) revision of the module */
    struct lysp_ext *extensions;     /**< list of extension statements (NULL-terminated) */
    struct lysp_feature *features;   /**< list of feature definitions (NULL-terminated) */
    struct lysp_ident *identities;   /**< list of identities (NULL-terminated) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs (NULL-terminated) */
    struct lysp_grp *groupings;      /**< list of groupings (NULL-terminated) */
    struct lysp_node *data;          /**< list of module's top-level data nodes (linked list) */
    struct lysp_augment *augments;   /**< list of augments (NULL-terminated) */
    struct lysp_action *rpcs;   /**< list of RPCs (NULL-terminated) */
    struct lysp_notif *notifications; /**< list of notifications (NULL-terminated) */
    struct lysp_deviation *deviations; /**< list of deviations (NULL-terminated) */
    struct lysp_ext_instance **exts; /**< list of the extension instances (NULL-terminated) */

    uint8_t submodule:1;             /**< flag to distinguish main modules and submodules */
    uint8_t deviated:1;              /**< flag if the module is deviated by another module */
    uint8_t implemented:1;           /**< flag if the module is implemented, not just imported */
    uint8_t latest_revision:1;       /**< flag if the module was loaded without specific revision and is
                                          the latest revision found */
    uint8_t version:4;               /**< yang-version (LYS_VERSION values) */
};

/**
 * @brief Compiled YANG schema tree structure representing YANG module.
 *
 * Semantically validated YANG schema tree for data tree parsing.
 * Contains only the necessary information for the data validation.
 */
struct lysc_module {
};

/**
 * @brief Available YANG schema tree structures representing YANG module.
 */
struct lys_module {
    struct lysp_module *parsed;      /**< Simply parsed (unresolved) YANG schema tree */
    struct lysc_module *compiled;    /**< Compiled and fully validated YANG schema tree for data parsing */
};

/** @} */

#endif /* LY_TREE_SCHEMA_H_ */
