/* min12xxw.c		driver for minolta pagepro 1[234]xxW printers
 *
 * It converts pages in pbmraw format read from stdin to the printer
 * language used by Minolta PagePro 1[234]00W. The output goes to stdout.
 *
 * Copyright (C) 2004-2006 Manuel Tobias Schiller <mala@hinterbergen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
/* Note: This program was written without any documentation from the
 * manufacturer. The description of the printer language may be
 * inaccurate or even wrong because it was obtained by looking at the
 * output of the windoof driver only. The code dealing with the
 * compression of raster data is in its spirit heavily based on an
 * initial driver by Adam Bocim <beetman@seznam.cz> who managed to
 * find out how things are done (thanks, Adam, you did a great job!).
 * This new version is considerably faster, implemented more cleanly
 * and (hopefully) well documented.
 * It also features querying the printer status and page counter, a
 * suggestion made by Bruno Schoedlbauer <bruno.schoedlbauer@gmx.de>,
 * who also pointed me to a nice USB sniffer by Benoit Papillault for
 * Windoof 98 and up, see http://benoit.papillault.free.fr/usbsnoop.
 * (This little program may prove very helpful on similar occasions.
 * Happy USB sniffing...)
 * */
/* see the file CHANGELOG for a list of changes between revisions.
 */

#include "config.h"

#define _GNU_SOURCE /* we might need this because we use getline */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>	/* usleep */
#include <sys/stat.h>	/* fstat */
#include <getopt.h>

/* include the inttypes.h for uint8_t and similar types */
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
/* don't know yet what to do in this case. just put some typedefs there
 * and hope for the best? until someone complains we just make the
 * compiler throw an error... */
#error "You'll need either stdint.h or inttypes.h"
#endif

/* make sure we get getline even if the system libc does not provide it */
#ifndef HAVE_GETLINE
#include <getline.h>
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y))?(x):(y))
#endif
#ifndef MAX /* not used now - but might come handy in the future */
#define MAX(x, y) (((x) > (y))?(x):(y))
#endif

/* we wish to know what version we are ;) */
static char *versionstr = "version " PACKAGE_VERSION " ($Id: min12xxw.c 115 2005-12-31 13:50:48Z mala $)";

/* defaults used for the output */
static int ptype = 0x00; /* normal paper */
static int pformat = 0x04; /* a4 */
static int res = 0x0001; /* 600 dpi, horizontal resolution not doubled */
static int tray = 0xff; /* auto select paper tray */
static int nomargins = 0; /* if flag is set, margins are not enforced */
static int ecomode = 0; /* if set, toner-saving mode is enabled */
static int model = 0x81; /* 12xxW series printers are the default */
static char *device = "/dev/lp0"; /* default device used for queries */

/* try to exit gracefully in case of fatal errors */
static void fatal(char *str)
{
	perror(str);
	exit(1);
}

/**********************************************************************
 * minolta esc command utility
 *
 * this is used to send printer commands and read printer response
 * packets
 * see the file format.txt for an overview of what has been found out
 * about the format used
 **********************************************************************/
static void do_cmd(FILE *out, uint8_t cmd, uint32_t len, uint8_t *data)
{
	/* command sequence number, incremented with each command */
	static uint8_t sq = 0;
	uint8_t cksum = 0; /* every command is transmitted with a checksum */
	uint8_t buf[6]; /* buffer to build up command header */
	uint8_t *end = &data[len], *p = buf;

	assert(len < 0x10000); /* do not transmit way too much data */

	/* fill in command header */
	*p++ = 0x1b; *p++ = cmd; *p++ = sq++;
	*p++ = len & 0xff; *p++ = len >> 8; *p++ = ~cmd;

	/* calculate the checksum */
	p = data;
	while (p < end) cksum += *p++;
	p = buf;
	end = &buf[6];
	while (p < end) cksum += *p++;

	/* ok, write the command to the output file */
	if (fwrite(buf, sizeof(uint8_t), 6, out) != 6)
		fatal("min12xxw: error writing command to output file.");
	if (fwrite(data, sizeof(uint8_t), len, out) != len)
		fatal("min12xxw: error writing command to output file.");
	if (fputc(cksum, out) == EOF)
		fatal("min12xxw: error writing command to output file.");
}

/* this will read data from the printer
 * note: when reading data from the printer over the USB port, we must
 * check what we get back because the printer will give back the wrong
 * register when it is not ready - in this case we sleep a bit and try
 * again */
