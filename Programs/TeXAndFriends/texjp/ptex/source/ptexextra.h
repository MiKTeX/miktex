/* ptexextra.h: banner etc. for pTeX.

   This is included by pTeX, from ptexextra.c
*/

#include <ptexdir/ptex_version.h> /* for PTEX_VERSION */

#define BANNER "This is pTeX, Version 3.141592653-" PTEX_VERSION
#define COPYRIGHT_HOLDER "D.E. Knuth"
#define AUTHOR "Japanese TeX Development Community"
#define PROGRAM_HELP PTEXHELP
#define BUG_ADDRESS "issue@texjp.org"
#define DUMP_VAR TEXformatdefault
#define DUMP_LENGTH_VAR formatdefaultlength
#define DUMP_OPTION "fmt"
#define DUMP_EXT ".fmt"
#define INPUT_FORMAT kpse_tex_format
#define INI_PROGRAM "iniptex"
#define VIR_PROGRAM "virptex"

#ifdef Xchr
#undef Xchr
#define Xchr(x) (x)
#endif /* Xchr */
