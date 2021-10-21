/* ptexextra.c: Hand-coded routines for pTeX.

   This file is public domain.  */

#if defined(MIKTEX)
#include "miktex-ptex.h"
using namespace MiKTeX::TeXAndFriends;
#else
#define	EXTERN /* Instantiate data from ptexd.h here.  */
#define DLLPROC dllptexmain

/* This file defines TeX and pTeX.  */
#include <ptexd.h>
#endif

/* Hand-coded routines for TeX or Metafont in C.  */
#include <lib/texmfmp.c>
