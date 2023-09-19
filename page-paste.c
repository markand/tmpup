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
#include "tmp.h"
#include "tmpupd.h"
#include "util.h"

#include "html/paste.h"
#include "html/paste-new.h"

#define TAG "page-paste: "

struct self {
	const struct paste *paste;
	struct kreq *req;
	struct khtmlreq html;
};

enum {
	KW_AUTHOR,
	KW_CODE,
	KW_DEFAULT_AUTHOR,
	KW_DEFAULT_CODE,
	KW_DEFAULT_FILENAME,
	KW_DEFAULT_TITLE,
	KW_DURATIONS,
	KW_EXPIRES,
	KW_FILENAME,
	KW_ID,
	KW_LANGUAGES,
	KW_TITLE
};

static const char * const keywords[] = {
	[KW_AUTHOR]           = "author",
	[KW_CODE]             = "code",
	[KW_DEFAULT_AUTHOR]   = "default-author",
	[KW_DEFAULT_CODE]     = "default-code",
	[KW_DEFAULT_FILENAME] = "default-filename",
	[KW_DEFAULT_TITLE]    = "default-title",
	[KW_DURATIONS]        = "durations",
	[KW_EXPIRES]          = "expires",
	[KW_FILENAME]         = "filename",
	[KW_ID]               = "id",
	[KW_LANGUAGES]        = "languages",
	[KW_TITLE]            = "title"
};

static inline int
is_this_language(const struct paste *paste, const char *language)
{
	if (paste)
		return strcmp(paste->language, language) == 0;

	return strcmp(language, TMP_DEFAULT_LANG) == 0;
}

static int
format(size_t index, void *data)
{
	struct self *self = data;

	switch (index) {
	case KW_AUTHOR:
		if (self->paste)
			khtml_printf(&self->html, "%s", self->paste->author);
		break;
	case KW_CODE:
		if (self->paste)
			khtml_printf(&self->html, "%s", self->paste->code);
		break;
	case KW_DEFAULT_AUTHOR:
		khtml_printf(&self->html, "%s", TMP_DEFAULT_AUTHOR);
		break;
	case KW_DEFAULT_CODE:
		khtml_printf(&self->html, "%s", TMP_DEFAULT_CODE);
		break;
	case KW_DEFAULT_FILENAME:
		khtml_printf(&self->html, "%s", TMP_DEFAULT_FILENAME);
		break;
	case KW_DEFAULT_TITLE:
		khtml_printf(&self->html, "%s", TMP_DEFAULT_TITLE);
		break;
	case KW_DURATIONS:
		for (size_t i = 0; i < tmp_durationsz; ++i) {
			khtml_attr(&self->html, KELEM_OPTION,
			    KATTR_VALUE, tmp_durations[i],
			    KATTR__MAX);
			khtml_printf(&self->html, "%s", tmp_durations[i]);
			khtml_closeelem(&self->html, 1);
		}
		break;
	case KW_EXPIRES:
		if (self->paste)
			khtml_printf(&self->html, "%s", tmpupd_expiresin(self->paste->start, self->paste->end));
		break;
	case KW_ID:
		if (self->paste)
			khtml_printf(&self->html, "%s", self->paste->id);
		break;
	case KW_FILENAME:
		if (self->paste)
			khtml_printf(&self->html, "%s", self->paste->filename);
		break;
	case KW_LANGUAGES:
		for (size_t i = 0; i < paste_langsz; ++i) {
			/*
			 * If there is an existing paste, use the paste
			 * language to be marked as selected, otherwise we use
			 * the default global language.
			 */
			if (is_this_language(self->paste, paste_langs[i]))
				khtml_attr(&self->html, KELEM_OPTION,
					KATTR_VALUE, paste_langs[i],
					KATTR_SELECTED, "selected",
					KATTR__MAX);
			else
				khtml_attr(&self->html, KELEM_OPTION,
					KATTR_VALUE, paste_langs[i],
					KATTR__MAX);

			khtml_printf(&self->html, "%s", paste_langs[i]);
			khtml_closeelem(&self->html, 1);
		}
		break;
	case KW_TITLE:
		if (self->paste)
			khtml_printf(&self->html, "%s", self->paste->title);
		break;
	default:
		break;
	}

	return 1;
}

static int
find(struct paste *paste, const char *id)
{
	struct db db;
	int rv;

	if (tmpupd_open(&db, DB_RDONLY) < 0)
		return -1;

	rv = db_paste_get(paste, id, &db);
	db_finish(&db);

	return rv;
}

static void
render(struct kreq *r, const struct paste *paste, const unsigned char *html, size_t htmlsz)
{
	struct self self = {
		.req = r,
		.paste = paste
	};
	struct ktemplate kt = {
		.key = keywords,
		.keysz = LEN(keywords),
		.cb = format,
		.arg = &self
	};

	khtml_open(&self.html, self.req, KHTML_PRETTY);
	page_template(self.req, KHTTP_200, &kt, html, htmlsz);
	khtml_close(&self.html);
}

