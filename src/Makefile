SHELL        = /bin/sh
INSTALL      = /usr/bin/ginstall -c

prefix       = /usr/local
exec_prefix  = ${prefix}
BINDIR       = /usr/local/bin
MANDIR       = ${prefix}/man

PRG          = zoom.cgi

.SUFFIXES: .cxx

                C = gcc
           CFLAGS = -march=pentium -O5 -funroll-loops -ffast-math -finline-functions

SRC = zoom.c

#######################
#
# Targets.
#
#######################

.PHONY: all install uninstall install-man uninstall-man



all:
	$(C) $(CFLAGS) $(SRC) -ljpeg -o $(PRG)
	chmod +x $(PRG)
#install

install:
	$(INSTALL) -o root -g root -d $(BINDIR)
	$(INSTALL) -o root -g root -m 4755 $(PRG) $(BINDIR)/$(PRG)

uninstall:
	rm $(BINDIR)/$(PRG)

install-man:
	cp $(MAN) $(MANDIR)/man6/$(addsuffix .6,$(basename $(MAN)))

uninstall-man:
	rm $(MANDIR)/man6/$(addsuffix .6,$(basename $(MAN)))

distrib:
	tar zcfv $(PRG)-1.0.tar.gz zoom/*
