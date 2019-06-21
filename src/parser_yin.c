/**
 * @file parser_yin.c
 * @author David Sedlák <xsedla1d@stud.fit.vutbr.cz>
 * @brief YIN parser.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "context.h"
#include "dict.h"
#include "xml.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "parser_yin.h"

/**
 * @brief check if given string is URI of yin namespace.
 * @param ns Namespace URI to check.
 *
 * @return true if ns equals YIN_NS_URI false otherwise.
 */
#define IS_YIN_NS(ns) (strcmp(ns, YIN_NS_URI) == 0)

const char *const yin_attr_list[] = {
    [YIN_ARG_NAME] = "name",
    [YIN_ARG_TARGET_NODE] = "target-node",
    [YIN_ARG_MODULE] = "module",
    [YIN_ARG_VALUE] = "value",
    [YIN_ARG_TEXT] = "text",
    [YIN_ARG_CONDITION] = "condition",
    [YIN_ARG_URI] = "uri",
    [YIN_ARG_DATE] = "date",
    [YIN_ARG_TAG] = "tag",
    [YIN_ARG_XMLNS] = "xmlns",
};

enum yang_keyword
yin_match_keyword(struct lyxml_context *xml_ctx, const char *name, size_t name_len, const char *prefix, size_t prefix_len)
{
    const char *start = NULL;
    enum yang_keyword kw = YANG_NONE;
    const struct lyxml_ns *ns = NULL;

    if (!name || name_len == 0) {
        return YANG_NONE;
    }

    ns = lyxml_ns_get(xml_ctx, prefix, prefix_len);
    if (ns) {
        if (!IS_YIN_NS(ns->uri)) {
            return YANG_CUSTOM;
        }
    } else {
        /* elements without namespace are automatically unknown */
        return YANG_NONE;
    }


    start = name;
    kw = lysp_match_kw(NULL, &name);

    if (name - start == (long int)name_len) {
        return kw;
    } else {
        return YANG_NONE;
    }
}

enum YIN_ARGUMENT
yin_match_argument_name(const char *name, size_t len)
{
    enum YIN_ARGUMENT arg = YIN_ARG_UNKNOWN;
    size_t already_read = 0;
    LY_CHECK_RET(len == 0, YIN_ARG_NONE);

#define IF_ARG(STR, LEN, STMT) if (!strncmp((name) + already_read, STR, LEN)) {already_read+=LEN;arg=STMT;}
#define IF_ARG_PREFIX(STR, LEN) if (!strncmp((name) + already_read, STR, LEN)) {already_read+=LEN;
#define IF_ARG_PREFIX_END }

    switch (*name) {
    case 'x':
        already_read += 1;
        IF_ARG("mlns", 4, YIN_ARG_XMLNS);
        break;
    case 'c':
        already_read += 1;
        IF_ARG("ondition", 8, YIN_ARG_CONDITION);
        break;

    case 'd':
        already_read += 1;
        IF_ARG("ate", 3, YIN_ARG_DATE);
        break;

    case 'm':
        already_read += 1;
        IF_ARG("odule", 5, YIN_ARG_MODULE);
        break;

    case 'n':
        already_read += 1;
        IF_ARG("ame", 3, YIN_ARG_NAME);
        break;

    case 't':
        already_read += 1;
        IF_ARG_PREFIX("a", 1)
            IF_ARG("g", 1, YIN_ARG_TAG)
            else IF_ARG("rget-node", 9, YIN_ARG_TARGET_NODE)
        IF_ARG_PREFIX_END
        else IF_ARG("ext", 3, YIN_ARG_TEXT)
        break;

    case 'u':
        already_read += 1;
        IF_ARG("ri", 2, YIN_ARG_URI)
        break;

    case 'v':
        already_read += 1;
        IF_ARG("alue", 4, YIN_ARG_VALUE);
        break;
    }

    /* whole argument must be matched */
    if (already_read != len) {
        arg = YIN_ARG_UNKNOWN;
    }

#undef IF_ARG
#undef IF_ARG_PREFIX
#undef IF_ARG_PREFIX_END

    return arg;
}

