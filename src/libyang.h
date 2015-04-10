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
