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
	LYXML_ATTR_TYPE type; /**< type of the attribute = LYXML_ATTR_NS */
	char *value;          /**< the namespace value */
	char *prefix;         /**< the namespace prefix if defined, NULL for
	                           default namespace */
};

/**
 * @brief Element's attribute definition
 *
 * The structure actually covers both the attributes as well as namespace
 * definitions.
 */
struct lyxml_attr {
	LYXML_ATTR_TYPE type;      /**< type of the attribute */
	char *value;               /**< data stored in the attribute */
	char *name;                /**< name of the attribute (the LocalPart of
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
 */
struct lyxml_elem {
	struct lyxml_elem *next;   /**< next sibling node */
	struct lyxml_elem *prev;   /**< previous sibling node */
	struct lyxml_elem *parent; /**< parent node */
	char *content;             /**< text content of the node if any */

	struct lyxml_elem *child;  /**< first children element */
	struct lyxml_attr *attr;   /**< first attribute declared in the element */
	const struct lyxml_ns *ns; /**< namespace of the element */
	char *name;                /**< name of the element */
};

/*
 * Functions
 */

/**
 * @brief Parse XML from in-memory string
 *
 * @param[in] data Pointer to a NULL-terminated string containing XML data to
 * parse.
 * @param[in] options Parser options. Currently ignored, no option defined yet.
 * @return pointer to root of the parsed XML document tree.
 */
struct lyxml_elem* lyxml_read(const char *data, int options);

/**
 * @brief Parse XML from file descriptor
 *
 * @param[in] fd File descriptor where read data to parse
 * @param[in] options Parser options. Currently ignored, no option defined yet.
 * @return pointer to root of the parsed XML document tree.
 */
struct lyxml_elem* lyxml_read_fd(int fd, int options);

/**
 * @brief Parse XML from filesystem
 *
 * @param[in] filename Path to the file where read data to parse
 * @param[in] options Parser options. Currently ignored, no option defined yet.
 * @return pointer to root of the parsed XML document tree.
 */
struct lyxml_elem* lyxml_read_file(const char* filename, int options);

/**
 * @brief Free the element structure with all its attributes.
 *
 * The operation includes unlinking the element from the XML tree. If the
 * element includes a namespace definition used in children, it is moved into
 * the children elements (in case the recursion value is 0).
 *
 * @param[in] elem Pointer to the element to free.
 * @param[in] recursive Flag for free also all the children of the element.
 */
void lyxml_free_elem(struct lyxml_elem* elem, int recursive);

/**@}*/
#endif /* LY_XML_H_ */
