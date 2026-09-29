/**
 * @file ietf.h
 * @author Petr Hanzlik <Petr.Hanzlik@cesnet.cz>
 * @brief yanglint ietf header
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef IETF_H_
#define IETF_H_

#include "libyang.h"
#include "yl_opt.h"

/**
 * @brief Validate a module against the IETF requirements.
 *
 * @param[in,out] ctx A double pointer to the libyang context containing the loaded modules.
 * @param[in,out] yo A pointer to the yanglint options structure, which manages global
 * @param[in] posv The positional command-line argument provided by the user, representing the target module name (e.g., "ietf-interfaces" or "ietf-interfaces@2014-05-08").
 * @return 0 on successful execution of the validation pipeline (note: this returns 0 even if IETF compliance warnings/errors were printed to the output).
 * @return 1 on fatal execution errors (e.g., module not found, module failed to compile, or memory allocation failure).
 */
int yl_validate_ietf(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);

#endif /* IETF_H_ */
