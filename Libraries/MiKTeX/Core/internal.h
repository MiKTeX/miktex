/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 1996-2019 Christian Schenk

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

#pragma once

#if defined(ENABLE_OPENSSL)
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#endif

#include <miktex/Trace/TraceStream>

#include <miktex/Core/Quoter>
#include <miktex/Core/Session>

#if MIKTEX_UNIX
#  define NO_REGISTRY 1
#else
#  define NO_REGISTRY 0
#endif

#define FIND_FILE_PREFER_RELATIVE_PATH_NAMES 1

// experimental
#define USE_CODECVT_UTF8 1

#define CORE_INTERNAL_BEGIN_NAMESPACE           \
namespace MiKTeX {                              \
  namespace ABF3880A6239B84E87DC7E727A8BBFD4 {

#define CORE_INTERNAL_END_NAMESPACE             \
  }                                             \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

CORE_INTERNAL_BEGIN_NAMESPACE;

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

#define INVALID_ARGUMENT(argumentName, argumentValue) MIKTEX_FATAL_ERROR_2(T_("MiKTeX encountered an internal error."), argumentName, argumentValue)

#define OUT_OF_MEMORY(function) MIKTEX_INTERNAL_ERROR()

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#if defined(_MSC_VER)
#define UNSUPPORTED_PLATFORM()                                          \
  __assume(false)
#else
#define UNSUPPORTED_PLATFORM()                                          \
  MIKTEX_ASSERT(false)
#endif

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

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

const char* const RECURSION_INDICATOR = "//";
const size_t RECURSION_INDICATOR_LENGTH = 2;
const char* const SESSIONSVC = "sessionsvc";

// The virtual TEXMF root MPM_ROOT_PATH is assigned to the MiKTeX
// package manager.  We make sure that MPM_ROOT_PATH is a valid path
// name.  On the other hand, it must not interfere with an existing
// file system.
#if defined(MIKTEX_WINDOWS)
// An UNC path with an impossible share name suits our needs: `['
// isn't a valid character in a share name (KB236388)
const char* const COMMON_MPM_ROOT_PATH = "\\\\MiKTeX\\[MPM]";
const char* const USER_MPM_ROOT_PATH = "\\\\MiKTeX\\]MPM[";
const size_t MPM_ROOT_PATH_LEN_ = 14;
#else
const char* const COMMON_MPM_ROOT_PATH = "//MiKTeX/[MPM]";
const char* const USER_MPM_ROOT_PATH = "//MiKTeX/]MPM[";
const size_t MPM_ROOT_PATH_LEN_ = 14;
#endif

#define MPM_ROOT_PATH                           \
  IsAdminMode()                                 \
  ? COMMON_MPM_ROOT_PATH                        \
  : USER_MPM_ROOT_PATH

#if defined(MIKTEX_DEBUG)
#define MPM_ROOT_PATH_LEN                                               \
  static_cast<size_t>(MIKTEX_ASSERT(MiKTeX::Util::StrLen(MPM_ROOT_PATH) \
                             == MPM_ROOT_PATH_LEN_),                    \
                      MPM_ROOT_PATH_LEN_)
#else
const size_t MPM_ROOT_PATH_LEN = MPM_ROOT_PATH_LEN_;
#endif

#if defined(ENABLE_OPENSSL)
using BIO_ptr = std::unique_ptr<BIO, decltype(&::BIO_free)>;
#if defined(LIBRESSL_VERSION_NUMBER) || OPENSSL_VERSION_NUMBER < 0x10100000L
using EVP_MD_CTX_ptr = std::unique_ptr<EVP_MD_CTX, decltype(&::EVP_MD_CTX_destroy)>;
#else
using EVP_MD_CTX_ptr = std::unique_ptr<EVP_MD_CTX, decltype(&::EVP_MD_CTX_free)>;
#endif
using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)>;
using RSA_ptr = std::unique_ptr<RSA, decltype(&::RSA_free)>;
void FatalOpenSSLError();
RSA_ptr LoadPublicKey_OpenSSL(const MiKTeX::Core::PathName& publicKeyFile);
#endif

void CreateDirectoryPath(const MiKTeX::Core::PathName& path);

bool FileIsOnROMedia(const char* path);

bool GetCrtErrorMessage(int functionResult, std::string& errorMessage);

