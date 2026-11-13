# Makefile of printer driver for Canon LBP-660/LBP-460
# Belfaux, June-July 2004 -- Nicolas Boichat
# Adapted from Samsung ML-85P driver by Rildo Pragana

CC=gcc
#CFLAGS=-O2 -g
CFLAGS=-O2 -s

RELEASE=0.3.1

all: lbp660 lbp460

lbp660: lbp660.c lbp660.h Makefile
	${CC} ${INCLUDE} ${CFLAGS} lbp660.c -o lbp660
	chmod +s lbp660

lbp460: lbp460.c Makefile
	${CC} ${INCLUDE} ${CFLAGS} lbp460.c -o lbp460
	chmod +s lbp460

cups-install: install foomatic 
	install -m a=rxs ppd/Canon-LBP-660-lbp660.ppd /usr/share/cups/model
	install -m a=rxs ppd/Canon-LBP-460-lbp460.ppd /usr/share/cups/model
	echo "Restarting CUPS..."
	./restartcups.sh
	echo "CUPS restarted."

cups-install-660: cups-install
	/usr/sbin/lpadmin -x LBP-660 | /bin/true
	/usr/sbin/lpadmin -p LBP-660 -E -m Canon-LBP-660-lbp660.ppd -v file:/dev/null

cups-install-660-a4: cups-install-660
	lpoptions -p LBP-660 -o PageSize=A4
	lpoptions -p LBP-660 -o LeftSkip=70
	lpoptions -p LBP-660 -o Topskip=100
	lpoptions -p LBP-660 -o AlwaysReset=True

# TODO: Check topskip and leftskip parameters
cups-install-660-letter: cups-install-660
	lpoptions -p LBP-660 -o PageSize=Letter
	lpoptions -p LBP-660 -o LeftSkip=70
	lpoptions -p LBP-660 -o Topskip=100
	lpoptions -p LBP-660 -o AlwaysReset=True

cups-install-460: cups-install
	/usr/sbin/lpadmin -x LBP-460 | /bin/true
	/usr/sbin/lpadmin -p LBP-460 -E -m Canon-LBP-460-lbp460.ppd -v file:/dev/null

# TODO: Check topskip and leftskip parameters
cups-install-460-a4: cups-install-460
	lpoptions -p LBP-460 -o PageSize=A4
	lpoptions -p LBP-460 -o LeftSkip=70
	lpoptions -p LBP-460 -o Topskip=100
#	lpoptions -p LBP-460 -o AlwaysReset=True

# TODO: Check topskip and leftskip parameters
cups-install-460-letter: cups-install-460
	lpoptions -p LBP-460 -o PageSize=Letter
	lpoptions -p LBP-460 -o LeftSkip=70
	lpoptions -p LBP-460 -o Topskip=100
	lpoptions -p LBP-460 -o AlwaysReset=True

foomatic: foomatic-rip foomatic-gswrapper
	echo "Installing foomatic..."
	install -m a=rx foomatic-rip /usr/bin
	install -m a=rx foomatic-gswrapper /usr/bin
	rm -f /usr/lib/cups/filter/foomatic-rip
	ln -s /usr/bin/foomatic-rip /usr/lib/cups/filter/foomatic-rip
	echo "Foomatic installed."

foomatic-rip:
	wget http://www.linuxprinting.org/foomatic-rip
	chmod 755 foomatic-rip

foomatic-gswrapper:
	wget http://www.linuxprinting.org/foomatic-gswrapper
	chmod 755 foomatic-gswrapper

install: all
	install -s -m a=rxs lbp660 /usr/bin
	install -s -m a=rxs lbp460 /usr/bin
#	install -m a=rxs lbp660-print /usr/bin
#	install -m a=rxs lbp460-print /usr/bin
	
clean:
	rm -f lbp660 lbp460

clean-all: clean
	rm -f foomatic-rip foomatic-gswrapper

package: clean
	rm -f *~ ppd/*~
	cp -a ../code ../lbp660-${RELEASE}
	tar cvfz ../lbp660-${RELEASE}.tar.gz ../lbp660-${RELEASE}
