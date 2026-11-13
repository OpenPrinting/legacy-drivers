/*
 * Project:    CMYK Color Driver for the Lexmark 2070 Color Jetprinter
 *             in 300dpi mode.
 *
 * Author:     Christian Kornblum
 *
 * Version:    0.99, 06.10.1999
 *
 * License:    GPL (GNU Public License)
 */

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* various constants */
#define MAX_LINES 600
#define PAGE_HEIGHT 3520
#define PAGE_WIDTH 2420
#define GS_PAGE_WIDTH 2480
#define GS_PAGE_HEIGHT 3508
#define CARTRIDGE_PENS 104 
#define BLACK_PENS 64
#define COLOR_PENS 32
#define COLOR_GAP 13
#define BYTES_PER_COLUMN 12
#define BYTES_PER_HEADER 26
#define STD_PAGE_MOVE 2*COLOR_PENS
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
void PrintSweep(char *buffer, char *header, int bytesize, int width, FILE *out)
{
  int bstart;
  int leftmargin = LEFT_MARGIN;
  register i;
  /* Remove zeros and set a margin instead. Faster Printing. */
  bstart = ReduceBytes(buffer, BYTES_PER_COLUMN, &leftmargin,
		       &width, &bytesize);
  
  /* Calculate the number of bytes for the checksum */
  bytesize = BYTES_PER_HEADER + BYTES_PER_COLUMN * width; 
  header[6] = (char) bytesize;
  header[5] = (char) (bytesize >> 8);
  header[4] = (char) (bytesize >> 16);

  /* The number of columns */
  header[14] = (char) width;
  header[13] = (char) (width >> 8);

  /* The left margin */
  header[16] = (char) leftmargin;
  header[15] = (char) (leftmargin >> 8);
      
  if (width > 0) { /* do not print empty sweeps */
    for(i=0; i<BYTES_PER_HEADER; i++) putc(header[i], out);
    for(i=0; i<(bytesize);i++) putc(buffer[i+bstart], out);
  }
} /* PrintSweep */	

/*
 * This finds out if there is anything but zeros in a string
 */
