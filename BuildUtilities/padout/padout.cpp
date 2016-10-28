/* padout.cpp:

   Copyright (C) 2007-2016 Christian Schenk

   This file is part of PadOut.

   PadOut is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   PadOut is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with PadOut; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>

using namespace std;

int
main (/*[in]*/ int		argc,
      /*[in]*/ const char **	argv)
{
  if (argc != 2)
    {
      cerr << "Usage: padout FILE" << '\n';
      return (1);
    }

  FILE * pFile = fopen(argv[1], "ab");
  
  if (pFile == 0)
    {
      cerr << "cannot open " << argv[1] << '\n';
      return (1);
    }

  if (fseek(pFile, 0, SEEK_END) != 0)
    {
      cerr << "seek error" << '\n';
      return (1);
    }

  long size = ftell(pFile);

  if (size < 0)
    {
      cerr << "I/O error" << '\n';
      return (1);
    }
  
  srand (static_cast<unsigned>(time(nullptr)));

  for (; (size % 512) != 0; ++ size)
    {
      int byte = rand() & 0xff;
      if (fputc(byte, pFile) == EOF)
	{
	  cerr << "I/O error" << '\n';
	  return (1);
	}
      ++ size;
    }

  fclose (pFile);

  return (0);
}
