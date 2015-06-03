/**
 * @file yin.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YIN parser for libyang
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "../libyang.h"
#include "../common.h"
#include "../context.h"
#include "../dict.h"
#include "../parser.h"

#include "../tree_internal.h"
#include "../xml.h"

enum LY_IDENT {
	LY_IDENT_FEATURE,
	LY_IDENT_IDENTITY,
	LY_IDENT_TYPE,
	LY_IDENT_NODE,
	LY_IDENT_NAME,
	LY_IDENT_PREFIX
};

#define LY_NSYIN "urn:ietf:params:xml:ns:yang:yin:1"

#define GETVAL(value, node, arg)                                    \
	value = lyxml_get_attr(node, arg, NULL);                        \
	if (!value) {                                                   \
		LOGVAL(VE_MISSARG, LOGLINE(node), arg, node->name);         \
		goto error;                                                 \
	}

static int read_yin_common(struct ly_module *, struct ly_mnode *, struct ly_mnode *, struct lyxml_elem *, int );
static struct ly_mnode *read_yin_choice(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_container(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_leaf(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_leaflist(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_list(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_uses(struct ly_module *, struct ly_mnode *, struct lyxml_elem *, int);
static struct ly_mnode *read_yin_grouping(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);

static int dup_typedef_check(const char* type, struct ly_tpdf *tpdf, int size)
{
	int i;

	for (i = 0; i < size; i++) {
		if (!strcmp(type, tpdf[i].name)) {
			/* name collision */
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

