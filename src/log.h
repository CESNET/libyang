/**
 * @file log.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Logger manipulation routines and error definitions.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_LOG_H_
#define LY_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* dummy context structure */
struct ly_ctx;

/**
 * @defgroup log Logger
 * @{
 *
 * Publicly visible functions and values of the libyang logger. For more
 * information, see \ref howtologger.
 */

/**
 * @typedef LY_LOG_LEVEL
 * @brief Verbosity levels of the libyang logger.
 */
typedef enum
{
    LY_LLERR = 0, /**< Print only error messages, default value. */
    LY_LLWRN = 1, /**< Print error and warning messages. */
    LY_LLVRB = 2, /**< Besides errors and warnings, print some other verbose messages. */
    LY_LLDBG = 3 /**< Print all messages including some development debug messages (be careful,
     without subsequently calling ly_verb_dbg() no debug messages will be printed!). */
} LY_LOG_LEVEL;

/**
 * @brief Set logger verbosity level.
 * @param[in] level Verbosity level.
 * @return Previous verbosity level.
 */
LY_LOG_LEVEL ly_verb(LY_LOG_LEVEL level);

/**
 * @defgroup logopts Logging options
 * @ingroup logger
 *
 * Logging option bits of libyang.
 *
 * @{
 */
#define LY_LOLOG        0x01 /**< Log messages normally, using callback if set. If not set, messages will
                                  not be printed by libyang. */
#define LY_LOSTORE      0x02 /**< Store any generated errors or warnings, never verbose or debug messages.
                                  Note that if #LY_LOLOG is not set then verbose and debug messages are always lost. */
#define LY_LOSTORE_LAST 0x06 /**< Store any generated errors or warnings but only the last message, always overwrite
                                  the previous one. */

/**
 * @}
 */

/**
 * @brief Set additional logger options. Default is #LY_LOLOG | #LY_LOSTORE_LAST.
 *
 * @param[in] opts Bitfield of @ref logopts.
 * @return Previous logger options.
 */
int ly_log_options(int opts);

#ifndef NDEBUG

/**
 * @defgroup dbggroup Debug message groups
 * @ingroup log
 *
 * Selected displayed debug message groups.
 *
 * @{
 */

#define LY_LDGDICT  0x01 /**< Dictionary additions and deletions. */
#define LY_LDGYANG  0x02 /**< YANG parser messages. */
#define LY_LDGYIN   0x04 /**< YIN parser messages. */
#define LY_LDGXPATH 0x08 /**< XPath parsing end evaluation. */
#define LY_LDGDIFF  0x10 /**< Diff processing and creation. */

/**
 * @}
 */

/**
 * @brief Enable specific debugging messages (independent of log level).
 * @param[in] dbg_groups Bitfield of enabled debug message groups (see @ref dbggroup).
 */
void ly_verb_dbg(int dbg_groups);

#endif

/**
 * @brief Set logger callback.
 *
 * !IMPORTANT! If an error has a specific error-app-tag defined in the model, it will NOT be set
 *             at the time of calling this callback. It will be set right after, so to retrieve it
 *             it must be checked afterwards with ly_errapptag().
 *
 * @param[in] clb Logging callback.
 * @param[in] path flag to resolve and provide path as the third parameter of the callback function. In case of
 *            validation and some other errors, it can be useful to get the path to the problematic element. Note,
 *            that according to the tree type and the specific situation, the path can slightly differs (keys
 *            presence) or it can be NULL, so consider it as an optional parameter. If the flag is 0, libyang will
 *            not bother with resolving the path.
 */
void ly_set_log_clb(void(*clb)(LY_LOG_LEVEL level, const char *msg, const char *path), int path);

/**
 * @brief Get logger callback.
 * @return Logger callback (can be NULL).
 */
void (*ly_get_log_clb(void))(LY_LOG_LEVEL, const char *, const char *);

/** @} log */

/**
 * @defgroup errors Error information
 * @{
 */

/**
 * @typedef LY_ERR
 * @brief libyang's error codes returned by the libyang functions.
 */
typedef enum
{
    LY_SUCCESS = 0, /**< no error, not set by functions, included just to complete #LY_ERR enumeration */
    LY_EMEM,        /**< Memory allocation failure */
    LY_ESYS,        /**< System call failure */
    LY_EINVAL,      /**< Invalid value */
    LY_EEXIST,      /**< Item already exists */
    LY_ENOTFOUND,   /**< Item does not exists */
    LY_EINT,        /**< Internal error */
    LY_EVALID,      /**< Validation failure */
    LY_EDENIED,     /**< Operation is not allowed */
    LY_EINCOMPLETE, /**< The operation did not failed, but for some reason it was not possible to finish it completely.
                         According to the specific use case, the caller is usually supposed to perform the operation again. */
    LY_ENOT,        /**< Negative result */
    LY_EOTHER,      /**< Unknown error */

    LY_EPLUGIN = 128/**< Error reported by a plugin - the highest bit in the first byte is set.
                         This value is used ORed with one of the other LY_ERR value and can be simply masked. */
} LY_ERR;

