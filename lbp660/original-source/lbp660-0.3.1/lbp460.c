/*
 *  Printer driver for Canon LBP-460 laser printer
 *  
 *  Adapted from LBP-660 driver by Indan Zupancic
 *
 *  Copyright (C) 2004 Nicolas Boichat <nicolas@boichat.ch>
 * 
 *  Adapted from a printer driver for Samsung ML-85G laser printer
 *  (C) Copyleft, 2000 Rildo Pragana <rpragana@acm.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <unistd.h> /* needed for ioperm() */
#include <sys/io.h> /* for outb() and inb() */
#include <sys/time.h>
#include <string.h>	/* for memset */
#include <stdlib.h> /* exit() */

#define LINE_SIZE			600
#define LINES_BY_PAGE460	3484
#define ROWS_BY_BAND		104 	// number of rows in a band
#define MAX_PACKET_COUNT	3072	// Maximum number of packet in a transfer
#define PAGE_DELAY			25  	// Delay between pages, in seconds

#define DATA 0x378
//#define DATA	0x3bc
#define STATUS	(DATA+1)
#define CONTROL	(DATA+2)

/* packet members for compressed data */
#pragma pack(1)
union pkt1 {
	struct {
		unsigned char al:6,rle:1,t:1;
	} bits;
	unsigned char c;
};
union pkt2 {
	struct {
		unsigned char ah:2,bl:4,pa:1,t:1;
	} bits;
	unsigned char c;
};
union pkt3 {
	struct {
		unsigned char bh:4,cl:2,pa:1,t:1;
	} bits;
	unsigned char c;
};
union pkt4 {
	struct {
		unsigned char ch:6,pa:1,t:1;
	} bits;
	unsigned char c;
};
#pragma pack()

int pagedata[] = {
-1, 0x89, /*100*/
-1, 0x8a, /*172*/
-1, 0x8b, /*244*/
-3, 0x8b, 0x89, 0x8c, /*333*/
-7, 0x8c, 0x4, 0x94, 0x3f, 0x95, 0x58, 0x94, /*456*/
-1, 0x95, /*528*/
-1, 0x89, /*600*/
-1, 0x8a, /*672*/
-1, 0x8b, /*744*/
-1, 0x89, /*816*/
-1, 0x8a, /*888*/
-1, 0x8b, /*960*/
-3, 0x8b, 0x89, 0x90, /*1049*/
-5, 0x91, 0x0, 0x90, 0x0, 0x89, /*1155*/
-1, 0x8a, /*1227*/
-1, 0x8b, /*1299*/
-1, 0x89, /*1372*/
-1, 0x8a, /*1444*/
-3, 0x8d, 0xa7, 0x8b, /*1533*/
-3, 0x8b, 0x8a, 0x90, /*1622*/
-3, 0x90, 0x8, 0x89, /*1712*/
-1, 0x8a, /*1784*/
-1, 0x8e, /*1856*/
-1, 0x90, /*1928*/
-3, 0x90, 0x9, 0x89, /*2018*/
-1, 0x8a, /*2090*/
-3, 0x8d, 0x40, 0x90, /*2179*/
-5, 0x90, 0xc9, 0xa0, 0x0, 0xa0, /*2285*/
-17, 0x81, 0xdc, 0x82, 0x0, 0x83, 0x61, 0x84, 0x0, 0x85, 0x58, 0x86, 0x2, 0x87, 0x9c, 0x88, 0x1a, 0x81, /*2493*/
-1, 0x82, /*2565*/
-1, 0x83, /*2637*/
-1, 0x84, /*2709*/
-1, 0x85, /*2781*/
-1, 0x86, /*2853*/
-1, 0x87, /*2925*/
-1, 0x88, /*2997*/
-1, 0x89, /*3070*/
-1, 0x8a, /*3142*/
-1, 0x8e, /*3214*/
-1, 0x89, /*3287*/
-1, 0x8a, /*3359*/
-3, 0x8d, 0x2, 0x89, /*3449*/
-1, 0x8a, /*3521*/
-1, 0x8e, /*3593*/
-1, 0x89, /*3666*/
-1, 0x8a, /*3738*/
-3, 0x8d, 0x9d, 0x89, /*3828*/
-1, 0x8a, /*3900*/
-1, 0x8e, /*3972*/
-1, 0x89, /*4045*/
-1, 0x8a, /*4117*/
-3, 0x8d, 0x2, 0x89, /*4207*/
-1, 0x8a, /*4279*/
-1, 0x8e, /*4351*/
-3, 0x93, 0x0, 0x89, /*4441*/
-1, 0x8a, /*4513*/
-3, 0x8d, 0x2, 0x89, /*4603*/
-1, 0x8a, /*4675*/
-1, 0x8e, /*4747*/
-1, 0x89, /*4820*/
-1, 0x8a, /*4892*/
-1, 0x89, /*4965*/
-1, 0x8a,
-256,
-260 }; //, /*5037*/

