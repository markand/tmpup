#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "image.h"
#include "log.h"
#include "tmpupd.h"
#include "util.h"

#include "sql/image-delete.h"

void
image_init(struct image *image,
           const char *id,
           const char *title,
           const char *author,
           const char *filename,
           const char *data,
           time_t start,
           time_t end)
{
	assert(image);
	assert(title);
	assert(author);
	assert(filename);
	assert(data);

	if (id)
		image->id = estrdup(id);
	else
		image->id = tmpupd_newid();

	if (!title)
	image->title = estrdup(title);
	image->author = estrdup(author);
	image->filename = estrdup(filename);
	image->data = estrdup(data);
	image->start = start;
	image->end = end;
}

int
image_save(struct image *image, struct db *db)
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
image_delete(struct image *image, struct db *db)
{
	assert(image);

	return db_execf(db, (const char *)sql_image_delete, "s", image->id);
}

void
image_finish(struct image *image)
{
	assert(image);

	free(image->id);
	free(image->title);
	free(image->author);
	free(image->filename);
	free(image->data);
	memset(image, 0, sizeof (*image));
}

char *
image_dump(const struct image *image)
{
	assert(image);

	return tmp_json("{ss ss ss ss sI sI}",
		"id",           image->id,
		"title",        image->title,
		"author",       image->author,
		"data",         image->data,
		"start",        (json_int_t)image->start,
		"end",          (json_int_t)image->end
	);
}

int
image_parse(struct image *image, const char *text)
{
	const char *title = NULL, *author = NULL, *filename = NULL, *data = NULL;
	json_int_t start = 0, end = 0;
	json_t *doc;
	json_error_t err;
	int rv;

	memset(image, 0, sizeof (*image));

	rv = tmp_parse(&doc, &err, text, "{s?s s?s s?s s?I s?I}",
		"title",        &title,
		"author",       &author,
		"data",         &data,
		"start",        &start,
		"end",          &end
	);

	if (rv < 0)
		log_warn("image: parse error: %s", err.text);
	else {
		image_init(image, NULL, title, author, filename, data, start, end);
		json_decref(doc);
	}

	return rv;
}
