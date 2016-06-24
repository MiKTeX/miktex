/* internal.h: internal definitions                     -*- C++ -*-

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

#if defined(MIKTEX_CORE_SHARED)
#  define MIKTEXCOREEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXCOREEXPORT
#endif

#define EAD86981C92C904D808A5E6CEC64B90E
#include "miktex/Core/config.h"

#include "miktex/Core/Quoter.h"
#include "miktex/Core/Session.h"

#if MIKTEX_UNIX
#  define NO_REGISTRY 1
#else
#  define NO_REGISTRY 0
#endif

#define FIND_FILE_PREFER_RELATIVE_PATH_NAMES 1

// experimental
#define USE_CODECVT_UTF8 1

#define BEGIN_INTERNAL_NAMESPACE                \
namespace MiKTeX {                              \
  namespace ABF3880A6239B84E87DC7E727A8BBFD4 {

#define END_INTERNAL_NAMESPACE                  \
  }                                             \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

BEGIN_INTERNAL_NAMESPACE;

#define REPORT_EVENTS 0

#define C_FUNC_BEGIN()
#define C_FUNC_END()

#if defined(MIKTEX_WINDOWS)
#  define DEBUG_BREAK() DebugBreak()
#else
#  define DEBUG_BREAK()
#endif

#if !defined(UNUSED)
#  if !defined(NDEBUG)
#    define UNUSED(x)
#  else
#    define UNUSED(x) static_cast<void>(x)
#  endif
#endif

#if !defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

#define BUF_TOO_SMALL() MIKTEX_INTERNAL_ERROR()

#define INVALID_ARGUMENT(argumentName, argumentValue) MIKTEX_INTERNAL_ERROR()

#define OUT_OF_MEMORY(function) MIKTEX_INTERNAL_ERROR()

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#if defined(_MSC_VER)
#define UNSUPPORTED_PLATFORM()                                          \
  __assume(false)
#else
#define UNSUPPORTED_PLATFORM()                                          \
  MIKTEX_ASSERT (false)
#endif

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#if defined(MIKTEX_WINDOWS)
#  define WU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#  define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

#define MIKTEXINTERNALFUNC(type) type
#define MIKTEXINTERNALVAR(type) type

#define ARRAY_SIZE(buf) (sizeof(buf) / sizeof(buf[0]))

#define MAKE_SEARCH_PATH(dir)                   \
  TEXMF_PLACEHOLDER                             \
  MIKTEX_PATH_DIRECTORY_DELIMITER_STRING        \
  dir

#define CFG_MACRO_NAME_BINDIR "bindir"
#define CFG_MACRO_NAME_ENGINE "engine"
#define CFG_MACRO_NAME_LOCALFONTDIRS "localfontdirs"
#define CFG_MACRO_NAME_PSFONTDIRS "psfontdirs"
#define CFG_MACRO_NAME_OTFDIRS "otfdirs"
#define CFG_MACRO_NAME_PROGNAME "progname"
#define CFG_MACRO_NAME_TTFDIRS "ttfdirs"
#define CFG_MACRO_NAME_WINDIR "windir"

const unsigned FNDB_PAGESIZE = 0x1000;

#define TEXMF_PLACEHOLDER "%R"

#define CURRENT_DIRECTORY "."

#define PARENT_DIRECTORY ".."

#define PARENT_PARENT_DIRECTORY                 \
  PARENT_DIRECTORY                              \
  MIKTEX_PATH_DIRECTORY_DELIMITER_STRING        \
  PARENT_DIRECTORY

#define PARENT_PARENT_PARENT_DIRECTORY          \
  PARENT_DIRECTORY                              \
  MIKTEX_PATH_DIRECTORY_DELIMITER_STRING        \
  PARENT_PARENT_DIRECTORY

#if defined(MIKTEX_WINDOWS)
const char PATH_DELIMITER = ';';
#define PATH_DELIMITER_STRING ";"
#else
const char PATH_DELIMITER = ':';
#define PATH_DELIMITER_STRING ":"
#endif

const char * const RECURSION_INDICATOR = "//";
const size_t RECURSION_INDICATOR_LENGTH = 2;
const char * const SESSIONSVC = "sessionsvc";

// The virtual TEXMF root MPM_ROOT_PATH is assigned to the MiKTeX
// package manager.  We make sure that MPM_ROOT_PATH is a valid path
// name.  On the other hand, it must not interfere with an existing
// file system.
#if defined(MIKTEX_WINDOWS)
// An UNC path with an impossible share name suits our needs: `['
// isn't a valid character in a share name (KB236388)
const char * const COMMON_MPM_ROOT_PATH = "\\\\MiKTeX\\[MPM]";
const char * const USER_MPM_ROOT_PATH = "\\\\MiKTeX\\]MPM[";
const size_t MPM_ROOT_PATH_LEN_ = 14;
#else
const char * const COMMON_MPM_ROOT_PATH = "//MiKTeX/[MPM]";
const char * const USER_MPM_ROOT_PATH = "//MiKTeX/]MPM[";
const size_t MPM_ROOT_PATH_LEN_ = 14;
#endif

#define MPM_ROOT_PATH                           \
  IsAdminMode()                                 \
  ? COMMON_MPM_ROOT_PATH                        \
  : USER_MPM_ROOT_PATH

#if defined(MIKTEX_DEBUG)
#define MPM_ROOT_PATH_LEN                                               \
  static_cast<size_t>(MIKTEX_ASSERT(MiKTeX::Core::StrLen(MPM_ROOT_PATH) \
                             == MPM_ROOT_PATH_LEN_),                    \
                      MPM_ROOT_PATH_LEN_)
#else
const size_t MPM_ROOT_PATH_LEN = MPM_ROOT_PATH_LEN_;
#endif

void AppendDirectoryDelimiter(std::string & path);
void AppendDirectoryDelimiter(char * lpszPath, size_t size);
void CopyString2(char * lpszBuf, size_t bufSize, const char * lpszSource, size_t count);
void CreateDirectoryPath(const char * lpszPath);
bool FileIsOnROMedia(const char * lpszPath);
bool GetCrtErrorMessage(int functionResult, std::string & errorMessage);
MiKTeX::Core::PathName GetFullPath(const char * lpszPath);
MiKTeX::Core::PathName GetHomeDirectory();

#if defined(MIKTEX_WINDOWS)
bool GetUserProfileDirectory(MiKTeX::Core::PathName & path);
#endif

#if defined(MIKTEX_WINDOWS)
void SetTimesInternal(HANDLE handle, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);
#endif

#if defined(MIKTEX_WINDOWS)
bool GetWindowsFontsDirectory(MiKTeX::Core::PathName & path);
#endif

#if defined(MIKTEX_WINDOWS)
bool GetWindowsErrorMessage(unsigned long functionResult, std::string & errorMessage);
#endif

const char * GetFileNameExtension(const char * lpszPath);
Botan::Public_Key * LoadPublicKey();
bool HaveEnvironmentString(const char * lpszName);
bool GetEnvironmentString(const char * lpszName, std::string & value);
bool IsExplicitlyRelativePath(const char * lpszPath);
bool IsMpmFile(const char * lpszPath);
std::string MakeSearchPath(const std::vector<MiKTeX::Core::PathName> & vec);
void RemoveDirectoryDelimiter(char * lpszPath);

#if defined(MIKTEX_WINDOWS) && REPORT_EVENTS
bool ReportMiKTeXEvent(unsigned short eventType, unsigned long eventId, ...);
#endif

void TraceError(const char * lpszFormat, ...);

#if defined(MIKTEX_WINDOWS)
void TraceWindowsError(const char * lpszWindowsFunction, unsigned long functionResult, const char * lpszInfo, const char * lpszSourceFile, int lpszSourceLine);
#endif

const char * GetShortSourceFile(const char * lpszSourceFile);

inline void DbgView(const std::string & s)
{
#if defined(_WIN32)
  OutputDebugStringW(UW_("MiKTeX Core: " + s));
#endif
}

inline bool AdminControlsUserConfig()
{
#if ADMIN_CONTROLS_USER_CONFIG
#if 1
  return true;
#else
  const char * lpsz = getenv("MIKTEX_ADMIN_CONTROLS_USER_CONFIG");
  return lpsz != 0 && strcmp(lpsz, "t") == 0;
#endif
#else
  return false;
#endif
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

struct StringComparerIgnoringCase :
  public std::binary_function<std::string, std::string, bool>
{
  bool operator() (const std::string & lhs, const std::string & rhs) const
  {
    return MiKTeX::Util::StringCompare(lhs.c_str(), rhs.c_str(), true) < 0;
  }
};

inline FILE * FdOpen(const MiKTeX::Core::PathName & path, int fd, const char * lpszMode)
{
  FILE * stream = fdopen(fd, lpszMode);
  if (stream == nullptr)
  {
    MIKTEX_FATAL_CRT_ERROR_2("fdopen", "path", path.ToString());
  }
  return stream;
}

inline FILE * FdOpen(int fd, const char * lpszMode)
{
  FILE * stream = fdopen(fd, lpszMode);
  if (stream == nullptr)
  {
    MIKTEX_FATAL_CRT_ERROR("fdopen");
  }
  return stream;
}

/* _________________________________________________________________________ */

extern int debugOnStdException;

END_INTERNAL_NAMESPACE;

using namespace MiKTeX::ABF3880A6239B84E87DC7E727A8BBFD4;

#undef MIKTEXINTERNALFUNC
#define MIKTEXINTERNALFUNC(type) \
  type MiKTeX::ABF3880A6239B84E87DC7E727A8BBFD4::
#define MIKTEXINTERNALFUNC2(type, callconv) \
  type callconv MiKTeX::ABF3880A6239B84E87DC7E727A8BBFD4::

#undef MIKTEXINTERNALVAR
#define MIKTEXINTERNALVAR(type) \
  type MiKTeX::ABF3880A6239B84E87DC7E727A8BBFD4::

#if 0
#  define MIKTEXSTATICFUNC(type) static type
#  define MIKTEXSTATICFUNC2(type, callconv) static type callconv
#else
#  define MIKTEXSTATICFUNC(type) type
#  define MIKTEXSTATICFUNC2(type, callconv) type callconv
#endif

#define MIKTEXPERMANENTVAR(type) static type
