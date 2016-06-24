/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if defined(MIKTEX_TEXMF_SHARED)
#  define C4PEXPORT MIKTEXDLLEXPORT
#  define MIKTEXMFEXPORT MIKTEXDLLEXPORT
#else
#  define C4PEXPORT
#  define MIKTEXMFEXPORT
#endif

#define B8C7815676699B4EA2DE96F0BD727276
#define C1F0C63F01D5114A90DDF8FC10FF410B
#include "miktex/C4P/C4P"
#include "miktex/TeXAndFriends/MetafontApp"
#include "miktex/TeXAndFriends/ETeXApp"

using namespace C4P;
using namespace MiKTeX::Packages;
using namespace MiKTeX::TeXAndFriends;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#define FIRST_OPTION_VAL 256

#define BEGIN_INTERNAL_NAMESPACE                \
namespace MiKTeX {                              \
  namespace B3EB240141E54EF2BE3E8E2C742908B9 {

#define END_INTERNAL_NAMESPACE                  \
  }                                             \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

BEGIN_INTERNAL_NAMESPACE;

#define STATICFUNC(type) type

#define MIKTEX_API_BEGIN(funcname) {
#define MIKTEX_API_END(funcname) }

#if !defined(UNUSED)
#  if ! defined(NDEBUG)
#    define UNUSED(x)
#  else
#    define UNUSED(x) x
#  endif
#endif

#if ! defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) x
#endif

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#if 1 // 2015-01-17
extern bool IsNameManglingEnabled;
#endif

inline int GetC(FILE * stream)
{
  int ch = getc(stream);
  if (ch == EOF && ferror(stream) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("getc");
  }
  return ch;
}

template<class VALTYPE> class AutoRestore
{
public:
  AutoRestore(VALTYPE & val) :
    oldVal(val),
    pVal(&val)
  {
  }

public:
  ~AutoRestore()
  {
    *pVal = oldVal;
  }

private:
  VALTYPE oldVal;

private:
  VALTYPE * pVal;
};

END_INTERNAL_NAMESPACE

using namespace MiKTeX::B3EB240141E54EF2BE3E8E2C742908B9;