static void free_arg_rec(struct lyxml_context *xml_ctx, struct yin_arg_record *record) {
    (void)xml_ctx; /* unused */
    if (record->dynamic_content) {
        free(record->content);
    }
}

LY_ERR
yin_load_attributes(struct lyxml_context *xml_ctx, const char **data, struct yin_arg_record **args)
{
    LY_ERR ret = LY_SUCCESS;
    struct yin_arg_record *argument_record = NULL;

    /* load all attributes first */
    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        LY_ARRAY_NEW_GOTO(xml_ctx->ctx, *args, argument_record, ret, cleanup);
        ret = lyxml_get_attribute(xml_ctx, data, &argument_record->prefix, &argument_record->prefix_len,
                                  &argument_record->name, &argument_record->name_len);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

        if (xml_ctx->status == LYXML_ATTR_CONTENT) {
            argument_record->content = NULL;
            argument_record->content_len = 0;
            argument_record->dynamic_content = 0;
            ret = lyxml_get_string(xml_ctx, data, &argument_record->content, &argument_record->content_len,
                                   &argument_record->content, &argument_record->content_len, &argument_record->dynamic_content);
            LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
        }
    }

cleanup:
    if (ret != LY_SUCCESS) {
        FREE_ARRAY(xml_ctx, *args, free_arg_rec);
        *args = NULL;
    }
    return ret;
}

LY_ERR
yin_parse_attribute(struct lyxml_context *xml_ctx, struct yin_arg_record **args, enum YIN_ARGUMENT arg_type, const char **arg_val)
{
    LY_ERR ret = LY_SUCCESS;
    enum YIN_ARGUMENT arg = YIN_ARG_UNKNOWN;
    struct yin_arg_record *iter = NULL;
    const struct lyxml_ns *ns = NULL;

    /* validation of attributes */
    LY_ARRAY_FOR(*args, struct yin_arg_record, iter) {
        ns = lyxml_ns_get(xml_ctx, iter->prefix, iter->prefix_len);
        /* yin arguments represented as attributes have no namespace, which in this case means no prefix */
        if (!iter->prefix) {
            arg = yin_match_argument_name(iter->name, iter->name_len);
            if (arg == YIN_ARG_NONE) {
                continue;
            } else if (arg == arg_type) {
                if (iter->dynamic_content) {
                    *arg_val = lydict_insert_zc(xml_ctx->ctx, iter->content);
                    LY_CHECK_ERR_GOTO(!(*arg_val), LOGMEM(xml_ctx->ctx); ret = LY_EMEM, cleanup);
                    /* string is no longer supposed to be freed when the sized array is freed */
                    iter->dynamic_content = 0;
                } else {
                    *arg_val = lydict_insert(xml_ctx->ctx, iter->content, iter->content_len);
                    LY_CHECK_ERR_GOTO(!(*arg_val), LOGMEM(xml_ctx->ctx); ret = LY_EMEM, cleanup);
                }
            } else {
                LOGERR(xml_ctx->ctx, LYVE_SYNTAX_YIN, "Unexpected attribute \"%.*s\".", iter->name_len, iter->name);
                ret = LY_EVALID;
                goto cleanup;
            }
        }
    }

cleanup:
    return ret;
}

LY_ERR
yin_parse_text_element(struct lyxml_context *xml_ctx, struct yin_arg_record **args, const char **data, const char **value)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *out = NULL;
    const char *prefix = NULL, *name = NULL;
    size_t buf_len = 0, out_len = 0, prefix_len = 0, name_len = 0;
    int dynamic = 0;

    if (xml_ctx->status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
        LY_CHECK_RET(ret);
        if (dynamic) {
            *value = lydict_insert_zc(xml_ctx->ctx, buf);
            LY_CHECK_RET(!(*value), LY_EMEM);
        } else {
            *value = lydict_insert(xml_ctx->ctx, out, out_len);
            LY_CHECK_RET(!(*value), LY_EMEM);
        }
    }

    LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
    /* probably should never happen, mixed content should be caught by lyxml_get_element call but
     * I will just leave it here for now to be sure */
    LY_CHECK_RET(name, LY_EVALID);
    return ret;
}

