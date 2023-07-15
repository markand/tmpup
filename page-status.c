#include <assert.h>
#include <stdlib.h>

#include "http.h"
#include "page-status.h"
#include "util.h"

static const struct status {
	enum khttp code;
	const char *message;
} table[] = {
	{ KHTTP_404, "page not found" },
};

static int
compare(const void *key, const void *value)
{
	const enum khttp *code = key;
	const struct status *status = value;

	return status->code - *code;
}

static inline const char *
message(enum khttp code)
{
	const struct status *rv;

	rv = bsearch(&code, table, LEN(table), sizeof (*rv), compare);

	return rv ? rv->message : ":(";
}

void
page_status(struct kreq *r, enum khttp code)
{
	assert(r);

	khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[code]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_body(r);
	khttp_puts(r, message(code));
}
