#include <assert.h>
#include <stdlib.h>
#include <regex.h>

#include "http.h"
#include "log.h"
#include "page-api-v0-paste.h"
#include "page-index.h"
#include "page-paste.h"
#include "page-status.h"
#include "util.h"

/* Need to use this to avoid "uninitialized regex field". smh. */
#define GET(p, e)       { .method = KMETHOD_GET,  .path = p, .exec = e }
#define POST(p, e)      { .method = KMETHOD_POST, .path = p, .exec = e }

struct page {
	enum kmethod method;
	const char *path;
	void (*exec)(struct kreq *, const char * const *);
	regex_t regex;
};

static struct page pages[] = {
	GET  ("^/$",                    page_index),
	POST ("^/api/v0/paste$",        page_api_v0_paste),
	GET  ("^/paste/([a-z0-9]+)$",   page_paste),
};

static inline char **
makeargs(const char *path, const regmatch_t *matches, size_t len)
{
	char **list = NULL;
	size_t listsz = 0;

	for (size_t i = 1; i < len && matches[i].rm_so != -1; ++i) {
		list = ereallocarray(list, listsz + 1, sizeof (char *));
		list[listsz++] = estrndup(path + matches[i].rm_so, matches[i].rm_eo);
	}

	/* Terminate as NULL. */
	list = ereallocarray(list, listsz + 1, sizeof (char *));
	list[listsz] = NULL;

	return list;
}

static inline void
freeargs(char **list)
{
	for (char **p = list; *p; ++p)
		free(*p);

	free(list);
}

void
http_init(void)
{
	struct page *page;
	int rv;
	char errstr[128] = "unknown error";

	for (size_t i = 0; i < LEN(pages); ++i) {
		page = &pages[i];
		rv = regcomp(&page->regex, page->path, REG_EXTENDED | REG_ICASE);

		if (rv != 0) {
			regerror(rv, &page->regex, errstr, sizeof (errstr));
			die("abort: regex failed: %s\n", errstr);
		}
	}
}

void
http_process(struct kreq *r)
{
	assert(r);

	regmatch_t matches[8] = {0};
	struct page *page = NULL, *iter;
	char **args;

	for (size_t i = 0; i < LEN(pages); ++i) {
		iter = &pages[i];

		if (r->method != iter->method)
			continue;
		if (regexec(&iter->regex, r->fullpath, LEN(matches), matches, 0) == 0) {
			page = iter;
			break;
		}
	}

	if (!page)
		page_status(r, KHTTP_404, KMIME_TEXT_HTML);
	else {
		args = makeargs(r->fullpath, matches, LEN(matches));
		page->exec(r, (const char * const *)args);
		freeargs(args);
	}
}

void
http_finish(void)
{
	for (size_t i = 0; i < LEN(pages); ++i)
		regfree(&pages[i].regex);
}
