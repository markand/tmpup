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

int
tmpupd_open(struct db *, enum db_mode);

/**
 * Create a somewhat unique identifier as much as we can.
 *
 * \return a newly allocated identifier (to be free'd)
 */
char *
tmpupd_newid(void);

const char *
tmpupd_expiresin(time_t start, time_t end);

int
tmpupd_isimage(const char *data, size_t datasz);

void
tmpupd_condamn(time_t *start, time_t *end, const char *duration);

#endif /* !TMPUPD_H */
