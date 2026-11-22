//
// dpl.h
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

#ifndef _DPL_H_
#define _DPL_H_

#include <stdint.h>

class DPL
{
	public:
		// Exception constants
		enum Error { WriteError };

		// Constructor/destructor
		DPL();
		~DPL();

		// Class methods
		static void Write(uint8_t byte);
		static void Write(uint8_t reg, uint8_t byte);
		static void Write(uint8_t reg, uint8_t byte1, uint8_t byte2);
		static void Write(uint8_t reg, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);
		static void Write(uint8_t reg, uint16_t word);
		static void Write(uint8_t reg, uint16_t word1, uint16_t word2);
		static void Write(uint8_t reg, uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4);
		static void Write(uint8_t reg, uint32_t dword);
		static void Write(uint8_t reg, uint32_t dword1, uint32_t dword2);
		static void Write(uint8_t reg, uint32_t dword1, uint32_t dword2, uint32_t dword3, uint32_t dword4);
		static void Write(uint8_t reg, float flt);
		static void Write(uint8_t reg, float flt1, float flt2);
		static void Write(const uint8_t * data, uint32_t length);
		static void Write(uint8_t reg, const void * buffer, uint32_t size);
		static void WriteW(uint8_t reg, const void * buffer, uint32_t size);

	private:
		static void WriteBuffer(const uint8_t * buf, uint32_t size);
		static void Write2(uint16_t word);
		static void Write4(uint32_t dword);
		static void WriteF(float f);
};

#endif	// _DPL_H_
