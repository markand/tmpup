/*
 * page.c -- page helpers
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
#include <stdlib.h>

#include "http.h"
#include "page.h"
#include "tmp.h"
#include "util.h"

static const struct status {
	enum khttp code;
	const char *message;
} table[] = {
	{ KHTTP_201, "Created"                  },
	{ KHTTP_400, "Bad request"              },
	{ KHTTP_404, "Not found"                }
};

static int
compare(const void *key, const void *value)
{
	const enum khttp *code = key;
	const struct status *status = value;

	return *code - status->code;
}

static inline const char *
message(enum khttp code)
{
	const struct status *rv;

	rv = bsearch(&code, table, LEN(table), sizeof (*rv), compare);

	return rv ? rv->message : "unknown error";
}

static inline char *
json(enum khttp code, const char *msg)
{
	return tmp_json("{si ss}",
		"status",       code,
		"message",      msg
	);
}

void
page_template(struct kreq *r,
              enum khttp code,
              const struct ktemplate *kt,
              const unsigned char *html,
              size_t htmlsz)
{
	assert(r);
	assert(kt);
	assert(html);

	khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[code]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_body(r);
	khttp_template_buf(r, kt, (const char *)html, htmlsz);
}

void
page_status(struct kreq *r, enum khttp code, enum kmime mime)
{
	assert(r);

	const char *msg;
	char *body = NULL;

	khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[code]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[mime]);
	khttp_body(r);

	msg = message(code);

	switch (mime) {
	case KMIME_TEXT_HTML:
		body = estrdup(msg);
		break;
	case KMIME_APP_JSON:
		body = json(code, msg);
		break;
	default:
		break;
	}

	khttp_printf(r, "%s\n", body);
	free(body);
}

void
page_json(struct kreq *r, enum khttp code, const char *fmt, ...)
{
	assert(r);
	assert(fmt);

	va_list ap;
	char *dump;

	va_start(ap, fmt);
	dump = tmp_jsonv(fmt, ap);
	va_end(ap);

	khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[code]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_APP_JSON]);
	khttp_body(r);
	khttp_printf(r, "%s\n", dump);
	free(dump);
}
