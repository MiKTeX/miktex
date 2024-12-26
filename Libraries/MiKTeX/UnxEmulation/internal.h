/**
 * @file internal.h
 * @defgroup Unx emulation
 * @author Christian Schenk
 * @brief Internal definitions
 *
 * @copyright Copyright © 2007-2024 Christian Schenk
 *
 * This file is part of the MiKTeX UNXEMU Library.
 *
 * MiKTeX UNXEMU Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#define MIKTEXUNXEXPORT MIKTEXDLLEXPORT

#define D2A2BA842ACE40C6A8A17A9358F2147E
#include "miktex/unxemu.h"

#include <miktex/Core/Exceptions>
#include <miktex/Core/Utils>

#define T_(x) MIKTEXTEXT(x)

#define MIKTEXSTATICFUNC(type) static type
#define MIKTEXINTERNALFUNC(type) type
#define MIKTEXINTERNALVAR(type) type

#define C_FUNC_BEGIN()  \
    {                   \
        try             \
        {

#define C_FUNC_END()                                    \
        }                                               \
        catch (const MiKTeX::Core::MiKTeXException& e)  \
        {                                               \
            if (stderr != nullptr)                      \
            {                                           \
                Utils::PrintException(e);               \
            }                                           \
            exit(1);                                    \
        }                                               \
        catch (const std::exception& e)                 \
        {                                               \
            if (stderr != nullptr)                      \
            {                                           \
                Utils::PrintException(e);               \
            }                                           \
            exit(1);                                    \
        }                                               \
    }
