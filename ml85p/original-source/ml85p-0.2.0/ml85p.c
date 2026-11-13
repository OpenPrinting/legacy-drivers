/*
 * Printer driver for ML-85G laser printer
 * (C) Copyleft, 2000 Rildo Pragana <rpragana@acm.org>
 *
 * This program is released under GPL. If you don't received a copy of the
 * GPL license, please go find one at http://www.gnu.org.
 */

#include <stdio.h>
#include <unistd.h>
#include <asm/io.h>
#include <asm/system.h>
#include <time.h>
#include "ml85p.h"

int cmd_seq[] = { 
0x80, 1, 0xa0, 1, 0x00, 0, 0xa0, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8b, 1, 0x8b, 1, -2,
	0x89, 0, 0x8c, 1, 0x8c, 1, 0x04, 0, 0x94, 1, -2, 
	0x3f, 0, 0x95, 1, 0x58, 0, 0x94, 1, 0x95, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8b, 1, 0x8b, 1, 0x9a, 0, -2,
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8b, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x46, 0, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x4f, 0, -2,
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1,  -2,
//-1 };

/* ASIC registers: 85,86 - control width of printing area (little endian)
                   87,88 - control height of printing area
*/
//int cmd_seq2[] = {
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8b, 1, 0x8b, 1, 0x9d, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8b, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, 0x97, 1, 0x00, 0, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, 0xa0, 1, 
	0x00, 0, 0xa0, 1, 0x81, 1, 0x2a, /*0x15,*/ 0, 0x82, 1,
	0x00, 0, 0x83, 1, 0x00, 0, 0x84, 1, 0x00, 0,
/* registers 85,86,87,88 select letter or a4 paper
	letter = 600x6360 pixels; a4 = 584x6774 pixels.
	I prefer to use 600x6774 (a4 wider) all the time.

	dimensoes:
	0x1a76 == 6774 ; 0x0258 == 600 ; 0x0248 = 584
*/
	0x85, 1, 0x58, 0, 0x86, 1, 0x02, 0, 
	0x87, 1, /*0x90*/ 0x76, 0, 0x88, 1, 0x1a, 0, 
	0x81, 1, 0x82, 1, 0x83, 1, 0x84, 1,
	0x85, 1, 0x86, 1, 0x87, 1, 0x88, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, -2,
//0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, 0x93, 1, 0x00, 0, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x62, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x62, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x49, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x54, 0, 
-1 }; 

int cmd_page[] = {
/*0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x01, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x02, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x08, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x40, 0, 
*/
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, -2,
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, -2,
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, -2,
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, 0x97, 1, 0x00, 0, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, -2,
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, 0x93, 1, 0x00, 0, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 
-1};

int cmd_fill[] = {
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x01, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x02, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x08, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x40, 0, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8e, 1, -2, 
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 
-1};

int cmd_econom[] = {
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, 0x8d, 1, 0x45, 0, 
-1 };

int cmd_short_pause[] = {
0x89, 1, 0x8a, 1, 0xa6, 1, 0x07, 0, 0xa7, 1, -2,
-1 };

unsigned char parity[] = {
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};

FILE *bitmapf,*cbmf;
int bmcnt=0;
unsigned char bmbuf[800]; 		/* the pbm bitmap line 
							with provision for leftskip */
unsigned char *bmptr=bmbuf;
int bmwidth=0, bmheight=0;
//unsigned char cbm[83200];	/* the compressed bitmap */
unsigned char cbm[100000];	/* the compressed bitmap */
unsigned char garbage[600];
unsigned char *cbmp=cbm;
int csize=0;				/* compressed line size */
static int last_ctrl=0x06;
int linecnt=0;
int pktcnt;
//int topskip=150;
int topskip=0;
//int leftskip=120;
int leftskip=0;
int reset_on_timeout=0;
int need_reset=0;

void
sdelay() {
	__sti();
	usleep(5000);
	__cli();
}

void
putmsg (char *msg) {
	__sti();
	printf("%s\n",msg);
	__cli();
}

int 
sinp () {
	return (inb(LPPORT+1));
}

void 
sinpwfast (wait) {
	time_t ct;
	__sti();
	ct = time(NULL);
	while ((inb(LPPORT+1)&0xf7) != (wait&0xf7)) {
		if (reset_on_timeout && (time(NULL) > (ct+5))) {
			need_reset=1;
			break;
		}
	}
	__cli();
}

int 
sinpw (wait) {
	//int timeout=80000;
	__sti();
	//printf("sinpw: %x\n",wait);

	while (inb(LPPORT+1) != wait) ;
	/*{
		if (timeout-- == 0) {
			__sti();
			iopl(0);
			exit(1);
		}
	}*/
	__cli();
}

void
outlp( data ) {
	//printf("O0,%x\n",data);
	outb(data,LPPORT);
}

void
coutlp( data ) {
	//printf("O2,%x\n",data);
	outb(data,LPPORT+2);
}

