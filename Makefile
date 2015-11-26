# Copyright © 2012, 2013, 2014  Mattias Andrée (maandree@member.fsf.org)
# 
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
# 
# [GNU All Permissive License]

OPTIMISATION = -Ofast
# -Os     optimise for small size
# -Ofast  optimise for performance
# -Og     optimise for debugging
# -g      include debugging data (use together with -Og or alone)

PKGNAME = adjbacklight
COMMAND = adjbacklight

PREFIX = /usr
BIN = /bin
BINDIR = $(PREFIX)$(BIN)
DATA = /share
DATADIR = $(PREFIX)$(DATA)
DOCDIR = $(DATADIR)/doc
INFODIR = $(DATADIR)/info
LICENSEDIR = $(DATADIR)/licenses

MANUAL = adjbacklight
MANUALDIR = info/

WARN = -Wall -Wextra -Wdouble-promotion -Wformat=2 -Winit-self -Wmissing-include-dirs            \
       -Wtrampolines -Wfloat-equal -Wshadow -Wmissing-prototypes -Wmissing-declarations          \
       -Wredundant-decls -Wnested-externs -Winline -Wno-variadic-macros -Wsync-nand              \
       -Wunsafe-loop-optimizations -Wcast-align -Wstrict-overflow -Wdeclaration-after-statement  \
       -Wundef -Wbad-function-cast -Wcast-qual -Wwrite-strings -Wlogical-op -Waggregate-return   \
       -Wstrict-prototypes -Wold-style-definition -Wpacked -Wvector-operation-performance        \
       -Wunsuffixed-float-constants -Wsuggest-attribute=const -Wsuggest-attribute=noreturn       \
       -Wsuggest-attribute=pure -Wsuggest-attribute=format -Wnormalized=nfkc -Wconversion        \
       -fstrict-aliasing -fstrict-overflow -fipa-pure-const -ftree-vrp -fstack-usage             \
       -funsafe-loop-optimizations
# excluded: -pedantic


# compile the package
.PHONY: default
default: code info

.PHONY: all
all: code doc

.PHONY: code
code: bin/adjbacklight

bin/adjbacklight: src/adjbacklight.c
	@mkdir -p bin
	$(CC) $(OPTIMISATION) $(WARN) -std=gnu90 -o "$@" "$<"

.PHONY: code
doc: info pdf dvi ps

.PHONY: info
info: bin/$(MANUAL).info
bin/%.info: $(MANUALDIR)%.texinfo
	@mkdir -p bin
	$(MAKEINFO) "$<"
	mv $*.info $@

.PHONY: pdf
pdf: bin/$(MANUAL).pdf
bin/%.pdf: $(MANUALDIR)%.texinfo
	@! test -d obj/pdf || rm -rf obj/pdf
	@mkdir -p bin obj/pdf
	cd obj/pdf && texi2pdf ../../"$<" < /dev/null
	mv obj/pdf/$*.pdf $@

.PHONY: dvi
dvi: bin/$(MANUAL).dvi
bin/%.dvi: $(MANUALDIR)%.texinfo
	@! test -d obj/dvi || rm -rf obj/dvi
	@mkdir -p bin obj/dvi
	cd obj/dvi && $(TEXI2DVI) ../../"$<" < /dev/null
	mv obj/dvi/$*.dvi $@

.PHONY: ps
ps: bin/$(MANUAL).ps
bin/%.ps: $(MANUALDIR)%.texinfo
	@! test -d obj/ps || rm -rf obj/ps
	@mkdir -p bin obj/ps
	cd obj/ps && texi2pdf --ps ../../"$<" < /dev/null
	mv obj/ps/$*.ps $@


# install to system
.PHONY: install
install: install-base install-info

.PHONY: install-all
install-all: install-base install-doc

.PHONY: install-base
install-base: install-cmd install-copyright

.PHONY: install-cmd
install-cmd: bin/adjbacklight
	install -d -- "$(DESTDIR)$(BINDIR)"
	install -m4755 -- bin/adjbacklight "$(DESTDIR)$(BINDIR)/$(COMMAND)"

.PHONY: install-copyright
install-copyright: install-copying install-license

.PHONY: install-copying
install-copying:
	install -d -- "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)"
	install -m644 -- COPYING "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)"

.PHONY: install-license
install-license:
	install -d -- "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)"
	install -m644 -- LICENSE "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)"

.PHONY: install-doc
install-doc: install-info install-pdf install-dvi install-ps

.PHONY: install-info
install-info: bin/$(MANUAL).info
	install -d -- "$(DESTDIR)$(INFODIR)"
	install -m644 -- "$<" "$(DESTDIR)$(INFODIR)/$(PKGNAME).info"

.PHONY: install-pdf
install-pdf: bin/$(MANUAL).pdf
	install -d -- "$(DESTDIR)$(DOCDIR)"
	install -m644 -- "$<" "$(DESTDIR)$(DOCDIR)/$(PKGNAME).pdf"

.PHONY: install-dvi
install-dvi: bin/$(MANUAL).dvi
	install -d -- "$(DESTDIR)$(DOCDIR)"
	install -m644 -- "$<" "$(DESTDIR)$(DOCDIR)/$(PKGNAME).dvi"

.PHONY: install-ps
install-ps: bin/$(MANUAL).ps
	install -d -- "$(DESTDIR)$(DOCDIR)"
	install -m644 -- "$<" "$(DESTDIR)$(DOCDIR)/$(PKGNAME).ps"


# remove files created by `install`
.PHONY: uninstall
uninstall:
	-rm -- "$(DESTDIR)$(BINDIR)/$(COMMAND)"
	-rm -- "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)/COPYING"
	-rm -- "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)/LICENSE"
	-rm -d -- "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)"
	-rm -- "$(DESTDIR)$(INFODIR)/$(PKGNAME).info"
	-rm -- "$(DESTDIR)$(DOCDIR)/$(PKGNAME).pdf"
	-rm -- "$(DESTDIR)$(DOCDIR)/$(PKGNAME).dvi"
	-rm -- "$(DESTDIR)$(DOCDIR)/$(PKGNAME).ps"


# remove files created by `all`
.PHONY: clean
clean:
	-rm -r bin obj

