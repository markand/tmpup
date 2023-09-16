#
# Makefile -- Makefile for tmpup
#
# Copyright (c) 2023 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

PREFIX :=       /usr/local
VARDIR :=       $(PREFIX)/var

CC :=           clang
CFLAGS :=       -g -O0 -Wall -Wextra

SQL_SRCS :=     sql/image-delete.sql
SQL_SRCS +=     sql/image-get.sql
SQL_SRCS +=     sql/image-prune.sql
SQL_SRCS +=     sql/image-recents.sql
SQL_SRCS +=     sql/image-save.sql
SQL_SRCS +=     sql/init.sql
SQL_SRCS +=     sql/paste-delete.sql
SQL_SRCS +=     sql/paste-get.sql
SQL_SRCS +=     sql/paste-prune.sql
SQL_SRCS +=     sql/paste-recents.sql
SQL_SRCS +=     sql/paste-save.sql
SQL_OBJS :=     $(SQL_SRCS:.sql=.h)

HTML_SRCS :=    html/footer.html
HTML_SRCS +=    html/header.html
HTML_SRCS +=    html/image.html
HTML_SRCS +=    html/index.html
HTML_SRCS +=    html/paste.html
HTML_OBJS :=    $(HTML_SRCS:.html=.h)

TMPUPD_SRCS :=  extern/libsqlite/sqlite3.c
TMPUPD_SRCS +=  base64.c
TMPUPD_SRCS +=  check.c
TMPUPD_SRCS +=  db-image.c
TMPUPD_SRCS +=  db-paste.c
TMPUPD_SRCS +=  db.c
TMPUPD_SRCS +=  http.c
TMPUPD_SRCS +=  image.c
TMPUPD_SRCS +=  log.c
TMPUPD_SRCS +=  page-api-v0-image.c
TMPUPD_SRCS +=  page-api-v0-paste.c
TMPUPD_SRCS +=  page-image.c
TMPUPD_SRCS +=  page-index.c
TMPUPD_SRCS +=  page-paste.c
TMPUPD_SRCS +=  page.c
TMPUPD_SRCS +=  paste.c
TMPUPD_SRCS +=  tmp.c
TMPUPD_SRCS +=  tmpupd.c
TMPUPD_SRCS +=  util.c
TMPUPD_OBJS :=  $(TMPUPD_SRCS:.c=.o)
TMPUPD_DEPS :=  $(TMPUPD_SRCS:.c=.d)

TMPUP_SRCS :=   base64.c check.c image.c paste.c tmp.c tmpup.c util.c
TMPUP_OBJS :=   $(TMPUP_SRCS:.c=.o)
TMPUP_DEPS :=   $(TMPUP_SRCS:.c=.d)

CURL_INCS :=    $(shell pkg-config --cflags libcurl)
CURL_LIBS :=    $(shell pkg-config --libs libcurl)

JANSSON_INCS := $(shell pkg-config --cflags jansson)
JANSSON_LIBS := $(shell pkg-config --libs jansson)

KCGI_INCS :=    $(shell pkg-config --cflags kcgi kcgi-html)
KCGI_LIBS :=    $(shell pkg-config --libs kcgi kcgi-html)

MAGIC_INCS :=   $(shell pkg-config --cflags libmagic)
MAGIC_LIBS :=   $(shell pkg-config --libs libmagic)

override CPPFLAGS += -DVARDIR=\"$(VARDIR)\"
override CPPFLAGS += -DSQLITE_DEFAULT_FOREIGN_KEYS=1
override CPPFLAGS += -DSQLITE_DEFAULT_MEMSTATUS=0
override CPPFLAGS += -DSQLITE_OMIT_DECLTYPE
override CPPFLAGS += -DSQLITE_OMIT_DEPRECATED
override CPPFLAGS += -DSQLITE_OMIT_LOAD_EXTENSION
override CPPFLAGS += -DSQLITE_THREADSAFE=0
override CPPFLAGS += -MMD
override CFLAGS   += -Iextern/libsqlite

%.h: %.sql
	extern/bcc/bcc -0cs $< $< > $@

%.h: %.html
	extern/bcc/bcc -cs $< $< > $@

all: tmpupd tmpup

$(SQL_OBJS): extern/bcc/bcc
$(HTML_OBJS): extern/bcc/bcc

# tmpupd

-include $(TMPUPD_DEPS)

# disable warnings on SQLite...
extern/libsqlite/sqlite3.o: private CPPFLAGS += -Wno-unused-parameter

$(TMPUPD_SRCS): $(HTML_OBJS) $(SQL_OBJS)
$(TMPUPD_OBJS): private CFLAGS += $(JANSSON_INCS) $(KCGI_INCS) $(MAGIC_INCS)

tmpupd: private LDLIBS += $(JANSSON_LIBS) $(KCGI_LIBS) $(MAGIC_LIBS) -lpthread
tmpupd: $(TMPUPD_OBJS)

# convenient spawner
tmpupd-run: tmpupd
	sh tmpupd-run

# tmpup

-include $(TMPUP_DEPS)

$(TMPUP_OBJS): private CFLAGS += $(CURL_INCS) $(JANSSON_INCS) $(MAGIC_INCS)

tmpup: private LDLIBS += $(CURL_LIBS) $(JANSSON_LIBS) $(MAGIC_LIBS)
tmpup: $(TMPUP_OBJS)

clean:
	rm -f extern/bcc/bcc
	rm -f $(HTML_OBJS) $(SQL_OBJS)
	rm -f tmpupd $(TMPUPD_OBJS) $(TMPUPD_DEPS)
	rm -f tmpup $(TMPUP_OBJS) $(TMPUP_DEPS)

.PHONY: all clean tmpupd-run
