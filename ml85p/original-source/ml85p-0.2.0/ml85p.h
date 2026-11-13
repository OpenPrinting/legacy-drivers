/*
 * Printer driver include file for ML-85G laser printer
 * (C) Copyleft, 2000 Rildo Pragana <rpragana@acm.org>
 *
 * This program is released under GPL. If you don't received a copy of the
 * GPL license, please go find one at http://www.gnu.org.
 */

#define LINE_SIZE		600	
#define PIXELS_BY_ROW  	(LINE_SIZE*8)	
#define LINES_BY_PAGE	6774
//#define LINES_BY_PAGE	6800
#define ROWS_BY_BAND  	20	

/* We must control the device bypassing the kernel driver,
 * because the interface don't follow any standard handshake
 * procedure. In the future, we can write a real device driver
 * to overcome this inconvenience.
 */
#define	LPPORT			0x378

#define COUTLP(d) outb( (d),LPPORT+2 ); usleep(10000)
#define OUTLP(d) outb( (d),LPPORT ); usleep(10000)
#define SINLP() (inb( LPPORT+1 ))

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

/* end of file */

