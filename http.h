#ifndef TMPUPD_HTTP_H
#define TMPUPD_HTTP_H

#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>

#include <kcgi.h>

void
http_init(void);

void
http_process(struct kreq *r);

void
http_finish(void);

#endif /* !TMPUPD_HTTP_H */