int bandinit[] = {
-1, 0x8a, /*25157*/
-1, 0x8e, /*25229*/
-1, 0x89, /*25302*/
-1, 0x8a, /*25374*/
-1, 0x89, /*25447*/
-1, 0x8a, /*25519*/
-3, 0x8d, 0x1, 0x89, /*25609*/
-1, 0x8a, /*25681*/
-1, 0x89, /*25754*/
-1, 0x8a, /*25826*/
//-3, 0x80, 0xff, 0x7f, /*25909*/
-256, /* data */ 
-260
};

/* Rildo Pragana constants and functions */
FILE *bitmapf = 0;
FILE *cbmf = 0;
int bmcnt=0;
unsigned char bmbuf[800];	/* the pbm bitmap line with provision for leftskip */
unsigned char *bmptr=bmbuf;
int bmwidth=0, bmheight=0;
unsigned char cbm[300000];	/* the compressed bitmap */
unsigned char garbage[600];
unsigned char *cbmp=cbm;
int csize=0;				/* compressed line size */
int last_ctrl=0x06;
int linecnt=0;
int pktcnt;
int topskip=100;
int leftskip=70;

unsigned char parity[] = {
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};

void
bitmap_seek (int offset) {
	if (offset) {
		while (offset > sizeof(garbage)) {
			fread(bmbuf,1,sizeof(garbage),bitmapf);
			offset -= sizeof(garbage);
		}
		fread(bmbuf,1,offset,bitmapf);
	}	
}

unsigned char
get_bitmap () {
	FILE *dbgf;
	int i,k,tmp;
	if (bmcnt==0) {
		memset(bmbuf,0,800);
		//if (!feof(bitmapf)) {
		if (linecnt<(bmheight-topskip)) {
			if (bmwidth > 800) {
				fread(bmbuf,1,800,bitmapf);
				bitmap_seek(bmwidth-800);
			}
			else {
				fread(bmbuf,1,bmwidth,bitmapf);
			}
		}
		bmptr = bmbuf+leftskip/8;
		bmcnt = LINE_SIZE;
		linecnt++;
	}
	bmcnt--;
	return *bmptr++;
}

void
next_page(int page) {
	/* we can't use fseek here because it may come from a pipe! */
	int skip;
	skip = (bmheight - topskip - linecnt) * bmwidth;
	fprintf(stderr, "bmheight = %d, bmwidth = %d, leftskip = %d, topskip = %d, linecnt = %d, skip = %d\n", 
		bmheight, bmwidth, leftskip, topskip, linecnt, skip);
	if (skip>0)
		bitmap_seek(skip);
	linecnt=0;
}

