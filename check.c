#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "paste.h"
#include "tmp.h"
#include "util.h"

static int
cmp_str(const void *key, const void *val)
{
	return strcmp(key, *(const char **)val);
}

int
check_language(const char *lang, char *error, size_t errorsz)
{
	assert(lang);
	assert(error);

	const char *iter;

	if (!lang)
		return -1;
	if (!(iter = bsearch(lang, paste_langs, paste_langsz, sizeof (char *), cmp_str))) {
		snprintf(error, errorsz, "invalid language '%s'", lang);
		return -1;
	}

	return 0;
}

int
check_duration(time_t start, time_t end, char *error, size_t errorsz)
{
	assert(error);

	/* Really? */
	if (end <= start) {
		snprintf(error, errorsz, "end date is prior start");
		return -1;
	}

	/* Don't think your upload is that notorious. */
	if (end - start >= TMP_DURATION_MONTH) {
		snprintf(error, errorsz, "too long duration");
		return -1;
	}

	return 0;
}