static void
get(struct kreq *r, const char * const *args)
{
	struct paste paste;

	switch (find(&paste, args[0])) {
	case 1:
		render(r, &paste, html_paste, sizeof (html_paste));
		paste_finish(&paste);
		break;
	case 0:
		page_status(r, KHTTP_404, KMIME_TEXT_HTML);
		break;
	default:
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		break;
	}
}

static void
get_download(struct kreq *r, const char * const *args)
{
	struct paste paste;

	switch (find(&paste, args[0])) {
	case 1:
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_APP_OCTET_STREAM]);
		khttp_head(r, kresps[KRESP_CONTENT_LENGTH], "%zu", strlen(paste.code));
		khttp_head(r, kresps[KRESP_CONNECTION], "keep-alive");
		khttp_head(r, kresps[KRESP_CONTENT_DISPOSITION],
		    "attachment; filename=\"%s\"", paste.filename);
		khttp_body(r);
		khttp_printf(r, "%s", paste.code);
		paste_finish(&paste);
		break;
	case 0:
		page_status(r, KHTTP_404, KMIME_TEXT_HTML);
		break;
	default:
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		break;
	}
}

static void
get_raw(struct kreq *r, const char * const *args)
{
	struct paste paste;
	struct khtmlreq html;

	switch (find(&paste, args[0])) {
	case 1:
		khtml_open(&html, r, KHTML_PRETTY);
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
		khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
		break;
	case 0:
		page_status(r, KHTTP_404, KMIME_TEXT_HTML);
		break;
	default:
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		break;
	}
}

static void
get_new(struct kreq *r, const char * const *args)
{
	struct paste paste = {};

	/*
	 * Try to find an existing paste to fork from in the form template
	 * (used by the /paste/fork route).
	 */
	if (args[0]) {
		log_debug(TAG "trying to fork paste '%s'", args[0]);

		if (find(&paste, args[0]) < 0)
			log_warn(TAG "unable to find existing paste '%s' to fork", args[0]);
	} else
		log_debug(TAG "creating a new paste");

	render(r, paste.id ? &paste : NULL, html_paste_new, sizeof (html_paste_new));
}

static void
post(struct kreq *r)
{
	struct db db = {};
	struct paste paste = {};
	const char *title = NULL,
	           *author = NULL,
	           *filename = NULL,
	           *language = NULL,
	           *code = NULL,
	           *duration = "day";
	time_t start, end;

	if (tmpupd_open(&db, DB_RDWR) < 0) {
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		return;
	}

	for (size_t i = 0; i < r->fieldsz; ++i) {
		if (strcmp(r->fields[i].key, "title") == 0)
			title = r->fields[i].val;
		else if (strcmp(r->fields[i].key, "author") == 0)
			author = r->fields[i].val;
		else if (strcmp(r->fields[i].key, "filename") == 0)
			filename = r->fields[i].val;
		else if (strcmp(r->fields[i].key, "language") == 0)
			language = r->fields[i].val;
		else if (strcmp(r->fields[i].key, "code") == 0)
			code = r->fields[i].val;
		else if (strcmp(r->fields[i].key, "duration") == 0)
			duration = r->fields[i].val;
	}

	tmpupd_condamn(&start, &end, duration);
	paste_init(&paste, NULL, title, author, filename, language, code, start, end);

	if (db_paste_save(&paste, &db) < 0) {
		log_warn(TAG "unable to create paste: %s", db.error);
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
	} else {
		/* Redirect to paste details. */
		log_debug(TAG "created new paste '%s'", paste.id);
		khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_302]);
		khttp_head(r, kresps[KRESP_LOCATION], "/paste/%s", paste.id);
		khttp_body(r);
		khttp_free(r);
	}

	paste_finish(&paste);
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
		page_status(r, 400, KMIME_TEXT_HTML);
		break;
	}
}

void
page_paste_download(struct kreq *r, const char * const *args)
{
	assert(r);
	assert(args);

	switch (r->method) {
	case KMETHOD_GET:
		get_download(r, args);
		break;
	default:
		page_status(r, 400, KMIME_TEXT_HTML);
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
		page_status(r, 400, KMIME_TEXT_HTML);
		break;
	}
}

void
page_paste_new(struct kreq *r, const char * const *args)
{
	assert(r);
	assert(args);

	switch (r->method) {
	case KMETHOD_GET:
		get_new(r, args);
		break;
	case KMETHOD_POST:
		post(r);
		break;
	default:
		page_status(r, 400, KMIME_TEXT_HTML);
		break;
	}
}
