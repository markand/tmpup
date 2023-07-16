#ifndef TMP_CHECK_H
#define TMP_CHECK_H

#include <stddef.h>
#include <time.h>

int
check_language(const char *lang, char *error, size_t errorsz);

int
check_duration(time_t start, time_t end, char *error, size_t errorsz);

#endif /* !TMP_CHECK_H */
