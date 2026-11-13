Linux Canon LBP-460/660 driver
------------------------------

In this package there is a linux driver for the Canon LBP-660 and
LBP-460 printers.

Most of my work is based on Rildo Pragana's driver for Samsung ML-85G,
see : http://pragana.net - "Adventures in Linux Programming".

It should support both A4 and Letter sized paper, but I only tested it
with A4 sized paper.

Install
-------

Type:

# make

Then log in as root and type:

#make install

Install in CUPS
---------------

Edit restartcups.sh and uncomment (remove #) the line which restart
CUPS on your operating system. Then type, replacing 660 by 460 if you
are using a Canon LBP-460 :

#make cups-install-660-a4

or 

#make cups-install-660-letter
