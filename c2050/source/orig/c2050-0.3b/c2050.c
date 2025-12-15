/*
 * Project:    CMYK Color Driver for the Lexmark 2050 Color Jetprinter
 *             in 300dpi mode.
 *
 * Author:     Marco Nenciarini
 * 
 * Version:    0.3b, 11.06.2007
 *
 * License:    GPL (GNU Public License)
 * 
 * This software is based on c2070 driver written by
 * Christian Kornblum .
 * 
 */

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* head alignment (values must be >= 0)*/
#define COL_O_OFFSET 4
#define BLK_O_OFFSET 0
#define COL_V_OFFSET 0
#define BLK_V_OFFSET 4

/* various constants */
#define MAX_LINES 600
#define PAGE_WIDTH 2420
#define PAGE_HEIGHT 3408
#define GS_PAGE_WIDTH 2480
#define GS_PAGE_HEIGHT 3507
#define BLACK_PENS 48
#define COLOR_PENS 16
#define COLOR_GAP 4
#define MAGENTA_OFFSET 16
#define BYTES_PER_COLUMN 6
#define BYTES_PER_HEADER 26
#define LEFT_MARGIN 10
#define UPPER_MARGIN 100
#define COLOR_BUFFERS 6
/* the ghostscript color identifiers */
#define BLACK   0x10
#define CYAN    0x80
#define MAGENTA 0x40
#define YELLOW  0x20

/* the structure for the pixmaps */
struct tSweepBuffer {
  int bytepos;
  int bitpos;
  int bufpos;        /* this is used for the colors! */
  int unprinted;     /* does this buffer contain data? */
  char *buffer;
};

/*
 * This writes a number of zeros to a string.
 */
void ClearBuffer(char *data, int bytes)
{
  register i;
  for(i = 0; i < bytes; data[i++] = 0);
} /* ClearBuffer */

/* 
 * Initialize a sweep buffer
 */
SweepBuffer_Init (struct tSweepBuffer *SweepBuffer, int bytesize)
{
  SweepBuffer->bytepos = 0;
  SweepBuffer->bitpos = 0;
  SweepBuffer->bufpos = 0;
  SweepBuffer->unprinted = 0;
  SweepBuffer->buffer = (char *) malloc(bytesize);
  ClearBuffer(SweepBuffer->buffer, bytesize);
} /* SweepBuffer_Init */

/*
 * This puts an unterminated amount of any chars to "out". The first 
 * byte of the "string" has to give the correct number of the following bytes.
 */
void fPutLString (FILE *out, char *data) {
  int i;
  for (i = 1; i <= data[0]; putc(data[i++], out));
} /* fPutLString */

/*
 * This moves the paper by a defined number of lines (600lpi!).
 */
void LexMove(FILE *out, long int pixel)
{
  char command[] = {5,0x1b,0x2a,0x03,0x00,0x00};
  command[5] = (char) pixel;
  command[4] = (char) (pixel >> 8);
  fPutLString(out, command);
} /* LexMove */

/*
 * This initializes the printer and sets the upper margin.
 */
void LexInit(FILE *out)
{
   char command[] = {12, 0x1B,0x2A,0x80,0x1B,0x2A,0x07,
		         0x73,0x30,0x1B,0x2A,0x07,0x63};
   fPutLString(out, command);
   LexMove(out, UPPER_MARGIN);
} /* LexInit */

/*
 * This tells the printer to throw out his current page.
 */
void LexEOP(FILE *out)
{
   char command[] = {4, 0x1B,0x2A,0x07,0x65};
   fPutLString(out, command);
}

/*
 * This confusing bit of code removes empty columns from the printbuffer.
 * It returns the byte of the buffer where the important data starts and
 * changes all referencered arguments accordingly.
 */
