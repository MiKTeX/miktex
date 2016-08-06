/* miktex/TeXAndFriends/TeXMFApp.h:                     -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(FC3C8BED3A42414E8290C71E0981D437)
#define FC3C8BED3A42414E8290C71E0981D437

#include <miktex/TeXAndFriends/config.h>

#include <memory>
#include <string>

#include <csignal>
#include <cstddef>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/Debug>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/FileType>
#include <miktex/Core/PathName>
#include <miktex/Core/Quoter>
#include <miktex/Core/Registry>

#include <miktex/Trace/TraceStream>

#include <miktex/Util/CharBuffer>
#include <miktex/Util/StringUtil>
#include <miktex/Util/inliners.h>

#if defined(MIKTEX_TEX) || defined(MIKTEX_TRIPTEX)
#  define MIKTEX_TEX_COMPILER 1
#endif

#if defined(MIKTEX_PDFTEX)
#  define MIKTEX_TEX_COMPILER 1
#endif

#if defined(MIKTEX_XETEX)
#  define MIKTEX_TEX_COMPILER 1
#  define MIKTEX_TEXMF_UNICODE 1
#endif

#if defined(MIKTEX_OMEGA)
#  define MIKTEX_TEX_COMPILER 1
#  define MIKTEX_TEXMF_UNICODE 1
#endif

#if defined(MIKTEX_METAFONT)
#  define MIKTEX_META_COMPILER 1
#  define ENABLE_8BIT_CHARS 1
#  define HAVE_MAIN_MEMORY 1
#  define IMPLEMENT_TCX 1
#endif

#if defined(MIKTEX_TEX_COMPILER)
#    define HAVE_EXTRA_MEM_BOT 1
#    define HAVE_EXTRA_MEM_TOP 1
#    define HAVE_MAIN_MEMORY 1
#    define HAVE_POOL_FREE 1
#    define HAVE_STRINGS_FREE 1
#  if !(defined(MIKTEX_XETEX) || defined(MIKTEX_OMEGA))
#    define IMPLEMENT_TCX 1
#  endif
#  if !defined(MIKTEX_OMEGA)
#    define ENABLE_8BIT_CHARS 1
#  endif
#endif

#if defined(MIKTEX_BIBTEX)
#  define IMPLEMENT_TCX 1
#endif

#include "WebAppInputLine.h"

namespace texmfapp {
#include <miktex/texmfapp.defaults.h>
}

MIKTEXMF_BEGIN_NAMESPACE;

#if defined(MIKTEX_TEXMF_UNICODE)
typedef wchar_t TEXMFCHAR;
typedef wint_t TEXMFCHARINT;
#else
typedef char TEXMFCHAR;
typedef int TEXMFCHARINT;
#endif

#define GETPARAM(param, varname, cfgname, defcfgval)                    \
{                                                                       \
  if (param < 0)                                                        \
  {                                                                     \
    THEDATA(varname) = GetParameter(#cfgname, defcfgval);               \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    THEDATA(varname) = param;                                           \
  }                                                                     \
  if (trace_mem->IsEnabled())                                           \
  {                                                                     \
    trace_mem->WriteFormattedLine("libtexmf", MIKTEXTEXT("Parameter %s: %d"), #cfgname, (int)THEDATA(varname)); \
  }                                                                     \
}

#define GETPARAMCHECK(param, varname, cfgname, defcfgval)               \
{                                                                       \
  if (param < 0)                                                        \
  {                                                                     \
    THEDATA(varname) = GetParameter(#cfgname, defcfgval);               \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    THEDATA(varname) = param;                                           \
  }                                                                     \
  if (THEDATA(varname) < inf##varname || THEDATA(varname) > sup##varname) \
  {                                                                     \
    MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("Bad parameter value."), "cfgname", #cfgname); \
    }                                                                   \
  if (trace_mem->IsEnabled())                                           \
  {                                                                     \
    trace_mem->WriteFormattedLine("libtexmf", MIKTEXTEXT("Parameter %s: %d"), #cfgname, (int)THEDATA(varname)); \
  }                                                                     \
}

#if defined(THEDATA)
inline TEXMFCHAR * GetTeXString(TEXMFCHAR * lpsz, std::size_t size, int stringStart, int stringLength)
{
  MIKTEX_ASSERT(sizeof(THEDATA(strpool)[0]) == sizeof(lpsz[0]));
  if (stringLength < 0 || stringLength >= size)
  {
    MIKTEX_FATAL_ERROR(MIKTEXTEXT("Bad string size."));
  }
  for (int idx = 0; idx < stringLength; ++idx)
  {
    lpsz[idx] = THEDATA(strpool)[stringStart + idx];
  }
  lpsz[stringLength] = 0;
  return lpsz;
}
#endif

#if defined(THEDATA) && defined(MIKTEX_TEXMF_UNICODE)
inline char * GetTeXString(char * lpsz, std::size_t size, int stringStart, int stringLength)
{
  MiKTeX::Util::CharBuffer<wchar_t, 200> buf(stringLength + 1);
  GetTeXString(buf.GetData(), buf.GetCapacity(), stringStart, stringLength);
  MiKTeX::Util::StringUtil::CopyString(lpsz, size, buf.Get());
  return lpsz;
}
#endif

#if defined(THEDATA)
inline int GetTeXStringStart(int stringNumber)
{
#if defined(MIKTEX_OMEGA) || defined(MIKTEX_XETEX)
  MIKTEX_ASSERT(stringNumber >= 65536);
  stringNumber -= 65536;
#endif
  MIKTEX_ASSERT(stringNumber >= 0 && stringNumber < THEDATA(strptr));
#if defined(MIKTEX_OMEGA)
  int stringStart = THEDATA(strstartar)[stringNumber];
#else
  int stringStart = THEDATA(strstart)[stringNumber];
#endif
  return stringStart;
}
#endif

#if defined(THEDATA)
inline int GetTeXStringLength(int stringNumber)
{
#if defined(MIKTEX_OMEGA) || defined(MIKTEX_XETEX)
  MIKTEX_ASSERT(stringNumber >= 65536);
  stringNumber -= 65536;
#endif
  MIKTEX_ASSERT(stringNumber >= 0 && stringNumber < THEDATA(strptr));
#if defined(MIKTEX_OMEGA)
  int stringLength =
    (THEDATA(strstartar)[stringNumber + 1]
      - THEDATA(strstartar)[stringNumber]);
#else
  int stringLength =
    (THEDATA(strstart)[stringNumber + 1] - THEDATA(strstart)[stringNumber]);
#endif
  return stringLength;
}
#endif

#if defined(THEDATA)
template<typename CharType> inline CharType * GetTeXString(CharType * lpsz, int stringNumber, std::size_t size = 0xffff)
{
  int stringStart = GetTeXStringStart(stringNumber);
  int stringLength = GetTeXStringLength(stringNumber);
  return GetTeXString(lpsz, size, stringStart, stringLength);
}
#endif

class MIKTEXMFTYPEAPI(TeXMFApp) : public WebAppInputLine
{
public:
  MIKTEXMFTHISAPI(void) Init(const char * lpszProgramInvocationName) override;

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

protected:
  MIKTEXMFTHISAPI(void) AddOptions() override;

protected:
  const char * GetUsage() const override
  {
    return MIKTEXTEXT("[OPTION...] [COMMAND...]");
  }

public:
  MIKTEXMFTHISAPI(void) ProcessCommandLineOptions() override;

protected:
  MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string & optArg) override;

public:
  MIKTEXMFTHISAPI(void) TouchJobOutputFile(FILE * pFile) const override;

public:
  virtual const char * GetMemoryDumpFileExtension() const
  {
    // must be implemented in sub-classes
    MIKTEX_ASSERT(false);
    return nullptr;
  }

public:
  virtual MiKTeX::Core::FileType GetMemoryDumpFileType() const
  {
    // must be implemented in sub-classes
    MIKTEX_ASSERT(false);
    return MiKTeX::Core::FileType::None;
  }

protected:
  virtual const char * GetMemoryDumpFileName() const
  {
    // must be implemented in sub-classes
    MIKTEX_ASSERT(false);
    return nullptr;
  }

protected:
  virtual const char * GetInitProgramName() const
  {
    // must be implemented in sub-classes
    MIKTEX_ASSERT(false);
    return nullptr;
  }

protected:
  virtual const char * GetVirginProgramName() const
  {
    // must be implemented in sub-classes
    MIKTEX_ASSERT(false);
    return nullptr;
  }

public:
  virtual void OnTeXMFInitialize() const
  {
#if defined(THEDATA)
    signal(SIGINT, OnKeybordInterrupt);
#endif
  }

public:
  virtual MIKTEXMFTHISAPI(void) OnTeXMFStartJob();

public:
  virtual MIKTEXMFTHISAPI(void) OnTeXMFFinishJob();

#if defined(MIKTEX_DEBUG)
public:
  virtual void CheckMemory()
  {
#if defined(THEDATA)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(buffer));
#  if defined(MIKTEX_TEX_COMPILER)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(yzmem));
#else
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(mem));
#endif
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(paramstack));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(strpool));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(trickbuf));
#  if !defined(MIKTEX_OMEGA)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(strstart));
#  endif
#endif
  }
#endif

protected:
  template<typename ValueType> ValueType GetParameter(const char * lpszParameterName, const ValueType & defaultValue) const
  {
    ValueType value = session->GetConfigValue(nullptr, lpszParameterName, -1);
    if (value < 0)
    {
      value = session->GetConfigValue(GetProgramName(), lpszParameterName, defaultValue);
    }
    return value;
  }

public:
  template<typename T> T * Reallocate(const char * lpszArrayName, T * & p, std::size_t n, const char * lpszFileName, int line)
  {
    std::size_t amount;
    if (n == 0)
    {
      amount = 0;
    }
    else
    {
      // one extra element becaus Pascal arrays are 1-based
      amount = (n + 1) * sizeof(T);
    }
    if (trace_mem->IsEnabled())
    {
      trace_mem->WriteFormattedLine("libtexmf", MIKTEXTEXT("Reallocate %s: p == %p, elementSize == %u, nElements == %u, bytes == %u"), (lpszArrayName == nullptr ? "array" : lpszArrayName), p, (unsigned)sizeof(T), (unsigned)n, (unsigned)amount);
    }
    p = reinterpret_cast<T*>(MiKTeX::Debug::Realloc(p, amount, lpszFileName, line));
    if (trace_mem->IsEnabled())
    {
      trace_mem->WriteFormattedLine("libtexmf", MIKTEXTEXT("Reallocate: return %p"), p);
    }
    return p;
  }

protected:
  template<typename T> T * Allocate(const char * lpszArrayName, T * & p, std::size_t n)
  {
    p = nullptr;
    return Reallocate(lpszArrayName, p, n, __FILE__, __LINE__);
  }

protected:
  template<typename T> T * Allocate(T * & p, std::size_t n)
  {
    return Allocate(nullptr, p, n);
  }

protected:
  template<typename T> T * Free(const char * lpszArrayName, T * & p)
  {
    return Reallocate(lpszArrayName, p, 0, __FILE__, __LINE__);
  }

protected:
  template<typename T> T * Free(T * & p)
  {
    return Free(nullptr, p);
  }

#if defined(THEDATA)
public:
  void AllocateMemory()
  {
    GETPARAMCHECK(param_buf_size, bufsize, buf_size, texmfapp::texmfapp::buf_size());
    GETPARAMCHECK(param_error_line, errorline, error_line, texmfapp::texmfapp::error_line());
#if defined(HAVE_EXTRA_MEM_BOT)
    GETPARAM(param_extra_mem_bot, extramembot, extra_mem_bot, texmfapp::texmfapp::extra_mem_bot());
#endif
#if defined(HAVE_EXTRA_MEM_TOP)
    GETPARAM(param_extra_mem_top, extramemtop, extra_mem_top, texmfapp::texmfapp::extra_mem_top());
#endif
    GETPARAMCHECK(param_half_error_line, halferrorline, half_error_line, texmfapp::texmfapp::half_error_line());
#if defined(HAVE_MAIN_MEMORY)
#  if !defined(infmainmemory)
    const int infmainmemory = 3000;
    const int supmainmemory = 256000000;
#  endif
    GETPARAMCHECK(param_main_memory, mainmemory, main_memory, texmfapp::texmfapp::main_memory());
#endif
    GETPARAMCHECK(param_max_print_line, maxprintline, max_print_line, texmfapp::texmfapp::max_print_line());
    GETPARAMCHECK(param_max_strings, maxstrings, max_strings, texmfapp::texmfapp::max_strings());
#if !defined(infparamsize)
    const int infparamsize = 60;
    const int supparamsize = 600000;
#endif
    GETPARAMCHECK(param_param_size, paramsize, param_size, texmfapp::texmfapp::param_size());
#if defined(HAVE_POOL_FREE)
    GETPARAMCHECK(param_pool_free, poolfree, pool_free, texmfapp::texmfapp::pool_free());
#endif
    GETPARAMCHECK(param_pool_size, poolsize, pool_size, texmfapp::texmfapp::pool_size());
    GETPARAMCHECK(param_stack_size, stacksize, stack_size, texmfapp::texmfapp::stack_size());
#if defined(HAVE_STRINGS_FREE)
    GETPARAMCHECK(param_strings_free, stringsfree, strings_free, texmfapp::texmfapp::strings_free());
#endif
    GETPARAMCHECK(param_string_vacancies, stringvacancies, string_vacancies, texmfapp::texmfapp::string_vacancies());

    THEDATA(maxstrings) += 0x100;

#if defined(HAVE_EXTRA_MEM_BOT)
    if (IsInitProgram())
    {
      THEDATA(extramembot) = 0;
    }
    if (THEDATA(extramembot) > supmainmemory)
    {
      THEDATA(extramembot) = supmainmemory;
    }
#endif

#if defined(HAVE_EXTRA_MEM_TOP)
    if (IsInitProgram())
    {
      THEDATA(extramemtop) = 0;
    }
    if (THEDATA(extramemtop) > supmainmemory)
    {
      THEDATA(extramemtop) = supmainmemory;
    }
#endif

#if defined(MIKTEX_TEX_COMPILER)
    MIKTEX_ASSERT(THEDATA(membot) == 0);
    THEDATA(memtop) = THEDATA(membot) + THEDATA(mainmemory) - 1;
    THEDATA(memmin) = THEDATA(membot);
    THEDATA(memmax) = THEDATA(memtop);
#elif defined(MIKTEX_META_COMPILER)
    THEDATA(memtop) = 0/*memmin*/ + THEDATA(mainmemory) - 1;
    THEDATA(memmax) = THEDATA(memtop);
