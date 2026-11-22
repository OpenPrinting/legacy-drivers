//
// DPL File Parser
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
		cout << "DPL parser" << endl;
		cout << "Usage: parse <filename>" << endl;
		return -1;
	}

	fstream ff;
	ff.open(argv[1], ios::in|ios::binary);

	if (!ff)
	{
		cout << "Unable to open file '" << argv[1] << "'!" << endl;
		return -1;
	}

	char buffer[100000];
	bool finished = false;

	while (!finished)
	{
		GetLine(ff, buffer);

//printf("%s\n", buffer);
		if (strcmp(buffer, ") HP-PCL XL;2;1;FXXPL2-1.0.0,WINNT-5.1,XPL2DRV-1.0.6.0") == 0)
			finished = true;
	}
fprintf(stderr, "header found\n");

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

		fflush(stdout);
	}

//	printf("\n");
	ff.close();

	return 1;
}

/*
12345X@PJL COMMENT DATE=10/26/2008
@PJL COMMENT TIME=01:00:38
@PJL COMMENT DNAME=Wallet prints
@PJL JOB MODE=PRINTER
@PJL SET JOBATTR="@LUNA=User"
@PJL SET COPIES=1
@PJL SET QTY=1
@PJL SET JOBATTR="@TRCH=OFF"
@PJL SET DUPLEX=OFF
@PJL SET BINDING=LONGEDGE
@PJL SET JOBATTR="@SPSE=AUTO"
@PJL SET JOBATTR="@MSIP=NORMAL"
@PJL SET RENDERMODE=COLOR
@PJL SET ECONOMODE=OFF
@PJL SET JOBATTR="@IREC=OFF"
@PJL SET JOBATTR="@TRAP=OFF"
@PJL SET JOBATTR="@HOAD=I0A00020F"
@PJL SET JOBATTR="@JOAU=User"
@PJL SET JOBATTR="@CNAM=XPPRO-VM"
@PJL SET OUTBIN=FACEDOWN
@PJL SET JOBATTR="@PODR=NORMAL"
@PJL SET SLIPSHEET=OFF
@PJL SET PAPERDIRECTION=SEF
@PJL SET JOBATTR="@APSP=OFF"
@PJL SET RESOLUTION=300
@PJL SET BITSPERPIXEL=8
@PJL SET JOBATTR="@DRDM=QCIQUALITY"
@PJL SET JOBATTR="@TSCR=9"
@PJL SET JOBATTR="@GSCR=9"
@PJL SET JOBATTR="@ISCR=9"
@PJL SET JOBATTR="@TTRC=9"
@PJL SET JOBATTR="@GTRC=9"
@PJL SET JOBATTR="@ITRC=9"
@PJL SET JOBATTR="@TCPR=24"
@PJL SET JOBATTR="@GCPR=24"
@PJL SET JOBATTR="@ICPR=25"
@PJL SET JOBATTR="@TUCR=24"
@PJL SET JOBATTR="@GUCR=24"
@PJL SET JOBATTR="@IUCR=25"
@PJL SET JOBATTR="@TDFT=1"
@PJL SET JOBATTR="@GDFT=1"
@PJL SET JOBATTR="@IDFT=1"
@PJL SET JOBATTR="@BANR=DEVICE"
@PJL ENTER LANGUAGE=DPL
) HP-PCL XL;2;1;FXXPL2-1.0.0,WINNT-5.1,XPL2DRV-1.0.6.0

In hex:

1B 25 2D 31 32 33 34 35 58 40 50 4A 4C 20 43 4F 4D 4D 45 4E 54 20 44 41 54 45 3D 31 30 2F 32 36 2F 32 30 30 38 0A 40 50 4A 4C 20 43 4F 4D 4D 45 4E 54 20 54 49 4D 45 3D 30 31 3A 33 39 3A 32 37 0A 40 50 4A 4C 20 43 4F 4D 4D 45 4E 54 20 44 4E 41 4D 45 3D 44 6F 63 75 6D 65 6E 74 0A 40 50 4A 4C 20 4A 4F 42 20 4D 4F 44 45 3D 50 52 49 4E 54 45 52 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 4C 55 4E 41 3D 55 73 65 72 22 0A 40 50 4A 4C 20 53 45 54 20 43 4F 50 49 45 53 3D 31 0A 40 50 4A 4C 20 53 45 54 20 51 54 59 3D 31 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 54 52 43 48 3D 4F 46 46 22 0A 40 50 4A 4C 20 53 45 54 20 44 55 50 4C 45 58 3D 4F 46 46 0A 40 50 4A 4C 20 53 45 54 20 42 49 4E 44 49 4E 47 3D 4C 4F 4E 47 45 44 47 45 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 53 50 53 45 3D 41 55 54 4F 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 4D 53 49 50 3D 4E 4F 52 4D 41 4C 22 0A 40 50 4A 4C 20 53 45 54 20 52 45 4E 44 45 52 4D 4F 44 45 3D 43 4F 4C 4F 52 0A 40 50 4A 4C 20 53 45 54 20 45 43 4F 4E 4F 4D 4F 44 45 3D 4F 46 46 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 49 52 45 43 3D 4F 46 46 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 54 52 41 50 3D 4F 46 46 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 48 4F 41 44 3D 49 30 41 30 30 30 32 30 46 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 4A 4F 41 55 3D 55 73 65 72 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 43 4E 41 4D 3D 58 50 50 52 4F 2D 56 4D 22 0A 40 50 4A 4C 20 53 45 54 20 4F 55 54 42 49 4E 3D 46 41 43 45 44 4F 57 4E 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 50 4F 44 52 3D 4E 4F 52 4D 41 4C 22 0A 40 50 4A 4C 20 53 45 54 20 53 4C 49 50 53 48 45 45 54 3D 4F 46 46 0A 40 50 4A 4C 20 53 45 54 20 50 41 50 45 52 44 49 52 45 43 54 49 4F 4E 3D 53 45 46 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 41 50 53 50 3D 4F 46 46 22 0A 40 50 4A 4C 20 53 45 54 20 52 45 53 4F 4C 55 54 49 4F 4E 3D 33 30 30 0A 40 50 4A 4C 20 53 45 54 20 42 49 54 53 50 45 52 50 49 58 45 4C 3D 38 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 44 52 44 4D 3D 51 43 49 51 55 41 4C 49 54 59 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 54 53 43 52 3D 39 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 47 53 43 52 3D 39 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 49 53 43 52 3D 39 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 54 54 52 43 3D 39 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 47 54 52 43 3D 39 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 49 54 52 43 3D 39 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 54 43 50 52 3D 32 34 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 47 43 50 52 3D 32 34 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 49 43 50 52 3D 32 35 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 54 55 43 52 3D 32 34 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 47 55 43 52 3D 32 34 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 49 55 43 52 3D 32 35 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 54 44 46 54 3D 31 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 47 44 46 54 3D 31 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 49 44 46 54 3D 31 22 0A 40 50 4A 4C 20 53 45 54 20 4A 4F 42 41 54 54 52 3D 22 40 42 41 4E 52 3D 44 45 56 49 43 45 22 0A 40 50 4A 4C 20 45 4E 54 45 52 20 4C 41 4E 47 55 41 47 45 3D 44 50 4C 0A 29 20 48 50 2D 50 43 4C 20 58 4C 3B 32 3B 31 3B 46 58 58 50 4C 32 2D 31 2E 30 2E 30 2C 57 49 4E 4E 54 2D 35 2E 31 2C 58 50 4C 32 44 52 56 2D 31 2E 30 2E 36 2E 30 0A
*/