static int dup_prefix_check(const char* prefix, struct ly_module *module)
{
	int i;

	if (!module->type && module->prefix && !strcmp(module->prefix, prefix)) {
		return EXIT_FAILURE;
	}
	for (i = 0; i < module->imp_size; i++) {
		if (!strcmp(module->imp[i].prefix, prefix)) {
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

static int check_identifier(const char *id, enum LY_IDENT type, unsigned int line,
                            struct ly_module *module, struct ly_mnode *parent)
{
	int i;
	int size;
	struct ly_tpdf *tpdf;



	assert(id);

	/* check id syntax */
	if (!(id[0] >= 'A' && id[0] <= 'Z') && !(id[0] >= 'a' && id[0] <= 'z') && id[0] != '_') {
		LOGVAL(VE_INID, line, id, "invalid start character");
		return EXIT_FAILURE;
	}
	for (i = 1; id[i]; i++) {
		if (!(id[i] >= 'A' && id[i] <= 'Z') && !(id[i] >= 'a' && id[i] <= 'z')
				&& !(id[i] >= '0' && id[i] <= '9') && id[i] != '_' && id[i] != '-' && id[i] != '.') {
			LOGVAL(VE_INID, line, id, "invalid %d. character", i + 1);
			return EXIT_FAILURE;
		}
	}

	if (i > 64) {
		LOGWRN("Identifier \"%s\" is long, you should use something shorter.", id);
	}

	switch(type) {
	case LY_IDENT_TYPE:
		assert(module);

		/* check collision with the built-in types */
		if (!strcmp(id, "binary") || !strcmp(id,"bits") ||
				!strcmp(id, "boolean") || !strcmp(id, "decimal64") ||
				!strcmp(id, "empty") || !strcmp(id, "enumeration") ||
				!strcmp(id, "identityref") || !strcmp(id, "instance-identifier") ||
				!strcmp(id, "int8") || !strcmp(id, "int16") ||
				!strcmp(id, "int32") || !strcmp(id, "int64") ||
				!strcmp(id, "leafref") || !strcmp(id, "string") ||
				!strcmp(id, "uint8") || !strcmp(id, "uint16") ||
				!strcmp(id, "uint32") || !strcmp(id, "uint64") ||
				!strcmp(id, "union")) {
			LOGVAL(VE_SPEC, line, "Typedef name duplicates built-in type.");
			return EXIT_FAILURE;
		}

		/* check locally scoped typedefs (avoid name shadowing) */
		for ( ; parent; parent = parent->parent) {
			switch(parent->nodetype) {
			case LY_NODE_CONTAINER:
				size = ((struct ly_mnode_container *)parent)->tpdf_size;
				tpdf = ((struct ly_mnode_container *)parent)->tpdf;
				break;
			case LY_NODE_LIST:
				size = ((struct ly_mnode_list *)parent)->tpdf_size;
				tpdf = ((struct ly_mnode_list *)parent)->tpdf;
				break;
			case LY_NODE_GROUPING:
				size = ((struct ly_mnode_grp *)parent)->tpdf_size;
				tpdf = ((struct ly_mnode_grp *)parent)->tpdf;
				break;
			default:
				continue;
			}

			if (dup_typedef_check(id, tpdf, size)) {
				LOGVAL(VE_DUPID, line, "typedef", id);
				return EXIT_FAILURE;
			}
		}

		/* check top-level names */
		if (dup_typedef_check(id, module->tpdf, module->tpdf_size)) {
			LOGVAL(VE_DUPID, line, "typedef", id);
			return EXIT_FAILURE;
		}

		/* check submodule's top-level names */
		for (i = 0; i < module->inc_size; i++) {
			if (dup_typedef_check(id, module->inc[i].submodule->tpdf, module->inc[i].submodule->tpdf_size)) {
				LOGVAL(VE_DUPID, line, "typedef", id);
				return EXIT_FAILURE;
			}
		}

		/* check top-level names in the main module */
		if (module->type) {
			if (dup_typedef_check(id, ((struct ly_submodule *)module)->belongsto->tpdf,
					((struct ly_submodule *)module)->belongsto->tpdf_size)) {
				LOGVAL(VE_DUPID, line, "typedef", id);
				return EXIT_FAILURE;
			}
		}

		break;
	case LY_IDENT_PREFIX:
		assert(module);

		if (module->type) {
			/* go to the main module */
			module = ((struct ly_submodule *)module)->belongsto;
		}

		/* check the main module itself */
		if (dup_prefix_check(id, module)) {
			LOGVAL(VE_DUPID, line, "prefix", id);
			return EXIT_FAILURE;
		}

		/* and all its submodules */
		for (i = 0; i < module->inc_size; i++) {
			if (dup_prefix_check(id, (struct ly_module *)module->inc[i].submodule)) {
				LOGVAL(VE_DUPID, line, "prefix", id);
				return EXIT_FAILURE;
			}
		}
		break;
	default:
		/* no check required */
		break;
	}

	return EXIT_SUCCESS;
}

static int check_default(struct ly_type *type, const char* value)
{
	/* TODO - RFC 6020, sec. 7.3.4 */
	(void)type;
	(void)value;
	return EXIT_SUCCESS;
}

static int check_date(const char* date, unsigned int line)
{
	int i;

	assert(date);

	if (strlen(date) != LY_REV_SIZE - 1) {
		goto error;
	}

	for (i = 0; i < LY_REV_SIZE - 1; i++) {
		if (i == 4 || i == 7) {
			if (date[i] != '-') {
				goto error;
			}
		} else if (!isdigit(date[i])) {
			goto error;
		}
	}

	return EXIT_SUCCESS;

error:

	LOGVAL(VE_INDATE, line, date);
	return EXIT_FAILURE;
}

static const char *read_yin_text(struct ly_ctx *ctx, struct lyxml_elem *node)
{
	int len;

	/* there should be <text> child */
	if (!node->child || !node->child->name
	        || strcmp(node->child->name, "text")) {
		LOGWRN("Expected \"text\" element in \"%s\" element.", node->name);
	} else if (node->child->content) {
		len = strlen(node->child->content);
		return lydict_insert(ctx, node->child->content, len);
	}

	LOGVAL(VE_INARG, LOGLINE(node), "text", node->name);
	return NULL;
}

static struct ly_tpdf *find_superior_type(const char *name,
                                          struct ly_module *module,
                                          struct ly_mnode *parent)
{
	int i, j, found = 0;
	int prefix_len = 0;
	const char *qname;
	struct ly_tpdf *tpdf;
	int tpdf_size;

	qname = strchr(name, ':');

	if (!qname) {
		/* no prefix, try built-in types */
		for (i = 1; i < LY_DATA_TYPE_COUNT; i++) {
			if (!strcmp(ly_types[i].def->name, name)) {
				return ly_types[i].def;
			}
		}
		qname = name;
	} else {
		/* set qname to correct position after colon */
		prefix_len = qname - name;
		qname++;

		if (!strncmp(name, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
			/* prefix refers to the current module, ignore it */
			prefix_len = 0;
		}
	}

	if (!prefix_len && parent) {
		/* search in local typedefs */
		while (parent) {
			switch (parent->nodetype) {
			case LY_NODE_CONTAINER:
				tpdf_size = ((struct ly_mnode_container *)parent)->tpdf_size;
				tpdf = ((struct ly_mnode_container *)parent)->tpdf;
				break;
			case LY_NODE_LIST:
				tpdf_size = ((struct ly_mnode_list *)parent)->tpdf_size;
				tpdf = ((struct ly_mnode_list *)parent)->tpdf;
				break;
			case LY_NODE_GROUPING:
				tpdf_size = ((struct ly_mnode_grp *)parent)->tpdf_size;
				tpdf = ((struct ly_mnode_grp *)parent)->tpdf;
				break;
			default:
				parent = parent->parent;
				continue;
			}

			for (i = 0; i < tpdf_size; i++) {
				if (!strcmp(tpdf[i].name, qname)) {
					return &tpdf[i];
				}
			}

			parent = parent->parent;
		}
	} else if (prefix_len) {
		/* get module where to search */
		for (i = 0; i < module->imp_size; i++) {
			if (!strncmp(module->imp[i].prefix, name, prefix_len) && !module->imp[i].prefix[prefix_len]) {
				module = module->imp[i].module;
				found = 1;
				break;
			}
		}
		if (!found) {
			return NULL;
		}
	}

	/* search in top level typedefs */
	for (i = 0; i < module->tpdf_size; i++) {
		if (!strcmp(module->tpdf[i].name, qname)) {
			return &module->tpdf[i];
		}
	}

	/* search in submodules */
	for (i = 0; i < module->inc_size; i++) {
		for (j = 0; j < module->inc[i].submodule->tpdf_size; j++) {
			if (!strcmp(module->inc[i].submodule->tpdf[j].name, qname)) {
				return &module->inc[i].submodule->tpdf[j];
			}
		}
	}

	return NULL;
}

static struct ly_ident *find_base_ident_sub(struct ly_module *module, struct ly_ident *ident, const char *basename)
{
	unsigned int i;
	struct ly_ident *base_iter;
	struct ly_ident_der *der;

	for (i = 0; i < module->ident_size; i++) {
		if (!strcmp(basename, module->ident[i].name)) {
			/* we are done */

			if (!ident) {
				/* just search for type, so do not modify anything, just return
				 * the base identity pointer
				 */
				return &module->ident[i];
			}

			/* we are resolving identity definition, so now update structures */
			ident->base = base_iter = &module->ident[i];

			while (base_iter) {
				for (der = base_iter->der; der && der->next; der = der->next);
				if (der) {
					der->next = malloc(sizeof *der);
					der = der->next;
				} else {
					ident->base->der = der = malloc(sizeof *der);
				}
				der->next = NULL;
				der->ident = ident;

				base_iter = base_iter->base;
			}
			return ident->base;
		}
	}

	return NULL;
}

static struct ly_ident *find_base_ident(struct ly_module *module, struct ly_ident *ident, struct lyxml_elem *node)
{
	const char *name;
	int prefix_len = 0;
	int i, found = 0;
	struct ly_ident *result;
	const char *basename;

	basename = lyxml_get_attr(node, "name", NULL);
	if (!basename) {
		LOGVAL(VE_MISSARG, LOGLINE(node), "name", "base");
		return NULL;
	}

	/* search for the base identity */
	name = strchr(basename, ':');
	if (name) {
		/* set name to correct position after colon */
		prefix_len = name - basename;
		name++;

		if (!strncmp(basename, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
			/* prefix refers to the current module, ignore it */
			prefix_len = 0;
		}
	} else {
		name = basename;
	}

	if (prefix_len) {
		/* get module where to search */
		for (i = 0; i < module->imp_size; i++) {
			if (!strncmp(module->imp[i].prefix, basename, prefix_len)
					&& !module->imp[i].prefix[prefix_len]) {
				module = module->imp[i].module;
				found = 1;
				break;
			}
		}
		if (!found) {
			/* identity refers unknown data model */
			LOGVAL(VE_INPREFIX, LOGLINE(node), basename);
			return NULL;
		}
	} else {
		/* search in submodules */
		for (i = 0; i < module->inc_size; i++) {
			result = find_base_ident_sub((struct ly_module *)module->inc[i].submodule, ident, name);
			if (result) {
				return result;
			}
		}
	}

	/* search in the identified module */
	result = find_base_ident_sub(module, ident, name);
	if (!result) {
		LOGVAL(VE_INARG, LOGLINE(node), basename, ident ? "identity" : "type");
	}

	return result;
}

static int fill_yin_identity(struct ly_module *module, struct lyxml_elem *yin, struct ly_ident *ident)
{
	struct lyxml_elem *node, *next;

	if (read_yin_common(module, NULL, (struct ly_mnode *)ident, yin, 0)) {
		return EXIT_FAILURE;
	}
	ident->module = module;

	LY_TREE_FOR_SAFE(yin->child, next, node) {
		if (!strcmp(node->name, "base")) {
			if (ident->base) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), "base", "identity");
				return EXIT_FAILURE;
			}
			if (!find_base_ident(module, ident, node)) {
				return EXIT_FAILURE;
			}
		} else {
			LOGVAL(VE_INSTMT, LOGLINE(node), node->name, "identity");
			return EXIT_FAILURE;
		}

		lyxml_free_elem(module->ctx, node);
	}

	return EXIT_SUCCESS;
}

static int fill_yin_type(struct ly_module *module, struct ly_mnode *parent,
                         struct lyxml_elem *yin, struct ly_type *type)
{
	const char *value, *delim;
	struct lyxml_elem *next, *node, root = {0};
	int i, j, r;
	int64_t v, v_;

	GETVAL(value, yin, "name")
	delim = strchr(value, ':');
	if (delim) {
		type->prefix = lydict_insert(module->ctx, value, delim - value);
	}

	type->der = find_superior_type(value, module, parent);
	if (!type->der) {
		LOGVAL(VE_INARG, LOGLINE(yin), value, yin->name);
		goto error;
	}
	type->base = type->der->type.base;

	switch (type->base) {
	case LY_TYPE_BINARY:
		/* TODO length, 9.4.4
		 * - optional, 0..1, rekurzivni - omezuje, string (podobne jako range),
		 * hodnoty se musi vejit do 64b, podelementy
		 */
		break;
	case LY_TYPE_BITS:
		/* TODO bit, 9.7.4
		 * 1..n, nerekurzivni, stringy s podelementy */
		break;
	case LY_TYPE_DEC64:
		/* TODO fraction-digits, 9.3.4
		 * - MUST, 1, nerekurzivni, hodnota 1-18 */
		/* TODO range, 9.2.4
		 * - optional, 0..1, rekurzivne - omezuje, string,  podelementy*/
		break;
	case LY_TYPE_ENUM:
		/* RFC 6020 9.6 */

		/* get enum specification, at least one must be present */
		LY_TREE_FOR_SAFE(yin->child, next, node) {
			if (!strcmp(node->name, "enum")) {
				lyxml_unlink_elem(node);
				lyxml_add_child(&root, node);
				type->info.enums.count++;
			}
		}
		if (yin->child) {
			LOGVAL(VE_INSTMT, LOGLINE(yin->child), yin->child->name);
			goto error;
		}
		if (!type->info.enums.count) {
			if (type->der->type.der) {
				/* this is just a derived type with no enum specified */
				break;
			}
			LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "enum", "type");
			goto error;
		}

		type->info.enums.list = calloc(type->info.enums.count, sizeof *type->info.enums.list);
		for (i = v = 0; root.child; i++) {
			r = read_yin_common(module, NULL, (struct ly_mnode *)&type->info.enums.list[i], root.child, 0);
			if (r) {
				type->info.enums.count = i + 1;
				goto error;
			}
			/* the assigned name MUST NOT have any leading or trailing whitespace characters */
			value = type->info.enums.list[i].name;
			if (isspace(value[0]) || isspace(value[strlen(value) - 1])) {
				LOGVAL(VE_ENUM_WS, LOGLINE(root.child), value);
				type->info.enums.count = i + 1;
				goto error;
			}

			/* check the name uniqueness */
			for (j = 0; j < i; j++) {
				if (!strcmp(type->info.enums.list[j].name, type->info.enums.list[i].name)) {
					LOGVAL(VE_ENUM_DUPNAME, LOGLINE(root.child), type->info.enums.list[i].name);
					type->info.enums.count = i + 1;
					goto error;
				}
			}

			node = root.child->child;
			if (node && !strcmp(node->name, "value")) {
				value = lyxml_get_attr(node, "value", NULL);
				v_ = strtol(value, NULL, 10);

				/* range check */
				if (v_ < INT32_MIN || v_ > INT32_MAX) {
					LOGVAL(VE_INARG, LOGLINE(node), value, "enum/value");
					type->info.enums.count = i + 1;
					goto error;
				}
				type->info.enums.list[i].value = v_;

				/* keep the highest enum value for automatic increment */
				if (type->info.enums.list[i].value > v) {
					v = type->info.enums.list[i].value;
					v++;
				} else {
					/* check that the value is unique */
					for (j = 0; j < i; j++) {
						if (type->info.enums.list[j].value == type->info.enums.list[i].value) {
							LOGVAL(VE_ENUM_DUPVAL, LOGLINE(node), type->info.enums.list[i].value, type->info.enums.list[i].name);
							type->info.enums.count = i + 1;
							goto error;
						}
					}
				}
			} else {
				/* assign value automatically */
				if (v > INT32_MAX) {
					LOGVAL(VE_INARG, LOGLINE(root.child), "2147483648", "enum/value");
					type->info.enums.count = i + 1;
					goto error;
				}
				type->info.enums.list[i].value = v;
				v++;
			}
			lyxml_free_elem(module->ctx, root.child);
		}
		break;
	case LY_TYPE_IDENT:
		/* RFC 6020 9.10 */

		/* get base specification, exactly one must be present */
		if (!yin->child) {
			LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "base", "type");
			goto error;
		}
		if (strcmp(yin->child->name, "base")) {
			LOGVAL(VE_INSTMT, LOGLINE(yin->child), yin->child->name);
			goto error;
		}
		if (yin->child->next) {
			LOGVAL(VE_INSTMT, LOGLINE(yin->child->next), yin->child->next->name);
			goto error;
		}
		type->info.ident.ref = find_base_ident(module, NULL, yin->child);
		if (!type->info.ident.ref) {
			return EXIT_FAILURE;
		}
		break;
	case LY_TYPE_INST:
		/* TODO require-instance, 9.13.2
		 * - 0..1, true/false */
		break;
	case LY_TYPE_INT8:
	case LY_TYPE_INT16:
	case LY_TYPE_INT32:
	case LY_TYPE_INT64:
	case LY_TYPE_UINT8:
	case LY_TYPE_UINT16:
	case LY_TYPE_UINT32:
	case LY_TYPE_UINT64:
		/* TODO range, 9.2.4
		 * - optional, 0..1, i rekurzivne - omezuje, string, podelementy*/
		break;
	case LY_TYPE_LEAFREF:
		/* TODO path, 9.9.2
		 * - 1, nerekurzivni, string */
		break;
	case LY_TYPE_STRING:
		/* TODO length, 9.4.4
		 * - optional, 0..1, rekurzivni - omezuje, string (podobne jako range), hodnoty se musi vejit do 64b, podelementy
		 * pattern, 9.4.6
		 * - optional, 0..n, rekurzivni - rozsiruje, string, podelementy */
		break;
	case LY_TYPE_UNION:
		/* TODO type, 7.4
		 * - 1..n, nerekurzivni, resp rekurzivni pro union ale bez vazby na predky, nesmi byt empty nebo leafref */
		break;
	default:
		/* nothing needed :
		 * LY_TYPE_BOOL, LY_TYPE_EMPTY
		 */
		break;
	}

	return EXIT_SUCCESS;

error:

	while(root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}

	return EXIT_FAILURE;
}

