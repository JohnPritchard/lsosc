## "@(#) $Id: Makefile.in,v 1.11 2004/05/27 01:55:25 jpritcha Exp $"
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

RM = rm
RMOPTS = -fv

SUBDIRS    = libsrc src

ifeq ($(HOST),w1d5tcs)
  RMOPTS = -f
else
endif

all:
	set -e; for i in $(SUBDIRS); do\
	  cd $$i ;\
	  $(MAKE) all ;\
	  cd .. ;\
	done

install:
	set -e; for i in $(SUBDIRS); do\
	  cd $$i ;\
	  $(MAKE) install ;\
	  cd .. ;\
	done

.PHONY: clean dep 
clean:
	set -e; for i in $(SUBDIRS); do\
	  cd $$i ;\
	  $(MAKE) clean ;\
	  cd .. ;\
	done

realclean:
	set -e; for i in $(SUBDIRS); do\
	  cd $$i ;\
	  $(MAKE) realclean ;\
	  cd .. ;\
	done
	$(RM) $(RMOPTS) Makefile config.cache config.status config.log

tgz.dist:
	cd tgz ;\
	mkdir -p lsosc ;\
	cvs export -D now -d lsosc dk154sc ;\
	$(RM) $(RMOPTS) -r lsosc/tgz ;\
	tar -cvzf lsosc-`date +%Y-%m-%dT%H_%M_%S`.tgz lsosc ;\
	$(RM) $(RMOPTS) -r lsosc ;\
	cd ..


## Suffixes ##
.c.o:
	$(CC) -c $(INCLUDE) $(CFLAGS) $(GGDB) $(PG) $<

dep:
	gcc -MM -MG ${INCLUDE} *.cc > .depend

-include .depend