/**
 * @typedef LY_VECODE
 * @brief libyang's codes of validation error. Whenever ly_errno is set to LY_EVALID, the ly_vecode is also set
 * to the appropriate LY_VECODE value.
 * @ingroup logger
 */
typedef enum {
    LYVE_SUCCESS = 0,  /**< no error */
    LYVE_SYNTAX,       /**< generic syntax error */
    LYVE_SYNTAX_YANG,  /**< YANG-related syntax error */
    LYVE_SYNTAX_YIN,   /**< YIN-related syntax error */
    LYVE_REFERENCE,    /**< invalid referencing or using an item */
    LYVE_XPATH,        /**< invalid XPath expression */
    LYVE_SEMANTICS,    /**< generic semantic error */
    LYVE_SYNTAX_XML,   /**< XML-related syntax error */
    LYVE_SYNTAX_JSON,  /**< JSON-related syntax error */
    LYVE_DATA,         /**< YANG data does not reflect some of the module restrictions */

    LYVE_OTHER         /**< Unknown error */
} LY_VECODE;

/**
 * @brief Libyang full error structure.
 */
struct ly_err_item {
    LY_LOG_LEVEL level;
    LY_ERR no;
    LY_VECODE vecode;
    char *msg;
    char *path;
    char *apptag;
    struct ly_err_item *next;
    struct ly_err_item *prev; /* first item's prev points to the last item */
};

/**
 * @brief Get the last (thread, context-specific) validation error code.
 *
 * This value is set only if ly_errno is #LY_EVALID.
 *
 * @param[in] ctx Relative context.
 * @return Validation error code.
 */
LY_VECODE ly_vecode(const struct ly_ctx *ctx);

/**
 * @brief Get the last (thread, context-specific) error code.
 *
 * @param[in] ctx Relative context.
 * @return LY_ERR value of the last error code.
 */
LY_ERR ly_errcode(const struct ly_ctx *ctx);

/**
 * @brief Get the last (thread, context-specific) error message. If the coresponding module defined
 * a specific error message, it will be used instead the default one.
 *
 * Sometimes, the error message is extended with path of the element where the problem is.
 * The path is available via ly_errpath().
 *
 * @param[in] ctx Relative context.
 * @return Text of the last error message, empty string if there is no error.
 */
const char *ly_errmsg(const struct ly_ctx *ctx);

/**
 * @brief Get the last (thread, context-specific) path of the element where was an error.
 *
 * The path always corresponds to the error message available via ly_errmsg(), so
 * whenever a subsequent error message is printed, the path is erased or rewritten.
 * The path reflects the type of the processed tree - data path for data tree functions
 * and schema path in case of schema tree functions. In case of processing YIN schema
 * or XML data, the path can be just XML path. In such a case, the corresponding
 * ly_vecode (value 1-3) is set.
 *
 * @param[in] ctx Relative context.
 * @return Path of the error element, empty string if error path does not apply to the last error.
 */
const char *ly_errpath(const struct ly_ctx *ctx);

/**
 * @brief Get the last (thread, context-specific) error-app-tag if there was a specific one defined
 * in the module for the last error.
 *
 * The app-tag always corresponds to the error message available via ly_errmsg(), so
 * whenever a subsequent error message is printed, the app-tag is erased or rewritten.
 *
 * @param[in] ctx Relative context.
 * @return Error-app-tag of the last error, empty string if the error-app-tag does not apply to the last error.
 */
const char *ly_errapptag(const struct ly_ctx *ctx);

/**
 * @brief Get the first (thread, context-specific) generated error structure.
 *
 * @param[in] ctx Relative context.
 * @return First error structure (can be NULL), do not modify!
 */
struct ly_err_item *ly_err_first(const struct ly_ctx *ctx);

/**
 * @brief Print the error structure as if just generated.
 *
 * @param[in] eitem Error item structure to print.
 */
void ly_err_print(struct ly_err_item *eitem);

/**
 * @brief Free error structures from a context.
 *
 * If \p eitem is not set, free all the error structures.
 *
 * @param[in] ctx Relative context.
 * @param[in] eitem Oldest error structure to remove, optional.
 */
void ly_err_clean(struct ly_ctx *ctx, struct ly_err_item *eitem);

/** @} errors */

#ifdef __cplusplus
}
#endif

#endif /* LY_LOG_H_ */
