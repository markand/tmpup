#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <stdatomic.h>
#include <string.h>
#include <time.h>

#include "maint.h"
#include "util.h"

static timer_t timer;
static atomic_int run;

static void
task(union sigval v)
{
	(void)v;

	/* Do nothing if the task is spawned but we are quitting. */
	if (!run)
		return;
}

void
maint_init(void)
{
	struct sigevent sev = {0};
	struct itimerspec its = {0};

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = task;

	its.it_value.tv_sec = 1;
	its.it_interval.tv_sec = 1;

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
