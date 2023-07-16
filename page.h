#ifndef TMPUPD_PAGE_STATUS
#define TMPUPD_PAGE_STATUS

struct kreq;
struct ktemplate;

enum khttp;
enum kmime;

void
page_template(struct kreq *r,
              enum khttp code,
              const struct ktemplate *kt,
              const unsigned char *html,
              size_t htmlsz);

void
page_status(struct kreq *r, enum khttp code, enum kmime mime);

void
page_json(struct kreq *r, enum khttp code, const char *fmt, ...);

#endif /* !TMPUPD_PAGE_STATUS */
