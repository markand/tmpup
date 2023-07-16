#include <assert.h>
#include <string.h>

#include "db-paste.h"
#include "db.h"
#include "paste.h"

#include "sql/paste-delete.h"
#include "sql/paste-get.h"
#include "sql/paste-save.h"

static void
get(sqlite3_stmt *stmt, void *data)
{
	struct paste *paste = data;

	paste_init(paste,
		(const char *)sqlite3_column_text(stmt, 0),
		(const char *)sqlite3_column_text(stmt, 1),
		(const char *)sqlite3_column_text(stmt, 2),
		(const char *)sqlite3_column_text(stmt, 3),
		(const char *)sqlite3_column_text(stmt, 4),
		(const char *)sqlite3_column_text(stmt, 5),
		(time_t)sqlite3_column_int64(stmt, 6),
		(time_t)sqlite3_column_int64(stmt, 6)
	);
}

int
db_paste_save(struct paste *paste, struct db *db)
{
	assert(paste);
	assert(db);

	return db_insert(db, (const char *)sql_paste_save, "sssssstt",
		paste->id,
		paste->title,
		paste->author,
		paste->filename,
		paste->language,
		paste->code,
		paste->start,
		paste->end
	);
}

int
db_paste_get(struct paste *paste, const char *id, struct db *db)
{
	assert(paste);
	assert(id);
	assert(db);

	memset(paste, 0, sizeof (*paste));

	struct db_select select = {
		.data = paste,
		.datasz = 1,
		.elemsz = sizeof (*paste),
		.get = get
	};

	return db_select(db, &select, (const char *)sql_paste_get, "s", id);
}

int
db_paste_delete(struct paste *paste, struct db *db)
{
	assert(paste);
	assert(db);

	return 0;
}
