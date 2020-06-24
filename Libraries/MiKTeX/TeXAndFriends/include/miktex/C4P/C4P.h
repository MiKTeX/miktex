/* miktex/C4P/C4P.h:                                    -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

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

/// @file miktex/C4P/C4P.h
/// @brief Pascalish run-time support.

#pragma once

#include <miktex/C4P/config.h>

#include <cctype>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <miktex/App/Application>

#include <miktex/Core/File>
#include <miktex/Core/IntegerTypes>
#include <miktex/Core/Session>

#include <miktex/Util/StringUtil>
#include <miktex/Util/inliners.h>

/// @namespace C4P
///
/// @brief The "C/C++ for Pascal" namespace.
///
/// This namespace contains C/C++ utilities to support the
/// Pascal-to-C++ conversion.
C4P_BEGIN_NAMESPACE;

typedef MIKTEX_INT8 C4P_signed8;
typedef MIKTEX_UINT8 C4P_unsigned8;
typedef MIKTEX_INT16 C4P_signed16;
typedef MIKTEX_UINT16 C4P_unsigned16;
typedef MIKTEX_INT32 C4P_signed32;
typedef MIKTEX_UINT32 C4P_unsigned32;
typedef MIKTEX_INT64 C4P_signed64;
typedef MIKTEX_UINT64 C4P_unsigned64;

typedef int C4P_integer;
typedef MIKTEX_INT64 C4P_longinteger;

#if defined(C4P_REAL_IS_DOUBLE)
typedef double C4P_real;
#else
typedef float C4P_real;
#endif

typedef double C4P_longreal;

// assert (sizeof(bool) == 1)
typedef bool C4P_boolean;

struct FileRoot
{
protected:
  FILE* file = nullptr;

protected:
  enum { NotOwner = 0x00000001 };
  
protected:
  unsigned flags = 0;

public:
  void AssertValid() const
  {
    MIKTEX_ASSERT(file != nullptr);
  }

public:
  C4PTHISAPI(bool) Open(const MiKTeX::Core::PathName& path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access, bool text, bool mustExist);

public:
  void Close()
  {
    AssertValid();
    FILE* file = this->file;
    this->file = nullptr;
    if ((flags & NotOwner) != 0)
    {
      std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
      session->CloseFile(file);
    }
  }

public:
  void Attach(FILE* file, bool takeOwnership)
  {
    flags = 0;
    if (!takeOwnership)
    {
      flags |= NotOwner;
    }
    this->file = file;
  }

public:
  operator FILE*()
  {
    return file;
  }

public:
  FILE*& fileref()
  {
    flags = 0;
    return file;
  }

public:
  FILE* operator->()
  {
    return file;
  }

protected:
  MiKTeX::Core::PathName path;
};

template<class T> struct BufferedFile :
  public FileRoot
{
public:
  typedef T ElementType;

protected:
  enum { Buffered = 0x00010000 };

public:
  void PascalFileIO(bool turnOn)
  {
    if (turnOn)
    {
      flags |= Buffered;
    }
    else
    {
      flags &= ~Buffered;
    }
  }

public:
  bool IsPascalFileIO() const
  {
    return (flags & Buffered) != 0;
  }

protected:
  ElementType currentElement;

public:
  const ElementType& bufref() const
  {
    MIKTEX_EXPECT(IsPascalFileIO());
    return currentElement;
  }

public:
  ElementType& bufref()
  {
    PascalFileIO(true);
    return currentElement;
  }

public:
  const ElementType& operator*() const
  {
    return bufref();
  }

public:
  ElementType& operator*()
  {
    return bufref();
  }

public:
  bool Eof()
  {
    if (feof(file) != 0)
    {
      return true;
    }

    if (IsPascalFileIO())
    {
      return false;
    }

    int lookAhead = getc(file);

    if (lookAhead == EOF)
    {
      if (ferror(file) != 0)
      {
        MIKTEX_FATAL_CRT_ERROR_2("getc", "path", path.ToString());
      }
      return true;
    }

    if (ungetc(lookAhead, file) != lookAhead)
    {
      MIKTEX_FATAL_CRT_ERROR_2("ungetc", "path", path.ToString());
    }

    return false;
  }

protected:
  void ReadInternal(ElementType* buf, std::size_t n)
  {
    AssertValid();
    MIKTEX_ASSERT_BUFFER(buf, n);
    if (feof(*this) != 0)
    {
      MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("Read operation failed: end of file reached"), "path", path.ToString(), "n", std::to_string(n));
    }
    size_t read = fread(buf, sizeof(ElementType), n, *this);
    if (ferror(*this) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("ferror", "path", path.ToString());
    }
    if (read != n)
    {
      MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("Read operation failed."), "path", path.ToString(), "read", std::to_string(read), "n", std::to_string(n));
    }
  }

public:
  void Read(ElementType* buf, std::size_t n)
  {
    ReadInternal(buf, n);
    if (IsPascalFileIO())
    {
      currentElement = buf[n - 1];
    }
  }

public:
  void Read()
  {
    PascalFileIO(true);
    ReadInternal(&currentElement, 1);
  }

public:
  void Reset()
  {
    AssertValid();
    rewind(*this);
    Read();
  }

public:
  void Rewrite()
  {
    AssertValid();
    rewind(*this);
  }

public:
  void Write()
  {
    AssertValid();
    MIKTEX_ASSERT(IsPascalFileIO());
    if (fwrite(&currentElement, sizeof(ElementType), 1, *this) != 1 || ferror(*this) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("fwrite", "path", path.ToString());
    }
  }

public:
  void Seek(long offset, int origin)
  {
    AssertValid();
    if (fseek(*this, offset, origin) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("fseek", "path", path.ToString(), "offset", std::to_string(offset), "origin", std::to_string(origin));
    }
    if (IsPascalFileIO() && !(origin == SEEK_END && offset == 0))
    {
      Read();
    }
  }
};

#define C4P_FILE_STRUCT(Bt) C4P::BufferedFile<Bt>

struct C4P_text :
  BufferedFile<char>
{
public:
  C4PTHISAPI(bool) IsTerminal();

public:
  C4PTHISAPI(void) DiscardLine();

public:
  C4PTHISAPI(char) GetChar();

public:
  C4PTHISAPI(int) GetInteger();

public:
  C4PTHISAPI(C4P_real) GetReal();
};

#define c4pargc GetArgC()
#define c4pargv GetArgV()

#define c4ppline GetCmdLine()
#define c4pplen strlen(GetCmdLine())

#define input (*(GetStdFilePtr(0)))
#define output (*(GetStdFilePtr(1)))
#define c4perroroutput (*(GetStdFilePtr(2)))

class C4PTYPEAPI(ProgramBase)
{
public:
  C4PEXPORT MIKTEXTHISCALL ProgramBase();

public:
  ProgramBase(const ProgramBase& other) = delete;

public:
  ProgramBase& operator=(const ProgramBase& other) = delete;

public:
  ProgramBase(ProgramBase&& other) = delete;

public:
  ProgramBase& operator=(ProgramBase&& other) = delete;

public:
  virtual C4PEXPORT MIKTEXTHISCALL ~ProgramBase() noexcept;

protected:
  C4PTHISAPI(void) Initialize(const char* programName, int argc, char* argv[]);

protected:
  C4PTHISAPI(void) Finish();

public:
  C4PTHISAPI(void) SetParent(MiKTeX::App::Application* parent);

public:
  C4PTHISAPI(int) MakeCommandLine(const std::vector<std::string>& args);

protected:
  C4PTHISAPI(const char*) GetCmdLine();

public:
  C4PTHISAPI(int) GetArgC();

public:
  C4PTHISAPI(const char**) GetArgV();

protected:
  C4PTHISAPI(unsigned) GetYear();

protected:
  C4PTHISAPI(unsigned) GetMonth();

protected:
  C4PTHISAPI(unsigned) GetDay();

protected:
  C4PTHISAPI(unsigned) GetHour();

protected:
  C4PTHISAPI(unsigned) GetMinute();

protected:
  C4PTHISAPI(unsigned) GetSecond();

public:
  C4PTHISAPI(void) SetStartUpTime(time_t time, bool useUtc);

public:
  C4PTHISAPI(time_t) GetStartUpTime();

protected:
  C4PTHISAPI(C4P_text*) GetStdFilePtr(unsigned idx);

protected:
  template<class Ft>
  void c4p_write_f(Ft&, Ft&)
  {
  }

protected:
  template<class Vt, class Ft>
  void c4p_write_v(Vt v, Ft& f)
  {
    f.AssertValid();
    *f = v;
    put(f);
  }

private:
  C4PTHISAPI(void) WriteChar(int ch, FILE* file);

protected:
  template<class Vt, class Ft>
  void c4p_write_c(Vt v, Ft& f)
  {
    f.AssertValid();
    WriteChar(v, f);
  }

protected:
  template<class Vt, class Ft>
  void c4p_write_c1(Vt v, Ft& f)
  {
    return c4p_write_c(v, f);
  }

protected:
  template<class Vt, class Ft>
  void c4p_write_i(Vt v, Ft& f)
  {
    f.AssertValid();
    if (fprintf(f, "%ld", static_cast<long>(v)) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fprintf");
    }
  }

protected:
  template<class Vt, class Ft>
  void c4p_write_i1(Vt v, int w1, Ft& f)
  {
    f.AssertValid();
    if (fprintf(f, "%*ld", static_cast<int>(w1), static_cast<long>(v)) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fprintf");
    }
  }

protected:
  template<class Vt, class Ft>
  void c4p_write_i2(Vt v, int w1, int w2, Ft& f)
  {
    f.AssertValid();
    if (fprintf(f, "%*.*ld", static_cast<int>(w1), static_cast<int>(w2), static_cast<long>(v)) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fprintf");
    }
  }

protected:
  template<class Vt, class Ft>
  void c4p_write_r(Vt v, Ft& f)
  {
    f.AssertValid();
    if (fprintf(f, "%f", static_cast<float>(v)) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fprintf");
    }
  }

protected:
  template<class Vt, class Ft>
  void c4p_write_r2(Vt v, int w1, int w2, Ft& f)
  {
    f.AssertValid();
    if (fprintf(f, "%*.*f", static_cast<int>(w1), static_cast<int>(w2), static_cast<float>(v)) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fprintf");
    }
  }

protected:
  template <class Vt, class Ft>
  void c4p_write_s(Vt v, Ft& f)
  {
    f.AssertValid();
    if (fputs(v, f) == EOF)
    {
      MIKTEX_FATAL_CRT_ERROR("fputs");
    }
  }

protected:
  template<class Ft>
  void c4p_read_f(Ft&, Ft&)
  {
  }

protected:
  template<class Vt, class Ft>
  void c4p_read_v(Vt& v, Ft& f)
  {
    f.AssertValid();
    v = *f;
    get(f);
  }

protected:
  template<class Vt, class Ft>
  void c4p_read_c(Vt& v, Ft& f)
  {
    f.AssertValid();
    v = f.GetChar();
  }

protected:
  template<class Vt, class Ft>
  void c4p_read_i(Vt& v, Ft& f)
  {
    f.AssertValid();
    v = f.GetInteger();
  }

protected:
  template<class Vt, class Ft>
  void c4p_read_r(Vt& v, Ft& f)
  {
    f.AssertValid();
    v = f.GetReal();
  }

protected:
  const C4P_integer maxint = INT_MAX;

protected:
  void c4p_readln()
  {
    input.AssertValid();
    input.DiscardLine();
  }

protected:
  void c4p_writeln()
  {
    output.AssertValid();
    if (putc('\n', output) == EOF)
    {
      MIKTEX_FATAL_CRT_ERROR("putc");
    }
  }

protected:
  template<class T>
  void c4pmget(T& f, typename T::ElementType* buf, std::size_t n)
  {
    f.AssertValid();
    f.Read(buf, n);
  }

protected:
  template<class T>
  char c4pgetc(T& f)
  {
    f.AssertValid();
    return f.GetChar();
  }

protected:
  template<class T>
  void c4pputc(T& f)
  {
    f.AssertValid();
    if (putc(*f, f) == EOF)
    {
      MIKTEX_FATAL_CRT_ERROR("putc");
    }
  }

protected:
  template<class T>
  void c4p_break(T& f)
  {
    f.AssertValid();
    if (fflush(f) == EOF)
    {
      MIKTEX_FATAL_CRT_ERROR("flush");
    }
  }

protected:
  template<class Ft>
  void c4pfseek(Ft& f, int offs, int orig)
  {
    f.AssertValid();
    f.Seek(offs, orig);
  }

protected:
  template<class Ft>
  long c4pftell(Ft& f)
  {
    f.AssertValid();
    long n = ftell(f);
    if (n < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("ftell");
    }
    return n;
  }

protected:
  template<class Ft>
  void c4pbufwrite(Ft& f, const void* buf, std::size_t buf_size)
  {
    f.AssertValid();
    //MIKTEX_ASSERT_BUFFER (buf, buf_size);
    if (fwrite(buf, buf_size, 1, f) != 1)
    {
      MIKTEX_FATAL_CRT_ERROR("fwrite");
    }
  }

protected:
  void c4pmemcpy(void* pdst, std::size_t size, const void* psrc, std::size_t count)
  {
    MIKTEX_ASSERT(size >= count);
    MIKTEX_ASSERT_BUFFER(pdst, size);
    //MIKTEX_ASSERT_BUFFER (psrc, count);
  #if defined(_MSC_VER) && (_MSC_VER > 1400)
    memcpy_s(pdst, size, psrc, count);
  #else
    memcpy(pdst, psrc, count);
  #endif
  }

protected:
  void c4pstrcpy(char* dst, std::size_t size, const char* src)
  {
    MiKTeX::Util::StringUtil::CopyString(dst, size, src);
  }

protected:
  std::size_t c4pstrlen(const char* lpsz)
  {
    MIKTEX_ASSERT_STRING(lpsz);
    return MiKTeX::Util::StrLen(lpsz);
  }

protected:
  template<class T>
  bool c4pfopen(T& f, const char* lpszName, const char* lpszMode, bool mustExist)
  {
    MIKTEX_ASSERT_STRING(lpszName);
    MIKTEX_ASSERT_STRING(lpszMode);
    MIKTEX_ASSERT
      (MiKTeX::Core::Utils::Equals(lpszMode, "r")
        || MiKTeX::Core::Utils::Equals(lpszMode, "rb")
        || MiKTeX::Core::Utils::Equals(lpszMode, "w")
        || MiKTeX::Core::Utils::Equals(lpszMode, "wb"));
    bool reading = (lpszMode[0] == 'r');
    bool text = (lpszMode[1] == 0);
    return f.Open(
      MiKTeX::Core::PathName(lpszName),
      (reading ? MiKTeX::Core::FileMode::Open : MiKTeX::Core::FileMode::Create),
      (reading ? MiKTeX::Core::FileAccess::Read : MiKTeX::Core::FileAccess::Write),
      text,
      mustExist);
  }

protected:
  template<class T>
  bool c4ptryfopen(T& f, const char* lpszName, const char* lpszMode)
  {
    return c4pfopen(f, lpszName, lpszMode, false);
  }

protected:
  template<class T>
  void c4pfclose(T& f)
  {
    f.Close();
  }

protected:
  void c4pexit(int n)
  {
    throw n;
  }

protected:
  template<class T>
  T* c4pptr(T& obj)
  {
    return &obj;
  };

protected:
  template<class T>
  void c4pincr(T& lvalue)
  {
    ++lvalue;
  }

protected:
  template<class T> void c4pdecr(T& lvalue)
  {
    --lvalue;
  }

protected:
  double c4p_abs(double x)
  {
    return ::fabs(x);
  }

protected:
  int c4p_abs(int x)
  {
    return ::abs(x);
  }

protected:
  double arctan(double x)
  {
    return atan(x);
  }

protected:
  template<class T>
  int chr(T i)
  {
    return i;
  }

protected:
  template<class T>
  C4P_boolean eof(T& f)
  {
  #if 0
    return feof(f) != 0;
  #else
    return f.Eof();
  #endif
  }

protected:
  template<class T>
  C4P_boolean eoln(T& f)
  {
    return *f == '\n';
  };

protected:
  template<class T>
  void get(T& f)
  {
    f.Read();
  }

protected:
#if 0 // optimization?
  template<>
  void get<BufferedFile<C4P_unsigned8>>(BufferedFile<C4P_unsigned8>& f)
  {
    *f = getc(f);
    if (static_cast<int>(*f) == EOF)
    {
      MIKTEX_FATAL_CRT_ERROR("getc");
    }
  }
#endif

protected:
  double ln(double x)
  {
    return log(x);
  }

protected:
#if defined(odd)
#  undef odd
#endif
  template<class T>
  C4P_boolean odd(T i)
  {
    return (i & 1) != 0 ? true : false;
  }

protected:
  template<class T>
  int ord(T x)
  {
    return x;
  }

protected:
  template<class T>
  T pred(T x)
  {
    return x - 1;
  }

protected:
  template<class T>
  void put(T& f)
  {
    f.Write();
  }

protected:
  template<class T>
  void reset(T& f)
  {
    f.Reset();
  }

protected:
  template<class T>
  void rewrite(T& f)
  {
    f.Rewrite();
  }

protected:
  double sqr(double x)
  {
    return sqrt(x);
  }

protected:
  template<class T>
  T succ(T x)
  {
    return x + 1;
  }

protected:
  template<int handle>
  void c4p_proc_entry()
  {
  }

protected:
  template<int handle>
  void c4p_proc_exit()
  {
  }

protected:
  const int c4pcur = SEEK_CUR;
  const int c4pend = SEEK_END;
  const int c4pset = SEEK_SET;

protected:
  const char* const c4prmode = "r";
  const char* const c4pwmode = "w";
  const char* const c4prbmode = "rb";
  const char* const c4pwbmode = "wb";

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

#define C4P_BEGIN_PROGRAM(programName, argc, argv)                                 \
  {                                                                                \
    int c4p_retcode = 0;                                                           \
    try                                                                            \
    {                                                                              \
      this->Initialize(programName, argc, argv);

#define C4P_END_PROGRAM()                                                          \
      this->Finish();                                                              \
    }                                                                              \
    catch(int retcode)                                                             \
    {                                                                              \
      c4p_retcode = retcode;                                                       \
    }                                                                              \
    return c4p_retcode;                                                            \
}

#define c4pbegintryblock(n)                     \
  try                                           \
  {

#define c4pendtryblock(n)                       \
    ;                                           \
  }                                             \
  catch (const C4P::Exception##n &)             \
  {                                             \
  }

#define c4pthrow(n) throw C4P::Exception##n()

#define C4P_FOR_BEGIN(var, startval, relop, lastval)    \
{                                                       \
  int c4p_temp = (lastval);                             \
  if ((var = (startval)) relop c4p_temp)                \
  {                                                     \
    do                                                  \
    {

#define C4P_FOR_END(var, op)                    \
    }                                           \
    while (var op != c4p_temp);                 \
  }                                             \
}

#define C4P_READ_BEGIN() {
#define C4P_READLN_BEGIN() C4P_READ_BEGIN()

#define C4P_READ_END(f) }
#define C4P_READLN_END(f) f.DiscardLine(); }

#define C4P_WRITE_BEGIN() {
#define C4P_WRITELN_BEGIN() C4P_WRITE_BEGIN()

#define C4P_WRITE_END(f) }
#define C4P_WRITELN_END(f) putc('\n', f); }

#define c4preturn() goto C4P_LABEL_PROC_EXIT
#define c4pbreakloop() break

#define c4parrcpy(dst, src) c4pmemcpy(dst, sizeof(dst), src, sizeof(dst))

#define c4pyear GetYear()
#define c4pmonth GetMonth()
#define c4pday GetDay()
#define c4phour GetHour()
#define c4pminute GetMinute()
#define c4psecond GetSecond()

#define C4P_PROC_ENTRY(handle) c4p_proc_entry<handle>();
#define C4P_PROC_EXIT(handle) C4P_LABEL_PROC_EXIT: c4p_proc_exit<handle>();

C4PCEEAPI(C4P_integer) Round(double r);

C4P_END_NAMESPACE;
