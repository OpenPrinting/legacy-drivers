/*
 * $Header: /cvsroot/m2300w/m2300w/src/m2300w.c,v 1.3 2005/10/01 22:06:56 gfuer Exp $
 *
 * m2300w converts the pbmraw or pksmraw output of ghostscript
 * (www.ghostscript.com) into the format of the Minolta Magicolor
 * 2300W Winprinter
 *
 * Copyright 2004 Leif Birkenfeld (leif@birkenfeld-home.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* ----------------------------------------------------------------------
    other Variables
    andere Variablen
   ----------------------------------------------------------------------*/

FILE *in_stream, *out_stream;
/* FILE *cStream, *mStream, *yStream, *kStream; */

int verb = 0;			/* verbose level */
int MediaCode = 0;
int PaperCode = 4;
int ResXmul = 1;
int ResYmul = 1;
int colorMode = 0;
int thisSiteColorMode = 0;

int saveToner = 0;
int useUCR = 0;

int linesPerBlock;
unsigned char paperFormat;
unsigned char paperQuality;
unsigned short blocksPerPage;
unsigned short thisSiteBlocksPerPage;
unsigned int resBreite;
unsigned int resHoehe;

int jobHeaderWritten = 0;
int headerCount = 0;		/* zaehler fuer alle header */
int siteInitHeaderCount = 0;	/* zaehler fuer alle header */
int reservedHeaderCountSH =0;   /* reservierter HeaderCount fuer den Seitenheader da dieser erst spaeter verwendet wird */

long pix[4][3] = {		/* pixel, ucr , tonerSave */
    {0, 0, 0},			/* cyan	*/
    {0, 0, 0},			/* magenta */
    {0, 0, 0},			/* yellow */
    {0, 0, 0},			/* schwarz */
};

long cPix = 0;
long cPixSaved = 0;
long mPix = 0;
long mPixSaved = 0;
long yPix = 0;
long yPixSaved = 0;
long kPix = 0;
long kPixSaved = 0;


/* ----------------------------------------------------------
    Formateinstellungen
-------------------------------------------------------------- */

struct format
{
    char desc[32];
    int resX;
    int resY;
    int mediaWidth;
    int mediaHeight;
    int mediaMustBe;
};

struct format form[42] = {
/* 0*/ {"no data\0", 0, 0, 0, 0, -1},
/* 1*/ {"no data\0", 0, 0, 0, 0, -1},
/* 2*/ {"no data\0", 0, 0, 0, 0, -1},
/* 3*/ {"no data\0", 0, 0, 0, 0, -1},
/* 4*/ {"A4\0", 4752, 6808, 0, 0, -1},
/* 5*/ {"no data\0", 0, 0, 0, 0, -1},
/* 6*/ {"B5 JIS\0", 4088, 5856, 0, 0, -1},
/* 7*/ {"no data\0", 0, 0, 0, 0, -1},
/* 8*/ {"A5\0", 3288, 4752, 0, 0, -1},
/* 9*/ {"no data\0", 0, 0, 0, 0, -1},
/* a*/ {"no data\0", 0, 0, 0, 0, -1},
/* b*/ {"no data\0", 0, 0, 0, 0, -1},
/* c*/ {"no data\0", 0, 0, 0, 0, -1},
/* d*/ {"no data\0", 0, 0, 0, 0, -1},
/* e*/ {"no data\0", 0, 0, 0, 0, -1},
/* f*/ {"no data\0", 0, 0, 0, 0, -1},
/*10*/ {"no data\0", 0, 0, 0, 0, -1},
/*11*/ {"no data\0", 0, 0, 0, 0, -1},
/*12*/ {"Folio\0", 4752, 7584, 0, 0, -1},
/*13*/ {"no data\0", 0, 0, 0, 0, -1},
/*14*/ {"no data\0", 0, 0, 0, 0, -1},
/*15*/ {"no data\0", 0, 0, 0, 0, -1},
/*16*/ {"no data\0", 0, 0, 0, 0, -1},
/*17*/ {"no data\0", 0, 0, 0, 0, -1},
/*18*/ {"no data\0", 0, 0, 0, 0, -1},
/*19*/ {"Legal\0", 4896, 8136, 0, 0, -1},
/*1a*/ {"Government Legal\0", 4896, 7592, 0, 0, -1},
/*1b*/ {"Letter\0", 4896, 6392, 0, 0, -1},
/*1c*/ {"no data\0", 0, 0, 0, 0, -1},
/*1d*/ {"no data\0", 0, 0, 0, 0, -1},
/*1e*/ {"no data\0", 0, 0, 0, 0, -1},
/*1f*/ {"Executive\0", 6096, 4144, 0, 0, -1},
/*20*/ {"no data\0", 0, 0, 0, 0, -1},
/*21*/ {"Statement\0", 3096, 4896, 0, 0, -1},
/*22*/ {"no data\0", 0, 0, 0, 0, -1},
/*23*/ {"no data\0", 0, 0, 0, 0, -1},
/*24*/ {"Kuvert Monarch\0", 2120, 4296, 0, 0, 3},
/*25*/ {"Kuver #10\0", 2272, 5496, 0, 0, 3},
/*26*/ {"Kuvert DL\0", 2392, 4992, 0, 0, 3},
/*27*/ {"Kuvert C5\0", 3616, 5200, 0, 0, 3},
/*28*/ {"Kuvert C6\0", 2480, 3616, 0, 0, 3},
/*29*/ {"B5 ISO\0", 3944, 5696, 0, 0, 3},
};

/* noch nicht eingebaute

31;Letter Plus;4896;7408;216;322;8;926;
31;UK Quadro;4592;5792;203;254;8;724;
31;Druckerpapier;4592;7592;203;330;8;949;

c0;Japan Postkarte;2152;3288;0;0;8;411;
*/



struct media
{
    char desc[32];
};

struct media med[7] = {
/* 0*/ {"Normal (HQ)"},
/* 1*/ {"Karton"},
/* 2*/ {"Folie"},
/* 3*/ {"Kuvert"},
/* 4*/ {"Briefkopf"},
/* 5*/ {"Postkarte"},
/* 6*/ {"Etikette"},
};


