/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Set to 1 if PostScript support should be disabled */
#cmakedefine DISABLE_GS 1

/* Define if WOFF support is disabled */
#cmakedefine DISABLE_WOFF 1

/* Define to 1 if `TIOCGWINSZ' requires <sys/ioctl.h>. */
#cmakedefine GWINSZ_IN_SYS_IOCTL 1

/* define if the compiler supports basic C++11 syntax */
#cmakedefine HAVE_CXX11 1

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you have the `ftime' function. */
#cmakedefine HAVE_FTIME 1

/* Define to 1 if you have the `gettimeofday' function. */
#cmakedefine HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the `dl' library (-ldl). */
#cmakedefine HAVE_LIBDL 1

/* Define to 1 if you have the `gs' library (-lgs). */
#cmakedefine HAVE_LIBGS 1

/* Define to 1 if you have the <libintl.h> header file. */
#cmakedefine HAVE_LIBINTL_H 1

/* Define to 1 if you have the `kpathsea' library (-lkpathsea). */
#cmakedefine HAVE_LIBKPATHSEA 1

/* Define to 1 if you have the `m' library (-lm). */
#cmakedefine HAVE_LIBM 1

/* Define to 1 if you have the `potrace' library (-lpotrace). */
#cmakedefine HAVE_LIBPOTRACE 1

/* Define to 1 if you have the `ttfautohint' library (-lttfautohint). */
#cmakedefine HAVE_LIBTTFAUTOHINT 1

/* Define to 1 if you have the `xxhash' library (-lxxhash). */
#cmakedefine HAVE_LIBXXHASH 1

/* Define to 1 if you have the `z' library (-lz). */
#cmakedefine HAVE_LIBZ 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H 1

/* Define to 1 if you have the <openssl/md5.h> header file. */
#cmakedefine HAVE_OPENSSL_MD5_H 1

/* Define to 1 if you have the `sigaction' function. */
#cmakedefine HAVE_SIGACTION 1

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
#cmakedefine HAVE_STAT_EMPTY_STRING_BUG 1

/* Define to 1 if stdbool.h conforms to C99. */
#cmakedefine HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/timeb.h> header file. */
#cmakedefine HAVE_SYS_TIMEB_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <ttfautohint.h> header file. */
#cmakedefine HAVE_TTFAUTOHINT_H 1

/* Define to 1 if you have the `umask' function. */
#cmakedefine HAVE_UMASK 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the `uselocale' function. */
#cmakedefine HAVE_USELOCALE 1

/* Define to 1 if you have the <xlocale.h> header file. */
#cmakedefine HAVE_XLOCALE_H 1

/* Define to 1 if the system has the type `_Bool'. */
#cmakedefine HAVE__BOOL 1

/* Define to 1 if the kpathsea headers are not C++ safe. */
#cmakedefine KPSE_CXX_UNSAFE 1

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#cmakedefine LSTAT_FOLLOWS_SLASHED_SYMLINK 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#cmakedefine LT_OBJDIR 1

/* Name of package */
#cmakedefine PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "dvisvgm 2.6"

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME

/* Define to the home page for this package. */
#cmakedefine PACKAGE_URL

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* The machine triplet of the target system */
#define TARGET_SYSTEM ""

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#cmakedefine TM_IN_SYS_TIME 1

/* Version number of package */
#define VERSION "2.6"

/* Define to `unsigned int' if <sys/types.h> does not define. */
#cmakedefine size_t

#if defined(MIKTEX)
#define NOMINMAX
#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#endif
