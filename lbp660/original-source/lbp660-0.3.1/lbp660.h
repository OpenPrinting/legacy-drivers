/*
 *  Printer driver for Canon LBP-660 laser printer, header file
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

#define LINE_SIZE		600	
//#define PIXELS_BY_ROW  	(LINE_SIZE*8)	
//#define LINES_BY_PAGE	6760 //6774
#define LINES_BY_PAGE660	6968
#define LINES_BY_PAGE460	3484
#define ROWS_BY_BAND  	104 // number of rows in a band

#define MAX_PACKET_COUNT 3072 // Maximum number of packet in a transfer
//#define MAX_PACKET_COUNT 12000 // Maximum number of packet in a transfer

#define PAGE_DELAY 3000000 //Delay between pages, in usec

/* We must control the device bypassing the kernel driver,
 * because the interface don't follow any standard handshake
 * procedure. In the future, we can write a real device driver
 * to overcome this inconvenience.
 */
#define DATA 0x378
#define STATUS DATA+1
#define CONTROL DATA+2

#ifdef DEBUG
#define INLINE 
#else
#define INLINE inline
#endif

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

