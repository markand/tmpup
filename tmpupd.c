/*
 * tmpupd.c -- tmpupd main files
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

#include <sys/time.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "check.h"
#include "db-image.h"
#include "db-paste.h"
#include "db.h"
#include "http.h"
#include "log.h"
#include "route.h"
#include "tmp.h"
#include "tmpupd.h"
#include "util.h"

#include "sql/init.h"

#define TAG "tmpupd: "

static const char *dbpath = VARDIR "/db/tmpup/tmpup.db";
static sigset_t sigs;

static void
prune(void)
{
	struct db db;

	if (tmpupd_open(&db, DB_RDWR) < 0) {
		log_warn(TAG "skipping");
		return;
	}

	log_debug(TAG "pruning pastes...");

	if (db_paste_prune(&db) < 0)
		log_warn(TAG "unable to prune pastes: %s", db.error);

	log_debug(TAG "pruning images...");

	if (db_image_prune(&db) < 0)
		log_warn(TAG "unable to prune images: %s", db.error);

	db_finish(&db);
}

static inline void
init_signals(void)
{
	struct itimerval ts = {};

	/*
	 * Block the following signals:
	 *
	 * SIGINT: stop the application
	 * SIGALRM: periodic cleanup routine
	 */
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGINT);
	sigaddset(&sigs, SIGALRM);

	pthread_sigmask(SIG_BLOCK, &sigs, NULL);

	/*
	 * Setup the periodic timer for cleaning outdated elements in the
	 * database.
	 */
	ts.it_value.tv_sec = 5;
	ts.it_interval.tv_sec = 5;

	if (setitimer(ITIMER_REAL, &ts, NULL) < 0)
		die("abort: setitimer: %s\n", strerror(errno));
}

static inline void
init_db(void)
{
	struct db db;

	/*
	 * Initialize at least once to get table populated since some pages
	 * would just open for read-only access.
	 */
	if (db_open(&db, dbpath, DB_RDWR) < 0 ||
	    db_exec(&db, (const char *)sql_init) < 0)
		die("abort: %s: %s\n", dbpath, db.error);

	db_finish(&db);
}

static inline void
init_logs(enum log_level level)
{
	log_open(level);
}

static inline void
init_misc(void)
{
	srandom(time(NULL));

	if (check_init() < 0) {
		log_warn(TAG "libmagic initialization error: %s", strerror(errno));
		log_warn(TAG "image verification will be disabled");
	} else
		log_debug(TAG "image verification enabled");
}

static inline void
init_tmpupd(void)
{
	http_init();
}

static void
init(enum log_level level)
{
	init_signals();
	init_db();
	init_logs(level);
	init_misc();
	init_tmpupd();
}

static void
loop(void)
{
	int n, run = 1;

	while (run) {
		if (sigwait(&sigs, &n) < 0) {
			log_warn(TAG "sigwait: %s", strerror(errno));
			break;
		}

		switch (n) {
		case SIGINT:
			log_info(TAG "exiting on signal %d", n);
			run = 0;
			break;
		case SIGALRM:
			if (run)
				prune();
			break;
		default:
			break;
		}
	}
}

static void
finish(void)
{
	log_info("tmpupd: exiting...");
	http_finish();
	log_finish();
	check_finish();
}

int
tmpupd_open(struct db *db, enum db_mode mode)
{
	assert(db);

	if (db_open(db, dbpath, mode) < 0) {
		log_warn("tmpupd: %s: %s", dbpath, db->error);
		return -1;
	}

	return 0;
}

const char *
tmpupd_expiresin(time_t end)
{
	static _Thread_local char ret[64];
	time_t now;
	intmax_t diff;

	now = time(NULL);

	if (now >= end)
		sprintf(ret, "expired");
	else {
		diff = difftime(end, now);

		if (diff < TMP_DURATION_HOUR)
			sprintf(ret, "%jd minutes", diff / 60);
		else if (diff < TMP_DURATION_DAY)
			sprintf(ret, "%jd hours", diff / TMP_DURATION_HOUR);
		else
			sprintf(ret, "%jd days", diff / TMP_DURATION_DAY);
	}

	return ret;
}

const char *
tmpupd_visibility(int val)
{
	return val ? "visible" : "hidden";
}

void
tmpupd_condamn(time_t *start, time_t *end, const char *duration)
{
	assert(start);
	assert(end);
	assert(duration);

	*start = *end = time(NULL);

	if (strcmp(duration, "day") == 0)
		*end += TMP_DURATION_DAY;
	else if (strcmp(duration, "week") == 0)
		*end += TMP_DURATION_WEEK;
	else if (strcmp(duration, "month") == 0)
		*end += TMP_DURATION_MONTH;
	else
		*end += TMP_DURATION_HOUR;
}

int
tmpupd_isdef(const struct kpair *pair, const char *key)
{
	return strcmp(pair->key, key) == 0 && strlen(pair->val) > 0;
}

int
main(int argc, char **argv)
{
	enum log_level level = LOG_LEVEL_WARNING;
	int ch;

	opterr = 0;

	while ((ch = egetopt(argc, argv, "d:v")) != -1) {
		switch (ch) {
		case 'd':
			dbpath = optarg;
			break;
		case 'v':
			level++;
			break;
		default:
			break;
		}
	}

	init(level);
	loop();
	finish();
}
