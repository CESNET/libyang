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

#ifndef LY_TREE_H_
#define LY_TREE_H_

#include <stdint.h>

typedef enum {
	LY_UNKNOWN,
	LY_YANG,
	LY_YIN,
} LY_MFORMAT;

typedef enum {
	LY_XML,
	LY_JSON,
} LY_DFORMAT;

typedef enum {
	LY_TYPE_DEF,     /**< Derived type */
	LY_TYPE_BINARY,  /**< Any binary data */
	LY_TYPE_BITS,    /**< A set of bits or flags */
	LY_TYPE_BOOL,    /**< "true" or "false" */
	LY_TYPE_DEC64,   /**< 64-bit signed decimal number */
	LY_TYPE_EMPTY,   /**< A leaf that does not have any value */
	LY_TYPE_ENUM,    /**< Enumerated strings */
	LY_TYPE_IDENT,   /**< A reference to an abstract identity */
	LY_TYPE_INST,    /**< References a data tree node */
	LY_TYPE_LEAFREF, /**< A reference to a leaf instance */
	LY_TYPE_STRING,  /**< Human-readable string */
	LY_TYPE_UNION,   /**< Choice of member types */
	LY_TYPE_INT8,    /**< 8-bit signed integer */
	LY_TYPE_INT16,   /**< 16-bit signed integer */
	LY_TYPE_INT32,   /**< 32-bit signed integer */
	LY_TYPE_INT64,   /**< 64-bit signed integer */
	LY_TYPE_UINT8,   /**< 8-bit unsigned integer */
	LY_TYPE_UINT16,  /**< 16-bit unsigned integer */
	LY_TYPE_UINT32,  /**< 32-bit unsigned integer */
	LY_TYPE_UINT64,  /**< 64-bit unsigned integer */
} LY_DATA_TYPE;
#define LY_DATA_TYPE_COUNT 20

struct ly_types {
	LY_DATA_TYPE type;
	struct ly_tpdf *def;
};
extern struct ly_types ly_types[LY_DATA_TYPE_COUNT];

struct ly_type {
	char *prefix;           /**< prefix for the type referenced in der pointer*/
	LY_DATA_TYPE base;      /**< base type */
	struct ly_tpdf *der;    /**< pointer to the superior type. If NULL,
	                             structure describes one of the built-in type */

	union {
		/* LY_TYPE_BINARY */
		struct {
			char* length;
		} binary;

		/* LY_TYPE_BITS */
		struct {
			struct {
				char *value;
				char *dsc;
				char *ref;
				uint8_t status;
				uint32_t pos;
			} *bit;
			int count;
		} bits;

		/* LY_TYPE_DEC64 */
		struct {
			char *range;
			int dig;
		} dec64;

		/* LY_TYPE_ENUM */
		struct {
			struct {
				char *name;
				char *dsc;
				char *ref;
				uint8_t status;
				int32_t value;
			} *list;
			int count;
		} enums;

		/* LY_TYPE_IDENT */
		struct {
			char *base;
			struct ly_ident *ident;
		} ident;

		/* LY_TYPE_INST */
		struct {
			int req; /* -1 not defined, 0 = false, 1 = true */
		} inst;

		/* LY_TYPE_*INT* */
		struct {
			char *range;
		} num;

		/* LY_TYPE_LEAFREF */
		struct {
			char *path;
		} lref;

		/* LY_TYPE_STRING */
		struct {
			char *length;
			char **pattern;
			int pat_count;
		} str;

		/* LY_TYPE_UNION */
		struct {
			struct ly_type *type;
			int count;
		};
	} info;
};

struct ly_tpdf {
	char *name;             /**< name of the module */
	struct ly_module *module; /**< module where the data type is defined, NULL
	                               in case of built-in type */
	char *dsc;              /**< description */
	char *ref;              /**< reference */
	uint8_t flags;	        /**< only for LY_NODE_STATUS_ values */

	struct ly_type type;    /**< type restrictions and reference to a superior
	                             type definition. Empty in case of built-in
	                             type */
};

typedef enum ly_node_type {
	LY_NODE_CONTAINER = 0x01,
	LY_NODE_CHOICE = 0x02,
	LY_NODE_LEAF = 0x04,
	LY_NODE_LEAFLIST = 0x08,
	LY_NODE_LIST = 0x10,
	LY_NODE_ANYXML = 0x20,
	LY_NODE_USES = 0x40,
	LY_NODE_GROUPING = 0x80
} LY_NODE_TYPE;

struct ly_module {
	struct ly_ctx *ctx;     /**< libyang context of the module */
	char *name;             /**< name of the module */
	char *ns;               /**< namespace of the module */
	char *prefix;           /**< prefix of the module */
	uint8_t version;        /**< yang-version: 1 = 1.0, 2 = 1.1 */

#define LY_REV_SIZE 11
	int imp_size;           /**< number of elements in imp array */
	struct {
		struct ly_module *module; /**< link to the imported module */
		char *prefix;             /**< prefix for the data from the imported
		                               module */
		char rev[LY_REV_SIZE];    /**< revision-date of the imported module */
	} *imp;                 /**< array of imported modules */

	char *org;              /**< party responsible for the module */
	char *contact;          /**< contact information for the module */

	int rev_size;           /**< number of elements in rev array */
	struct {
		char date[LY_REV_SIZE];   /**< revision-date */
		char *dsc;                /**< revision's dsc */
		char *ref;                /**< revision's reference */
	} *rev;                 /**< array of the module revisions,
	                             revisions[0] is the last revision of the
	                             module */
	char *dsc;              /**< description of the module */
	char *ref;              /**< cross-reference for the module */