/**
 * @brief function to parse meta tags eg. elements with text element as child
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] args Sized array of arguments of current elements.
 * @param[in,out] data Data to read from.
 * @param[out] value Where the content of meta tag should be stored.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_meta_element(struct lyxml_context *xml_ctx, struct yin_arg_record **args, const char **data, const char **value)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *out = NULL;
    const char *prefix = NULL, *name = NULL;
    size_t buf_len = 0, out_len = 0, prefix_len = 0, name_len = 0;
    int dynamic = 0;
    enum YIN_ARGUMENT arg = YANG_NONE;
    struct yin_arg_record *subelem_args = NULL;
    enum yang_keyword kw = YANG_NONE;

    LY_CHECK_ERR_RET(xml_ctx->status != LYXML_ELEM_CONTENT, LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Expected content of meta element."), LY_EVALID);

    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_ERR_RET(ret != LY_EINVAL, LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Expected \"text\" element as first child of meta element."), LY_EINVAL);

    /* first element should be argument element <text> */
    LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
    ret = yin_load_attributes(xml_ctx, data, &subelem_args);
    LY_CHECK_GOTO(ret, cleanup);
    arg = yin_match_argument_name(name, name_len);
    LY_CHECK_ERR_GOTO(arg != YIN_ARG_TEXT, LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Expected \"text\" element as first child of meta element.");
                      ret = LY_EINVAL, cleanup);
    ret = yin_parse_text_element(xml_ctx, &subelem_args, data, value);
    LY_CHECK_GOTO(ret, cleanup);

    /* loop over all child elements and parse them */
    while (xml_ctx->status == LYXML_ELEMENT) {
        ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_GOTO(ret, cleanup);
        ret = yin_load_attributes(xml_ctx, data, &subelem_args);
        LY_CHECK_GOTO(ret, cleanup);
        kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);

        if (!name) {
            /* end of meta element reached */
            break;
        }

        switch (kw) {
            case YANG_CUSTOM:
                // TODO parse extension instance
                break;

            default:
                LOGERR(xml_ctx->ctx, LYVE_SYNTAX_YIN, "Unexpected child element \"%.*s\".", name_len, name);
                return LY_EVALID;
        }
    }

cleanup:
    FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
    return ret;
}

/**
 * @brief Parse revision date.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] args Sized array of arguments of current element.
 * @param[in,out] data Data to read from.
 * @param[in,out] rev Array to store the parsed value in.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_revision_date(struct lyxml_context *xml_ctx, struct yin_arg_record **args, const char **data, char *rev, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    const char *temp_rev, *prefix = NULL, *name = NULL;
    char *out;
    struct yin_arg_record *subelem_args = NULL;
    size_t prefix_len = 0, name_len = 0;
    enum yang_keyword kw = YANG_NONE;
    int dynamic;

    if (rev[0]) {
        LOGVAL_PARSER(xml_ctx, LY_VCODE_DUPSTMT, "revision-date");
        return LY_EVALID;
    }

    LY_CHECK_RET(yin_parse_attribute(xml_ctx, args, YIN_ARG_DATE, &temp_rev))
    LY_CHECK_RET(ret != LY_SUCCESS, ret);
    LY_CHECK_RET(lysp_check_date((struct lys_parser_ctx *)xml_ctx, temp_rev, strlen(temp_rev), "revision-date") != LY_SUCCESS, LY_EVALID);

    strcpy(rev, temp_rev);
    lydict_remove(xml_ctx->ctx, temp_rev);

    if (xml_ctx->status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(xml_ctx, data, &out, &name_len, &out, &name_len, &dynamic);
        /* unknown element text content is ignored */
        if (ret == LY_EINVAL) {
            while (xml_ctx->status == LYXML_ELEMENT) {
                ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
                LY_CHECK_GOTO(ret, cleanup);
                ret = yin_load_attributes(xml_ctx, data, &subelem_args);
                LY_CHECK_GOTO(ret, cleanup);
                kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);

                if (!name) {
                    /* end of revisin-date element reached */
                    break;
                }

                switch (kw) {
                    case YANG_CUSTOM:
                        // TODO parse extension instance
                        break;

                    default:
                        LOGERR(xml_ctx->ctx, LYVE_SYNTAX_YIN, "Unexpected child element \"%.*s\".", name_len, name);
                        return LY_EVALID;
                }
            }
        } else {
            /* load closing element */
            LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
            LY_CHECK_RET(name, LY_EINVAL);
        }
    }

