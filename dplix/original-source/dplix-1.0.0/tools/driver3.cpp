//
// Driver mkIII
//
// An attempt to connect to the Dell 3010cn printer
// by passing in Dell Printer Language comands...
//
// by James L. Hammons
// (C) 2008 Underground Software
//

#include <stdio.h>                /* perror() */
#include <stdlib.h>               /* atoi() */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>               /* read() */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdint.h>

// Let's try some text...
#include <ft2build.h>
#include FT_FREETYPE_H

class BufferLoader
{
	public:
		BufferLoader();
		~BufferLoader();
		bool Allocate(uint32_t size);
//		void Append(uint8_t * buf, uint32_t size);
		void operator+=(uint8_t byte);
		void operator+=(uint16_t word);
		void operator+=(uint32_t dword);
		void operator+=(float f);
		void Append(const uint8_t * buffer, uint32_t size);
		uint32_t Size(void);
		const uint8_t * GetBuffer(void);

	private:
		uint8_t * array;
		uint32_t count;
		uint32_t allocatedSize;
};

BufferLoader::BufferLoader(void): array(NULL), count(0), allocatedSize(0)
{
}

BufferLoader::~BufferLoader(void)
{
	if (array)
		delete[] array;
}

bool BufferLoader::Allocate(uint32_t size)
{
	try
	{
		array = new uint8_t[size];
	}
	catch (...)
	{
		return false;
	}

	allocatedSize = size;

	return true;
}

void BufferLoader::operator+=(uint8_t byte)
{
	if (count == allocatedSize)
		throw 1;//make an enumerated type. !!! FIX !!!

	array[count++] = byte;
}

void BufferLoader::operator+=(uint16_t word)
{
	if ((count + 2) > allocatedSize)
		throw 1;//make an enumerated type. !!! FIX !!!

	array[count++] = (word & 0xFF);
	array[count++] = (word >> 8);
}

void BufferLoader::operator+=(uint32_t dword)
{
	if ((count + 4) > allocatedSize)
		throw 1;//make an enumerated type. !!! FIX !!!

	for(int i=0; i<4; i++)
	{
		array[count++] = (dword & 0xFF);
		dword >>= 8;
	}
}

void BufferLoader::operator+=(float f)
{
	if ((count + 4) > allocatedSize)
		throw 1;//make an enumerated type. !!! FIX !!!

	// NOTE: This is little endian specific. !!! FIX !!!
	uint8_t * pFlt = (uint8_t *)&f;

	array[count++] = pFlt[0];
	array[count++] = pFlt[1];
	array[count++] = pFlt[2];
	array[count++] = pFlt[3];
}

uint32_t BufferLoader::Size(void)
{
	return count;
}

const uint8_t * BufferLoader::GetBuffer(void)
{
	return array;
}

void BufferLoader::Append(const uint8_t * buffer, uint32_t size)
{
	if ((count + size) > allocatedSize)
		throw 1;//make an enumerated type. !!! FIX !!!

	memcpy(&array[count], buffer, size);
	count += size;
}


class DPL
{
	public:
		DPL();
		~DPL();
		bool Allocate(uint32_t size);
		void Write(uint8_t byte);
		void Write(uint8_t reg, uint8_t byte);
		void Write(uint8_t reg, uint8_t byte1, uint8_t byte2);
		void Write(uint8_t reg, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);
		void Write(uint8_t reg, uint16_t word);
		void Write(uint8_t reg, uint16_t word1, uint16_t word2);
		void Write(uint8_t reg, uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4);
		void Write(uint8_t reg, uint32_t dword);
		void Write(uint8_t reg, uint32_t dword1, uint32_t dword2);
		void Write(uint8_t reg, uint32_t dword1, uint32_t dword2, uint32_t dword3, uint32_t dword4);
		void Write(uint8_t reg, float flt);
		void Write(uint8_t reg, float flt1, float flt2);
		void Write(const uint8_t * data, uint32_t length);
		void Write(uint8_t reg, const void * buffer, uint32_t size);
		void WriteW(uint8_t reg, const void * buffer, uint32_t size);
		void Append(const uint8_t * buffer, uint32_t size);
		uint32_t Size(void);
		const uint8_t * GetBuffer(void);

	private:
		BufferLoader buf;
};

DPL::DPL(void)
{
}

DPL::~DPL(void)
{
}

bool DPL::Allocate(uint32_t size)
{
	return buf.Allocate(size);
}

void DPL::Write(uint8_t byte)
{
	buf += byte;
}

