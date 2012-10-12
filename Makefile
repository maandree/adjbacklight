all:
	javac -cp . Adjbacklight.java

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 775 adjbacklight $(DESTDIR)$(PREFIX)/bin/
	install -m 664 Adjbacklight.class $(DESTDIR)$(PREFIX)/bin/

uninstall:
	unlink $(DESTDIR)$(PREFIX)/bin/Adjbacklight.class
	unlink $(DESTDIR)$(PREFIX)/bin/adjbacklight

clean:
	rm Adjbacklight.class

