#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "db.h"
#include "http.h"
#include "log.h"
#include "maint.h"
#include "tmp.h"
#include "tmpupd.h"
#include "util.h"

#include "sql/init.h"

static const char *dbpath = "test.db";

static void
stop(int n)
{
	(void)n;

	/*
	 * To terminate kcgi loop we have to kill ourselves using SIGTERM. It
	 * will automatically triggers an error of EINTR at the moment but do
	 * it in the case this is fixed at some point.
	 */
	kill(getpid(), SIGTERM);
}

static inline void
init_signals(void)
{
	struct sigaction sa = {0};

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = stop;

	if (sigaction(SIGINT, &sa, NULL) < 0)
		die("abort: %s\n", strerror(errno));
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
}

static inline void
init_tmpupd(void)
{
	http_init();
	maint_init();
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
	struct kfcgi *fcgi;
	struct kreq req;

	if (khttp_fcgi_init(&fcgi, NULL, 0, NULL, 0, 0) != KCGI_OK)
		die("abort: could not allocate FastCGI");

	for (;;) {
		if (khttp_fcgi_parse(fcgi, &req) != KCGI_OK)
			break;

		http_process(&req);
		khttp_free(&req);
	}

	khttp_fcgi_free(fcgi);
}

static void
finish(void)
{
	log_info("tmpupd: exiting...");
	http_finish();
	maint_finish();
	log_finish();
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
tmpupd_expiresin(time_t start, time_t end)
{
	static _Thread_local char ret[64];
	unsigned long long gap = end - start;

	if (gap < TMP_DURATION_HOUR)
		sprintf(ret, "%llu minutes", gap / 60);
	else if (gap < TMP_DURATION_DAY)
		sprintf(ret, "%llu hours", gap / TMP_DURATION_HOUR);
	else
		sprintf(ret, "%llu days", gap / TMP_DURATION_DAY);

	return ret;
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
