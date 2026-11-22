#include <iostream>
#include <iomanip>
#include <fstream>

int main(int argc, char * argv[])
{
	using namespace std;

	if (argc != 2)
	{
		cout << "Dell Graphic parser" << endl;
		cout << "Usage: grparse <filename1>" << endl;
		return -1;
	}

	fstream ff;
	ff.open(argv[1], ios::in|ios::binary);

	if (!ff)
	{
		cout << "Unable to open file '" << argv[1] << "'!" << endl;
		return -1;
	}

	char scanline[8192*3];
//NOTE: This is dependent on a specific bitmap...
//	int width = 3591;
	int width = 14733 / 3;
	int height = 0;
	for(int i=0; i<width*3; i++)
		scanline[i] = 0;

	char ch1, ch2, outchar;
	while (!ff.eof())
	{
		ff.get(ch1);
		ff.get(ch2);
if (ff.eof())
	break;//ick

		int count = ((unsigned char)ch1 + ((unsigned char)ch2 * 256));
cerr << "count=" << count*8 << hex << "\t($" << count << ")\t($" << ff.tellp() << dec << ")" << endl;
		int cursor = 0;
		while (count > 0)
		{
#if 0
// problem with this:
// [3F 08] E0 7F [1F FF 67] 03
// ....#...###.....   ...#############.##.###.......##
~40 bytes to next change...
skip 39. prev @ 40: ####.... ..######
replace:            ###..... ..######
#endif
			ff.get(outchar);
			count--;

			int num = (((unsigned char)outchar & 0xE0) >> 5) + 1;
			int skip = ((unsigned char)outchar & 0x1F);

// $3F doesn't fit this pattern--why?? it does
// $7F doesn't fit either... wrong, starts with $1F
			if (skip == 31)
			{
				ff.get(outchar);
				count--;
				skip += (unsigned int)((unsigned char)outchar);

//may chain further... use while instead of if
//				if ((unsigned char)outchar == 255)
				while ((unsigned char)outchar == 255)
				{
					ff.get(outchar);
					count--;
					skip += (unsigned int)((unsigned char)outchar);
				}
			}

			cursor += skip;

			for(int i=0; i<num; i++)
			{
				ff.get(outchar);
				count--;
				scanline[cursor + i] = outchar;
			}

			cursor += num;
		}

//This is dependent on whether or not this is an RGB vs. B/W  bitmap...
#if 1
		for(int i=0; i<width*3; i++)
			cout << scanline[i];
#else
		for(int i=0; i<(width+7)/8; i++)
		{
			uint8_t c = scanline[i];

			for(int j=0; j<8; j++)
			{
				for(int k=0; k<3; k++)
					cout << (char)(c & 0x80 ? 0x00 : 0xFF);

				c <<= 1;
			}
		}
#endif

		height++;
	}

cerr << "height=" << height << endl;
	ff.close();

	return 1;
}
