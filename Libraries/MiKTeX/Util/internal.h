/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Util Library.

   The MiKTeX Util Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Util Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Util Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(MIKTEX_UTIL_SHARED)
#  define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXUTILEXPORT
#endif

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/StringUtil.h"
#include "miktex/Util/Tokenizer.h"

#include <cstdarg>

#include <bitset>
#include <codecvt>
#include <locale>
#include <string>

#if defined(MIKTEX_WINDOWS)
#  include <Windows.h>
#endif

using namespace MiKTeX::Util;
using namespace std;

#define MIKTEXSTATICFUNC(type) type
#define MIKTEXSTATICFUNC2(type, callconv) type callconv

#define BEGIN_INTERNAL_NAMESPACE                \
namespace MiKTeX {                              \
  namespace DD62CE0F78794BD2AEEFCDD29780398B {

#define END_INTERNAL_NAMESPACE                  \
  }                                             \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

BEGIN_INTERNAL_NAMESPACE;

class UtilException : public std::exception
{
public:
  UtilException(const char * msg) :
    msg(msg)
  {
  }
public:
  const char * what() const noexcept override
  {
    return msg.c_str();
  }
private:
  std::string msg;
};

END_INTERNAL_NAMESPACE;

using namespace MiKTeX::DD62CE0F78794BD2AEEFCDD29780398B;

#define FATAL_ERROR() throw UtilException("MiKTeX encountered an internal error.")
