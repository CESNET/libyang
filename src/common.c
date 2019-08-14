/**
 * @file common.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief common internal definitions for libyang
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "extensions.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

const char *const ly_stmt_list[] = {
    [YANG_ACTION] = "action",
    [YANG_ANYDATA] = "anydata",
    [YANG_ANYXML] = "anyxml",
    [YANG_ARGUMENT] = "argument",
    [YANG_AUGMENT] = "augment",
    [YANG_BASE] = "base",
    [YANG_BELONGS_TO] = "belongs-to",
    [YANG_BIT] = "bit",
    [YANG_CASE] = "case",
    [YANG_CHOICE] = "choice",
    [YANG_CONFIG] = "config",
    [YANG_CONTACT] = "contact",
    [YANG_CONTAINER] = "container",
    [YANG_CUSTOM] = "<extension-instance>",
    [YANG_DEFAULT] = "default",
    [YANG_DESCRIPTION] = "description",
    [YANG_DEVIATE] = "deviate",
    [YANG_DEVIATION] = "deviation",
    [YANG_ENUM] = "enum",
    [YANG_ERROR_APP_TAG] = "error-app-tag",
    [YANG_ERROR_MESSAGE] = "error-message",
    [YANG_EXTENSION] = "extension",
    [YANG_FEATURE] = "feature",
    [YANG_FRACTION_DIGITS] = "fraction-digits",
    [YANG_GROUPING] = "grouping",
    [YANG_IDENTITY] = "identitiy",
    [YANG_IF_FEATURE] = "if-feature",
    [YANG_IMPORT] = "import",
    [YANG_INCLUDE] = "include",
    [YANG_INPUT] = "input",
    [YANG_KEY] = "key",
    [YANG_LEAF] = "leaf",
    [YANG_LEAF_LIST] = "leaf-list",
    [YANG_LENGTH] = "length",
    [YANG_LIST] = "list",
    [YANG_MANDATORY] = "mandatory",
    [YANG_MAX_ELEMENTS] = "max-elements",
    [YANG_MIN_ELEMENTS] = "min-elements",
    [YANG_MODIFIER] = "modifier",
    [YANG_MODULE] = "module",
    [YANG_MUST] = "must",
    [YANG_NAMESPACE] = "namespace",
    [YANG_NOTIFICATION] = "notification",
    [YANG_ORDERED_BY] = "ordered-by",
    [YANG_ORGANIZATION] = "organization",
    [YANG_OUTPUT] = "output",
    [YANG_PATH] = "path",
    [YANG_PATTERN] = "pattern",
    [YANG_POSITION] = "position",
    [YANG_PREFIX] = "prefix",
    [YANG_PRESENCE] = "presence",
    [YANG_RANGE] = "range",
    [YANG_REFERENCE] = "reference",
    [YANG_REFINE] = "refine",
    [YANG_REQUIRE_INSTANCE] = "require-instance",
    [YANG_REVISION] = "revision",
    [YANG_REVISION_DATE] = "revision-date",
    [YANG_RPC] = "rpc",
    [YANG_STATUS] = "status",
    [YANG_SUBMODULE] = "submodule",
    [YANG_TYPE] = "type",
    [YANG_TYPEDEF] = "typedef",
    [YANG_UNIQUE] = "unique",
    [YANG_UNITS] = "units",
    [YANG_USES] = "uses",
    [YANG_VALUE] = "value",
    [YANG_WHEN] = "when",
    [YANG_YANG_VERSION] = "yang-version",
    [YANG_YIN_ELEMENT] = "yin-element",
    [YANG_SEMICOLON] = ";",
    [YANG_LEFT_BRACE] = "{",
    [YANG_RIGHT_BRACE] = "}",
};

const char *const lyext_substmt_list[] = {
    [LYEXT_SUBSTMT_ARGUMENT] = "argument",
    [LYEXT_SUBSTMT_BASE] = "base",
    [LYEXT_SUBSTMT_BELONGSTO] = "belongs-to",
    [LYEXT_SUBSTMT_CONTACT] = "contact",
    [LYEXT_SUBSTMT_DEFAULT] = "default",
    [LYEXT_SUBSTMT_DESCRIPTION] = "description",
    [LYEXT_SUBSTMT_ERRTAG] = "error-app-tag",
    [LYEXT_SUBSTMT_ERRMSG] = "error-message",
    [LYEXT_SUBSTMT_KEY] = "key",
    [LYEXT_SUBSTMT_NAMESPACE] = "namespace",
    [LYEXT_SUBSTMT_ORGANIZATION] = "organization",
    [LYEXT_SUBSTMT_PATH] = "path",
    [LYEXT_SUBSTMT_PREFIX] = "prefix",
    [LYEXT_SUBSTMT_PRESENCE] = "presence",
    [LYEXT_SUBSTMT_REFERENCE] = "reference",
    [LYEXT_SUBSTMT_REVISIONDATE] = "revision-date",
    [LYEXT_SUBSTMT_UNITS] = "units",
    [LYEXT_SUBSTMT_VALUE] = "value",
    [LYEXT_SUBSTMT_VERSION] = "yang-version",
    [LYEXT_SUBSTMT_MODIFIER] = "modifier",
    [LYEXT_SUBSTMT_REQINSTANCE] = "require-instance",
    [LYEXT_SUBSTMT_YINELEM] = "yin-element",
    [LYEXT_SUBSTMT_CONFIG] = "config",
    [LYEXT_SUBSTMT_MANDATORY] = "mandatory",
    [LYEXT_SUBSTMT_ORDEREDBY] = "ordered-by",
    [LYEXT_SUBSTMT_STATUS] = "status",
    [LYEXT_SUBSTMT_FRACDIGITS] = "fraction-digits",
    [LYEXT_SUBSTMT_MAX] = "max-elements",
    [LYEXT_SUBSTMT_MIN] = "min-elements",
    [LYEXT_SUBSTMT_POSITION] = "position",
    [LYEXT_SUBSTMT_UNIQUE] = "unique",
    [LYEXT_SUBSTMT_IFFEATURE] = "if-feature",
};

const char *const ly_devmod_list[] = {
    [LYS_DEV_NOT_SUPPORTED] = "not-supported",
    [LYS_DEV_ADD] = "add",
    [LYS_DEV_DELETE] = "delete",
    [LYS_DEV_REPLACE] = "replace",
};

void *
ly_realloc(void *ptr, size_t size)
{
    void *new_mem;

    new_mem = realloc(ptr, size);
    if (!new_mem) {
        free(ptr);
    }

    return new_mem;
}

LY_ERR
ly_getutf8(const char **input, unsigned int *utf8_char, size_t *bytes_read)
{
    unsigned int c, len;
    int aux;
    int i;

    if (bytes_read) {
        (*bytes_read) = 0;
    }

    c = (*input)[0];
    LY_CHECK_RET(!c, LY_EINVAL);

    if (!(c & 0x80)) {
        /* one byte character */
        len = 1;

        if (c < 0x20 && c != 0x9 && c != 0xa && c != 0xd) {
            return LY_EINVAL;
        }
    } else if ((c & 0xe0) == 0xc0) {
        /* two bytes character */
        len = 2;

        aux = (*input)[1];
        if ((aux & 0xc0) != 0x80) {
            return LY_EINVAL;
        }
        c = ((c & 0x1f) << 6) | (aux & 0x3f);

        if (c < 0x80) {
            return LY_EINVAL;
        }
    } else if ((c & 0xf0) == 0xe0) {
        /* three bytes character */
        len = 3;

        c &= 0x0f;
        for (i = 1; i <= 2; i++) {
            aux = (*input)[i];
            if ((aux & 0xc0) != 0x80) {
                return LY_EINVAL;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if (c < 0x800 || (c > 0xd7ff && c < 0xe000) || c > 0xfffd) {
            return LY_EINVAL;
        }
    } else if ((c & 0xf8) == 0xf0) {
        /* four bytes character */
        len = 4;

        c &= 0x07;
        for (i = 1; i <= 3; i++) {
            aux = (*input)[i];
            if ((aux & 0xc0) != 0x80) {
                return LY_EINVAL;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if (c < 0x1000 || c > 0x10ffff) {
            return LY_EINVAL;
        }
    } else {
        return LY_EINVAL;
    }

    (*utf8_char) = c;
    (*input) += len;
    if (bytes_read) {
        (*bytes_read) = len;
    }
    return LY_SUCCESS;
}

/**
 * @brief Static table of the UTF8 characters lengths according to their first byte.
 */
static const unsigned char
utf8_char_length_table[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
};

size_t
ly_utf8len(const char *str, size_t bytes)
{
    size_t len;
    const char *ptr;

    for (len = 0, ptr = str; *ptr && (size_t)(ptr - str) < bytes; ++len, ptr += utf8_char_length_table[((unsigned char)(*ptr))]);
    return len;
}

size_t
LY_VCODE_INSTREXP_len(const char *str)
{
    size_t len = 0;
    if (!str) {
        return len;
    } else if (!str[0]) {
        return 1;
    }
    for (len = 1; len < LY_VCODE_INSTREXP_MAXLEN && str[len]; ++len);
    return len;
}

LY_ERR
ly_mmap(struct ly_ctx *ctx, int fd, size_t *length, void **addr)
{
    struct stat sb;
    long pagesize;
    size_t m;

    assert(length);
    assert(addr);
    assert(fd >= 0);

    if (fstat(fd, &sb) == -1) {
        LOGERR(ctx, LY_ESYS, "Failed to stat the file descriptor (%s) for the mmap().", strerror(errno));
        return LY_ESYS;
    }
    if (!S_ISREG(sb.st_mode)) {
        LOGERR(ctx, LY_EINVAL, "File to mmap() is not a regular file.");
        return LY_ESYS;
    }
    if (!sb.st_size) {
        *addr = NULL;
        return LY_SUCCESS;
    }
    pagesize = sysconf(_SC_PAGESIZE);

    m = sb.st_size % pagesize;
    if (m && pagesize - m >= 1) {
        /* there will be enough space (at least 1 byte) after the file content mapping to provide zeroed NULL-termination byte */
        *length = sb.st_size + 1;
        *addr = mmap(NULL, *length, PROT_READ, MAP_PRIVATE, fd, 0);
    } else {
        /* there will not be enough bytes after the file content mapping for the additional bytes and some of them
         * would overflow into another page that would not be zerroed and any access into it would generate SIGBUS.
         * Therefore we have to do the following hack with double mapping. First, the required number of bytes
         * (including the additinal bytes) is required as anonymous and thus they will be really provided (actually more
         * because of using whole pages) and also initialized by zeros. Then, the file is mapped to the same address
         * where the anonymous mapping starts. */
        *length = sb.st_size + pagesize;
        *addr = mmap(NULL, *length, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        *addr = mmap(*addr, sb.st_size, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, 0);
    }
    if (*addr == MAP_FAILED) {
        LOGERR(ctx, LY_ESYS, "mmap() failed (%s).", strerror(errno));
        return LY_ESYS;
    }

    return LY_SUCCESS;
}

LY_ERR
ly_munmap(void *addr, size_t length)
{
    if (munmap(addr, length)) {
        return LY_ESYS;
    }
    return LY_SUCCESS;
}

LY_ERR
ly_strcat(char **dest, const char *format, ...)
{
    va_list fp;
    char *addition = NULL;
    size_t len;

    va_start(fp, format);
    len = vasprintf(&addition, format, fp);
    len += (*dest ? strlen(*dest) : 0) + 1;

    if (*dest) {
        *dest = ly_realloc(*dest, len);
        if (!*dest) {
            return LY_EMEM;
        }
        *dest = strcat(*dest, addition);
        free(addition);
    } else {
        *dest = addition;
    }

    va_end(fp);
    return LY_SUCCESS;
}

LY_ERR
ly_parse_int(const char *val_str, size_t val_len, int64_t min, int64_t max, int base, int64_t *ret)
{
    char *strptr;
    int64_t i;

    LY_CHECK_ARG_RET(NULL, val_str, val_str[0], val_len, LY_EINVAL);

    /* convert to 64-bit integer, all the redundant characters are handled */
    errno = 0;
    strptr = NULL;

    /* parse the value */
    i = strtoll(val_str, &strptr, base);
    if (errno || strptr == val_str) {
        return LY_EVALID;
    } else if ((i < min) || (i > max)) {
        return LY_EDENIED;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr && strptr < val_str + val_len) {
            return LY_EVALID;
        }
    }

    *ret = i;
    return LY_SUCCESS;
}

LY_ERR
ly_parse_uint(const char *val_str, size_t val_len, uint64_t max, int base, uint64_t *ret)
{
    char *strptr;
    uint64_t u;

    LY_CHECK_ARG_RET(NULL, val_str, val_str[0], LY_EINVAL);

    errno = 0;
    strptr = NULL;
    u = strtoull(val_str, &strptr, base);
    if (errno || strptr == val_str) {
        return LY_EVALID;
    } else if ((u > max) || (u && val_str[0] == '-')) {
        return LY_EDENIED;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr && strptr < val_str + val_len) {
            return LY_EVALID;
        }
    }

    *ret = u;
    return LY_SUCCESS;
}

/**
 * @brief Parse an identifier.
 *
 * ;; An identifier MUST NOT start with (('X'|'x') ('M'|'m') ('L'|'l'))
 * identifier          = (ALPHA / "_")
 *                       *(ALPHA / DIGIT / "_" / "-" / ".")
 *
 * @param[in,out] id Identifier to parse. When returned, it points to the first character which is not part of the identifier.
 * @return LY_ERR value: LY_SUCCESS or LY_EINVAL in case of invalid starting character.
 */
static LY_ERR
lys_parse_id(const char **id)
{
    assert(id && *id);

    if (!is_yangidentstartchar(**id)) {
        return LY_EINVAL;
    }
    ++(*id);

    while (is_yangidentchar(**id)) {
        ++(*id);
    }
    return LY_SUCCESS;
}

LY_ERR
ly_parse_nodeid(const char **id, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len)
{
    assert(id && *id);
    assert(prefix && prefix_len);
    assert(name && name_len);

    *prefix = *id;
    *prefix_len = 0;
    *name = NULL;
    *name_len = 0;

    LY_CHECK_RET(lys_parse_id(id));
    if (**id == ':') {
        /* there is prefix */
        *prefix_len = *id - *prefix;
        ++(*id);
        *name = *id;

        LY_CHECK_RET(lys_parse_id(id));
        *name_len = *id - *name;
    } else {
        /* there is no prefix, so what we have as prefix now is actually the name */
        *name = *prefix;
        *name_len = *id - *name;
        *prefix = NULL;
    }

    return LY_SUCCESS;
}

LY_ERR
ly_parse_instance_predicate(const char **pred, size_t limit, LYD_FORMAT format,
                            const char **prefix, size_t *prefix_len, const char **id, size_t *id_len, const char **value, size_t *value_len,
                            const char **errmsg)
{
    LY_ERR ret = LY_EVALID;
    const char *in = *pred;
    size_t offset = 1;
    int expr = 0;
    char quot;

    assert(in[0] == '\[');

    *prefix = *id = *value = NULL;
    *prefix_len = *id_len = *value_len = 0;

    /* leading *WSP */
    for (; isspace(in[offset]); offset++);

    if (isdigit(in[offset])) {
        /* pos: "[" *WSP positive-integer-value *WSP "]" */
        if (in[offset] == '0') {
            /* zero */
            *errmsg = "The position predicate cannot be zero.";
            goto error;
        }

        /* positive-integer-value */
        *value = &in[offset++];
        for (; isdigit(in[offset]); offset++);
        *value_len = &in[offset] - *value;

    } else if (in[offset] == '.') {
        /* leaf-list-predicate: "[" *WSP "." *WSP "=" *WSP quoted-string *WSP "]" */
        *id = &in[offset];
        *id_len = 1;
        offset++;
        expr = 1;
    } else if (in[offset] == '-') {
        /* typically negative value */
        *errmsg = "Invalid instance predicate format (negative position or invalid node-identifier).";
        goto error;
    } else {
        /* key-predicate: "[" *WSP node-identifier *WSP "=" *WSP quoted-string *WSP "]" */
        in = &in[offset];
        if (ly_parse_nodeid(&in, prefix, prefix_len, id, id_len)) {
            *errmsg = "Invalid node-identifier.";
            goto error;
        }
        if (format == LYD_XML && !(*prefix)) {
            /* all node names MUST be qualified with explicit namespace prefix */
            *errmsg = "Missing prefix of a node name.";
            goto error;
        }
        offset = in - *pred;
        in = *pred;
        expr = 2;
    }

    if (expr) {
        /*  *WSP "=" *WSP quoted-string *WSP "]" */
        for (; isspace(in[offset]); offset++);

        if (in[offset] != '=') {
            if (expr == 1) {
                *errmsg = "Unexpected character instead of \'=\' in leaf-list-predicate.";
            } else { /* 2 */
                *errmsg = "Unexpected character instead of \'=\' in key-predicate.";
            }
            goto error;
        }
        offset++;
        for (; isspace(in[offset]); offset++);

        /* quoted-string */
        quot = in[offset++];
        if (quot != '\'' && quot != '\"') {
            *errmsg = "String value is not quoted.";
            goto error;
        }
        *value = &in[offset];
        for (;offset < limit && (in[offset] != quot || (offset && in[offset - 1] == '\\')); offset++);
        if (in[offset] == quot) {
            *value_len = &in[offset] - *value;
            offset++;
        } else {
            *errmsg = "Value is not terminated quoted-string.";
            goto error;
        }
    }

    /* *WSP "]" */
    for(; isspace(in[offset]); offset++);
    if (in[offset] != ']') {
        if (expr == 0) {
            *errmsg = "Predicate (pos) is not terminated by \']\' character.";
        } else if (expr == 1) {
            *errmsg = "Predicate (leaf-list-predicate) is not terminated by \']\' character.";
        } else { /* 2 */
            *errmsg = "Predicate (key-predicate) is not terminated by \']\' character.";
        }
        goto error;
    }
    offset++;

    if (offset <= limit) {
        *pred = &in[offset];
        return LY_SUCCESS;
    }

    /* we read after the limit */
    *errmsg = "Predicate is incomplete.";
    *prefix = *id = *value = NULL;
    *prefix_len = *id_len = *value_len = 0;
    offset = limit;
    ret = LY_EINVAL;

error:
    *pred = &in[offset];
    return ret;
}
