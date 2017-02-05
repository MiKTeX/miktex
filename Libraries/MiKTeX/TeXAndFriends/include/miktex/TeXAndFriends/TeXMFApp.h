/* miktex/TeXAndFriends/TeXMFApp.h:                     -*- C++ -*-

   Copyright (C) 1996-2017 Christian Schenk

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

class IStringHandler
{
public:
  virtual char* strpool() = 0;
public:
  virtual wchar_t* wstrpool() = 0;
public:
  virtual C4P::C4P_signed32& strptr() = 0;
public:
  virtual C4P::C4P_signed32* strstart() = 0;
public:
  virtual C4P::C4P_signed32& poolsize() = 0;
public:
  virtual C4P::C4P_signed32& poolptr() = 0;
public:
  virtual C4P::C4P_signed32 makestring() = 0;
};

class IErrorHandler
{
public:
  virtual C4P::C4P_integer& interrupt() = 0;
};

class ITeXMFMemoryHandler
{
public:
  virtual void Allocate(const std::unordered_map<std::string, int>& userParams) = 0;
public:
  virtual void Free() = 0;
public:
  virtual void Check() = 0;
public:
  virtual void* ReallocateArray(const std::string& arrayName, void* ptr, std::size_t elemSize, std::size_t numElem, const MiKTeX::Core::SourceLocation& sourceLocation) = 0;
};

#if defined(MIKTEX_TEXMF_UNICODE)
typedef wchar_t TEXMFCHAR;
typedef wint_t TEXMFCHARINT;
#else
typedef char TEXMFCHAR;
typedef int TEXMFCHARINT;
#endif

class MIKTEXMFTYPEAPI(TeXMFApp) :
  public WebAppInputLine
{
public:
  MIKTEXMFEXPORT MIKTEXTHISCALL TeXMFApp();

public:
  TeXMFApp(const TeXMFApp& other) = delete;

public:
  TeXMFApp& operator=(const TeXMFApp& other) = delete;

public:
  TeXMFApp(TeXMFApp&& other) = delete;

public:
  TeXMFApp& operator=(TeXMFApp&& other) = delete;

public:
  virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~TeXMFApp() noexcept;

public:
  MIKTEXMFTHISAPI(void) Init(const std::string& programInvocationName) override;

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

protected:
  MIKTEXMFTHISAPI(void) AddOptions() override;

public:
  MIKTEXMFTHISAPI(void) ProcessCommandLineOptions() override;

protected:
  MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string& optArg) override;

protected:
  std::string GetUsage() const override
  {
    return MIKTEXTEXT("[OPTION...] [COMMAND...]");
  }

public:
  MIKTEXMFTHISAPI(void) TouchJobOutputFile(FILE* file) const override;

public:
  virtual std::string GetMemoryDumpFileExtension() const
  {
    // must be implemented in sub-classes
    MIKTEX_UNEXPECTED();
  }

public:
  virtual MiKTeX::Core::FileType GetMemoryDumpFileType() const
  {
    // must be implemented in sub-classes
    MIKTEX_UNEXPECTED();
  }

protected:
  virtual MiKTeX::Core::PathName GetMemoryDumpFileName() const
  {
    // must be implemented in sub-classes
    MIKTEX_UNEXPECTED();
  }

protected:
  virtual std::string GetInitProgramName() const
  {
    // must be implemented in sub-classes
    MIKTEX_UNEXPECTED();
  }

protected:
  virtual std::string GetVirginProgramName() const
  {
    // must be implemented in sub-classes
    MIKTEX_UNEXPECTED();
  }

public:
  virtual void OnTeXMFInitialize() const
  {
    signal(SIGINT, OnKeybordInterrupt);
  }

public:
  virtual MIKTEXMFTHISAPI(void) OnTeXMFStartJob();

public:
  virtual MIKTEXMFTHISAPI(void) OnTeXMFFinishJob();

#if defined(MIKTEX_DEBUG)
public:
  virtual void CheckMemory()
  {
    GetTeXMFMemoryHandler()->Check();
  }
#endif

public:
  virtual void AllocateMemory()
  {
    std::unordered_map<std::string, int> params;
    GetTeXMFMemoryHandler()->Allocate(params);
  }

public:
  virtual void FreeMemory()
  {
    GetTeXMFMemoryHandler()->Free();
  }

protected:
  void CheckPoolPointer(int poolptr, std::size_t len) const
  {
    if (poolptr + len >= GetStringHandler()->poolsize())
    {
      MIKTEX_FATAL_ERROR(MIKTEXTEXT("String pool overflow."));
    }
  }

public:
  MIKTEXMFTHISAPI(bool) CStyleErrorMessagesP() const;

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetDefaultMemoryDumpFileName() const;

public:
  MIKTEXMFTHISAPI(int) GetInteraction() const;

public:
  int GetJobName()
  {
    if (jobName.empty())
    {
      MiKTeX::Core::PathName name = GetLastInputFileName().GetFileNameWithoutExtension();
#if defined(MIKTEX_XETEX)
      jobName = name.ToString();
#else
      jobName = MiKTeX::Core::Quoter<char>(name).GetData();
#endif
    }
    // FIXME: conserve strpool space
    return MakeTeXString(jobName.c_str());
  }

public:
  MIKTEXMFTHISAPI(bool) HaltOnErrorP() const;

public:
  MIKTEXMFTHISAPI(unsigned long) InitializeBuffer(char* buffer);

public:
  MIKTEXMFTHISAPI(unsigned long) InitializeBuffer(C4P::C4P_signed16* buffer);

public:
  MIKTEXMFTHISAPI(unsigned long) InitializeBuffer(C4P::C4P_signed32* buffer);

public:
  void InitializeBuffer()
  {
    IInputOutput* inout = GetInputOutput();
    inout->last() = InitializeBuffer(inout->buffer());
  }

public:
  TEXMFCHAR* GetTeXString(TEXMFCHAR* dest, std::size_t destSize, int stringStart, int stringLength) const
  {
    if (stringLength < 0 || stringLength >= destSize)
    {
      MIKTEX_FATAL_ERROR(MIKTEXTEXT("Bad string size."));
    }
    IStringHandler* stringHandler = GetStringHandler();
#if defined(MIKTEX_TEXMF_UNICODE)
    wchar_t* strpool = stringHandler->strpoolw();
#else
    char* strpool = stringHandler->strpool();
#endif
    for (int idx = 0; idx < stringLength; ++idx)
    {
      dest[idx] = strpool[stringStart + idx];
    }
    dest[stringLength] = 0;
    return dest;
  }

#if defined(MIKTEX_TEXMF_UNICODE)
public:
  char* GetTeXString(char* dest, std::size_t destSize, int stringStart, int stringLength) const
  {
    MiKTeX::Util::CharBuffer<wchar_t, 200> buf(stringLength + 1);
    GetTeXString(buf.GetData(), buf.GetCapacity(), stringStart, stringLength);
    MiKTeX::Util::StringUtil::CopyString(dest, destSize, buf.GetData());
    return dest;
  }
#endif

public:
  int GetTeXStringStart(int stringNumber) const
  {
#if defined(MIKTEX_OMEGA) || defined(MIKTEX_XETEX)
    MIKTEX_ASSERT(stringNumber >= 65536);
    stringNumber -= 65536;
#endif
    IStringHandler* stringHandler = GetStringHandler();
    MIKTEX_ASSERT(stringNumber >= 0 && stringNumber < stringHandler->strptr());
    return stringHandler->strstart()[stringNumber];
  }

public:
  int GetTeXStringLength(int stringNumber) const
  {
#if defined(MIKTEX_OMEGA) || defined(MIKTEX_XETEX)
    MIKTEX_ASSERT(stringNumber >= 65536);
    stringNumber -= 65536;
#endif
    IStringHandler* stringHandler = GetStringHandler();
    MIKTEX_ASSERT(stringNumber >= 0 && stringNumber < stringHandler->strptr());
    return stringHandler->strstart()[stringNumber + 1] - stringHandler->strstart()[stringNumber];
  }

public:
  template<typename CharType> CharType* GetTeXString(CharType* dest, int stringNumber, std::size_t destSize = 0xffff) const
  {
    int stringStart = GetTeXStringStart(stringNumber);
    int stringLength = GetTeXStringLength(stringNumber);
    return GetTeXString(dest, destSize, stringStart, stringLength);
  }

public:
  void InvokeEditor(const MiKTeX::Core::PathName& editFileName, int editLineNumber, const MiKTeX::Core::PathName& transcriptFileName) const
  {
    Application::InvokeEditor(editFileName, editLineNumber, GetInputFileType(), transcriptFileName);
  }

public:
  void InvokeEditor(int editFileName_, int editFileNameLength, int editLineNumber, int transcriptFileName_, int transcriptFileNameLength) const
  {
    TEXMFCHAR editFileName[Core::BufferSizes::MaxPath];
    GetTeXString(editFileName, Core::BufferSizes::MaxPath, editFileName_, editFileNameLength);
    TEXMFCHAR transcriptFileName[Core::BufferSizes::MaxPath];
    if (transcriptFileName_ != 0)
    {
      GetTeXString(transcriptFileName, Core::BufferSizes::MaxPath, transcriptFileName_, transcriptFileNameLength);
    }
    else
    {
      transcriptFileName[0] = 0;
    }
    InvokeEditor(editFileName, editLineNumber, transcriptFileName);
  }

public:
  MIKTEXMFTHISAPI(bool) IsInitProgram() const;

protected:
  MIKTEXMFTHISAPI(bool) IsVirgin() const;

public:
  int MakeFullNameString()
  {
    return MakeTeXString(GetFoundFile().GetData());
  }

public:
  template<typename CharType> int MakeTeXString(const CharType* lpsz)
  {
    MIKTEX_ASSERT_STRING(lpsz);
    IStringHandler* stringHandler = GetStringHandler();
#if defined(MIKTEX_TEXMF_UNICODE)
    MiKTeX::Util::CharBuffer<wchar_t, 200> buf(lpsz);
    std::size_t len = buf.GetLength();
    CheckPoolPointer(stringHandler->poolptr(), len);
    for (size_t idx = 0; idx < len; ++idx)
    {
      stringHandler->strpool()[stringHandler->poolptr()] = buf[idx];
      stringHandler->poolptr() += 1;
    }
#else
    std::size_t len = MiKTeX::Util::StrLen(lpsz);
    CheckPoolPointer(stringHandler->poolptr(), len);
    while (len-- > 0)
    {
      stringHandler->strpool()[stringHandler->poolptr()] = *lpsz++;
      stringHandler->poolptr() += 1;
    }
#endif
    return stringHandler->makestring();
  }

public:
  MIKTEXMFTHISAPI(bool) OpenMemoryDumpFile(const MiKTeX::Core::PathName& fileName, FILE** file, void* buf, std::size_t size, bool renew) const;

public:
  template<class T> bool OpenMemoryDumpFile(T& f, bool renew = false) const
  {
    FILE* file;
    if (!OpenMemoryDumpFile(GetNameOfFile(), &file, nullptr, sizeof(*f), renew))
    {
      return false;
    }
    f.Attach(file, true);
    f.PascalFileIO(false);
    return true;
  }

public:
  template<typename FILE_, typename ELETYPE_> void Dump(FILE_& f, const ELETYPE_& e, std::size_t n)
  {
    if (fwrite(&e, sizeof(e), n, static_cast<FILE*>(f)) != n)
    {
      MIKTEX_FATAL_CRT_ERROR("fwrite");
    }
  }

public:
  template<typename FILE_, typename ELETYPE_> void Dump(FILE_& f, const ELETYPE_& e)
  {
    Dump(f, e, 1);
  }

public:
  template<typename FILE_, typename ELETYPE_> void Undump(FILE_& f, ELETYPE_& e, std::size_t n)
  {
    f.PascalFileIO(false);
    if (fread(&e, sizeof(e), n, static_cast<FILE*>(f)) != n)
    {
      MIKTEX_FATAL_CRT_ERROR("fread");
    }
  }

public:
  template<typename FILE_, typename ELETYPE_> void Undump(FILE_& f, ELETYPE_& e)
  {
    Undump(f, e, 1);
  }

public:
  template<typename FILE_, typename ELETYPE_> void Undump(FILE_& f, ELETYPE_ low, ELETYPE_ high, ELETYPE_& e, std::size_t n)
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

public:
  template<typename FILE_, typename ELETYPE_> void Undump(FILE_ &f, ELETYPE_ high, ELETYPE_& e, std::size_t n)
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

public:
  static MIKTEXMFCEEAPI(MiKTeX::Core::Argv) ParseFirstLine(const MiKTeX::Core::PathName& path);

private:
  MIKTEXMFTHISAPI(void) CheckFirstLine(const MiKTeX::Core::PathName& fileName);

public:
  static void MIKTEXCEECALL OnKeybordInterrupt(int)
  {
    signal(SIGINT, SIG_IGN);
    ((TeXMFApp*)GetApplication())->GetErrorHandler()->interrupt() = 1;
    signal(SIGINT, OnKeybordInterrupt);
  }

protected:
  MIKTEXMFTHISAPI(void) SetTeX();

protected:
  MIKTEXMFTHISAPI(bool) AmITeXCompiler() const;

private:
  std::string jobName;

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

public:
  MIKTEXMFTHISAPI(void) SetStringHandler(IStringHandler* stringHandler);

public:
  MIKTEXMFTHISAPI(IStringHandler*) GetStringHandler() const;

public:
  MIKTEXMFTHISAPI(void) SetErrorHandler(IErrorHandler* errorHandler);

public:
  MIKTEXMFTHISAPI(IErrorHandler*) GetErrorHandler() const;

public:
  MIKTEXMFTHISAPI(void) SetTeXMFMemoryHandler(ITeXMFMemoryHandler* memoryHandler);

public:
  MIKTEXMFTHISAPI(ITeXMFMemoryHandler*) GetTeXMFMemoryHandler() const;

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEXMF_END_NAMESPACE;

#endif
