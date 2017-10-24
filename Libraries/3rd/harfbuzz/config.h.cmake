/* config.h.in.  Generated from configure.ac by autoheader.  */

/* The normal alignment of `struct{char;}', in bytes. */
#cmakedefine ALIGNOF_STRUCT_CHAR__

/* Define to 1 if you have the `atexit' function. */
#cmakedefine HAVE_ATEXIT 1

/* Have cairo graphics library */
#cmakedefine HAVE_CAIRO 1

/* Have cairo-ft support in cairo graphics library */
#cmakedefine HAVE_CAIRO_FT 1

/* Have Core Text backend */
#cmakedefine HAVE_CORETEXT 1

/* Have DirectWrite library */
#cmakedefine HAVE_DIRECTWRITE 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you have the <dwrite.h> header file. */
#cmakedefine HAVE_DWRITE_H 1

/* Have simple TrueType Layout backend */
#cmakedefine HAVE_FALLBACK

/* Have fontconfig library */
#cmakedefine HAVE_FONTCONFIG

/* Have FreeType 2 library */
#cmakedefine HAVE_FREETYPE 1

/* Define to 1 if you have the `FT_Get_Var_Blend_Coordinates' function. */
#cmakedefine HAVE_FT_GET_VAR_BLEND_COORDINATES 1

/* Define to 1 if you have the `getpagesize' function. */
#cmakedefine HAVE_GETPAGESIZE 1

/* Have glib2 library */
#cmakedefine HAVE_GLIB 1

/* Have gobject2 library */
#cmakedefine HAVE_GOBJECT 1

/* Have Graphite2 library */
#cmakedefine HAVE_GRAPHITE2 1

/* Have ICU library */
#cmakedefine HAVE_ICU 1

/* Use hb-icu Unicode callbacks */
#cmakedefine HAVE_ICU_BUILTIN

/* Have Intel __sync_* atomic primitives */
#cmakedefine HAVE_INTEL_ATOMIC_PRIMITIVES 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the `isatty' function. */
#cmakedefine HAVE_ISATTY 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the `mmap' function. */
#cmakedefine HAVE_MMAP 1

/* Define to 1 if you have the `mprotect' function. */
#cmakedefine HAVE_MPROTECT 1

/* Define to 1 if you have the `newlocale' function. */
#cmakedefine HAVE_NEWLOCALE 1

/* Have native OpenType Layout backend */
#cmakedefine HAVE_OT 1

/* Have POSIX threads */
#cmakedefine HAVE_PTHREAD 1

/* Have PTHREAD_PRIO_INHERIT. */
#cmakedefine HAVE_PTHREAD_PRIO_INHERIT 1

/* Define to 1 if you have the <sched.h> header file. */
#cmakedefine HAVE_SCHED_H 1

/* Have sched_yield */
#cmakedefine HAVE_SCHED_YIELD 1

/* Have Solaris __machine_*_barrier and atomic_* operations */
#cmakedefine HAVE_SOLARIS_ATOMIC_OPS 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the `strtod_l' function. */
#cmakedefine HAVE_STRTOD_L 1

/* Define to 1 if you have the `sysconf' function. */
#cmakedefine HAVE_SYSCONF 1

/* Define to 1 if you have the <sys/mman.h> header file. */
#cmakedefine HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Have UCDN Unicode functions */
#cmakedefine HAVE_UCDN 1

/* Have Uniscribe library */
#cmakedefine HAVE_UNISCRIBE 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the <usp10.h> header file. */
#cmakedefine HAVE_USP10_H 1

/* Define to 1 if you have the <windows.h> header file. */
#cmakedefine HAVE_WINDOWS_H 1

/* Define to 1 if you have the <xlocale.h> header file. */
#cmakedefine HAVE_XLOCALE_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#cmakedefine LT_OBJDIR 1

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME

/* Define to the home page for this package. */
#cmakedefine PACKAGE_URL

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
#cmakedefine PTHREAD_CREATE_JOINABLE 1

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# undef _ALL_SOURCE
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# undef _POSIX_PTHREAD_SEMANTICS
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# undef _TANDEM_SOURCE
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# undef __EXTENSIONS__
#endif


/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to 1 if on MINIX. */
#cmakedefine _MINIX 1

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#cmakedefine _POSIX_1_SOURCE 2

/* Define to 1 if you need to in order for `stat' and other things to work. */
#cmakedefine _POSIX_SOURCE 1
