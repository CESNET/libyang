/**
 * @file tree.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang representation of data model and data trees.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is, and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include <netinet/ip.h>

#ifndef LY_TREE_H_
#define LY_TREE_H_

#include <stdint.h>

/**
 * @defgroup tree Tree
 * @{
 *
 * Definitions of libyang data structures (for both the data models and
 * instance data trees) and functions to manipulate them.
 *
 */

typedef enum {
    LY_IN_UNKNOWN,
    LY_IN_YANG,
    LY_IN_YIN,
} LY_MINFORMAT;

typedef enum {
    LY_OUT_UNKNOWN,
    LY_OUT_YANG,
    LY_OUT_YIN,
    LY_OUT_TREE,
    LY_OUT_INFO,
} LY_MOUTFORMAT;

typedef enum {
    LY_XML,
    LY_JSON,
} LY_DFORMAT;

typedef enum {
    LY_TYPE_DER,         /**< Derived type */
    LY_TYPE_BINARY,      /**< Any binary data */
    LY_TYPE_BITS,        /**< A set of bits or flags */
    LY_TYPE_BOOL,        /**< "true" or "false" */
    LY_TYPE_DEC64,       /**< 64-bit signed decimal number */
    LY_TYPE_EMPTY,       /**< A leaf that does not have any value */
    LY_TYPE_ENUM,        /**< Enumerated strings */
    LY_TYPE_IDENT,       /**< A reference to an abstract identity */
    LY_TYPE_INST,        /**< References a data tree node */
    LY_TYPE_LEAFREF,     /**< A reference to a leaf instance */
    LY_TYPE_STRING,      /**< Human-readable string */
    LY_TYPE_UNION,       /**< Choice of member types */
    LY_TYPE_INT8,        /**< 8-bit signed integer */
    LY_TYPE_INT16,       /**< 16-bit signed integer */
    LY_TYPE_INT32,       /**< 32-bit signed integer */
    LY_TYPE_INT64,       /**< 64-bit signed integer */
    LY_TYPE_UINT8,       /**< 8-bit unsigned integer */
    LY_TYPE_UINT16,      /**< 16-bit unsigned integer */
    LY_TYPE_UINT32,      /**< 32-bit unsigned integer */
    LY_TYPE_UINT64,      /**< 64-bit unsigned integer */
} LY_DATA_TYPE;
#define LY_DATA_TYPE_COUNT 20

struct ly_types {
    LY_DATA_TYPE type;
    struct ly_tpdf *def;
};
extern struct ly_types ly_types[LY_DATA_TYPE_COUNT];

struct ly_restr {
    const char *expr;             /**< The restriction expression / value */
    const char *dsc;              /**< description */
    const char *ref;              /**< reference */
    const char *eapptag;          /**< error-app-tag value */
    const char *emsg;             /**< error-message */
};

struct ly_type {
    const char *prefix;           /**< prefix for the type referenced in der pointer*/
    LY_DATA_TYPE base;            /**< base type */
    struct ly_tpdf *der;          /**< pointer to the superior type. If NULL,
	                                   structure describes one of the built-in type */

    union {
        /* LY_TYPE_BINARY */
        struct {
            struct ly_restr *length;
        } binary;

        /* LY_TYPE_BITS */
        struct {
            struct ly_type_bit {
                const char *name;
                const char *dsc;
                const char *ref;
                uint8_t status;
                uint32_t pos;
            } *bit;
            int count;
        } bits;

        /* LY_TYPE_DEC64 */
        struct {
            struct ly_restr *range;
            uint8_t dig;
        } dec64;

        /* LY_TYPE_ENUM */
        struct {
            struct ly_type_enum {
                const char *name;
                const char *dsc;
                const char *ref;
                uint8_t status;
                int32_t value;
            } *list;
            int count;
        } enums;

        /* LY_TYPE_IDENT */
        struct {
            struct ly_ident *ref;
        } ident;

        /* LY_TYPE_INST */
        struct {
            int req;    /*  -1 = false, 0 not defined, 1 = true */
        } inst;

        /* LY_TYPE_*INT* */
        struct {
            struct ly_restr *range;
        } num;

        /* LY_TYPE_LEAFREF */
        struct {
            const char *path;
        } lref;

