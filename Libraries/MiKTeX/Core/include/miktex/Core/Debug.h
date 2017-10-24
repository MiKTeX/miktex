/* miktex/Core/Debug.h: MiKTeX debug macros             -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(ADD61B45737D4F4ABCD5725E989A2EDA)
#define ADD61B45737D4F4ABCD5725E989A2EDA

#include <miktex/Core/config.h>

#if defined(_MSC_VER)
#  include <crtdbg.h>
#endif

#if defined(__cplusplus)
#  include "Exceptions.h"
#endif

#include "Text.h"

#if defined(__cplusplus)
#  include <string>
#endif

#if defined(__cplusplus)
#  include <cstdlib>
#else
#  include <stdlib.h>
#endif

#define MIKTEX_DEBUG_BEGIN_NAMESPACE            \
  namespace MiKTeX {                            \
    namespace Debug {

#define MIKTEX_DEBUG_END_NAMESPACE              \
    }                                           \
  }

#if defined(__cplusplus)
MIKTEX_DEBUG_BEGIN_NAMESPACE;

MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL FatalMiKTeXError(const std::string& message, const MiKTeX::Core::MiKTeXException::KVMAP& info, const MiKTeX::Core::SourceLocation& sourceLocation);
MIKTEXCORECEEAPI(void*) Malloc(std::size_t size, const MiKTeX::Core::SourceLocation& sourceLocation);
MIKTEXCORECEEAPI(void*) Realloc(void* ptr, std::size_t size, const MiKTeX::Core::SourceLocation& sourceLocation);
MIKTEXCORECEEAPI(void*) Calloc(std::size_t num, std::size_t size, const MiKTeX::Core::SourceLocation& sourceLocation);

#if defined(_MSC_VER)
#  pragma push_macro("StrDup")
#  undef StrDup
#endif

MIKTEXCORECEEAPI(char *) StrDup(const char* lpsz, const MiKTeX::Core::SourceLocation& sourceLocation);

#if defined(_MSC_VER)
#  pragma pop_macro("StrDup")
#endif

MIKTEXCORECEEAPI(void) Free(void* ptr, const MiKTeX::Core::SourceLocation& sourceLocation);

MIKTEX_DEBUG_END_NAMESPACE;
#endif /* C++ */

MIKTEX_BEGIN_EXTERN_C_BLOCK;

MIKTEXCORECEEAPI(void*) miktex_core_malloc(size_t size, const char* lpszFileName, int line);
MIKTEXCORECEEAPI(void*) miktex_core_realloc(void* ptr, size_t size, const char* lpszFileName, int line);
MIKTEXCORECEEAPI(void*) miktex_core_calloc(size_t num, size_t size, const char* lpszFileName, int line);
MIKTEXCORECEEAPI(char*) miktex_core_strdup(const char* lpsz, const char* lpszFileName, int line);
MIKTEXCORECEEAPI(void) miktex_core_free(void* ptr, const char* lpszFileName, int line);
MIKTEXCOREEXPORT void MIKTEXNORETURN MIKTEXCEECALL miktex_core_fatal_error(const char* lpszMiktexFunction, const char* lpszMessage, const char* lpszInfo, const char* lpszSourceFile, int lpszSourceLine);

MIKTEX_END_EXTERN_C_BLOCK;

#define MIKTEX_MALLOC(size)                             \
  MiKTeX::Debug::Malloc((size), MIKTEX_SOURCE_LOCATION_DEBUG())

#define MIKTEX_REALLOC(ptr, size)                               \
  MiKTeX::Debug::Realloc((ptr), (size), MIKTEX_SOURCE_LOCATION_DEBUG())

#define MIKTEX_CALLOC(num, size)                        \
  MiKTeX::Debug::Calloc(num, size, MIKTEX_SOURCE_LOCATION_DEBUG())

#  define MIKTEX_STRDUP(str)                            \
  MiKTeX::Debug::StrDup((str), MIKTEX_SOURCE_LOCATION_DEBUG())