/* ----------------------------------------------------------------------------

    Headerstrukturen

   ---------------------------------------------------------------------------*/

unsigned char fileHeader[] = { 0x1B, 0x40, 0x00, 0x02, 0x00, 0xBF, 0x82, 0x10, 0xAE };

 /* char jobHeader[] ={0x1B,0x50,0x01,0x08,0x00,0xAF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x24}; */

struct
{
    unsigned char jobHeaderT1[6];
    unsigned char res1;
    unsigned char res2;
    unsigned char jobHeaderT2[6];
    unsigned char prSum;
}
jobHeaderStc =
{
    { 0x1B, 0x50, 0x01, 0x08, 0x00, 0xAF},
    0x01, 0x00,
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    0x00
};

unsigned char *jobHeader = (unsigned char *)&jobHeaderStc;



struct
{
    unsigned char seitenHeaderT1[2];
    unsigned char headerCount;
    unsigned char seitenHeaderT2[3];
    unsigned char colorMode;
    unsigned char seitenHeaderT2b[3];
    unsigned char breite1;
    unsigned char breite2;
    unsigned char seitenHeaderT3[2];
    unsigned char hoehe1;
    unsigned char hoehe2;
    unsigned char blocksPerPage1;
    unsigned char seitenHeaderT4[1];
    unsigned char blocksPerPage2;
    unsigned char seitenHeaderT5[2];
    unsigned char paperFormat;
    unsigned char seitenHeaderT6[6];
    unsigned char paperQuality;
    unsigned char seitenHeaderT7[5];
    unsigned char prSum;
}
seitenHeaderStc =
{
    { 0x1B, 0x51} ,
    0x02,
    { 0x1C, 0x00, 0xAE},
    0x00,
    { 0x01, 0x00, 0x00},
    0x00, 0x00,
    { 0x00, 0x00},
    0x00, 0x00, 0x08,
    { 0x00},
    0x08,
    { 0x00, 0x00},
    0x04,
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    0x00,
    { 0x00, 0x01, 0x00, 0x00, 0x00},
    0x00
};

unsigned char *seitenHeader = (unsigned char *) &seitenHeaderStc;

struct
{
    unsigned char jobFooterT1[2];
    unsigned char headerCount;
    unsigned char jobFooterT2[4];
    unsigned char prSum;
}
jobFooterStc =
{
    { 0x1B, 0x55},
    0x02,
    { 0x01, 0x00, 0xAA, 0x00},
    0x00
};

unsigned char *jobFooter = (unsigned char *) &jobFooterStc;

struct
{
    unsigned char fileFooterT1[2];
    unsigned char headerCount;
    unsigned char fileFooterT2[4];
    unsigned char prSum;
}
fileFooterStc =
{
    { 0x1B, 0x41},
    0x00,
    { 0x01, 0x00, 0xBE, 0x00},
    0x00
};

unsigned char *fileFooter = (unsigned char *) &fileFooterStc;

struct
{
    unsigned char blockHeaderT1[2];
    unsigned char headerCount;
    /* headerzaehler 0x?? */
    unsigned char blockHeaderT2[3];
    unsigned char blockLength1;
    unsigned char blockLength2;
    unsigned char blockLength3;
    unsigned char blockLength4;
    /* laenge des blocks in byte 0x??,0x??,0x??,0x?? */
    unsigned char tonerColor;
    unsigned char blockCount;
    /* bblockzaehler 0x?? */
    unsigned char linesPerBlock1;
    unsigned char linesPerBlock2;
    unsigned char prSum;
    /* pruefsumme 0x?? */
}
blockHeaderStc =
{
    { 0x1B, 0x52},
    0x03,
    { 0x08, 0x00, 0xAD},
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x53, 0x03, 0x00
};

unsigned char *blockHeader = (unsigned char *) &blockHeaderStc;

/* ---------------------------------------------------------------------------*/

struct steuerFelder
{
    unsigned int bytesIn;
    unsigned int linesOut;
    unsigned int blocksOut;

    unsigned char *encBuffer;
    unsigned long indexEncBuffer;

    unsigned int rleCount;
    unsigned short lastByte;

    unsigned char *blockBuffer;
    unsigned long indexBlockBuffer;

    unsigned char *pageOut;
    unsigned long indexPageOut;

};


struct steuerFelder stFeld[4] = {
    {0, 0, 0, NULL, 0, 0, 0, NULL, 0, NULL, 0},
    {0, 0, 0, NULL, 0, 0, 0, NULL, 0, NULL, 0},
    {0, 0, 0, NULL, 0, 0, 0, NULL, 0, NULL, 0},
    {0, 0, 0, NULL, 0, 0, 0, NULL, 0, NULL, 0}
};



/* ----------------------------------------------------------------------

    Procedure Devision ;-)

   ----------------------------------------------------------------------*/


void
Help (void)
{
    int ho;

    fprintf (stderr,
	     "\nm2300w Version 0.1_1 (2004-05-19), Copyright (C) 2004  Leif Birkenfeld\n"
	     "This Software comes with ABSOLUTELY NO WARRANTY\n"
	     "This is free software, and you are welcome to redistribute it under certain conditions.\n"
	     "See COPYING file for further information !\n\n"
	     "Usage:           foo22300w -i xxxxx.pbm -o xxxxx.prn -c x\n"
	     "\n"
	     "Options:\n"
	     "-i inFile         Filename to read from \n"
	     "                        Use \"-\" to read from stdin\n");
    fprintf (stderr,
	     "-o outfile        Filename to write into file\n"
	     "                        Use \"-\" to write to stdout\n"
	     "                        (Using stdout sets -v 0)\n"
	     "-c mode           B/W or Color Mode [no default]\n"
	     "                        1 - Black and White\n"
	     "                        2 - Color\n"
	     "-m media          Media code:  [%d]\n", MediaCode);
    for (ho = 0; ho < 6; ho++) {
	fprintf (stderr,
		 "                       %2d - %s\n", ho, med[ho].desc);
    }
    fprintf (stderr, "-p paper          Paper code:  [%d]\n", PaperCode);

    for (ho = 0; ho < 42; ho++) {
	if (form[ho].resX != 0) {
	    fprintf (stderr,
		     "                        %2d - %s\n", ho, form[ho].desc);
	}
    }

    fprintf (stderr,
	     "-r x              Resolution mode: [%d]\n"
	     "                        1 -  600x600 dpi\n"
	     "                        2 - 1200x600 dpi\n"
	     "-s                Save Toner\n"
	     "                        Discards every second pixel in a\n",
	     ResXmul);
    fprintf (stderr,
	     "                        chequerboard order to save 50 percent toner\n"
	     "                        It works fine for text and graphic, but\n"
	     "                        it dosn't for photos.\n"
	     "-v x              Verbose level [%d]\n"
	     "                        Please write as first Option \n"
	     "                        More than 5 will be extrem !\n",
	     verb);

    exit (1);


}

