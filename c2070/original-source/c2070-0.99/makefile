#
# makefile - simple makefile for the Lexmark 2070 color driver
#
# Copyright 1999, Christian Kornblum
#
#

# Important compiler and linker options
CC=gcc
LD=gcc
CFLAGS=
LDFLAGS=

# Required libraries
LDLIBS=

# Source files and modules
SRC=c2070.c
SHAREDHEADER=
MOD=$(SRC:.c=.o)

# Standard production rule
.c.o: 
	$(CC) -c $< $(CFLAGS)

# make all
all: c2070

# linking the modules
c2070: $(MOD)
	$(LD) -o $@ $(MOD) $(LDLIBS) $(LDFLAGS)

# dependencies, here a shared header
$(SRC): $(SHAREDHEADER) 

# clear up the mess to start over
clean: 
	rm *.o c2070

#install the driver
install: all
	cp c2070 /usr/bin
	chmod a+x /usr/bin/c2070