cleanup:
    FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
    return ret;
}

LY_ERR
yin_parse_import(struct lyxml_context *xml_ctx, struct yin_arg_record **import_args, const char *module_prefix, const char **data, struct lysp_import **imports)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw;
    struct lysp_import *imp;
    const char *prefix, *name;
    char *out;
    struct yin_arg_record *subelem_args = NULL;
    size_t prefix_len, name_len;
    int dynamic;

    /* allocate new element in sized array for import */
    LY_ARRAY_NEW_GOTO(xml_ctx->ctx, *imports, imp, ret, validation_err);

    /* parse import attributes  */
    LY_CHECK_GOTO(yin_parse_attribute(xml_ctx, import_args, YIN_ARG_MODULE, &imp->name) != LY_SUCCESS, validation_err);

    ret = lyxml_get_string(xml_ctx, data, &out, &name_len, &out, &name_len, &dynamic);
    LY_CHECK_ERR_RET(ret != LY_EINVAL, LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Expected new element after import element"), LY_EINVAL);

    while (xml_ctx->status == LYXML_ELEMENT) {
        ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_GOTO(ret, validation_err);
        if (!name) {
            /* end of import element reached */
            break;
        }
        LY_CHECK_GOTO(yin_load_attributes(xml_ctx, data, &subelem_args), validation_err);
        kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);

        switch (kw) {
        case YANG_PREFIX:
            LY_CHECK_ERR_GOTO(imp->prefix, LOGVAL_PARSER(xml_ctx, LY_VCODE_DUPSTMT, "prefix"), validation_err);
            LY_CHECK_GOTO(yin_parse_attribute(xml_ctx, &subelem_args, YIN_ARG_VALUE, &imp->prefix) != LY_SUCCESS, validation_err);
            LY_CHECK_GOTO(lysp_check_prefix((struct lys_parser_ctx *)xml_ctx, *imports, module_prefix, &imp->prefix) != LY_SUCCESS, validation_err);
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_ERR_GOTO(imp->dsc, LOGVAL_PARSER(xml_ctx, LY_VCODE_DUPSTMT, "description"), validation_err);
            LY_CHECK_GOTO(yin_parse_meta_element(xml_ctx, &subelem_args, data, &imp->dsc), validation_err);
            break;
        case YANG_REFERENCE:
            LY_CHECK_ERR_GOTO(imp->ref, LOGVAL_PARSER(xml_ctx, LY_VCODE_DUPSTMT, "reference"), validation_err);
            LY_CHECK_GOTO(yin_parse_meta_element(xml_ctx, &subelem_args, data, &imp->ref), validation_err);
            break;
        case YANG_REVISION_DATE:
            LY_CHECK_GOTO(yin_parse_revision_date(xml_ctx, &subelem_args, data, imp->rev, &imp->exts), validation_err);
            break;
        case YANG_CUSTOM:
            /* TODO parse extension */
            break;
        default:
            LOGERR(xml_ctx->ctx, LY_VCODE_UNEXP_SUBELEM, name_len, name, "import");
            goto validation_err;
        }
        FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
        subelem_args = NULL;
    }

    LY_CHECK_ERR_GOTO(!imp->prefix, LOGVAL_PARSER(xml_ctx, LY_VCODE_MISSATTR, "prefix", "import"), validation_err);
    return ret;