int LineSum(char line[], int length)
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
  char line[GS_PAGE_WIDTH / 2];
  int done_page, cur_height = 0, page_height = 0, numpages = 0;
  char lex_blkhd[BYTES_PER_HEADER] = {0x1b,0x2a,0x04,0x00,0x00,0xFF,0xFF,
				      0x00,0x01,0x02,0x01,0x0c,0x31,0xFF,0xFF,
				      0x00,0x30,0x01,0x97,0x00,0x00,0x00,0x00,
				      0x00,0x32,0x33};
  char lex_colhd[BYTES_PER_HEADER] = {0x1b,0x2a,0x04,0x00,0x00,0xFF,0xFF,
				      0x00,0x01,0x01,0x00,0x0c,0x31,0xFF,0xFF,
				      0x00,0x30,0x01,0x97,0x00,0x00,0x00,0x00,
				      0x00,0x32,0x33};
  long bytesize; 
  register int i=0;
  struct tSweepBuffer blkbuffer, colbuffer[COLOR_BUFFERS]; 
  int CurrentColBuffer = 0;
  int width;
  int empty_lines;
  char nibble;
  int cyancounter = 0;

  /* The printer may not be able to print every GhostScript pixel */
  if (GS_PAGE_WIDTH <= PAGE_WIDTH) width = GS_PAGE_WIDTH; 
  else width = PAGE_WIDTH;

  /* Calculating the size for the buffers */
  bytesize = BYTES_PER_COLUMN * width; 

  /* As long as we get input... */
  while((line[0] = getc(in)) != EOF)
  {

    /* Get memory and clear it. */
    SweepBuffer_Init(&blkbuffer, bytesize);
    for (i=0; i<COLOR_BUFFERS; i++) {
      SweepBuffer_Init(&colbuffer[i], bytesize);
      colbuffer[i].bufpos = i;
    }

    /* Initialize the printer, load a page  */
    LexInit(out);

    /* Reset all variables */
    done_page   = 0;
    page_height = 0;
    cur_height  = 0;
    empty_lines = 0;
    cyancounter = 0;

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
	    cyancounter = 0;
	  }

	  /* count lines and set values */
	  /* black is somewhat misaligned in the printer. not my fault.*/
	  blkbuffer.bitpos  = 7 - ((cyancounter % BLACK_PENS) + 4) % 8;
	  blkbuffer.bytepos = 1 + ((cyancounter % BLACK_PENS) + 4) / 8;
	  
	  /* yellow */
	  colbuffer[0].bitpos  = 7 - (cur_height % 8);
	  colbuffer[0].bytepos = 8 + (cur_height / 8) % 4;
	  colbuffer[0].bufpos  = cur_height / COLOR_PENS;
	  
	  /* magenta */
	  colbuffer[1].bitpos  
	    = 7 - ((cur_height + COLOR_GAP + COLOR_PENS) % 8);
	  colbuffer[1].bytepos 
	    = 4 + ((cur_height + COLOR_GAP + COLOR_PENS) / 8) % 4;
	  colbuffer[1].bufpos  
	    = ((cur_height + COLOR_GAP + COLOR_PENS) / COLOR_PENS) % 3;

	  /* cyan has 6 buffers, so that it is not mapped to buffers
	     which have not been printed by yellow yet. The Buffers
	     > 2 are mapped to the right corresponding buffer 
	     after it has been sent to the printer. */
	  colbuffer[2].bitpos  
	    = 7 - ((cur_height + 2 * (COLOR_GAP + COLOR_PENS)) % 8);
	  colbuffer[2].bytepos 
	    = ((cur_height + 2 * (COLOR_GAP + COLOR_PENS)) / 8) % 4;
	  colbuffer[2].bufpos  
	    = ((cur_height + 2 * (COLOR_GAP + COLOR_PENS)) / COLOR_PENS) % 3;
	  if (colbuffer[2].bufpos == colbuffer[0].bufpos)
	    colbuffer[2].bufpos += 3;

	  /* This extracts the nibbles and transforms them to the bits
	     in the output stream. */
	  for(i=0; (i <= width); i++)
	    {                              
	      nibble = (line[i/2] << (4 * (i % 2))) & 0xF0;
	      if (nibble & BLACK) {
		blkbuffer.buffer[(i * BYTES_PER_COLUMN) + blkbuffer.bytepos] 
		  |= 0x01 << blkbuffer.bitpos; 
		blkbuffer.unprinted = 1;
	      }
	      if (nibble & CYAN) {
		colbuffer[colbuffer[2].bufpos].buffer
		  [(i * BYTES_PER_COLUMN) + colbuffer[2].bytepos] 
		  |= 0x01 << colbuffer[2].bitpos; 
		colbuffer[colbuffer[2].bufpos].unprinted = 1;
	      }
	      if (nibble & MAGENTA) {
		colbuffer[colbuffer[1].bufpos].buffer
		  [(i * BYTES_PER_COLUMN) + colbuffer[1].bytepos] 
		  |= 0x01 << colbuffer[1].bitpos; 
		colbuffer[colbuffer[1].bufpos].unprinted = 1;
	      }
	      if (nibble & YELLOW) {
		colbuffer[colbuffer[0].bufpos].buffer
		  [(i * BYTES_PER_COLUMN) + colbuffer[0].bytepos] 
		  |= 0x01 << colbuffer[0].bitpos; 
		colbuffer[colbuffer[1].bufpos].unprinted = 1;
	      }
	    }
	  cur_height++;
	  cyancounter++;
	  /* Buffer is full or page is over. Print it. Color first...*/
	  if (!(cur_height % COLOR_PENS) || (page_height >= GS_PAGE_HEIGHT))
	    {
	      PrintSweep(colbuffer[CurrentColBuffer].buffer, 
			 lex_colhd, bytesize, width, out);
	      ClearBuffer(colbuffer[CurrentColBuffer].buffer, bytesize);
	      LexMove(out, 2*COLOR_PENS);
	      /* now handle the cyan stuff */
	      for(i = 0; i < bytesize; i++) 
		colbuffer[CurrentColBuffer].buffer[i] 
		  |= colbuffer[CurrentColBuffer + 3].buffer[i];
	      ClearBuffer(colbuffer[CurrentColBuffer + 3].buffer, bytesize);
	      colbuffer[CurrentColBuffer].unprinted = 
		colbuffer[CurrentColBuffer + 3].unprinted;
	      colbuffer[CurrentColBuffer + 3].unprinted = 0;
	      /* switch to the next buffer */
	      CurrentColBuffer = ++CurrentColBuffer % 3;
	    }
	  /* ...then finally black */
	  if (!(cyancounter % BLACK_PENS) || (page_height >= GS_PAGE_HEIGHT))
	    {
	      PrintSweep(blkbuffer.buffer, lex_blkhd, bytesize, width, out);
	      ClearBuffer(blkbuffer.buffer, bytesize);
	      blkbuffer.unprinted = 0;
	    }
	  if (cur_height == 3 * COLOR_PENS) cur_height = 0; 
	  if (cyancounter >= 128) cyancounter = 0;
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
	  i < ((GS_PAGE_HEIGHT - PAGE_HEIGHT) * GS_PAGE_WIDTH / 2);
	  line[i++] = getc(in));

    /* count the pages and free memory */
    numpages++;
    free(blkbuffer.buffer);
    for (i=0; i < COLOR_BUFFERS; free(colbuffer[i++].buffer));
  }
  if (numpages == 0) fprintf(stderr, "c2070: No pages printed!");
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
    
