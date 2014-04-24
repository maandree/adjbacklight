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
.PHONY: all
all: code info

.PHONY: code
code: bin/adjbacklight

bin/adjbacklight: src/adjbacklight.c
	mkdir -p bin
	$(CC) $(OPTIMISATION) $(WARN) -std=gnu90 -o "$@" "$<"

.PHONY: info
info: $(MANUAL).info
%.info: $(MANUALDIR)%.texinfo
	$(MAKEINFO) "$<"

.PHONY: pdf
pdf: $(MANUAL).pdf
%.pdf: $(MANUALDIR)%.texinfo
	texi2pdf "$<"

.PHONY: dvi
dvi: $(MANUAL).dvi
%.dvi: $(MANUALDIR)%.texinfo
	$(TEXI2DVI) "$<"


# install to system
.PHONY: install
install: install-cmd install-license install-info

.PHONY: install-cmd
install-cmd: bin/adjbacklight
	install -d -- "$(DESTDIR)$(PREFIX)$(BIN)"
	install -m4755 -- bin/adjbacklight "$(DESTDIR)$(PREFIX)$(BIN)/$(COMMAND)"

.PHONY: install-license
install-license:
	install -d -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"
	install -m644 -- COPYING LICENSE "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"

.PHONY: install-info
install-info: $(MANUAL).info
	install -d -- "$(DESTDIR)$(PREFIX)$(DATA)/info"
	install -m644 -- "$(MANUAL).info" "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info"


# remove files created by `install`
.PHONY: uninstall
uninstall:
	-rm -- "$(DESTDIR)$(PREFIX)$(BIN)/$(COMMAND)"
	-rm -- "$(DESTDIR)$(PREFIX)$(DATA)$(LICENSES)/$(PKGNAME)/COPYING"
	-rm -- "$(DESTDIR)$(PREFIX)$(DATA)$(LICENSES)/$(PKGNAME)/LICENSE"
	-rm -d -- "$(DESTDIR)$(PREFIX)$(DATA)$(LICENSES)/$(PKGNAME)"
	-rm -- "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info.gz"


# remove files created by `all`
.PHONY: clean
clean:
	-rm -r *.{class,t2d,aux,cp,cps,fn,ky,log,pg,pgs,toc,tp,vr,vrs,op,ops,bak,info,pdf,ps,dvi,gz,install} bin

