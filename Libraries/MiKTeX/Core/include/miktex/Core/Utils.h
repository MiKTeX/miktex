/* miktex/Core/Utils.h:                                 -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

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

#if !defined(EF1EA09F9E944352B856B3ECD37303CE)
#define EF1EA09F9E944352B856B3ECD37303CE

#include <miktex/Core/config.h>

#include <cstddef>
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <exception>
#include <string>

#include "Debug.h"
#include "Exceptions.h"
#include "VersionNumber.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

class PathName;

/// Information about an entry in a font map file.
struct FontMapEntry
{
  /// Name of the TeX font.
  std::string texName;
  /// Name of the PostScript font.
  std::string psName;
  /// Special instructions to be embedded into the PostScript document.
  std::string specialInstructions;
  /// Name of the encoding vector file.
  std::string encFile;
  /// Name of the font file.
  std::string fontFile;
  /// List of header files.
  std::string headerList;
};

inline bool operator< (const FontMapEntry & lhs, const FontMapEntry & rhs)
{
  return lhs.texName < rhs.texName;
}


/// MiKTeX utility class.
class MIKTEXNOVTABLE Utils
{
public:
  /// Gets the value of an environment variable.
  /// @param lpszName The name of the environment variable.
  /// @param[out] str The string object to be filled with the value.
  /// @return Returns true if the environment variable exists.
public:
  static MIKTEXCORECEEAPI(bool) GetEnvironmentString(const char * lpszName, std::string & str);

public:
  static MIKTEXCORECEEAPI(bool) GetEnvironmentString(const char * lpszName, PathName & path);

  /// Gets the value of an environment variable.
  /// @param lpszName The name of the environment variable.
  /// @param[out] lpszOut The destination string buffer.
  /// @param sizeOut The size (in characters) of the destination string
  /// buffer.
  /// @return Returns true if the environment variable exists.
public:
  static MIKTEXCORECEEAPI(bool) GetEnvironmentString(const char * lpszName, char * lpszOut, std::size_t sizeOut);

public:
  static MIKTEXCORECEEAPI(bool) FindProgram(const std::string & programName, PathName & path);

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(bool) GetDefPrinter(char * lpszPrinterName, std::size_t * pBufferSize);
#endif

public:
  static MIKTEXCORECEEAPI(std::string) GetExeName();

  /// Gets the MiKTeX version information as a string.
public:
  static MIKTEXCORECEEAPI(std::string) GetMiKTeXVersionString();

  /// Gets the MiKTeX banner string.
  /// The banner string includes the MiKTeX version number.
  /// @return Returns a string object.
public:
  static MIKTEXCORECEEAPI(std::string) GetMiKTeXBannerString();

public:
  static MIKTEXCORECEEAPI(std::string) MakeProgramVersionString(const char * lpszProgramName, const VersionNumber & versionNumber);

public:
  static MIKTEXCORECEEAPI(std::string) GetOSVersionString();

public:
  static MIKTEXCORECEEAPI(bool) RunningOnAServer();

public:
  static MIKTEXCORECEEAPI(void) UncompressFile(const char * lpszPathIn, PathName & pathOut);

public:
  static MIKTEXCORECEEAPI(const char *) GetRelativizedPath(const char * lpszPath, const char * lpszRoot);

public:
  static MIKTEXCORECEEAPI(bool) GetUncRootFromPath(const char * lpszPath, PathName & uncRoot);

public:
  static MIKTEXCORECEEAPI(bool) IsAbsolutePath(const char * lpszPath);

public:
  static MIKTEXCORECEEAPI(bool) IsSafeFileName(const char * lpszPath, bool forInput);

public:
  static MIKTEXCORECEEAPI(bool) IsParentDirectoryOf(const char * lpszParentDir, const char * lpszFileName);

public:
  static MIKTEXCORECEEAPI(bool) SupportsHardLinks(const PathName & path);

public:
  static MIKTEXCORECEEAPI(void) MakeTeXPathName(PathName & path);

public:
  static MIKTEXCORECEEAPI(void) SetEnvironmentString(const char * lpszValueName, const char * lpszValue);

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(void) RemoveBlanksFromPathName(PathName & path);
#endif

public:
  static MIKTEXCORECEEAPI(void) PrintException(const std::exception & e);

  /// Prints a MiKTeXException to the standard error stream.
  /// @param e Execption to print.  
public:
  static MIKTEXCORECEEAPI(void) PrintException(const MiKTeXException & e);

public:
  static MIKTEXCORECEEAPI(bool) ReadUntilDelim(std::string & str, int delim, FILE * stream);

public:
  static MIKTEXCORECEEAPI(bool) ReadLine(std::string & str, FILE * stream, bool keepLineEnding);

public:
  static MIKTEXCORECEEAPI(std::string) Hexify(const void * pv, std::size_t nBytes, bool lowerCase);

public:
  static MIKTEXCORECEEAPI(std::string) Hexify(const void * pv, std::size_t nBytes);

public:
  static MIKTEXCORECEEAPI(bool) ParseDvipsMapLine(const std::string & line, FontMapEntry & fontMapEntry);

public:
  static MIKTEXCORECEEAPI(bool) IsMiKTeXDirectRoot(const PathName & root);

#if !HAVE_MIKTEX_USER_INFO
public:
  static MIKTEXCORECEEAPI(void) RegisterMiKTeXUser();
#endif

#if !HAVE_MIKTEX_USER_INFO
public:
  static MIKTEXCORECEEAPI(bool) IsRegisteredMiKTeXUser();
#endif

public:
  static MIKTEXCORECEEAPI(void) ShowWebPage(const char * lpszUrl);
  
public:
#if defined(MIKTEX_WINDOWS)
  static MIKTEXCORECEEAPI(PathName) GetFolderPath(int nFolder, int nFallbackFolder, bool getCurrentPath);
#endif

public:
  static MIKTEXCORECEEAPI(void) CheckHeap();

public:
  static MIKTEXCORECEEAPI(void) CanonicalizePathName(PathName & path);

public:
  static MIKTEXCORECEEAPI(bool) GetPathNamePrefix(const PathName & path_, const PathName & suffix_, PathName & prefix);

public:
  static bool IsPureAscii(const char * lpsz)
  {
    MIKTEX_ASSERT_STRING(lpsz);
    for (; *lpsz != 0; ++lpsz)
    {
      if (static_cast<unsigned>(*lpsz) > 127)
      {
        return false;
      }
    }
    return true;
  }

public:
  static bool IsPureAscii(const std::string & str)
  {
    return IsPureAscii(str.c_str());
  }

public:
  static MIKTEXCORECEEAPI(bool) IsUTF8(const char * lpsz, bool allowPureAscii);

public:
  static bool IsUTF8(const char * lpsz)
  {
    return IsUTF8(lpsz, true);
  }

public:
  static bool Equals(const char * str1, const char * str2)
  {
    return strcmp(str1, str2) == 0;
  }

public:
  static bool EqualsIgnoreCase(const char * str1, const char * str2)
  {
    MIKTEX_ASSERT(IsPureAscii(str1) && IsPureAscii(str2));
#if defined(_MSC_VER)
    return _stricmp(str1, str2) == 0;
#else
    return strcasecmp(str1, str2) == 0;
#endif
  }

public:
  static bool EqualsIgnoreCase(const std::string & str1, const std::string & str2)
  {
    return EqualsIgnoreCase(str1.c_str(), str2.c_str());
  }

public:
  static std::string MakeLower(const std::string & str)
  {
    MIKTEX_ASSERT(IsPureAscii(str));
    std::string str2(str.size(), 0);
    std::transform(str.begin(), str.end(), str2.begin(), ::tolower);
    return str2;
  }

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(bool) CheckPath(bool repair);
#endif

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(void) RegisterShellFileAssoc(const char * lpszExtension, const char * lpszProgId, bool takeOwnership);
#endif

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(void) UnregisterShellFileAssoc(const char * lpszExtension, const char * lpszProgId);
#endif

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(void) RegisterShellFileType(const char * lpszProgId, const char * lpszUserFriendlyName, const char * lpszIconPath);
#endif

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(void) UnregisterShellFileType(const char * lpszProgId);
#endif

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(void) RegisterShellVerb(const char * lpszProgId, const char * lpszVerb, const char * lpszCommand, const char * lpszDdeExec);
#endif

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(std::string) MakeProgId(const char * lpszComponent);
#endif
};

MIKTEX_CORE_END_NAMESPACE;

#endif