void
out_packet ( int rle, unsigned char a, unsigned char b, unsigned char c ) {
	union pkt1 pk1;
	union pkt2 pk2;
	union pkt3 pk3;
	union pkt4 pk4;
	FILE *dbgf;
	int tmp;
	
	if (rle == 2) { // flush packet storage
		if (a == 1) { // truncated band
			tmp = csize | 0x10000;
			fwrite( &tmp,1,sizeof(int),cbmf );
		}
		else {
			fwrite( &csize,1,sizeof(int),cbmf );
		}
		
		fwrite( cbm,csize,4,cbmf );
		cbmp = cbm;
		//fprintf(stderr, "Flushing packet %d...\n", csize);
		csize=0;
		return;
	}
		
	pk1.bits.t=0;
	pk2.bits.t=1;
	pk3.bits.t=0;
	pk4.bits.t=1;
	pk1.bits.rle = rle;
	pk1.bits.al = a&0x3f;
	pk2.bits.ah = (a>>6)&0x3;
	pk2.bits.bl = b&0xf;
	pk3.bits.bh = (b>>4)&0xf;
	pk3.bits.cl = c&0x3;
	pk4.bits.ch = (c>>2)&0x3f;
	pk2.bits.pa = parity[pk2.c&0x3f];
	pk3.bits.pa = parity[pk3.c&0x3f]^1;
	pk4.bits.pa = parity[pk4.c&0x3f];
	cbmp += sprintf( cbmp,"%c%c%c%c",pk1.c,pk2.c,pk3.c,pk4.c );
	csize++;
	pktcnt++;
	
	if (csize == MAX_PACKET_COUNT) {
		fprintf(stderr, "Max packet size reached, flushing...\n");
		out_packet(2,1,0,0);
	}
}

int
compress_bitmap () {
	int line,band;
	unsigned char c1,c2,c3;
	int cnt;					/* count of characters processed in a band */
	int	pcnt;					/* count of chars for each packet */

	if (fgets(cbm,200,bitmapf) == NULL) {
		return 0;
	}
	if (strncmp(cbm,"P4",2)) {
		fprintf(stderr,"Wrong file format.\n");
		fprintf(stderr,"file position: %x\n",ftell(bitmapf));
		exit(1);
	}
	/* bypass the comment line */
	do {
		fgets(cbm,200,bitmapf);
	} while (cbm[0] == '#');
	/* read the bitmap's dimensions */
	if (sscanf(cbm,"%d %d",&bmwidth,&bmheight)<2) {
		fprintf(stderr,"Bitmap file with wrong size fields.\n");
		exit(1);
	}
	bmwidth = (bmwidth+7)/8;
	/* adjust top and left margins */
	if (topskip) {
		/* we can't do seek from a pipe
		fseek(bitmapf,bmwidth*topskip,SEEK_CUR);*/
		bitmap_seek(bmwidth*topskip);
	}

	bmcnt = 0; /* Needed, otherwise corrupt all but first page */

	/*printf("COMPRESS : bmcnt %d bmwidth %d bmheight %d csize %d linecnt %d pktcnt %d topskip %d leftskip %d\n", 
			bmcnt, bmwidth, bmheight, csize, linecnt, pktcnt, topskip, leftskip);*/

	/* now we process the real data */
	for	(band=0; linecnt < LINES_BY_PAGE460; ++band){
		pktcnt=0;
		/* setup packet size */
		if (((LINES_BY_PAGE460 - linecnt) > ROWS_BY_BAND)) {
			cnt = LINE_SIZE * ROWS_BY_BAND;
		}
		else {
			cnt = LINE_SIZE * (LINES_BY_PAGE460 - linecnt);
		}
		fprintf(stderr,"cnt: %d, band: %d, linecnt: %d\n",cnt,band,linecnt);
		c1 = get_bitmap();
		c2 = get_bitmap();
		cnt -= 2;
		pcnt=1;
		while (cnt) {
			if ((c1==c2) && (cnt>2)) {
				pcnt++;
				c2 = get_bitmap();
				cnt--;
				/* add more packets if there is room to */
				/*if ((cnt < LINE_SIZE) &&
					((pktcnt + pcnt/256) < 5000) &&
					(linecnt<lines_by_page)) {
						cnt += LINE_SIZE;
				}*/
				continue;
			}
			if (cnt==2) {	
				/* leave at least 2 bytes */
				while (pcnt > 258) {
					out_packet( 1,255,c1,c1 );
					pcnt -= 257;
				}
				/* one more if too large for one packet */
				if (pcnt > 256) {
					out_packet( 1,253,c1,c1 );
					pcnt -= 255;
				}
				out_packet( 1,(pcnt-1),c1,c2 );
				break;
			}
			if ((cnt==3) || (cnt==4)) {
				if (pcnt>1) {
					while (pcnt > 259) {
						out_packet( 1,255,c1,c1 );
						pcnt -= 257;
					}
					if (pcnt > 257) {
						out_packet( 1,253,c1,c1 );
						pcnt -= 255;
					}
					out_packet( 1,(pcnt-2),c1,c1 );
					c3 = get_bitmap();
					if (cnt==3) {
						out_packet( 1,0,c2,c3 );
					}
					else {
						c1 = get_bitmap();
						out_packet( 0,c2,c3,c1 );
					}
				}
				else {
					c3 = get_bitmap();
					if (cnt==3) {
						out_packet( 0,c1,c2,c3 );
					}
					else {
						out_packet( 1,0,c1,c2 );
						c1 = get_bitmap();
						out_packet( 1,0,c3,c1 );
					}
				}
				break;
			}
			if (pcnt>1) {
				while (pcnt > 258) {
					out_packet( 1,255,c1,c1 );
					pcnt -= 257;
				}
				if (pcnt > 256) {
					out_packet( 1,253,c1,c1 );
					pcnt -= 255;
				}
				out_packet( 1,(pcnt-1),c1,c2 );
				pcnt=1;
				c1 = get_bitmap();
				c2 = get_bitmap();
				cnt-=2;
			}
			else {
				c3 = get_bitmap();
				out_packet( 0,c1,c2,c3 );
				c1 = get_bitmap();
				c2 = get_bitmap();
				cnt-=3;
			}
			/* add more packets if there is room to */
			/*	if ((cnt < LINE_SIZE) &&
					((pktcnt + pcnt/256) < 5000) &&
					(linecnt<lines_by_page)) {
						cnt += LINE_SIZE;
				}*/
		}
		out_packet(2,0,0,0);
	}
	fflush(cbmf);
	fseek(cbmf,0,SEEK_SET);
	return 1;
}

