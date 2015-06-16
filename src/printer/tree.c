/**
 * @file printer/tree.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief TREE printer for libyang data model structure
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../common.h"
#include "../tree.h"

/* spec_config = 0 (no special config status), 1 (read-only - rpc output, notification), 2 (write-only - rpc input) */
static void tree_print_mnode_choice(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode,
							 int mask, int spec_config);
static void tree_print_mnode(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode,
                             int mask, int spec_config);

static int sibling_is_valid_child(const struct ly_mnode *mnode)
{
	struct ly_mnode *cur;

    if (mnode == NULL) {
        return 0;
    }

	/* has a following printed child */
	LY_TREE_FOR((struct ly_mnode *)mnode->next, cur) {
		if (cur->nodetype & (LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
            LY_NODE_ANYXML | LY_NODE_CHOICE | LY_NODE_RPC | LY_NODE_INPUT | LY_NODE_OUTPUT | LY_NODE_NOTIF)) {
			return 1;
		}
	}

	/* if in uses, the following printed child can actually be in the parent node :-/ */
    if (mnode->parent && mnode->parent->nodetype == LY_NODE_USES) {
        return sibling_is_valid_child(mnode->parent);
    }

	return 0;
}

static char *create_indent(int level, const char *old_indent, const struct ly_mnode *mnode, int shorthand)
{
	int next_is_case = 0, is_case = 0, has_next = 0, i, found;
	char *new_indent = malloc((level*4+1)*sizeof(char));

	strcpy(new_indent, old_indent);

	/* this is the indent of a case (standard or shorthand) */
	if (mnode->nodetype == LY_NODE_CASE || shorthand) {
		is_case = 1;
	}

	/* this is the direct child of a case */
	if (!is_case && mnode->parent && mnode->parent->nodetype & (LY_NODE_CASE | LY_NODE_CHOICE)) {
		/* it is not the only child */
		if (mnode->next && mnode->next->parent && mnode->next->parent->nodetype == LY_NODE_CHOICE) {
			next_is_case = 1;
		}
	}

    /* next is a node that will actually be printed */
    has_next = sibling_is_valid_child(mnode);

    /* there is no next, but we are in top-level of a submodule */
    if (!has_next && mnode->module->type == 1 && !mnode->parent) {
        struct ly_submodule *submod = (struct ly_submodule *)mnode->module;
        struct ly_module *mod = submod->belongsto;

        /* find this submodule, check all the next ones for valid printed nodes */
        found = 0;
        for (i = 0; i < mod->inc_size; i++) {
            /* we found ours, check all the following submodules and the module */
            if (found) {
                if (mnode->nodetype == LY_NODE_RPC) {
                    has_next = sibling_is_valid_child(mod->inc[i].submodule->rpc);
                } else if (mnode->nodetype == LY_NODE_NOTIF) {
                    has_next = sibling_is_valid_child(mod->inc[i].submodule->notif);
                } else {
                    has_next = sibling_is_valid_child(mod->inc[i].submodule->data);
                }
            }

            if (!found && !strcmp(submod->name, mod->inc[i].submodule->name)) {
                found = 1;
            }
        }

        /* there is nothing in submodules, check module */
        if (!has_next) {
            if (mnode->nodetype == LY_NODE_RPC) {
                has_next = sibling_is_valid_child(mod->inc[i].submodule->rpc);
            } else if (mnode->nodetype == LY_NODE_NOTIF) {
                has_next = sibling_is_valid_child(mod->inc[i].submodule->notif);
            } else {
                has_next = sibling_is_valid_child(mod->inc[i].submodule->data);
            }
        }
    }

	if (has_next && !next_is_case) {
		strcat(new_indent, "|  ");
	} else {
		strcat(new_indent, "   ");
	}

	return new_indent;
}

static unsigned int get_max_name_len(struct ly_mnode *mnode)
{
	struct ly_mnode *sub;
	unsigned int max_name_len = 0, uses_max_name_len;

	LY_TREE_FOR(mnode, sub) {
		if (sub->nodetype == LY_NODE_USES) {
			uses_max_name_len = get_max_name_len(sub->child);
			if (uses_max_name_len > max_name_len) {
				max_name_len = uses_max_name_len;
			}
		} else if (sub->nodetype & (LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF |
				   LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML)) {
			if (strlen(sub->name) > max_name_len) {
				max_name_len = strlen(sub->name);
			}
		}
	}

	return max_name_len;
}

static void tree_print_type(FILE *f, struct ly_type *type)
{
	if (type->base == LY_TYPE_LEAFREF) {
		fprintf(f, "-> %s", type->info.lref.path);
	} else if (type->prefix) {
		fprintf(f, "%s:%s", type->prefix, type->der->name);
	} else {
		fprintf(f, "%s", type->der->name);
	}
}