int ReduceBytes(char *buffer, int bytespercolumn, 
		int *leftmargin, int *breite, int *bytesize) {
  register int redleft = 0; 
  register int redright = 0; 
  int bstart = 0;
  while ((buffer[redleft] == 0) && (redleft < *bytesize)) redleft++;
  while ((buffer[*bytesize - 1 - redright] == 0) && 
	 (redright < *bytesize)) redright++;
  *breite -= redleft / bytespercolumn + redright / bytespercolumn;
  *leftmargin += redleft / bytespercolumn;
  bstart = redleft - (redleft % bytespercolumn);
  if (bstart < 0) bstart = 0;
  
  return bstart;
} /* ReduceBytes */

/*
 * This sends a complete sweep to the printer. Black or color, no difference.
 */
void PrintSweep(char *buffer, char *header, int bytesize, int width, int leftmargin, FILE *out)
{
  int bstart;
  register i;
  /* Remove zeros and set a margin instead. Faster Printing. */
  bstart = ReduceBytes(buffer, BYTES_PER_COLUMN, &leftmargin,
		       &width, &bytesize);
  
  /* Calculate the number of bytes for the checksum */
  bytesize = BYTES_PER_HEADER + BYTES_PER_COLUMN * width; 
  header[4] = (char) bytesize;
  header[3] = (char) (bytesize >> 8);

  /* The number of columns */
  header[12] = (char) width;
  header[11] = (char) (width >> 8);

  /* The left margin */
  header[14] = (char) leftmargin;
  header[13] = (char) (leftmargin >> 8);

  if (width > 0) { /* do not print empty sweeps */
    for(i=0; i<BYTES_PER_HEADER; i++) putc(header[i], out);
    for(i=0; i<(bytesize);i++) putc(buffer[i+bstart], out);
  }
} /* PrintSweep */	

/*
 * This finds out if there is anything but zeros in a string
 */
int LineSum(signed char line[], int length)
{
  register i = 0;
  while (i < length)
    if(line[i++] != 0) return 1;
  return 0;
} /* LineSum */

/*
 * This is the main printing routine. Wicked and insane. Nonetheless working.
 */