/* End of Rildo Pragana constants and functions */


void ssleep(const int usec)
{
	struct timeval lasttv;
	struct timeval newtv;
	int time;
	gettimeofday(&lasttv, NULL);
	do {
		gettimeofday(&newtv, NULL);
		time = (newtv.tv_usec - lasttv.tv_usec) + 
				((newtv.tv_sec - lasttv.tv_sec)*1000000);
	} while (time < usec);
}

inline void dataout(int data)
{
	outb(data, DATA);
}

inline void ctrlout(int cmd)
{
	outb(cmd, CONTROL);
}

inline int ctrlin()
{
	return inb(CONTROL);
}

void checkctrl(int control)
{
	int ctrl = inb(CONTROL);
	if ((ctrl & 0x1f) != (control & 0x1f)){
		fprintf(stderr, "Error, wrong control : %x instead of %x\n", ctrl, control);
		exit(1);
	}
}

inline int statusin()
{
	return inb(STATUS);
}

void checkstatus(int status)
{
	int stat = statusin();
	if ((stat & 0xf8) != (status & 0xf8)){
		fprintf(stderr, "Error, wrong status : %x instead of %x\n", stat, status);
		exit(1);
	}
}

int cmdout(int cmd)
{
	int stat;	
	ctrlout(cmd);
	ssleep(1);
	stat = statusin();
	checkctrl(cmd);
	return stat;
}

void checkcmdout(int cmd, int status, int mask)
{
	int stat = cmdout(cmd);
	if ((stat & mask) != (status & mask)) {
		fprintf(stderr, "Error, wrong status (checkcmdout) : %x instead of %x (mask : %x)\n",
				 stat, status, mask);
		exit(1);
	}
}

int cmddataouts(int cmd, int data, int sleep)
{
	int stat;	
	ctrlout(cmd);
	ssleep(sleep);
	stat = statusin();
	dataout(data);
	checkctrl(cmd);
	return stat;
}

void checkcmddataouts(int cmd, int data, int status, int mask, int sleep)
{
	int stat = cmddataouts(cmd, data, sleep);
	if ((stat & mask) != (status & mask)) {
		fprintf(stderr, "Error, wrong status (checkcmddataout) : %x instead of %x (mask : %x)\n",
				 stat, status, mask);
		exit(1);
	}
}

