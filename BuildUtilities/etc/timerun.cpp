#include <cstdio>
#include <process.h>
#include <windows.h>

void
main (int argc, const char ** argv)
{
  DWORD dw1 = GetTickCount();
  _spawnvp (_P_WAIT, argv[1], &argv[1]);
  DWORD dw2 =  GetTickCount();
  printf ("%lu\n", dw2 - dw1);
}
