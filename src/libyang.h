/**
 * @file common.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common definitions for libyang
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

#ifndef LY_LIBYANG_H_
#define LY_LIBYANG_H_

#include <stdio.h>

#include "yin.h"

int ly_model_print(FILE *f, struct ly_module *module, LY_MFORMAT format);

/**
 * @brief libyang context handler.
 */
struct ly_ctx;

/**
 * @brief Create libyang context
 *
 * Context is used to hold all information about data models. Usually, the
 * application is supposed to work with a single context in which libyang is
 * holding all data models and other internal information according to which
 * the instance data will be processed and validated. Therefore, also each
 * instance data are connected
 *
 * @param[in] search_dir Directory where libyang will search for the imported
 * or included modules and submodules. If no such directory is available, NULL
 * is accepted.
 *
 * @retrurn Pointer to the created libyang context, NULL in case of error.
 */
struct ly_ctx *ly_ctx_new(const char *search_dir);

/**
 * @brief Free all internal structures of the specified context.
 *
 * The function should be used before terminating the application to destroy
 * and free all structures internally used by libyang. If the caller uses
 * multiple contexts, the function should be called for each used context.
 *
 * All instance data are supposed to be freed before destroying the context.
 * Data models are destroyed automatically as part of ly_ctx_destroy() call.
 *
 * @param[in] ctx libyang context to destroy
 */
void ly_ctx_destroy(struct ly_ctx *ctx);

/**
 * @brief Get pointer to the data model structure of the specified name.
 *
 * If the module is not yet loaded in the context, libyang tries to find it in
 * the search directory specified when the context was created by ly_ctx_new().
 *
 * @param[in] ctx Context to work in.
 * @param[in] name Name of the YANG module to get.
 * @param[in] revision Optional revision date of the YANG module to get. If not
 * specified, the newest revision is returned.
 */
struct ly_module *ly_ctx_get_model(struct ly_ctx *ctx, const char *name,
                                   const char *revision);

/**
 * @defgroup libyang libyang
 * @{
 */

/**
 * @brief Load a data model into the specified context.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified
 * format
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct ly_module *ly_model_read(struct ly_ctx *ctx, const char *data,
                                LY_MFORMAT format);


struct ly_module *ly_model_read_fd(struct ly_ctx *ctx, int fd,
                                   LY_MFORMAT format);

/**
 * @brief Free data model
 *
 * It is up to the caller that there is no instance data using the module
 * being freed.
 *
 * @param[in] module Data model to free.
 */
void ly_model_free(struct ly_module *module);

/**@}*/

/**
 * @page howto How To ...
 *
 * - @subpage howtologger
 */

/**
 * @defgroup logger libyang logger
 *
 * @page howtologger Logger
 *
 * There are 4 verbosity levels defined as ::LY_VERB_LEVEL. The level can be
 * changed by the ly_verbosity() function. By default, the verbosity level is
 * set to #LY_VERB_ERR value;
 *
 */

/**
 * @typedef LY_VERB_LEVEL
 * @brief Verbosity levels.
 * @ingroup logger
 */
typedef enum {
	LY_VERB_ERR,  /**< Print only error messages. */
	LY_VERB_WRN,  /**< Print error and warning messages. */
	LY_VERB_VRB,  /**< Besides errors and warnings, print some other verbose messages. */
	LY_VERB_DBG   /**< Print all messages including some development debug messages. */
} LY_VERB_LEVEL;

/**
 *
 */
void ly_verbosity(LY_VERB_LEVEL level);


#endif /* LY_LIBYANG_H_ */
