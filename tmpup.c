/*
 * tmpup.c -- main tmpup(1) file
 *
 * Copyright (c) 2023-2024 David Demelier <markand@malikania.fr>
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

#include <curl/curl.h>

#include "image.h"
#include "paste.h"
#include "tmp.h"
#include "util.h"

struct req {
	long status;
	json_t *doc;
};

static const char *title, *author, *filename, *language;
static const char *host = "http://localhost";
static long timeout = 3;
static time_t start, end;
static int debug, visible;

static void
usage(void)
{
	fprintf(stderr, "usage: tmpup image [file]\n");
	fprintf(stderr, "       tmpup paste [file]\n");
	exit(1);
}

static void
readall(const char *file, char **dst, size_t *dstsz)
{
	FILE *in = stdin, *out;
	char tmp[BUFSIZ];
	size_t nr;

	if (file && !(in = fopen(file, "r")))
		die("abort: %s: %s\n", file, strerror(errno));

	out = eopen_memstream(dst, dstsz);

	while ((nr = fread(tmp, 1, sizeof (tmp), in))) {
		if (fwrite(tmp, 1, nr, out) != nr)
			die("abort: %s\n", strerror(errno));
	}

	if (ferror(in))
		die("abort: %s\n", strerror(errno));

	/* We won't use in anymore so that's fine. */
	fclose(in);
	fclose(out);
}

static void
post(struct req *req, const char *url, const char *body)
{
	struct curl_slist *headers = NULL;
	CURL *curl;
	FILE *fp;
	char *resp = NULL, httperr[CURL_ERROR_SIZE] = {0}, path[256] = {0};
	const char *message;
	size_t respsz = 0;
	json_error_t resperr;

	memset(req, 0, sizeof (*req));

	/* Output response (should be JSON content). */
	fp = eopen_memstream(&resp, &respsz);

	/* Make URL from only path part. */
	snprintf(path, sizeof (path), "%s/%s", host, url);

	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, path);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, httperr);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	if (body) {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(body));
	} else
		curl_easy_setopt(curl, CURLOPT_POST, 1L);

#if LIBCURL_VERSION_MAJOR >= 8 || (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 85)
	curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "http;https");
#else
	curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
#endif

	if (curl_easy_perform(curl) != CURLE_OK)
		die("abort: %s\n", httperr);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &req->status);
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

	fclose(fp);

	/* Try to decode JSON. */
	if (!(req->doc = json_loads(resp, 0, &resperr)))
		die("abort: %s\n", resperr.text);

	/* For convenience, check if there is an error message. */
	if (json_unpack(req->doc, "{ss}", "error", &message) == 0)
		die("abort: %s\n", message);

	free(resp);
}

static void
req_finish(struct req *req)
{
	assert(req);

	json_decref(req->doc);
	memset(req, 0, sizeof (*req));
}

static void
cmd_image(int argc, char **argv)
{
	struct image image;
	struct req req;
	const char *id;
	char *contents = NULL, *dump, *slash;
	size_t contentsz = 0;

	readall(argc >= 2 ? argv[1] : NULL, &contents, &contentsz);

	if (argc >= 2) {
		if ((slash = strrchr(argv[1], '/')))
			filename = slash + 1;
		else
			filename = argv[1];
	}

	image_init(&image, NULL, title, author, filename,
	    (const unsigned char *)contents, contentsz, start, end, visible);
	dump = image_dump(&image);

	post(&req, "api/v0/image", dump);

	if (req.status != 201)
		die("abort: HTTP %ld\n", req.status);
	if (json_unpack(req.doc, "{ss}", "id", &id) == 0)
		printf("%s/image/%s\n", host, id);

	image_finish(&image);

	req_finish(&req);

	free(contents);
	free(dump);
}

static void
cmd_paste(int argc, char **argv)
{
	struct paste paste;
	struct req req;
	const char *id;
	char *code, *dump;
	size_t codesz;

	readall(argc >= 2 ? argv[1] : NULL, &code, &codesz);

	paste_init(&paste, NULL, title, author, filename, language, code, start, end, visible);
	dump = paste_dump(&paste);

	post(&req, "api/v0/paste", dump);

	if (req.status != 201)
		die("abort: HTTP %ld\n", req.status);
	if (json_unpack(req.doc, "{ss}", "id", &id) == 0)
		printf("%s/paste/%s\n", host, id);

	paste_finish(&paste);

	req_finish(&req);

	free(code);
	free(dump);
}

static struct cmd {
	const char *name;
	void (*exec)(int, char **);
} table[] = {
	/* keep ordered */
	{ "image", cmd_image },
	{ "paste", cmd_paste }
};

static int
cmp_cmd(const void *key, const void *val)
{
	return strcmp(key, ((const struct cmd *)val)->name);
}

static inline void
set_duration(const char *arg)
{
	if (strcmp(arg, "hour") == 0)
		end += TMP_DURATION_HOUR;
	else if (strcmp(arg, "day") == 0)
		end += TMP_DURATION_DAY;
	else if (strcmp(arg, "week") == 0)
		end += TMP_DURATION_WEEK;
	else if (strcmp(arg, "month") == 0)
		end += TMP_DURATION_MONTH;
	else
		die("abort: invalid duration '%s'\n", arg);
}

int
main(int argc, char **argv)
{
	int ch;
	struct cmd *cmd;

	opterr = 0;

	/* Start with a duration of one day (to match HTML pages). */
	start = time(NULL);
	end = start + TMP_DURATION_DAY;

	while ((ch = egetopt(argc, argv, "a:e:f:h:l:pt:v")) != -1) {
		switch (ch) {
		case 'a':
			author = optarg;
			break;
		case 'e':
			set_duration(optarg);
			break;
		case 'f':
			filename = optarg;
			break;
		case 'h':
			host = optarg;
			break;
		case 'l':
			language = optarg;
			break;
		case 'p':
			visible = 1;
			break;
		case 't':
			title = optarg;
			break;
		case 'v':
			debug = 1;
			break;
		default:
			break;
		}
	}
	
	argc -= optind;
	argv += optind;
	optind = 1;

	if (argc <= 0)
		usage();
	if (!(cmd = bsearch(argv[0], table, LEN(table), sizeof (*cmd), cmp_cmd)))
		die("abort: invalid command '%s'\n", argv[0]);

	cmd->exec(argc, argv);
}
