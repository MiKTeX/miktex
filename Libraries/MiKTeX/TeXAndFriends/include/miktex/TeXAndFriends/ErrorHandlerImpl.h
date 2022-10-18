/**
 * @file miktex/TeXAndFriends/ErrorHandlerImpl.h
 * @author Christian Schenk
 * @brief Error handler implementation
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

template<class PROGRAM_CLASS> class ErrorHandlerImpl :
    public IErrorHandler
{

public:

    ErrorHandlerImpl(PROGRAM_CLASS& program) :
        program(program)
    {
    }

    C4P::C4P_integer& interrupt() override
    {
        return program.interrupt;
    }

private:

    PROGRAM_CLASS& program;
};

MIKTEX_TEXMF_END_NAMESPACE;
