/*
 * db.h -- SQLite wrapper
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

#ifndef DB_H
#define DB_H

/**
 * \file db.h
 * \brief SQLite database interface.
 *
 * This set of functions allows you to bind queries using a format string for
 * convenience. The following characters denotes which types must be passed:
 *
 * - `I`: `sqlite3_int64`
 * - `d`: `int`
 * - `f`: `double`
 * - `j`: `intmax_t`
 * - `s`: `const char *`
 * - `t`: `time_t`
 * - `u`: `unsigned int`
 * - `z`: `size_t`
 *
 * Example using db_exec() function:
 *
 * \code
 * const char *sql = "
 *   DELETE
 *     FROM boobs
 *    WHERE id = ?
 *      AND size ?
 * ";
 *
 * db_execf(db, sql, "dd", 1, 1000);
 * \endcode
 */

#include <sys/types.h>
#include <stdint.h>

#include <sqlite3.h>

/**
 * \def DB_ERR_MAX
 * Maximum error string size.
 */
#define DB_ERR_MAX 128

/**
 * \enum db_mode
 * \brief Database opening mode.
 */
enum db_mode {
	DB_RDONLY,      /*!< Open read-only. */
	DB_RDWR         /*!< Open read-write. */
};

/**
 * \struct db
 * \brief Database handle.
 */
struct db {
	sqlite3 *handle;        /*!< Native SQLite handle. */
	char error[DB_ERR_MAX]; /*!< Error string in last command. */
};

/**
 * Callback function for db_iterate().
 *
 * \param stmt the current statement
 * \param row the current row number
 * \param data optional user data
 * \return 0 on success or -1 to stop iteration
 */
typedef int (*db_iterate_fn)(sqlite3_stmt *stmt, size_t row, void *data);

/**
 * \struct db_select
 * \brief Convenient interface for SELECT-like queries.
 *
 * This structure is used to unpack a multiple row set.
 */
struct db_select {
	void *data;     /*!< Array of data to fetch. */
	size_t datasz;  /*!< Number of elements in the array. */
	size_t elemsz;  /*!< Size of individual elements. */

	/**
         * Use the sqlite3_column_* set of functions to retrieve columns and
         * store it into data. Top left column is 0.
         *
         * \param stmt the statement with current row
         * \param data the destination pointer
	 */
	void (*get)(sqlite3_stmt *stmt, void *data);
};

/**
 * Open the database at the specified path.
 *
 * \pre db != NULL
 * \pre path != NULL
 * \param db the instance object
 * \param path path to the file
 * \param mode opening mode
 * \return 0 on success or -1 on failure
 */
int
db_open(struct db *db, const char *path, enum db_mode mode);

/**
 * Set the error using the last SQLite error and return -1 for convenience.
 *
 * \pre db != NULL
 * \param db the database handle
 * \return -1
 */
int
db_set_error(struct db *db);

/**
 * Extract a multiple row query using a SELECT-like statement.
 *
 * Example of code that fetches data into a user defined `sometype` structure,
 * the table contains two columns of type int.
 *
 * \code
 * #define SOMETYPE_SELECT "SELECT * FROM sometype LIMIT ?"
 *
 * static void
 * sometype_get(sqlite3_stmt *stmt, void *data)
 * {
 *   struct sometype *tp = data;
 *
 *   tp->x = sqlite3_column_int(stmt, 0);
 *   tp->y = sqlite3_column_int(stmt, 1);
 * }
 *
 * static void
 * sometype_list(void)
 * {
 *   struct sometype data[128];
 *   ssize_t nr;
 *
 *   struct db_select sel = {
 *     .data = data,
 *     .datasz = LEN(data),
 *     .elemsz = sizeof (data[0]),
 *     .get = sometype_get
 *   };
 *
 *   nr = db_select(db, &sel, SOMETYPE_SELECT, "d", LEN(data));
 * }
 * \endcode
 *
 * \pre db != NULL
 * \pre sel != NULL
 * \pre sql != NULL
 * \param db the database handle
 * \param sel the selector object
 * \param sql the SQL query
 * \param fmt the format string for binding arguments to the query
 * \return -1 on failure or number of elements retrieved
 */
ssize_t
db_select(struct db *db, struct db_select *sel, const char *sql, const char *fmt, ...);

/**
 * Iterate over a indefinite number of rows through a SELECT-like statement.
 *
 * The function iter will be called for every row.
 *
 * \pre db != NULL
 * \pre iter != NULL
 * \pre sql != NULL
 * \param db the database handle
 * \param iter the iterator
 * \param data optional data to pass to iter function
 * \param sql the SQL query
 * \param fmt the format string for binding arguments to the query
 * \return 0 on success or -1 on failure
 */
int
db_iterate(struct db *db, db_iterate_fn iter, void *data, const char *sql, const char *fmt, ...);

/**
 * Insert a new row.
 *
 * \pre db != NULL
 * \pre sql != NULL
 * \param db the database handle
 * \param sql the SQL query
 * \param fmt the format string for binding arguments to the query
 * \return -1 on failure or the rowid generated if any
 */
intmax_t
db_insert(struct db *db, const char *sql, const char *fmt, ...);

/**
 * Exec a raw SQLite statement using optional bound parameters.
 *
 * This function can only execute one statement, use the ::db_exec normal
 * function if you want to execute multiple statements.
 *
 * \pre db != NULL
 * \pre sql != NULL
 * \param db the database handle
 * \param sql the SQL query
 * \param fmt the format string for binding arguments to the query
 * \return 0 on success or -1 on failure
 */
int
db_execf(struct db *db, const char *sql, const char *fmt, ...);

/**
 * Exec one or more statements as raw SQL.
 *
 * \pre db != NULL
 * \pre sql != NULL
 * \param db the database handle
 * \param sql the SQL query
 * \return 0 on success or -1 on failure
 */
int
db_exec(struct db *db, const char *sql);

/**
 * Close database.
 *
 * \pre db != NULL
 * \param db the database handle
 */
void
db_finish(struct db *db);

#endif /* DB_H */
