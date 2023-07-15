#ifndef TMPUP_PASTE_H
#define TMPUP_PASTE_H

#include <time.h>

struct db;

struct paste {
	char *id;
	char *title;
	char *author;
	char *language;
	char *code;
	time_t start;
	time_t end;
};

void
paste_init(struct paste *paste,
           const char *id,
           const char *title,
           const char *author,
           const char *language,
           const char *code,
           time_t start,
           time_t end);

int
paste_save(struct paste *paste, struct db *db);

int
paste_delete(struct paste *paste, struct db *db);

void
paste_finish(struct paste *paste);

#endif /* !TMPUP_PASTE_H */