inline void checkcmddataout(int cmd, int data, int status, int mask)
{
	checkcmddataouts(cmd, data, status, mask, 15);
}

void data6out(int data)
{
	//fprintf(stderr, "6 O0,%x\n", data);
	// Must be : cmdout(2, 4[e6])
	checkcmddataout(0x06, data, 0x70, 0x70);
	ctrlout(0x06);
	ssleep(10);
	checkcmdout(0x7, 0x70, 0x70);
	checkcmdout(0x6, 0x70, 0x70);
	ctrlout(0x06);
	//fprintf(stderr, "data6 : 0x%x -> 0x%x\n", data, statusin());
}

void data64out(int* data, int start, int end)
{
	int i;
	// Must be : cmdout(2, 4[e6])
	checkcmddataout(0x06, data[start], 0x70, 0x70);
	//fprintf(stderr, "data64[%d] - 6 : 0x%x -> 0x%x\n", start-0, data[start], statusin());
	//fprintf(stderr, "64 (%d, %d) O0,%x", start, end, data[start]);
	for (i = start + 1; i < end; i += 2) {
		ctrlout(0x06);
		checkcmdout(0x07, 0x70, 0x70);
		//fprintf(stderr, "data64 - 7 : -> 0x%x\n", statusin());
		checkcmddataout(0x06, data[i], 0x70, 0x70);
		//fprintf(stderr, "data64[%d] - 6 : 0x%x -> 0x%x\n", start-i, data[i], statusin());
		//fprintf(stderr, ",%x", data[i]);
		
		ctrlout(0x04);
		checkcmdout(0x05, 0x70, 0x70);
		//fprintf(stderr, "data64 - 5 : -> 0x%x\n", statusin());
		checkcmddataout(0x04, data[i+1], 0x70, 0x70);
		//fprintf(stderr, "data64[%d] - 4 : 0x%x -> 0x%x\n", start-i-1, data[i+1], statusin());
		//fprintf(stderr, ",%x", data[i+1]);
	}
	//fprintf(stderr, "\n");
	ctrlout(0x06);
	checkcmdout(0x7, 0x70, 0x70);
	checkcmdout(0x6, 0x70, 0x70);
	ctrlout(0x06);
}

void reset_printer()
{
	int i = 0;
	int sig = 0;
	int ret = 0;
	int offset = 0;

	fprintf(stderr, "Reseting the printer...\n");
	dataout(0x24);
	//ssleep(40000);	
	//checkctrl(0xc6);
	dataout(0x06);
	ssleep(100);
	//checkctrl(0xc6);
	ctrlout(0x0a);
	ctrlout(0x0a);
	ctrlout(0x0e);
//	ssleep(1000000); //16
	ssleep(500000);
	dataout(0x24);
	checkctrl(0xce);
	ctrlout(0x06);
	ssleep(150); /* 100-250 */
	checkstatus(0x3e);
	checkctrl(0xc6);
	ctrlout(0x07);
	ctrlout(0x07);
	ctrlout(0x04);
	ssleep(40);
	checkstatus(0xde);
	checkctrl(0xc4);
	ctrlout(0x06);
	ssleep(40);
	checkstatus(0xfe);
	ssleep(10);
	checkctrl(0xc6);
	ctrlout(0x06);
	sig = 0; /* true if a 5e has been received */
	i = 0; /* 0e - 4e count */
	while (1) {
		ret = cmdout(0x02) & 0xf8;
		if (ret == 0x08) {
			i++;
		}
		else if (ret == 0x18) {
			i = 0;		
		}
		else {
			fprintf(stderr, "Error, wrong status (init 2nd loop) : %x instead of 0x[01]8\n", ret);
			exit(1);
		}
		
		if (sig && (i == 21)) {
			break;
		}
		
		ret = cmdout(0x00) & 0xf8;
		if (ret == 0x48) {
			i++;
		}
		else if (ret == 0x58) {
			i = 0;
			sig = 1;
		}
		else {
			fprintf(stderr, "Error, wrong status (init 2nd loop) : %x instead of 0x[45]8\n", ret);
			exit(1);
		}	 
	}
	checkcmdout(0x06, 0x78, 0x78);
	ctrlout(0x04);
	checkcmdout(0x0c, 0x28, 0x78);
	ctrlout(0x0c);
	ssleep(15);
	dataout(0x20);
	checkctrl(0xcc);
	checkcmdout(0x06, 0x38, 0x78);
	ctrlout(0x07);
	ctrlout(0x07);
	ctrlout(0x04);
	ssleep(40);
	checkstatus(0xde);
	checkctrl(0xc4);
	ctrlout(0x06);
	ssleep(40);
	checkstatus(0xfe);
//	sleep(2);
	sleep(1);
	for (i = 0; i < 12287; i++) {
		dataout(0);
	}
	ssleep(500);
	checkstatus(0xfe);
	dataout(0xa0);
	checkctrl(0xc6);
	ctrlout(0x06);
	checkcmdout(0x07, 0x78, 0x78);
	ctrlout(0x06);
	ssleep(10);
	checkstatus(0xfe);
	dataout(0x00);
	checkctrl(0xc6);	
	ctrlout(0x04);
	checkcmdout(0x05, 0x78, 0x78);
	ctrlout(0x04);
	ssleep(20);
	checkstatus(0xfe);
	dataout(0xa0);
	checkctrl(0xc4);
	ctrlout(0x06);
	checkcmdout(0x07, 0x78, 0x78);
	checkcmdout(0x06, 0x78, 0x78);
	ctrlout(0x06);
	offset = 0;
	
	fprintf(stderr, "Done.\n");
}

