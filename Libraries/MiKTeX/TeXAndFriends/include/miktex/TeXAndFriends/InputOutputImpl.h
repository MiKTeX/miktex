/**
 * @file miktex/TeXAndFriends/InputOutputImpl.h
 * @author Christian Schenk
 * @brief MiKTeX TeXMF input/output handler implementation
 *
 * @copyright Copyright © 2017-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/TeXAndFriends/config.h>

#include "WebAppInputLine.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class InputOutputImpl :
    public IInputOutput
{

public:

    InputOutputImpl(PROGRAM_CLASS& program) :
        program(program)
    {
    }

    C4P::C4P_signed32& loc() override
    {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
        return program.curinput.locfield;
#else
        MIKTEX_UNEXPECTED();
#endif
    }

    C4P::C4P_signed32& limit() override
    {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
        return program.curinput.limitfield;
#else
        MIKTEX_UNEXPECTED();
#endif
    }

    C4P::C4P_signed32 first() override
    {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
        return program.first;
#else
        return 0;
#endif
    }

    C4P::C4P_signed32& last() override
    {
        return program.last;
    }

    C4P::C4P_signed32 bufsize() override
    {
        return program.bufsize;
    }

    char*& nameoffile() override
    {
        MIKTEX_ASSERT(sizeof(program.nameoffile[0]) == sizeof(char));
        return reinterpret_cast<char*&>(program.nameoffile);
    }

    C4P::C4P_signed32& namelength() override
    {
        return program.namelength;
    }

    char* buffer() override
    {
#if defined(MIKTEX_TEXMF_UNICODE)
        MIKTEX_UNEXPECTED();
#else
        MIKTEX_ASSERT(sizeof(program.buffer[0]) == sizeof(char));
        return reinterpret_cast<char*>(program.buffer);
#endif
    }

    char* buffer2() override
    {
        return nullptr;
    }

    char32_t* buffer32() override
    {
#if defined(MIKTEX_XETEX)
        MIKTEX_ASSERT(sizeof(program.buffer[0]) == sizeof(char32_t));
        return reinterpret_cast<char32_t*>(program.buffer);
#else
        MIKTEX_UNEXPECTED();
#endif
    }

    C4P::C4P_signed32& maxbufstack() override
    {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
        return program.maxbufstack;
#else
        MIKTEX_UNEXPECTED();
#endif
    }

    void overflow(C4P::C4P_signed32 s, C4P::C4P_integer n) override
    {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
        program.overflow(s, n);
#else
        MIKTEX_UNEXPECTED();
#endif
    }

protected:

    PROGRAM_CLASS& program;
};

MIKTEX_TEXMF_END_NAMESPACE;
