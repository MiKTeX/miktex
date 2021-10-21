/* uptexextra.c: Hand-coded routines for upTeX.

   This file is public domain.  */

#if defined(MIKTEX)
#include "miktex-uptex.h"
using namespace MiKTeX::TeXAndFriends;
#else
#define	EXTERN /* Instantiate data from uptexd.h here.  */
#define DLLPROC dlluptexmain
#endif

/* This file defines TeX and upTeX.  */
#include <uptexd.h>

/* Hand-coded routines for TeX or Metafont in C.  */
#include <lib/texmfmp.c>
