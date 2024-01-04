/*
 * route-image.c -- route /image
 *
 * Copyright (c) 2023-2024 David Demelier <markand@malikania.fr>
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

#include "db-image.h"
#include "db.h"
#include "http.h"
#include "image.h"
#include "log.h"
#include "route-image.h"
#include "route.h"
#include "tmp.h"
#include "tmpupd.h"
#include "util.h"

#include "html/image-new.h"
#include "html/image.h"

#define TAG "route-image: "

struct self {
	const struct image *image;
	struct kreq *req;
	struct khtmlreq html;
};

enum {
	KW_AUTHOR,
	KW_DEFAULT_AUTHOR,
	KW_DEFAULT_TITLE,
	KW_DURATIONS,
	KW_EXPIRES,
	KW_ID,
	KW_TITLE,
	KW_VISIBILITY
};

static const char * const keywords[] = {
	[KW_AUTHOR]             = "author",
	[KW_DEFAULT_AUTHOR]	= "default-author",
	[KW_DEFAULT_TITLE]      = "default-title",
	[KW_DURATIONS]          = "durations",
	[KW_EXPIRES]            = "expires",
	[KW_ID]                 = "id",
	[KW_TITLE]              = "title",
	[KW_VISIBILITY]         = "visibility"
};

static int
format(size_t index, void *data)
{
	struct self *self = data;

	switch (index) {
	case KW_AUTHOR:
		if (self->image)
			khtml_printf(&self->html, "%s", self->image->author);
		break;
	case KW_DEFAULT_AUTHOR:
		khtml_printf(&self->html, "%s", TMP_DEFAULT_AUTHOR);
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
		if (self->image)
			khtml_printf(&self->html, "%s", tmpupd_expiresin(self->image->end));
		break;
	case KW_ID:
		if (self->image)
			khtml_printf(&self->html, "%s", self->image->id);
		break;
	case KW_TITLE:
		if (self->image)
			khtml_printf(&self->html, "%s", self->image->title);
		break;
	case KW_VISIBILITY:
		if (self->image)
			khtml_printf(&self->html, "%s", tmpupd_visibility(self->image->visible));
		break;
	default:
		break;
	}

	return 1;
}

static int
find(struct image *img, const char *id)
{
	struct db db;
	int rv;

	if (tmpupd_open(&db, DB_RDONLY) < 0)
		return -1;

	rv = db_image_get(img, id, &db);
	db_finish(&db);

	return rv;
}

static void
render(struct kreq *r, const struct image *image, const unsigned char *html, size_t htmlsz)
{
	struct self self = {
		.req = r,
		.image = image
	};
	struct ktemplate kt = {
		.key = keywords,
		.keysz = LEN(keywords),
		.cb = format,
		.arg = &self
	};

	khtml_open(&self.html, self.req, KHTML_PRETTY);
	route_template(self.req, "image", KHTTP_200, &kt, html, htmlsz);
	khtml_close(&self.html);
}

static void
get(struct kreq *r, const char * const *args)
{
	struct image image;

	switch (find(&image, args[0])) {
	case 1:
		render(r, &image, html_image, sizeof (html_image));
		image_finish(&image);
		break;
	case 0:
		route_status(r, KHTTP_404, KMIME_TEXT_HTML);
		break;
	default:
		route_status(r, KHTTP_500, KMIME_TEXT_HTML);
		break;
	}
}

static void
get_download(struct kreq *r, const char * const *args)
{
	struct image image;

	switch (find(&image, args[0])) {
	case 1:
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_APP_OCTET_STREAM]);
		khttp_head(r, kresps[KRESP_CONTENT_LENGTH], "%zu", image.datasz);
		khttp_head(r, kresps[KRESP_CONNECTION], "keep-alive");
		khttp_head(r, kresps[KRESP_CONTENT_DISPOSITION],
		    "attachment; filename=\"%s\"", image.filename);
		khttp_body(r);
		khttp_write(r, (const char *)image.data, image.datasz);
		image_finish(&image);
		break;
	case 0:
		route_status(r, KHTTP_404, KMIME_TEXT_HTML);
		break;
	default:
		route_status(r, KHTTP_500, KMIME_TEXT_HTML);
		break;
	}
}

static void
get_new(struct kreq *r)
{
	/*
	 * Reuse render with a NULL image because in contrast to paste images
	 * can't be forked but we do have similar keywords in both HTML
	 * templates though.
	 */
	render(r, NULL, html_image_new, sizeof (html_image_new));
}

static void
post(struct kreq *r)
{
	struct db db;
	struct image image;
	const char *title = NULL,
	           *author = NULL,
	           *filename = NULL,
	           *duration = "day";
	const unsigned char *data = NULL;
	time_t start, end;
	int visible = 0;
	size_t datasz = 0;

	if (tmpupd_open(&db, DB_RDWR) < 0) {
		route_status(r, KHTTP_500, KMIME_TEXT_HTML);
		return;
	}

	for (size_t i = 0; i < r->fieldsz; ++i) {
		if (tmpupd_isdef(&r->fields[i], "title"))
			title = r->fields[i].val;
		else if (tmpupd_isdef(&r->fields[i], "author"))
			author = r->fields[i].val;
		else if (tmpupd_isdef(&r->fields[i], "filename"))
			filename = r->fields[i].val;
		else if (tmpupd_isdef(&r->fields[i], "duration"))
			duration = r->fields[i].val;
		else if (strcmp(r->fields[i].key, "file") == 0) {
			if (r->fields[i].file)
				filename = r->fields[i].file;

			data = (const unsigned char *)r->fields[i].val;
			datasz = r->fields[i].valsz;
		} else if (tmpupd_isdef(&r->fields[i], "visible"))
			visible = strcmp(r->fields[i].val, "on") == 0;
	}

	tmpupd_condamn(&start, &end, duration);
	image_init(&image, NULL, title, author, filename, data, datasz, start, end, visible);

	if (db_image_save(&image, &db) < 0) {
		log_warn(TAG "unable to create image: %s", db.error);
		route_status(r, KHTTP_500, KMIME_TEXT_HTML);
	} else {
		/* Redirect to image details. */
		log_debug(TAG "created new image '%s'", image.id);
		khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_302]);
		khttp_head(r, kresps[KRESP_LOCATION], "/image/%s", image.id);
		khttp_body(r);
	}

	image_finish(&image);
	db_finish(&db);
}

void
route_image(struct kreq *r, const char * const *args)
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
		route_status(r, KHTTP_400, KMIME_TEXT_HTML);
		break;
	}
}

void
route_image_download(struct kreq *r, const char * const *args)
{
	assert(r);
	assert(args);

	switch (r->method) {
	case KMETHOD_GET:
		get_download(r, args);
		break;
	default:
		route_status(r, KHTTP_400, KMIME_TEXT_HTML);
		break;
	}
}

void
route_image_new(struct kreq *r, const char * const *args)
{
	assert(r);
	assert(args);

	switch (r->method) {
	case KMETHOD_GET:
		get_new(r);
		break;
	case KMETHOD_POST:
		post(r);
		break;
	default:
		route_status(r, KHTTP_400, KMIME_TEXT_HTML);
		break;
	}
}
