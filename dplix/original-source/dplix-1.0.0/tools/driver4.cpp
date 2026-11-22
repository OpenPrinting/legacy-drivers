//
// Driver mkIV
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

// Let's try images...
#include "SDL_image.h"

// Function prototypes...
uint32_t EncodeBitmap(uint8_t * outBuf, const uint8_t * bitmap, uint32_t width, uint32_t height);
uint32_t EncodeRawBitmap(uint8_t * outBuf, const uint8_t * bitmap, uint32_t width, uint32_t height);

// Useful helper classes

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

class DiskFile
{
	public:
		DiskFile(const uint8_t * filename);
		~DiskFile();
		uint32_t Size(void);
		const uint8_t * GetBuffer(void);

	private:
		uint8_t * bits;
		uint32_t size;
};

DiskFile::DiskFile(const uint8_t * filename): bits(NULL), size(0)
{
	FILE * fp = fopen((const char *)filename, "rb");

	// Crappy error handling...
	if (NULL == fp)
		throw 1;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);

	bits = new uint8_t[size];

	if (fread(bits, 1, size, fp) != size)
	{
		delete[] bits;
		bits = NULL;
		// Crappy!!
		throw 2;
	}

	fclose(fp);
}

DiskFile::~DiskFile()
{
#if 1
printf("DiskFile::~DiskFile()...\n");
#endif
	if (bits)
		delete[] bits;
}

const uint8_t * DiskFile::GetBuffer(void)
{
	return bits;
}

uint32_t DiskFile::Size(void)
{
	return size;
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

//	error = FT_New_Face(library, "/usr/share/fonts/corefonts/arial.ttf", 0, &face);
	error = FT_New_Face(library, "/usr/share/fonts/corefonts/georgia.ttf", 0, &face);
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

void Test4(DPL &dpl, const void * text_in)
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
//	dpl.Write(0x0B, "\xFF\xFF\xFF", 3);
//	dpl.Write(0x63);
	dpl.Write(0x05, (uint8_t)0);
	dpl.Write(0x79);
	dpl.Write(0x46, (uint8_t)1);
	dpl.Write(0x6E);
//Not needed...
//	dpl.Write(0x42, (uint16_t)653, 503, 4253, 707);
//	dpl.Write(0xA0);
	dpl.Write(0x0B, "\x00\x00\x00", 3);					// Text color
//	dpl.Write(0x0B, "\xFF\x00\x7F", 3);					// Text color
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
//	dpl.Write(0xA2, (uint16_t)0);						// Glyph index
	dpl.Write(0xA2, (uint16_t)0x41);					// Glyph index
	dpl.Write(0xA3, (uint32_t)size);					// Size of bm glyph in bytes
	dpl.Write(0x53);
	dpl.Write(bits, size);								// Actual bm glyph data
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

	uint8_t buf[1024], buf2[1024];
	for(uint32_t i=0; i<len; i++)
	{
		// Note that these are just glyph indices--they could be anything you want
		buf[(i * 2) + 0] = text[i];
		buf[(i * 2) + 1] = 0x00;
		buf2[(i * 2) + 0] = chWidth[text[i]] & 0xFF;
		buf2[(i * 2) + 1] = chWidth[text[i]] >> 8;
	}
	dpl.WriteW(0xAB, buf, len);							// wchar text string (glyph index, actually)
	dpl.WriteW(0xAF, buf2, len);						// wchar ADVANCE WIDTHs string
	dpl.Write(0xA8);

#if 1
	// Yes!
	dpl.Write(0x0B, "\xFF\x7F\x00", 3);					// Text color
	dpl.Write(0x63);
	// Yes!
	dpl.Write(0x4C, (uint16_t)353, 750);				// Location on page x/y
	dpl.Write(0x6B);
	dpl.WriteW(0xAB, buf, len);							// wchar text string (glyph index, actually)
	dpl.WriteW(0xAF, buf2, len);						// wchar ADVANCE WIDTHs string
	dpl.Write(0xA8);

	// Yes!
	dpl.Write(0x0B, "\x00\x00\x00", 3);					// Text color
	dpl.Write(0x63);
	// Yes!
	dpl.Write(0x4C, (uint16_t)353, 900);				// Location on page x/y
	dpl.Write(0x6B);
	dpl.WriteW(0xAB, buf, len);							// wchar text string (glyph index, actually)
	dpl.WriteW(0xAF, buf2, len);						// wchar ADVANCE WIDTHs string
	dpl.Write(0xA8);
#endif

	dpl.Write(0xA8, fontName, 42);
	dpl.Write(0x55);


	dpl.Write(0x44);									// End of physical page ("D")

	dpl.Write(0x49);									// End of job ("IB")
	dpl.Write(0x42);

	dpl.Append(escape, 9);
	dpl.Append(eoj, 9);
}