static uint8_t *do_read(FILE *in, uint8_t reg, int *len)
{
	int c, d, i, j = 0;
	uint8_t *retVal = NULL;

	/* try to read the desired register in a loop */
	do {
		usleep(100000);
		if (retVal != NULL) free(retVal);
		if ((d = fgetc(in)) == EOF)
			fatal("min12xxw: error reading data from printer");
		if ((c = fgetc(in)) == EOF)
			fatal("min12xxw: error reading data from printer");

		if ((retVal = (uint8_t *) malloc(c * sizeof(uint8_t)))==NULL)
			fatal("min12xxw: error allocating buffer");
		for (i = 0; i < c; i++) {
			int cc;
			if ((cc = fgetc(in)) == EOF)
				fatal("min12xxw: error reading data from " \
					"printer");
			retVal[i] = (uint8_t) cc;
		}
		/* break if we have the desired register or we have waited
		 * long enough for the printer to reply and have not seen
		 * a decent answer */
	} while ((d != reg) && (j++ < 10));

	if (d != reg) {
		/* we didn't manage to read the desired register */
		free(retVal);
		*len = 0;
		return NULL;
	}

	/* we have read the desired register */
	*len = c;
	return retVal;
}

/* sends the start signal to the printer (make printer pay attention) */
static void do_start(FILE *out)
{
	uint8_t buf[2];

	buf[0] = (uint8_t) model;
	buf[1] = 0;
	do_cmd(out, 0x40, 2, buf);
}

/* sends the stop signal to the printer (terminate a command sequence) */
static void do_stop(FILE *out)
{
	uint8_t buf = 0;

	do_cmd(out, 0x41, 1, &buf);
}

/* send read register command */
static void do_readreg(FILE *out, uint8_t reg)
{
	uint8_t buf[2];

	buf[0] = reg;
	buf[1] = 0;
	do_cmd(out, 0x60, 2, buf);
}

/* send register enabler command - not sure if this is what this command
 * does */
static void do_enreg(FILE *out)
{
	uint8_t buf[3];

	/* FIXME: until we know better, we'll just treat the 1400W as a
	 * better 13xxW here - testers report that this seems to work
	 * well */
	buf[0] = (((model == 0x83) || (model == 0x86))?(0x1c):(0x78));
	buf[1] = 0;
	buf[2] = 0x04;
	do_cmd(out, 0x6a, 3, buf);
}

/* produce a quick and dirty register dump from the printer registers
 * this routine was mainly used for debugging purposes and is therefore
 * disabled by default - it was never meant to be used by a wider
 * audience */
#if 0
static void regdump(FILE *f)
{
	int i, len, state;
	uint8_t *buf = NULL, regs[] = { 0x81, 0x53, 0x05, 0x04, 0x03, 0x02 };

	do_start(f);
	buf = do_read(f, 0x4a, &len);
	printf("\nread start sequence state 0x%02x, %3d bytes:\n", 0x4a, len);
	for (state = 0; state < len; state++)
		putchar(buf[state]);
	free(buf);
	do_enreg(f);
	buf = do_read(f, 0x45, &len);
	printf("\nread reg enabler sequence state 0x%02x, %3d bytes:\n", \
		0x45, len);
	for (state = 0; state < len; state++)
		putchar(buf[state]);
	free(buf);
	for (i = 0; i < 6; i++) {
		do_readreg(f, regs[i]);
		buf = do_read(f, regs[i], &len);
		printf("\nread register 0x%02x, %3d bytes:\n", regs[i], len);
		if (len > 0) {
			for (state = 0; state < len; state++)
				putchar(buf[state]);
		}
		free(buf);
	}
	do_stop(f);
}
#endif

