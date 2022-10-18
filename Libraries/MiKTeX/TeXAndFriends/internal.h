/**
 * @file internal.h
 * @author Christian Schenk
 * @brief Internal definitions
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#define FIRST_OPTION_VAL 256

#define STATICFUNC(type) type

#define MIKTEX_API_BEGIN(funcname) {
#define MIKTEX_API_END(funcname) }

#define T_(x) this->Translate(x)

#define MIKTEX_UNIMPLEMENTED(x) MIKTEX_UNEXPECTED(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#define BEGIN_INTERNAL_NAMESPACE                        \
namespace MiKTeX {                                      \
    namespace TeXAndFriends {                           \
        namespace CDA7FC807F4A47E294AF2721867074A4 {

#define END_INTERNAL_NAMESPACE                  \
        }                                       \
    }                                           \
}

#include "TeXMFResources.h"

BEGIN_INTERNAL_NAMESPACE;

inline int GetC(FILE* file)
{
    MIKTEX_ASSERT(file != nullptr);
    int ch = getc(file);
    if (ch == EOF && ferror(file) != 0)
    {
        MIKTEX_FATAL_CRT_ERROR("getc");
    }
    return ch;
}

END_INTERNAL_NAMESPACE;


using namespace MiKTeX::TeXAndFriends::CDA7FC807F4A47E294AF2721867074A4;