void TestBitmap(DPL &dpl, const uint8_t * bitmap, uint16_t width, uint16_t height)
{
	dpl.Append(escape, 9);
	dpl.Append(preamble, strlen((const char *)preamble));

	// This seems to be common job header for all jobs...

	dpl.Write(0x89, (uint16_t)600, 600);				// DPI?
	dpl.Write(0x86, (uint8_t)0);
	dpl.Write(0x8F, (uint8_t)0);
	dpl.Write(0x41);
	dpl.Write(0x88, (uint8_t)0);
//1 for RGB as well??? no...? mebbe...
	dpl.Write(0x82, (uint8_t)1);						// Can be 1 too (b/w page)...
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

	dpl.Write(0x2C, (uint8_t)204);						// Signal bitmap
	dpl.Write(0x7B);
	dpl.Write(0x2D, (uint8_t)0);
	dpl.Write(0x78);
	dpl.Write(0x2D, (uint8_t)0);
	dpl.Write(0x7C);
	dpl.Write(0x69);

//bad line?YES	dpl.Write(0x03, (uint8_t)2);
//Can be byte or word? Seems so.
	dpl.Write(0x4C, (uint8_t)128, 203);					// Location on page x/y
	dpl.Write(0x6B);
	dpl.Write(0x03, (uint8_t)2);						// Type of bitmap?
	dpl.Write(0x6A);
	dpl.Write(0x64, (uint8_t)0);
	dpl.Write(0x62, (uint8_t)2);
	dpl.Write(0x6C, width);								// Bitmap width
	dpl.Write(0x6B, height);							// Bitmap height
	dpl.Write(0x67, width, height);						// Phys. width/height, in DPI

	dpl.Write(0xB0);
	dpl.Write(0x6D, (uint16_t)0);						// Beginning scanline of chunk
	dpl.Write(0x63, height);							// # of scanlines in chunk
	dpl.Write(0x65, (uint8_t)3);						// Always 3...
	dpl.Write(0xB1);									// Start of data chunk
	uint8_t data[4000000];	// 4 megs of data...
	uint32_t size = EncodeBitmap(data, bitmap, width, height);
printf("Encoded bitmap size: %u; nominal size: %u\n", size, width*height*3);
	dpl.Write(data, size);
	dpl.Write(0xB2);									// End of data chunk

	dpl.Write(0x44);									// End of physical page ("D")

	dpl.Write(0x49);									// End of job ("IB")
	dpl.Write(0x42);

	dpl.Append(escape, 9);
	dpl.Append(eoj, 9);
}

