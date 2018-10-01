/* unxCommandLineBuilder.cpp: command-line builder

   Copyright (C) 1996-2018 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "internal.h"

#include "miktex/Core/CommandLineBuilder.h"

#include "Utils/inliners.h"

#include "../ArgvImpl.h"

using namespace MiKTeX::Core;
using namespace std;

// borrowed from the popt library
void Argv::Append(const string& arguments)
{
  MIKTEX_ASSERT(!pimpl->argv.empty());
  MIKTEX_ASSERT(pimpl->argv.back() == nullptr);
  pimpl->argv.pop_back();
  string arg;
  char quote = 0;
  for (const char* lpsz = arguments.c_str(); *lpsz != 0; ++lpsz)
  {
    if (*lpsz == quote)
    {
      quote = 0;
    }
    else if (quote != 0)
    {
      if (*lpsz == '\\')
      {
        ++lpsz;
        if (*lpsz == 0)
        {
          MIKTEX_FATAL_ERROR_2(T_("Invalid command-line."), "arguments", arguments);
        }
        if (*lpsz != quote)
        {
          arg += '\\';
        }
      }
      arg += *lpsz;
    }
    else if (IsSpace(*lpsz))
    {
      if (arg.length() > 0)
      {
        pimpl->argv.push_back(MIKTEX_STRDUP(arg.c_str()));
        arg = "";
      }
    }
    else
    {
      switch (*lpsz)
      {
      case '"':
      case '\'':
        quote = *lpsz;
        break;
      case '\\':
        ++lpsz;
        if (*lpsz == 0)
        {
          MIKTEX_FATAL_ERROR_2(T_("Invalid command-line."), "arguments", arguments);
        }
        arg += *lpsz;
        break;
      default:
        arg += *lpsz;
        break;
      }
    }
  }
  if (!arg.empty())
  {
    pimpl->argv.push_back(MIKTEX_STRDUP(arg.c_str()));
  }
  pimpl->argv.push_back(nullptr);
}
