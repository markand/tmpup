#ifndef TMPUPD_DB_IMAGE_H
#define TMPUPD_DB_IMAGE_H

struct db;
struct image;

int
db_image_save(struct image *, struct db *);

int
db_image_delete(struct image *, struct db *);

#endif /* TMPUPD_DB_IMAGE_H */
