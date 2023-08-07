#include <assert.h>
#include <stdlib.h>

#include "db-image.h"
#include "db.h"
#include "http.h"
#include "log.h"
#include "page-api-v0-image.h"
#include "page.h"
#include "image.h"
#include "tmpupd.h"

#define TAG "page-api-v0-image: "

static void
post(struct kreq *r)
{
	struct image image = {0};
	struct db db;
	char error[128] = "invalid input";

	if (!r->fieldsz || image_parse(&image, r->fields[0].val, error, sizeof (error)) < 0) {
		page_json(r, KHTTP_400, "{ss}", "error", error);
		return;
	}

	// TODO: libmagic check

	if (tmpupd_open(&db, DB_RDWR) < 0)
		page_status(r, KHTTP_500, KMIME_APP_JSON);
	else {
		if (db_image_save(&image, &db) < 0) {
			log_warn(TAG "unable to create image: %s", db.error);
			page_status(r, KHTTP_500, KMIME_APP_JSON);
		} else {
			log_info(TAG "created image '%s'", image.id);
			page_json(r, KHTTP_201, "{ss}", "id", image.id);
		}

		db_finish(&db);
	}

	image_finish(&image);
}

void
page_api_v0_image(struct kreq *r, const char * const *args)
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