static int fill_yin_typedef(struct ly_module *module, struct ly_mnode *parent,
                            struct lyxml_elem *yin, struct ly_tpdf *tpdf)
{
	const char *value;
	struct lyxml_elem *node, *next;
	int r = 0;

	GETVAL(value, yin, "name");
	if (check_identifier(value, LY_IDENT_TYPE, LOGLINE(yin), module, parent)) {
		goto error;
	}
	tpdf->name = lydict_insert(module->ctx, value, strlen(value));

	/* generic part - status, description, reference */
	if (read_yin_common(module, NULL, (struct ly_mnode *)tpdf, yin, 0)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(yin->child, next, node) {
		if (!strcmp(node->name, "type")) {
			if (tpdf->type.der) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			r = fill_yin_type(module, parent, node, &tpdf->type);
		} else if (!strcmp(node->name, "default"))  {
			if (tpdf->dflt) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			GETVAL(value, node, "value");
			tpdf->dflt = lydict_insert(module->ctx, value, strlen(value));
		} else if (!strcmp(node->name, "units")) {
			if (tpdf->units) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			GETVAL(value, node, "name");
			tpdf->units = lydict_insert(module->ctx, value, strlen(value));
		} else {
			LOGVAL(VE_INSTMT, LOGLINE(node), value);
			r = 1;
		}
		lyxml_free_elem(module->ctx, node);
		if (r) {
			goto error;
		}
	}

	/* check mandatory value */
	if (!tpdf->type.der) {
		LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "type", yin->name);
		goto error;
	}

	/* check default value */
	if (check_default(&tpdf->type, tpdf->dflt)) {
		goto error;
	}

	return EXIT_SUCCESS;

error:

	return EXIT_FAILURE;
}

static int fill_yin_must(struct ly_module *module, struct lyxml_elem *yin, struct ly_must *must)
{
	struct lyxml_elem *child;
	const char *value;

	GETVAL(value, yin, "condition");
	must->cond = lydict_insert(module->ctx, value, strlen(value));

	LY_TREE_FOR(yin->child, child) {
		if (!strcmp(child->name, "description")) {
			if (must->dsc) {
				LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
				goto error;
			}
			must->dsc = read_yin_text(module->ctx, child);
			if (!must->dsc) {
				goto error;
			}
		} else if (!strcmp(child->name, "reference")) {
			if (must->ref) {
				LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
				goto error;
			}
			must->ref = read_yin_text(module->ctx, child);
			if (!must->ref) {
				goto error;
			}
		} else if (!strcmp(child->name, "error-app-tag")) {
			if (must->eapptag) {
				LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
				goto error;
			}
			must->eapptag = read_yin_text(module->ctx, child);
			if (!must->eapptag) {
				goto error;
			}
		} else if (!strcmp(child->name, "error-message")) {
			if (must->emsg) {
				LOGVAL(VE_TOOMANY, LOGLINE(child), child->name, yin->name);
				goto error;
			}
			must->emsg = read_yin_text(module->ctx, child);
			if (!must->emsg) {
				goto error;
			}
		} else {
			LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
			goto error;
		}

		lyxml_free_elem(module->ctx, child);
	}


	return EXIT_SUCCESS;

error:

	return EXIT_FAILURE;
}

static int fill_yin_import(struct ly_module *module, struct lyxml_elem *yin, struct ly_import *imp)
{
	struct lyxml_elem *child;
	const char *value;

	LY_TREE_FOR(yin->child, child) {
		if (!strcmp(child->name, "prefix")) {
			GETVAL(value, child, "value");
			if (check_identifier(value, LY_IDENT_PREFIX, LOGLINE(child), module, NULL)) {
				goto error;
			}
			imp->prefix = lydict_insert(module->ctx, value, strlen(value));
		} else if (!strcmp(child->name, "revision-date")) {
			if (imp->rev[0]) {
				LOGVAL(VE_TOOMANY, LOGLINE(child), "revision-date", yin->name);
				goto error;
			}
			GETVAL(value, child, "date");
			if (check_date(value, LOGLINE(child))) {
				goto error;
			}
			memcpy(imp->rev, value, LY_REV_SIZE - 1);
		} else {
			LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
			goto error;
		}
	}

	/* check mandatory information */
	if (!imp->prefix) {
		LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "prefix", yin->name);
		goto error;
	}

	GETVAL(value, yin, "module");
	imp->module = ly_ctx_get_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL, 1);
	if (!imp->module) {
		LOGERR(LY_EVALID, "Importing \"%s\" module into \"%s\" failed.",
		       value, module->name);
		LOGVAL(VE_INARG, LOGLINE(yin), value, yin->name);
		goto error;
	}

	return EXIT_SUCCESS;

error:

	return EXIT_FAILURE;
}

