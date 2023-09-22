/*
 * db-paste.h -- storage helpers for a paste
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

#ifndef TMPUPD_DB_PASTE_H
#define TMPUPD_DB_PASTE_H

/**
 * \file db-paste.h
 * \brief Storage helpers for a paste.
 */

#include <sys/types.h>

struct db;
struct paste;

/**
 * Save a paste into the database.
 *
 * Field id must be set prior to insertion.
 *
 * \pre paste != NULL
 * \pre db != NULL
 * \param paste the image
 * \param db the database
 * \return 0 on success or -1 on error
 */
int
db_paste_save(struct paste *paste, struct db *db);

/**
 * Get a unique paste from database.
 *
 * \pre paste != NULL
 * \pre id != NULL
 * \pre db != NULL
 * \param paste the image
 * \param id the image identifier
 * \param db the database
 * \return 1 if found, 0 if not found or -1 on error
 */
int
db_paste_get(struct paste *paste, const char *id, struct db *db);

/**
 * Get a list of most recent pastes.
 *
 * \pre pastes != NULL
 * \pre db != NULL
 * \param pastes array of pastes to fill
 * \param pastesz number of pastes to load at most
 * \param db the database
 * \return the number of pastes loaded or -1 on error
 */
ssize_t
db_paste_recents(struct paste *pastes, size_t pastesz, struct db *db);

/**
 * Delete the specified paste from database.
 *
 * The paste isn't cleaned up, ::paste_finish must still be called.
 *
 * \pre img != NULL
 * \pre db != NULL
 * \param paste the paste
 * \param db the database
 * \return 0 on success or -1 on error
 */
int
db_paste_delete(struct paste *paste, struct db *db);

/**
 * Delete outdated pastes from database.
 *
 * \pre db != NULL
 * \param db the database
 * \return 0 on success or -1 on error
 */
int
db_paste_prune(struct db *db);

#endif /* TMPUPD_DB_PASTE_H */
