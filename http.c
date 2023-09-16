/*
 * http.c -- HTTP request handling
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
#include <pthread.h>
#include <regex.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "log.h"
#include "page-api-v0-image.h"
#include "page-api-v0-paste.h"
#include "page-image.h"
#include "page-index.h"
#include "page-paste.h"
#include "page.h"
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
	GET  ("^/$",                            page_index),
	GET  ("^/image/([a-z0-9]+)$",           page_image),
	GET  ("^/paste/([a-z0-9]+)$",           page_paste),
	GET  ("^/image/raw/([a-z0-9]+)$",       page_image_raw),
#if 0
	GET  ("^/paste/raw/([a-z0-9]+)$",       page_paste_raw),
#endif
	POST ("^/api/v0/image$",                page_api_v0_image),
	POST ("^/api/v0/paste$",                page_api_v0_paste)
};

static pthread_t thread;

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

static void *
routine(void *data)
{
	(void)data;

	struct kfcgi *fcgi;
	struct kreq req;
	int run = 1;

	if (khttp_fcgi_init(&fcgi, NULL, 0, NULL, 0, 0) != KCGI_OK)
		die("abort: could not allocate FastCGI");

	while (run) {
		/* TODO: check return code. */
		switch (khttp_fcgi_parse(fcgi, &req)) {
		case KCGI_OK:
			http_process(&req);
			khttp_free(&req);
			break;
		case KCGI_EXIT:
			/* This is received from the main thread. */
			run = 0;
			break;
		default:
			/*
			 * Other error, break the loop and send SIGINT to main
			 * thread to stop the whole application.
			 */
			raise(SIGINT);
			run = 0;
			break;
		}
	}

	khttp_fcgi_free(fcgi);

	return NULL;
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

	if ((rv = pthread_create(&thread, NULL, routine, NULL)) != 0)
		die("abort: pthread_create: %s\n", strerror(rv));
}

void
http_process(struct kreq *r)
{
	assert(r);

	regmatch_t matches[8] = {};
	struct page *page = NULL, *iter;
	char **args;

	printf("=> [%s]\n", r->fullpath);

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

	pthread_join(thread, NULL);
}
