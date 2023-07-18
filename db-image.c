#include <assert.h>

#include "db-image.h"
#include "db.h"
#include "image.h"

#include "sql/image-delete.h"
#include "sql/image-get.h"
#include "sql/image-recents.h"

static void
get(sqlite3_stmt *stmt, void *data)
{
	struct image *image = data;

	image_init(image,
		(const char *)sqlite3_column_text(stmt, 0),
		(const char *)sqlite3_column_text(stmt, 1),
		(const char *)sqlite3_column_text(stmt, 2),
		(const char *)sqlite3_column_text(stmt, 3),
		(const char *)sqlite3_column_text(stmt, 4),
		(time_t)sqlite3_column_int64(stmt, 5),
		(time_t)sqlite3_column_int64(stmt, 6)
	);
}

int
db_image_save(struct image *image, struct db *db)
{
	assert(image);
	assert(db);

	return db_execf(db, "ssssstt",
	    image->id,
	    image->title,
	    image->author,
	    image->filename,
	    image->data,
	    image->start,
	    image->end
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
