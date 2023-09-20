/*
 * page.h -- page helpers
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

#ifndef TMPUPD_PAGE_STATUS
#define TMPUPD_PAGE_STATUS

#include <stddef.h>

struct kreq;
struct ktemplate;

enum khttp;
enum kmime;

void
page_template(struct kreq *r,
              const char *title,
              enum khttp code,
              const struct ktemplate *kt,
              const unsigned char *html,
              size_t htmlsz);

void
page_status(struct kreq *r, enum khttp code, enum kmime mime);

void
page_json(struct kreq *r, enum khttp code, const char *fmt, ...);

#endif /* !TMPUPD_PAGE_STATUS */
