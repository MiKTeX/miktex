/* bintoc.cpp: make a C char array from a binary file

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

#include <cstdio>
#include <fcntl.h>

#if (defined(_MSC_VER) && defined(_WIN32)) || defined(__CYGWIN__)
#  include <io.h>
#endif

size_t totalBytes;
size_t remainingBytesOnLine;

void OpenArray(const char * lpszArrayName)
{
  printf("unsigned char const %s[] = {", lpszArrayName);
  totalBytes = 0;
  remainingBytesOnLine = 0;
}

void CloseArray()
{
  puts("\n};");
}

void CloseLine()
{
  putchar('\n');
}

void AddByte(unsigned char byt)
{
  if (remainingBytesOnLine == 0)
  {
    CloseLine();
    remainingBytesOnLine = 8;
    putchar(' ');
  }
  printf(" 0x%02x,", byt);
  ++totalBytes;
  --remainingBytesOnLine;
}

unsigned char buf[4096 * 16];

int main(int argc, char ** argv)
{
  if (argc != 2)
  {
    return 1;
  }

#if defined(_MSC_VER) && defined(_WIN32)
  if (_setmode(_fileno(stdin), _O_BINARY) == -1)
  {
    return 1;
  }
#elif defined(__CYGWIN__)
  if (setmode(fileno(stdin), O_BINARY) == -1)
  {
    return 1;
  }
#endif

  OpenArray(argv[1]);

  size_t n;

  while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0)
  {
    for (size_t i = 0; i < n; ++i)
    {
      AddByte(buf[i]);
    }
  }

  CloseArray();

  return 0;
}