void
writeOut (FILE * ziel, void *vquelle, long length)
{
    int oByte;
    char *quelle = vquelle;

    for (oByte = 0; oByte < length; oByte++) {
	fputc (quelle[oByte], ziel);
    }
}

void
encodeToBlockBuffer (int colorID)
{
    int debu;
    long rohBytes = stFeld[colorID].indexEncBuffer - stFeld[colorID].rleCount;
    int rohByteCount = 0;
    unsigned char rleOut[2];
    int rle64, rle1;

    if (verb > 5)
	fprintf (stderr,
		 "--> Ausgabe von %.3i Bytes fuer colorID %i davon %.3i Rohbytes und %.3i mal %.2x RLE encoded am Ende.\n",
		 (int) stFeld[colorID].indexEncBuffer, (int) colorID,
		 (int) rohBytes, (int) stFeld[colorID].rleCount,
		 stFeld[colorID].lastByte);
    if (verb > 5) {
	int debu;
	fprintf (stderr, "Daten fuer Encoder:\n");
	for (debu = 0; debu < stFeld[colorID].indexEncBuffer; debu++) {
	    fprintf (stderr, " %.2x", stFeld[colorID].encBuffer[debu]);
	}
	fprintf (stderr, "\n");
    }
    /* rohbytes verarbeiten */
    /* 64er stufe */
    while ((rohBytes - (rohByteCount * 64)) >= 64) {
	unsigned char rBOut[65];
	if (verb > 5)
	    fprintf (stderr, "Segment mit 64 Bytes ausgeben ...");
	/* header belegen (standardwert) */
	rBOut[0] = (char) (64 - 1);
	memcpy (&rBOut[1], &stFeld[colorID].encBuffer[(rohByteCount * 64)],
		64);
	memcpy (&stFeld[colorID].
		blockBuffer[stFeld[colorID].indexBlockBuffer], &rBOut[0], 65);
	stFeld[colorID].indexBlockBuffer += 65;
	rohByteCount++;
	if (verb > 5)
	    fprintf (stderr, "OK\n");

	if (verb > 5) {
	    fprintf (stderr, "Rohbytes:\n");
	    for (debu = 0; debu < (65); debu++) {
		fprintf (stderr, " %.2x", rBOut[debu]);
	    }
	    fprintf (stderr, "\n");
	}


    }
    if ((rohBytes - (rohByteCount * 64)) > 0) {
	/* XXX */
	/* unsigned char rBOut[rohBytes - (64 * rohByteCount) + 1]; */
	unsigned char *rBOut = malloc(rohBytes - (64 * rohByteCount) + 1);

	/* einzelstufe */
	if (verb > 5)
	    fprintf (stderr, "Segment mit %i Bytes ausgeben ...",
		     (int) (rohBytes - (64 * rohByteCount)));

	/* header belegen */
	rBOut[0] = (char) ((rohBytes - (64 * rohByteCount)) - 1);
	memcpy (&rBOut[1], &stFeld[colorID].encBuffer[(rohByteCount * 64)],
		(rohBytes - (64 * rohByteCount)));
	memcpy (&stFeld[colorID].
		blockBuffer[stFeld[colorID].indexBlockBuffer], &rBOut[0],
		(rohBytes + 1 - (64 * rohByteCount)));
	stFeld[colorID].indexBlockBuffer +=
	    (rohBytes - (64 * rohByteCount) + 1);
	if (verb > 5)
	    fprintf (stderr, "OK\n");

	if (verb > 5) {
	    fprintf (stderr, "Rohbytes:\n");
	    for (debu = 0; debu < (rohBytes - (64 * rohByteCount)); debu++) {
		fprintf (stderr, " %.2x", rBOut[debu]);
	    }
	    fprintf (stderr, "\n");
	}

	/* XXX */
	free(rBOut);
    }

    /* rle verarbeiten */

    rle64 = floor ((double) (stFeld[colorID].rleCount) / 64);
    rle1 = (stFeld[colorID].rleCount) - (rle64 * 64);


    if (rle64 > 0) {
	rleOut[0] = 192 + rle64;
	rleOut[1] = stFeld[colorID].lastByte;
	memcpy (&stFeld[colorID].
		blockBuffer[stFeld[colorID].indexBlockBuffer], &rleOut[0], 2);
	stFeld[colorID].indexBlockBuffer += 2;

	if (verb > 5)
	    fprintf (stderr,
		     "---->64er RLE Encoding: %i mal %.2x - codiert als: %.2x%.2x\n",
		     rle64, rleOut[1], rleOut[0], rleOut[1]);
	if (verb > 5) {
	    fprintf (stderr, "64er RLE::\n");
	    for (debu = 0; debu < (2); debu++) {
		fprintf (stderr, " %.2x", rleOut[debu]);
	    }
	    fprintf (stderr, "\n");
	}



    }
    if (rle1 > 0) {
	rleOut[0] = 128 + rle1;
	rleOut[1] = stFeld[colorID].lastByte;
	memcpy (&stFeld[colorID].
		blockBuffer[stFeld[colorID].indexBlockBuffer], &rleOut[0], 2);
	stFeld[colorID].indexBlockBuffer += 2;
	if (verb > 5)
	    fprintf (stderr,
		     "---->1er RLE Encoding: %i mal %.2x - codiert als: %.2x%.2x\n",
		     rle1, rleOut[1], rleOut[0], rleOut[1]);

	if (verb > 5) {
	    fprintf (stderr, " 1er RLE::\n");
	    for (debu = 0; debu < (2); debu++) {
		fprintf (stderr, " %.2x", rleOut[debu]);
	    }
	    fprintf (stderr, "\n");
	}

    }


    if (verb > 5)
	fprintf (stderr, "--->RLE Encode for %i done.\n", colorID);
    stFeld[colorID].indexEncBuffer = 0;
}

