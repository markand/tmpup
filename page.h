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

/**
 * \file page.h
 * \brief Page helpers.
 */

#include <sys/types.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <kcgi.h>
#include <kcgihtml.h>

/**
 * Render the page using the HTML template.
 *
 * \pre r != NULL
 * \pre title != NULL
 * \pre html != NULL
 * \param r the kcgi request
 * \param title page title
 * \param code HTTP result code
 * \param html the HTML template
 * \param htmlsz HTML data length
 */
void
page_template(struct kreq *r,
              const char *title,
              enum khttp code,
              const struct ktemplate *kt,
              const unsigned char *html,
              size_t htmlsz);

/**
 * Create a status page either in the form of HTML or JSON depending on the
 * mime type.
 *
 * \pre r != NULL
 * \param r the kcgi request
 * \param code HTTP result code
 * \param mime the mime type (KMIME_APP_JSON or KMIME_TEXT_HTML)
 */
void
page_status(struct kreq *r, enum khttp code, enum kmime mime);

/**
 * Create a JSON result page using the jansson packing function.
 *
 * \pre r != NULL
 * \param r the kcgi request
 * \param code HTTP result code
 * \param fmt the format string to pass to json_pack
 */
void
page_json(struct kreq *r, enum khttp code, const char *fmt, ...);

#endif /* !TMPUPD_PAGE_STATUS */
