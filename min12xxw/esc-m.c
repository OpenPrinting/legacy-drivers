/* esc-m.c - try to parse minolta pagepro 1200 w printer data files
 *
 * Copyright (C) 2004, 2005 Manuel Tobias Schiller <mala@hinterbergen.de>
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
/*
 * this little program was meant to make understanding the output
 * of the printer driver easier. i guess it might sometimes still
 * be useful...
 *
 * this is a quick and _dirty_ hack based on looking at the windoze
 * driver output and some of the windoze driver plaintext files
 * (most notably msdmlt_c.sdd for the paper format codes - this
 * file might contain other valuable information I do not understand
 * yet...)
 *
 * this thing may or may not provide you with accurate information
 * the purpose of some of the data sent to the printer is still not
 * understood - have a look at format.txt and look for entries which
 * do not satisfy a curious mind...
 * the raster data is hex dumped by this utility
 */

#include <stdio.h>

/* file position ans sequence number */
static int i = 0, sq = 0;
/* current resolution */
static int gl_res = -1, gl_res_h = -1;
/* page dimensions in dots at current resolution */
static int gl_x, gl_y;
/* paper tray */
static int gl_tray = -1;
/* paper format */
static int gl_pformat = -1;

static int getdword(unsigned char *data)
{
	int retVal;

	retVal = ((int) data[1]) << 24;
	retVal |= ((int) data[0]) << 16;
	retVal |= ((int) data[3]) << 8;
	retVal |= ((int) data[2]);

	return retVal;
}

static void dumpraw(unsigned char *data)
{
	int tmp, lb;
	/* raw data dump */
	printf("ESC %02x: raw data dump:", data[1]);

	lb = data[3];
	data += 6;

	for (tmp = 0; tmp < lb; tmp++) {
		if ((tmp & 0xf) == 0) printf("\n\t");
		printf("%02x ", (int) *data++);
	}
	printf("\n\n");
}

static void dump50(unsigned char *data)
{
	char *res, *ptype, *rmstr;
       	unsigned char r, p, rm;
	int lb;

	lb = data[3];

	/* dump ESC 50 data */
	printf("ESC 50: select resolution and paper type:\n");

	if (lb != 8) {
		printf("\tExpected 8 data bytes for ESC 0x50 command, recieved %d.\n", lb);
		dumpraw(data);
		return;
	}
	/* we dump the raw data nevertheless to make is easier to spot
	 * format changes when Minolta introduces a new model... */
	dumpraw(data);

	data += 6;

	switch (r = *data++) {
	case 0:
		res = "300 dpi"; gl_res = 300;
		break;
	case 1:
		res = "600 dpi"; gl_res = 600;
		break;
	case 2:
		res = "1200 dpi"; gl_res = 1200;
		break;
	default:
		res = "unknown"; gl_res = -1;
		break;
	}
	switch (rm = *data++) {
	case 0:
		rmstr = "none";
		gl_res_h = gl_res;
		break;
	case 1:
		rmstr = "double horizontal resolution";
		gl_res_h = gl_res * 2;
		break;
	default:
		rmstr = "unknown horizontal resolution modifier";
		gl_res_h = -1;
		break;
	}
	data += 1; /* next two bytes seem to be don't care data */
	switch (p = *data++) {
	case 0:
		ptype = "normal paper";
		break;
	case 1:
		ptype = "thick paper";
		break;
	case 2:
		ptype = "transparency";
		break;
	case 3:
		ptype = "envelope/postcard";
		break;
	default:
		ptype = "unknown paper";
		break;
	}
	/* next four bytes seem to be don't care data */

	printf("\tresolution code %02x (%s)\n", r, res);
	printf("\thorizontal resolution modifier: %02x (%s)\n", rm, rmstr);
	printf("\teffective resolution is %d x %d dpi\n", gl_res_h, gl_res);
	printf("\tpaper code %02x (%s)\n\n", p, ptype);
}

