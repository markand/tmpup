/*
 * check.c -- miscellaneous helpers for parsing data types
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
#include <stdlib.h>
#include <string.h>

#include <magic.h>

#include "paste.h"
#include "tmp.h"
#include "util.h"

static magic_t cookie;

static int
cmp_str(const void *key, const void *val)
{
	return strcmp(key, *(const char **)val);
}

void
check_init(void)
{
	if (!(cookie = magic_open(MAGIC_SYMLINK | MAGIC_MIME)))

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

void
check_finish(void)
{
}