#endif

    Allocate("buffer", THEDATA(buffer), THEDATA(bufsize));
    Allocate("inputstack", THEDATA(inputstack), THEDATA(stacksize));
    Allocate("paramstack", THEDATA(paramstack), THEDATA(paramsize));
    Allocate("trickbuf", THEDATA(trickbuf), THEDATA(errorline));

    if (IsInitProgram() || AmI("mf"))
    {
      Allocate(THEDATA(strpool), THEDATA(poolsize));
    }

    if (IsInitProgram())
    {
#  if defined(MIKTEX_TEX_COMPILER)
      Allocate("mem", THEDATA(yzmem), THEDATA(memtop) - THEDATA(membot) + 2);
      MIKTEX_ASSERT(THEDATA(membot) == 0);
      THEDATA(zmem) = THEDATA(yzmem) - THEDATA(membot);
      THEDATA(mem) = THEDATA(zmem);
#  else
      Allocate("mem", THEDATA(mem), THEDATA(memtop) - 0/*memmin*/ + 2);
#  endif
    }

#if !defined(MIKTEX_OMEGA)
    Allocate("strstart", THEDATA(strstart), THEDATA(maxstrings));
#endif
  }
#endif

#if defined(THEDATA)
public:
  void FreeMemory()
  {
    Free(THEDATA(buffer));
#  if defined(MIKTEX_TEX_COMPILER)
    Free(THEDATA(yzmem));
#else
    Free(THEDATA(mem));
#endif
    Free(THEDATA(paramstack));
    Free(THEDATA(strpool));
    Free(THEDATA(trickbuf));
#if !defined(MIKTEX_OMEGA)
    Free(THEDATA(strstart));
#endif
  }
