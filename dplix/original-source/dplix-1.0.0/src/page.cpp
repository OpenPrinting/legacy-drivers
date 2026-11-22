//
// page.cpp
//
// by James Hammons
// (C) 2009 Underground Software
//
// This is the internal page representation. Rendering to the printer using DPL
// happens here as well.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 or later of the License.
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  04/09/2009  Modified this file, added DPL specific bits
//

#include "page.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "dpl.h"
#include "job.h"
#include "errlog.h"

//
// This magic formula reverse the bit of a byte. ie. the bit 1 becomes the
// bit 8, the bit 2 becomes the bit 7 etc.
//
// This is genius--a purely mathematical way of reversing bits!
// This is so cool that I'm leaving in here even though I'm not using it...!
#define REVERSE_BITS(N) ((N * 0x0202020202ULL & 0x010884422010ULL) % 1023)

//
// Constructor/Destructor
//
Page::Page(uint16_t w, uint16_t h):
	width(w), height(h), bitmap(NULL)
{
	// We assume RGB format at present...
	bitmap = new uint8_t[width * height * 3];
}

Page::~Page()
{
	if (bitmap)
		delete bitmap;
}


void Page::SetPageNum(unsigned long n)
{
	pageNum = n;
}

void Page::SetCopies(unsigned long n)
{
	copies = n;
}

uint8_t * Page::GetBitmap(void)
{
	return bitmap;
}

unsigned long Page::PageNum() const
{
	return pageNum;
}

// This really should be elsewhere...
unsigned long Page::Copies() const
{
	return copies;
}