/* this prints whatever information we can read from the hardware */
static void printhwstate(FILE *f)
{
	uint8_t *bufcfw, *bufefw = NULL, *bufpcnt, *bufst;
	int len;

	do_start(f);
	/* the printers appear to have a sort of enabler command - I'm
	 * not sure if this is what this does - we send it anyway,
	 * because the windoof driver does it as well */
	do_enreg(f);
	/* get printer state */
	do_readreg(f, 0x04);
	bufst = do_read(f, 0x04, &len);
	if (len != 0) {
		memmove(bufst, &bufst[1], len - 1);
		bufst[len - 1] = 0;
	}
	/* get controller firmware version */
	do_readreg(f, 0x02);
	bufcfw = do_read(f, 0x02, &len);
	if (len != 14)
		goto readerr;
	/* get engine firmware version for models that support it */
	do_readreg(f, 0x81);
	bufefw = do_read(f, 0x81, &len);
	/* if len != 0 the printer appears not to have register 0x81 */
	if ((len != 30) && (len != 0))
		goto readerr;
	/* ok, read page counter */
	do_readreg(f, 0x53);
	bufpcnt = do_read(f, 0x53, &len);
	if (len != 38)
		goto readerr;
	do_stop(f);

	printf("printer status: %s\n", bufst);
	free(bufst);
	printf("controller firmware version: %c%c%c%c\n", \
		bufcfw[3], bufcfw[2], bufcfw[1], bufcfw[0]);
	free(bufcfw);
	if (bufefw != NULL) {
		/* print engine firmware version if the model supports it */
		memmove(bufefw, &bufefw[18], 12);
		bufefw[12] = 0;
		printf("engine firmware version: %s\n", bufefw);
		free(bufefw);
	}
	len = bufpcnt[30] | (bufpcnt[31] << 8) | (bufpcnt[32] << 16) | \
	      (bufpcnt[33] << 24);
	printf("page counter: %lu pages\n", (unsigned long) len);
	free(bufpcnt);
	return;

readerr:
	do_stop(f); /* try not to hang the printer */
	fatal("min12xxw: read unexpected data from printer");
}

/* this procedure sends raster data to the printer */
static void send_raster_data(FILE *out, uint32_t nlines, \
				uint32_t len, uint8_t *data)
{
	uint8_t cmdbuf[6], *p = cmdbuf;

	assert(nlines < (1 << 16)); /* just to make sure */
	*p++ = len & 0xff; *p++ = (len >> 8) & 0xff;
	*p++ = (len >> 16) & 0xff; *p++ = len >> 24;
	*p++ = nlines & 0xff;
	*p++ = (nlines >> 8) & 0xff;
	/* we start with sending the command */
	do_cmd(out, 0x52, 6, cmdbuf);
	/* the data is appended immediately */
	if (fwrite(data, sizeof(uint8_t), len, out) != len)
		fatal("min12xxw: couldn't send raster data to output file");
}

/* this procedure sends a start-of-job/select-resolution-and-papertype
 * sequence to the printer */
static void send_start_job(FILE *out)
{
	uint8_t cmdbuf[8];

	memset(cmdbuf, 0x00, 8 * sizeof(uint8_t)); /* zero our buffer */


	do_start(out); /* start-of-printer-commands command */

	cmdbuf[0] = (uint8_t) (res & 0xff); /* set resolution and paper type */
	/* check if we are to use doubled horizontal resolution */
	cmdbuf[1] = (uint8_t) (res >> 8);
	cmdbuf[3] = (uint8_t) ptype;
	cmdbuf[4] = 0x04;
	/* 1[34]xxW series models might expect an 0x04 in cmdbuf[6] as well */
	if ((model == 0x83) || (model == 0x86)) cmdbuf[6] = 0x04;
	do_cmd(out, 0x50, 8, cmdbuf); /* issue it */
}

/* this procedure sends an end-of-job sequence to the printer */
static void send_end_job(FILE *out)
{
	uint8_t cmdbuf = 0;

	do_cmd(out, 0x55, 1, &cmdbuf);
	do_stop(out);
}

/* the following procedure sends a new page command */
static void send_new_page(FILE *out, uint32_t x, uint32_t y)
{
	uint8_t cmdbuf[22], *p = &cmdbuf[1];

	memset(cmdbuf, 0x00, 22 * sizeof(uint8_t)); /* zero our buffer */
	*p++ = 0x01;
	*p++ = (x >> 16) & 0xff; *p++ = x >> 24;
	*p++ = x & 0xff; *p++ = (x >> 8) & 0xff;
	*p++ = (y >> 16) & 0xff; *p++ = y >> 24;
	*p++ = y & 0xff; *p++ = (y >> 8) & 0xff;
	*p++ = 0x08; p++; *p++ = 0x08; p++;
	*p++ = tray; *p++ = pformat;
	if ((res & 0xff) == 0) {
		/* apparently, 300 dpi needs special flags set here */
		cmdbuf[20] = 0xc0;
	}
	do_cmd(out, 0x51, 22, cmdbuf); /* issue our command */
}

