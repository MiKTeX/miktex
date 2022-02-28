/**
 * @file miktex/TeXAndFriends/TeXMFMemoryHandlerImpl.h
 * @author Christian Schenk
 * @brief MiKTeX TeXMF memory handler implementation
 *
 * @copyright Copyright © 2017-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/Configuration/ConfigNames>

#include <miktex/TeXAndFriends/config.h>

#include <miktex/Trace/TraceStream>
#include <miktex/Trace/Trace>

#include "TeXMFApp.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

namespace texmfapp {
#include <miktex/texmfapp.defaults.h>
}

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

    void Allocate(const std::unordered_map<std::string, int>& userParams) override
    {
        program.bufsize = GetCheckedParameter("buf_size", program.infbufsize, program.supbufsize, userParams, texmfapp::texmfapp::buf_size());
        program.errorline = GetParameter("error_line", userParams, texmfapp::texmfapp::error_line());
#if defined(HAVE_EXTRA_MEM_BOT)
        program.extramembot = GetParameter("extra_mem_bot", userParams, texmfapp::texmfapp::extra_mem_bot());
#endif
#if defined(HAVE_EXTRA_MEM_TOP)
        program.extramemtop = GetParameter("extra_mem_top", userParams, texmfapp::texmfapp::extra_mem_top());
#endif
        program.halferrorline = GetParameter("half_error_line", userParams, texmfapp::texmfapp::half_error_line());
#if defined(HAVE_MAIN_MEMORY)
#  if defined(MIKTEX_METAFONT) || defined(MIKTEX_TEX) || defined(MIKTEX_PDFTEX) || defined(MIKTEX_PTEX_FAMILY) || defined(MIKTEX_XETEX)
        const int infmainmemory = 3000;
        const int supmainmemory = 256000000;
#  else
        const int infmainmemory = program.infmainmemory;
        const int supmainmemory = program.supmainmemory;
#  endif
        program.mainmemory = GetCheckedParameter("main_memory", infmainmemory, supmainmemory, userParams, texmfapp::texmfapp::main_memory());
#endif
        program.maxprintline = GetParameter("max_print_line", userParams, texmfapp::texmfapp::max_print_line());
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
            AllocateArray("strpool", program.strpool, program.poolsize);
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

        AllocateArray("strstart", program.strstart, program.maxstrings);
    }

    void Free() override
    {
        FreeArray("buffer", program.buffer);
        FreeArray("inputstack", program.inputstack);
        FreeArray("paramstack", program.paramstack);
        FreeArray("trickbuf", program.trickbuf);
        FreeArray("strpool", program.strpool);
#if defined(MIKTEX_TEX_COMPILER)
        FreeArray("yzmem", program.yzmem);
#else
        FreeArray("mem", program.mem);
#endif
        FreeArray("strstart", program.strstart);
    }

    void Check() override
    {
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.buffer);
#if defined(MIKTEX_TEX_COMPILER)
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.yzmem);
#else
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.mem);
#endif
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.inputstack);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.paramstack);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.strpool);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trickbuf);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.strstart);
    }

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
        trace_mem->WriteLine("libtexmf", "reallocate " + arrayName + ": ptr == " + std::string(ptr == nullptr ? "nullptr" : "...") + ", elementSize == " + std::to_string(elemSize) + ", nElements == " + std::to_string(numElem));
        ptr = MiKTeX::Debug::Realloc(ptr, amount, sourceLocation);
        return ptr;
    }

protected:

    int GetConfigValue(const std::string& valueName, int defaultValue) const
    {
        std::shared_ptr<MiKTeX::Core::Session> session = texmfapp.GetSession();
        int value = session->GetConfigValue(MIKTEX_CONFIG_SECTION_NONE, valueName, MiKTeX::Configuration::ConfigValue(-1)).GetInt();
        if (value < 0)
        {
            value = session->GetConfigValue(texmfapp.GetProgramName(), valueName, MiKTeX::Configuration::ConfigValue(defaultValue)).GetInt();
        }
        return value;
    }

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
        if (trace_mem->IsEnabled("libtexmf", MiKTeX::Trace::TraceLevel::Trace))
        {
            // TODO: trace_mem->WriteLine("libtexmf", fmt::format(MIKTEXTEXT("Parameter {0}: {1}"), parameterName, result));
        }
        return result;
    }

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
            MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("Bad parameter value."),
                "maxValue", std::to_string(maxValue),
                "minValue", std::to_string(minValue),
                "parameterName", parameterName,
                "value", std::to_string(result)
            );
        }
        if (trace_mem->IsEnabled("libtexmf", MiKTeX::Trace::TraceLevel::Trace))
        {
            // TODO: trace_mem->WriteLine("libtexmf", fmt::format(MIKTEXTEXT("Parameter {0}: {1}"), parameterName, result));
        }
        return result;
    }

    template<typename T> T* AllocateArray(const std::string& arrayName, T*& ptr, std::size_t n)
    {
        ptr = (T*)ReallocateArray(arrayName, nullptr, sizeof(*ptr), n, MIKTEX_SOURCE_LOCATION_DEBUG());
        return ptr;
    }

    template<typename T> void FreeArray(const std::string& arrayName, T*& ptr)
    {
        ReallocateArray(arrayName, ptr, sizeof(*ptr), 0, MIKTEX_SOURCE_LOCATION_DEBUG());
        ptr = nullptr;
    }

    PROGRAM_CLASS& program;
    TeXMFApp& texmfapp;
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mem;
};

MIKTEX_TEXMF_END_NAMESPACE;
