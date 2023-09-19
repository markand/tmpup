/*
 * tmp.c -- common helpers for every programs
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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "tmp.h"
#include "util.h"

const char * const tmp_durations[] = {
	"day",
	"hour",
	"week",
	"month"
};
size_t tmp_durationsz = LEN(tmp_durations);

char *
tmp_id(void)
{
	static char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	char id[TMP_ID_LEN] = {0};

	for (size_t i = 0; i < sizeof (id) - 1; ++i)
		id[i] = charset[random() % (sizeof (charset) - 1)];

	return estrdup(id);
}

char *
tmp_json(const char *fmt, ...)
{
	assert(fmt);

	va_list ap;
	char *ret;

	va_start(ap, fmt);
	ret = tmp_jsonv(fmt, ap);
	va_end(ap);

	return ret;
}

char *
tmp_jsonv(const char *fmt, va_list ap)
{
	json_t *doc;
	json_error_t err;
	char *ret;

	doc = json_vpack_ex(&err, 0, fmt, ap);

	if (!doc)
		die("abort: %s\n", err.text);
	if (!(ret = json_dumps(doc, JSON_INDENT(2))))
		die("abort: %s\n", strerror(ENOMEM));

	json_decref(doc);

	return ret;
}

int
tmp_parse(json_t **doc, json_error_t *err, const char *text, const char *fmt, ...)
{
	assert(err);
	assert(text);
	assert(fmt);

	va_list ap;
	int rv;

	if (!(*doc = json_loads(text, 0, err)))
		return -1;

	va_start(ap, fmt);
	rv = json_vunpack_ex(*doc, err, 0, fmt, ap);
	va_end(ap);

	return rv;
}