/**********************************************************************
 * raster data compression
 **********************************************************************/
/* the following table keeps sixteen bytes which can be sent to the
 * printer by referring to their table index (4 bit only)
 * this table is filled anew for every scanline */
static int tbllen = -1; /* all tables uninitialized by default */
static uint8_t tbl[16];
static uint8_t invtbl[256]; /* this is for inverse lookups */

/* return the number of repetitions of a byte at position pointed to by p */
static uint32_t get_len(uint8_t *p, uint8_t *end)
{
	uint8_t c = *p++;
	uint32_t len = 0;

	while ((p < end) && (c == *p++)) len++;

	return ++len;
}

/* add a byte to the table and return the table index to it */
static uint8_t add_tbl(uint8_t b)
{
	/* is there space in the table or is the byte already there?
	 * if the table is full, returning invtbl[b] is still correct
	 * doing it this way has the advantage of saving a constant load
	 * because we have the value already at hand (from the test) */
	if ((invtbl[b] < 16) || (tbllen >= 16)) return invtbl[b];
	/* ok, add it to the table */
	tbl[tbllen] = b;
	invtbl[b] = tbllen;
	return tbllen++;
}

/* check if the next n bytes are in the table, or if we have space for them */
static int next_n_in_tbl(uint8_t *p, long len, uint8_t *end)
{
	long i = 0, j = len;

	/* make sure we don't read past the end of our data */
	if ((p + len) >= end) return 0;

	/* count how many bytes are in the table */
	while (j--)
		if (invtbl[*p++] < 16) i++;

	/* check if we have space for those which are not */
	if (tbllen < (17 - len + i)) return 1;

	return 0; /* sorry - no space in table left... */
}

/* compress a scanline of data and write the result to an output buffer */
static uint32_t compress_scanline(uint8_t *p, uint8_t *end, uint8_t *obuf)
{
	uint32_t olen = 0;
	uint8_t *q; /* temporary pointer */

	/* clear the table and inverse lookup table - we try to do
	 * partial clearing of the inverse lookup table if possible,
	 * because reading and writing 16 or fewer bytes should be faster
	 * than writing 256 bytes... Can't do that the first time, though */
	if (tbllen == -1) memset(invtbl, 0xff, 256 * sizeof(uint8_t));
	else for (q = tbl; q < &tbl[tbllen]; q++) invtbl[*q] = 0xff;
	tbllen = 0;

	while (p < end) {
		uint32_t n = get_len(p, end);
		if (n > 2) {
			/* RLE compression pays off */
			/* make sure n is small enough - we're paranoid */
			assert(n < (63 * 64 + 63));
			if (n > 63) {
				/* how to encode multiples of 64 bytes */
				*obuf++ = 0xc0 | (n >> 6);
				*obuf++ = *p;
				olen += 2;
				p += n & (~0x3f);
				n &= 0x3f;
			}
			if (n) {
				/* encode the remainder */
				*obuf++ = 0x80 | n;
				*obuf++ = *p;
				olen += 2;
				p += n;
			}
		} else if (next_n_in_tbl(p, 4, end)) {
			/* ok - try to do table compression */
			/* the next four bytes are in the table or there
			 * is enough space to put them into the table
			 * we issue a 0x41 code to indicate that we send
			 * table indices - the 0x41 means that we send
			 * a quartet of table indices - we might even
			 * send more, see below */
			q = obuf; /* back up current position */
			*obuf++= 0x41;
			*obuf = add_tbl(*p++) << 4;
			*obuf++ |= add_tbl(*p++);
			*obuf = add_tbl(*p++) << 4;
			*obuf++ |= add_tbl(*p++);
			olen += 3;
			/* check if we can send even more table indices */
			while (next_n_in_tbl(p, 2, end) && (*q < 0x7f)) {
				/* break if we can RLE-code the next bytes */
				if (get_len(p, MIN(p + 3, end)) >= 3) break;
				/* we send one more pair of table indices */
				(*q)++;
				*obuf = add_tbl(*p++) << 4;
				*obuf++ |= add_tbl(*p++);
				olen++;
			}
		} else {
			/* can't do table compression - fall back to send
			 * up to 10 bytes in uncompressed plaintext */
			q = obuf; /* back up current output position */
			*obuf++ = 0xff;
			olen++;
			do {
				*obuf++ = *p++;
				olen++;
				(*q)++; /* update # of plaintext bytes */
				/* three repeating bytes ahead? if so,
				 * do RLE compression instead */
				if (get_len(p, MIN(p + 3, end)) >= 3) break;
				/* see if we can do table compression for
				 * the bytes ahead - comment the next line
				 * of code out if you absolutely need the
				 * speed and can live with things compressing
				 * a little worse (on the order of a percent
				 * or so) */
				if (next_n_in_tbl(p, 4, end)) break;
			} while ((p < end) && (*q < 9));
		}
	}

	return olen;
}

