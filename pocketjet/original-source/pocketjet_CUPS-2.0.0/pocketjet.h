/*
 *
 *   PocketJet filter for the Common UNIX Printing System (CUPS),
 *   supporting the Pentax PocketJet line of printers.
 *
 *   Copyright 2004 Tyler Blessing, all rights reserved.
 *
 *                                                                             
 *  This program is freed software; you can redistribute it and/or modify it   
 *  under the terms of the GNU General Public License as published by the Free 
 *  Software Foundation; either version 2 of the License, or (at your option)  
 *  any later version.                                                         
 *                                                                             
 *  Inquiries regarding the use of this copyrighted software under alternate
 *  license terms should be directed to one of the following addresses:                                                                               
 *                                                                             
 *      Software Licensing
 *      PO Box 300645
 *      Austin TX 78703
 *
 *      tylerb@linuxprinting.org
 *                                                                             
 *  This program is distributed in the hope that it will be useful, but        
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License          
 *  along with this program; if not, write to the Free Software                
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// check for autoconf config header
#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include <cups/raster.h>
#include <cups/cups.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

// limits.h for strtol and friends
#include <limits.h>



// constant defines
#ifndef PACKAGE_NAME
    #define PACKAGE_NAME            "rastertopocketjet"
#endif
#ifndef PACKAGE_VERSION
    #define PACKAGE_VERSION          "2.0.0"
#endif

#define PJ_MAX_DENSITY              255
#define PJ_MAX_LINEJUMP             255
#define PJ_300_MAX_LINE_BYTES       308
#define PJ_200_MAX_LINE_BYTES       204
#define PJ_2PLY_MEDIA_TYPE          2
#define PJ_X_COMPRESS_THRESHOLD     11
#define PJ_INITIAL_X_COMPRESS       6
#define PIXEL_BUFFER_SIZE           1024

// MACRO functions for PocketJet printer commands
#define pj_resetPrinter()           printf("\033@")
#define pj_sendFormFeed()           printf("\033~\014")
#define pj_lineJump(count)          printf("\033~J");putchar((count) & 0xFF)
#define pj_setDensity(level)        printf("\033~d");\
                                    putchar((level) & 0xFF);putchar(0)
#define pj_setFormFeed(mode)        printf("\033~f");putchar( mode )
#define pj_enable2PlyMode()         printf("\033~p\001");putchar(0)
#define pj_disable2PlyMode()        printf("\033~p");putchar(0);putchar(0)
#define pj_setPageWidth(bytes)      printf("\033~w");\
                                    putchar( (bytes) & 0xFF );\
                                    putchar(( (bytes) >> 8) & 0xFF )
#define pj_setPageHeight(lines)     printf("\033~h");\
                                    putchar( (lines) & 0xFF );\
                                    putchar(( (lines) >> 8) & 0xFF )
#define pj_setPageLength(lines)     printf("\033~l");\
                                    putchar( (lines) & 0xFF );\
                                    putchar(( (lines) >> 8) & 0xFF )
#define pj_setXPosition(column)     printf("\033~$");\
                                    putchar((column) & 0xF8);\
                                    putchar(((column) >> 8) & 0xFF)
#define pj_receiveDataOfSize(size)  printf("\033~*");\
                                    putchar((size) & 0xFF);\
                                    putchar(((size) >> 8) & 0xFF)

#define reverseBitsInByte(byte)    ((((byte) & 0x01) << 7)|(((byte) & 0x02) << 5)|\
                                    (((byte) & 0x04) << 3)|(((byte) & 0x08) << 1)|\
                                    (((byte) & 0x10) >> 1)|(((byte) & 0x20) >> 3)|\
                                    (((byte) & 0x40) >> 5)|(((byte) & 0x80) >> 7))

// enums
enum {
kPocketJet_FormFeedMode_NoFeed      = 0,    //CUPS_ADVANCE_NONE
kPocketJet_FormFeedMode_FixedPage   = 1,    //CUPS_ADVANCE_FILE
kPocketJet_FormFeedMode_EndOfPage   = 2,    //CUPS_ADVANCE_JOB

kPPDUnitValue                       = 72,   // 72 points per inch
};

// function prototypes
void beginNewPage(cups_page_header_t *	header);
void processRasterLine(cups_page_header_t * header, int row);