#endif

#if defined(poolsize) || defined(THEDATA)
protected:
  void CheckPoolPointer(int poolPtr, std::size_t len) const
  {
#if defined(poolsize)
    const std::size_t poolSize = poolsize;
#else
    const std::size_t poolSize = THEDATA(poolsize);
#endif
    if (poolPtr + len >= poolSize)
    {
      MIKTEX_FATAL_ERROR(MIKTEXTEXT("String pool overflow."));
    }
  }
#endif

public:
  bool CStyleErrorMessagesP() const
  {
    return showFileLineErrorMessages;
  }

public:
  MIKTEXMFTHISAPI(void) GetDefaultMemoryDumpFileName(char * lpszPath) const;

public:
  int GetInteraction() const
  {
    return interactionMode;
  }

public:

#if defined(THEDATA)
  int GetJobName()
  {
    if (jobName.empty())
    {
      char szName[MiKTeX::Core::BufferSizes::MaxPath];
      MiKTeX::Core::PathName::Split(lastInputFileName.Get(), nullptr, 0, szName, MiKTeX::Core::BufferSizes::MaxPath, nullptr, 0);
#if defined(MIKTEX_XETEX)
      jobName = szName;
#else
      jobName = MiKTeX::Core::Quoter<char>(szName).Get();
#endif
    }
    // FIXME: conserve strpool space
    return MakeTeXString(jobName.c_str());
  }
