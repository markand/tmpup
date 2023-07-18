#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "image.h"
#include "tmp.h"
#include "util.h"

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
	assert(data);

	if (id)
		image->id = estrdup(id);
	else
		image->id = NULL;

	image->title = estrdup(title ? title : TMP_DEFAULT_TITLE);
	image->author = estrdup(author ? author : TMP_DEFAULT_AUTHOR);
	image->filename = estrdup(filename ? filename : TMP_DEFAULT_FILENAME);
	image->data = estrdup(data);
	image->start = start;
	image->end = end;
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
image_parse(struct image *image, const char *text, char *error, size_t errorsz)
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

	if (rv < 0) {
		bstrlcpy(error, err.text, errorsz);
		return -1;
	}
	if (check_duration(start, end, error, errorsz) < 0)
		return -1;

	image_init(image, NULL, title, author, filename, data, start, end);
	json_decref(doc);

	return 0;
}
