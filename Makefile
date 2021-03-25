.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

all: adjbacklight test

adjbacklight.o: adjbacklight.c arg.h
	$(CC) -c -o adjbacklight.o adjbacklight.c $(CCFLAGS) $(CPPFLAGS)

adjbacklight: adjbacklight.o
	$(CC) -o adjbacklight adjbacklight.o $(LDFLAGS)

test.o: test.c adjbacklight.c arg.h
	$(CC) -c -o test.o test.c $(CCFLAGS) $(CPPFLAGS)

test: test.o
	$(CC) -o test test.o $(LDFLAGS)

check: test
	./test.sh

install: adjbacklight
	mkdir -p -- "$(DESTDIR)$(PREFIX)/bin"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man1"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/share/licenses/adjbacklight"
	cp -- adjbacklight "$(DESTDIR)$(PREFIX)/bin"
	cp -- adjbacklight.1 "$(DESTDIR)$(MANPREFIX)/man1"
	cp -- LICENSE "$(DESTDIR)$(PREFIX)/share/licenses/adjbacklight"

post-install:
	chown -- '0:$(VIDEO_GROUP)' "$(DESTDIR)$(PREFIX)/bin/adjbacklight"
	chmod -- 4754 "$(DESTDIR)$(PREFIX)/bin/adjbacklight"

uninstall:
	-rm -- "$(DESTDIR)$(PREFIX)/bin/adjbacklight"
	-rm -- "$(DESTDIR)$(MANPREFIX)/man1/adjbacklight.1"
	-rm -- "$(DESTDIR)$(PREFIX)/share/licenses/adjbacklight/LICENSE"
	-rmdir -- "$(DESTDIR)$(PREFIX)/share/licenses/adjbacklight"

clean:
	-rm -rf -- adjbacklight test *.o .testdir

.PHONY: all check install post-install uninstall clean
