#ifndef TMPUPD_DB_PASTE_H
#define TMPUPD_DB_PASTE_H

#include <sys/types.h>

struct db;
struct paste;

int
db_paste_save(struct paste *, struct db *);

int
db_paste_get(struct paste *, const char *, struct db *);

ssize_t
db_paste_recents(struct paste *, size_t, struct db *);

int
db_paste_delete(struct paste *, struct db *);

#endif /* TMPUPD_DB_PASTE_H */
