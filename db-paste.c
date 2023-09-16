/*
 * db-paste.c -- storage helpers for a paste
 *
 * Copyright (c) 2023 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <string.h>
#include <time.h>

#include "db-paste.h"
#include "db.h"
#include "paste.h"

#include "sql/paste-delete.h"
#include "sql/paste-get.h"
#include "sql/paste-prune.h"
#include "sql/paste-recents.h"
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
		(time_t)sqlite3_column_int64(stmt, 7)
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

	struct db_select select = {
		.data = paste,
		.datasz = 1,
		.elemsz = sizeof (*paste),
		.get = get
	};

	return db_select(db, &select, (const char *)sql_paste_get, "s", id);
}

ssize_t
db_paste_recents(struct paste *pastes, size_t pastesz, struct db *db)
{
	assert(pastes);
	assert(db);

	struct db_select select = {
		.data = pastes,
		.datasz = pastesz,
		.elemsz = sizeof (*pastes),
		.get = get
	};

	return db_select(db, &select, (const char *)sql_paste_recents, "z", pastesz);
}

int
db_paste_delete(struct paste *paste, struct db *db)
{
	assert(paste);
	assert(db);

	return db_execf(db, (const char *)sql_paste_delete, "s", paste->id);
}

int
db_paste_prune(struct db *db)
{
	assert(db);

	return db_execf(db, (const char *)sql_paste_prune, "t", time(NULL));
}