void
doEncode (int inByte, int colorID)
{
    if (stFeld[colorID].bytesIn == 0) {
	unsigned char dummyTable[1] = { 0x80 };
	stFeld[colorID].linesOut++;
	/* jede Zeile beginnt mit einer Tabelle */
	/* die tabellenkompression wurde vorerst weggelassen deshalb eine leere tabelle */
	if (verb > 5)
	    fprintf (stderr, "Dummy Tabelle fuer neue Zeile %3i ausgeben .\n",
		     stFeld[colorID].linesOut);
	memcpy (&stFeld[colorID].
		blockBuffer[stFeld[colorID].indexBlockBuffer], &dummyTable[0],
		1);
	stFeld[colorID].indexBlockBuffer += 1;

    }
    if (saveToner > 0) {
	/* spar Toner indem es jedes 2te bit loescht. */
	/* die loeschung erfolgt pro zeile versetzt (schachbrettmuster) */
	if (stFeld[colorID].linesOut % 2 > 0) {
	    if (verb > 0) {
		/* zaehlen der gesparten pixel */
		pix[colorID][2] +=
		    ((inByte) & 0x01) + ((inByte >> 2) & 0x01) +
		    ((inByte >> 4) & 0x01) + ((inByte >> 6) & 0x01);
	    }
	    inByte = inByte & 0xaa;
	}
	else {
	    if (verb > 0) {
		pix[colorID][2] +=
		    ((inByte >> 1) & 0x01) + ((inByte >> 3) & 0x01) +
		    ((inByte >> 5) & 0x01) + ((inByte >> 7) & 0x01);
	    }
	    inByte = inByte & 0x55;
	}
    }

    if (inByte == stFeld[colorID].lastByte || stFeld[colorID].rleCount < 1) {
	stFeld[colorID].rleCount++;	/* zaehlen wie oft ein byte wiederholt wird */
    }
    else {
	if (stFeld[colorID].rleCount > 3) {	/* rle ausgabe lohnt sich, deshalb ausgeben */
	    encodeToBlockBuffer (colorID);
	    /* wenn sich das ausgeben nicht lohnt, werden die aufgelaufenen rles verworfen und als normale bytes gewertet */
	}
	stFeld[colorID].rleCount = 0;
    }
    stFeld[colorID].lastByte = inByte;

    stFeld[colorID].bytesIn++;	/* EingabeByte hochzaehlen und byyte auf den buffer legen */
    stFeld[colorID].encBuffer[stFeld[colorID].indexEncBuffer] = inByte;
    stFeld[colorID].indexEncBuffer++;


    if ((stFeld[colorID].bytesIn + 1) > (resBreite / 8)) {
	/* eine zeile ist voll. den rest im buffer codieren */

	encodeToBlockBuffer (colorID);
	stFeld[colorID].bytesIn = 0;
	stFeld[colorID].rleCount = 0;
	/* wenn die anzahl der zeilen f|r einen block erreicht ist muss der blockheader generiert werden */
	/* dies kann erst jetzt geschehen, da die anzahl der im block befindlichen bytes im header steht */
	if (stFeld[colorID].linesOut >= linesPerBlock) {
	    int psr;
	    stFeld[colorID].blocksOut++;
	    if (verb > 4)
		fprintf (stderr,
			 "Blockheader fuer Block %i mit %5i Bytes generieren und Block raus kopieren...\n",
			 (int) stFeld[colorID].blocksOut,
			 (int) stFeld[colorID].indexBlockBuffer);
	    blockHeaderStc.blockLength1 =
		(char) stFeld[colorID].indexBlockBuffer;
	    blockHeaderStc.blockLength2 =
		(char) (stFeld[colorID].indexBlockBuffer >> 8);
	    blockHeaderStc.blockLength3 =
		(char) (stFeld[colorID].indexBlockBuffer >> 16);
	    blockHeaderStc.blockLength4 =
		(char) (stFeld[colorID].indexBlockBuffer >> 24);
	    blockHeaderStc.linesPerBlock1 = (char) (linesPerBlock);
	    blockHeaderStc.linesPerBlock2 = (char) (linesPerBlock >> 8);
	    /* die berechnung der headerCount muss sicherstellen das die reichenfolge stimmt */
	    if (thisSiteColorMode == 0xf0) {
		blockHeaderStc.headerCount =
		    siteInitHeaderCount + stFeld[colorID].blocksOut - 1 +
		    ((3 - colorID) * 8);
	    }
	    else {
		blockHeaderStc.headerCount =
		    siteInitHeaderCount + stFeld[colorID].blocksOut - 1;
	    }
	    headerCount++;
	    if (verb > 4)
		fprintf (stderr,
			 "BlockHeader.headerCount fuer colorID %i ist %i\n",
			 colorID, blockHeaderStc.headerCount);
	    blockHeaderStc.blockCount = stFeld[colorID].blocksOut;
	    blockHeaderStc.tonerColor = colorID;

	    /* pruefsumme berechnen */
	    blockHeaderStc.prSum = 0;
	    for (psr = 0; psr < 14; psr++) {
		blockHeaderStc.prSum += blockHeader[psr];

	    }
	    memcpy (&stFeld[colorID].pageOut[stFeld[colorID].indexPageOut],
		    blockHeader, 15);
	    stFeld[colorID].indexPageOut += 15;

	    if (verb > 4) {
		int vR;
		fprintf (stderr, "Blockheader: ");
		for (vR = 0; vR < 15; vR++) {
		    fprintf (stderr, " %.2x", blockHeader[vR]);
		}
		fprintf (stderr, "\n");
	    }


	    memcpy (&stFeld[colorID].pageOut[stFeld[colorID].indexPageOut],
		    &stFeld[colorID].blockBuffer[0],
		    stFeld[colorID].indexBlockBuffer);
	    stFeld[colorID].indexPageOut += stFeld[colorID].indexBlockBuffer;

	    stFeld[colorID].linesOut = 0;
	    stFeld[colorID].indexBlockBuffer = 0;
	}

    }


}

