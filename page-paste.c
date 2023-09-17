/*
 * page-paste.c -- page /paste
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

#include "db-paste.h"
#include "db.h"
#include "http.h"
#include "log.h"
#include "page-paste.h"
#include "page.h"
#include "paste.h"
#include "tmpupd.h"
#include "util.h"

#include "html/paste.h"

#define TAG "page-paste: "

struct self {
	struct db db;
	struct paste paste;
	struct kreq *req;
	struct khtmlreq html;
};

enum {
	KW_AUTHOR,
	KW_CODE,
	KW_EXPIRES,
	KW_ID,
	KW_TITLE
};

static const char * const keywords[] = {
	[KW_AUTHOR]     = "author",
	[KW_EXPIRES]    = "expires",
	[KW_ID]         = "id",
	[KW_TITLE]      = "title",
	[KW_CODE]       = "code"
};

static int
format(size_t index, void *data)
{
	struct self *self = data;

	switch (index) {
	case KW_AUTHOR:
		khtml_printf(&self->html, "%s", self->paste.author);
		break;
	case KW_CODE:
		khtml_printf(&self->html, "%s", self->paste.code);
		break;
	case KW_EXPIRES:
		khtml_printf(&self->html, "%s", tmpupd_expiresin(self->paste.start, self->paste.end));
		break;
	case KW_ID:
		khtml_printf(&self->html, "%s", self->paste.id);
		break;
	case KW_TITLE:
		khtml_printf(&self->html, "%s", self->paste.title);
		break;
	default:
		break;
	}

	return 1;
}

static int
find(struct paste *paste, const char *id)
{
	struct db db = {};
	int rv;

	log_debug(TAG "searching paste '%s'", id);

	if (tmpupd_open(&db, DB_RDONLY) < 0)
		return -1;

	rv = db_paste_get(paste, id, &db);
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

	if (tmpupd_open(&self.db, DB_RDONLY) < 0) {
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		return;
	}

	log_debug(TAG "searching paste '%s'", args[0]);

	switch (db_paste_get(&self.paste, args[0], &self.db)) {
	case 1:
		khtml_open(&self.html, self.req, 0);
		page_template(self.req, KHTTP_200, &kt, html_paste, sizeof (html_paste));
		khtml_close(&self.html);
		break;
	case 0:
		page_status(r, KHTTP_404, KMIME_TEXT_HTML);
		break;
	default:
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		break;
	}

	db_finish(&self.db);
}

static void
get_raw(struct kreq *r, const char * const *args)
{
	struct paste paste = {};

	switch (find(&paste, args[0])) {
	case 1:
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_APP_OCTET_STREAM]);
		khttp_head(r, kresps[KRESP_CONTENT_LENGTH], "%zu", strlen(paste.code));
		khttp_head(r, kresps[KRESP_CONNECTION], "keep-alive");
		khttp_head(r, kresps[KRESP_CONTENT_DISPOSITION],
		    "attachment; filename=\"%s\"", paste.filename);
		khttp_body(r);
		khttp_printf(r, "%s", paste.code);
		break;
	case 0:
		page_status(r, KHTTP_404, KMIME_TEXT_HTML);
		break;
	default:
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		break;
	}

	paste_finish(&paste);
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
page_paste(struct kreq *r, const char * const *args)
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
page_paste_raw(struct kreq *r, const char * const *args)
{
	assert(r);
	assert(args);

	switch (r->method) {
	case KMETHOD_GET:
		get_raw(r, args);
		break;
	default:
		break;
	}
}
