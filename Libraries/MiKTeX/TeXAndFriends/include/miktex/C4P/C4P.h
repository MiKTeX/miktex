/* miktex/C4P/C4P.h: Pascalish run-time support         -*- C++ -*-

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

#if !defined(D8041ED6B5CDA942BA2AEEA8E29FD1D9)
#define D8041ED6B5CDA942BA2AEEA8E29FD1D9

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

#include <miktex/Core/File>
#include <miktex/Core/IntegerTypes>
#include <miktex/Core/Session>

#include <miktex/Util/StringUtil>
#include <miktex/Util/inliners.h>

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

// assert (sizeof(bool) == 1)
typedef bool C4P_boolean;

struct FileRoot
{
protected:
  FILE * pFile = nullptr;

protected:
  unsigned flags = 0;

public:
  void AssertValid() const
  {
    MIKTEX_ASSERT(pFile != nullptr);
  }

public:
  C4PTHISAPI(bool) Open(const char * lpszName, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access, MiKTeX::Core::FileShare share, bool text, bool mustExist);

public:
  void Close()
  {
    AssertValid();
    FILE * pFile = this->pFile;
    this->pFile = nullptr;
    std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
    session->CloseFile(pFile);
  }

public:
  void Attach(FILE * pFile, bool takeOwnership)
  {
    MIKTEX_ASSERT(takeOwnership);
    flags = 0;
    this->pFile = pFile;
  }

public:
  operator FILE * ()
  {
    return pFile;
  }

public:
  FILE * & fileref()
  {
    flags = 0;
    return pFile;
  }

public:
  FILE * operator -> ()
  {
    return pFile;
  }
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
  const ElementType & bufref() const
  {
    MIKTEX_ASSERT(IsPascalFileIO());
    return currentElement;
  }

public:
  ElementType & bufref()
  {
    PascalFileIO(true);
    return currentElement;
  }

public:
  const ElementType & operator * () const
  {
    return bufref();
  }

public:
  ElementType & operator * ()
  {
    return bufref();
  }

public:
  bool Eof()
  {
    if (feof(pFile) != 0)
    {
      return true;
    }

    if (IsPascalFileIO())
    {
      return false;
    }

    int lookAhead = getc(pFile);

    if (lookAhead == EOF)
    {
      if (ferror(pFile) != 0)
      {
        MIKTEX_FATAL_CRT_ERROR("getc");
      }
      return true;
    }

    if (ungetc(lookAhead, pFile) != lookAhead)
    {
      MIKTEX_FATAL_CRT_ERROR("ungetc");
    }

    return false;
  }

protected:
  void ReadInternal(ElementType * pBuf, std::size_t n)
  {
    AssertValid();
    MIKTEX_ASSERT_BUFFER(pBuf, n);
    if (feof(*this) != 0)
    {
      MIKTEX_FATAL_ERROR(MIKTEXTEXT("Read operation failed."));
    }
    if (fread(pBuf, sizeof(ElementType), n, *this) != n)
    {
      MIKTEX_FATAL_ERROR(MIKTEXTEXT("Read operation failed."));
    }
    if (ferror(*this) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR("ferror");
    }
  }

public:
  void Read(ElementType * pBuf, std::size_t n)
  {
    ReadInternal(pBuf, n);
    if (IsPascalFileIO())
    {
      currentElement = pBuf[n - 1];
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
      MIKTEX_FATAL_CRT_ERROR("fwrite");
    }
  }

public:
  void Seek(long offset, int origin)
  {
    AssertValid();
    if (fseek(*this, offset, origin) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fseek");
    }
    if (IsPascalFileIO() && !(origin == SEEK_END && offset == 0))
    {
      Read();
    }
  }
};

#define C4P_FILE_STRUCT(Bt) C4P::BufferedFile<Bt>

typedef C4P_FILE_STRUCT(char) C4P_text;

#if defined(C4P_REAL_IS_DOUBLE)
typedef double C4P_real;
#else
typedef float C4P_real;
#endif

typedef double C4P_longreal;

C4PCEEAPI(int) GetArgC();

C4PCEEAPI(const char **) GetArgV();

C4PCEEAPI(const char *) GetCmdLine();

C4PCEEAPI(char *) GetProgName(char * lpsz);

C4PCEEAPI(C4P_text *) GetStdFilePtr(unsigned idx);

#define c4pargc C4P::GetArgC()
#define c4pargv C4P::GetArgV()

#define c4ppline C4P::GetCmdLine()
#define c4pplen strlen(C4P::GetCmdLine())

#define input (*(C4P::GetStdFilePtr(0)))
#define output (*(C4P::GetStdFilePtr(1)))
#define c4perroroutput (*(C4P::GetStdFilePtr(2)))

class Program
{
public:
  C4PEXPORT MIKTEXTHISCALL Program(const char * lpszName, int argc, const char ** argv);

public:
  virtual ~Program()
  {
    try
    {
      if (running)
      {
        Finish();
      }
    }
    catch (const std::exception &)
    {
    }
  }

public:
  C4PTHISAPI(void) Finish();

private:
  bool running;
};

#define C4P_BEGIN_PROGRAM(lpszName, argc, argv)         \
  {                                                     \
    int c4p_retcode = 0;                                \
    try                                                 \
    {                                                   \
      C4P::Program c4p_program (lpszName, argc, argv);

#define C4P_END_PROGRAM()                       \
      c4p_program.Finish();                     \
    }                                           \
    catch (int retcode)                         \
    {                                           \
      c4p_retcode = retcode;                    \
    }                                           \
    return c4p_retcode;                         \
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

const C4P_integer maxint = INT_MAX;

C4PCEEAPI(void) DiscardLine(C4P_text & textfile);

C4PCEEAPI(char) GetChar(C4P_text & textfile);

C4PCEEAPI(int) GetInteger(C4P_text & textfile);

C4PCEEAPI(C4P_real) GetReal(C4P_text & textfile);

#define C4P_READ_BEGIN() {
#define C4P_READLN_BEGIN() C4P_READ_BEGIN()

template<class Ft> inline void c4p_read_f(Ft &, Ft &)
{
}

template<class Vt, class Ft> inline void c4p_read_v(Vt & v, Ft & f)
{
  f.AssertValid();
  v = *f;
  get(f);
}

template<class Vt, class Ft> inline void c4p_read_c(Vt & v, Ft & f)
{
  f.AssertValid();
  v = GetChar(f);
}

template<class Vt, class Ft> inline void c4p_read_i(Vt & v, Ft & f)
{
  f.AssertValid();
  v = GetInteger(f);
}

template<class Vt, class Ft> inline void c4p_read_r(Vt & v, Ft & f)
{
  f.AssertValid();
  v = GetReal(f);
}

#define C4P_READ_END(f) }
#define C4P_READLN_END(f) C4P::DiscardLine(f); }

inline void c4p_readln()
{
  input.AssertValid();
  DiscardLine(input);
}

#define C4P_WRITE_BEGIN() {
#define C4P_WRITELN_BEGIN() C4P_WRITE_BEGIN()

template<class Ft> inline void c4p_write_f(Ft &, Ft &)
{
}

template<class Vt, class Ft> inline void c4p_write_v(Vt v, Ft & f)
{
  f.AssertValid();
  *f = v;
  put(f);
}

template<class Vt, class Ft> inline void c4p_write_c(Vt v, Ft & f)
{
  f.AssertValid();
  int ch = putc(v, f);
  if (ch == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("putc");
  }
}

template<class Vt, class Ft> inline void c4p_write_c1(Vt v, Ft & f)
{
  return c4p_write_c(v, f);
}

template<class Vt, class Ft> inline void c4p_write_i(Vt v, Ft & f)
{
  f.AssertValid();
  if (fprintf(f, "%ld", static_cast<long>(v)) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fprintf");
  }
}

template<class Vt, class Ft> inline void c4p_write_i1(Vt v, int w1, Ft & f)
{
  f.AssertValid();
  if (fprintf(f, "%*ld", static_cast<int>(w1), static_cast<long>(v)) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fprintf");
  }

}

template<class Vt, class Ft> inline void c4p_write_i2(Vt v, int w1, int w2, Ft & f)
{
  f.AssertValid();
  if (fprintf(f, "%*.*ld", static_cast<int>(w1), static_cast<int>(w2), static_cast<long>(v)) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fprintf");
  }
}

template<class Vt, class Ft> inline void c4p_write_r(Vt v, Ft & f)
{
  f.AssertValid();
  if (fprintf(f, "%f", static_cast<float>(v)) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fprintf");
  }
}

template<class Vt, class Ft> inline void c4p_write_r2(Vt v, int w1, int w2, Ft & f)
{
  f.AssertValid();
  if (fprintf(f, "%*.*f", static_cast<int>(w1), static_cast<int>(w2), static_cast<float>(v)) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fprintf");
  }
}

template<class Vt, class Ft> inline void c4p_write_s(Vt v, Ft & f)
{
  f.AssertValid();
  if (fputs(v, f) == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("fputs");
  }
}

#define C4P_WRITE_END(f) }
#define C4P_WRITELN_END(f) putc('\n', f); }

inline void c4p_writeln()
{
  output.AssertValid();
  if (putc('\n', output) == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("putc");
  }
}

template<class T> inline void c4pmget(T & f, typename T::ElementType * buf, std::size_t n)
{
  f.AssertValid();
  f.Read(buf, n);
}

template<class T> inline char c4pgetc(T & f)
{
  f.AssertValid();
  return GetChar(f);
}

template<class T> inline void c4pputc(T & f)
{
  f.AssertValid();
  if (putc(*f, f) == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("putc");
  }
}

template<class T> inline void c4p_break(T & f)
{
  f.AssertValid();
  if (fflush(f) == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("flush");
  }
}

template<class Ft> inline void c4pfseek(Ft & f, int offs, int orig)
{
  f.AssertValid();
  f.Seek(offs, orig);
}

template<class Ft> inline long c4pftell(Ft & f)
{
  f.AssertValid();
  long n = ftell(f);
  if (n < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("ftell");
  }
  return n;
}

template<class Ft> inline void c4pbufwrite(Ft & f, const void * buf, std::size_t buf_size)
{
  f.AssertValid();
  //MIKTEX_ASSERT_BUFFER (buf, buf_size);
  if (fwrite(buf, buf_size, 1, f) != 1)
  {
    MIKTEX_FATAL_CRT_ERROR("fwrite");
  }
}

#define c4parrcpy(dst, src) c4pmemcpy(dst, sizeof(dst), src, sizeof(dst))

inline void c4pmemcpy(void * pdst, std::size_t size, const void * psrc, std::size_t count)
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

inline void c4pstrcpy(char * dst, std::size_t size, const char * src)
{
  MiKTeX::Util::StringUtil::CopyString(dst, size, src);
}

inline std::size_t c4pstrlen(const char * lpsz)
{
  MIKTEX_ASSERT_STRING(lpsz);
  return MiKTeX::Util::StrLen(lpsz);
}

template<class T> inline bool c4pfopen(T & f, const char * lpszName, const char * lpszMode, bool mustExist)
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
  return (f.Open(lpszName, (reading
    ? MiKTeX::Core::FileMode::Open
    : MiKTeX::Core::FileMode::Create), (reading
      ? MiKTeX::Core::FileAccess::Read
      : MiKTeX::Core::FileAccess::Write), (reading
        ? MiKTeX::Core::FileShare::Read
        : MiKTeX::Core::FileShare::None), text, mustExist));
}

template<class T> inline bool c4ptryfopen(T & f, const char * lpszName, const char * lpszMode)
{
  return c4pfopen(f, lpszName, lpszMode, false);
}

template<class T> inline void c4pfclose(T & f)
{
  f.close();
}

inline void c4pexit(int n)
{
  throw n;
}

#define c4preturn() goto C4P_LABEL_PROC_EXIT
#define c4pbreakloop() break

template<class T> T * c4pptr(T & obj)
{
  return &obj;
};

template<class T> inline void c4pincr(T & lvalue)
{
  ++lvalue;
}

template<class T> inline void c4pdecr(T & lvalue)
{
  --lvalue;
}

const int c4pcur = SEEK_CUR;
const int c4pend = SEEK_END;
const int c4pset = SEEK_SET;

const char * const c4prmode = "r";
const char * const c4pwmode = "w";
const char * const c4prbmode = "rb";
const char * const c4pwbmode = "wb";

C4PCEEAPI(unsigned) GetYear();

C4PCEEAPI(unsigned) GetMonth();

C4PCEEAPI(unsigned) GetDay();

C4PCEEAPI(unsigned) GetHour();

C4PCEEAPI(unsigned) GetMinute();

C4PCEEAPI(unsigned) GetSecond();

#define c4pyear C4P::GetYear()
#define c4pmonth C4P::GetMonth()
#define c4pday C4P::GetDay()
#define c4phour C4P::GetHour()
#define c4pminute C4P::GetMinute()
#define c4psecond C4P::GetSecond()

template<int handle> inline void c4p_proc_entry()
{
}

template<int handle> inline void c4p_proc_exit()
{
}

#define C4P_PROC_ENTRY(handle) c4p_proc_entry<handle>();
#define C4P_PROC_EXIT(handle) C4P_LABEL_PROC_EXIT: c4p_proc_exit<handle>();

C4PCEEAPI(void) SetStartUpTime(time_t time);

C4PCEEAPI(time_t) GetStartUpTime();

C4PCEEAPI(int) MakeCommandLine(const std::vector<std::string> & args);

C4PCEEAPI(C4P_integer) Round(double r);

inline double arctan(double x)
{
  return atan(x);
}

template<class T> inline int chr(T i)
{
  return i;
}

template<class T> inline C4P_boolean eof(T & f)
{
#if 0
  return feof(f) != 0;
#else
  return f.Eof();
#endif
}

template<class T> inline C4P_boolean eoln(T & f)
{
  return *f == '\n';
};

template<class T> inline void get(T & f)
{
  f.Read();
}

#if 1 // optimization?
template<> inline void get<BufferedFile<C4P_unsigned8> >(BufferedFile<C4P_unsigned8> & f)
{
  *f = getc(f);
  if (static_cast<int>(*f) == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("getc");
  }
}
#endif

inline double ln(double x)
{
  return log(x);
}

#if defined(odd)
#  undef odd
#endif

template<class T> inline C4P_boolean odd(T i)
{
  return (i & 1) != 0 ? true : false;
}

template<class T> inline int ord(T x)
{
  return x;
}

template<class T> inline T pred(T x)
{
  return x - 1;
}

template<class T> inline void put(T & f)
{
  f.Write();
}

template<class T> inline void reset(T & f)
{
  f.Reset();
}

template<class T> inline void rewrite(T & f)
{
  f.Rewrite();
}

inline double sqr(double x)
{
  return sqrt(x);
}

template<class T> inline T succ(T x)
{
  return x + 1;
}

C4P_END_NAMESPACE;

#endif
