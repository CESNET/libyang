/**
 * @file validation.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data tree validation for libyang
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
 */

#ifndef LY_VALIDATION_H_
#define LY_VALIDATION_H_

#include "libyang.h"
#include "tree.h"

/**
 * @brief Check that the list data node contains all defined keys.
 *
 * Does not log.
 *
 * @param[in] list The list data node to check
 * @return NULL on success or pointer to the first missing key schema node
 */
struct lys_node_leaf * lyv_keys_present(struct lyd_node_list *list);

#endif /* LY_VALIDATION_H_ */
