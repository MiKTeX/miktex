/**
 * @file init/miktex-inittangle.cpp
 * @author Christian Schenk
 * @brief MiKTeX InitTangle main
 *
 * @copyright Copyright © 1991-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#define C4PEXTERN extern

#include "inittangle.h"
#include "miktex-inittangle.h"

MIKTEX_DEFINE_WEBAPP(MiKTeX_INITTANGLE, INITTANGLE, g_InitTangleApp, InitTangleProgram, g_InitTangleProg)