static int fill_yin_include(struct ly_module *module, struct lyxml_elem *yin, struct ly_include *inc)
{
	struct lyxml_elem *child;
	const char *value;

	LY_TREE_FOR(yin->child, child) {
		if (!strcmp(child->name, "revision-date")) {
			if (inc->rev[0]) {
				LOGVAL(VE_TOOMANY, LOGLINE(child), "revision-date", yin->name);
				goto error;
			}
			GETVAL(value, child, "date");
			if (check_date(value, LOGLINE(child))) {
				goto error;
			}
			memcpy(inc->rev, value, LY_REV_SIZE - 1);
		} else {
			LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
			goto error;
		}
	}

	GETVAL(value, yin, "module");
	inc->submodule = ly_ctx_get_submodule(module, value, inc->rev[0] ? inc->rev : NULL);
	if (!inc->submodule) {
		LOGERR(LY_EVALID, "Including \"%s\" module into \"%s\" failed.",
		       value, module->name);
		LOGVAL(VE_INARG, LOGLINE(yin), value, yin->name);
		goto error;
	}

	/* check that belongs-to corresponds */
	if (module->type) {
		module = ((struct ly_submodule *)module)->belongsto;
	}
	if (inc->submodule->belongsto != module) {
		LOGVAL(VE_INARG, LOGLINE(yin), value, yin->name);
		LOGVAL(VE_SPEC, 0, "The included module does not belongs-to the \"%s\" module", module->name);
		goto error;
	}

	return EXIT_SUCCESS;

error:

	return EXIT_FAILURE;
}


/*
 * Covers:
 * description, reference, status, optionaly config
 */
static int read_yin_common(struct ly_module *module, struct ly_mnode *parent,
		                   struct ly_mnode *mnode, struct lyxml_elem *xmlnode, int ext)
{
	const char *value;
	struct lyxml_elem *sub, *next;
	struct ly_ctx * const ctx = module->ctx;
	int r = 0;

	if (ext) {
		mnode->module = module;
	}

	GETVAL(value, xmlnode, "name");
	if (check_identifier(value, LY_IDENT_NAME, LOGLINE(xmlnode), NULL, NULL)) {
		goto error;
	}
	mnode->name = lydict_insert(ctx, value, strlen(value));

	/* process local parameters */
	LY_TREE_FOR_SAFE(xmlnode->child, next, sub) {
		if (!strcmp(sub->name, "description")) {
			if (mnode->dsc) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
				goto error;
			}
			mnode->dsc = read_yin_text(ctx, sub);
			if (!mnode->dsc) {
				r = 1;
			}
		} else if (!strcmp(sub->name, "reference")) {
			if (mnode->ref) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
				goto error;
			}
			mnode->ref = read_yin_text(ctx, sub);
			if (!mnode->ref) {
				r = 1;
			}
		} else if (!strcmp(sub->name, "status")) {
			if (mnode->flags & LY_NODE_STATUS_MASK) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
				goto error;
			}
			GETVAL(value, sub, "value");
			if (!strcmp(value, "current")) {
				mnode->flags |= LY_NODE_STATUS_CURR;
			} else if (!strcmp(value, "deprecated")) {
				mnode->flags |= LY_NODE_STATUS_DEPRC;
			} else if (!strcmp(value, "obsolete")) {
				mnode->flags |= LY_NODE_STATUS_OBSLT;
			} else {
				LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
				r = 1;
			}
		} else if (ext && !strcmp(sub->name, "config")) {
			if (mnode->flags & LY_NODE_CONFIG_MASK) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
				goto error;
			}
			GETVAL(value, sub, "value");
			if (!strcmp(value, "false")) {
				mnode->flags |= LY_NODE_CONFIG_R;
			} else if (!strcmp(value, "false")) {
				mnode->flags |= LY_NODE_CONFIG_W;
			} else {
				LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
				r = 1;
			}
		} else {
			/* skip the lyxml_free_elem */
			continue;
		}
		lyxml_free_elem(ctx, sub);
		if (r) {
			goto error;
		}
	}

	if (ext && !(mnode->flags & LY_NODE_CONFIG_MASK)) {
		/* get config flag from parent */
		if (parent) {
			mnode->flags |= parent->flags & LY_NODE_CONFIG_MASK;
		} else {
			/* default config is true */
			mnode->flags |= LY_NODE_CONFIG_W;
		}
	}

	return EXIT_SUCCESS;

error:

	return EXIT_FAILURE;
}

static struct ly_mnode *read_yin_choice(struct ly_module *module,
                                        struct ly_mnode *parent,
                                        struct lyxml_elem *node)
{
	struct lyxml_elem *sub, *next;
	struct ly_ctx * const ctx = module->ctx;
	struct ly_mnode *retval, *r;
	struct ly_mnode_choice *choice;

	choice = calloc(1, sizeof *choice);
	choice->nodetype = LY_NODE_CHOICE;
	choice->module = module;
	choice->prev = (struct ly_mnode *)choice;
	retval = (struct ly_mnode *)choice;

	if (read_yin_common(module, parent, retval, node, 1)) {
		goto error;
	}