//
// NOTE: This is really ugly at the moment, but it works.
//
bool Page::Render(bool lastPage/*= false*/)
{
	DEBUGMSG(_("Attempting to write DPL values to printer..."));

	try
	{
		// This seems to be common page header for all pages (physical page header)...

		DPL::Write(0x28, (uint8_t)0);
		DPL::Write(0x26, (uint8_t)2);
		DPL::Write(0x25, (uint8_t)0);
		DPL::Write(0x43);
		DPL::Write(0x61);

		// This seems to common logical page header
//b/w, rot.: $74 [(w)96,6503->$2A] $75 [(w)90->$29] $76 [(f)1.0,1.0->$2B] $77
		DPL::Write(0x74);
		DPL::Write(0x2A, (uint16_t)96, 96);		// Position of log. pg. on phys. pg.
		DPL::Write(0x75);
//		DPL::Write(0x29, (uint16_t)0);			// Degrees to rotate page
		DPL::Write(0x29, Job::Rotation());		// Degrees to rotate page
		DPL::Write(0x76);
		DPL::Write(0x2B, 1.0f, 1.0f);			// Logical page scale x/y
		DPL::Write(0x77);

		DPL::Write(0x2C, (uint8_t)204);			// Signal bitmap is coming
		DPL::Write(0x7B);
		DPL::Write(0x2D, (uint8_t)0);
		DPL::Write(0x78);
		DPL::Write(0x2D, (uint8_t)0);
		DPL::Write(0x7C);
		DPL::Write(0x69);

#if 0
//Can be byte or word? Seems so.
		DPL::Write(0x4C, (uint8_t)0, 0);		// Location on page x/y
		DPL::Write(0x6B);
		DPL::Write(0x03, (uint8_t)2);			// Type of bitmap? (palette data, if any)
		DPL::Write(0x6A);
		DPL::Write(0x64, (uint8_t)0);			// 0=RGB, 1=paletted
		DPL::Write(0x62, (uint8_t)2);			// 0=2 color, 1=16 color, 2=256 color
		DPL::Write(0x6C, width);				// Bitmap width
		DPL::Write(0x6B, height);				// Bitmap height
		DPL::Write(0x67, width, height);		// Phys. width/height, in DPI

		DPL::Write(0xB0);
		DPL::Write(0x6D, (uint16_t)0);			// Beginning scanline of chunk
		DPL::Write(0x63, height);				// # of scanlines in chunk
		DPL::Write(0x65, (uint8_t)3);			// Compression type (0=none, 2=JPEG, 3=DPL)
		DPL::Write(0xB1);						// Start of data chunk
DEBUGMSG(_("Attempting to allocate 128M..."));
		uint8_t * data = new uint8_t[128000000];	// 128 megs of data...
DEBUGMSG(_("Attempting to EncodeBitmap... [width=%u, height=%u]"), width, height);
		uint32_t size = EncodeBitmap(data, bitmap, width, height);
DEBUGMSG(_("Encoded bitmap size: %u; nominal size: %u"), size, width*height*3);
DEBUGMSG(_("Attempting to Write data to printer..."));
		DPL::Write(data, size);
		delete[] data;
		DPL::Write(0xB2);						// End of data chunk
#else
/*
[(w)658,1966->$4C] $6B
[(b)0->$64] [(b)2->$62] [(w)3482->$6C] [(w)68->$6B] [(w)3482,68->$67]
$B0 [(w)0->$6D]  [(w)33->$63] [(b)3->$65] $B1 [$FA (32b) data chunk:21156]
    [(w)33->$6D] [(w)33->$63] [(b)3->$65] $B1 [$FA (32b) data chunk:6356]
    [(w)66->$6D] [(w)2->$63]  [(b)3->$65] $B1 [$FA (32b) data chunk:420] $B2
*/

/* should probably put stuff like this in a subroutine... */

		uint8_t * data = new uint8_t[16000000];	// 16 megs of data...
		for(uint16_t i=0; i<height; i+=25)	// 25 scanlines is totally arbitrary
		{
			uint16_t slines = ((height - i) < 25 ? height - i : 25);
		DPL::Write(0x4C, (uint16_t)0, i);		// Location on page x/y
		DPL::Write(0x6B);
		DPL::Write(0x64, (uint8_t)0);			// 0=RGB, 1=paletted
		DPL::Write(0x62, (uint8_t)2);			// 0=2 color, 1=16 color, 2=256 color
		DPL::Write(0x6C, width);				// Bitmap width
		DPL::Write(0x6B, slines);				// Bitmap height
		DPL::Write(0x67, width, slines);		// Phys. width/height, in DPI

		DPL::Write(0xB0);
			DPL::Write(0x6D, (uint16_t)0);		// Beginning scanline of chunk
			DPL::Write(0x63, slines);			// # of scanlines in chunk
			DPL::Write(0x65, (uint8_t)3);		// Compression type (0=none, 2=JPEG, 3=DPL)
			DPL::Write(0xB1);					// Start of data chunk
//DEBUGMSG(_("Attempting to EncodeBitmap... [width=%u, height=%u]"), width, height);
DEBUGMSG(_("Attempting to EncodeBitmap... [width=%u, height=%u]"), width, slines);
			uint32_t size = EncodeBitmap(data, &bitmap[i*width*3], width, slines);
DEBUGMSG(_("Encoded bitmap size: %u; nominal size: %u"), size, width*slines*3);
DEBUGMSG(_("Attempting to Write data to printer..."));
			DPL::Write(data, size);
		DPL::Write(0xB2);						// End of data chunk
		}
		delete[] data;
#endif

		DPL::Write(0x44);						// End of physical page ("D")
	}
	catch (DPL::Error e)
	{
		// Oops, there was a file write error (we assume...)
		return false;
	}

	return true;
}

uint32_t Page::EncodeBitmap(uint8_t * outBuf, const uint8_t * bitmap, uint32_t width, uint32_t height)
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
	byte is encountered. Skip values are skipped before values are written into the
    printer's internal scanline buffer.

	Basically a scanline buffer is re-used and only changes are encoded (by the driver).
	*/

	// We don't compress the first line so we get a valid scanline buffer on subsequent calls
	uint32_t totalBytes = EncodeScanline(outBuf, bitmap, width * 3);

	for(uint32_t y=1; y<height; y++)
	{
		uint32_t size = EncodeCompressedScanline(&outBuf[totalBytes], &bitmap[y * (width * 3)], width * 3);
		totalBytes += size;
	}

	return totalBytes;
}

//
// This encodes an uncompressed scanline by ignoring skip values.
//
uint32_t Page::EncodeScanline(uint8_t * outBuf, const uint8_t * scanline, uint32_t width)
{
	uint32_t count = 2;							// Size of the encoded buffer
	uint32_t slPtr = 0;							// Scanline pointer
	uint32_t bytesToEncode = width;				// Number of bytes to encode

	do
	{
		// Can only encode 8 bytes max per compression segment
		uint8_t numBytesInSegment = (bytesToEncode < 8 ? bytesToEncode : 8);

		// Encode # bytes/skip value (zero, in this case)
		outBuf[count++] = (numBytesInSegment - 1) << 5;

		// Stuff in the actual scanline bytes
		for(uint8_t i=0; i<numBytesInSegment; i++)
			outBuf[count++] = scanline[slPtr++];

		bytesToEncode -= numBytesInSegment;
	}
	while (bytesToEncode);

	// Set the count in the output buffer...
	// (we subtract out 2 because we don't add the 'byte count' bytes to the total)
	outBuf[0] = (count - 2) & 0xFF;
	outBuf[1] = ((count - 2) >> 8) & 0xFF;

	// Copy current scanline to last scanline buffer (used by EncodeCompressedScanline)
	memcpy(lastScanline, scanline, width);

	return count;
}