void initband()
{
	ctrlout(0x02);
	checkctrl(0xc2);
	checkcmddataouts(0x06, 0x80, 0x70, 0x70, 1);
//	ctrlout(0x06);
	checkcmdout(0x07, 0x70, 0x70);
	checkcmddataouts(0x06, 0xff, 0x70, 0x70, 1);

	ctrlout(0x04);
	ctrlout(0x05);
}

/* band : index of the band
 * size : size of the band
 * white : should we send only a white band ?
 * timeout : should we timeout (1), or wait for paper forever (0)
 */
int print_band(int band, int size, int white, int timeout)
{
	int i;
	unsigned char *buf;
	int ret;
	int time = -1;
	
	buf = cbm;
	fprintf(stderr, "Initing band(%d - %d - %d - %d)...\n", band, size, white, timeout);
	initband();

	if (((ret = statusin()) & 0xf0) != 0x70){
		fprintf(stderr, "Waiting for ready status...\n");
		struct timeval tv; /* Begin time */
		struct timeval ntv; /* Current time */
		int secs = 0;
		gettimeofday(&tv, NULL);
		do {
			usleep(1);
			gettimeofday(&ntv, NULL);
			time = (ntv.tv_usec - tv.tv_usec)/1000000 + ntv.tv_sec - tv.tv_sec;
			/* This is needed to prevent the paper from getting stuck: */
			if (time > 1){
				++secs;
				initband();
				gettimeofday(&tv, NULL);
			}
			/* arbitrary timeout of 60 seconds, probably too long. */
			if (secs > 60){
				fprintf(stderr, "Band initialisation failed (0x%x) %i\n", ret, time);
				return 0;
			}
		} while (((ret = statusin()) & 0xf0) != 0x70);
	}
	fprintf(stderr, "Band inited (0x%x) after %d sec\n", ret, time);

	/* data */
	if (white){
		for (i = 0; i < 242; i++){
			dataout(0x7f);
			dataout(0x83);
			dataout(0x40);
			dataout(0x80);	
		}
		dataout(0x4d);
		dataout(0x83);
		dataout(0x40);
	} else {
		for (i = 0; i < size; i++){
			dataout(*buf++);
			dataout(*buf++);
			dataout(*buf++);
			dataout(*buf++);
		}
	}
	checkctrl(0xc5);
	checkcmddataouts(0x04, 0x89, 0x70, 0x70, 3000);
	ctrlout(0x06);
	checkcmdout(0x07, 0x70, 0x70);
	checkcmdout(0x06, 0x70, 0x70);
	ctrlout(0x06);
	return ret;
}

