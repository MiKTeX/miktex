/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
#undef AC_APPLE_UNIVERSAL_BUILD

/* whether memory barriers are needed around atomic operations */
#undef ATOMIC_OP_NEEDS_MEMORY_BARRIER

/* Define to 1 if the PDF backend can be tested (need poppler and other
   dependencies for pdf2png) */
#cmakedefine CAIRO_CAN_TEST_PDF_SURFACE 1

/* Define to 1 if the PS backend can be tested (needs ghostscript) */
#cmakedefine CAIRO_CAN_TEST_PS_SURFACE 1

/* Define to 1 if the SVG backend can be tested */
#cmakedefine CAIRO_CAN_TEST_SVG_SURFACE 1

/* Define to 1 if the Win32 Printing backend can be tested (needs ghostscript)
   */
#cmakedefine CAIRO_CAN_TEST_WIN32_PRINTING_SURFACE 1

/* Define to 1 if dlsym is available */
#cmakedefine CAIRO_HAS_DLSYM 1

/* Define to 1 to enable cairo's cairo-script-interpreter feature */
#cmakedefine CAIRO_HAS_INTERPRETER 1

/* Define to 1 to enable cairo's pthread feature */
#cmakedefine CAIRO_HAS_PTHREAD 1

/* Define to 1 if we have full pthread support */
#cmakedefine CAIRO_HAS_REAL_PTHREAD 1

/* Define to 1 if libspectre is available */
#cmakedefine CAIRO_HAS_SPECTRE 1

/* Define to 1 to enable cairo's symbol-lookup feature */
#cmakedefine CAIRO_HAS_SYMBOL_LOOKUP 1

/* Define to 1 to enable cairo's test surfaces feature */
#cmakedefine CAIRO_HAS_TEST_SURFACES 1

/* Define to 1 to enable cairo's cairo-trace feature */
#cmakedefine CAIRO_HAS_TRACE 1

/* Define to 1 to disable certain code paths that rely heavily on double
   precision floating-point calculation */
#cmakedefine DISABLE_SOME_FLOATING_POINT 1

/* Define to 1 if your system stores words within floats with the most
   significant word first */
#cmakedefine FLOAT_WORDS_BIGENDIAN 1

/* Define to (0) if freetype2 does not support color fonts */
#undef FT_HAS_COLOR

/* Enable pixman glyph cache */
#undef HAS_PIXMAN_GLYPHS

/* Define to 1 if you have the `alarm' function. */
#cmakedefine HAVE_ALARM 1

/* Define to 1 if you have the binutils development files installed */
#cmakedefine HAVE_BFD 1

/* Define to 1 if your compiler supports the __builtin_return_address()
   intrinsic. */
#cmakedefine HAVE_BUILTIN_RETURN_ADDRESS 1

/* Define to 1 if you have the <byteswap.h> header file. */
#cmakedefine HAVE_BYTESWAP_H 1

/* Define to 1 if you have the `clock_gettime' function. */
#cmakedefine HAVE_CLOCK_GETTIME 1

/* Define to 1 if you have the `ctime_r' function. */
#cmakedefine HAVE_CTIME_R 1

/* Enable if your compiler supports the GCC __atomic_* atomic primitives */
#undef HAVE_CXX11_ATOMIC_PRIMITIVES

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you have the `drand48' function. */
#cmakedefine HAVE_DRAND48 1

/* Define to 1 if you have the `FcFini' function. */
#cmakedefine HAVE_FCFINI 1

/* Define to 1 if you have the `FcInit' function. */
#cmakedefine HAVE_FCINIT 1

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H 1

/* Define to 1 if you have the `feclearexcept' function. */
#cmakedefine HAVE_FECLEAREXCEPT 1

/* Define to 1 if you have the `fedisableexcept' function. */
#cmakedefine HAVE_FEDISABLEEXCEPT 1

/* Define to 1 if you have the `feenableexcept' function. */
#cmakedefine HAVE_FEENABLEEXCEPT 1

/* Define to 1 if you have the <fenv.h> header file. */
#cmakedefine HAVE_FENV_H 1

