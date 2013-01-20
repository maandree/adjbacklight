# Copyright © 2012, 2013  Mattias Andrée (maandree@member.fsf.org)
# 
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
# 
# [GNU All Permissive License]


PREFIX=/usr

PROGRAM=adjbacklight
BOOK=$(PROGRAM)
BOOKDIR=./


# compile the package
.PHONY: all info
all: java


java: Adjbacklight.class

%.class: %.java
	javac -cp . "$<"

info: $(BOOK).info.gz
%.info: $(BOOKDIR)%.texinfo
        $(MAKEINFO) "$<"
%.info.gz: %.info
        gzip -9c < "$<" > "$@"


pdf: $(BOOK).pdf
%.pdf: $(BOOKDIR)%.texinfo
        texi2pdf "$<"

pdf.gz: $(BOOK).pdf.gz
%.pdf.gz: %.pdf
        gzip -9c < "$<" > "$@"

pdf.xz: $(BOOK).pdf.xz
%.pdf.xz: %.pdf
        xz -e9 < "$<" > "$@"


dvi: $(BOOK).dvi
%.dvi: $(BOOKDIR)%.texinfo
        $(TEXI2DVI) "$<"

dvi.gz: $(BOOK).dvi.gz
%.dvi.gz: %.dvi
        gzip -9c < "$<" > "$@"

dvi.xz: $(BOOK).dvi.xz
%.dvi.xz: %.dvi
        xz -e9 < "$<" > "$@"



# install to system
.PHONY: install
install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/licenses
	install -m 755 adjbacklight $(DESTDIR)$(PREFIX)/bin/
	install -m 644 Adjbacklight.class $(DESTDIR)$(PREFIX)/bin/
	install -m 644 COPYING $(DESTDIR)$(PREFIX)/share/licenses/adjbacklight
	install -m 644 LICENSE $(DESTDIR)$(PREFIX)/share/licenses/adjbacklight

# remove files created by `install`
.PHONY: uninstall
uninstall:
	unlink $(DESTDIR)$(PREFIX)/bin/Adjbacklight.class
	unlink $(DESTDIR)$(PREFIX)/bin/adjbacklight
	rm -r $(DESTDIR)$(PREFIX)/share/licenses/adjbacklight

# remove files created by `all`
.PHONY: clean
clean:
	rm -r *.{class,t2d,aux,cp,cps,fn,ky,log,pg,pgs,toc,tp,vr,vrs,op,ops,bak,info,pdf,ps,dvi,gz} || exit 0