        /* LY_TYPE_STRING */
        struct {
            struct ly_restr *length;
            struct ly_restr *patterns; /* array of patterns */
            int pat_count;
        } str;

        /* LY_TYPE_UNION */
        struct {
            struct ly_type *type;
            int count;
        } uni;
    } info;
};

struct ly_tpdf {
    const char *name;             /**< name of the module */
    const char *dsc;              /**< description */
    const char *ref;              /**< reference */
    uint8_t flags;                /**< only for LY_NODE_STATUS_ values */
    struct ly_module *module;     /**< module where the data type is defined, NULL
	                               in case of built-in type */

    struct ly_type type;          /**< type restrictions and reference to a superior
	                               type definition. Empty in case of built-in
	                               type */
    const char *units;            /**< units of the type */
    const char *dflt;             /**< default value of the type */
};

struct ly_when {
    const char *cond;
    const char *dsc;              /**< description */
    const char *ref;              /**< reference */
};

#define LY_REV_SIZE 11
struct ly_import {
    struct ly_module *module;     /**< link to the imported module */
    const char *prefix;                 /**< prefix for the data from the imported
	                               module */
    char rev[LY_REV_SIZE];        /**< revision-date of the imported module */
};

struct ly_include {
    struct ly_submodule *submodule;
    char rev[LY_REV_SIZE];
};

struct ly_unique {
    uint8_t leafs_size;
    struct ly_mnode_leaf **leafs;
};

struct ly_refine {
    const char *target;
    const char *dsc;
    const char *ref;
    uint8_t flags;

    uint16_t target_type;            /**< 0 - no limitations, ored LY_NODE_TYPE if
	                                  there are some limitations */

    uint8_t must_size;               /**< number of elements in must array */
    struct ly_restr *must;           /**< array of must constraints */

    union {
        const char *dflt;                /**< applicable to leaf or choice, in case of
		                              choice, the value must be resolved to
		                              target the default branch node */
        const char *presence;            /**< applicable to container */
        struct {
            uint32_t min;
            uint32_t max;
        } list;                          /**< applicable to list or leaf-list */
    } mod;
};

typedef enum ly_node_type {
    LY_NODE_AUGMENT = 0,
    LY_NODE_CONTAINER = 0x01,
    LY_NODE_CHOICE = 0x02,
    LY_NODE_LEAF = 0x04,
    LY_NODE_LEAFLIST = 0x08,
    LY_NODE_LIST = 0x10,
    LY_NODE_ANYXML = 0x20,
    LY_NODE_USES = 0x40,
    LY_NODE_GROUPING = 0x80,
    LY_NODE_CASE = 0x100,
    LY_NODE_INPUT = 0x200,
    LY_NODE_OUTPUT = 0x400,
    LY_NODE_NOTIF = 0x800,
    LY_NODE_RPC = 0x1000
} LY_NODE_TYPE;

struct ly_augment {
    const char *target_name;
    const char *dsc;
    const char *ref;
    uint8_t flags;
    uint8_t nacm;
    struct ly_mnode *target;
    LY_NODE_TYPE nodetype;           /**< 0 */
    struct ly_mnode *parent;
    struct ly_mnode *child;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */
    struct ly_when *when;            /**< when statement */
};

typedef enum ly_deviate_type {
    LY_DEVIATE_NO,                   /**< not-supported */
    LY_DEVIATE_ADD,                  /**< add */
    LY_DEVIATE_RPL,                  /**< replace */
    LY_DEVIATE_DEL                   /**< delete */
} LY_DEVIATE_TYPE;

struct ly_deviate {
    LY_DEVIATE_TYPE mod;             /**< type of deviation modification */

    uint8_t flags;                   /**< Properties: config, mandatory */
    const char *dflt;                /**< Properties: default (both type and choice represented as string value */
    uint32_t min;                    /**< Properties: min-elements */
    uint32_t max;                    /**< Properties: max-elements */
    uint8_t must_size;               /**< Properties: must - number of elements in must*/
    uint8_t unique_size;             /**< Properties: unique - number of elements in unique array */
    struct ly_restr *must;           /**< Properties: must - array of must constraints */
    struct ly_unique *unique;        /**< Properties: unique - array of unique statement structures */
    struct ly_type *type;            /**< Properties: type - pointer to type in target, type cannot be deleted or added */
    const char *units;               /**< Properties: units */
};