	/* process choice's specific children */
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "container")) {
			r = read_yin_container(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf-list")) {
			r = read_yin_leaflist(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf")) {
			r = read_yin_leaf(module, retval, sub);
		} else if (!strcmp(sub->name, "list")) {
			r = read_yin_list(module, retval, sub);
		} else {
			continue;
		}
		lyxml_free_elem(ctx, sub);
		if (!r) {
			goto error;
		}
	}

	if (parent) {
		ly_mnode_addchild(parent, retval);
	}

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

static struct ly_mnode *read_yin_leaf(struct ly_module *module,
                                      struct ly_mnode *parent,
                                      struct lyxml_elem *yin)
{
	struct ly_mnode *retval;
	struct ly_mnode_leaf *leaf;
	struct lyxml_elem *sub, *next;
	const char *value;
	int r;
	int c_must = 0, f_mand = 0;

	leaf = calloc(1, sizeof *leaf);
	leaf->nodetype = LY_NODE_LEAF;
	leaf->prev = (struct ly_mnode *)leaf;
	retval = (struct ly_mnode *)leaf;

	if (read_yin_common(module, parent, retval, yin, 1)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(yin->child, next, sub) {
		if (!strcmp(sub->name, "type")) {
			if (leaf->type.der) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			if (fill_yin_type(module, parent, sub, &leaf->type)) {
				goto error;
			}
		} else if (!strcmp(sub->name, "default"))  {
			if (leaf->dflt) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			GETVAL(value, sub, "value");
			leaf->dflt = lydict_insert(module->ctx, value, strlen(value));
		} else if (!strcmp(sub->name, "units"))  {
			if (leaf->units) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			GETVAL(value, sub, "name");
			leaf->units = lydict_insert(module->ctx, value, strlen(value));
		} else if (!strcmp(sub->name, "mandatory")) {
			if (f_mand) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			/* just checking the flags in leaf is not sufficient, we would allow
			 * multiple mandatory statements with the "false" value
			 */
			f_mand = 1;

			GETVAL(value, sub, "value");
			if (!strcmp(value, "true")) {
				leaf->flags |= LY_NODE_MANDATORY;
			} else if (strcmp(value, "false")) {
				LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
				goto error;
			} /* else false is the default value, so we can ignore it */
		} else if (!strcmp(sub->name, "must")) {
			c_must++;

			/* skip element free at the end of the loop */
			continue;
#if 0
		} else {
			LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
			goto error;
#endif
		}

		lyxml_free_elem(module->ctx, sub);
	}

	/* check mandatory parameters */
	if (!leaf->type.der) {
		LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "type", yin->name);
		goto error;
	}
	if (leaf->dflt && check_default(&leaf->type, leaf->dflt)) {
		goto error;
	}

	/* middle part - process nodes with cardinality of 0..n */
	if (c_must) {
		leaf->must = calloc(c_must, sizeof *leaf->must);
	}

	LY_TREE_FOR_SAFE(yin->child, next, sub) {
		if (!strcmp(sub->name, "must")) {
			r = fill_yin_must(module, sub, &leaf->must[leaf->must_size]);
			leaf->must_size++;

			if (r) {
				goto error;
			}
		}

		lyxml_free_elem(module->ctx, sub);
	}

	if (parent) {
		ly_mnode_addchild(parent, retval);
	}

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

static struct ly_mnode *read_yin_leaflist(struct ly_module *module,
                                          struct ly_mnode *parent,
                                          struct lyxml_elem *yin)
{
	struct ly_mnode *retval;
	struct ly_mnode_leaflist *llist;
	struct lyxml_elem *sub, *next;
	const char *value;
	char *endptr;
	long val;
	int r;
	int c_must = 0;
	int f_ordr = 0, f_min = 0, f_max;

	llist = calloc(1, sizeof *llist);
	llist->nodetype = LY_NODE_LEAFLIST;
	llist->prev = (struct ly_mnode *)llist;
	retval = (struct ly_mnode *)llist;

	if (read_yin_common(module, parent, retval, yin, 1)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(yin->child, next, sub) {
		if (!strcmp(sub->name, "type")) {
			if (llist->type.der) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			if (fill_yin_type(module, parent, sub, &llist->type)) {
				goto error;
			}
		} else if (!strcmp(sub->name, "units"))  {
			if (llist->units) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			GETVAL(value, sub, "name");
			llist->units = lydict_insert(module->ctx, value, strlen(value));
		} else if (!strcmp(sub->name, "ordered-by")) {
			if (f_ordr) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			/* just checking the flags in llist is not sufficient, we would
			 * allow multiple ordered-by statements with the "system" value
			 */
			f_ordr = 1;

			if (llist->flags & LY_NODE_CONFIG_R) {
				/* RFC 6020, 7.7.5 - ignore ordering when the list represents
				 * state data
				 */
				lyxml_free_elem(module->ctx, sub);
				continue;
			}

			GETVAL(value, sub, "value");
			if (!strcmp(value, "user")) {
				llist->flags |= LY_NODE_USERORDERED;
			} else if (strcmp(value, "system")) {
				LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
				goto error;
			} /* else system is the default value, so we can ignore it */
		} else if (!strcmp(sub->name, "must")) {
			c_must++;

			/* skip element free at the end of the loop */
			continue;
		} else if (!strcmp(sub->name, "min-elements")) {
			if (f_min) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			f_min = 1;

			GETVAL(value, sub, "value");
			while(isspace(value[0])) {
				value++;
			}

			/* convert it to uint32_t */
			errno = 0;
			endptr = NULL;
			val = strtoul(value, &endptr, 10);
			if (!*endptr || value[0] == '-' || errno) {
				LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
				goto error;
			}
			llist->min = val;
		} else if (!strcmp(sub->name, "max-elements")) {
			if (f_max) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			f_max = 1;

			GETVAL(value, sub, "value");
			while(isspace(value[0])) {
				value++;
			}

			/* convert it to uint32_t */
			errno = 0;
			endptr = NULL;
			val = strtoul(value, &endptr, 10);
			if (!*endptr || value[0] == '-' || errno || val == 0) {
				LOGVAL(VE_INARG, LOGLINE(sub), value, sub->name);
				goto error;
			}
			llist->max = val;

		} else {
#if 0
			LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
			goto error;
#endif
		}

		lyxml_free_elem(module->ctx, sub);
	}

	/* check constraints */
	if (!llist->type.der) {
		LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "type", yin->name);
		goto error;
	}
	if (llist->max && llist->min > llist->max) {
		LOGVAL(VE_SPEC, LOGLINE(yin), "\"min-elements\" is bigger than \"max-elements\".");
		goto error;
	}

	/* middle part - process nodes with cardinality of 0..n */
	if (c_must) {
		llist->must = calloc(c_must, sizeof *llist->must);
	}

	LY_TREE_FOR_SAFE(yin->child, next, sub) {
		if (!strcmp(sub->name, "must")) {
			r = fill_yin_must(module, sub, &llist->must[llist->must_size]);
			llist->must_size++;

			if (r) {
				goto error;
			}
		}

		lyxml_free_elem(module->ctx, sub);
	}

	if (parent) {
		ly_mnode_addchild(parent, retval);
	}

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

static struct ly_mnode *read_yin_list(struct ly_module *module,
                                      struct ly_mnode *parent,
                                      struct lyxml_elem *node)
{
	struct ly_mnode *retval, *mnode;
	struct ly_mnode_list *list;
	struct ly_mnode_leaf *key;
	struct lyxml_elem *sub, *next, root = {0};
	int i, j, r;
	size_t len;
	int c_tpdf = 0;
	const char *key_str = NULL, *s;
	char *dup;

	list = calloc(1, sizeof *list);
	list->nodetype = LY_NODE_LIST;
	list->prev = (struct ly_mnode *)list;
	retval = (struct ly_mnode *)list;

	if (read_yin_common(module, parent, retval, node, 1)) {
		goto error;
	}

	/* process list's specific children */
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		/* data statements */
		if (!strcmp(sub->name, "container") ||
				!strcmp(sub->name, "leaf-list") ||
				!strcmp(sub->name, "leaf") ||
				!strcmp(sub->name, "list") ||
				!strcmp(sub->name, "choice") ||
				!strcmp(sub->name, "uses") ||
				!strcmp(sub->name, "grouping")) {
			lyxml_unlink_elem(sub);
			lyxml_add_child(&root, sub);

		/* array counters */
		} else if (!strcmp(sub->name, "key")) {
			/* check cardinality 0..1 */
			if (list->keys_size) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), "key", list->name);
				goto error;
			}

			/* count the number of keys */
			key_str = s = lyxml_get_attr(sub, "value", NULL);
			if (!s) {
				LOGVAL(VE_MISSARG, LOGLINE(sub), "value", "key");
				goto error;
			}
			while((s = strpbrk(s, " \t\n"))) {
				list->keys_size++;
				while(isspace(*s)) {
					s++;
				}
			}
			list->keys_size++;

			list->keys = calloc(list->keys_size, sizeof *list->keys);

		} else if (!strcmp(sub->name, "typedef")) {
			c_tpdf++;
		}
	}

	/* check - if list is configuration, key statement is mandatory */
	if ((list->flags & LY_NODE_CONFIG_W) && !key_str) {
		LOGVAL(VE_MISSSTMT2, LOGLINE(node), "key", "list");
		goto error;
	}

	/* middle part - process nodes with cardinality of 0..n except the data nodes */
	if (c_tpdf) {
		list->tpdf_size = c_tpdf;
		list->tpdf = calloc(c_tpdf, sizeof *list->tpdf);
		c_tpdf = 0;
	}
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "typedef")) {
			r = fill_yin_typedef(module, retval, sub, &list->tpdf[c_tpdf]);
			c_tpdf++;

			if (r) {
				list->tpdf_size = c_tpdf;
				goto error;
			}
			lyxml_free_elem(module->ctx, sub);
		}
	}

	/* last part - process data nodes */
	LY_TREE_FOR_SAFE(root.child, next, sub) {
		if (!strcmp(sub->name, "container")) {
			mnode = read_yin_container(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf-list")) {
			mnode = read_yin_leaflist(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf")) {
			mnode = read_yin_leaf(module, retval, sub);
		} else if (!strcmp(sub->name, "list")) {
			mnode = read_yin_list(module, retval, sub);
		} else if (!strcmp(sub->name, "choice")) {
			mnode = read_yin_choice(module, retval, sub);
		} else if (!strcmp(sub->name, "uses")) {
			mnode = read_yin_uses(module, retval, sub, 1);
		} else if (!strcmp(sub->name, "grouping")) {
			mnode = read_yin_grouping(module, retval, sub);
		} else {
			/* TODO error */
			continue;
		}
		lyxml_free_elem(module->ctx, sub);

		if (!mnode) {
			goto error;
		}
	}

	if (parent) {
		ly_mnode_addchild(parent, retval);
	}

	if (!key_str) {
		/* config false list without a key */
		return retval;
	}

	/* link key leafs into the list structure and check all constraints  */
	/* TODO - include searching in uses/grouping */
	for (i = 0; i < list->keys_size; i++) {
		/* get the key name */
		if ((s = strpbrk(key_str, " \t\n"))) {
			len = s - key_str;
		} else {
			len = strlen(key_str);
		}
		LY_TREE_FOR(list->child, mnode) {
			if (!strncmp(mnode->name, key_str, len) && !mnode->name[len]) {
				list->keys[i] = mnode;
				break;
			}
		}
		key = (struct ly_mnode_leaf *)list->keys[i];

		/* existence */
		if (!key) {
			if ((s = strpbrk(key_str, " \t\n"))) {
				len = s - key_str;
				dup = strdup(key_str);
				dup[len] = '\0';
				key_str = dup;
			}
			LOGVAL(VE_KEY_MISS, LOGLINE(node), key_str);
			if (s) {
				free(dup);
			}
			goto error;
		}

		/* uniqueness */
		for (j = i - 1; j >= 0; j--) {
			if (list->keys[i] == list->keys[j]) {
				LOGVAL(VE_KEY_DUP, LOGLINE(node), key->name, list->name);
				goto error;
			}
		}

		/* key is a leaf */
		if (key->nodetype != LY_NODE_LEAF) {
			LOGVAL(VE_KEY_NLEAF, LOGLINE(node), key->name, list->name);
			goto error;
		}

		/* type of the leaf is not built-in empty */
		if (key->type.base == LY_TYPE_EMPTY) {
			LOGVAL(VE_KEY_TYPE, LOGLINE(node), key->name, list->name);
			goto error;
		}

		/* config attribute is the same as of the list */
		if ((list->flags & LY_NODE_CONFIG_MASK) != (key->flags & LY_NODE_CONFIG_MASK)) {
			LOGVAL(VE_KEY_CONFIG, LOGLINE(node), key->name, list->name);
			goto error;
		}

		/* prepare for next iteration */
		while (s && isspace(*s)) {
			s++;
		}
		key_str = s;
	}

	return retval;

error:

	ly_mnode_free(retval);
	while(root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}

	return NULL;
}

static struct ly_mnode *read_yin_container(struct ly_module *module,
                                           struct ly_mnode *parent,
                                           struct lyxml_elem *yin)
{
	struct lyxml_elem *sub, *next, root = {0};
	struct ly_mnode *mnode = NULL;
	struct ly_mnode *retval;
	struct ly_mnode_container *cont;
	const char *value;
	int r;
	int c_tpdf = 0, c_must = 0;

	cont = calloc(1, sizeof *cont);
	cont->nodetype = LY_NODE_CONTAINER;
	cont->prev = (struct ly_mnode *)cont;
	retval = (struct ly_mnode *)cont;

	if (read_yin_common(module, parent, retval, yin, 1)) {
		goto error;
	}

	/* process container's specific children */
	LY_TREE_FOR_SAFE(yin->child, next, sub) {
		if (!strcmp(sub->name, "presence")) {
			if (cont->presence) {
				LOGVAL(VE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
				goto error;
			}
			GETVAL(value, sub, "value");
			cont->presence = lydict_insert(module->ctx, value, strlen(value));

			lyxml_free_elem(module->ctx, sub);

		/* data statements */
		} else if (!strcmp(sub->name, "container") ||
				!strcmp(sub->name, "leaf-list") ||
				!strcmp(sub->name, "leaf") ||
				!strcmp(sub->name, "list") ||
				!strcmp(sub->name, "choice") ||
				!strcmp(sub->name, "uses") ||
				!strcmp(sub->name, "grouping")) {
			lyxml_unlink_elem(sub);
			lyxml_add_child(&root, sub);

		/* array counters */
		} else if (!strcmp(sub->name, "typedef")) {
			c_tpdf++;
		} else if (!strcmp(sub->name, "must")) {
			c_must++;
#if 0
		} else {
			LOGVAL(VE_INSTMT, LOGLINE(sub), sub->name);
			goto error;
#endif
		}
	}

	/* middle part - process nodes with cardinality of 0..n except the data nodes */
	if (c_tpdf) {
		cont->tpdf = calloc(c_tpdf, sizeof *cont->tpdf);
	}
	if (c_must) {
		cont->must = calloc(c_must, sizeof *cont->must);
	}

	LY_TREE_FOR_SAFE(yin->child, next, sub) {
		if (!strcmp(sub->name, "typedef")) {
			r = fill_yin_typedef(module, retval, sub, &cont->tpdf[cont->tpdf_size]);
			cont->tpdf_size++;

			if (r) {
				goto error;
			}
		} else if (!strcmp(sub->name, "must")) {
			r = fill_yin_must(module, sub, &cont->must[cont->must_size]);
			cont->must_size++;

			if (r) {
				goto error;
			}
		}

		lyxml_free_elem(module->ctx, sub);
	}

	/* last part - process data nodes */
	LY_TREE_FOR_SAFE(root.child, next, sub) {
		if (!strcmp(sub->name, "container")) {
			mnode = read_yin_container(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf-list")) {
			mnode = read_yin_leaflist(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf")) {
			mnode = read_yin_leaf(module, retval, sub);
		} else if (!strcmp(sub->name, "list")) {
			mnode = read_yin_list(module, retval, sub);
		} else if (!strcmp(sub->name, "choice")) {
			mnode = read_yin_choice(module, retval, sub);
		} else if (!strcmp(sub->name, "uses")) {
			mnode = read_yin_uses(module, retval, sub, 1);
		} else if (!strcmp(sub->name, "grouping")) {
			mnode = read_yin_grouping(module, retval, sub);
		}
		lyxml_free_elem(module->ctx, sub);

		if (!mnode) {
			goto error;
		}
	}

	if (parent) {
		ly_mnode_addchild(parent, retval);
	}

	return retval;

error:

	ly_mnode_free(retval);
	while (root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}

	return NULL;
}

static struct ly_mnode *read_yin_grouping(struct ly_module *module,
                                          struct ly_mnode *parent,
                                          struct lyxml_elem *node)
{
	struct lyxml_elem *sub, *next, root = {0};
	struct ly_mnode *mnode = NULL;
	struct ly_mnode *retval;
	struct ly_mnode_grp *grp;
	int r;
	int c_tpdf = 0;

	grp = calloc(1, sizeof *grp);
	grp->nodetype = LY_NODE_GROUPING;
	grp->module = module;
	grp->prev = (struct ly_mnode *)grp;
	retval = (struct ly_mnode *)grp;

	if (read_yin_common(module, parent, retval, node, 0)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(node->child, next, sub) {
		/* data statements */
		if (!strcmp(sub->name, "container") ||
				!strcmp(sub->name, "leaf-list") ||
				!strcmp(sub->name, "leaf") ||
				!strcmp(sub->name, "list") ||
				!strcmp(sub->name, "choice") ||
				!strcmp(sub->name, "uses") ||
				!strcmp(sub->name, "grouping")) {
			lyxml_unlink_elem(sub);
			lyxml_add_child(&root, sub);

		/* array counters */
		} else if (!strcmp(sub->name, "typedef")) {
			c_tpdf++;
		}
	}

	/* middle part - process nodes with cardinality of 0..n except the data nodes */
	if (c_tpdf) {
		grp->tpdf_size = c_tpdf;
		grp->tpdf = calloc(c_tpdf, sizeof *grp->tpdf);
		c_tpdf = 0;
	}
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "typedef")) {
			r = fill_yin_typedef(module, retval, sub, &grp->tpdf[c_tpdf]);
			c_tpdf++;

			if (r) {
				grp->tpdf_size = c_tpdf;
				goto error;
			}
		}

		lyxml_free_elem(module->ctx, sub);
	}

	/* last part - process data nodes */
	LY_TREE_FOR_SAFE(root.child, next, sub) {
		if (!strcmp(sub->name, "container")) {
			mnode = read_yin_container(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf-list")) {
			mnode = read_yin_leaflist(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf")) {
			mnode = read_yin_leaf(module, retval, sub);
		} else if (!strcmp(sub->name, "list")) {
			mnode = read_yin_list(module, retval, sub);
		} else if (!strcmp(sub->name, "choice")) {
			mnode = read_yin_choice(module, retval, sub);
		} else if (!strcmp(sub->name, "uses")) {
			mnode = read_yin_uses(module, retval, sub, 0);
		} else if (!strcmp(sub->name, "grouping")) {
			mnode = read_yin_grouping(module, retval, sub);
		} else {
			/* TODO error */
			continue;
		}
		lyxml_free_elem(module->ctx, sub);

		if (!mnode) {
			goto error;
		}
	}

	if (parent) {
		ly_mnode_addchild(parent, retval);
	}

	return retval;

error:

	ly_mnode_free(retval);
	while (root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}

	return NULL;
}

/*
 * resolve - referenced grouping should be bounded to the namespace (resolved)
 * only when uses does not appear in grouping. In a case of grouping's uses,
 * we just get information but we do not apply augment or refine to it.
 */
static struct ly_mnode *read_yin_uses(struct ly_module *module,
                                      struct ly_mnode *parent,
                                      struct lyxml_elem *node, int resolve)
{
	struct ly_mnode *retval;
	struct ly_mnode *mnode = NULL, *par;
	struct ly_mnode_uses *uses;
	struct ly_module *searchmod = NULL;
	const char *name;
	int prefix_len = 0;
	int i;

	uses = calloc(1, sizeof *uses);
	uses->nodetype = LY_NODE_USES;
	uses->module = module;
	uses->prev = (struct ly_mnode *)uses;
	retval = (struct ly_mnode *)uses;

	if (read_yin_common(module, parent, retval, node, 0)) {
		goto error;
	}

	/* get referenced grouping */
	name = strchr(uses->name, ':');
	if (!name) {
		/* no prefix, search in local tree */
		name = uses->name;
	} else {
		/* there is some prefix, check if it refer the same data model */

		/* set name to correct position after colon */
		prefix_len = name - uses->name;
		name++;

		if (!strncmp(uses->name, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
			/* prefix refers to the current module, ignore it */
			prefix_len = 0;
		}
	}

	/* search */
	if (prefix_len) {
		/* in top-level groupings of some other module */
		for (i = 0; i < module->imp_size; i++) {
			if (!strncmp(module->imp[i].prefix, uses->name, prefix_len)
					&& !module->imp[i].prefix[prefix_len]) {
				searchmod = module->imp[i].module;
				break;
			}
		}
		if (!searchmod) {
			/* uses refers unknown data model */
			LOGVAL(VE_INPREFIX, LOGLINE(node), name);
			goto error;
		}

		LY_TREE_FOR(module->data, mnode) {
			if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
				uses->grp = (struct ly_mnode_grp *)mnode;
				break;
			}
		}

		if (!uses->grp) {
			LOGVAL(VE_INARG, LOGLINE(node), uses->name, "uses");
			goto error;
		}

	} else {
		/* in local tree hierarchy */
		for(par = parent; par; par = par->parent) {
			LY_TREE_FOR(parent->child, mnode) {
				if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
					uses->grp = (struct ly_mnode_grp *)mnode;
					break;
				}
			}
		}

		/* search in top level of the current module */
		LY_TREE_FOR(module->data, mnode) {
			if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
				uses->grp = (struct ly_mnode_grp *)mnode;
				break;
			}
		}
	}

	if (parent) {
		ly_mnode_addchild(parent, retval);
	}

	if (!resolve) {
		/* this is uses statement inside the grouping, so do not bound grouping
		 * to the current content
		 */
		return retval;
	}

	/* TODO */

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

/* common code for yin_read_module() and yin_read_submodule() */
static int read_sub_module(struct ly_module *module, struct lyxml_elem *yin)
{
	struct ly_ctx *ctx = module->ctx;
	struct ly_submodule *submodule = (struct ly_submodule *)module;
	struct lyxml_elem *next, *node, *child, root = {0};
	struct ly_mnode *mnode = NULL;
	const char *value;
	int c_imp = 0, c_rev = 0, c_tpdf = 0, c_ident = 0, c_inc = 0; /* counters */
	int r;
	int i;

	/*
	 * in the first run, we process elements with cardinality of 1 or 0..1 and
	 * count elements with cardinality 0..n. Data elements (choices, containers,
	 * leafs, lists, leaf-lists) are moved aside to be processed last, since we
	 * need have all top-level and groupings already prepared at that time. In
	 * the middle loop, we process other elements with carinality of 0..n since
	 * we need to allocate arrays to store them.
	 */
	LY_TREE_FOR_SAFE(yin->child, next, node) {
		if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
			lyxml_free_elem(ctx, node);
			continue;
		}

		if (!module->type && !strcmp(node->name, "namespace")) {
			if (module->ns) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			GETVAL(value, node, "uri");
			module->ns = lydict_insert(ctx, value, strlen(value));
			lyxml_free_elem(ctx, node);
		} else if (!module->type && !strcmp(node->name, "prefix")) {
			if (module->prefix) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			GETVAL(value, node, "value");
			if (check_identifier(value, LY_IDENT_PREFIX, LOGLINE(node), module, NULL)) {
				goto error;
			}
			module->prefix = lydict_insert(ctx, value, strlen(value));
			lyxml_free_elem(ctx, node);
		} else if (module->type && !strcmp(node->name, "belongs-to")) {
			if (submodule->belongsto) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			GETVAL(value, node, "module");
			submodule->belongsto = ly_ctx_get_module(module->ctx, value, NULL, 0);
			if (!submodule->belongsto) {
				LOGVAL(VE_INARG, LOGLINE(node), value, node->name);
				goto error;
			}
			/* get the prefix substatement, start with checks */
			if (!node->child) {
				LOGVAL(VE_MISSSTMT2, LOGLINE(node), "prefix", node->name);
				goto error;
			} else if (strcmp(node->child->name, "prefix")) {
				LOGVAL(VE_INSTMT, LOGLINE(node->child), node->child->name);
				goto error;
			} else if (node->child->next) {
				LOGVAL(VE_INSTMT, LOGLINE(node->child->next), node->child->next->name);
				goto error;
			}
			/* and now finally get the value */
			GETVAL(value, node->child, "value");
			/* check here differs from a generic prefix check, since this prefix
			 * don't have to be unique
			 */
			if (check_identifier(value, LY_IDENT_NAME, LOGLINE(node->child), NULL, NULL)) {
				goto error;
			}
			module->prefix = lydict_insert(ctx, value, strlen(value));

			/* we are done with belongs-to */
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "import")) {
			c_imp++;
		} else if (!strcmp(node->name, "revision")) {
			c_rev++;
		} else if (!strcmp(node->name, "typedef")) {
			c_tpdf++;
		} else if (!strcmp(node->name, "identity")) {
			c_ident++;
		} else if (!strcmp(node->name, "include")) {
			c_inc++;

		/* data statements */
		} else if (!strcmp(node->name, "container") ||
				!strcmp(node->name, "leaf-list") ||
				!strcmp(node->name, "leaf") ||
				!strcmp(node->name, "list") ||
				!strcmp(node->name, "choice") ||
				!strcmp(node->name, "uses") ||
				!strcmp(node->name, "grouping")) {
			lyxml_unlink_elem(node);
			lyxml_add_child(&root, node);

		/* optional statements */
		} else if (!strcmp(node->name, "description")) {
			if (module->dsc) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			module->dsc = read_yin_text(ctx, node);
			lyxml_free_elem(ctx, node);
			if (!module->dsc) {
				goto error;
			}
		} else if (!strcmp(node->name, "reference")) {
			if (module->ref) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			module->ref = read_yin_text(ctx, node);
			lyxml_free_elem(ctx, node);
			if (!module->ref) {
				goto error;
			}
		} else if (!strcmp(node->name, "organization")) {
			if (module->org) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			module->org = read_yin_text(ctx, node);
			lyxml_free_elem(ctx, node);
			if (!module->org) {
				goto error;
			}
		} else if (!strcmp(node->name, "contact")) {
			if (module->contact) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			module->contact = read_yin_text(ctx, node);
			lyxml_free_elem(ctx, node);
			if (!module->contact) {
				goto error;
			}
		} else if (!strcmp(node->name, "yang-version")) {
			/* TODO: support YANG 1.1 ? */
			if (module->version) {
				LOGVAL(VE_TOOMANY, LOGLINE(node), node->name, yin->name);
				goto error;
			}
			GETVAL(value, node, "value");
			if (strcmp(value, "1")) {
				LOGVAL(VE_INARG, LOGLINE(node), value, "yang-version");
				goto error;
			}
			module->version = 1;
			lyxml_free_elem(ctx, node);
#if 0
		} else {
			LOGVAL(VE_INSTMT, LOGLINE(node), node->name);
			goto error;
#endif
		}
	}

	if (!submodule) {
		/* check for mandatory statements */
		if (!module->ns) {
			LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "namespace", "module");
			goto error;
		}
		if (!module->prefix) {
			LOGVAL(VE_MISSSTMT2, LOGLINE(yin), "prefix", "module");
			goto error;
		}
	}

	/* allocate arrays for elements with cardinality of 0..n */
	if (c_imp) {
		module->imp = calloc(c_imp, sizeof *module->imp);
	}
	if (c_rev) {
		module->rev = calloc(c_rev, sizeof *module->rev);
	}
	if (c_tpdf) {
		module->tpdf = calloc(c_tpdf, sizeof *module->tpdf);
	}
	if (c_ident) {
		module->ident = calloc(c_ident, sizeof *module->ident);
	}
	if (c_inc) {
		module->inc = calloc(c_inc, sizeof *module->inc);
	}

	/* middle part - process nodes with cardinality of 0..n except the data nodes */
	LY_TREE_FOR_SAFE(yin->child, next, node) {
		if (!strcmp(node->name, "import")) {
			r = fill_yin_import(module, node, &module->imp[module->imp_size]);
			module->imp_size++;
			if (r) {
				goto error;
			}

			/* check duplicities in imported modules */
			for (i = 0; i < module->imp_size - 1; i++) {
				if (!strcmp(module->imp[i].module->name, module->imp[module->imp_size - 1].module->name)) {
					LOGVAL(VE_SPEC, LOGLINE(node), "Importing module \"%s\" repeatedly.", module->imp[i].module->name);
					goto error;
				}
			}
		} else if (!strcmp(node->name, "include")) {
			r = fill_yin_include(module, node, &module->inc[module->inc_size]);
			module->inc_size++;
			if (r) {
				goto error;
			}

			/* check duplications in include submodules */
			for (i = 0; i < module->inc_size - 1; i++) {
				if (!strcmp(module->inc[i].submodule->name, module->inc[module->inc_size - 1].submodule->name)) {
					LOGVAL(VE_SPEC, LOGLINE(node), "Importing module \"%s\" repeatedly.", module->inc[i].submodule->name);
					goto error;
				}
			}
		} else if (!strcmp(node->name, "revision")) {
			GETVAL(value, node, "date");
			if (check_date(value, LOGLINE(node))) {
				goto error;
			}
			memcpy(module->rev[module->rev_size].date, value, LY_REV_SIZE - 1);
			/* check uniqueness of the revision date - not required by RFC */
			for (i = 0; i < module->rev_size; i++) {
				if (!strcmp(value, module->rev[i].date)) {
					LOGVAL(VE_INARG, LOGLINE(node), value, node->name);
					LOGVAL(VE_SPEC, 0, "Revision is not unique.");
				}
			}

			LY_TREE_FOR(node->child, child) {
				if (!strcmp(child->name, "description")) {
					if (module->rev[module->rev_size].dsc) {
						LOGVAL(VE_TOOMANY, LOGLINE(node), child->name, node->name);
						goto error;
					}
					module->rev[module->rev_size].dsc = read_yin_text(ctx, child);
					if (!module->rev[module->rev_size].dsc) {
						goto error;
					}
				} else if (!strcmp(child->name, "reference")) {
					if (module->rev[module->rev_size].ref) {
						LOGVAL(VE_TOOMANY, LOGLINE(node), child->name, node->name);
						goto error;
					}
					module->rev[module->rev_size].ref = read_yin_text(ctx, child);
					if (!module->rev[module->rev_size].ref) {
						goto error;
					}
				} else {
					LOGVAL(VE_INSTMT, LOGLINE(child), child->name);
					goto error;
				}
			}

			/* keep the latest revision at position 0 */
			if (module->rev_size && strcmp(module->rev[module->rev_size].date, module->rev[0].date) > 0) {
				/* switch their position */
				value = strdup(module->rev[0].date);
				memcpy(module->rev[0].date, module->rev[module->rev_size].date, LY_REV_SIZE - 1);
				memcpy(module->rev[module->rev_size].date, value, LY_REV_SIZE - 1);
				free((char*)value);

				if (module->rev[0].dsc != module->rev[module->rev_size].dsc) {
					value = module->rev[0].dsc;
					module->rev[0].dsc = module->rev[module->rev_size].dsc;
					module->rev[module->rev_size].dsc = value;
				}

				if (module->rev[0].ref != module->rev[module->rev_size].ref) {
					value = module->rev[0].ref;
					module->rev[0].ref = module->rev[module->rev_size].ref;
					module->rev[module->rev_size].ref = value;
				}
			}

			module->rev_size++;
		} else if (!strcmp(node->name, "typedef")) {
			r = fill_yin_typedef(module, NULL, node, &module->tpdf[module->tpdf_size]);
			module->tpdf_size++;

			if (r) {
				goto error;
			}
		} else if (!strcmp(node->name, "identity")) {
			r = fill_yin_identity(module, node, &module->ident[module->ident_size]);
			module->ident_size++;

			if (r) {
				goto error;
			}
		}

		lyxml_free_elem(ctx, node);
	}

	/* last part - process data nodes */
	LY_TREE_FOR_SAFE(root.child, next, node) {

		if (!strcmp(node->name, "container")) {
			mnode = read_yin_container(module, NULL, node);
		} else if (!strcmp(node->name, "leaf-list")) {
			mnode = read_yin_leaflist(module, NULL, node);
		} else if (!strcmp(node->name, "leaf")) {
			mnode = read_yin_leaf(module, NULL, node);
		} else if (!strcmp(node->name, "list")) {
			mnode = read_yin_list(module, NULL, node);
		} else if (!strcmp(node->name, "choice")) {
			mnode = read_yin_choice(module, NULL, node);
		} else if (!strcmp(node->name, "grouping")) {
			mnode = read_yin_grouping(module, NULL, node);
		} else if (!strcmp(node->name, "uses")) {
			mnode = read_yin_uses(module, NULL, node, 1);
		} else {
			/* TODO error */
			continue;
		}
		lyxml_free_elem(ctx, node);

		if (!mnode) {
			goto error;
		}

		/* include data element */
		if (module->data) {
			module->data->prev->next = mnode;
			mnode->prev = module->data->prev;
			module->data->prev = mnode;
		} else {
			module->data = mnode;
		}
	}

	return EXIT_SUCCESS;

error:
	/* cleanup */
	while (root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}

	return EXIT_FAILURE;
}

