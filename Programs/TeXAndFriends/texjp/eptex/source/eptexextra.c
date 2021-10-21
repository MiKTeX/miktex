/* eptexextra.c: Hand-coded routines for e-pTeX.

   This file is public domain.  */

#if defined(MIKTEX)
#include "miktex-eptex.h"
using namespace MiKTeX::TeXAndFriends;
#else
#define	EXTERN /* Instantiate data from eptexd.h here.  */
#define DLLPROC dlleptexmain

/* This file defines TeX and epTeX.  */
#include <eptexd.h>
#endif

/* Hand-coded routines for TeX or Metafont in C.  */
#include <lib/texmfmp.c>
