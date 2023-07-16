#include <assert.h>

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
	KW_CODE
};

static const char * const keywords[] = {
	[KW_CODE] = "code"
};

static int
format(size_t index, void *data)
{
	struct self *self = data;

	switch (index) {
	case KW_CODE:
		khtml_printf(&self->html, "%s", self->paste.code);
		break;
	default:
		break;
	}

	return 1;
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

	if (db_paste_get(&self.paste, args[0], &self.db) < 0)
		page_status(r, KHTTP_404, KMIME_TEXT_HTML);
	else {
		khtml_open(&self.html, self.req, 0);
		page_template(self.req, KHTTP_200, &kt, html_paste, sizeof (html_paste));
		khtml_close(&self.html);
	}

	db_finish(&self.db);
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
