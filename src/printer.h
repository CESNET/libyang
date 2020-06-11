/**
 * @file printer.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic libyang printer structures and functions
 *
 * Copyright (c) 2015-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PRINTER_H_
#define LY_PRINTER_H_

#include <stdio.h>
#include <unistd.h>

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Printer output structure specifying where the data are printed.
 */
struct ly_out;

/**
 * @brief Types of the printer's output
 */
typedef enum LY_OUT_TYPE {
    LY_OUT_ERROR = -1,  /**< error value to indicate failure of the functions returning LY_OUT_TYPE */
    LY_OUT_FD,          /**< file descriptor printer */
    LY_OUT_FDSTREAM,    /**< internal replacement for LY_OUT_FD in case vdprintf() is not available */
    LY_OUT_FILE,        /**< FILE stream printer */
    LY_OUT_FILEPATH,    /**< filepath printer */
    LY_OUT_MEMORY,      /**< memory printer */
    LY_OUT_CALLBACK     /**< callback printer */
} LY_OUT_TYPE;

/**
 * @brief Get output type of the printer handler.
 *
 * @param[in] out Printer handler.
 * @return Type of the printer's output.
 */
LY_OUT_TYPE ly_out_type(const struct ly_out *out);

/**
 * @brief Reset the output medium to write from its beginning, so the following printer function will rewrite the current data
 * instead of appending.
 *
 * Note that in case the underlying output is not seekable (stream referring a pipe/FIFO/socket or the callback output type),
 * nothing actually happens despite the function succeeds. Also note that the medium is not returned to the state it was when
 * the handler was created. For example, file is seeked into the offset zero and truncated, the content from the time it was opened with
 * ly_out_new_file() is not restored.
 *
 * @param[in] out Printer handler.
 * @return LY_SUCCESS in case of success
 * @return LY_ESYS in case of failure
 */
LY_ERR ly_out_reset(struct ly_out *out);

/**
 * @brief Create printer handler using callback printer function.
 *
 * @param[in] writeclb Pointer to the printer callback function writing the data (see write(2)).
 * @param[in] arg Optional caller-specific argument to be passed to the @p writeclb callback.
 * @return NULL in case of memory allocation error.
 * @return Created printer handler supposed to be passed to different ly*_print_*() functions.
 */
struct ly_out *ly_out_new_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg);

/**
 * @brief Get or reset callback function associated with a callback printer handler.
 *
 * @param[in] out Printer handler.
 * @param[in] fd Optional value of a new file descriptor for the handler. If -1, only the current file descriptor value is returned.
 * @return Previous value of the file descriptor.
 */
ssize_t (*ly_out_clb(struct ly_out *out, ssize_t (*writeclb)(void *arg, const void *buf, size_t count)))(void *arg, const void *buf, size_t count);

/**
 * @brief Get or reset callback function's argument aasociated with a callback printer handler.
 *
 * @param[in] out Printer handler.
 * @param[in] arg caller-specific argument to be passed to the callback function associated with the printer handler.
 * If NULL, only the current file descriptor value is returned.
 * @return The previous callback argument.
 */
void *ly_out_clb_arg(struct ly_out *out, void *arg);

/**
 * @brief Create printer handler using file descriptor.
 *
 * @param[in] fd File descriptor to use.
 * @return NULL in case of error.
 * @return Created printer handler supposed to be passed to different ly*_print_*() functions.
 */
struct ly_out *ly_out_new_fd(int fd);

/**
 * @brief Get or reset file descriptor printer handler.
 *
 * @param[in] out Printer handler.
 * @param[in] fd Optional value of a new file descriptor for the handler. If -1, only the current file descriptor value is returned.
 * @return Previous value of the file descriptor. Note that caller is responsible for closing the returned file descriptor in case of setting new descriptor @p fd.
 * @return -1 in case of error when setting up the new file descriptor.
 */
int ly_out_fd(struct ly_out *out, int fd);

/**
 * @brief Create printer handler using file stream.
 *
 * @param[in] f File stream to use.
 * @return NULL in case of error.
 * @return Created printer handler supposed to be passed to different ly*_print_*() functions.
 */
struct ly_out *ly_out_new_file(FILE *f);