void
clearBuffer (int i)
{
/* initialisiert die stucktur[i] */
    stFeld[i].bytesIn = 0;
    stFeld[i].linesOut = 0;
    stFeld[i].blocksOut = 0;
    stFeld[i].indexEncBuffer = 0;
    stFeld[i].rleCount = 0;
    stFeld[i].lastByte = 0;
    stFeld[i].indexBlockBuffer = 0;
    stFeld[i].indexPageOut = 0;
}

void
writeJobHeader (void)
{
    long psr;

    writeOut (out_stream, &fileHeader[0], 9);
    headerCount++;

    for (psr = 0; psr < 14; psr++) {
	jobHeaderStc.prSum += jobHeader[psr];
    }

    writeOut (out_stream, &jobHeader[0], 15);
    headerCount++;
    if (verb > 1)
	fprintf (stderr, "JobHeader written.\n");
}

void
writeSiteHeader (void)
{
    long psr;

    /* seitenHeader ausgeben */
    seitenHeaderStc.headerCount = reservedHeaderCountSH;
    seitenHeaderStc.breite1 = (unsigned char) resBreite;
    seitenHeaderStc.breite2 = (unsigned char) (resBreite >> 8);
    seitenHeaderStc.hoehe1 = (unsigned char) resHoehe;
    seitenHeaderStc.hoehe2 = (unsigned char) (resHoehe >> 8);
    seitenHeaderStc.colorMode = thisSiteColorMode;
    seitenHeaderStc.blocksPerPage1 = thisSiteBlocksPerPage;
    seitenHeaderStc.blocksPerPage2 = thisSiteBlocksPerPage;

    seitenHeaderStc.paperFormat = PaperCode;
    seitenHeaderStc.paperQuality = MediaCode;

    /* pruefsumme berechnen */
    seitenHeaderStc.prSum = 0x00;
    for (psr = 0; psr < 34; psr++) {
	seitenHeaderStc.prSum += seitenHeader[psr];
    }
    writeOut (out_stream, &seitenHeader[0], 35);

    if (verb > 4) {
	int vR;
	fprintf (stderr, "Seitenheader: ");
	for (vR = 0; vR < 35; vR++) {
	    fprintf (stderr, " %.2x", seitenHeader[vR]);
	}
	fprintf (stderr, "\n");
    }

}


