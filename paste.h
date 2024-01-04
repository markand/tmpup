/*
 * paste.h -- paste definition
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

#ifndef TMP_PASTE_H
#define TMP_PASTE_H

/**
 * \file paste.h
 * \brief Paste definition.
 */

#include <stddef.h>
#include <time.h>

/**
 * \struct paste
 * \brief Paste definition structure
 */
struct paste {
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
	 * Code language.
	 */
	char *language;

	/**
	 * (read-write)
	 *
	 * Code content.
	 */
	char *code;

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
	 * If non-zero lists the paste in the index and searches.
	 */
	int visible;
};

/**
 * Array of strings for every supported language.
 */
extern const char * const paste_langs[];

/**
 * Number of supported languages.
 */
extern const size_t paste_langsz;

/**
 * Initialize the paste copying parameters into local fields.
 *
 * Previous fields are ignored.
 *
 * \pre paste != NULL
 * \pre data != NULL
 * \pre end > start
 * \param paste paste to initialize
 * \param id optional unique id (can be NULL)
 * \param title optional title
 * \param author optional author
 * \param filename optional filename
 * \param language optional language code
 * \param code the paste code content
 * \param start paste creation date
 * \param end paste expiration date
 * \param visible non-zero to make the paste public
 */
void
paste_init(struct paste *paste,
           const char *id,
           const char *title,
           const char *author,
           const char *filename,
           const char *language,
           const char *code,
           time_t start,
           time_t end,
           int visible);

/**
 * Produce a JSON representation of that paste.
 *
 * \pre paste != NULL
 * \param paste the paste to dump
 * \return the JSON content as a dynamically allocated string
 */
char *
paste_dump(const struct paste *paste);

/**
 * Try to parse the JSON text as an paste.
 *
 * \pre paste != NULL
 * \param text JSON document to parse
 * \param error the error string to fill
 * \param errosz maximum error length
 * \return 0 on success or -1 on error
 */
int
paste_parse(struct paste *paste, const char *text, char *error, size_t errorsz);

/**
 * Cleanup the paste.
 *
 * \pre paste != NULL
 * \param paste the paste to cleanup
 */
void
paste_finish(struct paste *paste);


#endif /* !TMP_PASTE_H */
