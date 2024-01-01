/**
 * @file mplib/miktex/mpost.h
 * @author Christian Schenk
 * @brief MiKTeX Metapost helpers
 *
 * @copyright Copyright © 2017-2024 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

int miktex_emulate__do_spawn(void* mpx, const char* fileName, char* const* argv);
void miktex_print_banner(FILE* file, const char* name, const char* version);
void miktex_show_library_versions();

#if defined(__cplusplus)
}
#endif