#endif

public:
  bool HaltOnErrorP() const
  {
    return haltOnError;
  }

public:
  MIKTEXMFTHISAPI(unsigned long) InitializeBuffer(unsigned char * pBuffer);

public:
  MIKTEXMFTHISAPI(unsigned long) InitializeBuffer(unsigned short * pBuffer);

public:
  MIKTEXMFTHISAPI(unsigned long) InitializeBuffer(C4P::C4P_signed32 * pBuffer);

public:
  MIKTEXMFTHISAPI(void) InvokeEditor(const MiKTeX::Core::PathName & editFileName, int editLineNumber, const MiKTeX::Core::PathName & transcriptFileName) const;

public:

#if defined(THEDATA)
  void InvokeEditor(int editFileName_, int editFileNameLength, int editLineNumber, int transcriptFileName_, int transcriptFileNameLength) const
  {
    TEXMFCHAR szEditFileName[Core::BufferSizes::MaxPath];
    GetTeXString(szEditFileName, Core::BufferSizes::MaxPath, editFileName_, editFileNameLength);
    TEXMFCHAR szTranscriptFileName[Core::BufferSizes::MaxPath];
    if (transcriptFileName_ != 0)
    {
      GetTeXString(szTranscriptFileName, Core::BufferSizes::MaxPath, transcriptFileName_, transcriptFileNameLength);
    }
    else
    {
      szTranscriptFileName[0] = 0;
    }
    InvokeEditor(szEditFileName, editLineNumber, szTranscriptFileName);
  }
