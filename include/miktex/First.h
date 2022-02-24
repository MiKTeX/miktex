/**
 * @file miktex/First.h
 * @author Christian Schenk
 * @brief MiKTeX header to be included first
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives
 * unlimited permission to copy and/or distribute it, with or
 * without modifications, as long as this notice is preserved.
 */

#pragma once

#if defined(_MSC_VER)
#   include <crtdbg.h>
#endif

#if defined(_MSC_VER) && defined(__cplusplus)

namespace MiKTeX
{
    namespace Debug {
        void
#if defined(EAD86981C92C904D808A5E6CEC64B90E)
            __declspec(dllexport)
#else
            __declspec(dllimport)
#endif
            __cdecl OnThrowStdException();
    }
}

#endif /* Microsoft C++ */