validation_err:
    FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
    return LY_EVALID;
}

LY_ERR
yin_parse_status(struct lyxml_context *xml_ctx, struct yin_arg_record **status_args, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    const char *value = NULL, *prefix = NULL, *name = NULL;
    char *out;
    size_t prefix_len = 0, name_len = 0, out_len = 0;
    int dynamic = 0;
    struct yin_arg_record *subelem_args = NULL;
    if (*flags & LYS_STATUS_MASK) {
        LOGVAL_PARSER(xml_ctx, LY_VCODE_DUPELEM, "status");
        return LY_EVALID;
    }

    LY_CHECK_RET(yin_parse_attribute(xml_ctx, status_args, YIN_ARG_VALUE, &value));
    if (strcmp(value, "current") == 0) {
        *flags |= LYS_STATUS_CURR;
    } else if (strcmp(value, "deprecated") == 0) {
        *flags |= LYS_STATUS_DEPRC;
    } else if (strcmp(value, "obsolete") == 0) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL_PARSER(xml_ctx, LY_VCODE_INVAL_YIN, value, "status");
        lydict_remove(xml_ctx->ctx, value);
        return LY_EVALID;
    }
    lydict_remove(xml_ctx->ctx, value);

    /* TODO check if dynamic was set to 1 in case of error */
    if (xml_ctx->status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
        /* if there are any xml subelements parse them, unknown text content is silently ignored */
        if (ret == LY_EINVAL) {
            /* load subelements */
            while (xml_ctx->status == LYXML_ELEMENT) {
                LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
                if (!name) {
                    /* end of status reached */
                    break;
                }
                ret = yin_load_attributes(xml_ctx, data, &subelem_args);
                LY_CHECK_GOTO(ret, cleanup);
                kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);
                switch (kw) {
                    case YANG_CUSTOM:
                        /* TODO parse extension instance */
                        break;
                    default:
                        LOGVAL_PARSER(xml_ctx, LY_VCODE_INCHILDSTMT_YIN, name_len, name, 6, "status");
                        ret = LY_EVALID;
                        goto cleanup;
                }

                FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
                subelem_args = NULL;
            }
        } else {
            /* load closing element */
            LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
            LY_CHECK_RET(name, LY_EINVAL);
        }
    }

    return ret;

cleanup:
    FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
    return ret;
}

