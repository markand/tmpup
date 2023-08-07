#ifndef TMP_CHECK_H
#define TMP_CHECK_H

#include <stddef.h>
#include <time.h>

void
check_init(void);

int
check_image(const void *data, size_t datasz);

int
check_language(const char *lang, char *error, size_t errorsz);

int
check_duration(time_t start, time_t end, char *error, size_t errorsz);

void
check_finish(void);

#endif /* !TMP_CHECK_H */
