# Copyright © 2012, 2013  Mattias Andrée (maandree@member.fsf.org)
# 
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
# 
# [GNU All Permissive License]


PREFIX=/usr
BIN=/bin
DATA=/share
LICENSES=$(DATA)/licenses
PKGNAME=adjbacklight
COMMAND=adjbacklight
BINCLASS=$(DATA)/misc

BOOK=adjbacklight
BOOKDIR=info/


# compile the package
.PHONY: all
all: code info

code: bash java
bash: adjbacklight.install
java: Adjbacklight.class

%.class: %.java
	javac -cp . "$<"

adjbacklight.install: adjbacklight
	cp "$<" "$@"
	sed -i 's:\$${BASH_SOURCE%/\*}:$(PREFIX)$(BINCLASS):g' "adjbacklight.install"

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
install: install-cmd install-license install-info

install-cmd: adjbacklight.install Adjbacklight.class
	mkdir -p "$(DESTDIR)$(PREFIX)$(BIN)"
	mkdir -p "$(DESTDIR)$(PREFIX)$(BINCLASS)"
	install -m 755 "adjbacklight.install" "$(DESTDIR)$(PREFIX)$(BIN)/$(COMMAND)"
	install -m 644 "Adjbacklight.class" "$(DESTDIR)$(PREFIX)$(BINCLASS)/Adjbacklight.class"

install-license:
	mkdir -p "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"
	install -m 644 COPYING LICENSE "$(DESTDIR)$(PREFIX)$(DATA)$(LICENSES)/$(PKGNAME)"

install-info: $(BOOK).info.gz
	mkdir -p "$(DESTDIR)$(PREFIX)$(DATA)/info"
	install -m 644 "$(BOOK).info.gz" "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info.gz"


# remove files created by `install`
uninstall:
	-rm "$(DESTDIR)$(PREFIX)$(BIN)/$(COMMAND)"
	-rm "$(DESTDIR)$(PREFIX)$(BINCLASS)/Adjbacklight.class"
	-rm "$(DESTDIR)$(PREFIX)$(DATA)$(LICENSES)/$(PKGNAME)/COPYING"
	-rm "$(DESTDIR)$(PREFIX)$(DATA)$(LICENSES)/$(PKGNAME)/LICENSE"
	-rmdir "$(DESTDIR)$(PREFIX)$(DATA)$(LICENSES)/$(PKGNAME)"
	-rm "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info.gz"


# remove files created by `all`
.PHONY: clean
clean:
	-rm -r *.{class,t2d,aux,cp,cps,fn,ky,log,pg,pgs,toc,tp,vr,vrs,op,ops,bak,info,pdf,ps,dvi,gz,install} 2>/dev/null

