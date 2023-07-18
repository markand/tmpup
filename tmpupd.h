#ifndef TMPUPD_H
#define TMPUPD_H

#include <time.h>

struct db;
enum db_mode;

int
tmpupd_open(struct db *, enum db_mode);

/**
 * Create a somewhat unique identifier as much as we can.
 *
 * \return a newly allocated identifier (to be free'd)
 */
char *
tmpupd_newid(void);

const char *
tmpupd_expiresin(time_t start, time_t end);

#endif /* !TMPUPD_H */
