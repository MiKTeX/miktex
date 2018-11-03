/* padout.cpp:

   Copyright (C) 2007-2018 Christian Schenk

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

int main(int argc, const char** argv)
{
  if (argc != 2)
  {
    cerr << "Usage: padout FILE" << '\n';
    return 1;
  }

  FILE* file = fopen(argv[1], "ab");

  if (file == nullptr)
  {
    cerr << "cannot open " << argv[1] << '\n';
    return 1;
  }

  if (fseek(file, 0, SEEK_END) != 0)
  {
    cerr << "seek error" << '\n';
    return 1;
  }

  long size = ftell(file);

  if (size < 0)
  {
    cerr << "I/O error" << '\n';
    return 1;
  }

  srand(static_cast<unsigned>(time(nullptr)));

  for (; (size % 512) != 0; ++size)
  {
    int byte = rand() & 0xff;
    if (fputc(byte, file) == EOF)
    {
      cerr << "I/O error" << '\n';
      return 1;
    }
    ++size;
  }

  fclose(file);

  return 0;
}
