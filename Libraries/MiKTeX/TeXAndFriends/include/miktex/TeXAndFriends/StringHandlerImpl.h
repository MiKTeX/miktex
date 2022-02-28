/**
 * @file miktex/TeXAndFriends/StringHandlerImpl.h
 * @author Christian Schenk
 * @brief String handler implementation
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

#include "TeXMFApp.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class StringHandlerImpl :
    public IStringHandler
{

public:

    StringHandlerImpl(PROGRAM_CLASS& program) :
        program(program)
    {
    }

    char* strpool() override
    {
#if defined(MIKTEX_TEXMF_UNICODE)
        MIKTEX_UNEXPECTED();
#else
        MIKTEX_ASSERT(sizeof(program.strpool[0]) == sizeof(char));
        return reinterpret_cast<char*>(program.strpool);
#endif
    }

    char16_t* strpool16() override
    {
#if defined(MIKTEX_TEXMF_UNICODE)
        MIKTEX_ASSERT(sizeof(program.strpool[0]) == sizeof(char16_t));
        return (char16_t*)program.strpool;
#else
        MIKTEX_UNEXPECTED();
#endif
    }

    C4P::C4P_signed32& strptr() override
    {
        return program.strptr;
    }

    C4P::C4P_signed32* strstart() override
    {
        return program.strstart;
    }

    C4P::C4P_signed32& poolsize() override
    {
        return program.poolsize;
    }

    C4P::C4P_signed32& poolptr() override
    {
        return program.poolptr;
    }

    C4P::C4P_signed32 makestring() override
    {
        return program.makestring();
    }

private:

    PROGRAM_CLASS& program;
};

MIKTEX_TEXMF_END_NAMESPACE;