struct ly_deviation {
    const char *target_name;
    const char *dsc;
    const char *ref;
    struct ly_mnode *target;

    uint8_t deviate_size;            /**< number of elements in deviate array */
    struct ly_deviate *deviate;      /**< deviate information */

};

struct ly_feature {
    const char *name;
    const char *dsc;
    const char *ref;
    uint8_t flags;                   /**< LY_NODE_STATUS_* values and LY_NODE_FENABLED */
    struct ly_module *module;        /**< link to the features's data model */

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */
};

struct ly_module {
    struct ly_ctx *ctx;              /**< libyang context of the module */
    const char *name;                /**< name of the module */
    const char *prefix;              /**< prefix of the module */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */
    const char *org;                 /**< party responsible for the module */
    const char *contact;             /**< contact information for the module */
    uint8_t version:6;               /**< yang-version: 1 = 1.0, 2 = 1.1 */
    uint8_t type:1;                  /**< structure type: 0 - module, used to distinguish structure from submodule */
    uint8_t deviated:1;              /**< deviated flag (true/false) if the module is deviated by some other module */

    /* array sizes */
    uint8_t rev_size;                /**< number of elements in rev array */
    uint8_t imp_size;                /**< number of elements in imp array */
    uint8_t inc_size;                /**< number of elements in inc array */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    uint32_t ident_size;             /**< number of elements in ident array */
    uint8_t features_size;           /**< number of elements in features array */
    uint8_t augment_size;            /**< number of elements in augment array */
    uint8_t deviation_size;          /**< number of elements in deviation array */

    struct ly_revision {
        char date[LY_REV_SIZE];          /**< revision-date */
        const char *dsc;                 /**< revision's dsc */
        const char *ref;                 /**< revision's reference */
    } *rev;                          /**< array of the module revisions,
	                                  revisions[0] is the last revision of the
	                                  module */
    struct ly_import *imp;           /**< array of imported modules */
    struct ly_include *inc;          /**< array of included submodules */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
    struct ly_ident *ident;          /**< array of identities */
    struct ly_feature *features;     /**< array of feature definitions */
    struct ly_augment *augment;      /**< array of augments */
    struct ly_deviation *deviation;  /**< array of specified deviations */

    struct ly_mnode *data;           /**< first data statement */
    struct ly_mnode *rpc;            /**< first rpc statement */
    struct ly_mnode *notif;          /**< first notification statement */

    /* specific module's items in comparison to submodules */
    const char *ns;                  /**< namespace of the module */
};

struct ly_submodule {
    struct ly_ctx *ctx;              /**< libyang context of the module */
    const char *name;                /**< name of the submodule */
    const char *prefix;              /**< prefix of the belongs-to module */
    const char *dsc;                 /**< description of the submodule */
    const char *ref;                 /**< cross-reference for the submodule */
    const char *org;                 /**< party responsible for the submodule */
    const char *contact;             /**< contact information for the submodule */
    uint8_t version:6;               /**< yang-version: 1 = 1.0, 2 = 1.1 */
    uint8_t type:1;                  /**< structure type: 1 - submodule, used to distinguish structure from module */
    uint8_t deviated:1;              /**< deviated flag (true/false) if the module is deviated by some other module */

    /* array sizes */
    uint8_t rev_size;                /**< number of elements in rev array */
    uint8_t imp_size;                /**< number of elements in imp array */
    uint8_t inc_size;                /**< number of elements in inc array */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    uint32_t ident_size;             /**< number of elements in ident array */
    uint8_t features_size;           /**< number of elements in features array */
    uint8_t augment_size;            /**< number of elements in augment array */
    uint8_t deviation_size;          /**< number of elements in deviation array */

    struct ly_revision *rev;         /**< array of the module revisions,
	                                  revisions[0] is the last revision of the
	                                  module */
    struct ly_import *imp;           /**< array of imported modules */
    struct ly_include *inc;          /**< array of included submodules */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
    struct ly_ident *ident;          /**< array if identities */
    struct ly_feature *features;     /**< array of feature definitions */
    struct ly_augment *augment;      /**< array of augments */
    struct ly_deviation *deviation;  /**< array of specified deviations */

