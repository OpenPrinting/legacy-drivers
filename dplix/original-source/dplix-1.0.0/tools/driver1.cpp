//
// An attempt to connect to the Dell 3010cn printer
// by passing in an intercepted message...
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
	{
		array[counter++] = (length & 0xFF);
		array[counter++] = (length >> 8) & 0xFF;
		array[counter++] = (length >> 16) & 0xFF;
		array[counter++] = (length >> 24) & 0xFF;
	}

	memcpy(&array[counter], data, length);
	counter += length;
}


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


#define DEBUG

int main(int argc, char *argv[])
{
#ifdef DEBUG
	printf("DEBUG: Length of preamble: %u bytes.\n", strlen((const char *)preamble));
#endif

	if (2 != argc)
	{
		fprintf(stderr, "Dell Printer Driver v1\nby James L. Hammons\n");
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		exit(1);
	}

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

	printf("Writing payload to server...\n");

	FILE * fp = fopen(argv[1], "rb");

	if (NULL == fp)
	{
		perror("fopen()");
	}

	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	rewind(fp);

//	char * payload = new char[fileSize];
	char * bits = new char[fileSize];
	uint8_t * payload = new uint8_t[2000000];	// 2Meg size

	if (fread(bits, 1, fileSize, fp) != fileSize)
	{
		perror("fread()");
	}

	uint32_t count = 0;
	Test1(payload, count, bits, fileSize);

//	printf("Need to send this stuff over the wire...[%u bytes]\n", fileSize);
//	int num = write(clientSocket, (void *)((char *)(buffer + bytesWritten)), bytesToSend - bytesWritten);
//	int num = write(clientSocket, (void *)payload, fileSize);
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

	delete[] bits;
	delete[] payload;
	fclose(fp);
#if 0
	/*
	 * Client application specific code goes here
	 *
	 * e.g. receive messages from server, respond,
	 * etc. */
	while (0 < (status = read(clientSocket, buffer, sizeof(buffer) - 1)))
	{
		printf("%d: %s", status, buffer);
	}

	if (-1 == status)
	{
		perror("read()");
	}
#endif

	close(clientSocket);

	return 0;
}

#if 0
/*
 * Listing 3:
 * Handling short writes
 * Ivan Griffin (ivan.griffin@ul.ie)
 */
int bytesToSend = 0,
    bytesWritten = 0,
    num = 0;

/*
 * somewhere here bytesToSend, buffer, and
 * fileDesc must be set up.
 */

for (bytesWritten = 0; bytesWritten < bytesToSend;
     bytesWritten += num)
{
    num = write(fileDesc,
        (void *)( (char *)buffer +
      (char *)bytesWritten ),
        bytesToSend - bytesWritten);

    if (num < 0)
    {
        perror("write()");

        if (errno != EINTR)
        {
            exit(1);
        }
    }
}
#endif
