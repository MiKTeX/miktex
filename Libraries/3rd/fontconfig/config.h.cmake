/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
#cmakedefine AC_APPLE_UNIVERSAL_BUILD

/* The normal alignment of `double', in bytes. */
#define ALIGNOF_DOUBLE 8

/* The normal alignment of `void *', in bytes. */
#define ALIGNOF_VOID_P 8

/* Use libxml2 instead of Expat */
#cmakedefine ENABLE_LIBXML2

/* Additional font directories */
#cmakedefine FC_ADD_FONTS

/* Architecture prefix to use for cache file names */
#cmakedefine FC_ARCHITECTURE

/* System font directory */
#cmakedefine FC_DEFAULT_FONTS

/* The type of len parameter of the gperf hash/lookup function */
#define FC_GPERF_SIZE_T unsigned int

/* Define to nothing if C supports flexible array members, and to 1 if it does
   not. That way, with a declaration like `struct s { int n; double
   d[FLEXIBLE_ARRAY_MEMBER]; };', the struct hack can be used with pre-C99
   compilers. When computing the size of such an object, don't use 'sizeof
   (struct s)' as it overestimates the size. Use 'offsetof (struct s, d)'
   instead. Don't use 'offsetof (struct s, d[0])', as this doesn't work with
   MSVC and with C++ compilers. */
#define FLEXIBLE_ARRAY_MEMBER

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#cmakedefine HAVE_DOPRNT 1

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H 1

/* Define to 1 if you have the `fstatfs' function. */
#cmakedefine HAVE_FSTATFS 1

/* Define to 1 if you have the `fstatvfs' function. */
#cmakedefine HAVE_FSTATVFS 1

/* FT_Bitmap_Size structure includes y_ppem field */
#define HAVE_FT_BITMAP_SIZE_Y_PPEM 1

/* Define to 1 if you have the `FT_Get_BDF_Property' function. */
#define HAVE_FT_GET_BDF_PROPERTY 1

/* Define to 1 if you have the `FT_Get_Next_Char' function. */
#define HAVE_FT_GET_NEXT_CHAR 1

/* Define to 1 if you have the `FT_Get_PS_Font_Info' function. */
#define HAVE_FT_GET_PS_FONT_INFO 1

/* Define to 1 if you have the `FT_Get_X11_Font_Format' function. */
#cmakedefine HAVE_FT_GET_X11_FONT_FORMAT 1

/* Define to 1 if you have the `FT_Has_PS_Glyph_Names' function. */
#define HAVE_FT_HAS_PS_GLYPH_NAMES 1

/* Define to 1 if you have the `FT_Select_Size' function. */
#cmakedefine HAVE_FT_SELECT_SIZE 1

/* Define to 1 if you have the `getexecname' function. */
#cmakedefine HAVE_GETEXECNAME 1

/* Define to 1 if you have the `getopt' function. */
#define HAVE_GETOPT 1

/* Define to 1 if you have the `getopt_long' function. */
#define HAVE_GETOPT_LONG 1

/* Define to 1 if you have the `getpagesize' function. */
#cmakedefine HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `getprogname' function. */
#cmakedefine HAVE_GETPROGNAME 1

/* Have Intel __sync_* atomic primitives */
#cmakedefine HAVE_INTEL_ATOMIC_PRIMITIVES

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the `link' function. */
#cmakedefine HAVE_LINK 1

/* Define to 1 if you have the `lrand48' function. */
#cmakedefine HAVE_LRAND48 1

/* Define to 1 if you have the `lstat' function. */
#cmakedefine HAVE_LSTAT 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkdtemp' function. */
#cmakedefine HAVE_MKDTEMP 1

/* Define to 1 if you have the `mkostemp' function. */
#cmakedefine HAVE_MKOSTEMP 1

/* Define to 1 if you have the `mkstemp' function. */
#cmakedefine HAVE_MKSTEMP 1

/* Define to 1 if you have a working `mmap' system call. */
#cmakedefine HAVE_MMAP 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H 1

/* Define to 1 if you have the 'posix_fadvise' function. */
#cmakedefine HAVE_POSIX_FADVISE 1

/* Have POSIX threads */
#cmakedefine CMAKE_USE_PTHREADS_INIT ${CMAKE_USE_PTHREADS_INIT}
#if defined(CMAKE_USE_PTHREADS_INIT) && CMAKE_USE_PTHREADS_INIT
#  define HAVE_PTHREAD 1
#else
#  undef HAVE_PTHREAD
#endif