    struct ly_mnode *data;           /**< first data statement */
    struct ly_mnode *rpc;            /**< first rpc statement */
    struct ly_mnode *notif;          /**< first notification statement */

    /* specific submodule's items in comparison to modules */
    struct ly_module *belongsto;     /**< belongs-to (parent module) */
};

/* Macros to iterate via all trees elements */
#define LY_TREE_FOR(START, ELEM) \
    for ((ELEM) = (START); (ELEM); (ELEM) = (ELEM)->next)
#define LY_TREE_FOR_SAFE(START, NEXT, ELEM) \
	for ((ELEM) = (START); \
	     (ELEM) ? (NEXT = (ELEM)->next, 1) : 0; \
	     (ELEM)= (NEXT))

/**
 * @brief Common structure representing single YANG model statement describing
 * data.
 *
 * Covers:
 * choice, container, leaf, leaf-list and list
 */
struct ly_mnode {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;                   /**< various flags */
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;
/* ly_mnode's nacm flags */
#define LY_NACM_DENYW        0x01 /**< default-deny-write */
#define LY_NACM_DENYA        0x02 /**< default-deny-all */

/* ly_mnode's flags */
#define LY_NODE_CONFIG_W     0x01 /**< config true; */
#define LY_NODE_CONFIG_R     0x02 /**< config false; */
#define LY_NODE_CONFIG_MASK  0x03 /**< mask for config value */
#define LY_NODE_STATUS_CURR  0x04 /**< status current; */
#define LY_NODE_STATUS_DEPRC 0x08 /**< status deprecated; */
#define LY_NODE_STATUS_OBSLT 0x10 /**< status obsolete; */
#define LY_NODE_STATUS_MASK  0x1c /**< mask for status value */
#define LY_NODE_MAND_TRUE    0x20 /**< mandatory flag of the node, applicable only to
                                       struct ly_mnode_choice, ly_mnode_leaf and ly_mnode_anyxml */
#define LY_NODE_MAND_FALSE   0x40 /**< mandatory false */
#define LY_NODE_MAND_MASK    0x60 /**< mask for mandatory values */
#define LY_NODE_USERORDERED  0x80 /**< ordered-by user lists, applicable only to
                                       struct ly_mnode_list and ly_mnode_leaflist */
#define LY_NODE_FENABLED     0x80 /**< enable flag for features, applicable only to strcut ly_feature */

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */
};

struct ly_mnode_grp {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    uint8_t flags;                   /**< only for LY_NODE_STATUS_ values */
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;

    LY_NODE_TYPE nodetype;           /**< YANG statement */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< dummy memeber to follow struct ly_mnode, always 0 */
    struct ly_feature **features;    /**< dummy memeber to follow struct ly_mnode, always NULL */

    /* specific container's data */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
};

struct ly_mnode_uses {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;                   /**< only for LY_NODE_STATUS_ values */
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_USES */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    /* specific uses's data */
    struct ly_when *when;            /**< when statement */
    struct ly_mnode_grp *grp;        /**< referred grouping definition */

    uint16_t refine_size;
    uint16_t augment_size;

    struct ly_refine *refine;
    struct ly_augment *augment;
};

struct ly_mnode_container {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_CONTAINER */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    /* specific container's data */
    struct ly_when *when;            /**< when statement */
    const char *presence;            /**< presence description, used also as a presence flag */

    uint8_t must_size;               /**< number of elements in must array */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */

    struct ly_tpdf *tpdf;            /**< array of typedefs */
    struct ly_restr *must;           /**< array of must constraints */
};

struct ly_mnode_choice {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_CHOICE */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    /* specific choice's data */
    struct ly_when *when;            /**< when statement */
    struct ly_mnode *dflt;           /**< default case of the choice */
};

struct ly_mnode_case {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_CASE */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    struct ly_when *when;            /**< when statement */
};

struct ly_mnode_anyxml {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_ANYXML */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    /* specific leaf's data */
    struct ly_when *when;            /**< when statement */
    uint8_t must_size;               /**< number of elements in must array */
    struct ly_restr *must;           /**< array of must constraints */
};

