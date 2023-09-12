/*
 * db.c -- SQLite wrapper
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
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "db.h"

#define FLAGS(mode)                                             \
        mode == DB_RDONLY                                       \
                ? SQLITE_OPEN_READONLY                          \
                : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE

#define PREPARE(db, sql, fmt, ap, stmt)                                         \
do {                                                                            \
        int err;                                                                \
                                                                                \
        if (sqlite3_prepare((db)->handle, sql, -1, stmt, NULL) != SQLITE_OK)    \
                return db_set_error(db);                                        \
                                                                                \
        va_start(ap, fmt);                                                      \
        err = vbind(*stmt, fmt, ap);                                            \
        va_end(ap);                                                             \
                                                                                \
        if (err < 0) {                                                          \
                sqlite3_finalize(*stmt);                                        \
                return db_set_error(db);                                        \
        }                                                                       \
} while (0)

static int
vbind(sqlite3_stmt *stmt, const char *fmt, va_list ap)
{
	int index = 1, status;
	const void *blob;
	size_t blobsz;

	for (; *fmt; fmt++) {
		status = 0;

		switch (*fmt) {
		case 'd':
			sqlite3_bind_int(stmt, index++, va_arg(ap, int));
			break;
		case 'b':
			blob = va_arg(ap, const void *);
			blobsz = va_arg(ap, size_t);
			sqlite3_bind_blob(stmt, index++, blob, blobsz, NULL);
			break;
		case 'f':
			sqlite3_bind_double(stmt, index++, va_arg(ap, double));
			break;
		case 'j':
			sqlite3_bind_int64(stmt, index++, va_arg(ap, intmax_t));
			break;
		case 'I':
			sqlite3_bind_int64(stmt, index++, va_arg(ap, sqlite3_int64));
			break;
		case 's':
			sqlite3_bind_text(stmt, index++, va_arg(ap, const char *), -1, SQLITE_STATIC);
			break;
		case 't':
			sqlite3_bind_int64(stmt, index++, va_arg(ap, time_t));
			break;
		case 'u':
			sqlite3_bind_int64(stmt, index++, va_arg(ap, unsigned int));
			break;
		case 'z':
			sqlite3_bind_int64(stmt, index++, va_arg(ap, size_t));
			break;
		default:
			assert(isspace((unsigned char)*fmt));
			break;
		}

		if (status != SQLITE_OK)
			return -1;
	}

	return 0;
}

int
db_open(struct db *db, const char *path, enum db_mode mode)
{
	assert(db);
	assert(path);

	memset(db, 0, sizeof (*db));

	/*
	 * SQLite opens an existing database as readonly if the process does
	 * not have appropriate write permissions but will correctly fail when
	 * attempting to create a database without permissions. In that case it
	 * will throw "SQL logic error" only when attempting to commit
	 * statements which isn't meaningful. As such, if the file already
	 * exists we check that we have write permissions to it.
	 */
	if (mode == DB_RDWR && access(path, F_OK) == 0 && access(path, W_OK) < 0) {
		snprintf(db->error, sizeof (db->error), "%s", strerror(errno));
		return -1;
	}

	if (sqlite3_open_v2(path, &db->handle, FLAGS(mode), NULL) != SQLITE_OK)
		return db_set_error(db);

	/* SQLite's own busy logic required for file locking. */
	sqlite3_busy_timeout(db->handle, 30000);

	return 0;
}

int
db_set_error(struct db *db)
{
	snprintf(db->error, sizeof (db->error), "%s", sqlite3_errmsg(db->handle));

	return -1;
}

ssize_t
db_select(struct db *db, struct db_select *sel, const char *sql, const char *fmt, ...)
{
	assert(db);
	assert(sel);

	sqlite3_stmt *stmt;
	va_list ap;
	int err;
	ssize_t ret = 0;

	PREPARE(db, sql, fmt, ap, &stmt);

	while ((err = sqlite3_step(stmt)) == SQLITE_ROW && sel->datasz--)
		sel->get(stmt, (unsigned char *)sel->data + (sel->elemsz * ret++));

	if (err != SQLITE_DONE) {
		db_set_error(db);
		ret = -1;
	}

	sqlite3_finalize(stmt);

	return ret;
}

int
db_iterate(struct db *db, db_iterate_fn iter, void *data, const char *sql, const char *fmt, ...)
{
	assert(db);
	assert(iter);
	assert(sql);

	sqlite3_stmt *stmt;
	va_list ap;
	int err, ret = 0;

	PREPARE(db, sql, fmt, ap, &stmt);

	for (size_t row = 0; (err = sqlite3_step(stmt)) == SQLITE_ROW; ++row) {
		if (iter(stmt, row, data) < 0) {
			strcpy(db->error, "user function abort");

			/* Don't put error. */
			ret = -1;
			err = SQLITE_DONE;
			break;
		}
	}

	if (err != SQLITE_DONE) {
		db_set_error(db);
		ret = -1;
	}

	sqlite3_finalize(stmt);

	return ret;
}

intmax_t
db_insert(struct db *db, const char *sql, const char *fmt, ...)
{
	assert(db);
	assert(sql);
	assert(fmt);

	sqlite3_stmt *stmt;
	va_list ap;
	int ret = 0;

	PREPARE(db, sql, fmt, ap, &stmt);

	if (sqlite3_step(stmt) != SQLITE_DONE)
		ret = db_set_error(db);

	sqlite3_finalize(stmt);

	return ret < 0 ? -1 : sqlite3_last_insert_rowid(db->handle);
}

int
db_execf(struct db *db, const char *sql, const char *fmt, ...)
{
	assert(db);
	assert(sql);
	assert(fmt);

	sqlite3_stmt *stmt;
	va_list ap;
	int ret = 0;

	PREPARE(db, sql, fmt, ap, &stmt);

	if (sqlite3_step(stmt) != SQLITE_DONE)
		ret = db_set_error(db);

	sqlite3_finalize(stmt);

	return ret;
}

int
db_exec(struct db *db, const char *sql)
{
	assert(db);
	assert(sql);

	if (sqlite3_exec(db->handle, sql, NULL, NULL, NULL) != SQLITE_OK)
		return db_set_error(db);

	return 0;
}

void
db_finish(struct db *db)
{
	assert(db);

	if (db->handle) {
		sqlite3_close(db->handle);
		db->handle = NULL;
	}
}