void
toggle_control (times) {
	int i;
	__sti();
	for (i=0;i<times;i++) {
		last_ctrl = last_ctrl ^ 2;
		coutlp( last_ctrl );
	}
	__cli();
	usleep(5000);
}

void
outstb () {
	int st;
	while(1) {
		st = inb(LPPORT+1);
		if ((st&0x78)==0x38) break;
	}
	st = inb(LPPORT+2) & 0x0f | 1;
	outb(st,LPPORT+2);
	outb(st,LPPORT+2);
	st &= 0x0c;
	outb(st,LPPORT+2);
}

void
lpout ( data ) {
	outlp(data);
	outstb();
}

void
lpout1 ( data ) {
	//putmsg( "lpout1" );
	outlp(data);
	sdelay();
	coutlp( 6 );
	sinpw(0xbf) ;
	coutlp( 7 );
	coutlp( 7 );
	coutlp( 4 );
	sinpw(0xdf) ;
	coutlp( 6 );
	sinpw(0xff) ;
}

void lpctrl1 () {
	int *s;
	int i;
	//putmsg( "lpctrl1" );
	coutlp(2);
	coutlp(6);
	sinpw(0xff) ;
	coutlp(4);
	coutlp(0xc);
	sinpw(0x2f) ;
	coutlp(0xe);
	sinpw(0x5f) ;
	coutlp(0xc);
	sdelay();
	outlp(0x20);
	sdelay();
	//lpout1(0x20);
	coutlp(0x6);
	sinpw(0xbf) ;
	coutlp( 7 );
	coutlp( 7 );
	coutlp( 4 );
	sinpw(0xdf) ;
	coutlp( 6 );
	sinpw(0xff) ;
	__sti();
	for (i=0;i<30;i++) {
		//printf("O0,0 ");
		outlp(0);
		usleep(1500);
	}
	__cli();
}

void
lpoutw ( int data,int type ) {
	int mask=0;
	char s[100];
	//sprintf(s,"lpoutw(%x)",data);
	//putmsg(s);
	//sdelay();
	outlp(data);
	//sdelay();

	if (type) {
		mask=2;
	}
	coutlp( 4+mask );
	coutlp( 5+mask );
	sinpwfast(0x7f);
	if (need_reset) 
		return;
	coutlp( 4+mask );
	//sinpw(0xff);
	toggle_control(17);
	//toggle_control(3);
}

void
send_cmd ( int *cmd ) {
	int cmd_code,cmd_type;
	while (1) {
		cmd_code = *cmd++;
		if (cmd_code >= 0) {
			cmd_type = *cmd++;
		}
		if (cmd_code == -1) {
			break;
		}
		if (cmd_code == -2) {
			__sti();
			usleep(5000);
			__cli();
			continue;
		}
		lpoutw(cmd_code,cmd_type);
		if (need_reset)
			return;
	}
}

void
reset_printer () {
	int *cmd;
	/* phase 0 */
	coutlp(8);
	usleep(1000);
	coutlp(6);
	usleep(1000);

	/* phase 1 */
	lpout1(0x24);
	last_ctrl=2;
	toggle_control(51);

	/* phase 2 */
	//putmsg("phase 2");
	lpctrl1();

	/* phase 3 */
	//putmsg("phase 3");
	send_cmd(cmd_seq);
}

int
print_band ( int band, int size ) {
	int i;
	char msg[50];
	unsigned char *buf;
	int group;
	//time_t ct;
	sprintf(msg,"band: %d",band);
	putmsg(msg);
	buf = cbm;
	__sti();
	coutlp(6);
	while ((inb(LPPORT+1)&0xf0) != 0xf0) ;
	outlp(0x80);
	coutlp(6);
	coutlp(7);
	__sti();
	while (inb(LPPORT+1)&0x80) ;
	__cli();
	//sinpwfast(0x7f);
	coutlp(6);
	sinpwfast(0xff);
	outlp(0xff);
	coutlp(4);
	coutlp(5);
	coutlp(5);
	__sti();
	while (inb(LPPORT+1)&0x80) ;
	__cli();
	for (i=0;i<size;i++) {
		outb(*buf++,LPPORT);
		outb(*buf++,LPPORT);
		outb(*buf++,LPPORT);
		outb(*buf++,LPPORT);
	}
	coutlp(4);
/*	__sti();
	while (!(inb(LPPORT+1)&0x80)) ;
	__cli();
*/
	sinpwfast(0xff);
	return 1;
}

void
set_modo_econom() {
	send_cmd(cmd_econom);
}

void
wait_page_ready () {
	__sti();
	while (sinp()&0x8) { 
		fprintf(stderr,"waiting: %x\n",sinp());
		__cli();
		toggle_control(8);
		send_cmd(cmd_short_pause);
		__sti();
	}
}

