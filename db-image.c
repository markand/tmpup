#include <assert.h>

#include "db.h"
#include "image.h"

#include "sql/image-delete.h"

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
db_image_delete(struct image *image, struct db *db)
{
	assert(image);

	return db_execf(db, (const char *)sql_image_delete, "s", image->id);
}