void DPL::Write(uint8_t reg, uint8_t byte)
{
	buf += (uint8_t)0xC0;
	buf += byte;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, uint8_t byte1, uint8_t byte2)
{
	buf += (uint8_t)0xD0;
	buf += byte1;
	buf += byte2;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4)
{
	buf += (uint8_t)0xE0;
	buf += byte1;
	buf += byte2;
	buf += byte3;
	buf += byte4;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, uint16_t word)
{
	buf += (uint8_t)0xC1;						// Can also be $C3, but when???
	buf += word;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, uint16_t word1, uint16_t word2)
{
	buf += (uint8_t)0xD1;						// Can also be $D3, but when???
	buf += word1;
	buf += word2;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4)
{
	buf += (uint8_t)0xE1;						// Can also be $E3, but when???
	buf += word1;
	buf += word2;
	buf += word3;
	buf += word4;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, uint32_t dword)
{
	buf += (uint8_t)0xC2;
	buf += dword;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, uint32_t dword1, uint32_t dword2)
{
	buf += (uint8_t)0xD2;
	buf += dword1;
	buf += dword2;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, uint32_t dword1, uint32_t dword2, uint32_t dword3, uint32_t dword4)
{
	buf += (uint8_t)0xE2;
	buf += dword1;
	buf += dword2;
	buf += dword3;
	buf += dword4;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, float flt)
{
	buf += (uint8_t)0xC5;
	buf += flt;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(uint8_t reg, float flt1, float flt2)
{
	buf += (uint8_t)0xD5;
	buf += flt1;
	buf += flt2;
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Write(const uint8_t * data, uint32_t length)
{
	buf += (uint8_t)(length < 256 ? 0xFB : 0xFA);

	if (length < 256)
		buf += (uint8_t)length;
	else
		buf += length;

	buf.Append(data, length);
}

void DPL::Write(uint8_t reg, const void * buffer, uint32_t size)
{
	buf += (uint8_t)0xC8;
	buf += (uint8_t)(size < 256 ? 0xC0 : 0xC1);	// Could be bigger as well...

	if (size < 256)
		buf += (uint8_t)size;
	else
		buf += (uint16_t)size;

	buf.Append((const uint8_t *)buffer, size);
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::WriteW(uint8_t reg, const void * buffer, uint32_t size)
{
	buf += (uint8_t)0xC9;
	buf += (uint8_t)(size < 256 ? 0xC0 : 0xC1);	// Could be bigger as well...

	if (size < 256)
		buf += (uint8_t)size;
	else
		buf += (uint16_t)size;

	buf.Append((const uint8_t *)buffer, size * 2);
	buf += (uint8_t)0xF8;
	buf += reg;
}

void DPL::Append(const uint8_t * buffer, uint32_t size)
{
	buf.Append(buffer, size);
}

uint32_t DPL::Size(void)
{
	return buf.Size();
}

const uint8_t * DPL::GetBuffer(void)
{
	return buf.GetBuffer();
}


const uint8_t escape[9] = { 0x1B, 0x25, 0x2D, 0x31, 0x32, 0x33, 0x34, 0x35, 0x58 };

// NOTE: Need to go through and figure out what's necessary and what isn't.
//       @PJL COMMENTs can be deleted
const uint8_t preamble[] = {
//	"@PJL COMMENT DATE=10/26/2008\x0A"
//	"@PJL COMMENT TIME=01:00:38\x0A"
//	"@PJL COMMENT DNAME=Wallet prints\x0A"
	"@PJL JOB MODE=PRINTER\x0A"
	"@PJL SET JOBATTR=\"@LUNA=User\"\x0A"
	"@PJL SET COPIES=1\x0A"
	"@PJL SET QTY=1\x0A"
	"@PJL SET JOBATTR=\"@TRCH=OFF\"\x0A"
	"@PJL SET DUPLEX=OFF\x0A"
	"@PJL SET BINDING=LONGEDGE\x0A"
	"@PJL SET JOBATTR=\"@SPSE=AUTO\"\x0A"
	"@PJL SET JOBATTR=\"@MSIP=NORMAL\"\x0A"
	"@PJL SET RENDERMODE=COLOR\x0A"
	"@PJL SET ECONOMODE=OFF\x0A"
	"@PJL SET JOBATTR=\"@IREC=OFF\"\x0A"
	"@PJL SET JOBATTR=\"@TRAP=OFF\"\x0A"
	"@PJL SET JOBATTR=\"@HOAD=I0A00020F\"\x0A"
	"@PJL SET JOBATTR=\"@JOAU=User\"\x0A"
	"@PJL SET JOBATTR=\"@CNAM=XPPRO-VM\"\x0A"
	"@PJL SET OUTBIN=FACEDOWN\x0A"
	"@PJL SET JOBATTR=\"@PODR=NORMAL\"\x0A"
	"@PJL SET SLIPSHEET=OFF\x0A"
	"@PJL SET PAPERDIRECTION=SEF\x0A"
	"@PJL SET JOBATTR=\"@APSP=OFF\"\x0A"
	"@PJL SET RESOLUTION=300\x0A"
	"@PJL SET BITSPERPIXEL=8\x0A"
	"@PJL SET JOBATTR=\"@DRDM=QCIQUALITY\"\x0A"
	"@PJL SET JOBATTR=\"@TSCR=9\"\x0A"
	"@PJL SET JOBATTR=\"@GSCR=9\"\x0A"
	"@PJL SET JOBATTR=\"@ISCR=9\"\x0A"
	"@PJL SET JOBATTR=\"@TTRC=9\"\x0A"
	"@PJL SET JOBATTR=\"@GTRC=9\"\x0A"
	"@PJL SET JOBATTR=\"@ITRC=9\"\x0A"
	"@PJL SET JOBATTR=\"@TCPR=24\"\x0A"
	"@PJL SET JOBATTR=\"@GCPR=24\"\x0A"
	"@PJL SET JOBATTR=\"@ICPR=25\"\x0A"
	"@PJL SET JOBATTR=\"@TUCR=24\"\x0A"
	"@PJL SET JOBATTR=\"@GUCR=24\"\x0A"
	"@PJL SET JOBATTR=\"@IUCR=25\"\x0A"
	"@PJL SET JOBATTR=\"@TDFT=1\"\x0A"
	"@PJL SET JOBATTR=\"@GDFT=1\"\x0A"
	"@PJL SET JOBATTR=\"@IDFT=1\"\x0A"
	"@PJL SET JOBATTR=\"@BANR=DEVICE\"\x0A"
	"@PJL ENTER LANGUAGE=DPL\x0A"
	") HP-PCL XL;2;1;FXXPL2-1.0.0,WINNT-5.1,XPL2DRV-1.0.6.0\x0A"
};

uint8_t eoj[10] = { "@PJL EOJ\x0A" };

// Helper functions

void StuffWord(uint8_t * array, uint32_t &counter, uint16_t word)
{
	array[counter++] = (word & 0xFF);
	array[counter++] = (word >> 8);
}

void StuffDWord(uint8_t * array, uint32_t &counter, uint32_t dword)
{
	for(int i=0; i<4; i++)
	{
		array[counter++] = (dword & 0xFF);
		dword >>= 8;
	}
}

void StuffFloat(uint8_t * array, uint32_t &counter, float flt)
{
	// NOTE: This is little endian specific. !!! FIX !!!
	uint8_t * pFlt = (uint8_t *)&flt;

	array[counter++] = pFlt[0];
	array[counter++] = pFlt[1];
	array[counter++] = pFlt[2];
	array[counter++] = pFlt[3];
}

// More helper functions

void WriteByte(uint8_t * array, uint32_t &counter, uint8_t reg, uint8_t byte)
{
	array[counter++] = 0xC0;
	array[counter++] = byte;
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void Write2Bytes(uint8_t * array, uint32_t &counter, uint8_t reg, uint8_t byte1, uint8_t byte2)
{
	array[counter++] = 0xD0;
	array[counter++] = byte1;
	array[counter++] = byte2;
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void Write4Bytes(uint8_t * array, uint32_t &counter, uint8_t reg, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4)
{
	array[counter++] = 0xE0;
	array[counter++] = byte1;
	array[counter++] = byte2;
	array[counter++] = byte3;
	array[counter++] = byte4;
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void WriteWord(uint8_t * array, uint32_t &counter, uint8_t reg, uint16_t word)
{
	array[counter++] = 0xC1;					// Can also be $C3, but when???
	StuffWord(array, counter, word);
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void Write2Words(uint8_t * array, uint32_t &counter, uint8_t reg, uint16_t word1, uint16_t word2)
{
	array[counter++] = 0xD1;					// Can also be $D3, but when???
	StuffWord(array, counter, word1);
	StuffWord(array, counter, word2);
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void Write4Words(uint8_t * array, uint32_t &counter, uint8_t reg, uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4)
{
	array[counter++] = 0xE1;					// Can also be $E3, but when???
	StuffWord(array, counter, word1);
	StuffWord(array, counter, word2);
	StuffWord(array, counter, word3);
	StuffWord(array, counter, word4);
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void WriteDWord(uint8_t * array, uint32_t &counter, uint8_t reg, uint32_t dword)
{
	array[counter++] = 0xC2;
	StuffDWord(array, counter, dword);
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void Write2DWords(uint8_t * array, uint32_t &counter, uint8_t reg, uint32_t dword1, uint32_t dword2)
{
	array[counter++] = 0xD2;
	StuffDWord(array, counter, dword1);
	StuffDWord(array, counter, dword2);
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void Write4DWords(uint8_t * array, uint32_t &counter, uint8_t reg, uint32_t dword1, uint32_t dword2, uint32_t dword3, uint32_t dword4)
{
	array[counter++] = 0xE2;
	StuffDWord(array, counter, dword1);
	StuffDWord(array, counter, dword2);
	StuffDWord(array, counter, dword3);
	StuffDWord(array, counter, dword4);
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void WriteFloat(uint8_t * array, uint32_t &counter, uint8_t reg, float flt)
{
	array[counter++] = 0xC5;
	StuffFloat(array, counter, flt);
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void Write2Floats(uint8_t * array, uint32_t &counter, uint8_t reg, float flt1, float flt2)
{
	array[counter++] = 0xD5;
	StuffFloat(array, counter, flt1);
	StuffFloat(array, counter, flt2);
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void WriteDataChunk(uint8_t * array, uint32_t &counter, const uint8_t * data, uint32_t length)
{
	array[counter++] = (length < 256 ? 0xFB : 0xFA);

	if (length < 256)
		array[counter++] = length;
	else
		StuffDWord(array, counter, length);

	memcpy(&array[counter], data, length);
	counter += length;
}

void WriteBuffer(uint8_t * array, uint32_t &counter, uint8_t reg, const void * buf, uint32_t size)
{
	array[counter++] = 0xC8;
	array[counter++] = (size < 256 ? 0xC0 : 0xC1);	// Could be bigger as well...

	if (size < 256)
		array[counter++] = size;
	else
		StuffWord(array, counter, size);

	memcpy(&array[counter], buf, size);
	counter += size;
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

void WriteWBuffer(uint8_t * array, uint32_t &counter, uint8_t reg, const void * buf, uint32_t size)
{
	array[counter++] = 0xC9;
	array[counter++] = (size < 256 ? 0xC0 : 0xC1);	// Could be bigger as well...

	if (size < 256)
		array[counter++] = size;
	else
		StuffWord(array, counter, size);

	memcpy(&array[counter], buf, size*2);
	counter += size*2;
	array[counter++] = 0xF8;
	array[counter++] = reg;
}

//
// Truetype stuff
//

FT_Library library;										// Handle to library
FT_Face face;											// Handle to face object

uint8_t * ch[0x10000];									// Local glyph cache
uint32_t chSize[0x10000];								// Local glyph cache sizes
uint16_t chWidth[0x10000];								// "     "     "     widths

void InitFreeType(void)
{
	int error = FT_Init_FreeType(&library);
	if (error)
	{
		printf("... an error occurred during library initialization ...\n");
	}

	error = FT_New_Face(library, "/usr/share/fonts/corefonts/arial.ttf", 0, &face);
	if (error == FT_Err_Unknown_File_Format)
	{
		printf("... the font file could be opened and read, but it appears\n");
		printf("... that its font format is unsupported\n");
	}
	else if (error)
	{
		printf("... another error code means that the font file could not\n");
		printf("... be opened or read, or simply that it is broken...\n");
	}

	error = FT_Set_Char_Size(
		face,		/* handle to face object           */
		0,			/* char_width in 1/64th of points  */
		10*64,		/* char_height in 1/64th of points */
		600,		/* horizontal device resolution    */
		600);		/* vertical device resolution      */
}

void PrepCharArray(void)
{
	for(uint32_t i=0; i<0x10000; i++)
		ch[i] = NULL;
}

void FreeCharArray(void)
{
	for(uint32_t i=0; i<0x10000; i++)
		if (ch[i] != NULL)
			delete[] ch[i];
}

uint32_t GetCharGlyph(uint8_t * &pBits, wchar_t c)
{
	if (ch[c] == NULL)
	{
//		int error = FT_Load_Char(face, c, FT_LOAD_MONOCHROME | FT_LOAD_RENDER);
		int error = FT_Load_Char(face, c, FT_LOAD_TARGET_MONO | FT_LOAD_RENDER);
		if (error)
		{
			printf("... An error occurred in the FT_Load_Char() function...\n");
		}

		FT_GlyphSlot slot = face->glyph;		/* a small shortcut */
fprintf(stderr, "GCG: Glyph: %c, width=%d, height=%d, pitch=%d, left=%d, top=%d, aw=%d\n", c, slot->bitmap.width, slot->bitmap.rows, slot->bitmap.pitch, slot->bitmap_left, slot->bitmap_top, slot->advance.x >> 6);

		uint32_t realWidth = (slot->bitmap.width + 7) / 8;
		uint32_t bitmapByteSize = slot->bitmap.rows * realWidth;
		ch[c] = new uint8_t[bitmapByteSize + 10];

		for(uint32_t y=0; y<slot->bitmap.rows; y++)
			for(uint32_t x=0; x<realWidth; x++)
				ch[c][10 + (y * realWidth) + x] = slot->bitmap.buffer[(y * slot->bitmap.pitch) + x];

		ch[c][0] = 0x00;
		ch[c][1] = 0x00;
		ch[c][2] = slot->bitmap_left >> 8;				// left (?)
		ch[c][3] = slot->bitmap_left & 0xFF;
		ch[c][4] = slot->bitmap_top >> 8;				// top (?)
		ch[c][5] = slot->bitmap_top & 0xFF;
		ch[c][6] = slot->bitmap.width >> 8;				// width (in pixels)
		ch[c][7] = slot->bitmap.width & 0xFF;
		ch[c][8] = slot->bitmap.rows >> 8;				// height (in pixels)
		ch[c][9] = slot->bitmap.rows & 0xFF;

		chSize[c] = bitmapByteSize + 10;
		chWidth[c] = slot->advance.x >> 6;
	}

	pBits = ch[c];
	return chSize[c];
}

//
// Driver tests
//

void Test1(uint8_t * array, uint32_t &counter, char * bits, int size)
{
	memcpy(array, escape, 9);
	counter += 9;
	memcpy(&array[counter], preamble, strlen((const char *)preamble));
	counter += strlen((const char *)preamble);

	// This seems to be common job header for all jobs...

	Write2Words(array, counter, 0x89, 600, 600);		// DPI?
	WriteByte(array, counter, 0x86, 0);
	WriteByte(array, counter, 0x8F, 0);
	array[counter++] = 0x41;
	WriteByte(array, counter, 0x88, 0);
	WriteByte(array, counter, 0x82, 0);					// Can be 1 too (b/w page)...
	array[counter++] = 0x48;

	// This seems to be common page header for all pages...

	WriteByte(array, counter, 0x28, 0);
	WriteByte(array, counter, 0x26, 2);
	WriteByte(array, counter, 0x25, 0);
	array[counter++] = 0x43;
	array[counter++] = 0x61;

	// This seems to common logical page header, above is for phys. page...
	array[counter++] = 0x74;
	Write2Words(array, counter, 0x2A, 96, 96);			// Position of log. pg. on phys. pg.
//b/w: $74 [(w)96,6503->$2A] $75 [(w)90->$29] $76 [(f)1.0,1.0->$2B] $77
	array[counter++] = 0x75;
	WriteWord(array, counter, 0x29, 0);					// Degrees to rotate page
	array[counter++] = 0x76;
	Write2Floats(array, counter, 0x2B, 1.0f, 1.0f);		// Logical page scale x/y
	array[counter++] = 0x77;

	// Actual bitmap routine

	WriteByte(array, counter, 0x2C, 204);
	array[counter++] = 0x7B;
	WriteByte(array, counter, 0x2D, 0);
	array[counter++] = 0x78;
	WriteByte(array, counter, 0x2D, 0);
	array[counter++] = 0x7C;
	array[counter++] = 0x69;
//	Write2Bytes(array, counter, 0x4C, 128, 203);		// Location on page x/y
	Write2Bytes(array, counter, 0x4C, 0, 0);			// Location on page x/y
	array[counter++] = 0x6B;

	//only needed if changing modes...
	WriteByte(array, counter, 0x03, 2);					// Tells it to expect RGB data?
// Palette goes here: $02=2, varlen w/palette (RGB triplets)->$06
	array[counter++] = 0x6A;

	WriteByte(array, counter, 0x64, 0);					// 1=paletted, 0=non
	WriteByte(array, counter, 0x62, 2);					// 0,1,2=1bit,4bit,8bit palette
	WriteWord(array, counter, 0x6C, 40);				// Pixmap width
	WriteWord(array, counter, 0x6B, 53);				// Pixmap height
//	Write2Words(array, counter, 0x67, 1500, 1950);		// Scale x/y (@ 600 DPI)
	Write2Words(array, counter, 0x67, 40*4, 53*4);		// Scale x/y (@ 600 DPI)
	array[counter++] = 0xB0;
	WriteWord(array, counter, 0x6D, 0);					// Starting scanline
	WriteWord(array, counter, 0x63, 53);				// # of scanlines in chunk
	WriteByte(array, counter, 0x65, 3);					// Always 3
	array[counter++] = 0xB1;
	WriteDataChunk(array, counter, (uint8_t *)bits, size);
	array[counter++] = 0xB2;


	array[counter++] = 0x44;							// End of page

	array[counter++] = 0x49;							// End of job ("IB")
	array[counter++] = 0x42;

	memcpy(&array[counter], escape, 9);
	counter += 9;

	memcpy(&array[counter], eoj, 9);
	counter += 9;
}

void Test2(uint8_t * array, uint32_t &counter, const uint8_t * bits, uint32_t size)
{
	memcpy(array, escape, 9);
	counter += 9;
	memcpy(&array[counter], preamble, strlen((const char *)preamble));
	counter += strlen((const char *)preamble);

	// This seems to be common job header for all jobs...

	Write2Words(array, counter, 0x89, 600, 600);		// DPI?
	WriteByte(array, counter, 0x86, 0);
	WriteByte(array, counter, 0x8F, 0);
	array[counter++] = 0x41;
	WriteByte(array, counter, 0x88, 0);
	WriteByte(array, counter, 0x82, 0);					// Can be 1 too (b/w page)...
	array[counter++] = 0x48;

	// This seems to be common page header for all pages...

	WriteByte(array, counter, 0x28, 0);
	WriteByte(array, counter, 0x26, 2);
	WriteByte(array, counter, 0x25, 0);
	array[counter++] = 0x43;
	array[counter++] = 0x61;

	// This seems to common logical page header, above is for phys. page...
	array[counter++] = 0x74;
	Write2Words(array, counter, 0x2A, 96, 96);			// Position of log. pg. on phys. pg.
//b/w: $74 [(w)96,6503->$2A] $75 [(w)90->$29] $76 [(f)1.0,1.0->$2B] $77
	array[counter++] = 0x75;
	WriteWord(array, counter, 0x29, 0);					// Degrees to rotate page
	array[counter++] = 0x76;
	Write2Floats(array, counter, 0x2B, 1.0f, 1.0f);		// Logical page scale x/y
	array[counter++] = 0x77;

	uint8_t bmHdr[24] = { 0x00, 0x00, 0x01, 0x35, 0xFE, 0x00, 0x04, 0x00, 0x42, 0x52, 0x00, 0x00, 0x00, 0x04, 0x02, 0x58, 0x02, 0x58, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 };
//	uint8_t rgb1[3] = { 0xFF, 0xFF, 0xFF }, rgb2[3] = { 0x00, 0x00, 0x00 };
	uint8_t fontName[43] = "JohannesNormal:42A60000:0000:3F800000:BM:0";// (83.0)

	array[counter++] = 0x69;
	WriteByte(array, counter, 0x2C, 240);
	array[counter++] = 0x7B;
	WriteByte(array, counter, 0x2D, 0);
	array[counter++] = 0x78;
	WriteByte(array, counter, 0x2D, 0);
	array[counter++] = 0x7C;
	WriteByte(array, counter, 0x03, 2);
	array[counter++] = 0x6A;
	WriteBuffer(array, counter, 0x0B, "\xFF\xFF\xFF", 3);
	array[counter++] = 0x63;
	WriteByte(array, counter, 0x05, 0);
	array[counter++] = 0x79;
	WriteByte(array, counter, 0x46, 1);
	array[counter++] = 0x6E;
//Not needed...
//	Write4Words(array, counter, 0x42, 653, 503, 4253, 707);
//	array[counter++] = 0xA0;
//	WriteBuffer(array, counter, 0x0B, "\x00\x00\x00", 3);	// Text color
	WriteBuffer(array, counter, 0x0B, "\xFF\x00\x7F", 3);	// Text color
	array[counter++] = 0x63;
	WriteBuffer(array, counter, 0xA8, fontName, 42);
	WriteByte(array, counter, 0xA9, 0);
	array[counter++] = 0x4F;
	WriteWord(array, counter, 0xA7, 24);
	array[counter++] = 0x50;
	WriteDataChunk(array, counter, bmHdr, 24);
	array[counter++] = 0x51;
	WriteBuffer(array, counter, 0xA8, fontName, 42);
	array[counter++] = 0x52;

//	WriteWord(array, counter, 0xA2, 0);					// Glyph index
	WriteWord(array, counter, 0xA2, 0x41);				// Glyph index
	WriteDWord(array, counter, 0xA3, size);				// Size of bm glyph in bytes
	array[counter++] = 0x53;
	WriteDataChunk(array, counter, bits, size);			// Actual bm glyph data

	array[counter++] = 0x54;
	WriteFloat(array, counter, 0xB1, 0.0f);
	array[counter++] = 0x7D;
	Write2Floats(array, counter, 0xA5, 0.0f, 0.0f);
	array[counter++] = 0x66;
	Write2Floats(array, counter, 0xA4, 1.0f, 1.0f);
	array[counter++] = 0x65;
	WriteBuffer(array, counter, 0xAC, "\x00", 1);
	array[counter++] = 0x81;
	WriteByte(array, counter, 0xAD, 0);
	array[counter++] = 0x56;
	WriteBuffer(array, counter, 0xA8, fontName, 42);
	WriteFloat(array, counter, 0xA6, 83.0f);			// point size * ???
	WriteWord(array, counter, 0xAA, 309);				// 590 on others...
	array[counter++] = 0x6F;
	Write2Words(array, counter, 0x4C, 653, 662);		// Location on page x/y
	array[counter++] = 0x6B;
	WriteWord(array, counter, 0xA1, 0);
	array[counter++] = 0x64;
//	WriteWBuffer(array, counter, 0xAB, "\x00\x00", 1);	// wchar text string
	WriteWBuffer(array, counter, 0xAB, "\x41\x00", 1);	// wchar text string (glyph index, actually)
	WriteWBuffer(array, counter, 0xAF, "\x00\x00", 1);	// wchar ??? string
	array[counter++] = 0xA8;
	WriteBuffer(array, counter, 0xA8, fontName, 42);
	array[counter++] = 0x55;


	array[counter++] = 0x44;							// End of page D

	array[counter++] = 0x49;							// End of job ("IB")
	array[counter++] = 0x42;

	memcpy(&array[counter], escape, 9);
	counter += 9;

	memcpy(&array[counter], eoj, 9);
	counter += 9;
}

void Test3(uint8_t * array, uint32_t &counter, const void * text_in)
{
	uint8_t * text = (uint8_t *)text_in;

	memcpy(array, escape, 9);
	counter += 9;
	memcpy(&array[counter], preamble, strlen((const char *)preamble));
	counter += strlen((const char *)preamble);

	// This seems to be common job header for all jobs...

	Write2Words(array, counter, 0x89, 600, 600);		// DPI?
	WriteByte(array, counter, 0x86, 0);
	WriteByte(array, counter, 0x8F, 0);
	array[counter++] = 0x41;
	WriteByte(array, counter, 0x88, 0);
	WriteByte(array, counter, 0x82, 0);					// Can be 1 too (b/w page)...
	array[counter++] = 0x48;

	// This seems to be common page header for all pages...

	WriteByte(array, counter, 0x28, 0);
	WriteByte(array, counter, 0x26, 2);
	WriteByte(array, counter, 0x25, 0);
	array[counter++] = 0x43;
	array[counter++] = 0x61;

	// This seems to common logical page header, above is for phys. page...
//b/w, rot.: $74 [(w)96,6503->$2A] $75 [(w)90->$29] $76 [(f)1.0,1.0->$2B] $77
	array[counter++] = 0x74;
	Write2Words(array, counter, 0x2A, 96, 96);			// Position of log. pg. on phys. pg.
	array[counter++] = 0x75;
	WriteWord(array, counter, 0x29, 0);					// Degrees to rotate page
	array[counter++] = 0x76;
	Write2Floats(array, counter, 0x2B, 1.0f, 1.0f);		// Logical page scale x/y
	array[counter++] = 0x77;

	uint8_t bmHdr[24] = { 0x00, 0x00, 0x01, 0x35, 0xFE, 0x00, 0x04, 0x00, 0x42, 0x52, 0x00, 0x00, 0x00, 0x04, 0x02, 0x58, 0x02, 0x58, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 };
	uint8_t fontName[43] = "FooBar:42A60000:0000:3F800000:BM:0";// (83.0)

	array[counter++] = 0x69;
	WriteByte(array, counter, 0x2C, 240);
	array[counter++] = 0x7B;
	WriteByte(array, counter, 0x2D, 0);
	array[counter++] = 0x78;
	WriteByte(array, counter, 0x2D, 0);
	array[counter++] = 0x7C;
	WriteByte(array, counter, 0x03, 2);
	array[counter++] = 0x6A;
//Not needed...
//	WriteBuffer(array, counter, 0x0B, "\xFF\xFF\xFF", 3);
//	array[counter++] = 0x63;
	WriteByte(array, counter, 0x05, 0);
	array[counter++] = 0x79;
	WriteByte(array, counter, 0x46, 1);
	array[counter++] = 0x6E;
//Not needed...
//	Write4Words(array, counter, 0x42, 653, 503, 4253, 707);
//	array[counter++] = 0xA0;
	WriteBuffer(array, counter, 0x0B, "\x00\x00\x00", 3);	// Text color
//	WriteBuffer(array, counter, 0x0B, "\xFF\x00\x7F", 3);	// Text color
	array[counter++] = 0x63;

	WriteBuffer(array, counter, 0xA8, fontName, 42);
	WriteByte(array, counter, 0xA9, 0);
	array[counter++] = 0x4F;
	WriteWord(array, counter, 0xA7, 24);
	array[counter++] = 0x50;
	WriteDataChunk(array, counter, bmHdr, 24);
	array[counter++] = 0x51;
	WriteBuffer(array, counter, 0xA8, fontName, 42);
	array[counter++] = 0x52;

#if 0
//	WriteWord(array, counter, 0xA2, 0);					// Glyph index
	WriteWord(array, counter, 0xA2, 0x41);				// Glyph index
	WriteDWord(array, counter, 0xA3, size);				// Size of bm glyph in bytes
	array[counter++] = 0x53;
	WriteDataChunk(array, counter, bits, size);			// Actual bm glyph data
#else
	uint32_t len = strlen((char *)text);
	for(uint32_t i=0; i<len; i++)
	{
		if (ch[text[i]] == NULL)						// Only pull it in the 1st time
		{
			uint8_t * bits = NULL;
			uint32_t size = GetCharGlyph(bits, text[i]);

			WriteWord(array, counter, 0xA2, text[i]);	// Glyph index
			WriteDWord(array, counter, 0xA3, size);		// Size of bm glyph in bytes
			array[counter++] = 0x53;
			WriteDataChunk(array, counter, bits, size);	// Actual bm glyph data
		}
	}
#endif

	array[counter++] = 0x54;
	WriteFloat(array, counter, 0xB1, 0.0f);
	array[counter++] = 0x7D;
	Write2Floats(array, counter, 0xA5, 0.0f, 0.0f);
	array[counter++] = 0x66;
	Write2Floats(array, counter, 0xA4, 1.0f, 1.0f);
	array[counter++] = 0x65;
	WriteBuffer(array, counter, 0xAC, "\x00", 1);
	array[counter++] = 0x81;
	WriteByte(array, counter, 0xAD, 0);
	array[counter++] = 0x56;
	WriteBuffer(array, counter, 0xA8, fontName, 42);
	WriteFloat(array, counter, 0xA6, 83.0f);			// point size * ???
	WriteWord(array, counter, 0xAA, 309);				// 590 on others... ???
	array[counter++] = 0x6F;
	Write2Words(array, counter, 0x4C, 353, 600);		// Location on page x/y
	array[counter++] = 0x6B;
	WriteWord(array, counter, 0xA1, 0);
	array[counter++] = 0x64;
#if 0
//	WriteWBuffer(array, counter, 0xAB, "\x00\x00", 1);	// wchar text string
	WriteWBuffer(array, counter, 0xAB, "\x41\x00", 1);	// wchar text string (glyph index, actually)
	WriteWBuffer(array, counter, 0xAF, "\x00\x00", 1);	// wchar ??? string
#else
	uint8_t buf[1024], buf2[1024];
	for(uint32_t i=0; i<len; i++)
	{
		buf[(i * 2) + 0] = text[i];
		buf[(i * 2) + 1] = 0x00;
		buf2[(i * 2) + 0] = chWidth[text[i]] & 0xFF;
		buf2[(i * 2) + 1] = chWidth[text[i]] >> 8;
	}
	WriteWBuffer(array, counter, 0xAB, buf, len);		// wchar text string (glyph index, actually)
	WriteWBuffer(array, counter, 0xAF, buf2, len);		// wchar ADVANCE WIDTHs string
#endif
	array[counter++] = 0xA8;
	WriteBuffer(array, counter, 0xA8, fontName, 42);
	array[counter++] = 0x55;


	array[counter++] = 0x44;							// End of page D

	array[counter++] = 0x49;							// End of job ("IB")
	array[counter++] = 0x42;

	memcpy(&array[counter], escape, 9);
	counter += 9;

	memcpy(&array[counter], eoj, 9);
	counter += 9;
}

void Test3a(DPL &dpl, const void * text_in)
{
	uint8_t * text = (uint8_t *)text_in;

	dpl.Append(escape, 9);
	dpl.Append(preamble, strlen((const char *)preamble));

	// This seems to be common job header for all jobs...

	dpl.Write(0x89, (uint16_t)600, 600);				// DPI?
	dpl.Write(0x86, (uint8_t)0);
	dpl.Write(0x8F, (uint8_t)0);
	dpl.Write(0x41);
	dpl.Write(0x88, (uint8_t)0);
	dpl.Write(0x82, (uint8_t)0);						// Can be 1 too (b/w page)...
	dpl.Write(0x48);

	// This seems to be common page header for all pages...

	dpl.Write(0x28, (uint8_t)0);
	dpl.Write(0x26, (uint8_t)2);
	dpl.Write(0x25, (uint8_t)0);
	dpl.Write(0x43);
	dpl.Write(0x61);

	// This seems to common logical page header, above is for phys. page...
//b/w, rot.: $74 [(w)96,6503->$2A] $75 [(w)90->$29] $76 [(f)1.0,1.0->$2B] $77
	dpl.Write(0x74);
	dpl.Write(0x2A, (uint16_t)96, 96);					// Position of log. pg. on phys. pg.
	dpl.Write(0x75);
	dpl.Write(0x29, (uint16_t)0);						// Degrees to rotate page
	dpl.Write(0x76);
	dpl.Write(0x2B, 1.0f, 1.0f);						// Logical page scale x/y
	dpl.Write(0x77);

	uint8_t bmHdr[24] = { 0x00, 0x00, 0x01, 0x35, 0xFE, 0x00, 0x04, 0x00, 0x42, 0x52, 0x00, 0x00, 0x00, 0x04, 0x02, 0x58, 0x02, 0x58, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 };
	uint8_t fontName[43] = "FooBar:42A60000:0000:3F800000:BM:0";// (83.0)

	dpl.Write(0x69);
	dpl.Write(0x2C, (uint8_t)240);
	dpl.Write(0x7B);
	dpl.Write(0x2D, (uint8_t)0);
	dpl.Write(0x78);
	dpl.Write(0x2D, (uint8_t)0);
	dpl.Write(0x7C);
	dpl.Write(0x03, (uint8_t)2);
	dpl.Write(0x6A);
//Not needed...
//	WriteBuffer(array, counter, 0x0B, "\xFF\xFF\xFF", 3);
//	array[counter++] = 0x63;
	dpl.Write(0x05, (uint8_t)0);
	dpl.Write(0x79);
	dpl.Write(0x46, (uint8_t)1);
	dpl.Write(0x6E);
//Not needed...
//	Write4Words(array, counter, 0x42, 653, 503, 4253, 707);
//	array[counter++] = 0xA0;
	dpl.Write(0x0B, "\x00\x00\x00", 3);					// Text color
//	WriteBuffer(array, counter, 0x0B, "\xFF\x00\x7F", 3);	// Text color
	dpl.Write(0x63);

	dpl.Write(0xA8, fontName, 42);
	dpl.Write(0xA9, (uint8_t)0);
	dpl.Write(0x4F);
	dpl.Write(0xA7, (uint16_t)24);
	dpl.Write(0x50);
	dpl.Write(bmHdr, 24);
	dpl.Write(0x51);
	dpl.Write(0xA8, fontName, 42);
	dpl.Write(0x52);

#if 0
//	WriteWord(array, counter, 0xA2, 0);					// Glyph index
	WriteWord(array, counter, 0xA2, 0x41);				// Glyph index
	WriteDWord(array, counter, 0xA3, size);				// Size of bm glyph in bytes
	array[counter++] = 0x53;
	WriteDataChunk(array, counter, bits, size);			// Actual bm glyph data
#else
	uint32_t len = strlen((char *)text);
	for(uint32_t i=0; i<len; i++)
	{
		if (ch[text[i]] == NULL)						// Only pull it in the 1st time
		{
			uint8_t * bits = NULL;
			uint32_t size = GetCharGlyph(bits, text[i]);

			dpl.Write(0xA2, (uint16_t)text[i]);			// Glyph index
			dpl.Write(0xA3, (uint32_t)size);			// Size of bm glyph in bytes
			dpl.Write(0x53);
			dpl.Write(bits, size);						// Actual bm glyph data
		}
	}
#endif

	dpl.Write(0x54);
	dpl.Write(0xB1, 0.0f);
	dpl.Write(0x7D);
	dpl.Write(0xA5, 0.0f, 0.0f);
	dpl.Write(0x66);
	dpl.Write(0xA4, 1.0f, 1.0f);
	dpl.Write(0x65);
	dpl.Write(0xAC, "\x00", 1);
	dpl.Write(0x81);
	dpl.Write(0xAD, (uint8_t)0);
	dpl.Write(0x56);
	dpl.Write(0xA8, fontName, 42);
	dpl.Write(0xA6, 83.0f);								// point size * ???
	dpl.Write(0xAA, (uint16_t)309);						// 590 on others... ???
	dpl.Write(0x6F);
	dpl.Write(0x4C, (uint16_t)353, 600);				// Location on page x/y
	dpl.Write(0x6B);
	dpl.Write(0xA1, (uint16_t)0);
	dpl.Write(0x64);
#if 0
//	WriteWBuffer(array, counter, 0xAB, "\x00\x00", 1);	// wchar text string
	WriteWBuffer(array, counter, 0xAB, "\x41\x00", 1);	// wchar text string (glyph index, actually)
	WriteWBuffer(array, counter, 0xAF, "\x00\x00", 1);	// wchar ??? string
#else
	uint8_t buf[1024], buf2[1024];
	for(uint32_t i=0; i<len; i++)
	{
		buf[(i * 2) + 0] = text[i];
		buf[(i * 2) + 1] = 0x00;
		buf2[(i * 2) + 0] = chWidth[text[i]] & 0xFF;
		buf2[(i * 2) + 1] = chWidth[text[i]] >> 8;
	}
	dpl.WriteW(0xAB, buf, len);							// wchar text string (glyph index, actually)
	dpl.WriteW(0xAF, buf2, len);						// wchar ADVANCE WIDTHs string
#endif
	dpl.Write(0xA8);
	dpl.Write(0xA8, fontName, 42);
	dpl.Write(0x55);


	dpl.Write(0x44);									// End of page D

	dpl.Write(0x49);									// End of job ("IB")
	dpl.Write(0x42);

	dpl.Append(escape, 9);
	dpl.Append(eoj, 9);
}

void WritePayloadToSocket(int clientSocket, const void * payload, uint32_t count)
{
	printf("Writing payload to server...\n");

//	int num = write(clientSocket, (void *)((char *)(buffer + bytesWritten)), bytesToSend - bytesWritten);
	int num = write(clientSocket, (void *)payload, count);

	if (num < 0)
	{
		perror("write()");

//		if (errno != EINTR)
//		{
//			exit(1);
//		}
	}
	else
		printf("Wrote %u (out of %u) bytes to the server.\n", num, count);
}


#define DEBUG

int main(int argc, char *argv[])
{
#ifdef DEBUG
	printf("DEBUG: Length of preamble: %u bytes.\n", strlen((const char *)preamble));

	BufferLoader bl;
	bl.Allocate(100);
	bl += (uint8_t)0x9E;
	bl += (uint16_t)0xBEEF;
	bl += (uint32_t)0xCAFEBABE;
	bl += 1.0f;
#endif

	if (argc < 2)
	{
		fprintf(stderr, "Dell Printer Driver v2\nby James L. Hammons\n");
		fprintf(stderr, "Usage: %s t <\"text string\">\n", argv[0]);
		exit(1);
	}

	InitFreeType();
	PrepCharArray();

#if 1
	DPL dpl;
	dpl.Allocate(2000000);
	Test3a(dpl, (argc < 3 ? "YHVH 99.9% Teghi" : argv[2]));
	printf("DPL size: %u\n", dpl.Size());
#endif

//fprintf(stderr, "Second argument string=\"%s\"\n", argv[1]);

	printf("Creating client socket...\n");

	int clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (-1 == clientSocket)
	{
		herror("socket()");
		exit(1);
	}

	// need to resolve the remote server name or IP address
	printf("Resolving server...\n");
	char remoteHost[] = "192.168.0.10";
	int remotePort = 9100;

	hostent * hostPtr = gethostbyname(remoteHost);

	if (NULL == hostPtr)
	{
		// This doesn't work.
		hostPtr = gethostbyaddr(remoteHost, strlen(remoteHost), AF_INET);

		if (NULL == hostPtr)
		{
			herror("Error resolving server address");
			exit(1);
		}
	}

	printf("Connecting to server...\n");

	sockaddr_in serverName = { 0 };

	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(remotePort);
	memcpy(&serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length);

	int status = connect(clientSocket, (sockaddr *)&serverName, sizeof(serverName));

	if (-1 == status)
	{
		herror("connect()");
		exit(1);
	}

	// Write payload to printer here...

	uint8_t * payload = new uint8_t[2000000];			// 2 Meg size
	uint32_t count = 0;
	Test3(payload, count, (argc < 3 ? "YHVH 99.9% Teghi" : argv[2]));

	if (argv[1][0] == 't')
//		WritePayloadToSocket(clientSocket, payload, count);
		WritePayloadToSocket(clientSocket, dpl.GetBuffer(), dpl.Size());

	close(clientSocket);

	delete[] payload;
	FreeCharArray();

	return 0;
}
