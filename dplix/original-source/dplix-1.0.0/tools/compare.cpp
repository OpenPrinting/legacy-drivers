#include <iostream>
#include <iomanip>
#include <fstream>

int main(int argc, char * argv[])
{
  using namespace std;

  if (argc != 3)
  {
    cout << "File comparer" << endl;
    cout << "Usage: compare <filename1> <filename2>" << endl;
    cout << "Counts start at zero (not one!)." << endl;
    return -1;
  }

  fstream ff, ff2;
  ff.open(argv[1], ios::in|ios::binary);
  ff2.open(argv[2], ios::in|ios::binary);

  if (!ff)
  {
    cout << "Unable to open file '" << argv[1] << "'!" << endl;
    return -1;
  }

  if (!ff2)
  {
    cout << "Unable to open file '" << argv[2] << "'!" << endl;
    return -1;
  }

//  unsigned char ch1, ch2;
  char ch1, ch2;
  int count = 0;
  while (!ff.eof() && !ff2.eof())
  {
    ff.get(ch1);
    ff2.get(ch2);
    if (ch1 != ch2)
      cout << "Difference at byte " << dec << count << ": ["
        << ch1 << "][" << ch2 << "] ($"
        << hex << count << ": $" << (((int)ch1)&0xFF) << " $" << (((int)ch2)&0xFF) << ")"
        << endl;
    count++;
  }

  ff.close();
  ff2.close();

  return 1;
}
