//
// page.h
//
// by James Hammons
// (C) 2009 Underground Software
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the
// Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef _PAGE_H_
#define _PAGE_H_

#include <stddef.h>
#include <stdint.h>

class Page
{
	protected:
		unsigned long pageNum;
		unsigned long copies;
		uint16_t width, height;
		uint8_t * bitmap;

	public:
		Page(uint16_t w, uint16_t h);
		virtual ~Page();

	public:
		void SetPageNum(unsigned long);
		void SetCopies(unsigned long);
		uint8_t * GetBitmap(void);
		unsigned long PageNum() const;
		unsigned long Copies() const;
		bool Render(bool lastPage = false);

	private:
		uint32_t EncodeBitmap(uint8_t * outBuf, const uint8_t * bitmap, uint32_t width, uint32_t height);
		uint32_t EncodeScanline(uint8_t * outBuf, const uint8_t * scanline, uint32_t width);
		uint32_t EncodeCompressedScanline(uint8_t * outBuf, const uint8_t * scanline, uint32_t width);
		uint32_t NumBytesEqual(const uint8_t * buf1, const uint8_t * buf2, uint32_t bytes);
		uint32_t NumBytesUnequal(const uint8_t * buf1, const uint8_t * buf2, uint32_t bytes);

	protected:
		uint8_t lastScanline[0x10000];			// 64K scanline buffer
};

#endif	// _PAGE_H_
