#ifndef TMPUP_PASTE_H
#define TMPUP_PASTE_H

#include <stddef.h>
#include <time.h>

struct paste {
	char *id;
	char *title;
	char *author;
	char *filename;
	char *language;
	char *code;
	time_t start;
	time_t end;
};

extern const char * const paste_langs[];
extern const size_t paste_langsz;

void
paste_init(struct paste *paste,
           const char *id,
           const char *title,
           const char *author,
           const char *filename,
           const char *language,
           const char *code,
           time_t start,
           time_t end);

void
paste_finish(struct paste *paste);

char *
paste_dump(const struct paste *paste);

int
paste_parse(struct paste *paste, const char *text);

#endif /* !TMPUP_PASTE_H */
