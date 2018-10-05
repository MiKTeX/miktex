#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>

#include <iostream>
#include <string>

using namespace std;

#if defined(_MSC_VER)
#  define MAIN wmain
#else
#  define MAIN main
#endif

void FatalError(const string & msg)
{
  cerr << msg << endl;
  throw 1;
}

void Run()
{
  string test = u8"\U000000E4\U000000F6\U000000FC\U000000DF";
  string dirname = test + ".dir";
  string filename = dirname + "/" + test + ".txt";
  if (_mkdir(dirname.c_str()) != 0)
  {
    FatalError("cannot _mkdir " + dirname);
  }
  FILE * stream = fopen(filename.c_str(), "wb");
  if (stream == nullptr)
  {
    FatalError("cannot open " + filename);
  }
  for (const char & ch : test)
  {
    putc(ch, stream);
  }
  putc('\n', stream);
  fclose(stream);
  if (_access(filename.c_str(), 0) != 0)
  {
    FatalError("cannot _access " + filename);
  }
  struct stat statbuf;
  if (stat(filename.c_str(), &statbuf) != 0)
  {
    FatalError("cannot stat " + filename);
  }
  cout << statbuf.st_size << " bytes written" << endl;
  if (remove(filename.c_str()) != 0)
  {
    FatalError("cannot remove " + filename);
  }
  if (_rmdir(dirname.c_str()) != 0)
  {
    FatalError("cannot _rmdir " + dirname);
  }
}

int MAIN()
{
  try
  {
    Run();
    return 0;
  }
  catch (int x)
  {
    return x;
  }
}
