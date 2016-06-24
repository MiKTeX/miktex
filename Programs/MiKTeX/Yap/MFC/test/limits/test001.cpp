#include <iomanip>
#include <iostream>

using namespace std;

void
CreatePages (/*[in]*/ int nPages)
{
  for (int n = 1; n <= nPages; ++ n)
    {
      if (n > 1)
	{
	  cout << "\\pagebreak4" << endl;
	}
      cout << "Page " << n << endl;
      cout << endl;
      for (int i = 0; i < 10; ++ i)
	{
	  cout << "Hello, world!" << endl;
	}
    }
}

int
main ()
{
  cout << "\\documentclass{article}" << endl
       << "\\begin{document}" << endl;
  CreatePages (12000);
  cout << "\\end{document}" << endl;
  return (0);
}