	int tpdf_size;          /**< number of elements in tpdf array */
	struct ly_tpdf *tpdf;   /**< array of typedefs */

	int ident_size;         /**< number of elements in ident array */
	struct ly_ident *ident; /**< array if identities */

	struct ly_mnode *data;  /**< first data statement */

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
	char *name;                /**< name argument */
	char *dsc;                 /**< description statement */
	char *ref;                 /**< reference statement */
	uint8_t flags;
	struct ly_module *module;  /**< link to the node's data model */

	LY_NODE_TYPE nodetype;     /**< YANG statement */
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

	char *feature;   /**< if-feature statement */
	char *when;      /**< when statement */
};

struct ly_mnode_grp {
	char *name;             /**< name argument */
	char *dsc;              /**< description */
	char *ref;              /**< reference */
	uint8_t flags;	        /**< only for LY_NODE_STATUS_ values */
	struct ly_module *module;

	LY_NODE_TYPE nodetype;     /**< YANG statement */
	struct ly_mnode *parent;
	struct ly_mnode *child;
	struct ly_mnode *next;
	struct ly_mnode *prev;

	/* specific container's data */
	int tpdf_size;          /**< number of elements in tpdf array */
	struct ly_tpdf *tpdf;   /**< array of typedefs */

	/* TODO uses */
};

struct ly_mnode_container {
	char *name;                /**< name argument */
	char *dsc;                 /**< description statement */
	char *ref;                 /**< reference statement */
	uint8_t flags;
	struct ly_module *module;  /**< link to the node's data model */

	LY_NODE_TYPE nodetype;     /**< YANG statement - LY_NODE_CONTAINER */
	struct ly_mnode *parent;
	struct ly_mnode *child;
	struct ly_mnode *next;
	struct ly_mnode *prev;

	char *feature;   /**< if-feature statement */
	char *when;      /**< when statement */

	/* specific container's data */
	int tpdf_size;          /**< number of elements in tpdf array */
	struct ly_tpdf *tpdf;   /**< array of typedefs */
};

struct ly_mnode_choice {
	char *name;                /**< name argument */
	char *dsc;                 /**< description statement */
	char *ref;                 /**< reference statement */
	uint8_t flags;
	struct ly_module *module;  /**< link to the node's data model */

	LY_NODE_TYPE nodetype;     /**< YANG statement - LY_NODE_CHOICE */
	struct ly_mnode *parent;
	struct ly_mnode *child;
	struct ly_mnode *next;
	struct ly_mnode *prev;

	char *feature;   /**< if-feature statement */
	char *when;      /**< when statement */
};

struct ly_mnode_leaf {
	char *name;                /**< name argument */
	char *dsc;                 /**< description statement */
	char *ref;                 /**< reference statement */
	uint8_t flags;
	struct ly_module *module;  /**< link to the node's data model */

	LY_NODE_TYPE nodetype;     /**< YANG statement - LY_NODE_LEAF */
	struct ly_mnode *parent;
	struct ly_mnode *child;
	struct ly_mnode *next;
	struct ly_mnode *prev;

	char *feature;   /**< if-feature statement */
	char *when;      /**< when statement */

	/* specific leaf's data */
	struct ly_type type;    /**< YANG type of the element */
};

struct ly_mnode_leaflist {
	char *name;                /**< name argument */
	char *dsc;                 /**< description statement */
	char *ref;                 /**< reference statement */
	uint8_t flags;
	struct ly_module *module;  /**< link to the node's data model */

	LY_NODE_TYPE nodetype;     /**< YANG statement - LY_NODE_LEAFLIST */
	struct ly_mnode *parent;
	struct ly_mnode *child;
	struct ly_mnode *next;
	struct ly_mnode *prev;

	char *feature;   /**< if-feature statement */
	char *when;      /**< when statement */

	/* specific leaf's data */
	struct ly_type type;    /**< YANG type of the element */
};

struct ly_mnode_list {
	char *name;                /**< name argument */
	char *dsc;                 /**< description statement */
	char *ref;                 /**< reference statement */
	uint8_t flags;
	struct ly_module *module;  /**< link to the node's data model */

	LY_NODE_TYPE nodetype;     /**< YANG statement - LY_NODE_LIST */
	struct ly_mnode *parent;
	struct ly_mnode *child;
	struct ly_mnode *next;
	struct ly_mnode *prev;

	char *feature;   /**< if-feature statement */
	char *when;      /**< when statement */

	/* specific list's data */
	int tpdf_size;          /**< number of elements in tpdf array */
	struct ly_tpdf *tpdf;   /**< array of typedefs */

	int keys_size;          /**< number of elements in keys array */
	struct ly_mnode **keys; /**< array of pointers to the keys */
};

struct ly_ident_der {
	struct ly_ident *ident;
	struct ly_ident_der *next;
};
struct ly_ident {
	char *name;             /**< name argument */
	char *dsc;              /**< description */
	char *ref;              /**< reference */
	uint8_t flags;	        /**< only for LY_NODE_STATUS_ values */
	struct ly_module *module;  /**< link to the module of the identity */

	struct ly_ident *base;  /**< pointer to the base identity */
	struct ly_ident_der *der; /**< list of pointers to the derived identities */
};

#endif /* LY_TREE_H_ */

