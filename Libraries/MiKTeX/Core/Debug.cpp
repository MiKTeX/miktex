/* Debug.cpp: debugging memory allications

   Copyright (C) 1996-2017 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "core-version.h"

#include "miktex/Core/Exceptions.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

void* MIKTEXCEECALL MiKTeX::Debug::Malloc(size_t size, const SourceLocation& sourceLocation)
{
#if 0 // too costly
#if defined(MIKTEX_DEBUG) && defined(MIKTEX_DEBUG)
  Utils::CheckHeap();
#endif
#endif
#if defined(_MSC_VER) && defined(_DEBUG) && defined(MIKTEX_DEBUG)
  void* ptr = _malloc_dbg(size, _NORMAL_BLOCK, sourceLocation.fileName.c_str(), sourceLocation.lineNo);
#else
  void* ptr = malloc(size);
#endif
  if (ptr == nullptr)
  {
    OUT_OF_MEMORY("malloc");
  }
  return ptr;
}

void* MIKTEXCEECALL MiKTeX::Debug::Calloc(size_t num, size_t size, const SourceLocation& sourceLocation)
{
#if 0 // too costly
#if defined(MIKTEX_DEBUG) && defined(MIKTEX_DEBUG)
  Utils::CheckHeap();
#endif
#endif
#if defined(_MSC_VER) && defined(_DEBUG) && defined(MIKTEX_DEBUG)
  void* ptr = _calloc_dbg(num, size, _NORMAL_BLOCK, sourceLocation.fileName.c_str(), sourceLocation.lineNo);
#else
  void* ptr = calloc(num, size);
#endif
  if (ptr == nullptr)
  {
    OUT_OF_MEMORY("calloc");
  }
  return ptr;
}

void* MIKTEXCEECALL MiKTeX::Debug::Realloc(void* ptr, size_t size, const SourceLocation& sourceLocation)
{
#if 0 // too costly
#if defined(MIKTEX_DEBUG) && defined(MIKTEX_DEBUG)
  Utils::CheckHeap();
#endif
#endif
#if defined(_MSC_VER) && defined(_DEBUG) && defined(MIKTEX_DEBUG)
  ptr = _realloc_dbg(ptr, size, _NORMAL_BLOCK, sourceLocation.fileName.c_str(), sourceLocation.lineNo);
#else
  ptr = realloc(ptr, size);
#endif
  if (ptr == nullptr && size > 0)
  {
    OUT_OF_MEMORY("realloc");
  }
  return ptr;
}

char* MIKTEXCEECALL MiKTeX::Debug::StrDup(const char* lpsz, const SourceLocation& sourceLocation)
{
  size_t len = StrLen(lpsz);
  char* lpsz2 = reinterpret_cast<char *>(Malloc(sizeof(*lpsz) * (len + 1), sourceLocation));
  StringUtil::CopyString(lpsz2, len + 1, lpsz);
  return lpsz2;
}

void MIKTEXCEECALL MiKTeX::Debug::Free(void* ptr, const SourceLocation& sourceLocation)
{
  int oldErrno = errno;
#if defined(_MSC_VER) && defined(_DEBUG) && defined(MIKTEX_DEBUG)
  if (ptr != nullptr && !_CrtIsValidHeapPointer(ptr))
  {
    MIKTEX_UNEXPECTED();
  }
#endif
#if 0 // too costly
#if defined(MIKTEX_DEBUG) && defined(MIKTEX_DEBUG)
  Utils::CheckHeap();
#endif
#endif
#if defined(_MSC_VER) && defined(_DEBUG) && defined(MIKTEX_DEBUG)
  _free_dbg(ptr, _NORMAL_BLOCK);
#else
  free(ptr);
#endif
  if (errno != 0 && errno != oldErrno)
  {
    MIKTEX_FATAL_CRT_ERROR("free");
  }
}

#if defined(_MSC_VER) && MIKTEX_COMP_INTERFACE_VERSION < 3
// TODO: remove API
void __declspec(dllexport) __cdecl MiKTeX::Debug::OnThrowStdException()
{
}
#endif

MIKTEXNORETURN void MIKTEXCEECALL MiKTeX::Debug::FatalMiKTeXError(const std::string& message, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation)
{
  Session::FatalMiKTeXError(message, "", "", "", info, sourceLocation);
}
