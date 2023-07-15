#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "paste.h"
#include "tmpupd.h"
#include "util.h"

#include "sql/paste-save.h"

void
paste_init(struct paste *paste,
           const char *id,
           const char *title,
           const char *author,
           const char *language,
           const char *code,
           time_t start,
           time_t end)
{
	assert(paste);
	assert(title);
	assert(author);
	assert(language);
	assert(code);

	if (id)
		paste->id = estrdup(id);
	else
		paste->id = tmpupd_newid();

	paste->title = estrdup(title);
	paste->author = estrdup(author);
	paste->language = estrdup(language);
	paste->code = estrdup(code);
	paste->start = start;
	paste->end = end;
}

int
paste_save(struct paste *paste, struct db *db)
{
	assert(paste);
	assert(db);

	intmax_t ret;

	ret = db_insert(db, (const char *)sql_paste_save, "ssssstt",
		paste->id,
		paste->title,
		paste->author,
		paste->language,
		paste->code,
		paste->start,
		paste->end
	);

	return ret;
}

int
paste_delete(struct paste *paste, struct db *db)
{
	assert(paste);
	assert(db);

	return 0;
}

void
paste_finish(struct paste *paste)
{
	assert(paste);

	free(paste->id);
	free(paste->title);
	free(paste->author);
	free(paste->language);
	free(paste->code);
	memset(paste, 0, sizeof (*paste));
}