/********************************************************************** 
 * process all the pages in the rawpbm input job
 **********************************************************************/
/* printer has non-printable margins of 17/100" on all sides of a sheet
 * of paper which are present in ghostscript output and need to be
 * removed - these are in units of 8 pixels; note - this is not exact
 * to the last decimal place but it has to be good enough
 * update this if new printers allow for new resolutions codes or equal
 * codes with different meanings!!! */
static int skiptbl[3] = { 6, 13, 25 };

/* processes a single page */
static void dopage(FILE *in, FILE *out, uint32_t x, uint32_t y)
{
	uint32_t sclbytes = x / 8; /* # of bytes per scanline */
	/* # of scanlines per block (rounded up) */
	uint32_t sclperbl = y / 8 + ((y & 7)?1:0);
	/* size of buffer for processed data - note: the estimate given
	 * below is conservative - the buffer can not become larger... */
	uint32_t blblen = sclperbl * (17 + sclbytes + sclbytes / 10 + 1);
	uint8_t *scl, *blbuf; /* buffers for scanline and output block */
	uint32_t bllen; /* block length so far */
	uint32_t sclen; /* length of scanline excluding table */
	uint32_t yy, yc; /* counters */
	/* this controls how many pixels are skipped on each side of a
	 * sheet of paper to keep the margins that the printer expects */
	int skip = ((nomargins == 0)?skiptbl[res & 0xff]:0);
	int i; /* counter */
	int ecofl = 0; /* flag used for toner-saving mode */

	/* allocate buffers */
	if ((scl = (uint8_t *) malloc(sclbytes * sizeof(uint8_t))) == NULL)
		fatal("min12xxw: couldn't allocate memory");
	if ((blbuf = (uint8_t *) malloc(blblen * sizeof(uint8_t))) == NULL)
		fatal("min12xxw: couldn't allocate memory");

	yc = 0;
	for (yc = 0; yc < skip * 8; yc++) {
		/* skip the first few scanlines in case we enforce margins */
		if (fread(scl, sizeof(uint8_t), sclbytes, in) != sclbytes)
			fatal("min12xxw: couldn't read scanline");
	}
	/* we will always build eight blocks per page - the windows driver
	 * seems to do the same thing */
	for (i = 0; i < 8; i++) {
		bllen = 0;
		/* process the scanlines in the block */
		for (yy = 0; (yy < sclperbl) && (yc < y); yy++, yc++) {
			/* read in the next scanline */
			if (fread(scl, sizeof(uint8_t), sclbytes, in) != \
				sclbytes)
				fatal("min12xxw: couldn't read scanline");
			/* check if we need to do this line (we don't work
			 * for the margins...) */
			if ((yc + skip * 8) > y) continue;
			/* check if in toner-saving mode */
			if (ecomode) {
				/* clear every other scanline */
				if (ecofl) memset(scl, 0,
						sclbytes * sizeof(uint8_t));
				ecofl = ~ecofl;
			}
			/* compress a scanline - leave some space for the
			 * table (we'll probably have to remove a hole) */
			sclen = compress_scanline(&scl[skip], \
						&scl[sclbytes-skip], \
						blbuf + 17);
			blbuf[0] = 0x80 + tbllen; /* copy table */
			if (tbllen) memcpy(blbuf + 1, tbl, tbllen);
			if (tbllen < 16) /* remove the hole */
				memmove(blbuf + 1 + tbllen, blbuf + 17, sclen);
			blbuf += sclen + 1 + tbllen;
			bllen += sclen + 1 + tbllen;
		}
		blbuf -= bllen;
		/* account for skipped scanlines in the last block */
		if (i == 7) yy -= 8 * skip;
		/* send the raster data to the printer */
		send_raster_data(out, yy, bllen, blbuf);
	}
	free(scl); /* avoid memory leaks */
	free(blbuf);
}

