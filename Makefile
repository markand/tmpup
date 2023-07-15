CC :=           clang
CFLAGS :=       -g -O0 -Wall -Wextra

SQL_SRCS :=     sql/init.sql \
                sql/image-delete.sql \
                sql/image-save.sql \
                sql/paste-delete.sql \
                sql/paste-save.sql
SQL_OBJS :=     $(SQL_SRCS:.sql=.h)

TMPUPD_SRCS :=  extern/libsqlite/sqlite3.c \
                db.c \
                http.c \
                image.c \
                log.c \
                maint.c \
                page-index.c \
                page-paste.c \
                page-status.c \
                paste.c \
                tmpupd.c \
                util.c
TMPUPD_OBJS :=  $(TMPUPD_SRCS:.c=.o)
TMPUPD_DEPS :=  $(TMPUPD_SRCS:.c=.d)

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
                     -DSQLITE_THREADSAFE=0
override CFLAGS +=   -Iextern/libsqlite

%.h: %.sql
	extern/bcc/bcc -0cs $< $< > $@

all: tmpupd

# disable warnings on SQLite...
extern/libsqlite/sqlite3.o: private CPPFLAGS += -Wno-unused-parameter

$(SQL_OBJS): extern/bcc/bcc

# tmpupd

-include $(TMPUPD_DEPS)

$(TMPUPD_SRCS): $(SQL_OBJS)
$(TMPUPD_OBJS): private CFLAGS += $(JANSSON_INCS) $(KCGI_INCS)

tmpupd: private LDLIBS += $(JANSSON_LIBS) $(KCGI_LIBS)
tmpupd: $(TMPUPD_OBJS)

clean:
	rm -f extern/bcc/bcc
	rm -f $(SQL_OBJS)
	rm -f $(TMPUPD_OBJS) $(TMPUPD_DEPS)

.PHONY: all clean
