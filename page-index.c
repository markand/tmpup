#include <assert.h>

#include "http.h"
#include "page-index.h"

void
page_index(struct kreq *r, const char * const *args)
{
	(void)args;

	assert(r);

	khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_body(r);
	khttp_puts(r, "index\n");
}