void LexPrint(FILE *in, FILE *out) {
  signed char line[GS_PAGE_WIDTH / 2];
  int done_page, cur_height = 0, page_height = 0, numpages = 0;
  char lex_blkhd[BYTES_PER_HEADER] = {0x1b,0x2a,0x04,0xff,0xff,0x00,0x01,
				      0x00,0x01,0x06,0x31,0xff,0xff,0xff,0xff,
				      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,
				      0x33,0x34,0x35};
  char lex_colhd[BYTES_PER_HEADER] = {0x1b,0x2a,0x04,0xff,0xff,0x00,0x01,
				      0x00,0x00,0x06,0x31,0xff,0xff,0xff,0xff,
				      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,
				      0x33,0x34,0x35};
  long blkbytesize, colbytesize; 
  register int i=0;
  struct tSweepBuffer blkbuffer, colbuffer[COLOR_BUFFERS]; 
  int CurrentColBuffer = 0;
  int blkwidth, colwidth;
  int empty_lines, skipcolors;
  signed char nibble;
  int yellowcounter = 0;

  /* The printer may not be able to print every GhostScript pixel */
  if (GS_PAGE_WIDTH <= PAGE_WIDTH) blkwidth = GS_PAGE_WIDTH; 
  else blkwidth = PAGE_WIDTH;

  colwidth = blkwidth + MAGENTA_OFFSET; 
  
  /* Calculating the size for the buffers */
  blkbytesize = BYTES_PER_COLUMN * blkwidth; 
  colbytesize = BYTES_PER_COLUMN * colwidth; 

  /* As long as we get input... */
  while((line[0] = getc(in)) != EOF)
  {

    /* Get memory and clear it. */
    SweepBuffer_Init(&blkbuffer, blkbytesize);
    for (i=0; i<COLOR_BUFFERS; i++) {
      SweepBuffer_Init(&colbuffer[i], colbytesize);
      colbuffer[i].bufpos = i;
    }

    /* Initialize the printer, load a page  */
    LexInit(out);

    /* Reset all variables */
    done_page        = 0;
    page_height      = 0;
    cur_height       = COL_V_OFFSET;
    empty_lines      = 0;
    skipcolors       = 0;
    yellowcounter    = BLK_V_OFFSET;
    CurrentColBuffer = 0;

    /* ... we do the pages. */
    while(!done_page)
    {
 
      /* Read a CMYK line (GS -sDEVICE=bitcmyk) from the input */
      if (page_height == 0) {
	for (i = 1; i < (GS_PAGE_WIDTH / 2); line[i++] = getc(in));
      } else {
	for (i = 0; i < (GS_PAGE_WIDTH / 2); line[i++] = getc(in));
      }
      
      /* optimize for empty lines, if buffers are empty */
      if ((cur_height == 0) 
	  && !LineSum(line, GS_PAGE_WIDTH / 2)
	  && (page_height < PAGE_HEIGHT)
	  && (page_height < GS_PAGE_HEIGHT)
	  && !(blkbuffer.unprinted | colbuffer[0].unprinted 
	       | colbuffer[1].unprinted | colbuffer[3].unprinted))
	{
	  empty_lines++;
	}
      else /* This line does not seem to be empty or there is still data */
	{
	  if (empty_lines) {
	    LexMove(out, empty_lines * 2);
	    empty_lines = 0;
	    yellowcounter = BLK_V_OFFSET;
	  }

	  /* count lines and set values */
	  blkbuffer.bitpos  = 
	     (yellowcounter % BLACK_PENS) % 8;
	  blkbuffer.bytepos = 5 - (yellowcounter % BLACK_PENS) / 8;
	  
	  /* cyan */
	  colbuffer[0].bitpos  = (cur_height % 8);
	  colbuffer[0].bytepos = 1 - (cur_height / 8) % 2;
	  colbuffer[0].bufpos  = cur_height / COLOR_PENS;
	  
	  /* magenta */
	  colbuffer[1].bitpos  
	    = ((cur_height + COLOR_GAP + COLOR_PENS) % 8);
	  colbuffer[1].bytepos 
	    = (MAGENTA_OFFSET * BYTES_PER_COLUMN) + 3 - ((cur_height + COLOR_GAP + COLOR_PENS) / 8) % 2;
	  colbuffer[1].bufpos  
	    = ((cur_height + COLOR_GAP + COLOR_PENS) / COLOR_PENS) % 3;

	  /* yellow has 6 buffers, so that it is not mapped to buffers
	     which have not been printed by cyan yet. The Buffers
	     > 2 are mapped to the right corresponding buffer 
	     after it has been sent to the printer. */
	  colbuffer[2].bitpos  
	    = ((cur_height + 2 * (COLOR_GAP + COLOR_PENS)) % 8);
	  colbuffer[2].bytepos 
	    = 5 - ((cur_height + 2 * (COLOR_GAP + COLOR_PENS)) / 8) % 2;
	  colbuffer[2].bufpos
	    = ((cur_height + 2 * (COLOR_GAP + COLOR_PENS)) / COLOR_PENS) % 3;
	  if (colbuffer[2].bufpos == colbuffer[0].bufpos)
	    colbuffer[2].bufpos += 3;

	  /* This extracts the nibbles and transforms them to the bits
	     in the output stream. */
	  for(i=0; (i <= blkwidth); i++)
	    {                              
	      nibble = (line[i/2] << (4 * (i % 2))) & 0xF0;
	      if (nibble & BLACK) {
		blkbuffer.buffer[(i * BYTES_PER_COLUMN) + blkbuffer.bytepos] 
		  |= 0x01 << blkbuffer.bitpos; 
		blkbuffer.unprinted = 1;
	      }
	      if (nibble & CYAN) {
		colbuffer[colbuffer[0].bufpos].buffer
		  [(i * BYTES_PER_COLUMN) + colbuffer[0].bytepos] 
		  |= 0x01 << colbuffer[0].bitpos; 
		colbuffer[colbuffer[0].bufpos].unprinted = 1;
	      }
	      if (nibble & MAGENTA) {
		colbuffer[colbuffer[1].bufpos].buffer
		  [(i * BYTES_PER_COLUMN) + colbuffer[1].bytepos] 
		  |= 0x01 << colbuffer[1].bitpos; 
		colbuffer[colbuffer[1].bufpos].unprinted = 1;
	      }
	      if (nibble & YELLOW) {
		colbuffer[colbuffer[2].bufpos].buffer
		  [(i * BYTES_PER_COLUMN) + colbuffer[2].bytepos] 
		  |= 0x01 << colbuffer[2].bitpos; 
		colbuffer[colbuffer[2].bufpos].unprinted = 1;
	      }
	    }
	  cur_height++;
	  yellowcounter++;
	   /* Buffer is full or page is over. Print it. Black first...*/
	  if (!(yellowcounter % BLACK_PENS) || (page_height >= PAGE_HEIGHT))
	    {
	      if (skipcolors) {
		 LexMove(out, 2*COLOR_PENS*skipcolors);
		 skipcolors=0;
	      }
	      PrintSweep(blkbuffer.buffer, lex_blkhd, blkbytesize, blkwidth, LEFT_MARGIN + BLK_O_OFFSET, out);
	      ClearBuffer(blkbuffer.buffer, blkbytesize);
	      blkbuffer.unprinted = 0;
	    }
	  /* ...then finally colors */
	  if (!(cur_height % COLOR_PENS) || (page_height >= PAGE_HEIGHT))
	    {
	      if (colbuffer[CurrentColBuffer].unprinted) {
  	         if (skipcolors) {
		    LexMove(out, 2*COLOR_PENS*skipcolors);
		    skipcolors=0;
		 }
  	         PrintSweep(colbuffer[CurrentColBuffer].buffer, 
			    lex_colhd, colbytesize, colwidth, LEFT_MARGIN + COL_O_OFFSET, out);
	         ClearBuffer(colbuffer[CurrentColBuffer].buffer, colbytesize);
	         LexMove(out, 2*COLOR_PENS);
	      }
	      else
		 skipcolors++;
	      /* now handle the yellow stuff */
	      for(i = 0; i < colbytesize; i++) 
		colbuffer[CurrentColBuffer].buffer[i] 
		  |= colbuffer[CurrentColBuffer + 3].buffer[i];
	      ClearBuffer(colbuffer[CurrentColBuffer + 3].buffer, colbytesize);
	      colbuffer[CurrentColBuffer].unprinted = 
		colbuffer[CurrentColBuffer + 3].unprinted;
	      colbuffer[CurrentColBuffer + 3].unprinted = 0;
	      /* switch to the next buffer */
	      CurrentColBuffer = ++CurrentColBuffer % 3;
	    }
	  if (cur_height == 3 * COLOR_PENS) cur_height = 0; 
	  if (!(yellowcounter % BLACK_PENS) && !(yellowcounter % COLOR_PENS)) yellowcounter = 0;
	}
      
      /* this page has found an end */ 
      if ((page_height++ >= PAGE_HEIGHT)||
	  (page_height >= GS_PAGE_HEIGHT)) done_page = 1; 
    }

    /* hand out the page */
    LexEOP(out);

    /* eat any remaining whitespace so process will not hang */
    if (PAGE_HEIGHT < GS_PAGE_HEIGHT) 
      for(i=0;
	  (i < ((GS_PAGE_HEIGHT - PAGE_HEIGHT) * GS_PAGE_WIDTH / 2)) && (nibble != EOF);
          i++)
	 nibble = getc(in);

    /* count the pages and free memory */
    numpages++;
    free(blkbuffer.buffer);
    for (i=0; i < COLOR_BUFFERS; free(colbuffer[i++].buffer));
  }
  if (numpages == 0) fprintf(stderr, "c2050: No pages printed!");
} /* LexPrint */

/*
 * The main program. Sets input and output streams.
 */
int main(int argc, char *argv[]) {
  FILE *InputFile;
  FILE *OutPutFile;
  
  InputFile  = stdin;
  OutPutFile = stdout;

  LexPrint(InputFile, OutPutFile);

  fclose(OutPutFile);
  fclose(InputFile);
}
    
