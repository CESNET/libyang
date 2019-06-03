

#ifndef LY_PARSER_YIN_H_
#define LY_PARSER_YIN_H_

#include <stdio.h>
#include "log.h"
#include "xml.h"
#include "stdlib.h"

enum YIN_ARGUMENT {
    YIN_ARG_UNKNOWN = 0,   /**< parsed argument can not be matched with any known yin argument keyword */
    YIN_ARG_NAME,          /**< argument name */
    YIN_ARG_TARGET_NODE,   /**< argument target-node */
    YIN_ARG_MODULE,        /**< argument module */
    YIN_ARG_VALUE,         /**< argument value */
    YIN_ARG_TEXT,          /**< argument text */
    YIN_ARG_CONDITION,     /**< argument condition */
    YIN_ARG_URI,           /**< argument uri */
    YIN_ARG_DATE,          /**< argument data */
    YIN_ARG_TAG,           /**< argument tag */
    YIN_ARG_XMLNS,         /**< argument xmlns */
    YIN_ARG_NONE,          /**< special value to specify no valid argument except xmlns, do not confuse with YIN_ARG_NONE */
};

/**
 * @brief Match argument name.
 *
 * @param[in] name String representing name.
 * @param[in] len Lenght of the name.
 *
 * @reurn YIN_ARGUMENT value.
 */
enum YIN_ARGUMENT match_argument_name(const char *name, size_t len);

/**
 * @brief Parse content of whole element as text.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] data Data to read from.
 * @param[out] value Where content of element should be stored.
 */
LY_ERR parse_text_element(struct lyxml_context *xml_ctx, const char **data, const char **value);


#endif /* LY_PARSER_YIN_H_*/
