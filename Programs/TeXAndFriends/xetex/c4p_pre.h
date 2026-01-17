/**
 * @file c4p_pre.h
 * @author Christian Schenk
 * @brief C4P first include
 *
 * @copyright Copyright © 2007-2026 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif

#include <cstdio>

/* _________________________________________________________________________

   texmfmp.h stuff
   _________________________________________________________________________ */

#define XETEX_UNICODE_FILE_DEFINED      1
typedef struct {
    FILE* f;
    long  savedChar;
    short skipNextLF;
    short encodingMode;
    void* conversionData;
} UFILE;
typedef UFILE* unicodefile;

typedef void* voidpointer;

#if _MSC_VER == 1800 || _MSC_VER == 1900
// workaround bug #2371 mathchoice in pdftex broken 
#  define C4P_NOOPT_mlisttohlist
#endif
