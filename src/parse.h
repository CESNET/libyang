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
