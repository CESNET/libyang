/**
 * @file xml.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML parser for libyang
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
 */

#ifndef LY_XML_H_
#define LY_XML_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/**
 * @defgroup xmlparser XML Parser
 * @{
 */

/*
 * Structures
 */

/**
 * @brief enumeration of attribute types
 */
typedef enum lyxml_attr_type {
	LYXML_ATTR_STD = 1, /**< standard XML attribute */
	LYXML_ATTR_NS = 2   /**< XML namespace definition */
} LYXML_ATTR_TYPE;

/**
 * @brief Namespace definition.
 *
 * The structure is actually casted lyxml_attr structure which covers all
 * attributes defined in an element. The namespace definition is in this case
 * also covered by lyxml_attr structure.
 */
struct lyxml_ns {
	LYXML_ATTR_TYPE type;      /**< type of the attribute = LYXML_ATTR_NS */
	struct lyxml_ns *next;     /**< next sibling attribute */
	struct lyxml_elem *parent; /**< parent node of the attribute */
	const char *value;         /**< the namespace value */
	const char *prefix;        /**< the namespace prefix if defined, NULL for
	                                default namespace */
};

/**
 * @brief Element's attribute definition
 *
 * The structure actually covers both the attributes as well as namespace
 * definitions.
 *
 * Attributes are being connected only into a singly linked list (compare it
 * with the elements).
 */
struct lyxml_attr {
	LYXML_ATTR_TYPE type;      /**< type of the attribute */
	struct lyxml_attr *next;   /**< next sibling attribute */
	struct lyxml_elem *parent; /**< parent node of the attribute */
	const char *value;         /**< data stored in the attribute */
	const char *name;          /**< name of the attribute (the LocalPart of
	                                the qualified name) */
	const struct lyxml_ns *ns; /**< pointer to the namespace of the attribute
	                                if any */
};

/**
 * @brief Structure describing an element in an XML tree.
 *
 * The structure extends the basic lyxml_node structure with attributes
 * specific to XML elements and not applicable to XML comments and PIs
 * (Processing instructions).
 *
 * If the name item is NULL, then the content is part of the mixed content.
 *
 * Children elements are connected in a half ring doubly linked list:
 * - first's prev pointer points to the last children
 * - last's next pointer is NULL
 */
struct lyxml_elem {
	struct lyxml_elem *next;   /**< next sibling node */
	struct lyxml_elem *prev;   /**< previous sibling node */
	struct lyxml_elem *parent; /**< parent node */
	const char *content;       /**< text content of the node if any */

	struct lyxml_elem *child;  /**< first children element */
	struct lyxml_attr *attr;   /**< first attribute declared in the element */
	const struct lyxml_ns *ns; /**< namespace of the element */
	const char *name;          /**< name of the element */

	char flags;                /**< special flags */
#define LYXML_ELEM_MIXED 0x01  /* element contains mixed content */

#ifndef NDEBUG
	unsigned int line;         /* input line number */
#endif
};

/*
 * Functions
 * Parser
 */

/**
 * @brief Parse XML from in-memory string
 *
 * @param[in] data Pointer to a NULL-terminated string containing XML data to
 * parse.
 * @param[in] options Parser options. Currently ignored, no option defined yet.
 * @return pointer to root of the parsed XML document tree.
 */
struct lyxml_elem* lyxml_read(struct ly_ctx *ctx, const char *data,
                              int options);

/**
 * @brief Parse XML from file descriptor - TODO: NOT IMPLEMENTED
 *
 * @param[in] fd File descriptor where read data to parse
 * @param[in] options Parser options. Currently ignored, no option defined yet.
 * @return pointer to root of the parsed XML document tree.
 */
struct lyxml_elem* lyxml_read_fd(struct ly_ctx *ctx, int fd, int options);

/**
 * @brief Parse XML from filesystem - TODO: NOT IMPLEMENTED
 *
 * @param[in] filename Path to the file where read data to parse
 * @param[in] options Parser options. Currently ignored, no option defined yet.
 * @return pointer to root of the parsed XML document tree.
 */
struct lyxml_elem* lyxml_read_file(struct ly_ctx *ctx, const char* filename,
                                   int options);

/**
 * @brief Dump XML tree to a IO stream
 *
 * @param[in] stream IO stream to print out the tree.
 * @param[in] elem Root element of the XML tree to print
 * @param[in] options Parser options. Currently ignored, no option defined yet.
 * @return number of printed characters.
 *
 */
int lyxml_dump(FILE *stream, struct lyxml_elem *elem, int options);

/*
 * Functions
 * Tree Manipulation
 */

/**
 * @brief Connect the attribute into the specified element.
 *
 * @param[in] parent Element where to connect the attribute.
 * @param[in] attr Attribute to connect. Can be both, the common attribute as
 * well as a namespace definition.
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int lyxml_add_attr(struct lyxml_elem *parent, struct lyxml_attr *attr);

/**
 * @brief Get value of the attribute in the specified element.
 */
const char *lyxml_get_attr(struct lyxml_elem *elem, const char *name,
                           const char *ns);

/**
 * @brief Add a child element into a parent element.
 *
 * The child is added as a last child.
 *
 * @param[in] parent Element where to add the child.
 * @param[in] child Element to be added as a last child of the parent.
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int lyxml_add_child(struct lyxml_elem *parent, struct lyxml_elem *child);

struct lyxml_elem *lyxml_dup_elem(struct ly_ctx *ctx, struct lyxml_elem *elem,
                                  struct lyxml_elem *parent, int recursive);

/**
 * @brief Free attribute. Includes unlinking from an element if the attribute
 * is placed anywhere.
 *
 * @param[in] ctx libyang context to use
 * @param[in] attr Attribute to free.
 */
void lyxml_free_attr(struct ly_ctx *ctx, struct lyxml_attr *attr);

/**
 * @brief Free (and unlink from their element) all attributes (including
 * namespace definitions) of the specified element.
 *
 * @param[in] elem Element to modify.
 */
void lyxml_free_attrs(struct ly_ctx *ctx, struct lyxml_elem *elem);

/**
 * @brief Free (and unlink from the XML tree) the specified element with all
 * its attributes and namespace definitions.
 *
 * @param[in] ctx libyang context to use
 * @param[in] elem Pointer to the element to free.
 */
void lyxml_free_elem(struct ly_ctx *ctx, struct lyxml_elem* elem);

/**
 * @brief Unlink the attribute from its parent element. In contrast to
 * lyxml_free_attr(), after return the caller can still manipulate with the
 * attr.
 *
 * @param[in] attr Attribute to unlink from its parent (if any).
 */
void lyxml_unlink_attr(struct lyxml_attr *attr);

/**
 * @brief Unlink the element from its parent. In contrast to lyxml_free_elem(),
 * after return the caller can still manipulate with the elem.
 *
 * @param[in] elem Element to unlink from its parent (if any).
 */
void lyxml_unlink_elem(struct lyxml_elem *elem);

/**@}*/
#endif /* LY_XML_H_ */
