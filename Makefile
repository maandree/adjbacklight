# Copyright © 2012, 2013  Mattias Andrée (maandree@member.fsf.org)
# 
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
# 
# [GNU All Permissive License]


PREFIX=/usr


# compile the package
all:
	javac -cp . Adjbacklight.java

# install to system
install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/licenses
	install -m 755 adjbacklight $(DESTDIR)$(PREFIX)/bin/
	install -m 644 Adjbacklight.class $(DESTDIR)$(PREFIX)/bin/
	install -m 644 COPYING $(DESTDIR)$(PREFIX)/share/licenses/adjbacklight
	install -m 644 LICENSE $(DESTDIR)$(PREFIX)/share/licenses/adjbacklight

# remove files created by `install`
uninstall:
	unlink $(DESTDIR)$(PREFIX)/bin/Adjbacklight.class
	unlink $(DESTDIR)$(PREFIX)/bin/adjbacklight
	rm -r $(DESTDIR)$(PREFIX)/share/licenses/adjbacklight

# remove files created by `all`
clean:
	rm Adjbacklight.class