/**
 * @brief Get or reset file stream printer handler.
 *
 * @param[in] out Printer handler.
 * @param[in] f Optional new file stream for the handler. If NULL, only the current file stream is returned.
 * @return Previous file stream of the handler. Note that caller is responsible for closing the returned stream in case of setting new stream @p f.
 */
FILE *ly_out_file(struct ly_out *out, FILE *f);

/**
 * @brief Create printer handler using memory to dump data.
 *
 * @param[in] strp Pointer to store the resulting data. If it points to a pointer to an allocated buffer and
 * @p size of the buffer is set, the buffer is used (and extended if needed) to store the printed data.
 * @param[in] size Size of the buffer provided via @p strp. In case it is 0, the buffer for the printed data
 * is newly allocated even if @p strp points to a pointer to an existing buffer.
 * @return NULL in case of error.
 * @return Created printer handler supposed to be passed to different ly*_print_*() functions.
 */
struct ly_out *ly_out_new_memory(char **strp, size_t size);

/**
 * @brief Get or change memory where the data are dumped.
 *
 * @param[in] out Printer handler.
 * @param[in] strp Optional new string pointer to store the resulting data, same rules as in ly_out_new_memory() are applied.
 * @param[in] size Size of the buffer provided via @p strp. In case it is 0, the buffer for the printed data
 * is newly allocated even if @p strp points to a pointer to an existing buffer. In case the @p strp is NULL, this
 * parameter is ignored.
 * @return Previous dumped data. Note that the caller is responsible to free the data in case of changing string pointer @p strp.
 */
char *ly_out_memory(struct ly_out *out, char **strp, size_t size);

/**
 * @brief Create printer handler file of the given filename.
 *
 * @param[in] filepath Path of the file where to write data.
 * @return NULL in case of error.
 * @return Created printer handler supposed to be passed to different ly*_print_*() functions.
 */
struct ly_out *ly_out_new_filepath(const char *filepath);

/**
 * @brief Get or change the filepath of the file where the printer prints the data.
 *
 * Note that in case of changing the filepath, the current file is closed and a new one is
 * created/opened instead of renaming the previous file. Also note that the previous filepath
 * string is returned only in case of not changing it's value.
 *
 * @param[in] out Printer handler.
 * @param[in] filepath Optional new filepath for the handler. If and only if NULL, the current filepath string is returned.
 * @return Previous filepath string in case the @p filepath argument is NULL.
 * @return NULL if changing filepath succeedes and ((void *)-1) otherwise.
 */
const char *ly_out_filepath(struct ly_out *out, const char *filepath);

/**
 * @brief Generic printer of the given format string into the specified output.
 *
 * Alternatively, ly_write() can be used.
 *
 * @param[in] out Output specification.
 * @param[in] format format string to be printed.
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 * @return The number of printed bytes.
 * @return Negative value in case of error, absolute value of the return code maps to LY_ERR value.
 */
ssize_t ly_print(struct ly_out *out, const char *format, ...);

/**
 * @brief Flush the output from any internal buffers and clean any auxiliary data.
 * @param[in] out Output specification.
 */
void ly_print_flush(struct ly_out *out);

/**
 * @brief Generic printer of the given string buffer into the specified output.
 *
 * Alternatively, ly_print() can be used.
 *
 * As an extension for printing holes (skipping some data until they are known),
 * ly_write_skip() and ly_write_skipped() can be used.
 *
 * @param[in] out Output specification.
 * @param[in] buf Memory buffer with the data to print.
 * @param[in] len Length of the data to print in the @p buf.
 * @return The number of printed bytes.
 * @return Negative value in case of error, absolute value of the return code maps to LY_ERR value.
 */
ssize_t ly_write(struct ly_out *out, const char *buf, size_t len);

/**
 * @brief Free the printer handler.
 * @param[in] out Printer handler to free.
 * @param[in] clb_arg_destructor Freeing function for printer callback (LY_OUT_CALLBACK) argument.
 * @param[in] destroy Flag to free allocated buffer (for LY_OUT_MEMORY) or to
 * close stream/file descriptor (for LY_OUT_FD, LY_OUT_FDSTREAM and LY_OUT_FILE)
 */
void ly_out_free(struct ly_out *out, void (*clb_arg_destructor)(void *arg), int destroy);

#ifdef __cplusplus
}
#endif

#endif /* LY_PRINTER_H_ */
