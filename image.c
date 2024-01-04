/*
 * image.c -- image definition
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
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include "check.h"
#include "default-image.h"
#include "image.h"
#include "tmp.h"
#include "util.h"

void
image_init(struct image *image,
           const char *id,
           const char *title,
           const char *author,
           const char *filename,
           const unsigned char *data,
           size_t datasz,
           time_t start,
           time_t end,
           int visible)
{
	assert(image);
	assert(end > start);

	memset(image, 0, sizeof (*image));

	if (id)
		image->id = estrdup(id);
	else
		image->id = tmp_id();

	image->title = estrdup(title ? title : TMP_DEFAULT_TITLE);
	image->author = estrdup(author ? author : TMP_DEFAULT_AUTHOR);
	image->filename = estrdup(filename ? filename : TMP_DEFAULT_FILENAME);

	/* Add some fun to trollers. */
	if (data) {
		image->data = ememdup(data, datasz);
		image->datasz = datasz;
	} else {
		image->data = ememdup(wow, sizeof (wow));
		image->datasz = sizeof (wow);
	}

	image->start = start;
	image->end = end;
	image->visible = visible;
}

void
image_finish(struct image *image)
{
	assert(image);

	free(image->id);
	free(image->title);
	free(image->author);
	free(image->filename);
	free(image->data);

	memset(image, 0, sizeof (*image));
}

char *
image_dump(const struct image *image)
{
	assert(image);

	/* We need to encode image data. */
	char *enc, *ret;
	size_t encsz;

	encsz = B64_ENCODE_LENGTH(image->datasz) + 8;
	enc = ecalloc(encsz + 1, 1);

	b64_encode(image->data, image->datasz, enc, encsz);

	ret = tmp_json("{ss* ss* ss* ss* ss sI sI sb}",
		"id",           image->id,
		"title",        image->title,
		"author",       image->author,
		"filename",     image->filename,
		"data",         enc,
		"start",        (json_int_t)image->start,
		"end",          (json_int_t)image->end,
		"visible",      image->visible
	);
	free(enc);

	return ret;
}

int
image_parse(struct image *image, const char *text, char *error, size_t errorsz)
{
	const char *title = NULL, *author = NULL, *filename = NULL, *data = NULL;
	json_int_t start = 0, end = 0;
	size_t datasz = 0, decsz;
	json_t *doc = NULL;
	json_error_t err;
	int rv, visible = 0;
	unsigned char *dec;

	memset(image, 0, sizeof (*image));

	rv = tmp_parse(&doc, &err, text, "{s?s s?s s?s s?s% s?I s?I s?b}",
		"title",        &title,
		"author",       &author,
		"filename",     &filename,
		"data",         &data, &datasz,
		"start",        &start,
		"end",          &end,
		"visible",      &visible
	);

	if (rv < 0) {
		bstrlcpy(error, err.text, errorsz);
		return -1;
	}
	if (check_duration(start, end, error, errorsz) < 0)
		return -1;

	/*
	 * Allocate large enough bytes to decode data but get the real number
	 * of bytes just after decoding.
	 */
	decsz = B64_DECODE_LENGTH(datasz) + 8;
	dec = ecalloc(decsz, 1);
	decsz = b64_decode(data, datasz, dec, decsz);

	if (decsz == (size_t)-1) {
		rv = -1;
		bstrlcpy(error, strerror(errno), errorsz);
	} else {
		rv = 0;
		image_init(image, NULL, title, author, filename, dec, decsz, start, end, visible);
	}

	free(dec);
	json_decref(doc);

	return rv;
}