struct ly_mnode_leaf {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_LEAF */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    /* specific leaf's data */
    struct ly_when *when;            /**< when statement */
    struct ly_type type;             /**< YANG type of the element */
    const char *units;               /**< units of the type */
    const char *dflt;                /**< default value of the type */

    uint8_t must_size;               /**< number of elements in must array */
    struct ly_restr *must;           /**< array of must constraints */
};

struct ly_mnode_leaflist {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_LEAFLIST */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    /* specific leaf's data */
    struct ly_when *when;            /**< when statement */

    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */

    struct ly_type type;             /**< YANG type of the element */
    const char *units;               /**< units of the type */

    uint8_t must_size;               /**< number of elements in must array */

    struct ly_restr *must;           /**< array of must constraints */
};

struct ly_mnode_list {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_LIST */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    /* specific list's data */
    struct ly_when *when;            /**< when statement */

    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */

    uint8_t must_size;               /**< number of elements in must array */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    uint8_t keys_size;               /**< number of elements in keys array */
    uint8_t unique_size;             /**< number of elements in unique array (number
	                                  of unique statements in the list */

    struct ly_restr *must;           /**< array of must constraints */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
    struct ly_mnode_leaf **keys;     /**< array of pointers to the keys */
    struct ly_unique *unique;        /**< array of unique statement structures */
};

struct ly_mnode_input_output {
    void *name_fill;                 /**< just compatibility */
    void *dsc_fill;
    void *ref_fill;
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_INPUT / LY_NODE_OUTPUT */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< dummy memeber to follow struct ly_mnode, always 0 */
    struct ly_feature **features;    /**< dummy memeber to follow struct ly_mnode, always NULL */

    /* specific list's data */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
};

struct ly_mnode_rpc {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_RPC */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    /* specific rpc's data */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
};

struct ly_mnode_notif {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    uint8_t nacm;                    /**< NACM extension flags */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_NOTIF */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    uint8_t features_size;           /**< number of elements in features array */
    struct ly_feature **features;    /**< array of pointers to feature definitions, this is
                                          not the list of feature definitions itself, but list
                                          of if-feature references */

    /* specific list's data */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
};

/**
 * @brief Item to serialize pointers to the identities.
 *
 * The list of derived identities cannot be static since any new schema can
 * extend the current set of derived identities.
 *
 * TODO: the list could be just an array and we can reallocate it whenever it
 * is needed. Since we are not going to allow removing a particular schema from
 * the context, we don't need to remove a subset of pointers to derived
 * identities.
 */
struct ly_ident_der {
    struct ly_ident *ident;
    struct ly_ident_der *next;
};

/**
 * @brief Structure to hold information about identity (RFC 6020, 7.16)
 *
 * First 5 members maps to struct ly_mnode.
 */
struct ly_ident {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    uint8_t flags;                   /**< only for LY_NODE_STATUS_ values */
    struct ly_module *module;        /**< link to the module of the identity */

    struct ly_ident *base;           /**< pointer to the base identity */
    struct ly_ident_der *der;        /**< list of pointers to the derived identities */
};

/* public libyang functions */
struct ly_submodule *ly_submodule_read(struct ly_module *module, const char *data, LY_MINFORMAT format);
struct ly_submodule *ly_submodule_read_fd(struct ly_module *module, int fd, LY_MINFORMAT format);

/**
 * @brief Enable specified feature in the module
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be enabled.
 * @param[in] name Name of the feature to enable. To enable all features at once, use asterisk character.
 * @return 0 on success, 1 when the feature is not defined in the specified module
 */
int ly_features_enable(struct ly_module *module, const char *name);

/**
 * @brief Disable specified feature in the module
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be disabled.
 * @param[in] name Name of the feature to disable. To disable all features at once, use asterisk character.
 * @return 0 on success, 1 when the feature is not defined in the specified module
 */
int ly_features_disable(struct ly_module *module, const char *name);

/**
 * @brief Get all the features of a module and its direct submodules
 *
 * Optionally, also the enable state can be retrieved.
 *
 * @param[in] module Module with the returned features.
 * @param[out] enable_state Array with the information about enabled state of every feature.
 * @return NULL-terminated array of all the defined features. If enable_state was specififed,
 * it includes NULL-terminated array of either "on" or "off" matching the features in the result.
 */
