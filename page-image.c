/*
 * page-image.c -- page /image
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

#include "db-image.h"
#include "db.h"
#include "http.h"
#include "log.h"
#include "page-image.h"
#include "page.h"
#include "image.h"
#include "tmpupd.h"
#include "util.h"

#include "html/image.h"

#define TAG "page-image: "

struct self {
	struct image image;
	struct kreq *req;
	struct khtmlreq html;
};

enum {
	KW_AUTHOR,
	KW_EXPIRES,
	KW_ID,
	KW_TITLE
};

static const char * const keywords[] = {
	[KW_AUTHOR]     = "author",
	[KW_EXPIRES]    = "expires",
	[KW_ID]         = "id",
	[KW_TITLE]      = "title"
};

static int
format(size_t index, void *data)
{
	struct self *self = data;

	switch (index) {
	case KW_AUTHOR:
		khtml_printf(&self->html, "%s", self->image.author);
		break;
	case KW_EXPIRES:
		khtml_printf(&self->html, "%s", tmpupd_expiresin(self->image.start, self->image.end));
		break;
	case KW_ID:
		khtml_printf(&self->html, "%s", self->image.id);
		break;
	case KW_TITLE:
		khtml_printf(&self->html, "%s", self->image.title);
		break;
	default:
		break;
	}

	return 1;
}

static int
find(struct image *img, const char *id)
{
	struct db db = {};
	int rv;

	log_debug(TAG "searching image '%s'", id);

	if (tmpupd_open(&db, DB_RDONLY) < 0)
		return -1;

	rv = db_image_get(img, id, &db);
	db_finish(&db);

	return rv;
}

static void
get(struct kreq *r, const char * const *args)
{
	struct self self = {
		.req = r
	};
	struct ktemplate kt = {
		.key = keywords,
		.keysz = LEN(keywords),
		.cb = format,
		.arg = &self
	};

	switch (find(&self.image, args[0])) {
	case 1:
		khtml_open(&self.html, self.req, 0);
		page_template(self.req, KHTTP_200, &kt, html_image, sizeof (html_image));
		khtml_close(&self.html);
		break;
	case 0:
		page_status(r, KHTTP_404, KMIME_TEXT_HTML);
		break;
	default:
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		break;
	}

	image_finish(&self.image);
}

static void
get_download(struct kreq *r, const char * const *args)
{
	struct image image = {};

	switch (find(&image, args[0])) {
	case 1:
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_APP_OCTET_STREAM]);
		khttp_head(r, kresps[KRESP_CONTENT_LENGTH], "%zu", image.datasz);
		khttp_head(r, kresps[KRESP_CONNECTION], "keep-alive");
		khttp_head(r, kresps[KRESP_CONTENT_DISPOSITION],
		    "attachment; filename=\"%s\"", image.filename);
		khttp_body(r);
		khttp_write(r, (const char *)image.data, image.datasz);
		break;
	case 0:
		page_status(r, KHTTP_404, KMIME_TEXT_HTML);
		break;
	default:
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		break;
	}

	image_finish(&image);
}

static void
post(struct kreq *r)
{
	struct db db;

	if (tmpupd_open(&db, DB_RDWR) < 0) {
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		return;
	}

	db_finish(&db);
}

void
page_image(struct kreq *r, const char * const *args)
{
	assert(r);
	assert(args);

	switch (r->method) {
	case KMETHOD_GET:
		get(r, args);
		break;
	case KMETHOD_POST:
		post(r);
		break;
	default:
		break;
	}
}

void
page_image_download(struct kreq *r, const char * const *args)
{
	assert(r);
	assert(args);

	switch (r->method) {
	case KMETHOD_GET:
		get_download(r, args);
		break;
	default:
		break;
	}
}
