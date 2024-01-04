/*
 * check.c -- miscellaneous helpers for parsing data types
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
#include <stdlib.h>
#include <string.h>

#include <magic.h>

#include "check.h"
#include "paste.h"
#include "tmp.h"
#include "util.h"

#if defined(WITH_MAGIC)
static magic_t cookie;
#endif

static int
cmp_str(const void *key, const void *val)
{
	return strcmp(key, *(const char **)val);
}

int
check_init(void)
{
	int rv = 0;

#if defined(WITH_MAGIC)
	if (!(cookie = magic_open(MAGIC_SYMLINK | MAGIC_MIME_TYPE)))
		return -1;
	if (magic_load(cookie, NULL) < 0)
		return -1;
#endif

	return rv;
}

int
check_language(const char *lang, char *error, size_t errorsz)
{
	assert(lang);
	assert(error);

	const char *iter;

	if (!lang)
		return -1;
	if (!(iter = bsearch(lang, paste_langs, paste_langsz, sizeof (char *), cmp_str))) {
		snprintf(error, errorsz, "invalid language '%s'", lang);
		return -1;
	}

	return 0;
}

int
check_duration(time_t start, time_t end, char *error, size_t errorsz)
{
	assert(error);

	/* Really? */
	if (end <= start) {
		snprintf(error, errorsz, "end date is prior start");
		return -1;
	}

	/* Don't think your upload is that notorious. */
	if (end - start >= TMP_DURATION_MONTH) {
		snprintf(error, errorsz, "too long duration");
		return -1;
	}

	return 0;
}

int
check_image(const void *data, size_t datasz)
{
#if defined(WITH_MAGIC)
	assert(data);

	static const char * const mimes[] = {
		"image/png",
		"image/jpeg"
	};
	const char *rv;

	/* No cookie? assume yes as last resort. */
	if (!cookie)
		return 0;

	if (!(rv = magic_buffer(cookie, data, datasz))) {
		log_warn(TAG "magic_buffer: %s", magic_error(cookie));
		return 0;
	}

	for (size_t i = 0; i < LEN(mimes); ++i)
		if (strcmp(rv, mimes[i]) == 0)
			return 0;

	return -1;
#else
	/* No libmagic, accept everything. */
	(void)data;
	(void)datasz;

	return 1;
#endif
}

void
check_finish(void)
{
#if defined(WITH_MAGIC)
	if (cookie)
		magic_close(cookie);
#endif
}
