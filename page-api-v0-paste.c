#include <assert.h>
#include <stdlib.h>

#include "db-paste.h"
#include "db.h"
#include "http.h"
#include "log.h"
#include "page-api-v0-paste.h"
#include "page.h"
#include "paste.h"
#include "tmpupd.h"

#define TAG "page-api-v0-paste: "

static void
post(struct kreq *r)
{
	struct paste paste = {0};
	struct db db;
	char error[128] = "invalid input";

	if (!r->fieldsz || paste_parse(&paste, r->fields[0].val, error, sizeof (error)) < 0) {
		page_json(r, KHTTP_400, "{ss}", "error", error);
		return;
	}

	if (tmpupd_open(&db, DB_RDWR) < 0)
		page_status(r, KHTTP_500, KMIME_APP_JSON);
	else {
		if (db_paste_save(&paste, &db) < 0) {
			log_warn(TAG "unable to create paste: %s", db.error);
			page_status(r, KHTTP_500, KMIME_APP_JSON);
		} else {
			log_info(TAG "created paste '%s'", paste.id);
			page_json(r, KHTTP_201, "{ss}", "id", paste.id);
		}

		db_finish(&db);
	}

	paste_finish(&paste);
}

void
page_api_v0_paste(struct kreq *r, const char * const *args)
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
