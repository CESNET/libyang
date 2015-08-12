/**
 * @file validation.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data tree validation functions
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

#include "libyang.h"

struct lys_node_leaf *
lyv_keys_present(struct lyd_node_list *list)
{
    struct lyd_node *aux;
    struct lys_node_list *schema;
    int i;

    schema = (struct lys_node_list *)list->schema;

    for (i = 0; i < schema->keys_size; i++) {
        for (aux = list->child; aux; aux = aux->next) {
            if (aux->schema == (struct lys_node *)schema->keys[i]) {
                break;
            }
        }
        if (!aux) {
            /* key not found in the data */
            return schema->keys[i];
        }
    }

    return EXIT_SUCCESS;
}