static void dump51(unsigned char *data)
{
	unsigned char lb;
       	char *tray, *pformat;

	lb = data[3];

	/* dump ESC 51 data */
	printf("ESC 51: start new page and set paper format:\n");

	if (lb != 22) {
		printf("\tExpected 22 data bytes for ESC 0x51 command, recieved %d.\n", lb);
		dumpraw(data);
		return;
	}
	/* we dump the raw data nevertheless to make is easier to spot
	 * format changes when Minolta introduces a new model... */
	dumpraw(data);
	data += 6;

	data += 2; /* seems to be don't care data */
	gl_x = getdword(data); data += 4;
	gl_y = getdword(data); data += 4;
	data += 4; /* seems to be don't care data */
	switch (gl_tray = *data++) {
	case 0xff:
		tray = "auto"; break;
	case 0x00:
		tray = "tray 1"; break;
	case 0x01:
		tray = "tray 2"; break;
	case 0x80:
		tray = "manual feed"; break;
	default:
		tray = "unknown"; break;
	}
	switch (gl_pformat = *data) {
	default:
		pformat = "unknown"; break;
	case 0x04:
		pformat = "a4"; break;
	case 0x06:
		pformat = "b5"; break;
	case 0x08:
		pformat = "a5"; break;
	case 0x0c:
		pformat = "j-post"; break;
	case 0x0d:
		pformat = "cor. post"; break;
	case 0x10:
		pformat = "jis y6"; break;
	case 0x11:
		pformat = "jis y0"; break;
	case 0x13:
		pformat = "chinese 16k"; break;
	case 0x15:
		pformat = "chinese 32k"; break;
	case 0x19:
		pformat = "legal"; break;
	case 0x1a:
		pformat = "g. legal"; break;
	case 0x1b:
		pformat = "letter"; break;
	case 0x1d:
		pformat = "g. letter"; break;
	case 0x1f:
		pformat = "executive"; break;
	case 0x21:
		pformat = "half letter"; break;
	case 0x24:
		pformat = "env monarch"; break;
	case 0x25:
		pformat = "env #10"; break;
	case 0x26:
		pformat = "env dl"; break;
	case 0x27:
		pformat = "env c5"; break;
	case 0x28:
		pformat = "env c6"; break;
	case 0x29:
		pformat = "env b5"; break;
	case 0x2d:
		pformat = "choukei-3gou"; break;
	case 0x2e:
		pformat = "choukei-4gou"; break;
	case 0x31:
		pformat = "custom"; break;
	}
	/* rest seems to be don't care data... */

	printf("\tpaper format is %02x (%s).\n", gl_pformat, pformat);
	printf("\tpage size is %d x %d dots (%.3lf x %.3lf \" or %.2lf x %.2lf mm).\n",
		gl_x, gl_y,
		(double) gl_x / (double) gl_res_h,
		(double) gl_y / (double) gl_res,
		25.4 * (double) gl_x / (double) gl_res_h,
		25.4 * (double) gl_y / (double) gl_res);
	printf("\tpaper tray id %02x (%s)\n\n", gl_tray, tray);
}

static void dump52(unsigned char *data)
{
	int tmp2, tmp;
	unsigned char lb = data[3];

	/* dump ESC 52 data */
	printf("ESC 52: send raster data:\n");

	if (lb != 6) {
		printf("\tExpected 6 data bytes for ESC 0x52 command, recieved %d.\n", lb);
		dumpraw(data);
		return;
	}
	data += 6;

	/* copy further data bytes */
	/* get length out of data array */
	tmp2 = data[3]; tmp2 <<= 8;
	tmp2 |= data[2]; tmp2 <<= 8;
	tmp2 |= data[1]; tmp2 <<= 8;
	tmp2 |= data[0];
	/* get number of lines */
	tmp = data[5]; tmp <<= 8;
	tmp |= data[4];
	printf("\tdumping %d raster data bytes (%d lines):", tmp2, tmp);
	for (tmp = 0; tmp < tmp2; tmp++) {
		if ((tmp & 0xf) == 0) printf("\n\t");
		printf("%02x ", getchar());
		i++;
	}
	printf("\n\n");
}

static int readesc(unsigned char *data)
{
	unsigned char *p = data, lb, ck;

	ck = *p = getchar();
	if (feof(stdin)) {
		printf("Reached EOF at %08x.\n", i);
		return -1;
	}
	if (*p++ != 0x1b) {
		printf("Expected start of a ESC sequence at %08x.\n", i);
		return -2;
	}
	i++;
	/* cmd byte */
	ck += *p++ = getchar(); i++;
	ck += *p = getchar();
	if (*p != sq++) {
		/* ESC 0x51 seems to set sequence numbers, also make
		 * concatetnations of print files work */
		if ((*p != 0) && (data[1] != 0x51)) {
			printf("Sequence error (is %02x, should be %02x) at %08x.\n",
				(int) *p, sq - 1, i);
			return -3;
		} else {
			printf("Sequence restarts at %08x.\n", i);
			sq = *p + 1;
		}
	}
	p++;
	i++;
	ck += lb = *p++ = getchar(); i++;
	*p = getchar();
	if (*p++ != 0) {
		printf("Expected zero byte at %08x.\n", i);
		return -4;
	}
	i++;
	ck += *p = getchar();
	if (*p++ != (~data[1] & 0xff)) {
		printf("Command %02x not terminated at %08x.\n", data[1], i);
		return -5;
	}
	i++;

	while (lb--) {
		ck += *p++ = getchar();
		i++;
	}

	*p = getchar();
	if (*p != ck) {
		printf("Checksum error at %08x.\n", i);
		return -6;
	}

	return data[1];
}

int main()
{
	unsigned char data[300];
	int c;

	while (!feof(stdin)) {
		c = readesc(data);
		if (c < 0) return c;

		switch (c) {
		case 0x50:
			dump50(data); break;
		case 0x51:
			dump51(data); break;
		case 0x52:
			dump52(data); break;
		default:
			dumpraw(data); break;
		}
	}

	return 0;
}
