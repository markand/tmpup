/*
 * route.h -- route helpers
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

#ifndef TMPUPD_ROUTE_STATUS
#define TMPUPD_ROUTE_STATUS

/**
 * \file route.h
 * \brief Page helpers.
 */

#include <sys/types.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <kcgi.h>
#include <kcgihtml.h>

/**
 * Render the route using the HTML template.
 *
 * \pre r != NULL
 * \pre title != NULL
 * \pre html != NULL
 * \param r the kcgi request
 * \param title route title
 * \param code HTTP result code
 * \param html the HTML template
 * \param htmlsz HTML data length
 */
void
route_template(struct kreq *r,
              const char *title,
              enum khttp code,
              const struct ktemplate *kt,
              const unsigned char *html,
              size_t htmlsz);

/**
 * Create a status route either in the form of HTML or JSON depending on the
 * mime type.
 *
 * \pre r != NULL
 * \param r the kcgi request
 * \param code HTTP result code
 * \param mime the mime type (KMIME_APP_JSON or KMIME_TEXT_HTML)
 */
void
route_status(struct kreq *r, enum khttp code, enum kmime mime);

/**
 * Create a JSON result route using the jansson packing function.
 *
 * \pre r != NULL
 * \param r the kcgi request
 * \param code HTTP result code
 * \param fmt the format string to pass to json_pack
 */
void
route_json(struct kreq *r, enum khttp code, const char *fmt, ...);

#endif /* !TMPUPD_ROUTE_STATUS */
