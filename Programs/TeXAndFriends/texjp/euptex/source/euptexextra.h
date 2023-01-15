/* euptexextra.h: banner etc. for e-upTeX.

   This is included by e-upTeX, from euptexextra.c
*/

#include <eptexdir/eptex_version.h> /* for EPTEX_VERSION */
#include <etexdir/etex_version.h> /* for ETEX_VERSION */
#include <ptexdir/ptex_version.h> /* for PTEX_VERSION */
#include <uptexdir/uptex_version.h> /* for UPTEX_VERSION */

#define BANNER "This is e-upTeX, Version 3.141592653-" PTEX_VERSION "-" UPTEX_VERSION "-" EPTEX_VERSION "-" ETEX_VERSION
#define COPYRIGHT_HOLDER "D.E. Knuth"
#define AUTHOR "Japanese TeX Development Community"
#define PROGRAM_HELP EUPTEXHELP
#define BUG_ADDRESS "issue@texjp.org"
#define DUMP_VAR TEXformatdefault
#define DUMP_LENGTH_VAR formatdefaultlength
#define DUMP_OPTION "fmt"
#define DUMP_EXT ".fmt"
#define INPUT_FORMAT kpse_tex_format
#define INI_PROGRAM "inieuptex"
#define VIR_PROGRAM "vireuptex"

#ifdef Xchr
#undef Xchr
#define Xchr(x) (x)
#endif /* Xchr */