LY_ERR
yin_parse_yin_element_element(struct lyxml_context *xml_ctx, struct yin_arg_record **attrs, const char **data, uint16_t *flags, struct lysp_ext **extensions)
{
    LY_ERR ret = LY_SUCCESS;
    const char *temp_val = NULL, *prefix, *name;
    char *out;
    size_t prefix_len, name_len;
    struct yin_arg_record *subelem_args = NULL;
    int dynamic;
    enum yang_keyword kw = YANG_NONE;

    if (*flags & LYS_YINELEM_MASK) {
        LOGVAL_PARSER(xml_ctx, LY_VCODE_DUPSTMT, "yin-element");
        return LY_EVALID;
    }

    LY_CHECK_RET(yin_parse_attribute(xml_ctx, attrs, YIN_ARG_VALUE, &temp_val));
    if (strcmp(temp_val, "true") == 0) {
        *flags |= LYS_YINELEM_TRUE;
    } else if (strcmp(temp_val, "false") == 0) {
        *flags |= LYS_YINELEM_FALSE;
    } else {
        LOGVAL_PARSER(xml_ctx, LY_VCODE_INVAL_YIN, temp_val, "yin-element");
        lydict_remove(xml_ctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(xml_ctx->ctx, temp_val);

    if (xml_ctx->status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(xml_ctx, data, &out, &name_len, &out, &name_len, &dynamic);
        /* unknown element text content is ignored */
        if (ret == LY_EINVAL) {
            while (xml_ctx->status == LYXML_ELEMENT) {
                ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
                LY_CHECK_GOTO(ret, cleanup);
                ret = yin_load_attributes(xml_ctx, data, &subelem_args);
                LY_CHECK_GOTO(ret, cleanup);
                kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);

                if (!name) {
                    /* end of yin-element element reached */
                    break;
                }

                switch (kw) {
                    case YANG_CUSTOM:
                        // TODO parse extension instance
                        break;

                    default:
                        LOGERR(xml_ctx->ctx, LYVE_SYNTAX_YIN, "Unexpected child element \"%.*s\".", name_len, name);
                        return LY_EVALID;
                }
            }
        } else {
            /* load closing element */
            LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
            LY_CHECK_RET(name, LY_EINVAL);
        }
    }

cleanup:
    FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
    return ret;
}

LY_ERR
yin_parse_argument_element(struct lyxml_context *xml_ctx, struct yin_arg_record **attrs, const char **data,
                           uint16_t *flags, const char **argument, struct lysp_ext **extensions)
{
    LY_ERR ret = LY_SUCCESS;
    const char *name, *prefix;
    size_t name_len, prefix_len;
    struct yin_arg_record *subelem_args = NULL;
    char *out;
    int dynamic;
    enum yang_keyword kw = YANG_NONE;

    LY_CHECK_RET(yin_parse_attribute(xml_ctx, attrs, YIN_ARG_NAME, argument));

    if (xml_ctx->status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(xml_ctx, data, &out, &name_len, &out, &name_len, &dynamic);
        /* unknown element text content is ignored */
        if (ret == LY_EINVAL) {
            while (xml_ctx->status == LYXML_ELEMENT) {
                ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
                LY_CHECK_GOTO(ret, cleanup);
                ret = yin_load_attributes(xml_ctx, data, &subelem_args);
                LY_CHECK_GOTO(ret, cleanup);
                kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);

                if (!name) {
                    /* end of yin-element element reached */
                    break;
                }

                switch (kw) {
                    case YANG_YIN_ELEMENT:
                            yin_parse_yin_element_element(xml_ctx, &subelem_args, data, flags, extensions);
                        break;
                    case YANG_CUSTOM:
                        // TODO parse extension instance
                        break;

                    default:
                        LOGERR(xml_ctx->ctx, LYVE_SYNTAX_YIN, "Unexpected child element \"%.*s\".", name_len, name);
                        return LY_EVALID;
                }
            }
        } else {
            /* load closing element */
            LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
            LY_CHECK_RET(name, LY_EINVAL);
        }
    }

cleanup:
    FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
    return ret;
}