/* Define to 1 if you have the `flockfile' function. */
#cmakedefine HAVE_FLOCKFILE 1

/* Define to 1 if you have the `fork' function. */
#cmakedefine HAVE_FORK 1

/* Define to 1 if you have the `FT_Done_MM_Var' function. */
#cmakedefine HAVE_FT_DONE_MM_VAR 1

/* Define to 1 if you have the `FT_Get_Var_Design_Coordinates' function. */
#cmakedefine HAVE_FT_GET_VAR_DESIGN_COORDINATES 1

/* Define to 1 if you have the `FT_Get_X11_Font_Format' function. */
#cmakedefine HAVE_FT_GET_X11_FONT_FORMAT 1

/* Define to 1 if you have the `FT_GlyphSlot_Embolden' function. */
#cmakedefine HAVE_FT_GLYPHSLOT_EMBOLDEN 1

/* Define to 1 if you have the `FT_GlyphSlot_Oblique' function. */
#cmakedefine HAVE_FT_GLYPHSLOT_OBLIQUE 1

/* Define to 1 if you have the `FT_Library_SetLcdFilter' function. */
#cmakedefine HAVE_FT_LIBRARY_SETLCDFILTER 1

/* Define to 1 if you have the `FT_Load_Sfnt_Table' function. */
#cmakedefine HAVE_FT_LOAD_SFNT_TABLE 1

/* Define to 1 if you have the `funlockfile' function. */
#cmakedefine HAVE_FUNLOCKFILE 1

/* Enable if your compiler supports the legacy GCC __sync_* atomic primitives
   */
#cmakedefine HAVE_GCC_LEGACY_ATOMICS 1

/* Whether you have gcov */
#cmakedefine HAVE_GCOV 1

/* Define to 1 if you have the `getline' function. */
#cmakedefine HAVE_GETLINE 1

/* Define to 1 if you have the `gmtime_r' function. */
#cmakedefine HAVE_GMTIME_R 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the <io.h> header file. */
#cmakedefine HAVE_IO_H 1

/* Define to 1 if you have the <libgen.h> header file. */
#cmakedefine HAVE_LIBGEN_H 1

/* Define to 1 if you have the `rt' library (-lrt). */
#cmakedefine HAVE_LIBRT 1

/* Enable if you have libatomic-ops-dev installed */
#undef HAVE_LIB_ATOMIC_OPS

/* Define to 1 if you have the `link' function. */
#cmakedefine HAVE_LINK 1

/* Define to 1 if you have the `localtime_r' function. */
#cmakedefine HAVE_LOCALTIME_R 1

/* Define to 1 if you have the Valgrind lockdep tool */
#cmakedefine HAVE_LOCKDEP 1

/* Define to 1 if you have lzo available */
#cmakedefine HAVE_LZO 1

/* Define to 1 if you have the Valgrind memfault tool */
#cmakedefine HAVE_MEMFAULT 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to non-zero if your system has mkdir, and to 2 if your version of
   mkdir requires a mode parameter */
#undef HAVE_MKDIR

/* Define to 1 if you have the `mmap' function. */
#cmakedefine HAVE_MMAP 1

/* Define to 1 if you have the `newlocale' function. */
#cmakedefine HAVE_NEWLOCALE 1

/* Enable if you have MacOS X atomic operations */
#undef HAVE_OS_ATOMIC_OPS

/* Define to 1 if you have the `poppler_page_render' function. */
#cmakedefine HAVE_POPPLER_PAGE_RENDER 1

/* Define to 1 if you have the `raise' function. */
#cmakedefine HAVE_RAISE 1

/* Define to 1 if you have the `rsvg_pixbuf_from_file' function. */
#cmakedefine HAVE_RSVG_PIXBUF_FROM_FILE 1

/* Define to 1 if you have the `sched_getaffinity' function. */
#cmakedefine HAVE_SCHED_GETAFFINITY 1

/* Define to 1 if you have the <sched.h> header file. */
#cmakedefine HAVE_SCHED_H 1