void
print_page ( int page ) {
	int band=0,size;
	int *cmd;
	int i,cmd_code,cmd_type;
	wait_page_ready();
	__sti();
	fprintf(stderr,"waiting: %x\n",sinp());
	__cli();
	send_cmd(cmd_page);
	__sti();
	while (!feof(cbmf)) {
		if (!fread( &size,1,sizeof(int),cbmf ))
			break;
		fread( cbm,size*4,1,cbmf );
		if (!print_band( band++,size )) {
			/* timeout occured, reset printer and continue */
			reset_printer();
			fseek(cbmf,0,SEEK_SET);
			wait_page_ready();
			band=0;
		}
	}
	if (band < 25) {
		__sti();
		for (i=0;i<4;i++) {
			send_cmd(cmd_fill);
		}
		if (band < 10) {
			for (i=0;i<6;i++) {
				send_cmd(cmd_fill);
			}
		}
	}
}

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
	printf("bmheight = %d, topskip = %d, linecnt = %d, skip = %d\n", 
		bmheight, topskip, linecnt, skip);
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
	
	if (rle==2) { /* flush packet storage */
		fwrite( &csize,1,sizeof(int),cbmf );
		fwrite( cbm,csize,4,cbmf );
		cbmp = cbm;
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
}

int
compress_bitmap () {
	int line,band;
	unsigned char c1,c2,c3;
	int cnt;					/* count of characters processed in a line */
	int	pcnt;					/* count of chars for each packet */

	if (fgets(cbm,200,bitmapf)<=0) {
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
	
	/* now we process the real data */
	for	( band=0 ; linecnt<LINES_BY_PAGE ; band++ ) {
		pktcnt=0;
		/* setup packet size */
		if (((LINES_BY_PAGE-linecnt)>20)) {
			cnt=LINE_SIZE*10;  
		}
		else {
			cnt = LINE_SIZE * (LINES_BY_PAGE-linecnt);
		}
		fprintf(stderr,"cnt: %d, band: %d, linecnt: %d\n",cnt,band,linecnt);
		c1 = get_bitmap();
		c2 = get_bitmap();
		pcnt=1;
		while (cnt) {
			if ((c1==c2) && (cnt>2)) {
				pcnt++;
				c2 = get_bitmap();
				cnt--;
				/* add more packets if there is room to */
				if ((cnt < LINE_SIZE) &&
					((pktcnt + pcnt/256) < 5000) &&
					(linecnt<LINES_BY_PAGE)) {
						cnt += LINE_SIZE;
				}
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
				if ((cnt < LINE_SIZE) &&
					((pktcnt + pcnt/256) < 5000) &&
					(linecnt<LINES_BY_PAGE)) {
						cnt += LINE_SIZE;
				}
		}
		out_packet(2,0,0,0);
	}
	fflush(cbmf);
	fseek(cbmf,0,SEEK_SET);
	return 1;
}

int
main ( int argc, char *argv[] ) {
	int c,i;
	int reset_flag=1;
	char gname[20];
	int size;
	int simulate=0;
	int page=0;
	int modo_econom=0;
	int reset_only=0;
	int tfd;

	bitmapf = stdin;
	while ((c = getopt(argc,argv,"ERt:l:srf:")) != -1) {
		switch (c) {
			case 'R': {
				reset_only=1;
				break;
			}
			case 'E': {
				reset_only=1;
				modo_econom=1;
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
			case 's': {
				simulate++;
				break;
			}
			case 'r': {
				reset_flag = 1 - reset_flag; /* toggle it */
				break;
			}
			case 'f': {
				bitmapf = fopen( optarg,"r" );
				if (!bitmapf) {
					fprintf(stderr,"File not found on unreadable\n");
					exit(1);
				}
			}
		}
	}
	//ioperm( LPPORT,3,1 );
	iopl(3);
	__cli();
	if (reset_flag && !simulate)
		reset_printer();

	if (!reset_only) {
		/* probe for already reseted printer */
		/*if (!simulate) {
			send_cmd(cmd_short_pause);
			if (need_reset) {
				need_reset=0;
				reset_printer();
			}
			reset_on_timeout=0;
		}*/

		/* pages printing loop */
		while (1) {
			/* temporary file to store our results */
			strcpy(gname,"/tmp/ml85-XXXXXX");
			if ((tfd=mkstemp(gname))<0) {
				fprintf(stderr,"Can't open a temporary file.\n");
				exit(1);
			}
			cbmf = fdopen(tfd,"w+");
			if (!compress_bitmap())
				break;
			if (!simulate) {
				print_page( page  );
			}
			fclose(cbmf);
			next_page(page++);
		}
	}
	/* the following is to avoid leaving in a busy-printing state
	   so the next command may timeout and reset the printer */
/*	if (!simulate) {
		send_cmd(cmd_short_pause);
	}*/

	if (modo_econom)
		set_modo_econom();
	//ioperm( LPPORT,3,1 );
	__sti();
	iopl(0);
	
	fclose(bitmapf);
	return 0;
}

/* end of file */