LY_ERR
yin_parse_extension(struct lyxml_context *xml_ctx, struct yin_arg_record **extension_args, const char **data, struct lysp_ext **extensions)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_ext *ex;
    const char *prefix = NULL, *name = NULL;
    char *out = NULL;
    size_t out_len = 0, prefix_len = 0, name_len = 0;
    int dynamic = 0;
    enum yang_keyword kw = YANG_NONE;
    struct yin_arg_record *subelem_args = NULL;

    LY_ARRAY_NEW_GOTO(xml_ctx->ctx, *extensions, ex, ret, cleanup);
    ret = yin_parse_attribute(xml_ctx, extension_args, YIN_ARG_NAME, &ex->name);
    LY_CHECK_GOTO(ret, cleanup);
    ret = lyxml_get_string(xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
    LY_CHECK_ERR_GOTO(ret != LY_EINVAL, LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Expected new element after extension element.");
                      ret = LY_EINVAL, cleanup);

    while (xml_ctx->status == LYXML_ELEMENT) {
        ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_GOTO(ret, cleanup);
        if (!name) {
            /* end of extension element reached */
            break;
        }
        ret = yin_load_attributes(xml_ctx, data, &subelem_args);
        LY_CHECK_GOTO(ret, cleanup);
        kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);
        switch (kw) {
            case YANG_ARGUMENT:
                ret = yin_parse_argument_element(xml_ctx, &subelem_args, data, &ex->flags, &ex->argument, extensions);
                break;
            case YANG_DESCRIPTION:
                ret = yin_parse_meta_element(xml_ctx, &subelem_args, data, &ex->dsc);
                LY_CHECK_GOTO(ret, cleanup);
                break;
            case YANG_REFERENCE:
                ret = yin_parse_meta_element(xml_ctx, &subelem_args, data, &ex->ref);
                LY_CHECK_GOTO(ret, cleanup);
                break;
            case YANG_STATUS:
                ret = yin_parse_status(xml_ctx, &subelem_args, data, &ex->flags, &ex->exts);
                LY_CHECK_GOTO(ret, cleanup);
                break;
            case YANG_CUSTOM:
                /* TODO parse extension instance */
                break;
            default:
                LOGVAL_PARSER(xml_ctx, LY_VCODE_INCHILDSTMT_YIN, name_len, name, 9, "extension");
                ret = LY_EVALID;
                goto cleanup;
        }
        FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
        subelem_args = NULL;
    }

cleanup:
    FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
    return ret;
}

/**
 * @brief Parse module substatements.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in,out] data Data to read from.
 * @param[out] mod Parsed module structure.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_mod(struct lyxml_context *xml_ctx, struct yin_arg_record **mod_args, const char **data, struct lysp_module **mod)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    enum yang_module_stmt mod_stmt = Y_MOD_MODULE_HEADER;

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic = 0;
    struct yin_arg_record *substmt_args = NULL;

    yin_parse_attribute(xml_ctx, mod_args, YIN_ARG_NAME, &(*mod)->mod->name);
    LY_CHECK_ERR_RET(!(*mod)->mod->name, LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Missing argument name of a module"), LY_EVALID);
    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_ERR_RET(ret != LY_EINVAL, LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Expected new xml element after module element."), LY_EINVAL);

    /* loop over all elements and parse them */
    while (xml_ctx->status != LYXML_END) {
#define CHECK_ORDER(SECTION) \
        if (mod_stmt > SECTION) {return LY_EVALID;}mod_stmt = SECTION

        switch (kw) {
        /* module header */
        case YANG_NAMESPACE:
        case YANG_PREFIX:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        case YANG_YANG_VERSION:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        /* linkage */
        case YANG_INCLUDE:
        case YANG_IMPORT:
            CHECK_ORDER(Y_MOD_LINKAGE);
            break;
        /* meta */
        case YANG_ORGANIZATION:
        case YANG_CONTACT:
        case YANG_DESCRIPTION:
        case YANG_REFERENCE:
            CHECK_ORDER(Y_MOD_META);
            break;

        /* revision */
        case YANG_REVISION:
            CHECK_ORDER(Y_MOD_REVISION);
            break;
        /* body */
        case YANG_ANYDATA:
        case YANG_ANYXML:
        case YANG_AUGMENT:
        case YANG_CHOICE:
        case YANG_CONTAINER:
        case YANG_DEVIATION:
        case YANG_EXTENSION:
        case YANG_FEATURE:
        case YANG_GROUPING:
        case YANG_IDENTITY:
        case YANG_LEAF:
        case YANG_LEAF_LIST:
        case YANG_LIST:
        case YANG_NOTIFICATION:
        case YANG_RPC:
        case YANG_TYPEDEF:
        case YANG_USES:
        case YANG_CUSTOM:
            mod_stmt = Y_MOD_BODY;
            break;
        default:
            /* error will be handled in the next switch */
            break;
        }
#undef CHECK_ORDER

        LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
        if (name) {
            LY_CHECK_RET(yin_load_attributes(xml_ctx, data, &substmt_args));
            kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);
            switch (kw) {

            /* module header */
            case YANG_NAMESPACE:
                ret = yin_parse_attribute(xml_ctx, &substmt_args, YIN_ARG_URI, &(*mod)->mod->ns);
                LY_CHECK_GOTO(ret, error);
                break;
            case YANG_PREFIX:
                ret = yin_parse_attribute(xml_ctx, &substmt_args, YIN_ARG_VALUE, &(*mod)->mod->prefix);
                LY_CHECK_GOTO(ret, error);
                break;

            /* linkage */
            case YANG_IMPORT:
                ret = yin_parse_import(xml_ctx, &substmt_args, (*mod)->mod->prefix, data, &(*mod)->imports);
                LY_CHECK_GOTO(ret, error);
                break;

            /* meta */
            case YANG_ORGANIZATION:
                ret = yin_parse_meta_element(xml_ctx, &substmt_args, data, &(*mod)->mod->org);
                LY_CHECK_GOTO(ret, error);
                break;
            case YANG_CONTACT:
                ret = yin_parse_meta_element(xml_ctx, &substmt_args, data, &(*mod)->mod->contact);
                LY_CHECK_GOTO(ret, error);
                break;
            case YANG_DESCRIPTION:
                ret = yin_parse_meta_element(xml_ctx, &substmt_args, data, &(*mod)->mod->dsc);
                LY_CHECK_GOTO(ret, error);
                break;
            case YANG_REFERENCE:
                ret = yin_parse_meta_element(xml_ctx, &substmt_args, data, &(*mod)->mod->ref);
                LY_CHECK_GOTO(ret, error);
                break;
            /* revision */

            /*body */
            case YANG_EXTENSION:
                ret = yin_parse_extension(xml_ctx, &substmt_args, data, &(*mod)->extensions);
                LY_CHECK_GOTO(ret, error);
                break;

            default:
                ret = LY_EVALID;
                goto error;
            }
            FREE_ARRAY(xml_ctx, substmt_args, free_arg_rec);
            substmt_args = NULL;
        }
    }

    return LY_SUCCESS;

