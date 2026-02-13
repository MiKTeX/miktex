/**
 * @file c-auto.h
 * @author Christian Schenk
 * @brief Web2C emulation
 *
 * @copyright Copyright © 2010-2026 Christian Schenk
 *
 * This file is part of the MiKTeX W2CEMU Library.
 *
 * MiKTeX W2CEMU Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#define RETSIGTYPE void
#include <w2c/c-auto.h>

#if defined(MIKTEX_WINDOWS)
#define EDITOR "notepad.exe"
#else
#define EDITOR ""
#endif