#endif

public:
  bool IsInitProgram() const
  {
    return isInitProgram;
  }

protected:
  MIKTEXMFTHISAPI(bool) IsVirgin() const;

public:

#if defined(THEDATA)
  int MakeFullNameString()
  {
    return MakeTeXString(GetFoundFile());
  }
#endif

public:

#if defined(THEDATA)
  template<typename CharType> int MakeTeXString(const CharType * lpsz)
  {
    MIKTEX_ASSERT_STRING(lpsz);
#if defined(MIKTEX_TEXMF_UNICODE)
    MiKTeX::Util::CharBuffer<wchar_t, 200> buf(lpsz);
    std::size_t len = buf.GetLength();
    CheckPoolPointer(THEDATA(poolptr), len);
    for (size_t idx = 0; idx < len; ++idx)
    {
      THEDATA(strpool)[THEDATA(poolptr)++] = buf[idx];
    }
#else
    std::size_t len = MiKTeX::Util::StrLen(lpsz);
    CheckPoolPointer(THEDATA(poolptr), len);
    while (len-- > 0)
    {
      THEDATA(strpool)[THEDATA(poolptr)++] = *lpsz++;
    }
#endif
    return makestring();
  }
#endif

public:
  MIKTEXMFTHISAPI(bool) OpenMemoryDumpFile(const MiKTeX::Core::PathName & fileName, FILE ** ppFile, void * pBuf, std::size_t size, bool renew) const;

#if defined(THEDATA)  
public:
  template<class T> bool OpenMemoryDumpFile(T & f, bool renew = false) const
  {
    FILE * pfile;
    if (!OpenMemoryDumpFile(GetNameOfFile(), &pfile, nullptr, sizeof(*f), renew))
    {
      return false;
    }
    f.Attach(pfile, true);
    f.PascalFileIO(false);
    return true;
  }
#endif

