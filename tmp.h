#ifndef TMP_H
#define TMP_H

#include <stdarg.h>

#include <jansson.h>

#define TMP_DEFAULT_TITLE       "void"
#define TMP_DEFAULT_AUTHOR      "anon"
#define TMP_DEFAULT_FILENAME    "box"
#define TMP_DEFAULT_LANG        "nohighlight"
#define TMP_ID_LEN 9

char *
tmp_id(void);

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

char *
tmp_jsonv(const char *fmt, va_list ap);

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

#endif /* !TMP_H */
