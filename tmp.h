/*
 * tmp.h -- common helpers for every programs
 *
 * Copyright (c) 2023 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef TMP_H
#define TMP_H

#include <stdarg.h>

#include <jansson.h>

#define TMP_ID_LEN 9

#define TMP_DEFAULT_TITLE       "void"
#define TMP_DEFAULT_AUTHOR      "anonymous"
#define TMP_DEFAULT_FILENAME    "blank"
#define TMP_DEFAULT_LANG        "nohighlight"
#define TMP_DEFAULT_CODE        "the best code is no code"

#define TMP_DURATION_HOUR       3600
#define TMP_DURATION_DAY        86400
#define TMP_DURATION_WEEK       604800
#define TMP_DURATION_MONTH      2678400

extern const char * const tmp_durations[];
extern size_t tmp_durationsz;

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
