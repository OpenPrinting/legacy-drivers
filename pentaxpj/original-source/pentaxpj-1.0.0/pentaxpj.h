/*
 * Printer Filter for Pentax PocketJet / PocketJet II
 * (C) Copyright 2001, Rildo Pragana <rildo@pragana.net>
 * This is free software, released under the General Public License (GPL)
 * The development of this work was sponsored by Pentax Technologies.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <linux/serial.h>
#include <errno.h>

#define FALSE 0
#define TRUE 1

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

/* timeout reading pipes */
#define PSHDR_TIMEOUT   120000
//#define PSDATA_TIMEOUT     200
#define PSDATA_TIMEOUT  5000

/* misc constants */
#define HDRLINE_SIZE   200
#define COMP_THRESHOLD  16
#define PAGE_TOP_SKIP300   80
#define PAGE_TOP_SKIP200   54
#define DEFAULT_PAPER_WIDTH   8.50
#define DEFAULT_PAPER_HEIGHT 11.00

#define CFGFN "/etc/pentaxpj.conf"

/* end of pentaxpj.h */

