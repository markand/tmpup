#ifndef UTIL_H
#define UTIL_H

/**
 * \file util.h
 * \brief Miscellaneous utilities for end user application
 */

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include <jansson.h>

/**
 * \def LEN
 * \brief Return the size of an array.
 * \param x the tab object
 */
#define LEN(x) (sizeof (x) / sizeof (x[0]))

/*
 * OpenBSD that we ship with our code but with a 'b' prefix. These functions
 * will be part of next POSIX version but are still not widely supported.
 */

/**
 * OpenBSD's reallocarray function.
 *
 * \param optr the old pointer
 * \param nmemb number of members
 * \param size size of individual elements
 * \return the new pointer or NULL on allocation failure
 */
void *
breallocarray(void *optr, size_t nmemb, size_t size);

/**
 * OpenBSD strlcat function.
 *
 * \pre dst != NULL
 * \pre src != NULL
 * \param dst the destination string
 * \param src the source string
 * \param dsize the maximum storage space in dst
 * \return the number of bytes copied
 */
size_t
bstrlcat(char *dst, const char *src, size_t dsize);

/**
 * OpenBSD's strlcpy function.
 *
 * \pre dst != NULL
 * \pre src != NULL
 * \param dst the destination string
 * \param src the source string
 * \param dsize the maximum storage space in dst
 * \return the number of bytes copied
 */
size_t
bstrlcpy(char *dst, const char *src, size_t dsize);

/**
 * OpenBSD's strtonum function.
 *
 * \pre numstr != NULL
 * \param numstr the number to convert
 * \param minval the minimum range
 * \param maxval the maximum range
 * \param errstrp pointer to the corresponding error message
 * \return the converted number in the range
 */
long long
bstrtonum(const char *numstr,
          long long minval,
          long long maxval,
          const char **errstrp);

/**
 * Print a last message on standard error and exit with code 1.
 *
 * \pre fmt != NULL
 * \param fmt the printf(3) format string
 */
void
die(const char *fmt, ...);

/**
 * Convert binary data to hexadecimal string.
 *
 * The destination string must be twice as double as the source data + 1 to
 * store the trailing nul terminator.
 *
 * Example: srcsz = 4, dst must be >= 9.
 *
 * \pre src != NULL
 * \pre dst != NULL
 * \param src the source data
 * \param srcsz the source data length
 * \param dst the destination string
 */
void
btohex(const void *src, size_t srcsz, char *dst);

/**
 * Wrap getopt(3) and write our common error message with exit code 1 in case
 * of invalid usage.
 *
 * It is advised that you add POSIXLY_CORRECT environment variable before if
 * you want to disallow options after the first non-option argument.
 *
 * \pre argv != NULL
 * \pre optstring != NULL
 * \param argc number of arguments in argv
 * \param argv command line arguments
 * \param optstring the option string
 * \return a non invalid character option
 */
int
egetopt(int argc, char * const argv[], const char *optstring);

/**
 * Wrap malloc(3) and exit with code 1 in case of memory exhaustion.
 *
 * \note This function should be used only when memory failure is unrecoverable.
 * \param n number of element to create
 * \param w width of each element to allocate
 * \return a non-NULL pointer to memory
 */
void *
emalloc(size_t n, size_t w);

/**
 * Wrap calloc(3) and exit with code 1 in case of memory exhaustion.
 *
 * \note This function should be used only when memory failure is unrecoverable.
 * \param n number of element to create
 * \param w width of each element to allocate
 * \return a non-NULL pointer to memory that is zero-ed
 */
void *
ecalloc(size_t n, size_t w);

/**
 * Wrap realloc(3) and exit with code 1 in case of memory exhaustion.
 *
 * \note This function should be used only when memory failure is unrecoverable.
 * \param old the old pointer
 * \param n number of element to create
 * \param w width of each element to allocate
 * \return a non-NULL pointer to memory that is zero-ed
 */
void *
erealloc(void *old, size_t n, size_t w);

/**
 * Wrap strdup(3) and exit with code 1 in case of memory exhaustion.
 *
 * \note This function should be used only when memory failure is unrecoverable.
 * \pre str != NULL
 * \param str the string to duplicate
 * \return a non-NULL pointer that contains a copy of str
 */
char *
estrdup(const char *str);

/**
 * Wrap strndup(3) and exit with code 1 in case of memory exhaustion.
 *
 * \note This function should be used only when memory failure is unrecoverable.
 * \pre str != NULL
 * \param str the string to duplicate
 * \param n max number of bytes to copy
 * \return a non-NULL pointer that contains a copy of str
 */
char *
estrndup(const char *str, size_t n);

/**
 * Wrap breallocarray(3) and exit with code 1 in case of memory exhaustion.
 *
 * \note This function should be used only when memory failure is unrecoverable.
 * \param ptr the old memory region (may be NULL)
 * \param n number of element to create
 * \param w width of each element to allocate
 * \return a non-NULL pointer to memory that is zero-ed
 */
void *
ereallocarray(void *ptr, size_t n, size_t w);

/**
 * Wrap bstrtonum and exit with code 1 in case of invalid number.
 *
 * \pre numstr != NULL
 * \param numstr the number to convert
 * \param minval the minimum range
 * \param maxval the maximum range
 * \return the converted number in the range
 */
long long
estrtonum(const char *numstr,
          long long minval,
          long long maxval);

/**
 * Wrap open_memstream(3) and exit with code 1 in case of memory exhaustion.
 *
 * \pre buf != NULL
 * \pre bufsz != NULL
 * \param buf pointer to the string to re-allocate
 * \param bufsz pointer to the size of the final string
 * \return a non-NULL file pointer
 */
FILE *
eopen_memstream(char **buf, size_t *bufsz);

/**
 * Wrap fmemopen(3) and exit with code 1 in case of failure.
 *
 * \param buf destination pointer (maybe NULL)
 * \param bufsz maximum size
 * \param mode opening mode
 * \return a non-NULL file pointer
 */
FILE *
efmemopen(void *buf, size_t bufsz, const char *mode);

/**
 * Create a JSON representation using jansson json_pack.
 *
 * Conveniently wrap the function and exit on failure as unless a memory error
 * occurs it's more often a programming error.
 *
 * \param fmt the jansson format string
 * \return a newly allocated indented JSON representation
 */
char *
tmp_json(const char *fmt, ...);

/**
 * Conveniently parse a JSON document from text representation and unpack it
 * directly.
 *
 * User will have ownership of JSON document pointed by *doc as extracted
 * values are borrowed from it. You need to call json_decref on *doc when no
 * longer needed.
 *
 * \pre doc != NULL
 * \pre err != NULL
 * \pre text != NULL
 * \pre fmt != NULL
 * \param doc pointer to a json_t object that will be set to
 * \param err error object destination
 * \param text the JSON document to parse
 * \param fmt the format string and arguments to json_unpack
 * \return 0 on success or -1 on error
 */
int
tmp_parse(json_t **doc, json_error_t *err, const char *text, const char *fmt, ...);

#endif /* !UTIL_H */