struct ly_submodule *yin_read_submodule(struct ly_module *module, const char *data)
{
	struct lyxml_elem *yin;
	struct ly_submodule *submodule = NULL;
	const char *value;

	assert(module->ctx);

	yin = lyxml_read(module->ctx, data, 0);
	if (!yin) {
		return NULL;
	}

	/* check root element */
	if (!yin->name || strcmp(yin->name, "submodule")) {
		LOGVAL(VE_INSTMT, LOGLINE(yin), yin->name);
		goto error;
	}

	GETVAL(value, yin, "name");
	if (check_identifier(value, LY_IDENT_NAME, LOGLINE(yin), NULL, NULL)) {
		goto error;
	}

	submodule = calloc(1, sizeof *submodule);
	if (!submodule) {
		LOGMEM;
		goto error;
	}

	submodule->ctx = module->ctx;
	submodule->name = lydict_insert(submodule->ctx, value, strlen(value));
	submodule->type = 1;

	LOGVRB("reading submodule %s", submodule->name);
	if (read_sub_module((struct ly_module *)submodule, yin)) {
		goto error;
	}

	/* cleanup */
	lyxml_free_elem(module->ctx, yin);

	LOGVRB("submodule %s successfully parsed", submodule->name);

	return submodule;

error:
	/* cleanup */
	lyxml_free_elem(module->ctx, yin);
	ly_submodule_free(submodule);