int print_page(int page)
{
	/*	inited:
		0: the printer is not ready
		1: started to init the page
		2: started to print (there is paper) */
	int inited = 0;
	int i = 0;	 // Band counter
	int ret = 0;
	int offset = 0;
	int len = 0;
	int size;
	int* data = (int*)&pagedata;

	fprintf(stderr, "Sending page %i...\n", page);
	while (1){
		ret = cmdout(0);
		if ((cmdout(2) & 0xf0) == 0x40){ // 0x40 or 0x48
			if (!inited)
				inited = 1;
			len = -data[offset];
			if (len == 260){
				offset = 0;
				data = (int*)&bandinit;
			} else if (len > 255){
				fprintf(stderr, "Sending band %d...\n", i);
				if (!feof(cbmf) && fread(&size, 1, sizeof(int), cbmf)){
					size = size & 0x0FFF;
					fread( cbm,size*4,1,cbmf);
					ret = print_band(i, size, 0, (inited - 1) || (i == 0));
					if (!ret){
						fprintf(stderr, "len > 255 and return\n");
						return 0;
					}
					else if ((ret & 0xf0) != 0x70) {
						inited = 2;
					}
				}
#if 0			/* Fill if there is not enough data, is it really necessary ? */
				 else if ((i > 0) && (i < 50)) {
					fprintf(stderr, "\nfill\n");
					ret = print_band(i, len - 256, 1, (inited-1));
					if (!ret)
						return 0;
					else if ((ret & 0xf8) != 0x78)
						inited = 2;
				}
#endif
				 else {
					fprintf(stderr, "\nbreak\n");
					break;
				}
				offset++;
				i++;
			} else if (len > 1){
				if (data[offset+2] == -1){
					data[offset+2] = (i % 2) + 1;
					data64out(data, offset + 1, offset + 1 + len);
					data[offset+2] = -1;
				} else {
					data64out(data, offset + 1, offset + 1 + len);
				}
				offset += len + 1;
			} else {
				data6out(data[offset+1]);
				offset += 2;
			}
		}
	}
	fprintf(stderr, "Done printing page %i.\n", page);
	return 1;
}

int main(int argc, char** argv)
{
	int c;
	int page = 0;
	int reset_only = 0;
	int tfd;
	char gname[20];

	bitmapf = stdin;
	while ((c = getopt(argc,argv,"Rt:l:f:")) != -1){
		switch (c) {
			case 'R': {
				reset_only=1;
				break;
			}
			case 't': {
				sscanf(optarg,"%d",&topskip);
				break;
			}
			case 'l': {
				sscanf(optarg,"%d",&leftskip);
				break;
			}
			case 'f': {
				bitmapf = fopen( optarg,"r" );
				if (!bitmapf) {
					perror("opening file");
					exit(1);
				}
			}
		}
	}
	if (ioperm(DATA, 3, 1)){
		perror("Sorry, you were not able to gain access to the ports\n"
				"You must be root to run this program\n");
		exit(1);
	}
	reset_printer();
	if (reset_only){
		return 0;
	}
	
	/* pages printing loop */
	while (1) {
		/* temporary file to store our results */
		strcpy(gname,"/tmp/lbp660-XXXXXX");
		if ((tfd = mkstemp(gname)) < 0){
			unlink(gname);
			perror("Can't open a temporary file");
			exit(1);
		}
		cbmf = fdopen(tfd,"w+");
		if (!compress_bitmap())
			break;
		if (page > 0){
			sleep(PAGE_DELAY);
            /* I couldn't get it working without reseting, only the first 
             * page would be printed then, or when PAGE_DELAY was less
             * than 18 seconds it would print black.
             * So reset between each page.
             */
			reset_printer();
		}
		if (!print_page(page)){
			fprintf(stderr, "Error, cannot print this page.\n");
			reset_printer();
			exit(1);
		}
		fclose(cbmf);
		next_page(page++);
	}
	fprintf(stderr, "waiting for last page...\n");
	sleep(PAGE_DELAY);
//	reset_printer();
	fclose(bitmapf);
	return 0;
}
