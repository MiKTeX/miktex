/**
 * @file miktex/TeXAndFriends/FormatHandlerImpl.h
 * @author Christian Schenk
 * @brief MiKTeX TeX format handler implementation
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

#include "TeXApp.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class FormatHandlerImpl :
    public IFormatHandler
{

public:

    FormatHandlerImpl(PROGRAM_CLASS& program) :
        program(program)
    {
    }

    C4P::C4P_signed32& formatident() override
    {
        return program.formatident;
    }

private:

    PROGRAM_CLASS& program;
};

MIKTEX_TEXMF_END_NAMESPACE;
