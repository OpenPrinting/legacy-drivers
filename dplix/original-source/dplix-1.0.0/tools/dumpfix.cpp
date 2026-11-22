//
// TCPDump cleaner
// by James L. Hammons
// (C) 2008 Underground Software
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdint.h>
#include <string.h>

uint32_t GetByte(std::fstream &fs)
{
	char c;
	fs.get(c);

	return (uint32_t)((unsigned char)c);
}

uint32_t GetWord(std::fstream &fs)
{
	char c1, c2;
	fs.get(c1);
	fs.get(c2);
	uint8_t cl = (uint8_t)c1, ch = (uint8_t)c2;

	return (uint32_t)cl | (((uint32_t)ch) << 8);
}

uint32_t GetDWord(std::fstream &fs)
{
	char c1, c2, c3, c4;
	fs.get(c1);
	fs.get(c2);
	fs.get(c3);
	fs.get(c4);
	uint8_t cll = (uint8_t)c1, clh = (uint8_t)c2, chl = (uint8_t)c3, chh = (uint8_t)c4;

	return (uint32_t)cll | (((uint32_t)clh) << 8)
		| (((uint32_t)chl) << 16) | (((uint32_t)chh) << 24);
}

void GetLine(std::fstream &fs, char * str)
{
	int count = 0;
	char c;
	do
	{
		fs.get(c);
		str[count++] = c;
	}
	while (c != 0x0A);

	str[count - 1] = 0;
}

float ConvertHexToFloat(uint32_t hexVal)
{
	float * pFlt = (float *)&hexVal;
	return *pFlt;
}

int main(int argc, char * argv[])
{
//uint32_t v;
//v = 0x3F800000; printf("%08X -> %f\n", v, ConvertHexToFloat(v));
//v = 0x44160000; printf("%08X -> %f\n", v, ConvertHexToFloat(v));

	using namespace std;

	if (argc != 2)
	{
		cout << "dumpfix" << endl;
		cout << "Usage: dumpfix <filename>" << endl;
		return -1;
	}

	fstream ff;
	ff.open(argv[1], ios::in|ios::binary);

	if (!ff)
	{
		cout << "Unable to open file '" << argv[1] << "'!" << endl;
		return -1;
	}

	char ch;
	for(uint32_t i=0; i<24; i++)
		ff.get(ch);							// Skip tcpdump header

	while (!ff.eof())
	{
		GetDWord(ff);
if (ff.eof())
	break;//bleh
		GetDWord(ff);
		GetDWord(ff);
		uint32_t length = GetDWord(ff);

		if (length < 0x36)
		{
			for(uint32_t i=0; i<length; i++)
				ff.get(ch);
		}
		else
		{
			for(uint32_t i=0; i<0x36; i++)
				ff.get(ch);							// Skip junk

			length -= 0x36;

			for(uint32_t i=0; i<length; i++)
			{
				ff.get(ch);
				cout << ch;
			}
		}
	}

#if 0
	char buffer[100000];
	bool finished = false;

	while (!finished)
	{
		GetLine(ff, buffer);

		if (strcmp(buffer, ") HP-PCL XL;2;1;FXXPL2-1.0.0,WINNT-5.1,XPL2DRV-1.0.6.0") == 0)
			finished = true;
	}

	char ch1, ch2, outchar;
	while (!ff.eof())
	{
		ff.get(ch1);

//		if ((unsigned char)ch1 == 0xC8)
		if ((ch1 & 0xF8) == 0xC8)		// $C8-F
		{
			ff.get(ch2);
			if ((ch2 & 0xF0) != 0xC0)	// $C1-5 are also possible...
			{
				printf("\n$C8 failed: got $%02X instead of $Cx!\n", (unsigned char)ch2);
				break;
			}

			uint32_t result;

			if ((unsigned char)ch2 == 0xC0)
				result = GetByte(ff);
			else if ((unsigned char)ch2 == 0xC1)
				result = GetWord(ff);
			else
			{
				printf("\n$C8: $%02X: Unhandled case!\n", (unsigned char)ch2);
				break;
			}

			for(uint32_t i=0; i<result; i++)
			{
				if (ch1 & 0x01)
					GetWord(ff);
				else
					GetByte(ff);
			}

			ff.get(ch2);
			if ((unsigned char)ch2 != 0xF8)
			{
				printf("\n$C8 failed: Got $%02X instead of $F8!\n", (unsigned char)ch2);
				break;
			}

			ff.get(ch2);
//pre-emptive strike!
if ((unsigned char)ch2 != 0x81)
			printf("[Var-len%s: %u; $%02X]", (ch1 & 0x01 ? ".w" : ""), result, (unsigned char)ch2);
		}
// c0 -> 1100 (1 val)
// d0 -> 1101 (2 vals)
// e0 -> 1110 (4 vals)
		else if (((ch1 & 0xC0) == 0xC0) && ((ch1 & 0x30) != 0x30))
		{
char type[16][3] = { "b", "w", "dw", "w", "?", "f", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?" };
//			printf("[$%02X:", (unsigned char)ch1);
			printf("[(%s)", type[ch1 & 0x0F]);

			uint32_t result;
			int count = ((ch1 & 0x10) >> 4) + 1;
			count += ((ch1 & 0x20) ? 3 : 0);		// $Ex handling

			for(int i=0; i<count; i++)
			{
				if ((ch1 & 0x0F) == 0x00)
				{
					result = GetByte(ff);
					printf("%u", result);
				}
				else if (((ch1 & 0x0F) == 0x01) || ((ch1 & 0x0F) == 0x03))
				{
					result = GetWord(ff);
					printf("%u", result);
				}
				else if ((ch1 & 0x0F) == 0x02)
				{
					result = GetDWord(ff);
					printf("%u", result);
				}
				else if ((ch1 & 0x0F) == 0x05)
				{
					result = GetDWord(ff);
					printf("%f", ConvertHexToFloat(result));
				}
				else
				{
					printf("\n$%02X: Unhandled case!\n", (unsigned char)ch1);
					break;
				}

				if ((i == 0 && count == 2) || (i != 3 && count == 4))
					printf(",");
			}

			ff.get(ch2);
			if ((unsigned char)ch2 != 0xF8)
			{
				printf("\nFailed: Got $%02X instead of $F8!\n", (unsigned char)ch2);
				break;
			}

			ff.get(ch2);
            printf("->$%02X]", (unsigned char)ch2);
		}
		else if ((unsigned char)ch1 == 0xFA)
		{
			uint32_t count = GetDWord(ff);
			for(uint32_t i=0; i<count; i++)
				ff.get(ch2);
			printf("[$FA (32b) data chunk:%u]", count);
		}
		else if ((unsigned char)ch1 == 0xFB)
		{
			uint32_t count = GetByte(ff);
			for(uint32_t i=0; i<count; i++)
				ff.get(ch2);
			printf("[$FB (8b) data chunk:%u]", count);
		}
		else if ((unsigned char)ch1 == 0x1B)
		{
			printf("[ESC]\n");
			break;
		}
		else
		{
//Remove extraneous crap
if (!(((unsigned char)ch2 == 0x81) && ((unsigned char)ch1 == 0x47)))
			printf("$%02X ", (unsigned char)ch1);
		}
	}

//	printf("\n");
#endif
	ff.close();

	return 1;
}
