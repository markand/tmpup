/*
 * tmpupd.h -- tmpupd main files
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

#ifndef TMPUPD_H
#define TMPUPD_H

#include <time.h>

struct db;
enum db_mode;

/**
 * Convenient function to open and initialize the database depending on the
 * mode given. Also logs an error message if it fails.
 *
 * \pre db != NULL
 * \param db the database to initialize
 * \param mode the desired mode
 * \return 0 on success or -1 on error
 */
int
tmpupd_open(struct db *db, enum db_mode mode);

/**
 * Returns a static string with a human format telling the duration left for
 * the item using its expiration timestamp.
 *
 * \param end UTC end timestamp
 * \return a static thread local string with the left time
 */
const char *
tmpupd_expiresin(time_t end);

/**
 * Returns either "visible" or "hidden" depending if val is zero or non-zero
 * respectively.
 *
 * \param val the item visibility.
 * \return a compile-time string
 */
const char *
tmpupd_visibility(int val);

/**
 * Sets start and end arguments to the appropriate time depending on the
 * duration field which can be one of: `hour`, `day`, `week`, `month`.
 *
 * The argument start is always set to the current system time, end will be
 * computed from start plus the amount of seconds.
 *
 * \pre start != NULL
 * \pre end != NULL
 * \pre duration != NULL
 * \param start UTC start timestamp to set
 * \param end UTC end timestamp to set
 * \param duration string
 */
void
tmpupd_condamn(time_t *start, time_t *end, const char *duration);

#endif /* !TMPUPD_H */