error:
    FREE_ARRAY(xml_ctx, substmt_args, free_arg_rec);
    return ret;
}

LY_ERR
yin_parse_module(struct ly_ctx *ctx, const char *data, struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    struct lys_parser_ctx parser_ctx;
    struct lyxml_context *xml_ctx = (struct lyxml_context *)&parser_ctx;
    struct lysp_module *mod_p = NULL;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    struct yin_arg_record *args = NULL;

    /* initialize xml context */
    memset(&parser_ctx, 0, sizeof parser_ctx);
    xml_ctx->ctx = ctx;
    xml_ctx->line = 1;

    /* check submodule */
    ret = lyxml_get_element(xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_GOTO(ret, cleanup);
    ret = yin_load_attributes(xml_ctx, &data, &args);
    LY_CHECK_GOTO(ret, cleanup);
    kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);
    if (kw == YANG_SUBMODULE) {
        LOGERR(ctx, LY_EDENIED, "Input data contains submodule which cannot be parsed directly without its main module.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != YANG_MODULE) {
        LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".",
                    ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    /* allocate module */
    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(ctx), cleanup);
    mod_p->mod = mod;
    mod_p->parsing = 1;

    /* parse module substatements */
    ret = yin_parse_mod(xml_ctx, &args, &data, &mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    mod_p->parsing = 0;
    mod->parsed = mod_p;

cleanup:
    if (ret != LY_SUCCESS) {
        lysp_module_free(mod_p);
    }
    FREE_ARRAY(xml_ctx, args, free_arg_rec);
    lyxml_context_clear(xml_ctx);
    return ret;
}
