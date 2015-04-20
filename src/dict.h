/**
 * @file dict.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang dictionary
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

#ifndef LY_DICT_H_
#define LY_DICT_H_

char *dict_insert(const char *value, size_t len);
void dict_remove(const char *value);

#endif /* LY_DICT_H_ */
