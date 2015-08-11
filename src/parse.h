/**
 * @file parse.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief parsing functions header
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

#ifndef _PARSE_H
#define _PARSE_H

#include "libyang.h"

int check_length_range(const char *expr, struct lys_type *type);

int parse_path_predicate(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                         const char **path_key_expr, int *pke_len, int *has_predicate);

int parse_path_key_expr(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                        int *parent_times);

int parse_path_arg(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                   int *parent_times, int *has_predicate);

int parse_instance_identifier(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                              int *has_predicate);

int parse_predicate(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                    const char **value, int *val_len, int *has_predicate);

int parse_schema_nodeid(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                        int *first_call);

#endif /* _PARSE_H */
