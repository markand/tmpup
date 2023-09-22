/*
 * route-api-v0-paste.c -- route /api/v0/paste
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

#include "db-paste.h"
#include "db.h"
#include "http.h"
#include "log.h"
#include "paste.h"
#include "route-api-v0-paste.h"
#include "route.h"
#include "tmpupd.h"

#define TAG "route-api-v0-paste: "

static void
post(struct kreq *r)
{
	struct paste paste;
	struct db db;
	char error[128] = "invalid input";

	if (!r->fieldsz || paste_parse(&paste, r->fields[0].val, error, sizeof (error)) < 0) {
		route_json(r, KHTTP_400, "{ss}", "error", error);
		return;
	}

	if (tmpupd_open(&db, DB_RDWR) < 0)
		route_status(r, KHTTP_500, KMIME_APP_JSON);
	else {
		if (db_paste_save(&paste, &db) < 0) {
			log_warn(TAG "unable to create paste: %s", db.error);
			route_status(r, KHTTP_500, KMIME_APP_JSON);
		} else {
			log_info(TAG "created paste '%s'", paste.id);
			route_json(r, KHTTP_201, "{ss}", "id", paste.id);
		}

		db_finish(&db);
	}

	paste_finish(&paste);
}

void
route_api_v0_paste(struct kreq *r, const char * const *args)
{
	assert(r);

	(void)args;

	switch (r->method) {
	case KMETHOD_POST:
		post(r);
		break;
	default:
		break;
	}
}
