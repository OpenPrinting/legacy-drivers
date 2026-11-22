//
// Hex2float conversion program
// by James L. Hammons
// (C) 2008 Underground Software
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdint.h>
#include <string.h>

float ConvertHexToFloat(uint32_t hexVal)
{
	float * pFlt = (float *)&hexVal;
	return *pFlt;
}

int main(int argc, char * argv[])
{
	if (argc != 2)
	{
		printf("Hex to float converter\n");
		printf("Usage: %s <hex>\n", argv[0]);
		return -1;
	}

	if (strlen(argv[1]) != 8)
	{
		printf("Wrong # of hex digits: Need eight.\n");
		return -1;
	}

//	char buffer[64];
	uint32_t hexval;

	sscanf(argv[1], "%x", &hexval);

	printf("$%08X -> %f\n", hexval, ConvertHexToFloat(hexval));

	return 1;
}
