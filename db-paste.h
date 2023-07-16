#ifndef TMPUPD_DB_PASTE_H
#define TMPUPD_DB_PASTE_H

struct db;
struct paste;

int
db_paste_save(struct paste *, struct db *);

int
db_paste_get(struct paste *, const char *, struct db *);

int
db_paste_delete(struct paste *, struct db *);

#endif /* TMPUPD_DB_PASTE_H */