void TestBitmap2(DPL &dpl, const uint8_t * bitmap, uint16_t width, uint16_t height)
{
	dpl.Append(escape, 9);
	dpl.Append(preamble, strlen((const char *)preamble));

	// This seems to be common job header for all jobs...

	dpl.Write(0x89, (uint16_t)600, 600);				// DPI?
	dpl.Write(0x86, (uint8_t)0);
	dpl.Write(0x8F, (uint8_t)0);
	dpl.Write(0x41);
	dpl.Write(0x88, (uint8_t)0);
//1 for RGB as well??? no...? mebbe...
	dpl.Write(0x82, (uint8_t)1);						// Can be 1 too (b/w page)...
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

	dpl.Write(0x2C, (uint8_t)204);						// Signal bitmap
	dpl.Write(0x7B);
	dpl.Write(0x2D, (uint8_t)0);
	dpl.Write(0x78);
	dpl.Write(0x2D, (uint8_t)0);
	dpl.Write(0x7C);
	dpl.Write(0x69);

//Can be byte or word? Seems so.
	dpl.Write(0x4C, (uint8_t)0, 0);						// Location on page x/y
	dpl.Write(0x6B);
	dpl.Write(0x03, (uint8_t)2);						// Type of bitmap? (palette data, if any)
	dpl.Write(0x6A);
	dpl.Write(0x64, (uint8_t)0);						// 0=RGB, 1=paletted
	dpl.Write(0x62, (uint8_t)2);						// 0=2 color, 1=16 color, 2=256 color
	dpl.Write(0x6C, width);								// Bitmap width
	dpl.Write(0x6B, height);							// Bitmap height
	dpl.Write(0x67, width, height);						// Phys. width/height, in DPI

/*
  [(w)600,600->$89] [(b)0->$86] [(b)0->$8F] $41 [(b)0->$88] [(b)1->$82] $48
  [(b)0->$28] [(b)2->$26] [(b)0->$25] $43 $61
  $74 [(w)96,96->$2A] $75 [(w)0->$29] $76 [(f)1.0,1.0->$2B] $77

  [(b)204->$2C] $7B [(b)0->$2D] $78 [(b)0->$2D] $7C $69
  [(b)128,203->$4C] $6B [(b)2->$03] $6A [(b)0->$64] [(b)2->$62]

  [(w)387->$6C] [(w)504->$6B] [(w)1500,1950->$67]
$B0
 [(w)0->$6D]   [(w)300->$63] [(b)0->$65] $B1 [$FA (32b) data chunk:349200]
 [(w)300->$6D] [(w)204->$63] [(b)0->$65] $B1 [$FA (32b) data chunk:237456]
$B2 $44 $49 $42 [ESC]

NOTE: Even though the width of the bitmap is 387, the data chunk is 388x300 and 388x204
      respectively. So the length of the chunk is 388x300x3 = 349200. Zero RGB pixels
      are padded on the right side of the bitmap.
*/
	dpl.Write(0xB0);
#if 1
	dpl.Write(0x6D, (uint16_t)0);						// Beginning scanline of chunk
	dpl.Write(0x63, height);							// # of scanlines in chunk
	dpl.Write(0x65, (uint8_t)0);						// Compression type (0=none, 2=JPEG, 3=DPL)
	dpl.Write(0xB1);									// Start of data chunk
	uint8_t * data = new uint8_t[64000000];	// 64 megs of data...
	uint32_t size = EncodeRawBitmap(data, bitmap, width, height);
printf("Encoded bitmap size: %u; nominal size: %u\n", size, width*height*3);
	dpl.Write(data, size);
	// Need to do bitmap segmenting here...
//	dpl.Write(bitmap, width * height * 3);
	delete[] data;
#else
	// OK! data needs to be padded out to the nearest 16 byte boundary (X)
	// Also, it looks like it expects the bitmap to be rotated 90 degrees!
	for(uint16_t i=0; i<height; i+=50)	// 50 scanlines is totally arbitrary
	{
		uint16_t slines = ((height - i) < 50 ? height - i : 50);
		dpl.Write(0x6D, (uint16_t)i);					// Beginning scanline of chunk
		dpl.Write(0x63, slines);						// # of scanlines in chunk
		dpl.Write(0x65, (uint8_t)0);					// Compression type (0=none, 2=JPEG, 3=DPL)
		dpl.Write(0xB1);								// Start of data chunk
		dpl.Write(&bitmap[i*width*3], width * slines * 3);
	}
#endif
	dpl.Write(0xB2);									// End of data chunk

	dpl.Write(0x44);									// End of physical page ("D")

	dpl.Write(0x49);									// End of job ("IB")
	dpl.Write(0x42);

	dpl.Append(escape, 9);
	dpl.Append(eoj, 9);
}