/* processes all the pages in a pbmraw job file */
static void dojob(FILE *in, FILE *out)
{
	uint32_t x, y; /* page dimesions in dots at the current resolution */
	char *line = NULL; /* buffer for reading whole lines from the input */
	size_t llen = 0; /* length of line buffer */
	/* how much to skip because of margins */
	int skip = ((nomargins == 0)?(16 * skiptbl[res & 0xff]):0);

	/* send the printer a start-of-job/set-resolution-and-papertype
	 * sequence */
	send_start_job(out);

	/* process pages */
	while (!feof(in) && !ferror(in)) {
		/* read the png header for x and y dimensions */
		if (getline(&line, &llen, in) == -1) break;
		/* check for valid pbmraw signature */
		if ((line[0] != 'P') || (line[1] != '4'))
			fatal("min12xxw: input is not valid pbmraw "
				"(no valid signature)");
		/* read in comment lines and the line containing the
		 * resolution */
		do {
			if (getline(&line, &llen, in) == -1)
				fatal("min12xxw: input is not valid pbmraw "
					"(premature end of file)");
		} while (line[0] == '#');
		/* parse resolution */
		{
			/* this ugly hack is needed to make things work
			 * on 64 bit platforms like sparc64 or amd64 */
			unsigned long lx, ly;
			if (sscanf(line, "%lu %lu", &lx, &ly) != 2)
				fatal("min12xxw: input is not valid pbmraw "
					"(ill formatted bitmap dimensions)");
			x = lx; y = ly;
		}
		/* scanlines are byte-aligned - adjust x accordingly */
		if (x & 0x7) x = 8 + (x & ~0x7);
		/* check if the page dimensions are so small that we need
		 * switch of the margins
		 * many thanks go to Ben Cooper who alerted me to the
		 * problem */
		if ((((y - skip) / 8) <= skip) || (x <= (2 * skip))) {
			if (!nomargins)
				fprintf(stderr, "min12xxw: page dimensions"
					" are so small that I won't "
					"enforce page margins for this and"
					"all subsequent pages!\n");
			nomargins = 1;
			skip = 0;
		}

		/* tell the printer to start a new page - note the page
		 * dimensions are reduced because we need to take the
		 * margins into account - see above */
		send_new_page(out, x - skip, y - skip);
		/* read, process and send the raster data for the next
		 * page - this needs to know the real dimensions because
		 * it needs to know how much data to read... */
		dopage(in, out, x, y);
	}

	/* tell the printer we got our job finished... */
	send_end_job(out);

	free(line); /* free memory to avoid leaks... */
}

/**********************************************************************
 * option handling
 **********************************************************************/
/* the next few tables contain the mappings from human-readable strings
 * to id numbers the printer understands (or our routines, for that
 * matter) */
struct map {
	char *str;
	int id;
};
static struct map ptypes[] = {
	{ "normal", 0x00 }, { "thick", 0x01 }, { "transparency", 0x02 },
	{ "postcard", 0x03 }, { "envelope", 0x03 }, { NULL, -1 }
};
static struct map trays[] = {
	{ "auto", 0xff }, { "tray1", 0x00 }, { "tray2", 0x01 },
	{ "manual", 0x80 }, { NULL, -1 }
};
static struct map pformats[] = {
	{ "a4", 0x04 }, { "b5", 0x06 }, { "a5", 0x08 }, { "jpost", 0x0c },
	{ "corpost", 0x0d }, { "jisy6", 0x10 }, { "jisy0", 0x11 },
	{ "chinese16k", 0x13 }, { "chinese32k", 0x15 }, { "legal", 0x19 },
	{ "glegal", 0x1a }, { "letter", 0x1b }, { "gletter", 0x1d },
	{ "executive", 0x1f }, { "halfletter", 0x21 }, { "envmonarch", 0x24 },
	{ "env10", 0x25 }, { "envdl", 0x26 }, { "envc5", 0x27 },
	{ "envc6", 0x28 }, { "envb5", 0x29 }, { "choukei3gou", 0x2d },
	{ "choukei5gou", 0x2e }, { "custom", 0x31 }, { "envb6", 0x31 },
	{ "folio", 0x31 }, { "jisy1", 0x31 }, { "jisy2", 0x31 },
	{ "quadpost", 0x31 }, { NULL, -1 }
};
static struct map models[] = {
	{ "1200W", 0x81 }, { "1250W", 0x81 },
	{ "1300W", 0x83 }, { "1350W", 0x83 },
	{ "1400W", 0x86 }, { NULL, -1 }
};
static struct map resolutions[] = {
	{  "300", 0x0000 }, {   "300x300", 0x0000 },
	{  "600", 0x0001 }, {   "600x600", 0x0001 },
	{ "1200", 0x0002 }, { "1200x1200", 0x0002 },
	{ "1200x600", 0x0101 }, { NULL, -1 } 
};

