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
 *
 */

#include <pocketjet.h>

// global variables

//unsigned gDeviceModelNumber;    // the model differentiator
unsigned gMaxLineBytes;         // the hardware-specific maximum bytes per line 
int gPrintDensity;       // the output density setting
int gFormFeedMode;         // the form feed mode setting
int gNoFeedExtraLength;       // the roll feed mode eject length
int gMirrorPrint;       // the roll feed mode eject length
int gNegativePrint;       // the roll feed mode eject length
int gCCMode;       // the roll feed mode eject length

int gPage;                      // the current page number
int gLinesToSkip;               // the number of blank lines to skip
int gLastXCursor;               // the most recent argument to the ESC~$ command

unsigned char gPixelBuffer[PIXEL_BUFFER_SIZE];
unsigned char gPrintableData[PIXEL_BUFFER_SIZE];

int main (int argc, const char * argv[])
{
// for testing purposes only
//    if (3 < argc && (0==strcmp(argv[3], "xCode-test")))
//        freopen("/Users/Shared/xout","w",stdout);
    
    
    int			fd=-1;           // the input file descriptor
    cups_raster_t *     raster;         // a CUPS raster struct
    cups_page_header_t	header;         // the CUPS page header from the raster stream
    int			row;            // the current raster row
    unsigned            rowFraction;    // a scaling factor for the current row position
//    ppd_file_t *	ppd;              // the CUPS PPD struct

    cups_option_t       *options=NULL;	// CUPS print job options
    int			num_options;    // the number of CUPS options
    const char          *value;         // option value

    // unset buffering for stderr so that status messages will work properly
    setbuf(stderr, NULL);


    // process the command line argments
    switch(argc)
    {
    case 6:
        fd = 0;
        break;

    case 7:
        if ((fd = open(argv[6], O_RDONLY)) == -1)
        {
            fprintf(stderr, "ERROR: Error opening the print file!\n");
            fprintf(stderr, "DEBUG: Unable to open the raster file \"%s\" - ", argv[6]);
            perror("");
            sleep(1);
            return (1);
        }
        break;

    case 2:
        if (strcmp(argv[1], "--version") == 0)
        {
            // print version info

            fprintf(stdout, "%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);

            return 0;
        }
        // fall through for all other arguments
    default:
        fprintf(stderr, "USAGE: %s job-id user title copies options [file]\n", PACKAGE_NAME);
        return (1);
        break;
    }

    // parse the command line PPD options
    num_options = cupsParseOptions(argv[5], 0, &options);
    value=cupsGetOption("Density",num_options, options);
    if (value)
    {
        gPrintDensity = (unsigned)strtoul(value, (char **)NULL, 10) * 25;
    }
    value=cupsGetOption("FormFeedMode",num_options, options);
    if (value)
    {
        if (0 == strcasecmp(value, "EndOfPage"))
            gFormFeedMode = kPocketJet_FormFeedMode_EndOfPage;
        else if (0 == strcasecmp(value, "NoFeed"))
            gFormFeedMode = kPocketJet_FormFeedMode_NoFeed;
        else
            gFormFeedMode = kPocketJet_FormFeedMode_FixedPage;
    }
    else
        gFormFeedMode = kPocketJet_FormFeedMode_FixedPage;

    value=cupsGetOption("ExtraFeed",num_options, options);
    if (value)
    {
        gNoFeedExtraLength = (int)strtol(value, (char **)NULL, 10);
    }
    value=cupsGetOption("MirrorPrint",num_options, options);
    if (value)
    {
        if (0 == strcasecmp(value, "true"))
            gMirrorPrint = 1;
        else
            gMirrorPrint = 0;
    }
    value=cupsGetOption("NegativePrint",num_options, options);
    if (value)
    {
        if (0 == strcasecmp(value, "true"))
            gNegativePrint = 1;
        else
            gNegativePrint = 0;
    }
    value=cupsGetOption("CCMode",num_options, options);
    if (value)
    {
        if (0 == strcasecmp(value, "true"))
            gCCMode = 1;
        else
            gCCMode = 0;
    }

    // open the raster stream
    raster = cupsRasterOpen(fd, CUPS_RASTER_READ);

    // reset the printer for this job
    pj_resetPrinter();

    // reset the page counter
    gPage = 0;

    // process the job data, page by page

    while (cupsRasterReadHeader(raster, &header))
    {
        // fill in some of the missing header info for jaguar,
        // which fails to properly populate the raster header
        // header.AdvanceMedia should never be 0 in Panther or later
        if (0 == header.AdvanceMedia)
        {
		header.MediaWeight=gPrintDensity;
		header.MirrorPrint=gMirrorPrint;
		header.NegativePrint=gNegativePrint;
//		header.AdvanceMedia=gFormFeedMode;
//		header.AdvanceDistance=gNoFeedExtraLength;
//		header.cupsMediaType=gCCMode;
        }
        // increment the page counter
        gPage ++;
        // reset the line-skip counter to zero
        gLinesToSkip = 0;
        gLastXCursor = -1;

        // send a CUPS PAGE status message
        fprintf(stderr, "PAGE: %d %d\n", gPage, header.NumCopies);

        // initialize the printer for this page
        beginNewPage(&header);

        // get the current rowFraction for this page;
        rowFraction = header.cupsHeight / 9;

        // loop over the raster rows in this page
        for (row = 0; row < header.cupsHeight; row ++)
        {
            // output a status message
            if (0 == (row % rowFraction) && row)
            fprintf(stderr, "INFO: Printing page %d, %d%% ...\n", gPage, 100 * row / header.cupsHeight);

            // Read a row of raster data and process it
            if (cupsRasterReadPixels(raster, gPixelBuffer, header.cupsBytesPerLine) < 1)
                break;

            processRasterLine(&header, row);
        }
        
        // now we're finished processing this page ...

        // work around a documented printer firmware bug
        pj_receiveDataOfSize(0);
        // and eject the page
        pj_sendFormFeed();

        // remember to flush it all out of the buffer
        fflush(stdout);
    }
    // Now we are done with this job
    
    // check the formfeed mode and send any "extra" amount for "no feed" mode
    if (kPocketJet_FormFeedMode_NoFeed == gFormFeedMode && 0 < gNoFeedExtraLength)
    {
        // the number of rows to advance is sent in the header as PostScript points
        gLinesToSkip =  gNoFeedExtraLength * header.HWResolution[1];
        
        // skip the specified number of lines, remembering that
        // we can skip only one byte's worth at a time ...
        while(PJ_MAX_LINEJUMP < gLinesToSkip)
        {
            pj_lineJump(PJ_MAX_LINEJUMP);
            gLinesToSkip -= PJ_MAX_LINEJUMP;
        }

        pj_lineJump(gLinesToSkip);
        
        // and send one more formfeed for the extra feed
        pj_sendFormFeed();

        fflush(stdout);
    }

    // free any CUPS options
    if (NULL != options)
        cupsFreeOptions(num_options, options);
	options = NULL;

    // Close the CUPS raster stream
    cupsRasterClose(raster);
    if (fd != 0)
        close(fd);

    // send a status message and return
    if (gPage == 0)
    {
        fprintf(stderr, "ERROR: No printable data!\n");
        return (1);
    }

    fprintf(stderr, "INFO: Ready to print.\n");
    return (0);
}
void beginNewPage(cups_page_header_t *	header)
{
    // print a pocketjet `header' for this page, consisting of
    // print density, formfeed mode, page width in bytes, and page length in pixel rows

    // get the print density from the PPD and limit it to the valid range
    // the valid range is 1 to PJ_MAX_DENSITY
    int printDensity = header->MediaWeight;
    if (0 >= printDensity || PJ_MAX_DENSITY < printDensity)
        printDensity = PJ_MAX_DENSITY / 2;
        
    // check if we have valid data in the header
    if (0 < header->AdvanceMedia)
    {
        // get the advance distance from the header
        gNoFeedExtraLength = header->AdvanceDistance/kPPDUnitValue;
        
        switch (header->AdvanceMedia)
        {
        case 3:
            gFormFeedMode = kPocketJet_FormFeedMode_NoFeed;
            break;
        case 2:
            gFormFeedMode = kPocketJet_FormFeedMode_EndOfPage;
            break;
        case 1:
        default:
            gFormFeedMode = kPocketJet_FormFeedMode_FixedPage;
            break;
        }
    }

    // set the maximum number of bytes per row for this hardware
    // the 300 dpi PocketJet has a larger line buffer than the 200 dpi pocketjet
    // In all cases, the maximum bytes per line must not exceed the hardware limits
    // or bad things happen. To be safe we default to the lower limit for unknown
    // horizontal resoultions.
    switch(header->HWResolution[0])
    {
    case 300:
        gMaxLineBytes = PJ_300_MAX_LINE_BYTES;
        break;
    default:
    case 200:
    case 203:
        gMaxLineBytes = PJ_200_MAX_LINE_BYTES;
        break;
    }

    // now write out the header for this page ...
    
    // set the density          (ESC~d##)
    pj_setDensity(printDensity);

    // set the 2-ply (also called carbon-copy) mode  (ESC~p##)
    if (1==gCCMode || PJ_2PLY_MEDIA_TYPE == header->cupsMediaType)
    {
        pj_enable2PlyMode();
    }
    else
    {
        pj_disable2PlyMode();
    }
    
    // set the page eject mode  (ESC~f#)
    pj_setFormFeed(gFormFeedMode);

    // set the page width       (ESC~w##)
    pj_setPageWidth(header->cupsBytesPerLine);
    
    // set the page height      (ESC~h##) ... 3200 for letter 4100 for legal 3300 for A4
    // set the page length      (ESC~l##) ... for all other cases
    switch(header->cupsHeight)
    {
    case 2200:
    case 2133:
    case 2733:
    case 3200:
    case 3300:
    case 4100:
        pj_setPageHeight(header->cupsHeight);
        break;
    default:
        pj_setPageLength(header->cupsHeight);
        break;
    }
}
void processRasterLine(cups_page_header_t * header, int row)
{
    // check the raster line for printable data and write it out ...
    
    // the strategy here is to read each raster line and process the data,
    // which means either sending it "raw" or "compressing" it. Because the
    // bulk of savings from "compression" is derived from skipping blank lines
    // we necessarily need to "buffer" the compression product rather than
    // output each line as we read it.
    
    // so, read a raster line and look for non-zero data. If we get any then we
    // can't skip the line so we can go ahead and print it out. If the line is blank
    // then we add it to the blank-line `bank' and read the next line. As soon as we
    // encounter a non-blank line we first print out the command to skip the number
    // of lines in the blank `bank', zero the bank, then process the line as usual.
    
    // we also compress in the horizontal direction by skipping over columns of 
    // blank pixels `ESC~$0x0x'
    // the optimum number of consecutive NULL bytes to skip is easy to calculate since
    // the commands to `set cursor position' and `begin data' require a fixed
    // number of bytes  : 1b 7e 24 xx xx 1b 7e 2a xx xx
    // so if there are 10 NULL bytes we break even. If there are 11 NULL bytes or more
    // then we are better off compressing. Also, we have to be aware that the `Set
    // Cursor Postion' command is persistent and new lines will start at the most
    // recently set value. This means that we typically need to issue this command
    // before the first non-NULL byte on a line.
    
    int x;                      // the current x index (column position) on this row
    int maxIndex;               // the maximum index for this row (typically (rowBytes -1))
    int xp=0;                   // the x-index into the printable data buffer for this row
    int lastX= -1;              // the most recent x index with printable data
    unsigned char currentByte;  // the current "byte of interest" in this row
    unsigned char mirroredOutput = header->MirrorPrint;
    unsigned char negativeOutput = header->NegativePrint;

    maxIndex = (gMaxLineBytes > header->cupsBytesPerLine ? header->cupsBytesPerLine : gMaxLineBytes) -1;


    // process each byte's worth of pixels in this row
    for(x=0; maxIndex >= x && sizeof(gPixelBuffer) > x; x++)
    {
        // get the current byte, adjusting the index for mirrored output if need be.
        if(mirroredOutput)
            currentByte = gPixelBuffer[maxIndex - x];
        else
            currentByte = gPixelBuffer[x];
                    
        // flip the bits for inverted (negative) output
        if(negativeOutput)
            currentByte ^= 0xFF;

        // look for non-NULL pixel bytes
        if(currentByte)
        {
            // check if this byte is the first non-zero for this line
            if (0 > lastX)
            {
                // check for any buffered blank lines
                if (0 < gLinesToSkip)
                {
                    // skip the specified number of lines, remembering that
                    // we can skip only one byte's worth at a time ...
                    while(PJ_MAX_LINEJUMP < gLinesToSkip)
                    {
                        pj_lineJump(PJ_MAX_LINEJUMP);
                        gLinesToSkip -= PJ_MAX_LINEJUMP;
                    }
                    // now skip any remaining lines
                    if (0 < gLinesToSkip)
                    {
                        pj_lineJump(gLinesToSkip);
                    }
                    // flush the blank lines to the printer ...
                    fflush(stdout);
                    // and reset the blank-line "bank" to zero
                    gLinesToSkip = 0;
                }
                
                // check if we need to set the x position cursor for this line
                // The printer will continue to use the most recent set position
                // We can use that value if it is close to the current value
                // otherwise we need to set the new position
                if ((x >= gLastXCursor) && (PJ_INITIAL_X_COMPRESS > x - gLastXCursor))
                {
                    lastX = gLastXCursor -1;
                }
                else
                {
                    pj_setXPosition(x * 8);
                    lastX = x - 1;
                    gLastXCursor = x;
                }
            }
            // check if we should "compress" the x data by setting the cursor position.
            // only do "compression" over PJ_X_COMPRESS_THRESHOLD bytes or more,
            // and never compress the very top row (to work around a firmware bug)
            if ( PJ_X_COMPRESS_THRESHOLD < x - lastX && 0 < row )
            {
                // write out any accumulated data before we send a "setCursorPosition" command
                if (0 < xp)
                {
                    pj_receiveDataOfSize(xp);
                    while (fwrite(gPrintableData, sizeof(unsigned char), xp, stdout) != xp)
                        if (errno != EAGAIN || errno != EINTR)
                        {
                            fprintf(stderr,"failed trying to write a raster line to stdout");
                            break;
                        }
                    // reset xp (the printable data index) to zero after writing the data
                    xp = 0;
                }


                // the printer hardware will set the cursor postion only to
                // full byte increments, but it requires the position in pixel columns
                pj_setXPosition(x * 8);
                
                // update the record holders to reflect the new cursor start position
                lastX = x - 1;
                gLastXCursor = x;
            }
            
            // accumulate any remaining NULL data that wasn't "compressed"
            for (; 1 < (x - lastX) && sizeof(gPrintableData) > xp;lastX++)
                gPrintableData[xp++] = 0;
                
            // copy the printable data byte
            if (sizeof(gPrintableData) > xp)
            {
                // reverse the byte bits for mirrored output
                if(mirroredOutput)
                {
                    gPrintableData[xp++] = reverseBitsInByte(currentByte);
                }
                else
                    gPrintableData[xp++] = currentByte;

                lastX = x;
            }
            else
            {
                fprintf(stderr,"DEBUG: attempted overflow. gPrintableData is too small (%lu bytes)\n", sizeof(gPrintableData));
                return;
            }
        }
    }
    
    // done with this raster line ... check if it contained printable data
    // print out the data or just skip empty lines
    if (0 <= lastX)
    {
        // write out any remaining printable data ...
        if (0 < xp)
        {
            pj_receiveDataOfSize(xp);
            while (fwrite(gPrintableData, sizeof(unsigned char), xp, stdout) != xp)
                if (errno != EAGAIN || errno != EINTR)
                {
                    fprintf(stderr,"failed trying to write a raster line to stdout");
                    break;
                }
        }

        // ... and send a "line feed" to flush the printer buffer.
        // remembering that "jumping one line" also flushes (and is a
        // slight optimization) we just fall through to the "no data" case.
    }

    // if no data then skip empty lines
        gLinesToSkip ++;
}
