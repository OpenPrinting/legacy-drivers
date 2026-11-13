# Makefile of printer driver for Samsung ML-85G
# Recife, Agosto de 2000 -- Rildo Pragana

CC=gcc
CFLAGS=-O2 -g

all: ml85p 

ml85p: ml85p.c ml85p.h 
	${CC} ${INCLUDE} ${CFLAGS} ml85p.c -o ml85p
	chmod +s ml85p

install:
	install -s -m a=rxs ml85p /usr/local/bin
	install -m a=rxs ml85-print /usr/local/bin
	
clean:
	rm ml85p

