#ifndef TMPUP_IMAGE_H
#define TMPUP_IMAGE_H

#include <time.h>

struct db;

#define IMAGE_DEFAULT_TITLE     "blank"
#define IMAGE_DEFAULT_AUTHOR    "anonymous"
#define IMAGE_DEFAULT_FILENAME  "blank"

struct image {
	char *id;
	char *title;
	char *author;
	char *filename;
	char *data;
	time_t start;
	time_t end;
};

void
image_init(struct image *image,
           const char *id,
           const char *title,
           const char *author,
           const char *filename,
           const char *data,
           time_t start,
           time_t end);

int
image_save(struct image *, struct db *);

int
image_delete(struct image *, struct db *);

void
image_finish(struct image *);

char *
image_dump(const struct image *);

int
image_parse(struct image *, const char *);

#endif /* !TMPUP_IMAGE_H */