#define MIKTEX_FREE(ptr)                                \
  MiKTeX::Debug::Free((ptr), MIKTEX_SOURCE_LOCATION_DEBUG())

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_ASSERT(expr)                                           \
  static_cast<void>                                                     \
   ((expr)                                                              \
    ? 0                                                                 \
    : (MiKTeX::Debug::FatalMiKTeXError(MIKTEXTEXT("Assertion failed."), MiKTeX::Core::MiKTeXException::KVMAP("expr", #expr), MIKTEX_SOURCE_LOCATION()), \
       0))
#else
#  define MIKTEX_ASSERT(expr) (static_cast<void>(0))
#endif

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_VERIFY(expr)                   \
  (void)                                        \
   ((expr)                                      \
    ? 0                                         \
    : (miktex_core_fatal_error                  \
           (0,                                  \
            MIKTEXTEXT("Assertion failed."),    \
            #expr,                              \
            __FILE__,                           \
            __LINE__),                          \
       0))
#else
#  define MIKTEX_VERIFY(expr) ((void)(expr))
#endif

#if defined(_MSC_VER)
#  define MIKTEX_ASSUME(expr) __assume(expr)
#else
#  define MIKTEX_ASSUME(expr)
#endif

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_ASSERT_VALID_HEAP() MiKTeX::Debug::AssertValidHeap()
#else
#  define MIKTEX_ASSERT_VALID_HEAP()
#endif

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(ptr)  \
  if (ptr != nullptr)                                   \
  {                                                     \
    MiKTeX::Debug::AssertValidHeapPointer(ptr);         \
  }
#else
#  define MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(ptr)
#endif

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_ASSERT_BUFFER(ptr, n) MiKTeX::Debug::AssertValidBuf(ptr, n)
#else
#  define MIKTEX_ASSERT_BUFFER(ptr, n)
#endif

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_ASSERT_BUFFER_OR_NIL(ptr, n)   \
  if (ptr != nullptr)                           \
  {                                             \
    MiKTeX::Debug::AssertValidBuf(ptr, n);      \
  }
#else
#  define MIKTEX_ASSERT_BUFFER_OR_NIL(ptr, n)
#endif

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_ASSERT_CHAR_BUFFER(ptr, n)                     \
  MiKTeX::Debug::AssertValidBuf(ptr, sizeof(ptr[0]) * (n))
#else
#  define MIKTEX_ASSERT_CHAR_BUFFER(ptr, n)
#endif

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(ptr, n)              \
  if (ptr != nullptr)                                           \
  {                                                             \
    MiKTeX::Debug::AssertValidBuf(ptr, sizeof(ptr[0]) * (n));   \
  }
#else
#  define MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(ptr, n)
#endif

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_ASSERT_STRING(ptr) MiKTeX::Debug::AssertValidString(ptr)
#else
#  define MIKTEX_ASSERT_STRING(ptr)
#endif

#if defined(MIKTEX_DEBUG)
#  define MIKTEX_ASSERT_STRING_OR_NIL(ptr)      \
  if (ptr != nullptr)                           \
  {                                             \
    MiKTeX::Debug::AssertValidString(ptr);      \
  }
#else
#  define MIKTEX_ASSERT_STRING_OR_NIL(ptr)
#endif

#  define MIKTEX_ASSERT_PATH_BUFFER(ptr) \
  MIKTEX_ASSERT_CHAR_BUFFER (ptr, MiKTeX::Core::BufferSizes::MaxPath)

#  define MIKTEX_ASSERT_PATH_BUFFER_OR_NIL(ptr) \
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL (ptr, MiKTeX::Core::BufferSizes::MaxPath)

#  define MIKTEX_ASSERT_FNAME_BUFFER(ptr) \
  MIKTEX_ASSERT_CHAR_BUFFER (ptr, MiKTeX::Core::BufferSizes::MaxPath)

#if defined(__cplusplus)
MIKTEX_DEBUG_BEGIN_NAMESPACE;

inline void AssertValidBuf(void* ptr, std::size_t n)
{
  MIKTEX_ASSERT(ptr != nullptr);
#if defined(MIKTEX_DEBUG)
  // TODO: check write
#endif
}

MIKTEX_DEBUG_END_NAMESPACE;
#endif

#if defined(__cplusplus)
MIKTEX_DEBUG_BEGIN_NAMESPACE;

inline void AssertValidString(const char* ptr, std::size_t n = 4096)
{
  MIKTEX_ASSERT(ptr != nullptr);
#if defined(MIKTEX_DEBUG)
  // TODO: check null terminated string
#endif
}

inline void AssertValidString(const wchar_t* ptr, std::size_t n = 4096)
{
  MIKTEX_ASSERT(ptr != nullptr);
#if defined(MIKTEX_DEBUG)
  // TODO: check null terminated string
#endif
}

MIKTEX_DEBUG_END_NAMESPACE;
#endif

#if defined(__cplusplus)
MIKTEX_DEBUG_BEGIN_NAMESPACE;

inline void AssertValidHeap()
{
#if defined(_MSC_VER)
#  if defined(_DEBUG)
  MIKTEX_ASSERT(_CrtCheckMemory());
#  elif defined(MIKTEX_DEBUG)
  int heapStatus;
  MIKTEX_ASSERT((heapStatus = _heapchk()) == _HEAPOK || (heapStatus == _HEAPEMPTY));
#  endif
#endif
}

MIKTEX_DEBUG_END_NAMESPACE;
#endif

#if defined(__cplusplus)
MIKTEX_DEBUG_BEGIN_NAMESPACE;

inline void AssertValidHeapPointer(const void* ptr)
{
#if defined(_MSC_VER)
#  if defined(_DEBUG)
  MIKTEX_ASSERT(_CrtIsValidHeapPointer(ptr));
#  endif
#endif
}

MIKTEX_DEBUG_END_NAMESPACE;
#endif

#endif
