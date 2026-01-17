/**
 * @file c4p_post.h
 * @author Christian Schenk
 * @brief C4P last include
 *
 * @copyright Copyright © 2020-2026 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "c4p_pre.h"
#include <miktex/C4P/C4P>

class XeTeXProgramBase :
    public C4P::ProgramBase
{
protected:
    void flushunicodefile(unicodefile f)
    {
        if (fflush(f->f) == EOF)
        {
            MIKTEX_FATAL_CRT_ERROR("fflush");
        }
    }
};
