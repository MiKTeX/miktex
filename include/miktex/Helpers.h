/**
 * @file miktex/Helpers.h
 * @author Christian Schenk
 * @brief MiKTeX helper macros
 *
 * @copyright Copyright © 2020-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives
 * unlimited permission to copy and/or distribute it, with or
 * without modifications, as long as this notice is preserved.
 */

#pragma once

#define MIKTEX_UNIQUE__(x, y) x##y
#define MIKTEX_UNIQUE_(x, y) MIKTEX_UNIQUE__(x, y)
#define MIKTEX_UNIQUE(x) MIKTEX_UNIQUE_(x, __COUNTER__)
