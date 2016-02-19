/**
 * @file printer.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Printers for libyang
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

#ifndef LY_PRINTER_H_
#define LY_PRINTER_H_

#include "libyang.h"
#include "tree_schema.h"
#include "tree_internal.h"

typedef enum LYOUT_TYPE {
    LYOUT_FD,          /**< file descriptor */
    LYOUT_STREAM,      /**< FILE stream */
    LYOUT_MEMORY,      /**< memory */
    LYOUT_CALLBACK     /**< print via provided callback */
} LYOUT_TYPE;

struct lyout {
    LYOUT_TYPE type;
    union {
        int fd;
        FILE *f;
        struct {
            char *buf;
            size_t len;
            size_t size;
        } mem;
        struct {
            ssize_t (*f)(void *arg, const void *buf, size_t count);
            void *arg;
        } clb;
    } method;
};

/**
 * @brief Generic printer, replacement for printf() / write() / etc
 */
int ly_print(struct lyout *out, const char *format, ...);
int ly_write(struct lyout *out, const char *buf, size_t count);

int yang_print_model(struct lyout *out, const struct lys_module *module);
int yin_print_model(struct lyout *out, const struct lys_module *module);
int tree_print_model(struct lyout *out, const struct lys_module *module);
int info_print_model(struct lyout *out, const struct lys_module *module, const char *target_node);

int json_print_data(struct lyout *out, const struct lyd_node *root, int options);
int xml_print_data(struct lyout *out, const struct lyd_node *root, int format, int options);

/* 0 - same, 1 - different */
int nscmp(const struct lyd_node *node1, const struct lyd_node *node2);

#endif /* LY_PRINTER_H_ */
