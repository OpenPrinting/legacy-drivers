#
# makefile - simple makefile for the Lexmark 2050 color driver
#
# Copyright 2000-2007, Marco Nenciarini
# Copyright 1999, Christian Kornblum
#
# License: GPL (GNU Public License)
#

# Important compiler and linker options
CC=gcc
LD=gcc
CFLAGS=-g -O2
LDFLAGS=-g -O2

# Required libraries
LDLIBS=

# Source files and modules
SRC=c2050.c
SHAREDHEADER=
MOD=$(SRC:.c=.o)

# Standard production rule
.c.o: 
	$(CC) $(CFLAGS) -c $<

# make all
all: c2050

# linking the modules
c2050: $(MOD)
	$(LD) $(LDFLAGS) $(LDLIBS) -o $@ $(MOD) 

# dependencies, here a shared header
$(SRC): $(SHAREDHEADER) 

# clear up the mess to start over
clean: 
	rm -f *.o *~ core c2050

#install the driver
install: all
	install -m 0755 c2050 $(DESTDIR)/usr/bin
	install -m 0755 ps2lexmark $(DESTDIR)/usr/bin
	install -m 0755 c2050.1 $(DESTDIR)/usr/share/man/man1/
	install -m 0755 ps2lexmark.1 $(DESTDIR)/usr/share/man/man1/
