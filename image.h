/*
 * image.h -- image definition
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

#ifndef TMP_IMAGE_H
#define TMP_IMAGE_H

/**
 * \file image.h
 * \brief Image definition
 */

#include <stddef.h>
#include <time.h>

/**
 * \struct image
 * \brief Image definition structure
 */
struct image {
	/**
	 * (read-write)
	 *
	 * Unique identifier.
	 */
	char *id;

	/**
	 * (read-write)
	 *
	 * Image title.
	 */
	char *title;

	/**
	 * (read-write)
	 *
	 * Image author.
	 */
	char *author;

	/**
	 * (read-write)
	 *
	 * Image filename.
	 */
	char *filename;

	/**
	 * (read-write)
	 *
	 * Image content uncompressed as-is.
	 */
	unsigned char *data;

	/**
	 * (read-write)
	 *
	 * Image length.
	 */
	size_t datasz;

	/**
	 * (read-write)
	 *
	 * Creation date.
	 */
	time_t start;

	/**
	 * (read-write)
	 *
	 * Expiration date.
	 */
	time_t end;

	/**
	 * (read-write)
	 *
	 * If non-zero lists the image in the index and searches.
	 */
	int visible;
};

/**
 * Initialize the image copying parameters into local fields.
 *
 * Previous fields are ignored.
 *
 * \pre image != NULL
 * \pre end > start
 * \param image image to initialize
 * \param id optional unique id (can be NULL)
 * \param title optional title
 * \param author optional author
 * \param filename optional filename
 * \param data the image file content
 * \param datasz image content length
 * \param start image creation date
 * \param end image expiration date
 * \param visible non-zero to make the image public
 */
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
           int visible);

/**
 * Produce a JSON representation of that image.
 *
 * \pre image != NULL
 * \param image the image to dump
 * \return the JSON content as a dynamically allocated string
 */
char *
image_dump(const struct image *image);

/**
 * Try to parse the JSON text as an image.
 *
 * \pre image != NULL
 * \param text JSON document to parse
 * \param error the error string to fill
 * \param errosz maximum error length
 * \return 0 on success or -1 on error
 */
int
image_parse(struct image *image, const char *text, char *error, size_t errorsz);

/**
 * Cleanup the image.
 *
 * \pre image != NULL
 * \param image the image to cleanup
 */
void
image_finish(struct image *image);

#endif /* !TMP_IMAGE_H */
