/*
 * db-image.h -- storage helpers for a image
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

#ifndef TMPUPD_DB_IMAGE_H
#define TMPUPD_DB_IMAGE_H

/**
 * \file db-image.h
 * \brief Storage helpers for a image.
 */

#include <sys/types.h>

struct db;
struct image;

/**
 * Save an image into the database.
 *
 * Field id must be set prior to insertion.
 *
 * \pre img != NULL
 * \pre db != NULL
 * \param img the image
 * \param db the database
 * \return 0 on success or -1 on error
 */
int
db_image_save(struct image *img, struct db *db);

/**
 * Get a unique image from database.
 *
 * \pre img != NULL
 * \pre id != NULL
 * \pre db != NULL
 * \param img the image
 * \param id the image identifier
 * \param db the database
 * \return 1 if found, 0 if not found or -1 on error
 */
int
db_image_get(struct image *img, const char *id, struct db *db);

/**
 * Get a list of most recent images.
 *
 * \pre imgs != NULL
 * \pre db != NULL
 * \param imgs array of images to fill
 * \param imgsz number of images to load at most
 * \param db the database
 * \return the number of images loaded or -1 on error
 */
ssize_t
db_image_recents(struct image *imgs, size_t imgsz, struct db *db);

/**
 * Delete the specified image from database.
 *
 * The image isn't cleaned up, ::image_finish must still be called.
 *
 * \pre img != NULL
 * \pre db != NULL
 * \param img the image
 * \param db the database
 * \return 0 on success or -1 on error
 */
int
db_image_delete(struct image *img, struct db *db);

/**
 * Delete outdated images from database.
 *
 * \pre db != NULL
 * \param db the database
 * \return 0 on success or -1 on error
 */
int
db_image_prune(struct db *db);

#endif /* TMPUPD_DB_IMAGE_H */
