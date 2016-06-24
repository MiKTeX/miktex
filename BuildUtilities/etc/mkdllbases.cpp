/* mkdllbases.cpp: find dll bases

   Copyright (C) 2000-2016 Christian Schenk

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include <windows.h>
#include <imagehlp.h>
#include <iostream>
#include <cstdlib>

const intptr_t DEFAULT_BASE = 0x60000000;

using namespace std;

int
main (int	argc,
      char **	argv)
{
  if (argc < 2)
    {
      return (1);
    }
  intptr_t ipBase = DEFAULT_BASE;
  for (int i = 1; i < argc; ++ i)
    {
      PLOADED_IMAGE pli = ImageLoad(argv[i], 0);
      if (pli == 0)
	{
	  return (2);
	}
      char szName[_MAX_FNAME];
#if _MSC_VER >= 1400
      _splitpath_s (argv[i], 0, 0, 0, 0, szName, _MAX_FNAME, 0, 0);
#else
      _splitpath (argv[i], 0, 0, szName, 0);
#endif
      ULONG ulSize = pli->FileHeader->OptionalHeader.SizeOfImage;
      cout << hex << szName
	   << "\t0x" << static_cast<unsigned>(ipBase)
	   << "\t0x" << ulSize << endl;
      ipBase += ulSize + 0xffff;
      ipBase &= 0xffff0000;
      ImageUnload (pli);
    }
  return (0);
}
