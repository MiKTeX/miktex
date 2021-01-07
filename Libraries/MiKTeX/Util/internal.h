/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 1996-2021 Christian Schenk

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

#include <locale>
#include <string>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if !defined(MIKTEXUTILEXPORT)
  #if defined(MIKTEX_UTIL_SHARED)
  #  define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
  #else
  #  define MIKTEXUTILEXPORT
  #endif
#endif

#include "miktex/Util/PathName.h"

#define BEGIN_INTERNAL_NAMESPACE                \
namespace MiKTeX {                              \
  namespace Util {                              \
    namespace DD62CE0F78794BD2AEEFCDD29780398B {

#define END_INTERNAL_NAMESPACE                  \
    }                                           \
  }                                             \
}

BEGIN_INTERNAL_NAMESPACE;

class Helpers
{
public:
  static void CanonicalizePathName(MiKTeX::Util::PathName& path);

public:
  static bool GetEnvironmentString(const std::string& name, std::string& value);

public:
  static bool GetEnvironmentString(const std::string& name, MiKTeX::Util::PathName& path);

public:
  static bool IsPureAscii(const char* lpsz);

public:
  static const char* GetFileNameExtension(const char* path);

public:
  static void RemoveDirectoryDelimiter(char* path);

public:
  static bool DirectoryExists(const MiKTeX::Util::PathName& path);

public:
  static PathName GetHomeDirectory();

public:
  static wchar_t ToLower(wchar_t ch, std::locale& loc)
  {
    if (static_cast<unsigned>(ch) < 128)
    {
      if (ch >= L'A' && ch <= L'Z')
      {
        ch = ch - L'A' + L'a';
      }
    }
    else
    {
      ch = std::use_facet<std::ctype<wchar_t>>(loc).tolower(ch);
    }
    return ch;
  }

public:
  static char ToLowerAscii(char ch)
  {
    if (ch >= 'A' && ch <= 'Z')
    {
      ch = ch - 'A' + 'a';
    }
    return ch;
  }

public:
  static wchar_t ToUpper(wchar_t ch, std::locale& loc)
  {
    if (static_cast<unsigned>(ch) < 128)
    {
      if (ch >= L'a' && ch <= L'z')
      {
        ch = ch - L'a' + L'A';
      }
    }
    else
    {
      ch = std::use_facet<std::ctype<wchar_t>>(loc).toupper(ch);
    }
    return ch;
  }

public:
  static char ToUpperAscii(char ch)
  {
    if (ch >= 'a' && ch <= 'z')
    {
      ch = ch - 'a' + 'A';
    }
    return ch;
  }
};

class Exception :
  public std::exception
{
public:
  Exception(const std::string& msg) :
    msg(msg)
  {
  }
public:
  const char* what() const noexcept override
  {
    return msg.c_str();
  }
private:
  std::string msg;
};

class Unexpected :
  public Exception
{
public:
  Unexpected(const std::string& msg) :
    Exception(msg)
  {
  }
};

class CRuntimeError :
  public Exception
{
public:
  CRuntimeError(const std::string& msg, const std::string& function) :
    Exception(msg)
  {
  }
public:
  CRuntimeError(const std::string& function) :
    CRuntimeError("C runtime error", function)
  {
  }
};

#if defined(MIKTEX_WINDOWS)
class WindowsError :
  public Exception
{
public:
  WindowsError(const std::string& msg, const std::string& function) :
    Exception(msg)
  {
  }
public:
  WindowsError(const std::string& function) :
    Exception("Windows error")
  {
  }
};
#endif

END_INTERNAL_NAMESPACE;

using namespace MiKTeX::Util::DD62CE0F78794BD2AEEFCDD29780398B;

#define FATAL_ERROR() throw Unexpected("MiKTeX encountered an internal error.")
