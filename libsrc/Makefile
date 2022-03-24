## "@(#) $Id: Makefile.in,v 1.13 2006/07/20 13:06:44 jpritcha Exp $"
# Makefile for lsosc

#  Copyright (C) 2000  J.D.Pritchard

#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.

#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

CC = gcc-mp-8
CFLAGS = -I /usr/local/astro/eb/src/libskycalc/include
#### For Linux
#CFLAGS = -O -Wall -Wno-unused
#### For HPUX
#CFLAGS = -Ae -O -Wall

AR = ar
AR_OPT = ruv
RANLIB = ranlib
INSTALL = /opt/local/bin/ginstall -c
STRIP = strip
RM = rm
RMOPTS = -fv

prefix = /usr/local
exec_prefix = ${prefix}
bindir = $(exec_prefix)/bin
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
infodir = ${prefix}/info

LDKD       = ..
LDKINC     = -I $(LDKD)/include
LDKLIB     = $(LDKD)/lib
LDKLIBA    = $(LDKLIB)/liblsosc.a
LDKLIBO    = liblsosc.o

INCLUDE    = $(LDKINC) -I ../../libskycalc/include

SUBDIRS =

ifeq ($(HOST),w1d5tcs)
 INCLUDE    = $(LDKINC) -I../../libskycalc/include
  RMOPTS = -f
else
endif

all:	$(LDKLIBA)

install:
	@echo "Nothing to do..."

.PHONY: clean dep

clean:
	$(RM) $(RMOPTS) *.o

realclean: clean
	$(RM) $(RMOPTS) $(LDKLIBA)
	$(RM) $(RMOPTS) Makefile

distclean:

$(LDKLIBA)		:  $(LDKLIBO)
	$(AR) $(AR_OPT) $(LDKLIBA) $(LDKLIBO)
	$(RANLIB) $(LDKLIBA)


## Suffixes ##
.c.o:
	$(CC) -c $(INCLUDE) $(CFLAGS) $(GGDB) $(PG) $<

dep:
	gcc -MM -MG ${INCLUDE} *.cc > .depend

-include .depend
