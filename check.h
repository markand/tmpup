/*
 * check.h -- miscellaneous helpers for parsing data types
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

#ifndef TMP_CHECK_H
#define TMP_CHECK_H

/**
 * \file check.h
 * \brief Miscellaneous helpers for parsing data types.
 */

#include <stddef.h>
#include <time.h>

/**
 * Initialize check system.
 *
 * \return 0 on success or -1 on error (and sets errno)
 */
int
check_init(void);

/**
 * Check if the paste language is supported.
 *
 * \pre lang != NULL
 * \pre error != NULL
 * \param language name to check (e.g. cpp, bash)
 * \param error error string to fill
 * \param errorsz maximum error string
 * \return 0 on success or -1 if invalid
 */
int
check_language(const char *lang, char *error, size_t errorsz);

/**
 * Check if start and end times are valid.
 *
 * \param start start UTC timestamp
 * \param end end UTC timestamp
 * \pre error != NULL
 * \param error error string to fill
 * \param errorsz maximum error string
 * \return 0 on success or -1 if invalid
 */
int
check_duration(time_t start, time_t end, char *error, size_t errorsz);

/**
 * Check if image is valid.
 *
 * This function will accept images if the libmagic isn't enabled/available to
 * avoid blocking users from sending images.
 *
 * \pre data != NULL
 * \param data the image data content
 * \param datasz the image data length
 * \return 0 on success or -1 if invalid
 */
int
check_image(const void *data, size_t datasz);

/**
 * Close allocated resources.
 */
void
check_finish(void);

#endif /* !TMP_CHECK_H */
