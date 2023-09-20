/*
 * page-static.c -- page /static
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
#include <string.h>

#include "http.h"
#include "page.h"
#include "util.h"

/* CSS files. */
#include "static/normalize.h"
#include "static/style.h"

/* fonts. */
#include "static/dosis.h"

#define STATIC(path, data, mime) { path, data, mime, sizeof (data) }

static struct entry {
	const char *path;
	const unsigned char *data;
	enum kmime mime;
	size_t datasz;
} table[] = {
	STATIC("normalize.css", static_normalize, KMIME_TEXT_CSS),
	STATIC("style.css",     static_style, KMIME_TEXT_CSS),
	STATIC("dosis.ttf",     static_dosis, KMIME_APP_OCTET_STREAM)
};

static void
get(struct kreq *req, const char *file)
{
	const struct entry *ent = NULL;

	for (size_t i = 0; i < LEN(table); ++i) {
		if (strcmp(table[i].path, file) == 0) {
			ent = &table[i];
			break;
		}
	}

	if (ent) {
		khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
		khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[ent->mime]);
		khttp_head(req, kresps[KRESP_CONTENT_LENGTH], "%zu", ent->datasz);
		khttp_body(req);
		khttp_write(req, (const char *)ent->data, ent->datasz);
	} else
		page_status(req, KHTTP_404, KMIME_TEXT_HTML);
}

void
page_static(struct kreq *r, const char * const *args)
{
	assert(r);
	assert(args);

	switch (r->method) {
	case KMETHOD_GET:
		get(r, args[0]);
		break;
	default:
		page_status(r, KHTTP_400, KMIME_TEXT_HTML);
		break;
	}
}
