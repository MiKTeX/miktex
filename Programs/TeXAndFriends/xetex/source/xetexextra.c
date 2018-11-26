/* xetexextra.c: Hand-coded routines for XeTeX.

   This file is public domain.  */

#if defined(MIKTEX)
#include "xetex-miktex.h"
using namespace MiKTeX::TeXAndFriends;
#else
#define	EXTERN /* Instantiate data from xetexd.h here.  */

/* This file defines TeX and XeTeX.  */
#include <xetexd.h>
#endif

/* Hand-coded routines for TeX or Metafont in C.  */
#include <lib/texmfmp.c>