void TestJPEG(DPL &dpl, const uint8_t * bitmap, uint32_t size, uint16_t width, uint16_t height)
{
	dpl.Append(escape, 9);
	dpl.Append(preamble, strlen((const char *)preamble));

	// This seems to be common job header for all jobs...

	dpl.Write(0x89, (uint16_t)600, 600);				// DPI?
	dpl.Write(0x86, (uint8_t)0);
	dpl.Write(0x8F, (uint8_t)0);
	dpl.Write(0x41);
	dpl.Write(0x88, (uint8_t)0);
//1 for RGB as well??? no...? mebbe...
	dpl.Write(0x82, (uint8_t)1);						// Can be 1 too (b/w page)...
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

	dpl.Write(0x2C, (uint8_t)204);						// Signal bitmap
	dpl.Write(0x7B);
	dpl.Write(0x2D, (uint8_t)0);
	dpl.Write(0x78);
	dpl.Write(0x2D, (uint8_t)0);
	dpl.Write(0x7C);
	dpl.Write(0x69);

//bad line?YES	dpl.Write(0x03, (uint8_t)2);
//Can be byte or word? Seems so.
	dpl.Write(0x4C, (uint8_t)128, 203);					// Location on page x/y
	dpl.Write(0x6B);
	dpl.Write(0x03, (uint8_t)2);						// Type of bitmap?
	dpl.Write(0x6A);
	dpl.Write(0x64, (uint8_t)0);
	dpl.Write(0x62, (uint8_t)2);
	dpl.Write(0x6C, width);								// Bitmap width
	dpl.Write(0x6B, height);							// Bitmap height
	dpl.Write(0x67, width, height);						// Phys. width/height, in DPI
	dpl.Write(0xB0);
	dpl.Write(0x6D, (uint16_t)0);						// Beginning scanline of chunk
	dpl.Write(0x63, height);							// # of scanlines in chunk
	dpl.Write(0x65, (uint8_t)2);						// Compression type (0=none, 2=JPEG, 3=DPL)
	dpl.Write(0x6E, (uint8_t)3);						// Signal JPEG?
	dpl.Write(0xB1);									// Start of data chunk

	dpl.Write(bitmap, size);							// Just slap the JPEG in there...
	dpl.Write(0xB2);									// End of data chunk

	dpl.Write(0x44);									// End of page ("D")

	dpl.Write(0x49);									// End of job ("IB")
	dpl.Write(0x42);

	dpl.Append(escape, 9);
	dpl.Append(eoj, 9);
}

