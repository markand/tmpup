/*
 * route-index.c -- route /
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
#include <stdarg.h>
#include <stdio.h>

#include "db-image.h"
#include "db-paste.h"
#include "db.h"
#include "http.h"
#include "image.h"
#include "paste.h"
#include "route-index.h"
#include "route.h"
#include "tmpupd.h"
#include "util.h"

#include "html/index.h"

#define LIMIT 10

struct self {
	struct db db;
	struct kreq *req;
	struct khtmlreq html;
};

enum {
	KW_IMAGES,
	KW_PASTES
};

static const char * const keywords[] = {
	[KW_IMAGES] = "images",
	[KW_PASTES] = "pastes",
};

static const char *
url(const char *fmt, ...)
{
	static _Thread_local char ret[128];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(ret, sizeof (ret), fmt, ap);
	va_end(ap);

	return ret;
}

static void
format_pastes(struct self *self)
{
	struct paste pastes[LIMIT] = {}, *p;
	ssize_t pastesz;

	if ((pastesz = db_paste_recents(pastes, LEN(pastes), &self->db)) < 0)
		return;

	for (ssize_t i = 0; i < pastesz; ++i) {
		p = &pastes[i];

		khtml_elem(&self->html, KELEM_TR);

		/* id */
		khtml_elem(&self->html, KELEM_TD);
		khtml_attr(&self->html, KELEM_A,
		    KATTR_HREF, url("paste/%s", p->id),
		    KATTR__MAX);
		khtml_printf(&self->html, "%s", p->id);
		khtml_closeelem(&self->html, 2);

		/* title */
		khtml_elem(&self->html, KELEM_TD);
		khtml_printf(&self->html, "%s", p->title);
		khtml_closeelem(&self->html, 1);

		/* author */
		khtml_elem(&self->html, KELEM_TD);
		khtml_printf(&self->html, "%s", p->author);
		khtml_closeelem(&self->html, 1);

		/* language */
		khtml_elem(&self->html, KELEM_TD);
		khtml_printf(&self->html, "%s", p->language);
		khtml_closeelem(&self->html, 1);

		/* language */
		khtml_elem(&self->html, KELEM_TD);
		khtml_printf(&self->html, "expiration");
		khtml_closeelem(&self->html, 1);

		khtml_closeelem(&self->html, 0);

		paste_finish(&pastes[i]);
	}
}

static void
format_images(struct self *self)
{
	struct image images[LIMIT] = {}, *img;
	ssize_t imagesz;

	if ((imagesz = db_image_recents(images, LEN(images), &self->db)) < 0)
		return;

	for (ssize_t i = 0; i < imagesz; ++i) {
		img = &images[i];

		khtml_elem(&self->html, KELEM_TR);

		/* id */
		khtml_elem(&self->html, KELEM_TD);
		khtml_attr(&self->html, KELEM_A,
		    KATTR_HREF, url("image/%s", img->id),
		    KATTR__MAX);
		khtml_printf(&self->html, "%s", img->id);
		khtml_closeelem(&self->html, 2);

		/* title */
		khtml_elem(&self->html, KELEM_TD);
		khtml_printf(&self->html, "%s", img->title);
		khtml_closeelem(&self->html, 1);

		/* author */
		khtml_elem(&self->html, KELEM_TD);
		khtml_printf(&self->html, "%s", img->author);
		khtml_closeelem(&self->html, 1);

		/* expiration */
		khtml_elem(&self->html, KELEM_TD);
		khtml_printf(&self->html, "%s", tmpupd_expiresin(img->start, img->end));
		khtml_closeelem(&self->html, 1);

		khtml_closeelem(&self->html, 0);

		image_finish(img);
	}
}

static int
format(size_t index, void *arg)
{
	switch (index) {
	case KW_PASTES:
		format_pastes(arg);
		break;
	case KW_IMAGES:
		format_images(arg);
		break;
	default:
		break;
	}

	return 1;
}

void
route_index(struct kreq *r, const char * const *args)
{
	(void)args;

	assert(r);

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
		route_status(r, KHTTP_500, KMIME_TEXT_HTML);
		return;
	}

	khtml_open(&self.html, self.req, 0);
	route_template(r, "tmpup", KHTTP_200, &kt, html_index, sizeof (html_index));
	khtml_close(&self.html);

	db_finish(&self.db);
}
