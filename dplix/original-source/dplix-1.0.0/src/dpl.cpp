//
// dpl.cpp
//
// by James Hammons
// (C) 2009 Underground Software
//
// This writes data to the printer in the format it expects.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 or later of the License.
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  04/01/2009  Created this file
// JLH  04/02/2009  Modified all methods to be class methods, write directly to
//                  stdout
//

#include "dpl.h"
#include <stdio.h>

DPL::DPL(void)
{
}

DPL::~DPL(void)
{
}

//
// The reason we don't overload the single argument version of Write() is that it
// would cause a whole bunch of unnecessary (uint8_t) casting. It can be done, but
// seems to me to be more trouble than it's worth. :-P
//
void DPL::Write(uint8_t byte)
{
	WriteBuffer(&byte, 1);
}

void DPL::Write(uint8_t reg, uint8_t byte)
{
	Write((uint8_t)0xC0);
	Write(byte);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, uint8_t byte1, uint8_t byte2)
{
	Write((uint8_t)0xD0);
	Write(byte1);
	Write(byte2);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4)
{
	Write((uint8_t)0xE0);
	Write(byte1);
	Write(byte2);
	Write(byte3);
	Write(byte4);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, uint16_t word)
{
	Write((uint8_t)0xC1);						// Can also be $C3, but when???
	Write2(word);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, uint16_t word1, uint16_t word2)
{
	Write((uint8_t)0xD1);						// Can also be $D3, but when???
	Write2(word1);
	Write2(word2);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4)
{
	Write((uint8_t)0xE1);						// Can also be $E3, but when???
	Write2(word1);
	Write2(word2);
	Write2(word3);
	Write2(word4);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, uint32_t dword)
{
	Write((uint8_t)0xC2);
	Write4(dword);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, uint32_t dword1, uint32_t dword2)
{
	Write((uint8_t)0xD2);
	Write4(dword1);
	Write4(dword2);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, uint32_t dword1, uint32_t dword2, uint32_t dword3, uint32_t dword4)
{
	Write((uint8_t)0xE2);
	Write4(dword1);
	Write4(dword2);
	Write4(dword3);
	Write4(dword4);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, float flt)
{
	Write((uint8_t)0xC5);
	WriteF(flt);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(uint8_t reg, float flt1, float flt2)
{
	Write((uint8_t)0xD5);
	WriteF(flt1);
	WriteF(flt2);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::Write(const uint8_t * data, uint32_t length)
{
	Write((uint8_t)(length < 256 ? 0xFB : 0xFA));

	if (length < 256)
		Write((uint8_t)length);
	else
		Write4(length);

	WriteBuffer(data, length);
}

void DPL::Write(uint8_t reg, const void * buffer, uint32_t size)
{
	Write((uint8_t)0xC8);
	Write((uint8_t)(size < 256 ? 0xC0 : 0xC1));	// Could be bigger as well...

	if (size < 256)
		Write((uint8_t)size);
	else
		Write2((uint16_t)size);

	WriteBuffer((const uint8_t *)buffer, size);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::WriteW(uint8_t reg, const void * buffer, uint32_t size)
{
	Write((uint8_t)0xC9);
	Write((uint8_t)(size < 256 ? 0xC0 : 0xC1));	// Could be bigger as well...

	if (size < 256)
		Write((uint8_t)size);
	else
		Write2((uint16_t)size);

	WriteBuffer((const uint8_t *)buffer, size * 2);
	Write((uint8_t)0xF8);
	Write(reg);
}

void DPL::WriteBuffer(const uint8_t * buf, uint32_t size)
{
	size_t bytesWritten = fwrite(buf, 1, size, stdout);

	if (bytesWritten != size)
		throw WriteError;
}

void DPL::Write2(uint16_t word)
{
	uint8_t buf[2];
	buf[0] = (word & 0xFF);
	buf[1] = (word >> 8);
	WriteBuffer(buf, 2);
}

void DPL::Write4(uint32_t dword)
{
	uint8_t buf[4];

	for(int i=0; i<4; i++)
	{
		buf[i] = (dword & 0xFF);
		dword >>= 8;
	}

	WriteBuffer(buf, 4);
}

void DPL::WriteF(float f)
{
	// NOTE: This is little endian specific. !!! FIX !!!
	uint8_t * pFlt = (uint8_t *)&f;
	uint8_t buf[4];

	buf[0] = pFlt[0];
	buf[1] = pFlt[1];
	buf[2] = pFlt[2];
	buf[3] = pFlt[3];

	WriteBuffer(buf, 4);
}
