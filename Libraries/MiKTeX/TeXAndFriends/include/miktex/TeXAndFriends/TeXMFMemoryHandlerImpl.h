/* miktex/TeXAndFriends/TeXMFMemoryHandlerImpl.h:       -*- C++ -*-

   Copyright (C) 2017 Christian Schenk

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

#if !defined(FEFFF218B53147ED8CDE64F68A13D234)
#define FEFFF218B53147ED8CDE64F68A13D234

#include <miktex/TeXAndFriends/config.h>
#include <miktex/Trace/TraceStream>
#include <miktex/Trace/Trace>

#include "TeXMFApp.h"

MIKTEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class TeXMFMemoryHandlerImpl :
  public ITeXMFMemoryHandler
{
public:
  TeXMFMemoryHandlerImpl(PROGRAM_CLASS& program, TeXMFApp& texmfapp) :
    program(program),
    texmfapp(texmfapp),
    trace_mem(MiKTeX::Trace::TraceStream::Open(MIKTEX_TRACE_MEM))
  {
  }

protected:
  PROGRAM_CLASS& program;

protected:
  TeXMFApp& texmfapp;

protected:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mem;

protected:
  int GetConfigValue(const std::string& valueName, int defaultValue) const
  {
    std::shared_ptr<MiKTeX::Core::Session> session = texmfapp.GetSession();
    int value = session->GetConfigValue("", valueName, -1).GetInt();
    if (value < 0)
    {
      value = session->GetConfigValue(texmfapp.GetProgramName(), valueName, defaultValue).GetInt();
    }
    return value;
  }

protected:
  int GetParameter(const std::string& parameterName, const std::unordered_map<std::string, int>& userParams, int defaultValue)
  {
    int result;
    auto it = userParams.find(parameterName);
    if (it == userParams.end())
    {
      result = GetConfigValue(parameterName, defaultValue);
    }
    else
    {
      result = it->second;
    }
    if (trace_mem->IsEnabled())
    {
      trace_mem->WriteFormattedLine("libtexmf", MIKTEXTEXT("Parameter %s: %d"), parameterName.c_str(), result);
    }
    return result;
  }

protected:
  int GetCheckedParameter(const std::string& parameterName, int minValue, int maxValue, const std::unordered_map<std::string, int>& userParams, int defaultValue)
  {
    int result;
    auto it = userParams.find(parameterName);
    if (it == userParams.end())
    {
      result = GetConfigValue(parameterName, defaultValue);
    }
    else
    {
      result = it->second;
    }
    if (result < minValue || result > maxValue)
    {
      MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("Bad parameter value."), "parameterName", parameterName);
    }
    if (trace_mem->IsEnabled())
    {
      trace_mem->WriteFormattedLine("libtexmf", MIKTEXTEXT("Parameter %s: %d"), parameterName.c_str(), result);
    }
  }

protected:
  template<typename T> T* AllocateArray(const std::string& arrayName, T*& ptr, std::size_t n)
  {
    ptr = (T*)ReallocateArray(arrayName, nullptr, sizeof(*ptr), n, MIKTEX_SOURCE_LOCATION());
    return ptr;
  }

protected:
  template<typename T> void FreeArray(const std::string& arrayName, T*& ptr)
  {
    ReallocateArray(arrayName, ptr, sizeof(*ptr), 0, MIKTEX_SOURCE_LOCATION());
    ptr = nullptr;
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    program.bufsize = GetCheckedParameter("buf_size", program.infbufsize, program.supbufsize, userParams, texmfapp::texmfapp::buf_size());
    program.errorline = GetCheckedParameter("error_line", program.inferrorline, program.superrorline, userParams, texmfapp::texmfapp::error_line());
#if defined(HAVE_EXTRA_MEM_BOT)
    program.extramembot = GetParameter("extra_mem_bot", userParams, texmfapp::texmfapp::extra_mem_bot());
#endif
#if defined(HAVE_EXTRA_MEM_TOP)
    program.extramemtop = GetParameter("extra_mem_top", userParams, texmfapp::texmfapp::extra_mem_top());
#endif
    program.halferrorline = GetCheckedParameter("half_error_line", program.infhalferrorline, program.suphalferrorline, userParams, texmfapp::texmfapp::half_error_line());
#if defined(HAVE_MAIN_MEMORY)
#  if defined(MIKTEX_METAFONT) || defined(MIKTEX_TEX) || defined(MIKTEX_PDFTEX)
    const int infmainmemory = 3000;
    const int supmainmemory = 256000000;
#  else
    const int infmainmemory = program.infmainmemory;
    const int supmainmemory = program.supmainmemory;
#  endif
    program.mainmemory = GetCheckedParameter("main_memory", infmainmemory, supmainmemory, userParams, texmfapp::texmfapp::main_memory());
#endif
    program.maxprintline = GetCheckedParameter("max_print_line", program.infmaxprintline, program.supmaxprintline, userParams, texmfapp::texmfapp::max_print_line());
    program.maxstrings = GetCheckedParameter("max_strings", program.infmaxstrings, program.supmaxstrings, userParams, texmfapp::texmfapp::max_strings());
#if defined(MIKTEX_METAFONT)
    const int infparamsize = 60;
    const int supparamsize = 600000;
#else
    const int infparamsize = program.infparamsize;
    const int supparamsize = program.supparamsize;
#endif
    program.paramsize = GetCheckedParameter("param_size", infparamsize, supparamsize, userParams, texmfapp::texmfapp::param_size());
#if defined(HAVE_POOL_FREE)
    program.poolfree = GetCheckedParameter("pool_free", program.infpoolfree, program.suppoolfree, userParams, texmfapp::texmfapp::pool_free());
#endif
    program.poolsize = GetCheckedParameter("pool_size", program.infpoolsize, program.suppoolsize, userParams, texmfapp::texmfapp::pool_size());
    program.stacksize = GetCheckedParameter("stack_size", program.infstacksize, program.supstacksize, userParams, texmfapp::texmfapp::stack_size());
#if defined(HAVE_STRINGS_FREE)
    program.stringsfree = GetCheckedParameter("strings_free", program.infstringsfree, program.supstringsfree, userParams, texmfapp::texmfapp::strings_free());
#endif
    program.stringvacancies = GetCheckedParameter("string_vacancies", program.infstringvacancies, program.supstringvacancies, userParams, texmfapp::texmfapp::string_vacancies());

    program.maxstrings += 0x100;

#if defined(HAVE_EXTRA_MEM_BOT)
    if (texmfapp.IsInitProgram())
    {
      program.extramembot = 0;
    }
    if (program.extramembot > supmainmemory)
    {
      program.extramembot = supmainmemory;
    }
#endif

#if defined(HAVE_EXTRA_MEM_TOP)
    if (texmfapp.IsInitProgram())
    {
      program.extramemtop = 0;
    }
    if (program.extramemtop > supmainmemory)
    {
      program.extramemtop = supmainmemory;
    }
#endif

#if defined(MIKTEX_TEX_COMPILER)
    MIKTEX_ASSERT(program.membot == 0);
    program.memtop = program.membot + program.mainmemory - 1;
    program.memmin = program.membot;
    program.memmax = program.memtop;
#elif defined(MIKTEX_META_COMPILER)
    program.memtop = 0/*memmin*/ + program.mainmemory - 1;
    program.memmax = program.memtop;