/* Have PTHREAD_PRIO_INHERIT. */
#cmakedefine HAVE_PTHREAD_PRIO_INHERIT

/* Define to 1 if you have the `rand' function. */
#cmakedefine HAVE_RAND 1

/* Define to 1 if you have the `random' function. */
#cmakedefine HAVE_RANDOM 1

/* Define to 1 if you have the `random_r' function. */
#cmakedefine HAVE_RANDOM_R 1

/* Define to 1 if you have the `rand_r' function. */
#cmakedefine HAVE_RAND_R 1

/* Define to 1 if you have the `readlink' function. */
#cmakedefine HAVE_READLINK 1

/* Define to 1 if you have the <sched.h> header file. */
#cmakedefine HAVE_SCHED_H 1

/* Have sched_yield */
#cmakedefine HAVE_SCHED_YIELD

/* Have Solaris __machine_*_barrier and atomic_* operations */
#cmakedefine HAVE_SOLARIS_ATOMIC_OPS

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if `d_type' is a member of `struct dirent'. */
#cmakedefine HAVE_STRUCT_DIRENT_D_TYPE 1

/* Define to 1 if `f_flags' is a member of `struct statfs'. */
#cmakedefine HAVE_STRUCT_STATFS_F_FLAGS 1

/* Define to 1 if `f_fstypename' is a member of `struct statfs'. */
#cmakedefine HAVE_STRUCT_STATFS_F_FSTYPENAME 1

/* Define to 1 if `f_basetype' is a member of `struct statvfs'. */
#cmakedefine HAVE_STRUCT_STATVFS_F_BASETYPE 1

/* Define to 1 if `f_fstypename' is a member of `struct statvfs'. */
#cmakedefine HAVE_STRUCT_STATVFS_F_FSTYPENAME 1

/* Define to 1 if `st_mtim' is a member of `struct stat'. */
#cmakedefine HAVE_STRUCT_STAT_ST_MTIM 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H 1

/* Define to 1 if you have the <sys/mount.h> header file. */
#cmakedefine HAVE_SYS_MOUNT_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#cmakedefine HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/statfs.h> header file. */
#cmakedefine HAVE_SYS_STATFS_H 1

/* Define to 1 if you have the <sys/statvfs.h> header file. */
#cmakedefine HAVE_SYS_STATVFS_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/vfs.h> header file. */
#cmakedefine HAVE_SYS_VFS_H 1

/* Define to 1 if `usLowerOpticalPointSize' is a member of `TT_OS2'. */
#cmakedefine HAVE_TT_OS2_USLOWEROPTICALPOINTSIZE 1

/* Define to 1 if `usUpperOpticalPointSize' is a member of `TT_OS2'. */
#cmakedefine HAVE_TT_OS2_USUPPEROPTICALPOINTSIZE 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the `vprintf' function. */
#cmakedefine HAVE_VPRINTF 1

/* Can use #warning in C files */
#cmakedefine HAVE_WARNING_CPP_DIRECTIVE

/* Use xmlparse.h instead of expat.h */
#cmakedefine HAVE_XMLPARSE_H

/* Define to 1 if you have the `XML_SetDoctypeDeclHandler' function. */
#cmakedefine HAVE_XML_SETDOCTYPEDECLHANDLER 1

/* Define to 1 if you have the `_mktemp_s' function. */
#cmakedefine HAVE__MKTEMP_S 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#cmakedefine LT_OBJDIR

/* Name of package */
#cmakedefine PACKAGE

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
#cmakedefine PTHREAD_CREATE_JOINABLE

/* The size of `char', as computed by sizeof. */
#cmakedefine SIZEOF_CHAR

/* The size of `int', as computed by sizeof. */
#cmakedefine SIZEOF_INT

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG

/* The size of `short', as computed by sizeof. */
#cmakedefine SIZEOF_SHORT

/* The size of `void*', as computed by sizeof. */
#cmakedefine SIZEOF_VOIDP

/* The size of `void *', as computed by sizeof. */
#cmakedefine SIZEOF_VOID_P

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* Use iconv. */
#cmakedefine USE_ICONV

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


/* Version number of package */
#cmakedefine VERSION

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
#  undef WORDS_BIGENDIAN
# endif
#endif

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#cmakedefine _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#cmakedefine _LARGE_FILES

/* Define to 1 if on MINIX. */
#cmakedefine _MINIX 1

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#cmakedefine _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#cmakedefine _POSIX_SOURCE 1

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#define inline __inline
#endif

/* Define to `int' if <sys/types.h> does not define. */
#undef pid_t

#include "config-fixups.h"
