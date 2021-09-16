.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

BIN = adjbacklight test
OBJ = $(BIN:=.o)


all: adjbacklight test
$(OBJ): arg.h

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.o:
	$(CC) -o $@ $< $(LDFLAGS)

check: test
	./test.sh

install: adjbacklight
	mkdir -p -- "$(DESTDIR)$(PREFIX)/bin"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man1"
	cp -- adjbacklight "$(DESTDIR)$(PREFIX)/bin"
	cp -- adjbacklight.1 "$(DESTDIR)$(MANPREFIX)/man1"

post-install:
	chown -- '0:$(VIDEO_GROUP)' "$(DESTDIR)$(PREFIX)/bin/adjbacklight"
	chmod -- 4754 "$(DESTDIR)$(PREFIX)/bin/adjbacklight"

uninstall:
	-rm -- "$(DESTDIR)$(PREFIX)/bin/adjbacklight"
	-rm -- "$(DESTDIR)$(MANPREFIX)/man1/adjbacklight.1"

clean:
	-rm -rf -- adjbacklight test .testdir *.o *.su

.PHONY: all check install post-install uninstall clean
