.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

all: adjbacklight

adjbacklight.o: adjbacklight.c arg.h
	$(CC) -c -o adjbacklight.o adjbacklight.c $(CCFLAGS) $(CPPFLAGS)

adjbacklight: adjbacklight.o
	$(CC) -o adjbacklight adjbacklight.o $(LDFLAGS)

install: adjbacklight
	mkdir -p -- "$(DESTDIR)$(PREFIX)/bin"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man1"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/share/licenses/adjbacklight"
	cp -- adjbacklight "$(DESTDIR)$(PREFIX)/bin"
	cp -- adjbacklight.1 "$(DESTDIR)$(MANPREFIX)/man1"
	cp -- LICENSE "$(DESTDIR)$(PREFIX)/share/licenses/adjbacklight"
	chmod -- 4755 "$(DESTDIR)$(PREFIX)/bin/adjbacklight"

uninstall:
	-rm -- "$(DESTDIR)$(PREFIX)/bin/adjbacklight"
	-rm -- "$(DESTDIR)$(MANPREFIX)/man1/adjbacklight.1"
	-rm -- "$(DESTDIR)$(PREFIX)/share/licenses/adjbacklight/LICENSE"
	-rmdir -- "$(DESTDIR)$(PREFIX)/share/licenses/adjbacklight"

clean:
	-rm -- adjbacklight *.o

.PHONY: all install uninstall clean
