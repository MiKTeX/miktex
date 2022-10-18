/**
 * @file miktex/asy-first.h
 * @author Christian Schenk
 * @brief MiKTeX Asy things first
 *
 * @copyright Copyright © 2018-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include <miktex/ExitThrows>

#if !defined(MIKTEX_BEGIN_NS)
#define MIKTEX_BEGIN_NS                         \
namespace MiKTeX {                              \
    namespace Aymptote {
#define MIKTEX_END_NS                           \
    }                                           \
}
#endif