uint32_t EncodeBitmap(uint8_t * outBuf, const uint8_t * bitmap, uint32_t width, uint32_t height)
{
	/*
	There are many different encodings for bitmaps and we'll document them eventually.
	For now, we use RGB mode and a simple minded encoding scheme.
	Basically, it encodes using scanlines like so:

	[countlo] [counthi] [encbyte] bytes...

	count is a 16-bit value so that means no RGB line can be longer 65535 bytes.
		If count = 0, then reuse the scanline buffer.
	encbyte is the encoding scheme. It breaks down like so:
	[nnns ssss] where nnn is the # of bytes to follow + 1 and sssss is # of bytes
	to skip in the buffer. If sssss = $1F, then the follow-on byte extends the skip
	value. If the follow-on is $FF, then there is another follow-on until a non-$FF
	byte is encountered.

	Basically a scanline buffer is re-used and only changes are encoded (by the driver).
	But since we don't care about that, we can do anything we want here. :-)
	Generally this will result in a much larger payload being generated, but we don't
	care too much. Yet. ;-)
	*/

	// Wasteful, but it works...
	uint32_t totalBytes = 0, bmPtr = 0;
	uint8_t buffer[0x10000];					// 64K buffer
	uint32_t pitch = width * 3;//QnD fix... !!! FIX !!!

	for(uint32_t y=0; y<height; y++)
	{
		uint16_t count = 0;

		uint32_t bytes = (width*3) / 8, remainder = (width*3) % 8;

		for(uint32_t x=0; x<bytes; x++)
		{
			buffer[count++] = 0xE0;

			for(uint32_t i=0; i<8; i++)
				buffer[count++] = bitmap[bmPtr + (x * 8) + i];
		}

		if (remainder)
		{
			buffer[count++] = (remainder - 1) << 5;

			for(uint32_t i=0; i<remainder; i++)
				buffer[count++] = bitmap[bmPtr + (bytes * 8) + i];
		}

		bmPtr += pitch;
		// export count and buffer...
		outBuf[totalBytes++] = count & 0xFF;
		outBuf[totalBytes++] = (count >> 8) & 0xFF;
		memcpy(&outBuf[totalBytes], buffer, count);
		totalBytes += count;
	}

	return totalBytes;
}

//This is obsolete! OR is it???
uint32_t EncodeRawBitmap(uint8_t * outBuf, const uint8_t * bitmap, uint32_t width, uint32_t height)
{
	/*
	It seems there's a 350KB limit on chunks sent through the data mechanism.
	Need to take that into account in TestBitmap2().

	Also, there seems to be some wonkiness with the SDL_image loading routines and PNGs...
	(Fixed...)

	May need to rotate the image 90 degrees as well...
	*/
#if 1
	uint32_t extra = 4 - (width % 4);
	uint32_t totalBytes;

	if (extra == 4)
	{
printf("ERB: No extra.\n");
		totalBytes = height * width * 3;
		memcpy(outBuf, bitmap, totalBytes);
	}
	else
	{
printf("ERB: %u extra bytes.\n", extra);
		totalBytes = 0;
		uint32_t bmPtr = 0;

		for(uint32_t y=0; y<height; y++)
		{
			memcpy(&outBuf[totalBytes], &bitmap[bmPtr], width * 3);
			totalBytes += (width + extra) * 3;
			bmPtr += width * 3;
		}
	}
#else
	uint32_t totalBytes = height * width * 3;
	memcpy(outBuf, bitmap, totalBytes);
#endif

#if 0
	FILE * fp = fopen("./raw-bits7.raw", "wb");

	if (NULL == fp)
	{
		perror("fopen()");
	}

	fwrite(outBuf, 1, totalBytes, fp);
	fclose(fp);
#endif

	return totalBytes;
}

