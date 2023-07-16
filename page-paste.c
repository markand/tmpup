#include <assert.h>

#include "db-paste.h"
#include "db.h"
#include "http.h"
#include "log.h"
#include "page-paste.h"
#include "page.h"
#include "paste.h"
#include "tmpupd.h"

#define TAG "page-paste: "

static void
get(struct kreq *r, const char * const *args)
{
	struct db db;
	struct paste paste;
	struct khtmlreq html;

	if (tmpupd_open(&db, DB_RDONLY) < 0) {
		page_status(r, KHTTP_500, KMIME_TEXT_HTML);
		return;
	}

	// TODO: check between not found and error.
	log_debug(TAG "searching paste '%s'", args[0]);

	if (db_paste_get(&paste, args[0], &db) < 0)
		page_status(r, KHTTP_400, KMIME_TEXT_HTML);
	else {
		khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
		khttp_body(r);

		// TODO: real page here.
		khtml_open(&html, r, 0);
		khtml_printf(&html, "%s\n", paste.code);
		khtml_close(&html);
	}

	db_finish(&db);
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
