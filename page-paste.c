#include <assert.h>

#include "db.h"
#include "http.h"
#include "page-paste.h"
#include "page-status.h"
#include "tmpupd.h"

void
page_paste(struct kreq *r, const char * const *args)
{
	assert(r);
	assert(args);

	struct db db;

	if (tmpupd_open(&db, DB_RDONLY) < 0) {
		page_status(r, KHTTP_500);
	} else {
		khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
		khttp_body(r);
	}
}