/* Define to 1 if you have the <setjmp.h> header file. */
#cmakedefine HAVE_SETJMP_H 1

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the `strndup' function. */
#cmakedefine HAVE_STRNDUP 1

/* Define to 1 if you have the `strtod_l' function. */
#cmakedefine HAVE_STRTOD_L 1

/* Define to 1 if you have the <sys/int_types.h> header file. */
#cmakedefine HAVE_SYS_INT_TYPES_H 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#cmakedefine HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/mman.h> header file. */
#cmakedefine HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/poll.h> header file. */
#cmakedefine HAVE_SYS_POLL_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#cmakedefine HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
#cmakedefine HAVE_SYS_UN_H 1

/* Define to 1 if you have the <sys/wait.h> header file. */
#cmakedefine HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <time.h> header file. */
#cmakedefine HAVE_TIME_H 1

/* Define to 1 if typeof works with your compiler. */
#cmakedefine HAVE_TYPEOF 1

/* Define to 1 if the system has the type `uint128_t'. */
#cmakedefine HAVE_UINT128_T 1

/* Define to 1 if the system has the type `uint64_t'. */
#cmakedefine HAVE_UINT64_T 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have Valgrind */
#cmakedefine HAVE_VALGRIND 1

/* Define to 1 if you have the `waitpid' function. */
#cmakedefine HAVE_WAITPID 1

/* Define to 1 if you have the <X11/extensions/shmproto.h> header file. */
#cmakedefine HAVE_X11_EXTENSIONS_SHMPROTO_H 1

/* Define to 1 if you have the <X11/extensions/shmstr.h> header file. */
#cmakedefine HAVE_X11_EXTENSIONS_SHMSTR_H 1

/* Define to 1 if you have the <X11/extensions/XShm.h> header file. */
#cmakedefine HAVE_X11_EXTENSIONS_XSHM_H 1

/* Define to 1 if you have the <xlocale.h> header file. */
#cmakedefine HAVE_XLOCALE_H 1

/* Define to 1 if you have the `XRenderCreateConicalGradient' function. */
#cmakedefine HAVE_XRENDERCREATECONICALGRADIENT 1

/* Define to 1 if you have the `XRenderCreateLinearGradient' function. */
#cmakedefine HAVE_XRENDERCREATELINEARGRADIENT 1

/* Define to 1 if you have the `XRenderCreateRadialGradient' function. */
#cmakedefine HAVE_XRENDERCREATERADIALGRADIENT 1

/* Define to 1 if you have the `XRenderCreateSolidFill' function. */
#cmakedefine HAVE_XRENDERCREATESOLIDFILL 1

/* Define to 1 if you have zlib available */
#cmakedefine HAVE_ZLIB 1

/* Define to 1 if the system has the type `__uint128_t'. */
#cmakedefine HAVE___UINT128_T 1

/* Define to 1 if shared memory segments are released deferred. */
#cmakedefine IPC_RMID_DEFERRED_RELEASE 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#undef LT_OBJDIR

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* Shared library file extension */
#undef SHARED_LIB_EXT

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT ${SIZEOF_INT}

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG ${SIZEOF_LONG}

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG ${SIZEOF_LONG_LONG}

/* The size of `size_t', as computed by sizeof. */
#undef SIZEOF_SIZE_T

/* The size of `void *', as computed by sizeof. */
#define SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P}

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


/* Define to the value your compiler uses to support the warn-unused-result
   attribute */
#define WARN_UNUSED_RESULT

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


/* Deal with multiple architecture compiles on Mac OS X */
#ifdef __APPLE_CC__
#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#define FLOAT_WORDS_BIGENDIAN 1
#else
#undef WORDS_BIGENDIAN
#undef FLOAT_WORDS_BIGENDIAN
#endif
#endif


/* Define to 1 if the X Window System is missing or not being used. */
#cmakedefine X_DISPLAY_MISSING 1

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to 1 if on MINIX. */
#undef _MINIX

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#undef _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#cmakedefine _POSIX_SOURCE 1

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#define inline __inline
#endif

/* Define to __typeof__ if your compiler spells it that way. */
#undef typeof
