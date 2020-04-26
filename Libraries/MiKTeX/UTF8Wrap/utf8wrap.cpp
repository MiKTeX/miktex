/* utf8wrap.cpp:

   Copyright (C) 2011-2020 Christian Schenk

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
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include "internal.h"

#include <miktex/Util/PathNameUtil>
#include <miktex/Util/StringUtil>

using namespace std;

using namespace MiKTeX::Util;

class utf8wraperror :
  public std::exception
{
public:
  utf8wraperror(const char* function, const char* utf8)
  {
    sprintf_s(errorMessage, "UTF-8 conversion failed; func=%s, utf8=\"%s\"", function, utf8);
  }
public:
  utf8wraperror(const char* function, const wchar_t* wch)
  {
    sprintf_s(errorMessage, "UTF-8 conversion failed; func=%s, LPCWCH=\"%S\"", function, wch);
  }
public:
  const char* what() const override
  {
    return errorMessage;
  }
private:
  char errorMessage[2048];
};

MIKTEXSTATICFUNC(unique_ptr<wchar_t[]>) UTF8ToLengthExtendedPath(const char* utf8String, const char* function)
{
  try
  {
    wstring wch = PathNameUtil::ToLengthExtendedPathName(utf8String);
    unique_ptr<wchar_t[]> buf(new wchar_t[wch.length() + 1]);
    StringUtil::CopyString(buf.get(), wch.length() + 1, wch.c_str());
    return buf;
  }
  catch (const exception&)
  {
    throw utf8wraperror(function, utf8String);
  }
}


MIKTEXSTATICFUNC(unique_ptr<wchar_t[]>) UTF8ToWideChar(const char* utf8String, const char* function)
{
  try
  {
    wstring wch = StringUtil::UTF8ToWideChar(utf8String);
    unique_ptr<wchar_t[]> buf(new wchar_t[wch.length() + 1]);
    StringUtil::CopyString(buf.get(), wch.length() + 1, wch.c_str());
    return buf;
  }
  catch (const exception&)
  {
    throw utf8wraperror(function, utf8String);
  }
}

MIKTEXSTATICFUNC(unique_ptr<char[]>) WideCharToUTF8(const wchar_t* wideCharString, const char* function)
{
  try
  {
    string utf8 = StringUtil::WideCharToUTF8(wideCharString);
    unique_ptr<char[]> buf(new char[utf8.length() + 1]);
    StringUtil::CopyString(buf.get(), utf8.length() + 1, utf8.c_str());
    return buf;
  }
  catch (const exception&)
  {
    throw utf8wraperror(function, wideCharString);
  }
};

#define EXPATH_(x) UTF8ToLengthExtendedPath(x, __func__).get()
#define UW_(x) UTF8ToWideChar(x, __func__).get()
#define WU_(x) WideCharToUTF8(x, __func__).get()

MIKTEXUTF8WRAPCEEAPI(FILE*) miktex_utf8_fopen(const char* path, const char* mode)
{
  return _wfopen(EXPATH_(path), UW_(mode));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__open(const char* path, int flags, ...)
{
  int pmode = 0;
  if ((flags & _O_CREAT) != 0)
  {
    va_list ap;
    va_start(ap, flags);
    pmode = va_arg(ap, int);
    va_end(ap);
  }
  return _wopen(EXPATH_(path), flags, pmode);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__stat64i32(const char* path, struct _stat64i32* statBuf)
{
  return _wstat(EXPATH_(path), statBuf);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__unlink(const char* path)
{
  return _wunlink(EXPATH_(path));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_remove(const char* path)
{
  return _wremove(EXPATH_(path));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__access(const char* path, int mode)
{
  return _waccess(EXPATH_(path), mode);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__chmod(const char* path, int mode)
{
  return _wchmod(EXPATH_(path), mode);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__mkdir(const char* path)
{
  return _wmkdir(EXPATH_(path));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__rmdir(const char* path)
{
  return _wrmdir(EXPATH_(path));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__chdir(const char* path)
{
  return _wchdir(EXPATH_(path));
}

MIKTEXUTF8WRAPCEEAPI(char*) miktex_utf8__getcwd(char* path, size_t maxSize)
{
  unique_ptr<wchar_t[]> wideChar(new wchar_t[maxSize]);
  if (_wgetcwd(wideChar.get(), static_cast<int>(maxSize)) == nullptr)
  {
    return nullptr;
  }
  unique_ptr<char[]> utf8(WideCharToUTF8(wideChar.get(), __func__));
  if (strlen(utf8.get()) >= maxSize)
  {
    throw std::runtime_error("buffer too small");
  }
  strcpy_s(path, maxSize, utf8.get());
  return path;
}

static unordered_map<string, unique_ptr<char[]>> env;

MIKTEXUTF8WRAPCEEAPI(char*) miktex_utf8_getenv(const char* name)
{
  wchar_t* wideCharString = _wgetenv(UW_(name));
  if (wideCharString == nullptr)
  {
    return nullptr;
  }
  env[name] = WideCharToUTF8(wideCharString, __func__);
  return env[name].get();
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_putenv(const char* envString)
{
  return _wputenv(UW_(envString));
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__utime64(const char* path, struct __utimbuf64* timeBuf)
{
  return _wutime64(EXPATH_(path), timeBuf);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_rename(const char* oldName, const char* newName)
{
  return _wrename(EXPATH_(oldName), EXPATH_(newName));
}

MIKTEXUTF8WRAPCEEAPI(intptr_t) miktex_utf8__spawnvp(int mode, const char* path, const char* const* argv)
{
  vector<wstring> wideArguments;
  int count;
  for (count = 0; argv[count] != nullptr; ++count)
  {
    wideArguments.push_back(UW_(argv[count]));
  }
  vector<const wchar_t*> wargv;
  wargv.reserve(count + 1);
  for (int idx = 0; idx < count; ++idx)
  {
    wargv.push_back(wideArguments[idx].c_str());
  }
  wargv.push_back(nullptr);
  return _wspawnvp(mode, EXPATH_(path), &wargv[0]);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_system(const char* command)
{
  return _wsystem(command == nullptr ? nullptr : UW_(command));
}

MIKTEXUTF8WRAPCEEAPI(FILE*) miktex_utf8__popen(const char* command, const char* mode)
{
  return _wpopen(UW_(command), UW_(mode));
}

MIKTEXSTATICFUNC(HANDLE) GetConsoleHandle(FILE* file)
{
  bool isStdout = _fileno(file) == _fileno(stdout);
  bool isStderr = _fileno(file) == _fileno(stderr);
  if (_isatty(_fileno(file)) && (isStdout || isStderr))
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
    : console(console)
  {
  }
public:
  ~NonUtf8ConsoleWarning()
  {
    const char* envvar = "MIKTEX_UTF8_SUPPRESS_CONSOLE_WARNING";
    if (console != nullptr && getenv(envvar) == nullptr)
    {
      fprintf(console, "\
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
  void Emit(FILE* console)
  {
    this->console = console;
  }
private:
  FILE* console;
};
#endif

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_fputc(int ch, FILE* file)
{
  HANDLE hConsole = GetConsoleHandle(file);
  if (hConsole == INVALID_HANDLE_VALUE)
  {
    return fputc(ch, file);
  }
  if (GetConsoleOutputCP() != CP_UTF8)
  {
#if MIKTEX_UTF8_CONSOLE_WARNING
    static NonUtf8ConsoleWarning warning;
    if ((ch & 0x80) != 0)
    {
      warning.Emit(file);
      ch = '?';
    }
#endif
    return fputc(ch, file);
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
    if (fputs(buf, file) < 0)
    {
      ch = EOF;
    }
  }
  return ch;
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_putc(int ch, FILE* file)
{
  return miktex_utf8_fputc(ch, file);
}

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_putchar(int ch)
{
  return miktex_utf8_fputc(ch, stdout);
}

#if MIKTEX_UTF8_CONSOLE_WARNING
MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_fprintf(FILE* file, const char* format, ...)
{
  HANDLE hConsole = GetConsoleHandle(file);
  va_list ap;
  va_start(ap, format);
  int ret;
  if (hConsole == INVALID_HANDLE_VALUE || GetConsoleOutputCP() == CP_UTF8)
  {
    ret = vfprintf(file, format, ap);
  }
  else
  {
    int n = _vscprintf(format, ap);
    if (n >= 0)
    {
      char* buffer = new char[n + 1];
      n = vsprintf_s(buffer, n + 1, format, ap);
      if (n >= 0)
      {
        // TODO: check buffer and warn if it contains UTF-8 bytes
        n = fputs(buffer, file);
      }
      delete[] buffer;
    }
  }
  va_end(ap);
  return ret;
}
#endif