uint8_t * FixBits(SDL_Surface * surface)
{
	// First, allocate memory for the bitmap...
	uint8_t * buffer = new uint8_t[surface->w * surface->h * 3];
	uint32_t bufPos = 0;
	SDL_PixelFormat * fmt = surface->format;

printf("FixBits: Pixel format: %u bpp, bytesPerPix=%u, palette=%08X\n", fmt->BitsPerPixel, fmt->BytesPerPixel, fmt->palette);

	// This only works for non-paletted images [DONE: now does both]...

	for(uint32_t y=0; y<surface->h; y++)
	{
		for(uint32_t x=0; x<surface->w; x++)
		{
			if (fmt->palette == NULL)
			{
				SDL_LockSurface(surface);
				uint8_t * p = (uint8_t *)surface->pixels + (y * surface->pitch) + (x * 4);
				uint32_t pixel = *((uint32_t *)p);
				SDL_UnlockSurface(surface);

				/* Get Red component */
				uint32_t temp = pixel & fmt->Rmask;  /* Isolate red component */
				temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
				temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
				uint8_t red = (uint8_t)temp;

				/* Get Green component */
				temp = pixel & fmt->Gmask;  /* Isolate green component */
				temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
				temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
				uint8_t green = (uint8_t)temp;

				/* Get Blue component */
				temp = pixel & fmt->Bmask;  /* Isolate blue component */
				temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
				temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
				uint8_t blue = (uint8_t)temp;

				buffer[bufPos++] = red;
				buffer[bufPos++] = green;
				buffer[bufPos++] = blue;
			}
			else
			{
				SDL_LockSurface(surface);
				uint8_t index = ((uint8_t *)surface->pixels)[(y * surface->pitch) + x];
				SDL_Color color = fmt->palette->colors[index];
				SDL_UnlockSurface(surface);

				buffer[bufPos++] = color.r;
				buffer[bufPos++] = color.g;
				buffer[bufPos++] = color.b;
			}
		}
	}

printf("FixBits: Size of buffer is %u bytes.\n", bufPos);

	return buffer;
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
#endif

	if (argc < 2)
	{
		fprintf(stderr, "Dell Printer Driver v4\nby James L. Hammons\n");
		fprintf(stderr, "Usage: %s t <\"text string\">\n", argv[0]);
		fprintf(stderr, "       %s i <image file>\n", argv[0]);
		fprintf(stderr, "       %s j <JPEG image file>\n", argv[0]);
		exit(1);
	}

	InitFreeType();
	PrepCharArray();

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
//		herror("connect()");
		perror("connect()");
		exit(1);
	}

	// Write payload to printer here...

	DPL dpl;
	dpl.Allocate(36000000);		// 36M

	if (argv[1][0] == 't')
	{
		Test4(dpl, (argc < 3 ? "YHVH 99.9% Teghi" : argv[2]));
		printf("DPL size: %u\n", dpl.Size());

//		WritePayloadToSocket(clientSocket, payload, count);
		WritePayloadToSocket(clientSocket, dpl.GetBuffer(), dpl.Size());
	}
	else if (argv[1][0] == 'i')
	{
		if (argc > 2)
		{
			SDL_Surface * image = IMG_Load(argv[2]);
			if (!image)
			{
				printf("Could not open image file \"%s\"!\n", argv[2]);
				close(clientSocket);
				FreeCharArray();
				return -1;
			}

			printf("Image size: %u x %u.\n", image->w, image->h);
			uint8_t * buffer = FixBits(image);
			TestBitmap2(dpl, (const uint8_t *)buffer, image->w, image->h);
			printf("DPL size: %u\n", dpl.Size());
			delete[] buffer;
			SDL_FreeSurface(image);

			WritePayloadToSocket(clientSocket, dpl.GetBuffer(), dpl.Size());
		}
	}
	else if (argv[1][0] == 'j')
	{
		if (argc > 2)
		{
			SDL_Surface * image = IMG_Load(argv[2]);
			if (!image)
			{
				printf("Could not open image file \"%s\"!\n", argv[2]);
				close(clientSocket);
				FreeCharArray();
				return -1;
			}

			// Need to do this in a try/catch block!
			try
			{
				DiskFile df((const uint8_t *)argv[2]);

printf("Image size: %u x %u.\n", image->w, image->h);
				TestJPEG(dpl, df.GetBuffer(), df.Size(), image->w, image->h);
				printf("DPL size: %u\n", dpl.Size());
				WritePayloadToSocket(clientSocket, dpl.GetBuffer(), dpl.Size());
			}
			catch (int e)
			{
				if (e == 1)
					printf("Could not open file \"%s\"!\n", argv[2]);
				else if (e == 2)
					printf("Could not read file \"%s\" into memory!\n", argv[2]);
				else
					printf("An unknown exception occurred!\n");
			}

			SDL_FreeSurface(image);
		}
	}

	close(clientSocket);
	FreeCharArray();

	return 0;
}