static void tree_print_input_output(FILE *f, int level, char *indent, struct ly_mnode *mnode, int spec_config)
{
    unsigned int max_child_len;
    char *new_indent;
    struct ly_mnode *sub;

    assert(spec_config);

    fprintf(f, "%s+--%s %s\n", indent, (spec_config == 1 ? "-w" : "ro"), (spec_config == 1 ? "input" : "output"));

    level++;
    new_indent = create_indent(level, indent, mnode, 0);

    max_child_len = get_max_name_len(mnode->child);

    LY_TREE_FOR(mnode->child, sub) {
        tree_print_mnode(f, level, new_indent, max_child_len, sub, LY_NODE_CHOICE | LY_NODE_CONTAINER |
                         LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML | LY_NODE_USES, spec_config);
    }

    free(new_indent);
}

static void tree_print_container(FILE *f, int level, char *indent, struct ly_mnode *mnode, int spec_config)
{
	unsigned int max_child_len;
	char *new_indent;
	struct ly_mnode_container *cont = (struct ly_mnode_container *)mnode;
	struct ly_mnode *sub;

    assert(spec_config >= 0 && spec_config <= 2);

	fprintf(f, "%s%s--", indent, (cont->flags & LY_NODE_STATUS_DEPRC ? "x" : (cont->flags & LY_NODE_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (cont->flags & LY_NODE_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    fprintf(f, "%s%s\n", cont->name, (cont->presence ? "!" : ""));

	level++;
	new_indent = create_indent(level, indent, mnode, 0);

	max_child_len = get_max_name_len(mnode->child);

	LY_TREE_FOR(cont->child, sub) {
		tree_print_mnode(f, level, new_indent, max_child_len, sub, LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML |
						 LY_NODE_USES, spec_config);
	}

	free(new_indent);
}

static void tree_print_choice(FILE *f, int level, char *indent, struct ly_mnode *mnode, int spec_config)
{
	unsigned int max_child_len;
	char *new_indent;
	struct ly_mnode_choice *choice = (struct ly_mnode_choice *)mnode;
	struct ly_mnode *sub;

    assert(spec_config >= 0 && spec_config <= 2);

	fprintf(f, "%s%s--", indent, (choice->flags & LY_NODE_STATUS_DEPRC ? "x" : (choice->flags & LY_NODE_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (choice->flags & LY_NODE_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    fprintf(f, "(%s)%s", choice->name, (choice->flags & LY_NODE_MAND_TRUE ? "" : "?"));

	if (choice->dflt != NULL) {
		fprintf(f, " <%s>", choice->dflt->name);
	}
	fprintf(f, "\n");

	level++;
	new_indent = create_indent(level, indent, mnode, 0);

	max_child_len = get_max_name_len(mnode->child);

	LY_TREE_FOR(choice->child, sub) {
		tree_print_mnode_choice(f, level, new_indent, max_child_len, sub,
								LY_NODE_CASE | LY_NODE_CONTAINER |
								LY_NODE_LEAF | LY_NODE_LEAFLIST |
								LY_NODE_LIST | LY_NODE_ANYXML, spec_config);
	}

	free(new_indent);
}

static void tree_print_case(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode, int shorthand, int spec_config)
{
	char *new_indent;
	struct ly_mnode_case *cas = (struct ly_mnode_case *)mnode;
	struct ly_mnode *sub;

	fprintf(f, "%s%s--:(%s)\n", indent, (cas->flags & LY_NODE_STATUS_DEPRC ? "x" : (cas->flags & LY_NODE_STATUS_OBSLT ? "o" : "+")),
			mnode->name);

	level++;
	new_indent = create_indent(level, indent, mnode, shorthand);

	if (shorthand) {
		tree_print_mnode(f, level, new_indent, max_name_len, mnode,
						 LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
						 LY_NODE_ANYXML | LY_NODE_USES, spec_config);
	} else {
		LY_TREE_FOR(mnode->child, sub) {
			tree_print_mnode(f, level, new_indent, max_name_len, sub,
							LY_NODE_CHOICE | LY_NODE_CONTAINER |
							LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
							LY_NODE_ANYXML | LY_NODE_USES, spec_config);
		}
	}

	free(new_indent);
}

static void tree_print_anyxml(FILE *f, char *indent, unsigned int max_name_len, struct ly_mnode *mnode, int spec_config)
{
	struct ly_mnode_anyxml *anyxml = (struct ly_mnode_anyxml *)mnode;

    assert(spec_config >= 0 && spec_config <= 2);

	fprintf(f, "%s%s--", indent, (anyxml->flags & LY_NODE_STATUS_DEPRC ? "x" : (anyxml->flags & LY_NODE_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (anyxml->flags & LY_NODE_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    fprintf(f, "%s%s%*sanyxml\n", anyxml->name, (anyxml->flags & LY_NODE_MAND_TRUE ? " " : "?"),
            3+(int)(max_name_len-strlen(anyxml->name)), "   ");
}

static void tree_print_leaf(FILE *f, char *indent, unsigned int max_name_len, struct ly_mnode *mnode, int spec_config)
{
	struct ly_mnode_leaf *leaf = (struct ly_mnode_leaf *)mnode;
	struct ly_mnode_list *list;
	int i, is_key = 0;

    assert(spec_config >= 0 && spec_config <= 2);

	if (leaf->parent->nodetype == LY_NODE_LIST) {
		list = (struct ly_mnode_list *)leaf->parent;
		for (i = 0; i < list->keys_size; i++) {
			if (strcmp(list->keys[i]->name, leaf->name) == 0) {
				is_key = 1;
				break;
			}
		}
	}

	fprintf(f, "%s%s--", indent, (leaf->flags & LY_NODE_STATUS_DEPRC ? "x" : (leaf->flags & LY_NODE_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (leaf->flags & LY_NODE_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    fprintf(f, "%s%s%*s", leaf->name, (leaf->flags & LY_NODE_MAND_TRUE || is_key ? " " : "?"),
            3+(int)(max_name_len-strlen(leaf->name)), "   ");

    tree_print_type(f, &leaf->type);

	if (leaf->dflt != NULL) {
		fprintf(f, " <%s>", leaf->dflt);
	}
	fprintf(f, "\n");
}

static void tree_print_leaflist(FILE *f, char *indent, unsigned int max_name_len, struct ly_mnode *mnode, int spec_config)
{
	struct ly_mnode_leaflist *leaflist = (struct ly_mnode_leaflist *)mnode;

    assert(spec_config >= 0 && spec_config <= 2);

	fprintf(f, "%s%s--", indent, (leaflist->flags & LY_NODE_STATUS_DEPRC ? "x" : (leaflist->flags & LY_NODE_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (leaflist->flags & LY_NODE_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    fprintf(f, "%s*%*s", leaflist->name, 3+(int)(max_name_len-strlen(leaflist->name)), "   ");

	tree_print_type(f, &leaflist->type);

	fprintf(f, "\n");
}

static void tree_print_list(FILE *f, int level, char *indent, struct ly_mnode *mnode, int spec_config)
{
	int i;
	unsigned int max_child_len;
	char *new_indent;
	struct ly_mnode *sub;
	struct ly_mnode_list *list = (struct ly_mnode_list *)mnode;

	fprintf(f, "%s%s--", indent, (list->flags & LY_NODE_STATUS_DEPRC ? "x" : (list->flags & LY_NODE_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (list->flags & LY_NODE_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    fprintf(f, "%s*", list->name);

	for (i = 0; i < list->keys_size; i++) {
		if (i == 0) {
			fprintf(f, " [");
		}
		fprintf(f, "%s%s", list->keys[i]->name, i + 1 < list->keys_size ? "," : "]");
	}

	fprintf(f, "\n");

	level++;
	new_indent = create_indent(level, indent, mnode, 0);

	max_child_len = get_max_name_len(mnode->child);

	LY_TREE_FOR(mnode->child, sub) {
		tree_print_mnode(f, level, new_indent, max_child_len, sub, LY_NODE_CHOICE | LY_NODE_CONTAINER |
		                 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
						 LY_NODE_USES | LY_NODE_ANYXML, spec_config);
	}

	free(new_indent);
}

static void tree_print_uses(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode, int spec_config)
{
	struct ly_mnode *node;
	struct ly_mnode_uses *uses = (struct ly_mnode_uses *)mnode;

	LY_TREE_FOR(uses->child, node) {
		tree_print_mnode(f, level, indent, max_name_len, node,
						 LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF |
						 LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_USES | LY_NODE_ANYXML, spec_config);
	}
}

static void tree_print_rpc(FILE *f, int level, char *indent, struct ly_mnode *mnode)
{
    char *new_indent;
    struct ly_mnode *node;
    struct ly_mnode_rpc *rpc = (struct ly_mnode_rpc *)mnode;

    fprintf(f, "%s%s---x %s\n", indent, (rpc->flags & LY_NODE_STATUS_DEPRC ? "x" : (rpc->flags & LY_NODE_STATUS_OBSLT ? "o" : "+")),
            rpc->name);

    level++;
    new_indent = create_indent(level, indent, mnode, 0);

    LY_TREE_FOR(rpc->child, node) {
        if (node->nodetype == LY_NODE_INPUT) {
            tree_print_input_output(f, level, new_indent, node, 1);
        } else if (node->nodetype == LY_NODE_OUTPUT) {
            tree_print_input_output(f, level, new_indent, node, 2);
        }
    }

    free(new_indent);
}

static void tree_print_notif(FILE *f, int level, char *indent, struct ly_mnode *mnode)
{
    unsigned int max_child_len;
    char *new_indent;
    struct ly_mnode *node;
    struct ly_mnode_notif *notif = (struct ly_mnode_notif *)mnode;

    fprintf(f, "%s%s---n %s\n", indent, (notif->flags & LY_NODE_STATUS_DEPRC ? "x" : (notif->flags & LY_NODE_STATUS_OBSLT ? "o" : "+")),
            notif->name);

    level++;
    new_indent = create_indent(level, indent, mnode, 0);

    max_child_len = get_max_name_len(mnode->child);

    LY_TREE_FOR(notif->child, node) {
        tree_print_mnode(f, level, new_indent, max_child_len, node, LY_NODE_CHOICE | LY_NODE_CONTAINER |
                         LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML |
                         LY_NODE_USES, 2);
    }

    free(new_indent);
}

static void tree_print_mnode_choice(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode, int mask, int spec_config)
{
	if (mnode->nodetype & mask) {
		if (mnode->nodetype == LY_NODE_CASE) {
			tree_print_case(f, level, indent, max_name_len, mnode, 0, spec_config);
		} else {
			tree_print_case(f, level, indent, max_name_len, mnode, 1, spec_config);
		}
	}
}

static void tree_print_mnode(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode, int mask, int spec_config)
{
	switch (mnode->nodetype & mask) {
	case LY_NODE_CONTAINER:
		tree_print_container(f, level, indent, mnode, spec_config);
		break;
	case LY_NODE_CHOICE:
		tree_print_choice(f, level, indent, mnode, spec_config);
		break;
	case LY_NODE_LEAF:
		tree_print_leaf(f, indent, max_name_len, mnode, spec_config);
		break;
	case LY_NODE_LEAFLIST:
		tree_print_leaflist(f, indent, max_name_len, mnode, spec_config);
		break;
	case LY_NODE_LIST:
		tree_print_list(f, level, indent, mnode, spec_config);
		break;
	case LY_NODE_ANYXML:
		tree_print_anyxml(f, indent, max_name_len, mnode, spec_config);
		break;
	case LY_NODE_USES:
		tree_print_uses(f, level, indent, max_name_len, mnode, spec_config);
		break;
	default: break;
	}
}

int tree_print_model(FILE *f, struct ly_module *module)
{
	struct ly_mnode *mnode;
	unsigned int max_child_len;
	int level = 1, i, have_rpcs = 0, have_notifs = 0;
	char *indent = malloc((level*4+1)*sizeof(char));
	strcpy(indent, "   ");

	fprintf(f, "module: %s\n", module->name);

	/* included submodules */
	for (i = 0; i < module->inc_size; i++) {
		max_child_len = get_max_name_len(module->inc[i].submodule->data);

		LY_TREE_FOR(module->inc[i].submodule->data, mnode) {
			tree_print_mnode(f, level, indent, max_child_len, mnode, LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML |
						 LY_NODE_USES, 0);
		}
	}

	/* module */
	max_child_len = get_max_name_len(module->data);
	level++;

	LY_TREE_FOR(module->data, mnode) {
		tree_print_mnode(f, level, indent, max_child_len, mnode, LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML |
						 LY_NODE_USES, 0);
	}

	/* rpc */
	if (module->rpc) {
        have_rpcs = 1;
    } else {
        for (i = 0; i < module->inc_size; i++) {
            if (module->inc[i].submodule->rpc) {
                have_rpcs = 1;
                break;
            }
        }
    }
	if (have_rpcs) {
        fprintf(f, "rpcs:\n");
        for (i = 0; i < module->inc_size; i++) {
            LY_TREE_FOR(module->inc[i].submodule->rpc, mnode) {
                tree_print_rpc(f, level, indent, mnode);
            }
        }
        LY_TREE_FOR(module->rpc, mnode) {
            tree_print_rpc(f, level, indent, mnode);
        }
    }

    /* notification */
    if (module->notif) {
        have_notifs = 1;
    } else {
        for (i = 0; i < module->inc_size; i++) {
            if (module->inc[i].submodule->notif) {
                have_notifs = 1;
                break;
            }
        }
    }
    if (have_notifs) {
        fprintf(f, "notifications:\n");
        for (i = 0; i < module->inc_size; i++) {
            LY_TREE_FOR(module->inc[i].submodule->notif, mnode) {
                tree_print_notif(f, level, indent, mnode);
            }
        }
        LY_TREE_FOR(module->notif, mnode) {
            tree_print_notif(f, level, indent, mnode);
        }
    }

	free(indent);
	return EXIT_SUCCESS;
#undef LEVEL
}
