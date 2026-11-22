
CC=gcc
CFLAGS=-O2 -g  -DLONG_FORM_FEED 

all: pentaxpj

pentaxpj: pentaxpj.c pentaxpj.h
	${CC} ${INCLUDE} ${CFLAGS} $< -o $@
	ln -sf ./pentaxpj ./pentaxpj_sh

package:
	cvs2cl.pl
	rm -rf pentaxpj pentaxpj_sh ChangeLog.bak
	(cd ..;tar cvzf pentaxpj-`date +%Y%m%d-%H`.tar.gz \
		-X pentaxpj/exclude pentaxpj)
	
clean:
	rm -rf pentaxpj pentaxpj_sh ChangeLog.bak