char **ly_get_features(struct ly_module *module, char ***enable_state);


/******************************************** Data Tree *****************************************************/

typedef enum lyd_attr_type {
    LYD_ATTR_STD = 1,
    LYD_ATTR_NS = 2
} LYD_ATTR_TYPE;

struct lyd_ns {
    LYD_ATTR_TYPE type;
    struct lyd_attr *next;
    struct lyd_node *parent;
    const char *prefix;
    const char *value;
};

struct lyd_attr {
    LYD_ATTR_TYPE type;
    struct lyd_attr *next;
    struct lyd_ns *ns;
    const char *name;
    const char *value;
};

struct lyd_node {
    struct lyd_attr *attr;
    struct lyd_node *next;
    struct lyd_node *prev;
    struct lyd_node *parent;

    struct ly_mnode *schema;
    void *callback;

    struct lyd_node *child;
};

struct lyd_node_list {
    struct lyd_attr *attr;
    struct lyd_node *next;
    struct lyd_node *prev;
    struct lyd_node *parent;

    struct ly_mnode *schema;
    void *callback;

    struct lyd_node *child;

    /* list's specific members */
    struct lyd_node_list* lprev;
    struct lyd_node_list* lnext;
};

struct lyd_node_leaf {
    struct lyd_attr *attr;
    struct lyd_node *next;
    struct lyd_node *prev;
    struct lyd_node *parent;

    struct ly_mnode *schema;
    void *callback;

    /* struct lyd_node *child; is here replaced by the 'value' union */
    /* leaf's specific members */
    union {
        const char *binary;          /**< base64 encoded, NULL terminated string */
        struct ly_type_bit **bit;    /**< array of pointers to the schema definition of the bit value that are set */
        int8_t bool;                 /**< 0 as false, 1 as true */
        int64_t dec64;               /**< decimal64: value = dec64 / 10^fraction-digits  */
        struct ly_type_enum *enm;    /**< pointer to the schema definition of the enumeration value */
        struct ly_ident *ident;      /**< pointer to the schema definition of the identityref value */
        struct lyd_node *instance;   /**< instance-identifier, pointer to the referenced data tree node */
        int8_t int8;
        int16_t int16;
        int32_t int32;
        int64_t int64;
        struct lyd_node_leaf *leafref; /**< pointer to the referenced leaf in data tree */
        const char *string;
        uint8_t uint8;
        uint16_t uint16;
        uint32_t uint32;
        uint64_t uint64;
    } value;
    LY_DATA_TYPE value_type;         /**< mainly for union types to avoid repeating of type detection */
};

struct lyd_node_leaflist {
    struct lyd_attr *attr;
    struct lyd_node *next;
    struct lyd_node *prev;
    struct lyd_node *parent;

    struct ly_mnode *schema;
    void *callback;

    /* struct lyd_node *child; is here replaced by the 'value' union */
    /* leaflist's specific members */
    union {
        const char *binary;          /**< base64 encoded, NULL terminated string */
        struct ly_type_bit **bit;    /**< array of pointers to the schema definition of the bit value that are set */
        int8_t bool;                 /**< 0 as false, 1 as true */
        int64_t dec64;               /**< decimal64: value = dec64 / 10^fraction-digits  */
        struct ly_type_enum *enm;    /**< pointer to the schema definition of the enumeration value */
        struct ly_ident *ident;      /**< pointer to the schema definition of the identityref value */
        struct lyd_node *instance;   /**< instance-identifier, pointer to the referenced data tree node */
        int8_t int8;
        int16_t int16;
        int32_t int32;
        int64_t int64;
        struct lyd_node_leaf *leafref; /**< pointer to the referenced leaf in data tree */
        const char *string;
        uint8_t uint8;
        uint16_t uint16;
        uint32_t uint32;
        uint64_t uint64;
    } value;
    LY_DATA_TYPE value_type;         /**< mainly for union types to avoid repeating of type detection */
    struct lyd_node_leaflist* lprev;
    struct lyd_node_leaflist* lnext;
};



void lyd_node_free(struct lyd_node *node);


/**@} tree */

#endif /* LY_TREE_H_ */
