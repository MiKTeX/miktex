/* euptexextra.c: Hand-coded routines for e-upTeX.

   This file is public domain.  */

#if defined(MIKTEX)
#include "miktex-euptex.h"
using namespace MiKTeX::TeXAndFriends;
#else
#define	EXTERN /* Instantiate data from euptexd.h here.  */
#define DLLPROC dlleuptexmain
#endif

/* This file defines TeX and eupTeX.  */
#include <euptexd.h>

/* Hand-coded routines for TeX or Metafont in C.  */
#include <lib/texmfmp.c>
