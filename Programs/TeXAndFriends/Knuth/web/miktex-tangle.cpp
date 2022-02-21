/**
 * @file miktex-tangle.cpp
 * @author Christian Schenk
 * @brief MiKTeX Tangle main
 *
 * @copyright Copyright © 1991-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#define C4PEXTERN extern

#include "tangle.h"
#include "miktex-tangle.h"

MIKTEX_DEFINE_WEBAPP(MiKTeX_TANGLE, TANGLE, g_TangleApp, TangleProgram, g_TangleProg)
