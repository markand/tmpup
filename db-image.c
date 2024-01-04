/*
 * db-image.c -- storage helpers for a image
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

#include <assert.h>

#include "db-image.h"
#include "db.h"
#include "image.h"

#include "sql/image-delete.h"
#include "sql/image-get.h"
#include "sql/image-recents.h"
#include "sql/image-save.h"
#include "sql/image-prune.h"

static void
get(sqlite3_stmt *stmt, void *data)
{
	struct image *image = data;

	image_init(image,
		(const char *)sqlite3_column_text(stmt, 1),
		(const char *)sqlite3_column_text(stmt, 2),
		(const char *)sqlite3_column_text(stmt, 3),
		(const char *)sqlite3_column_text(stmt, 4),
		(const unsigned char *)sqlite3_column_blob(stmt, 5),
		(size_t)sqlite3_column_int64(stmt, 0),
		(time_t)sqlite3_column_int64(stmt, 6),
		(time_t)sqlite3_column_int64(stmt, 7),
		(int)sqlite3_column_int(stmt, 8)
	);
}

int
db_image_save(struct image *image, struct db *db)
{
	assert(image);
	assert(db);

	return db_insert(db, (const char *)sql_image_save, "ssssbttd",
		image->id,
		image->title,
		image->author,
		image->filename,
		image->data, image->datasz,
		image->start,
		image->end,
		image->visible
	);
}

int
db_image_get(struct image *image, const char *id, struct db *db)
{
	assert(image);
	assert(id);
	assert(db);

	struct db_select select = {
		.data = image,
		.datasz = 1,
		.elemsz = sizeof (*image),
		.get = get
	};

	return db_select(db, &select, (const char *)sql_image_get, "s", id);
}

ssize_t
db_image_recents(struct image *images, size_t imagesz, struct db *db)
{
	assert(images);
	assert(db);

	struct db_select select = {
		.data = images,
		.datasz = imagesz,
		.elemsz = sizeof (*images),
		.get = get
	};

	return db_select(db, &select, (const char *)sql_image_recents, "z", imagesz);
}

int
db_image_delete(struct image *image, struct db *db)
{
	assert(image);

	return db_execf(db, (const char *)sql_image_delete, "s", image->id);
}

int
db_image_prune(struct db *db)
{
	assert(db);

	return db_execf(db, (const char *)sql_image_prune, "t", time(NULL));
}
