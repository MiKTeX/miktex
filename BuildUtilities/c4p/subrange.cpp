/* subrange.c: subrange types                           -*- C++ -*-

   Copyright (C) 1991-2016 Christian Schenk

   This file is part of C4P.

   C4P is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   C4P is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with C4P; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <climits>

#include "common.h"

struct subrange_t
{
  C4P_integer lb;
  C4P_integer ub;
  const char * type_name;
};

namespace {
  const subrange_t subrange_table[] =
  {
    -128l, 127l, "C4P_signed8",
    0l, 255l, "C4P_unsigned8",
    -32768, 32767l, "C4P_signed16",
    0l, 65535l, "C4P_unsigned16",
    -2147483648l, 2147483647l, "C4P_signed32"
  };
}

const char * subrange(C4P_integer lb, C4P_integer ub)
{
  const subrange_t * s = subrange_table;
  int i = sizeof(subrange_table) / sizeof(subrange_table[0]);
  for (; i--; ++s)
  {
    if (s->lb <= lb && lb <= s->ub && ub <= s->ub)
    {
      return s->type_name;
    }
  }
  return "C4P_unsigned32";
}