uint32_t Page::EncodeCompressedScanline(uint8_t * outBuf, const uint8_t * scanline, uint32_t width)
{
	/*
	To compress the page, we need to (1) compare to the previous scanline, and (2)
	encode skip values.

	Algorithm:
	[NOTE: skips come first, so check those first, then != values]
		# bytes different = # bytes identical = 0
		if 1st bytes are !=, then count # of bytes that are different.
		go through current line, count # of bytes that are identical to prev scanline
		encode line:
			encode different bytes
			encode skip value(s)
		repeat until line is scanned.
		if # bytes identical == width, then set count = 0 and return size == 2.

		also, if skip value reaches the end, encode a zero skip length and exit
	*/

	// Check for the trivial case of this scanline being equal to the previous
	if (NumBytesEqual(scanline, lastScanline, width) == width)
	{
		// Zero length line tells the printer to reuse previous scanline
		outBuf[0] = outBuf[1] = 0x00;
		return 2;
	}

	// Otherwise, we have to encode the scanline by comparing this current line
	// to the previous scanline.
	uint32_t count = 2;
	uint32_t bytesLeft = width;
	uint32_t bufPtr = 0;
	uint32_t slPtr = 0;

	while (bytesLeft)
	{
		uint32_t bytesToSkip = NumBytesEqual(&scanline[bufPtr], &lastScanline[bufPtr], bytesLeft);

		// If the rest of the line is the same, we don't encode anything
		if (bytesToSkip == bytesLeft)
			break;

		uint32_t bytesToEncode = NumBytesUnequal(&scanline[bufPtr + bytesToSkip], &lastScanline[bufPtr + bytesToSkip], bytesLeft - bytesToSkip);

		uint32_t bytesProcessed = bytesToSkip + bytesToEncode;
		uint8_t skipValue = (bytesToSkip >= 0x1F ? 0x1F : bytesToSkip);
		slPtr += bytesToSkip;

		do
		{
			// Encode in 8 byte (maximum) chunks
			uint8_t bytes = (bytesToEncode < 8 ? bytesToEncode : 8);

			// Encode # bytes/skip value
			outBuf[count++] = ((bytes - 1) << 5) | skipValue;

			if (skipValue)
			{
				int32_t i = (int32_t)bytesToSkip - 0x1F;

				while (i >= 0)
				{
					outBuf[count++] = (i >= 0xFF ? 0xFF : (uint8_t)i);
					i -= 0xFF;
				}

				skipValue = 0x00;
			}

			for(uint8_t i=0; i<bytes; i++)
				outBuf[count++] = scanline[slPtr++];

			bytesToEncode -= bytes;
		}
		while (bytesToEncode);

		bytesLeft -= bytesProcessed;
		bufPtr += bytesProcessed;
	}

	// Set count in output buffer...
	// (we subtract out 2 because we don't add the byte count bytes to the total)
	outBuf[0] = (count - 2) & 0xFF;
	outBuf[1] = ((count - 2) >> 8) & 0xFF;

	// Copy current scanline to scanline buffer
	memcpy(lastScanline, scanline, width);

	return count;
}

//
// Compares two buffers, returns the # bytes that are equal
//
uint32_t Page::NumBytesEqual(const uint8_t * buf1, const uint8_t * buf2, uint32_t bytes)
{
	for(uint32_t i=0; i<bytes; i++)
		if (buf1[i] != buf2[i])
			return i;

	return bytes;
}

//
// Compares two buffers, returns the # bytes that are unequal
//
uint32_t Page::NumBytesUnequal(const uint8_t * buf1, const uint8_t * buf2, uint32_t bytes)
{
	for(uint32_t i=0; i<bytes; i++)
		if (buf1[i] == buf2[i])
			return i;

	return bytes;
}