#endif

    AllocateArray("buffer", program.buffer, program.bufsize);
    AllocateArray("inputstack", program.inputstack, program.stacksize);
    AllocateArray("paramstack", program.paramstack, program.paramsize);
    AllocateArray("trickbuf", program.trickbuf, program.errorline);

    if (texmfapp.IsInitProgram() || texmfapp.AmI("mf"))
    {
      AllocateArray("", program.strpool, program.poolsize);
    }

    if (texmfapp.IsInitProgram())
    {
#if defined(MIKTEX_TEX_COMPILER)
      AllocateArray("mem", program.yzmem, program.memtop - program.membot + 2);
      MIKTEX_ASSERT(program.membot == 0);
      program.zmem = program.yzmem - program.membot;
      program.mem = program.zmem;
#else
      AllocateArray("mem", program.mem, program.memtop - 0/*memmin*/ + 2);
#endif
    }

#if !defined(MIKTEX_OMEGA)
    AllocateArray("strstart", program.strstart, program.maxstrings);
#endif
  }

public:
  void Free() override
  {
    FreeArray("", program.buffer);
    FreeArray("", program.inputstack);
    FreeArray("", program.paramstack);
    FreeArray("", program.trickbuf);
    FreeArray("", program.strpool);
#if defined(MIKTEX_TEX_COMPILER)
    FreeArray("", program.yzmem);
#else
    FreeArray("", program.mem);
#endif
#if !defined(MIKTEX_OMEGA)
    FreeArray("", program.strstart);
#endif
  }

public:
  void Check() override
  {
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.buffer);
#if defined(MIKTEX_TEX_COMPILER)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.yzmem);
#else
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.mem);
#endif
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.paramstack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.strpool);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trickbuf);
#if !defined(MIKTEX_OMEGA)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.strstart);
#endif
  }

public:
  void* ReallocateArray(const std::string& arrayName, void* ptr, std::size_t elemSize, std::size_t numElem, const MiKTeX::Core::SourceLocation& sourceLocation) override
  {
    std::size_t amount;
    if (numElem == 0)
    {
      amount = 0;
    }
    else
    {
      // one extra element because Pascal arrays are 1-based
      amount = (numElem + 1) * elemSize;
    }
    if (trace_mem->IsEnabled())
    {
      trace_mem->WriteFormattedLine("libtexmf", MIKTEXTEXT("Reallocate %s: p == %p, elementSize == %u, nElements == %u, bytes == %u"), arrayName.empty() ? "array" : arrayName.c_str(), ptr, (unsigned)elemSize, (unsigned)numElem, (unsigned)amount);
    }
    ptr = MiKTeX::Debug::Realloc(ptr, amount, sourceLocation);
    if (trace_mem->IsEnabled())
    {
      trace_mem->WriteFormattedLine("libtexmf", MIKTEXTEXT("Reallocate: return %p"), ptr);
    }
    return ptr;
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
