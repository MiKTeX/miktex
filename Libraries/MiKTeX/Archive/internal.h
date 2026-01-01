/**
 * @file internal.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief Internal definitions for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/Core/Text>

#define BEGIN_INTERNAL_NAMESPACE                        \
namespace MiKTeX {                                      \
    namespace Archive {                                 \
        namespace AF1A1A64A53D45708F96161A1541D424 {

#define END_INTERNAL_NAMESPACE                          \
        }                                               \
    }                                                   \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

#if !defined(UNUSED)
#   if !defined(NDEBUG)
#       define UNUSED(x)
#   else
#       define UNUSED(x) static_cast<void>(x)
#   endif
#endif

#if !defined(UNUSED_ALWAYS)
#   define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#define ARCHIVESTATICFUNC(type) static type
#define ARCHIVEINTERNALFUNC(type) type
#define ARCHIVEINTERNALVAR(type) type

#define ARRAY_SIZE(buf) (sizeof(buf)/sizeof(buf[0]))

#define CURRENT_DIRECTORY "."

BEGIN_INTERNAL_NAMESPACE;
END_INTERNAL_NAMESPACE;

using namespace MiKTeX::Archive::AF1A1A64A53D45708F96161A1541D424;
