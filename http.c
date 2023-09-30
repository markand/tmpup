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
#include "route-api-v0-image.h"
#include "route-api-v0-paste.h"
#include "route-image.h"
#include "route-index.h"
#include "route-paste.h"
#include "route-static.h"
#include "route.h"
#include "util.h"

/* Need to use this to avoid "uninitialized regex field". smh. */
#define GET(p, e)       { .method = KMETHOD_GET,  .path = p, .exec = e }
#define POST(p, e)      { .method = KMETHOD_POST, .path = p, .exec = e }

struct route {
	enum kmethod method;
	const char *path;
	void (*exec)(struct kreq *, const char * const *);
	regex_t regex;
};

static struct route routes[] = {
	GET  ("^/$",                            route_index),
	GET  ("^/image/download/([a-z0-9]+)$",  route_image_download),
	GET  ("^/image/new",                    route_image_new),
	POST ("^/image/new",                    route_image_new),
	GET  ("^/image/([a-z0-9]+)$",           route_image),
	GET  ("^/paste/download/([a-z0-9]+)$",  route_paste_download),
	GET  ("^/paste/fork/([a-z0-9]+)?$",     route_paste_new),
	GET  ("^/paste/raw/([a-z0-9]+)$",       route_paste_raw),
	GET  ("^/paste/new",                    route_paste_new),
	POST ("^/paste/new",                    route_paste_new),
	GET  ("^/paste/([a-z0-9]+)$",           route_paste),
	POST ("^/api/v0/image$",                route_api_v0_image),
	POST ("^/api/v0/paste$",                route_api_v0_paste),
	GET  ("^/static/(.*)",                  route_static)
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

static void
process(struct kreq *r)
{
	assert(r);

	regmatch_t matches[8];
	struct route *route = NULL, *iter;
	char **args;

	for (size_t i = 0; i < LEN(routes); ++i) {
		iter = &routes[i];

		if (r->method != iter->method)
			continue;
		if (regexec(&iter->regex, r->fullpath, LEN(matches), matches, 0) == 0) {
			route = iter;
			break;
		}
	}

	if (!route)
		route_status(r, KHTTP_404, KMIME_TEXT_HTML);
	else {
		args = makeargs(r->fullpath, matches, LEN(matches));
		route->exec(r, (const char * const *)args);
		freeargs(args);
	}
}

static void *
routine(void *data)
{
	(void)data;

	sigset_t sigs;
	struct kfcgi *fcgi;
	struct kreq req;
	int run = 1;

	/* No need to block anything. */
	sigemptyset(&sigs);
	pthread_sigmask(SIG_SETMASK, &sigs, NULL);

	if (khttp_fcgi_init(&fcgi, NULL, 0, NULL, 0, 0) != KCGI_OK)
		die("abort: could not allocate FastCGI");

	while (run) {
		if (khttp_fcgi_parse(fcgi, &req) == KCGI_OK) {
			process(&req);
			khttp_free(&req);
		} else {
			/* Indicate to main thread to quit. */
			kill(getpid(), SIGINT);
			run = 0;
		}
	}

	khttp_fcgi_free(fcgi);

	return NULL;
}

void
http_init(void)
{
	struct route *route;
	int rv;
	char errstr[128] = "unknown error";

	for (size_t i = 0; i < LEN(routes); ++i) {
		route = &routes[i];
		rv = regcomp(&route->regex, route->path, REG_EXTENDED | REG_ICASE);

		if (rv != 0) {
			regerror(rv, &route->regex, errstr, sizeof (errstr));
			die("abort: regex failed: %s\n", errstr);
		}
	}

	if ((rv = pthread_create(&thread, NULL, routine, NULL)) != 0)
		die("abort: pthread_create: %s\n", strerror(rv));
}

void
http_finish(void)
{
	pthread_kill(thread, SIGTERM);
	pthread_join(thread, NULL);

	for (size_t i = 0; i < LEN(routes); ++i)
		regfree(&routes[i].regex);
}
