/* makebitcounts.cpp: make the bitcounts array (see pkchar.cpp)

   Copyright (C) 1996-2016 Christian Schenk

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <cstdio>

const unsigned max_bitfield_length = 16;

unsigned CountBits(unsigned n)
{
  unsigned result = 0;
  for (unsigned i = 0; i < max_bitfield_length; ++i)
  {
    if (n & 1)
    {
      result += 1;
    }
    n >>= 1;
  }
  return result;
}

int main()
{
  const unsigned maxval = 1 << max_bitfield_length;
  for (unsigned i = 0; i < maxval; ++i)
  {
    printf("%u,\n", CountBits(i));
  }
  return 0;
}
