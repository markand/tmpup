/*
 * route-image.h -- route /image
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

#ifndef TMPUPD_ROUTE_IMAGE
#define TMPUPD_ROUTE_IMAGE

/**
 * \file route-image.h
 * \brief Routes /image.
 */

struct kreq;

/**
 * Implement /image route.
 */
void
route_image(struct kreq *r, const char * const *args);

/**
 * Implement /image/download/<id> route.
 */
void
route_image_download(struct kreq *r, const char * const *args);

/**
 * Implement /image/new route.
 */
void
route_image_new(struct kreq *r, const char * const *args);

#endif /* !TMPUPD_ROUTE_IMAGE */
