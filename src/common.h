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

#ifndef LY_COMMON_H_
#define LY_COMMON_H_

#include <stdint.h>

#include "libyang.h"

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

/*
 * If the compiler supports attribute to mark objects as hidden, mark all
 * objects as hidden and export only objects explicitly marked to be part of
 * the public API.
 */
#define API __attribute__((visibility("default")))

/*
 * logger
 */
extern volatile uint8_t ly_verb_level;
void ly_log(LY_VERB_LEVEL level, int errno_, const char *format, ...);
#define LY_ERR(errno,str,args...) ly_log(LY_VERB_ERR,errno,str,##args)
#define LY_WRN(format,args...) if(ly_verb_level>=LY_VERB_WRN){ly_log(LY_VERB_WRN,format,##args);}
#define LY_VRB(format,args...) if(ly_verb_level>=LY_VERB_VRB){ly_log(LY_VERB_VRB,format,##args);}
#define LY_DBG(format,args...) if(ly_verb_level>=LY_VERB_DBG){ly_log(LY_VERB_DBG,format,##args);}

/*
 * libyang's errno
 */
extern int ly_errno;

#define	LY_EINVAL   1 /* Invalid argument */


#endif /* LY_COMMON_H_ */
