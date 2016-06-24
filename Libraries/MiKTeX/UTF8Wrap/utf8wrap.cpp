/* utf8wrap.cpp:

   Copyright (C) 2011-2016 Christian Schenk

   This file is part of the MiKTeX UTF8Wrap Library.

   The MiKTeX UTF8Wrap Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UTF8Wrap Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UTF8Wrap Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include <Windows.h>
#include <cstdarg>
#include <exception>
#include <memory>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include "internal.h"

using namespace std;

static char errorMessage[200];

MIKTEXSTATICFUNC(unique_ptr<wchar_t[]>) UTF8ToWideChar(const char * lpszUtf8)
{
  int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, lpszUtf8, -1, nullptr, 0);
  if (len <= 0)
  {
    sprintf_s(errorMessage, "MultiByteToWideChar() did not succeed; last error code is %d", (int)::GetLastError());
    throw std::runtime_error(errorMessage);
  }
  unique_ptr<wchar_t[]> buf(new wchar_t[len]);
  len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, lpszUtf8, -1, buf.get(), len);
  if (len <= 0)
  {
    sprintf_s(errorMessage, "MultiByteToWideChar() did not succeed; last error code is %d", (int)::GetLastError());
    throw std::runtime_error(errorMessage);
  }
  return buf;
}

MIKTEXSTATICFUNC(unique_ptr<char[]>) WideCharToUTF8(const wchar_t * lpszWideChar)
{
  int len = WideCharToMultiByte(CP_UTF8, 0, lpszWideChar, -1, nullptr, 0, nullptr, nullptr);
  if (len <= 0)
  {
    sprintf_s(errorMessage, "WideCharToMultiByte() did not succeed; last error code is %d", (int)::GetLastError());
    throw std::runtime_error(errorMessage);
  }
  unique_ptr<char[]> buf(new char[len]);
  len = WideCharToMultiByte(CP_UTF8, 0, lpszWideChar, -1, buf.get(), len, nullptr, nullptr);
  if (len <= 0)
  {
    sprintf_s(errorMessage, "WideCharToMultiByte() did not succeed; last error code is %d", (int)::GetLastError());
    throw std::runtime_error(errorMessage);
  }
  return buf;
};

#define UW_(x) UTF8ToWideChar(x).get()
#define WU_(x) WideCharToUTF8(x).get()

MIKTEXUTF8WRAPCEEAPI(FILE *) miktex_utf8_fopen(const char * lpszFileName, const char * lpszMode)
{
  return _wfopen(UW_(lpszFileName), UW_(lpszMode));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__open(const char * lpszFileName, int flags, ...)
{
  int pmode = 0;
  if ((flags & _O_CREAT) != 0)
  {
    va_list ap;
    va_start(ap, flags);
    pmode = va_arg(ap, int);
    va_end(ap);
  }
  return _wopen(UW_(lpszFileName), flags, pmode);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__stat64i32(const char * lpszFileName, struct _stat64i32 * pStat)
{
  return _wstat(UW_(lpszFileName), pStat);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__unlink(const char * lpszFileName)
{
  return _wunlink(UW_(lpszFileName));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_remove(const char * lpszFileName)
{
  return _wremove(UW_(lpszFileName));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__access(const char * lpszFileName, int mode)
{
  return _waccess(UW_(lpszFileName), mode);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__chmod(const char * lpszFileName, int mode)
{
  return _wchmod(UW_(lpszFileName), mode);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__mkdir(const char * lpszDirectoryName)
{
  return _wmkdir(UW_(lpszDirectoryName));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__rmdir(const char * lpszDirectoryName)
{
  return _wrmdir(UW_(lpszDirectoryName));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__chdir(const char * lpszDirectoryName)
{
  return _wchdir(UW_(lpszDirectoryName));
}

MIKTEXUTF8WRAPCEEAPI(char *) miktex_utf8__getcwd(char * lpszDirectoryName, size_t maxSize)
{
  unique_ptr<wchar_t[]> wideChar(new wchar_t[maxSize]);
  if (_wgetcwd(wideChar.get(), maxSize) == nullptr)
  {
    return nullptr;
  }
  unique_ptr<char[]> utf8(WideCharToUTF8(wideChar.get()));
  if (strlen(utf8.get()) >= maxSize)
  {
    throw std::runtime_error("buffer too small");
  }
  strcpy(lpszDirectoryName, utf8.get());
  return lpszDirectoryName;
}

static unordered_map<string, unique_ptr<char[]>> env;

MIKTEXUTF8WRAPCEEAPI(char *) miktex_utf8_getenv(const char * lpszName)
{
  wchar_t * lpszWideChar = _wgetenv(UW_(lpszName));
  if (lpszWideChar == nullptr)
  {
    return nullptr;
  }
  env[lpszName] = WideCharToUTF8(lpszWideChar);
  return env[lpszName].get();
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__utime64(const char * lpszFileName, struct __utimbuf64 * pTime)
{
  return _wutime64(UW_(lpszFileName), pTime);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_rename(const char * lpszOld, const char * lpszNew)
{
  return _wrename(UW_(lpszOld), UW_(lpszNew));
}

MIKTEXUTF8WRAPCEEAPI(intptr_t) miktex_utf8__spawnvp(int mode, const char * lpszFileName, const char * const * argv)
{
  vector<wstring> wideArguments;
  int count;
  for (count = 0; argv[count] != nullptr; ++count)
  {
    wideArguments.push_back(UW_(argv[count]));
  }
  vector<const wchar_t *> wargv;
  wargv.reserve(count + 1);
  for (int idx = 0; idx < count; ++idx)
  {
    wargv.push_back(wideArguments[idx].c_str());
  }
  wargv.push_back(0);
  return _wspawnvp(mode, UW_(lpszFileName), &wargv[0]);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_system(const char * lpszCommand)
{
  return _wsystem(lpszCommand == nullptr ? nullptr : UW_(lpszCommand));
}

MIKTEXUTF8WRAPCEEAPI(FILE *) miktex_utf8__popen(const char * lpszCommand, const char * lpszMode)
{
  return _wpopen(UW_(lpszCommand), UW_(lpszMode));
}

MIKTEXSTATICFUNC(HANDLE) GetConsoleHandle(FILE * pFile)
{
  bool isStdout = _fileno(pFile) == fileno(stdout);
  bool isStderr = _fileno(pFile) == fileno(stderr);
  if (_isatty(_fileno(pFile)) && (isStdout || isStderr))
  {
    return isStdout ? GetStdHandle(STD_OUTPUT_HANDLE) : GetStdHandle(STD_ERROR_HANDLE);
  }
  else
  {
    return INVALID_HANDLE_VALUE;
  }
}

#define MIKTEX_UTF8_CONSOLE_WARNING 0

#if MIKTEX_UTF8_CONSOLE_WARNING
class NonUtf8ConsoleWarning
{
public:
  NonUtf8ConsoleWarning()
    : pConsole(0)
  {
  }
public:
  ~NonUtf8ConsoleWarning()
  {
    const char * envvar = "MIKTEX_UTF8_SUPPRESS_CONSOLE_WARNING";
    if (pConsole != 0 && getenv(envvar) == 0)
    {
      fprintf(pConsole, "\
\n\n\
*** Warning:\n\
***\n\
*** Some characters could not be printed because the console's active code\n\
*** page is not UTF-8.\n\
***\n\
*** See http://miktex.org/howto/utf8-console for more information.\n\
***\n\
*** You can suppress this warning by setting the environment variable\n\
*** %s\n",
"MIKTEX_UTF8_SUPPRESS_CONSOLE_WARNING");
    }
  }
public:
  void Emit(FILE * pConsole)
  {
    this->pConsole = pConsole;
  }
private:
  FILE * pConsole;
};
#endif

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_fputc(int ch, FILE * pFile)
{
  HANDLE hConsole = GetConsoleHandle(pFile);
  if (hConsole == INVALID_HANDLE_VALUE)
  {
    return fputc(ch, pFile);
  }
  if (GetConsoleOutputCP() != CP_UTF8)
  {
#if MIKTEX_UTF8_CONSOLE_WARNING
    static NonUtf8ConsoleWarning warning;
    if ((ch & 0x80) != 0)
    {
      warning.Emit(pFile);
      ch = '?';
    }
#endif
    return fputc(ch, pFile);
  }
  static int remaining = 0;
  static char buf[5];
  static int bufidx = 0;
  if (bufidx > 4)
  {
    throw std::runtime_error("invalid UTF-8 byte sequence");
  }
  buf[bufidx++] = ch;
  if ((ch & 0x80) == 0)
  {
    remaining = 0;
  }
  else if ((ch & 0xc0) == 0x80)
  {
    if (remaining == 0)
    {
      throw std::runtime_error("invalid UTF-8 byte sequence");
    }
    --remaining;
  }
  else if ((ch & 0xe0) == 0xc0)
  {
    remaining = 1;
  }
  else if ((ch & 0xf0) == 0xe0)
  {
    remaining = 2;
  }
  else if ((ch & 0xf8) == 0xf0)
  {
    remaining = 3;
  }
  if (remaining == 0)
  {
    buf[bufidx] = 0;
    bufidx = 0;
    if (fputs(buf, pFile) < 0)
    {
      ch = EOF;
    }
  }
  return ch;
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_putc(int ch, FILE * pFile)
{
  return miktex_utf8_fputc(ch, pFile);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_putchar(int ch)
{
  return miktex_utf8_fputc(ch, stdout);
}

#if MIKTEX_UTF8_CONSOLE_WARNING
MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_fprintf(FILE * pFile, const char * lpszFormat, ...)
{
  HANDLE hConsole = GetConsoleHandle(pFile);
  va_list ap;
  va_start(ap, lpszFormat);
  int ret;
  if (hConsole == INVALID_HANDLE_VALUE || GetConsoleOutputCP() == CP_UTF8)
  {
    ret = vfprintf(pFile, lpszFormat, ap);
  }
  else
  {
    int n = _vscprintf(lpszFormat, ap);
    if (n >= 0)
    {
      char * pBuffer = new char[n + 1];
      n = vsprintf_s(pBuffer, n + 1, lpszFormat, ap);
      if (n >= 0)
      {
	// TODO: check buffer and warn if it contains UTF-8 bytes
	n = fputs(pBuffer, pFile);
      }
      delete[] pBuffer;
    }
  }
  va_end(ap);
  return ret;
}
#endif
