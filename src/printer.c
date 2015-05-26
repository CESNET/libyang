/**
 * @file printer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Wrapper for all libyang printers.
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

#include "common.h"
#include "tree.h"

/* printer/yang.c */
int yang_print_model(FILE *f, struct ly_module *module);


API int ly_model_print(FILE *f, struct ly_module *module, LY_MFORMAT format)
{
	if (!f || !module) {
		ly_errno = LY_EINVAL;
		return EXIT_FAILURE;
	}

	switch(format) {
	case LY_YIN:
		LY_ERR(LY_EINVAL, "YIN output format not supported yet.");
		return EXIT_FAILURE;
	case LY_YANG:
		return yang_print_model(f, module);
	default:
		LY_ERR(LY_EINVAL, "Unknown output format.");
		return EXIT_FAILURE;
	}
}