void
readPkmraw (void)
{

    int c = 0;


    int ccs = 0;

    /* die reichenfolge der farben in der eingabedatei ist verkehrt */
    /* die ausgabe erfolgt in der reihenfolge 3,2,1,0 */
    /* 3 - gelb , 2 - magenta , 1 - cyan , 0 - black */
    unsigned short colorKey[4] = { 1, 2, 3, 0 };

    int readHeader;
    char buffer[255];


    while ((fgets (buffer, 256, in_stream)) != NULL) {
	int inpX = 0;
	int inpY = 0;
	int inpXBytes = 0;
	unsigned char lastByteMask = 0;

	int shz, sbz;
	int realLines, realPixBytes;
	int leadLines, leadPixBytes;
	int trailLines, trailPixBytes;
	int xAdd, yAdd;

	if (verb > 2) {
	    fprintf (stderr, "gelesener Header: %s", buffer);
	}

	/* grobe prueffung - geht eigentlich besser */
	if (buffer[0] != 'P' || buffer[1] != '4') {
	    if (verb > 0) {
		fprintf (stderr, "Erwartet 'P4' but found >%s<\n", buffer);
	    }
	    exit (1);
	}

	/* wenn die seite voll ist wird sie ausgegeben */
	/* eine blde art die farbseiten zu zaehlen - blocksperpage ist bei farbe 32 (was dann 4 farbseiten ergibt) */
	/* bei sw ist es 8, also nur eine farbseite */
	if (ccs >= (blocksPerPage / 8)) {
	    writeSiteHeader ();
	    if (verb > 1)
		fprintf (stderr, "###Seitenheader geschrieben\n");

	    if (thisSiteColorMode == 0xf0) {
		writeOut (out_stream, stFeld[3].pageOut,
			  stFeld[3].indexPageOut);
		writeOut (out_stream, stFeld[2].pageOut,
			  stFeld[2].indexPageOut);
		writeOut (out_stream, stFeld[1].pageOut,
			  stFeld[1].indexPageOut);
		clearBuffer (1);
		clearBuffer (2);
		clearBuffer (3);
	    }
	    writeOut (out_stream, stFeld[0].pageOut, stFeld[0].indexPageOut);
	    clearBuffer (0);

	    ccs = 0;
	    if (verb > 1)
		fprintf (stderr, "###Seiteninhalt geschrieben\n");
	    if (verb > 1)
		fprintf (stderr, "Farbverteilung der Seite:\n"
		"Yellow:  %8d\n"
		"Magenta: %8d\n"
		"Cyan:    %8d\n"
		"Black:   %8d\n"
		, pix[0][0],pix[1][0],pix[2][0],pix[3][0]);
	        pix[0][0]=0;
	        pix[1][0]=0;
	        pix[2][0]=0;
	        pix[3][0]=0;



	}
	/* vor der seite seitenheader nicht vergessen */
	if (ccs == 0) {
	    if (jobHeaderWritten != 1) {
		writeJobHeader ();
		jobHeaderWritten = 1;
	    }
	    reservedHeaderCountSH = headerCount++;
	    siteInitHeaderCount = headerCount;
	    thisSiteColorMode=colorMode;
	    thisSiteBlocksPerPage=blocksPerPage;
	    if (verb > 1)
		fprintf (stderr, "Reservierter SeitenHeaderCount ist %2d\n", reservedHeaderCountSH );

	}
	if (verb > 1)
	    fprintf (stderr, "Beginne neue Farbe\n");
	
	if(ccs>2) {
	    if(pix[0][0]==0 && pix[1][0]==0 && pix[2][0]==0) {
		if (verb > 1)
		    fprintf (stderr, "--------------- Switch to Black and White !\n");
		
		thisSiteColorMode=0x00;
		thisSiteBlocksPerPage=0x08;
		headerCount=headerCount-24;
		siteInitHeaderCount = headerCount;
		clearBuffer (1);
		clearBuffer (2);
		clearBuffer (3);
	    }else{
		if (verb > 1)
		    fprintf (stderr, "--------------- Seite bleibt in Farbe\n");
	    }
		
	}

	/* weitere pbm header ueberlesen - koennte mann sptereigentlich auch auswerten */

	for (readHeader = 0; readHeader < 1; readHeader++) {
	    if (fgets (buffer, 256, in_stream) != NULL) {
		if (verb > 3) {
		    fprintf (stderr, "gelesener Header: %s", buffer);
		}
		if (buffer[0] == '#') {
		    if (verb > 3)
			fprintf (stderr, "Zeile ist ein Kommentar und zaehlt nicht mit.\n");	/* diese Ausage muss noch geprueft werden ! */
		    readHeader--;
		}
		else {
		    /* format auslesen */

		    inpX = atoi (strtok (buffer, " "));
		    inpY = atoi (strtok (NULL, " "));
		    inpXBytes = (int) ceil ((double) inpX / 8);
		    lastByteMask = 0xff << ((inpXBytes * 8) - inpX);
		}
	    }
	    else {
		if (verb > 0) {
		    fprintf (stderr, "Unexpected end of in-file !?\n");
		}
		exit (1);
	    }
	}


	shz = 0;
	sbz = 0;
	/* hier werden jetzt die bytes gelesen */

	realLines = 0;
	realPixBytes = 0;
	leadLines = 0;
	leadPixBytes = 0;
	trailLines = 0;
	trailPixBytes = 0;
	xAdd = 0;
	yAdd = 0;

	if (resBreite <= inpX) {	/* eingabe brreiter als druckbereich - clippen */
	    realPixBytes = (resBreite / 8);
	    leadPixBytes =
		(int) ceil (((double) inpXBytes - ((double) resBreite / 8)) /
			    2);
	    trailPixBytes =
		(int) floor (((double) inpXBytes - ((double) resBreite / 8)) /
			     2);
	    if (verb > 5)
		fprintf (stderr,
			 "Eingabeseite zu breit - clippe auf %i mit %i am Anfang und %i am Ende.\n",
			 realPixBytes, leadPixBytes, trailPixBytes);
	}
	else {			/* hinzufuegen */
	    realPixBytes = inpXBytes;
	    leadPixBytes =
		(int) ceil ((((double) resBreite / 8) - (double) inpXBytes) /
			    2);
	    trailPixBytes =
		(int) floor ((((double) resBreite / 8) - (double) inpXBytes) /
			     2);
	    xAdd = 1;
	    if (verb > 5)
		fprintf (stderr,
			 "Eingabeseite zu schmal - addiere auf %i -> %i am Anfang und %i am Ende.\n",
			 realPixBytes, leadPixBytes, trailPixBytes);
	}

	if (resHoehe <= inpY) {	/* eingabe hoeher als druckbereich - clippen */
	    realLines = resHoehe;
	    leadLines =
		(int) ceil ((((double) inpY - (double) resHoehe) / 2));
	    trailLines =
		(int) floor ((((double) inpY - (double) resHoehe) / 2));
	    if (verb > 5)
		fprintf (stderr,
			 "Eingabeseite zu lang - clippe auf %i mit %i am Anfang und %i am Ende.\n",
			 realLines, leadLines, trailLines);
	}
	else {			/* hinzufuegen */
	    realLines = inpY;
	    leadLines =
		(int) ceil ((((double) resHoehe - (double) inpY) / 2));
	    trailLines =
		(int) floor ((((double) resHoehe - (double) inpY) / 2));
	    yAdd = 1;
	    if (verb > 5)
		fprintf (stderr,
			 "Eingabeseite zu kurz - addiere auf %i -> %i am Anfang und %i am Ende.\n",
			 realLines, leadLines, trailLines);
	}

	if (yAdd == 0) {
	    if (verb > 5)
		fprintf (stderr,
			 "ueberlese %i zeilen mit je %i bytes am Anfang\n",
			 leadLines, inpXBytes);
	    for (shz = 0; shz < leadLines; shz++) {	/* vorschleife fuer  zu ueberlesende hohenzeilen */
		for (sbz = 0; sbz < inpXBytes; sbz++) {
		    if ((c = fgetc (in_stream)) != EOF) {
			/* verwerfe das byte */
		    }
		    else {
			if (verb > 0) {
			    fprintf (stderr, "\nUps, unerwartetes Ende\n");
			}
			exit (1);
		    }
		}
	    }

	}
	else {
	    if (verb > 5)
		fprintf (stderr,
			 "erzeuge %i zeilen mit je %i bytes (0x00) am Anfang\n",
			 leadLines, (resBreite / 8));
	    for (shz = 0; shz < leadLines; shz++) {	/* vorschleife fuer fehlende hohenzeilen */
		for (sbz = 0; sbz < (resBreite / 8); sbz++) {
		    if (colorMode == 0xf0) {
			doEncode (0x00, colorKey[ccs]);
		    }
		    else {
			doEncode (0x00, colorKey[3]);
		    }
		}
	    }
	}

	for (shz = 0; shz < realLines; shz++) {
	    if (xAdd == 0) {
		if (verb > 5)
		    fprintf (stderr,
			     "ueberlese %i bytes der Zeile am Anfang\n",
			     leadPixBytes);
		for (sbz = 0; sbz < leadPixBytes; sbz++) {	/* vorschleife fuer zu ueberlesende breiten pixel */
		    if ((c = fgetc (in_stream)) != EOF) {
			/* verwerfe das byte */
		    }
		    else {
			if (verb > 0) {
			    fprintf (stderr, "\nUps, unexpected End\n");
			}
			exit (1);
		    }
		}
	    }
	    else {
		if (verb > 5)
		    fprintf (stderr, "erzeuge %i bytes mit 0x00 am Anfang\n",
			     leadPixBytes);
		for (sbz = 0; sbz < leadPixBytes; sbz++) {
		    if (colorMode == 0xf0) {
			doEncode (0x00, colorKey[ccs]);
		    }
		    else {
			doEncode (0x00, colorKey[3]);
		    }
		}

	    }

	    if (verb > 5)
		fprintf (stderr, "verabreite normal %i bytes\n",
			 realPixBytes);
	    for (sbz = 0; sbz < realPixBytes; sbz++) {
		if ((c = fgetc (in_stream)) != EOF) {
		    if ((sbz + 1) == realPixBytes) {
			if (verb > 6)
			    fprintf (stderr,
				     "letzes Byte maskieren mit %.2x - aus %.2x wird ",
				     lastByteMask, c);
			c = c & lastByteMask;	/* das letzte byte muss bei ungeraden und zu kleinen eingaben maskiert werden */
			if (verb > 6)
			    fprintf (stderr, "%.2x\n", c);
		    }
		    if (colorMode == 0xf0) {
			/* hier die pixel zaehlen um s/w seiten zu erkennen */
                        if (c & 0x80)
                            pix[ccs][0]++;
                        if (c & 0x40)
                            pix[ccs][0]++;
                        if (c & 0x20)
                            pix[ccs][0]++;
                        if (c & 0x10)
                            pix[ccs][0]++;
                        if (c & 0x08)
                            pix[ccs][0]++;
                        if (c & 0x04)
                            pix[ccs][0]++;
                        if (c & 0x02)
                            pix[ccs][0]++;
                        if (c & 0x01)
                            pix[ccs][0]++;
			doEncode (c, colorKey[ccs]);
		    }
		    else {
			doEncode (c, colorKey[3]);
		    }
		}
		else {
		    if (verb > 0) {
			fprintf (stderr, "\nUps, unexpected End\n");
		    }
		    exit (1);
		}

	    }

	    if (xAdd == 0) {
		if (verb > 5)
		    fprintf (stderr, "ueberlese %i bytes der Zeile am Ende\n",
			     trailPixBytes);
		for (sbz = 0; sbz < trailPixBytes; sbz++) {	/* vorschleife fuer zu ueberlesende breiten pixel */
		    if ((c = fgetc (in_stream)) != EOF) {
			/* verwerfe das byte */
		    }
		    else {
			if (verb > 5) {
			    fprintf (stderr, "\nUps, unexpected End\n");
			}
			exit (1);
		    }
		}
	    }
	    else {
		if (verb > 5)
		    fprintf (stderr, "erzeuge %i bytes mit 0x00 am Ende\n",
			     trailPixBytes);
		for (sbz = 0; sbz < trailPixBytes; sbz++) {
		    if (colorMode == 0xf0) {
			doEncode (0x00, colorKey[ccs]);
		    }
		    else {
			doEncode (0x00, colorKey[3]);
		    }
		}

	    }

	}

	if (yAdd == 0) {
	    if (verb > 5)
		fprintf (stderr,
			 "ueberlese %i zeilen mit je %i bytes am Ende\n",
			 trailLines, inpXBytes);
	    for (shz = 0; shz < trailLines; shz++) {	/* endschleife fuer  zu ueberlesende hohenzeilen */
		for (sbz = 0; sbz < inpXBytes; sbz++) {
		    if ((c = fgetc (in_stream)) != EOF) {
			/* verwerfe das byte */
		    }
		    else {
			if (verb > 0) {
			    fprintf (stderr, "\nUps, unexpected End\n");
			}
			exit (1);
		    }
		}
	    }

	}
	else {
	    if (verb > 5)
		fprintf (stderr,
			 "erzeuge %i zeilen mit je %i bytes (0x00) am Ende\n",
			 trailLines, (resBreite / 8));
	    for (shz = 0; shz < trailLines; shz++) {	/* endschleife fuer fehlende hohenzeilen */
		for (sbz = 0; sbz < (resBreite / 8); sbz++) {
		    if (colorMode == 0xf0) {
			doEncode (c, colorKey[ccs]);
		    }
		    else {
			doEncode (c, colorKey[3]);
		    }
		}
	    }
	}


	ccs++;
    }
    writeSiteHeader ();
    if (verb > 1)
	fprintf (stderr, "###Seitenheader geschrieben\n");

    if (thisSiteColorMode == 0xf0) {
	writeOut (out_stream, stFeld[3].pageOut, stFeld[3].indexPageOut);
	writeOut (out_stream, stFeld[2].pageOut, stFeld[2].indexPageOut);
	writeOut (out_stream, stFeld[1].pageOut, stFeld[1].indexPageOut);
    }
    writeOut (out_stream, stFeld[0].pageOut, stFeld[0].indexPageOut);
    if (verb > 1)
	fprintf (stderr, "###Seiteninhalt geschrieben\n");

    if (verb > 1)
	fprintf (stderr, "Farbverteilung der Seite:\n"
	"Yellow:  %8d\n"
	"Magenta: %8d\n"
	"Cyan:    %8d\n"
	"Black:   %8d\n"
	, pix[0][0],pix[1][0],pix[2][0],pix[3][0]);

    pix[0][0]=0;
    pix[1][0]=0;
    pix[2][0]=0;
    pix[3][0]=0;

}