MiKTeX::Core::PathName GetFullPath(const char* path);

MiKTeX::Core::PathName GetHomeDirectory();

bool FixProgramSearchPath(const std::string& oldPath, const MiKTeX::Core::PathName& binDir_, bool checkCompetition, std::string& newPath, bool& competition);

#if defined(MIKTEX_WINDOWS)
bool GetUserProfileDirectory(MiKTeX::Core::PathName& path);
#endif

#if defined(MIKTEX_WINDOWS)
void SetTimesInternal(HANDLE handle, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);
#endif

#if defined(MIKTEX_WINDOWS)
bool GetSystemFontDirectory(MiKTeX::Core::PathName& path);
#endif

#if defined(MIKTEX_WINDOWS)
bool GetUserFontDirectory(MiKTeX::Core::PathName& path);
#endif

#if defined(MIKTEX_WINDOWS)
bool GetWindowsErrorMessage(unsigned long functionResult, std::string& errorMessage);
#endif

const char* GetFileNameExtension(const char* path);

enum class CryptoLib
{
  None,
  OpenSSL
};

CryptoLib GetCryptoLib();

bool HaveEnvironmentString(const char* name);

bool GetEnvironmentString(const std::string& name, std::string& value);

bool IsExplicitlyRelativePath(const char* path);

std::string MakeSearchPath(const std::vector<MiKTeX::Core::PathName>& vec);

void RemoveDirectoryDelimiter(char* path);

#if defined(MIKTEX_WINDOWS) && REPORT_EVENTS
bool ReportMiKTeXEvent(unsigned short eventType, unsigned long eventId, ...);
#endif

void TraceError(const char* format, ...);

#if defined(MIKTEX_WINDOWS)
void TraceWindowsError(const char* windowsFunction, unsigned long functionResult, const char* info, const char* sourceFileName, int sourceLine);
#endif

const char* GetShortSourceFile(const char* sourceFileName);

inline void DbgView(const std::string& s)
{
#if defined(_WIN32)
  OutputDebugStringW(UW_("MiKTeX Core: " + s));
#endif
}

class memstreambuf :
  public std::streambuf
{
public:
  memstreambuf(const unsigned char* mem, size_t len)
  {
    char* s = reinterpret_cast<char*>(const_cast<unsigned char*>(mem));
    setg(s, s, s + len);
  }
};

template<class VALTYPE> class AutoRestore
{
public:
  AutoRestore(VALTYPE& val) :
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
  VALTYPE* pVal;
};

template<class VALTYPE> class Optional
{
public:
  Optional() :
    hasValue(false)
  {
  }
public:
  Optional(const VALTYPE& value) :
    value(value),
    hasValue(true)
  {
  }
public:
  bool HasValue() const
  {
    return hasValue;
  }
public:
  operator bool() const
  {
    return HasValue();
  }
public:
  const VALTYPE& GetValue() const
  {
    if (!hasValue)
    {
      MIKTEX_UNEXPECTED();
    }
    return value;
  }
public:
  const VALTYPE& operator*() const
  {
    return GetValue();
  }
private:
  bool hasValue;
private:
  VALTYPE value;
};

struct StringComparerIgnoringCase :
  public std::binary_function<std::string, std::string, bool>
{
  bool operator()(const std::string& lhs, const std::string& rhs) const
  {
    return MiKTeX::Util::StringCompare(lhs.c_str(), rhs.c_str(), true) < 0;
  }
};

inline FILE* FdOpen(const MiKTeX::Core::PathName& path, int fd, const char* mode)
{
#if defined(_MSC_VER)
  FILE* stream = _fdopen(fd, mode);
#else
  FILE* stream = fdopen(fd, mode);
#endif
  if (stream == nullptr)
  {
    MIKTEX_FATAL_CRT_ERROR_2("fdopen", "path", path.ToString());
  }
  return stream;
}

inline FILE* FdOpen(int fd, const char* mode)
{
#if defined(_MSC_VER)
  FILE* stream = _fdopen(fd, mode);
#else
  FILE* stream = fdopen(fd, mode);
#endif
  if (stream == nullptr)
  {
    MIKTEX_FATAL_CRT_ERROR("fdopen");
  }
  return stream;
}

/* _________________________________________________________________________ */

CORE_INTERNAL_END_NAMESPACE;

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
