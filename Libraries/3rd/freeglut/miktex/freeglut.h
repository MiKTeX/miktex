/**
 * @file miktex/freeglut.h
 * @author Christian Schenk
 * @brief MiKTeX freeglut utilities
 *
 * @copyright Copyright © 2018-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#if defined(_MSC_VER)
#pragma once
#endif

#include <miktex/Definitions.h>

#include <wchar.h>

MIKTEX_BEGIN_EXTERN_C_BLOCK;

wchar_t* miktex_uw_strdup(const char* s);

MIKTEX_END_EXTERN_C_BLOCK;
