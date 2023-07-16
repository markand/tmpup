CC :=           clang
CFLAGS :=       -g -O0 -Wall -Wextra

SQL_SRCS :=     sql/init.sql \
                sql/image-delete.sql \
                sql/image-save.sql \
                sql/paste-delete.sql \
                sql/paste-get.sql \
                sql/paste-recents.sql \
                sql/paste-save.sql
SQL_OBJS :=     $(SQL_SRCS:.sql=.h)

HTML_SRCS :=    html/footer.html \
                html/header.html \
                html/index.html \
                html/paste.html
HTML_OBJS :=    $(HTML_SRCS:.html=.h)

TMPUPD_SRCS :=  extern/libsqlite/sqlite3.c \
                check.c \
                db-image.c \
                db-paste.c \
                db.c \
                http.c \
                image.c \
                log.c \
                maint.c \
                page-api-v0-paste.c \
                page-index.c \
                page-paste.c \
                page.c \
                paste.c \
                tmp.c \
                tmpupd.c \
                util.c
TMPUPD_OBJS :=  $(TMPUPD_SRCS:.c=.o)
TMPUPD_DEPS :=  $(TMPUPD_SRCS:.c=.d)

TMPUP_SRCS :=   check.c image.c paste.c tmp.c tmpup.c util.c
TMPUP_OBJS :=   $(TMPUP_SRCS:.c=.o)
TMPUP_DEPS :=   $(TMPUP_SRCS:.c=.d)

CURL_INCS :=    $(shell pkg-config --cflags libcurl)
CURL_LIBS :=    $(shell pkg-config --libs libcurl)

JANSSON_INCS := $(shell pkg-config --cflags jansson)
JANSSON_LIBS := $(shell pkg-config --libs jansson)

KCGI_INCS :=    $(shell pkg-config --cflags kcgi kcgi-html)
KCGI_LIBS :=    $(shell pkg-config --libs kcgi kcgi-html)

override CPPFLAGS += -DSQLITE_DEFAULT_FOREIGN_KEYS=1 \
                     -DSQLITE_DEFAULT_MEMSTATUS=0 \
                     -DSQLITE_OMIT_DECLTYPE \
                     -DSQLITE_OMIT_DEPRECATED \
                     -DSQLITE_OMIT_LOAD_EXTENSION \
                     -DSQLITE_THREADSAFE=0 \
                     -MMD
override CFLAGS +=   -Iextern/libsqlite

%.h: %.sql
	extern/bcc/bcc -0cs $< $< > $@

%.h: %.html
	extern/bcc/bcc -cs $< $< > $@

all: tmpupd tmpup

# disable warnings on SQLite...
extern/libsqlite/sqlite3.o: private CPPFLAGS += -Wno-unused-parameter

$(SQL_OBJS): extern/bcc/bcc
$(HTML_OBJS): extern/bcc/bcc

# tmpupd

-include $(TMPUPD_DEPS)

$(TMPUPD_SRCS): $(HTML_OBJS) $(SQL_OBJS)
$(TMPUPD_OBJS): private CFLAGS += $(JANSSON_INCS) $(KCGI_INCS)

tmpupd: private LDLIBS += $(JANSSON_LIBS) $(KCGI_LIBS)
tmpupd: $(TMPUPD_OBJS)

# tmpup

-include $(TMPUP_DEPS)

$(TMPUP_OBJS): private CFLAGS += $(CURL_INCS) $(JANSSON_INCS)

tmpup: private LDLIBS += $(CURL_LIBS) $(JANSSON_LIBS)
tmpup: $(TMPUP_OBJS)

clean:
	rm -f extern/bcc/bcc extern/libsqlite/sqlite3.o
	rm -f $(HTML_OBJS) $(SQL_OBJS)
	rm -f tmpupd $(TMPUPD_OBJS) $(TMPUPD_DEPS)
	rm -f tmpup $(TMPUP_OBJS) $(TMPUP_DEPS)

.PHONY: all clean