public:
#if defined(THEDATA)  
  template<typename FILE_, typename ELETYPE_> void Dump(FILE_ & f, const ELETYPE_ & e, std::size_t n)
  {
    if (fwrite(&e, sizeof(e), n, static_cast<FILE*>(f)) != n)
    {
      MIKTEX_FATAL_CRT_ERROR("fwrite");
    }
  }
#endif

public:
#if defined(THEDATA)  
  template<typename FILE_, typename ELETYPE_> void Dump(FILE_ & f, const ELETYPE_ & e)
  {
    Dump(f, e, 1);
  }
#endif

#if defined(THEDATA)  
public:
  template<typename FILE_, typename ELETYPE_> void Undump(FILE_ & f, ELETYPE_ & e, std::size_t n)
  {
    f.PascalFileIO(false);
    if (fread(&e, sizeof(e), n, static_cast<FILE*>(f)) != n)
    {
      MIKTEX_FATAL_CRT_ERROR("fread");
    }
  }
#endif

#if defined(THEDATA)  
public:
  template<typename FILE_, typename ELETYPE_> void Undump(FILE_ & f, ELETYPE_ & e)
  {
    Undump(f, e, 1);
  }
#endif

#if defined(THEDATA)  
public:
  template<typename FILE_, typename ELETYPE_> void Undump(FILE_ & f, ELETYPE_ low, ELETYPE_ high, ELETYPE_ & e, std::size_t n)
  {
    Undump(f, e, n);
    for (std::size_t idx = 0; idx < n; ++idx)
    {
      if ((&e)[idx] < low || (&e)[idx] > high)
      {
        MIKTEX_FATAL_ERROR(MIKTEXTEXT("Bad format file."));
      }
    }
  }
#endif

#if defined(THEDATA)  
public:
  template<typename FILE_, typename ELETYPE_> void Undump(FILE_ & f, ELETYPE_ high, ELETYPE_ & e, std::size_t n)
  {
    Undump(f, e, n);
    for (std::size_t idx = 0; idx < n; ++idx)
    {
      if ((&e)[idx] > high)
      {
        MIKTEX_FATAL_ERROR(MIKTEXTEXT("Bad format file."));
      }
    }
  }
#endif

public:
  static MIKTEXMFCEEAPI(bool) ParseFirstLine(const MiKTeX::Core::PathName & path, MiKTeX::Core::Argv & argv);

private:
  MIKTEXMFTHISAPI(void) ParseFirstLine(const MiKTeX::Core::PathName & fileName);

#if defined(THEDATA)
public:
  static void MIKTEXCEECALL OnKeybordInterrupt(int)
  {
    signal(SIGINT, SIG_IGN);
    THEDATA(interrupt) = 1;
    signal(SIGINT, OnKeybordInterrupt);
  }
#endif

protected:
  void SetTeX()
  {
    isTeXProgram = true;
  }

protected:
  bool AmITeXCompiler() const
  {
    return isTeXProgram;
  }

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_time;

protected:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mem;

private:
  std::string memoryDumpFileName;

private:
  std::string jobName;

private:
  clock_t clockStart;

private:
  bool timeStatistics;

private:
  bool parseFirstLine;

private:
  bool showFileLineErrorMessages;

private:
  bool haltOnError;

private:
  bool isInitProgram;

private:
  bool setJobTime;

private:
  bool recordFileNames;

private:
  bool disableExtensions;

private:
  bool isTeXProgram;

private:
  int interactionMode;

private:
  int param_buf_size;

private:
  int param_error_line;

private:
  int param_extra_mem_bot;

private:
  int param_extra_mem_top;

private:
  int param_half_error_line;

private:
  int param_main_memory;

private:
  int param_max_print_line;

private:
  int param_max_strings;

private:
  int param_param_size;

private:
  int param_pool_free;

private:
  int param_pool_size;

private:
  int param_stack_size;

private:
  int param_strings_free;

private:
  int param_string_vacancies;

private:
  int optBase;
};

template<> inline std::string TeXMFApp::GetParameter(const char * lpszParameterName, const std::string & defaultValue) const
{
  std::string value = session->GetConfigValue(nullptr, lpszParameterName, "");
  if (value.empty())
  {
    value = session->GetConfigValue(GetProgramName(), lpszParameterName, defaultValue.c_str());
  }
  return value;
}

MIKTEXMF_END_NAMESPACE;

#endif
