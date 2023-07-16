#ifndef TMPUPD_H
#define TMPUPD_H

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

#endif /* !TMPUPD_H */