	return NULL;
}

struct ly_module *yin_read_module(struct ly_ctx *ctx, const char *data)
{
	struct lyxml_elem *yin;
	struct ly_module *module = NULL, **newlist = NULL;
	const char *value;
	int i;

	yin = lyxml_read(ctx, data, 0);
	if (!yin) {
		return NULL;
	}

	/* check root element */
	if (!yin->name || strcmp(yin->name, "module")) {
		LOGVAL(VE_INSTMT, LOGLINE(yin), yin->name);
		goto error;
	}

	GETVAL(value, yin, "name");
	if (check_identifier(value, LY_IDENT_NAME, LOGLINE(yin), NULL, NULL)) {
		goto error;
	}

	module = calloc(1, sizeof *module);
	if (!module) {
		LOGMEM;
		goto error;
	}

	module->ctx = ctx;
	module->name = lydict_insert(ctx, value, strlen(value));
	module->type = 0;

	LOGVRB("reading module %s", module->name);
	if (read_sub_module(module, yin)) {
		goto error;
	}

	/* add to the context's list of modules */
	if (ctx->models.used == ctx->models.size) {
		newlist = realloc(ctx->models.list, ctx->models.size * 2);
		if (!newlist) {
			LOGMEM;
			goto error;
		}
		for (i = ctx->models.size; i < ctx->models.size * 2; i++) {
			newlist[i] = NULL;
		}
		ctx->models.size *= 2;
		ctx->models.list = newlist;
	}
	for (i = 0; ctx->models.list[i]; i++) {
		/* check name (name/revision) and namespace uniqueness */
		if (!strcmp(ctx->models.list[i]->name, module->name)) {
			if (!ctx->models.list[i]->rev_size && !module->rev_size) {
				/* both data models are same, with no revision specified */
				LOGERR(LY_EINVAL, "Module \"%s\" (no revision in either of them specified) already in context.", module->name);
				goto error;
			} else if (!ctx->models.list[i]->rev_size || !module->rev_size) {
				/* one of the models does not have a revision, so they differs */
				continue;
			} else {
				/* both models have a revision statement which we have to
				 * compare, revision at position 0 is the last revision
				 */
				if (!strcmp(ctx->models.list[i]->rev[0].date, module->rev[0].date)) {
					/* we have the same modules */
					LOGERR(LY_EINVAL, "Module \"%s\", revision %s already in context.", module->name, module->rev[0].date);
					goto error;
				}
			}
		} else if (!strcmp(ctx->models.list[i]->ns, module->ns)) {
			LOGERR(LY_EINVAL, "Two different modules (\"%s\" and \"%s\") have the same namespace \"%s\"",
					ctx->models.list[i]->name, module->name, module->ns);
			goto error;
		}
	}
	ctx->models.list[i] = module;
	ctx->models.used++;

	/* cleanup */
	lyxml_free_elem(ctx, yin);

	LOGVRB("module %s successfully parsed", module->name);

	return module;

error:
	/* cleanup */
	lyxml_free_elem(ctx, yin);
	ly_module_free(module);

	return NULL;
}