/* this procedure is used by help() below to print available somethings
 * in a nice and painless way from the maps above
 * see help() below and everything will become clearer */
static void printav(char *msg, struct map *m, int defid)
{
	int i, n;
	
	n = printf(msg);
	/* print all available somethings from our map */
	for (i = 0; m[i].str != NULL; i++) {
		/* in case we had to start a new line below, we need to
		 * print a tab here */
		if (n == 0) {
			printf("\t");
			n = 8;
		}
		/* print the next something */
		n += printf("%s", m[i].str);
		/* is it the default? if so, print "*" */
		if (m[i].id == defid)
			n += printf("*");
		/* print ", " unless it's the last something */
		if (m[i + 1].str != NULL)
			n += printf(", ");
		/* check if we have to start a new line */
		if (n > 72) {
			printf("\n");
			n = 0;
		}
	}
	/* unless the last thing we did was to start a new line, we append
	 * a newline here */
	if (n != 0) printf("\n");
}

/* this procedure gives help */
static void help()
{
	printf("min12xxw - a pbmraw to Minolta PagePro 1[234]xx W "
		"filter\n\noptions: (defaults are marked with an"
		" asterisk)\n"
		"\t-h\t--help\t\t\tthis help\n"
		"\t-v\t--version\t\tshow version number\n"
		"\t-d dev\t--device dev\t\tset device to use for "
			"queries (%s by default)\n"
		"\t-s\t--status\t\tquery printer status\n"
		"\t-e\t--ecomode\t\teconomic (toner saving) mode\n"
		"\t-n\t--nomargins\t\tdisable enforcement of margins\n"
		"\t-m mod\t--model mod\t\tset the printer model to "
			"produce output for\n"
		"\t-r res\t--res res\t\tset resolution\n"
		"\t-t tray\t--tray tray\t\tset paper tray\n"
		"\t-p type\t--papertype type\tset paper type"
		"\n\t-f fmt\t--paperformat fmt\tset paper format\n"
		"\n", device);
	/* print the available somethings */
	printav("available models: ", models, model);
	printav("available resolutions: ", resolutions, res);
	printav("available paper trays: ", trays, tray);
	printav("available paper types: ", ptypes, ptype);
	printav("available paper formats: ", pformats, pformat);
	printf("\n");
	exit(0);
}

