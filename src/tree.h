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
} LY_MOUTFORMAT;

typedef enum {
    LY_XML,
    LY_JSON,
} LY_DFORMAT;

typedef enum {
    LY_TYPE_DEF,         /**< Derived type */
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

struct ly_type {
    const char *prefix;               /**< prefix for the type referenced in der pointer*/
    LY_DATA_TYPE base;          /**< base type */
    struct ly_tpdf *der;        /**< pointer to the superior type. If NULL,
	                             structure describes one of the built-in type */

    union {
        /* LY_TYPE_BINARY */
        struct {
            const char *length;
        } binary;

        /* LY_TYPE_BITS */
        struct {
            struct {
                const char *value;
                const char *dsc;
                const char *ref;
                uint8_t status;
                uint32_t pos;
            } *bit;
            int count;
        } bits;

        /* LY_TYPE_DEC64 */
        struct {
            const char *range;
            int dig;
        } dec64;

        /* LY_TYPE_ENUM */
        struct {
            struct {
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
            int req;    /* -1 not defined, 0 = false, 1 = true */
        } inst;

        /* LY_TYPE_*INT* */
        struct {
            const char *range;
        } num;

        /* LY_TYPE_LEAFREF */
        struct {
            const char *path;
        } lref;

        /* LY_TYPE_STRING */
        struct {
            const char *length;
            const char **pattern;
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

struct ly_must {
    const char *cond;             /**< XPath expression of the must statement */
    const char *dsc;              /**< description */
    const char *ref;              /**< reference */
    const char *eapptag;          /**< error-app-tag value */
    const char *emsg;             /**< error-message */
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
    struct ly_must *must;            /**< array of must constraints */

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
    struct ly_mnode *target;
    LY_NODE_TYPE nodetype;           /**< 0 */
    struct ly_mnode *parent;
    struct ly_mnode *child;
};

struct ly_module {
    struct ly_ctx *ctx;              /**< libyang context of the module */
    const char *name;                /**< name of the module */
    const char *prefix;              /**< prefix of the module */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */
    const char *org;                 /**< party responsible for the module */
    const char *contact;             /**< contact information for the module */
    uint8_t version:7;               /**< yang-version: 1 = 1.0, 2 = 1.1 */
    uint8_t type:1;                  /**< structure type: 0 - module */

    /* array sizes */
    uint8_t rev_size;                /**< number of elements in rev array */
    uint8_t imp_size;                /**< number of elements in imp array */
    uint8_t inc_size;                /**< number of elements in inc array */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    uint32_t ident_size;             /**< number of elements in ident array */

    struct {
        char date[LY_REV_SIZE];          /**< revision-date */
        const char *dsc;                 /**< revision's dsc */
        const char *ref;                 /**< revision's reference */
    } *rev;                          /**< array of the module revisions,
	                                  revisions[0] is the last revision of the
	                                  module */
    struct ly_import *imp;           /**< array of imported modules */
    struct ly_include *inc;          /**< array of included submodules */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
    struct ly_ident *ident;          /**< array if identities */

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
    uint8_t version:7;               /**< yang-version: 1 = 1.0, 2 = 1.1 */
    uint8_t type:1;                  /**< structure type: 1 - submodule */

    /* array sizes */
    uint8_t rev_size;                /**< number of elements in rev array */
    uint8_t imp_size;                /**< number of elements in imp array */
    uint8_t inc_size;                /**< number of elements in inc array */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    uint32_t ident_size;             /**< number of elements in ident array */

    struct {
        char date[LY_REV_SIZE];          /**< revision-date */
        const char *dsc;                 /**< revision's dsc */
        const char *ref;                 /**< revision's reference */
    } *rev;                          /**< array of the module revisions,
	                                  revisions[0] is the last revision of the
	                                  module */
    struct ly_import *imp;           /**< array of imported modules */
    struct ly_include *inc;          /**< array of included submodules */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
    struct ly_ident *ident;          /**< array if identities */

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
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

/* ly_mnode's flags */
#define LY_NODE_CONFIG_W     0x01 /**< config true; */
#define LY_NODE_CONFIG_R     0x02 /**< config false; */
#define LY_NODE_CONFIG_MASK  0x03 /**< mask for config value */
#define LY_NODE_STATUS_CURR  0x04 /**< status current; */
#define LY_NODE_STATUS_DEPRC 0x08 /**< status deprecated; */
#define LY_NODE_STATUS_OBSLT 0x10 /**< status obsolete; */
#define LY_NODE_STATUS_MASK  0x1c /**< mask for status value */
#define LY_NODE_MAND_TRUE    0x20 /**< mandatory flag of the node */
#define LY_NODE_MAND_FALSE   0x40 /**< mandatory false */
#define LY_NODE_MAND_MASK    0x60 /**< mask for mandatory values */
#define LY_NODE_USERORDERED  0x80 /**< ordered-by user lists */

    const char *feature;             /**< if-feature statement */
    const char *when;                /**< when statement */
};

struct ly_mnode_grp {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    uint8_t flags;                   /**< only for LY_NODE_STATUS_ values */
    struct ly_module *module;

    LY_NODE_TYPE nodetype;           /**< YANG statement */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    /* specific container's data */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
};

struct ly_mnode_uses {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;                   /**< only for LY_NODE_STATUS_ values */
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_USES */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;                /**< when statement */

    /* specific uses's data */
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
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_CONTAINER */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;                /**< when statement */

    /* specific container's data */
    const char *presence;            /**< presence description, used also as a
	                                  presence flag */

    uint8_t must_size;               /**< number of elements in must array */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */

    struct ly_tpdf *tpdf;            /**< array of typedefs */
    struct ly_must *must;            /**< array of must constraints */
};

struct ly_mnode_choice {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_CHOICE */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;                /**< when statement */

    /* specific choice's data */
    struct ly_mnode *dflt;           /**< default case of the choice */
};

struct ly_mnode_case {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_CASE */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;                /**< when statement */
};

struct ly_mnode_anyxml {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_ANYXML */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;                /**< when statement */

    /* specific leaf's data */
    uint8_t must_size;               /**< number of elements in must array */
    struct ly_must *must;            /**< array of must constraints */
};

struct ly_mnode_leaf {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_LEAF */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;                /**< when statement */

    /* specific leaf's data */
    struct ly_type type;             /**< YANG type of the element */
    const char *units;               /**< units of the type */
    const char *dflt;                /**< default value of the type */

    uint8_t must_size;               /**< number of elements in must array */
    struct ly_must *must;            /**< array of must constraints */
};

struct ly_mnode_leaflist {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_LEAFLIST */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;                /**< when statement */

    /* specific leaf's data */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */

    struct ly_type type;             /**< YANG type of the element */
    const char *units;               /**< units of the type */

    uint8_t must_size;               /**< number of elements in must array */

    struct ly_must *must;            /**< array of must constraints */
};

struct ly_mnode_list {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_LIST */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;                /**< when statement */

    /* specific list's data */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */

    uint8_t must_size;               /**< number of elements in must array */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    uint8_t keys_size;               /**< number of elements in keys array */
    uint8_t unique_size;             /**< number of elements in unique array (number
	                                  of unique statements in the list */

    struct ly_must *must;            /**< array of must constraints */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
    struct ly_mnode_leaf **keys;     /**< array of pointers to the keys */
    struct ly_unique *unique;        /**< array of unique statement structures */
};

struct ly_mnode_input_output {
    void *name_fill;                 /**< just compatibility */
    void *dsc_fill;
    void *ref_fill;
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_INPUT / LY_NODE_OUTPUT */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    void *feature_fill;
    void *when_fill;

    /* specific list's data */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
};

struct ly_mnode_rpc {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_RPC */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;

    /* specific list's data */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
};

struct ly_mnode_notif {
    const char *name;                /**< name argument */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    uint8_t flags;
    struct ly_module *module;        /**< link to the node's data model */

    LY_NODE_TYPE nodetype;           /**< YANG statement - LY_NODE_NOTIF */
    struct ly_mnode *parent;
    struct ly_mnode *child;
    struct ly_mnode *next;
    struct ly_mnode *prev;

    const char *feature;             /**< if-feature statement */
    const char *when;

    /* specific list's data */
    uint8_t tpdf_size;               /**< number of elements in tpdf array */
    struct ly_tpdf *tpdf;            /**< array of typedefs */
};

struct ly_ident_der {
    struct ly_ident *ident;
    struct ly_ident_der *next;
};
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

/**@} tree */

#endif /* LY_TREE_H_ */
