#ifndef TMPUPD_DB_IMAGE_H
#define TMPUPD_DB_IMAGE_H

#include <sys/types.h>

struct db;
struct image;

int
db_image_save(struct image *, struct db *);

int
db_image_get(struct image *, const char *, struct db *);

ssize_t
db_image_recents(struct image *, size_t, struct db *);

int
db_image_delete(struct image *, struct db *);

#endif /* TMPUPD_DB_IMAGE_H */