int
main (int argc, char *argv[])
{
    extern char *optarg;
    char *inFile = NULL;
    char *outFile = NULL;
    int option;
    int encBufferSize;
    int blockBufferSize;
    int pageOutSize;
    long psr;

/* 1. parameter lesen */

    while ((option = getopt (argc, argv, "v:hi:o:c:m:p:r:s")) >= 0)
	switch (option) {
	case 'h':
	    Help ();
	    return (0);
	case 'i':
	    inFile = optarg;
	    break;
	case 'o':
	    outFile = optarg;
	    if (strcmp (outFile, "-") == 0)
		verb = 0;
	    break;
	case 'c':
	    if (optarg[0] == '1') {
		blocksPerPage = 0x08;
		colorMode = 0x00;
	    }
	    else if (optarg[0] == '2') {
		blocksPerPage = 0x20;
		colorMode = 0xf0;
	    }
	    else {
		if (verb > 0)
		    fprintf (stderr, "Wrong color Mode %s\n", optarg);
		exit (1);
	    }
	    break;
	case 'm':
	    MediaCode = atoi (optarg);
	    if (MediaCode > 6) {
		if (verb > 0)
		    fprintf (stderr, "Wrong Media Code %d\n", MediaCode);
		exit (1);
	    }
	    break;
	case 'p':
	    PaperCode = atoi (optarg);
	    if (PaperCode > 41 || form[PaperCode].resX == 0) {
		if (verb > 0)
		    fprintf (stderr, "Wrong Paper Code %d\n", PaperCode);
		exit (1);
	    }
	    break;
	case 'r':
	    ResXmul = atoi (optarg);
	    if (ResXmul == 1) {
		if (verb > 1)
		    fprintf (stderr, "Aufloesung 600dpi\n");
		jobHeaderStc.res1 = 0x01;
		jobHeaderStc.res2 = 0x00;
	    }
	    else if (ResXmul == 2) {
		if (verb > 1)
		    fprintf (stderr, "Aufloesung 1200dpi\n");
		jobHeaderStc.res1 = 0x02;
		jobHeaderStc.res2 = 0x01;
	    }
	    else {
		if (verb > 0)
		    fprintf (stderr, "Wrong Resolutin Mode !\n");
		exit (1);
	    }
	    break;

	case 'v':
	    verb = atoi (optarg);
	    /* commended out, since this disables debugging
	       -gfuer
	    if (outFile != NULL && strcmp (outFile, "-") == 0)
		verb = 0;
	    */
	    break;
	case 's':
	    saveToner = 1;
	    break;

/*	case 'u' : 		//ucr ist now done by crd
	    useUCR=1;
	    break;
*/
	case '?':
	    Help ();
	    return (1);
	}
    if (inFile == NULL || outFile == NULL) {
	Help ();
	return (1);
    }
/* 2. dateien oeffnen */

    if (strcmp (inFile, "-") == 0) {
	in_stream = stdin;
    }
    else {
	if ((in_stream = fopen (inFile, "r")) == NULL) {
	    if (verb > 0) {
		printf ("Fehler beim oeffnen der Eingabedatei\n");
	    }
	    perror ("");
	    exit (1);
	}
    }
    if (strcmp (outFile, "-") == 0) {
	out_stream = stdout;
    }
    else {
	if ((out_stream = fopen (outFile, "w")) == NULL) {
	    if (verb > 0) {
		printf ("Fehler beim oeffnen der Zieldatei\n");
	    }
	    perror ("");
	    exit (1);
	}
    }



    /* werte vorbereiten */
    resBreite = form[PaperCode].resX * ResXmul;
    resHoehe = form[PaperCode].resY * ResYmul;
    linesPerBlock = (form[PaperCode].resY / 8);



    /* speicher reservieren */

    encBufferSize = (resBreite / 8) + 150;	/* enthaelt im allgemeinen nur eine zeile (kleine zugabe die gleichzeitig die header abdeckt ;-) */
    blockBufferSize = encBufferSize * linesPerBlock;	/* einhaelt die zeilen mal anzahl zeilen pro block */
    pageOutSize = blockBufferSize * 8;	/* enthaelt 8 bloecke */

    stFeld[0].encBuffer = malloc (encBufferSize);
    stFeld[0].blockBuffer = malloc (blockBufferSize);
    stFeld[0].pageOut = malloc (pageOutSize);

    if (colorMode == 0xf0) {

	stFeld[1].encBuffer = malloc (encBufferSize);
	stFeld[1].blockBuffer = malloc (blockBufferSize);
	stFeld[1].pageOut = malloc (pageOutSize);

	stFeld[2].encBuffer = malloc (encBufferSize);
	stFeld[2].blockBuffer = malloc (blockBufferSize);
	stFeld[2].pageOut = malloc (pageOutSize);

	stFeld[3].encBuffer = malloc (encBufferSize);
	stFeld[3].blockBuffer = malloc (blockBufferSize);
	stFeld[3].pageOut = malloc (pageOutSize);
    }





    readPkmraw ();
    /* readBit(); */

    if (jobHeaderWritten == 1) {
	/* footer ausgeben */
	jobFooterStc.headerCount = headerCount++;
	/* pruefsumme berechnen */
	for (psr = 0; psr < 7; psr++) {
	    jobFooterStc.prSum += jobFooter[psr];
	}
	writeOut (out_stream, &jobFooter[0], 8);

	fileFooterStc.headerCount = headerCount++;
	/* pruefsumme berechnen */
	for (psr = 0; psr < 7; psr++) {
	    fileFooterStc.prSum += fileFooter[psr];
	}
	writeOut (out_stream, &fileFooter[0], 8);
	if (verb > 1)
	    fprintf (stderr, "JobFooter written.\n");
    }

    if (colorMode == 0xf0) {
	free (stFeld[0].encBuffer);
	free (stFeld[0].blockBuffer);
	free (stFeld[0].pageOut);

	free (stFeld[1].encBuffer);
	free (stFeld[1].blockBuffer);
	free (stFeld[1].pageOut);

	free (stFeld[2].encBuffer);
	free (stFeld[2].blockBuffer);
	free (stFeld[2].pageOut);
    }
    free (stFeld[3].encBuffer);
    free (stFeld[3].blockBuffer);
    free (stFeld[3].pageOut);

    fclose (in_stream);
    fclose (out_stream);
    return (0);

}