/* this procedure prints out version information */
static void version()
{
	printf("min12xxw: %s\n", versionstr);
	printf("\nCopyright (C) 2004-2006 Manuel Tobias Schiller\n"
		"This is free software; see the source for copying "
		"conditions.  There is NO\nwarranty; not even for "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	exit(0);
}

/* convert string to id number */
static int getid(struct map *m, char *thing, char *str, int deflt)
{
	while (m->str != NULL) {
		if (strcasecmp(str, m->str) == 0) return m->id;
		m++;
	}
	fprintf(stderr, "min12xxw: unknown %s: %s\n", thing, str);
	return deflt; /* return the default */
}

/* parse the options given to the filter */
static void parseopts(int argc, char **argv)
{
	/* this structure is needed by getopt_long */
	static struct option opts[] = {
		{ "res",		required_argument, 0, 'r' },
		{ "papertype",		required_argument, 0, 'p' },
		{ "paperformat",	required_argument, 0, 'f' },
		{ "tray",		required_argument, 0, 't' },
		{ "nomargins",		no_argument,	   0, 'n' },
		{ "ecomode",		no_argument,	   0, 'e' },
		{ "version",		no_argument,	   0, 'v' },
		{ "device",		required_argument, 0, 'd' },
		{ "status",		no_argument,	   0, 's' },
		{ "model",		required_argument, 0, 'm' },
		{ "help",		no_argument,	   0, 'h' },
		{ NULL,			0,		   0,   0 }
	};
	int optidx = 0, dostate = 0;
	int c;
	int lres = res, lptype = ptype, lpformat = pformat, lmodel = model;
	int ltray = tray, lnomargins = nomargins, lecomode = ecomode;
	char *ldevice = device;

	opterr = 1; /* let getopt_long do the error reporting */
	while (42) { /* this line is for all hitch-hikers out there */
		c = getopt_long(argc, argv, "hsevnd:r:p:f:t:m:", opts, &optidx);
		if (c == -1) break;
		switch (c) {
		case 'h':
			help();
		case 'v':
			version();
		case 'r':
			lres = getid(resolutions, "resolution", optarg, 0x0001);
			break;
		case 'p':
			lptype = getid(ptypes, "paper type", optarg, 0);
			break;
		case 'f':
			lpformat = getid(pformats, "paper format", optarg, 4);
			break;
		case 'm':
			lmodel = getid(models, "printer model", optarg, 0x81);
			break;
		case 't':
			ltray = getid(trays, "tray", optarg, 0xff);
			break;
		case 'n':
			lnomargins = 1;
			break;
		case 'd':
			ldevice = optarg;
			break;
		case 's':
			dostate = 1;
			break;
		case 'e':
			lecomode = 1;
			break;
		default:
			/* getopt_long told the user about the unknown option
			 * character, all we have to do now is bail out... */
			exit(1);
		}
	}
	/* copy the local copies of options to the global ones
	 * we need to do this copying to make sure that help sees the
	 * global defaults, not what the users sets with options */
	res = lres; ptype = lptype; pformat = lpformat; model = lmodel;
	tray = ltray; nomargins = lnomargins; device = ldevice;
	ecomode = lecomode;
	if (dostate) {
		/* process printer queries right away and exit */
		FILE *d;
		if ((d = fopen(device, "r+b")) == NULL)
			fatal("min12xxw: couldn't fdopen printer");
		printhwstate(d);
		if (fclose(d) != 0) /* close the printer */
			fatal("min12xxw: couldn't close printer");
		exit(0); /* exit */
	}
}

/* adjust the model defaults to the name of the executable
 * do some magic for the users to spare them from having to type model
 * selection options again and again and again */
static void modeladj(char *str)
{
	int len = strlen(str);

	if (len < 8) return; /* don't do anything on short strings */
	/* we are only interested in the end of the string */
	str = &str[len - 8];
	/* if it is a model name, set the defaults accordingly */
	if (strcmp(str, "min1200w") == 0) model = 0x81;
	if (strcmp(str, "min1250w") == 0) model = 0x81;
	if (strcmp(str, "min1300w") == 0) model = 0x83;
	if (strcmp(str, "min1350w") == 0) model = 0x83;
	if (strcmp(str, "min1400w") == 0) model = 0x86;
}

/**********************************************************************
 * main program
 **********************************************************************/
int main(int argc, char **argv)
{
	FILE *tmp;
	size_t i;
	char *buf;
	int fd;
	struct stat st;

	/* adjust the default model based on the name of the executable */
	modeladj(argv[0]);
	/* this is really boring now... */
	parseopts(argc, argv); /* parse any options given */
	/* if we write to a file or a pipe, we won't need a temp file so
	 * we'll use fstat to check for this */
	fd = fileno(stdout);
	if (fstat(fd, &st) != 0)
		fatal("min12xxw: couldn't examine stdout via fstat");
	if (S_ISREG(st.st_mode) || S_ISFIFO(st.st_mode)) {
		tmp = stdout;
	} else {
		/* create temp file to hold the output until we're done */
		if ((tmp = tmpfile()) == NULL)
			fatal("min12xxw: couldn't create temporary file");
	}
	
	/* filter our input */
	dojob(stdin, tmp);
	
	if (stdout != tmp) {
		/* if we used a temp file we need to copy output from temp
		 * file to stdout */
		rewind(tmp);
		if ((buf = (char*) malloc(16384 * sizeof(char))) == NULL)
			fatal("min12xxw: couldn't allocate memory");
		while ((!feof(tmp)) && (!ferror(tmp))) {
			i = fread(buf, sizeof(char), 16384, tmp);
			if (i <= 0) continue;
			if (fwrite(buf, 1, i, stdout) != i)
				fatal("min12xxw: couldn't write to stdout");
		}
		free(buf);
	}

	/* all went well :) */
	return 0;
}
