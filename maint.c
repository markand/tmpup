/*
 * maint.c -- maintenance system
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
#include <errno.h>
#include <signal.h>
#include <stdatomic.h>
#include <string.h>
#include <time.h>

#include "db-image.h"
#include "db-paste.h"
#include "db.h"
#include "log.h"
#include "maint.h"
#include "util.h"
#include "tmpupd.h"

#define TAG "maint: "

static timer_t timer;
static atomic_int run;

static void
task(union sigval v)
{
	(void)v;

	struct db db = {};

	/* Do nothing if the task is spawned but we are quitting. */
	if (!run)
		return;
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

void
maint_init(void)
{
	struct sigevent sev = {};
	struct itimerspec its = {};

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = task;

	its.it_value.tv_sec = 5;
	its.it_interval.tv_sec = 5;

	run = 1;

	if (timer_create(CLOCK_MONOTONIC, &sev, &timer) < 0)
		die("abort: timer_create: %s\n", strerror(errno));
	if (timer_settime(timer, 0, &its, NULL) < 0)
		die("abort: timer_settime: %s\n", strerror(errno));
}

void
maint_finish(void)
{
	run = 0;
	timer_delete(timer);
}
