/* src/config.h.in.  Generated from configure.ac by autoheader.  */

/* Inline assembly available (only VC/x86_64) */
#cmakedefine AO_ASM_X64_AVAILABLE

/* Assume Windows Server 2003, Vista or later target (only VC/x86) */
#cmakedefine AO_ASSUME_VISTA

/* Assume target is not old AMD Opteron chip (only x86_64) */
#cmakedefine AO_CMPXCHG16B_AVAILABLE

/* Define to avoid C11 atomic intrinsics even if available. */
#cmakedefine AO_DISABLE_GCC_ATOMICS

/* Force test_and_set to use SWP instruction instead of LDREX/STREX (only arm
   v6+) */
#cmakedefine AO_FORCE_USE_SWP

/* Force compare_and_swap definition via fetch_compare_and_swap */
#cmakedefine AO_GENERALIZE_ASM_BOOL_CAS

/* No pthreads library available */
#cmakedefine AO_NO_PTHREADS

/* Assume target is not sparc v9+ (only sparc) */
#cmakedefine AO_NO_SPARC_V9

/* Assume ancient MS VS Win32 headers (only VC/arm v6+, VC/x86) */
#cmakedefine AO_OLD_STYLE_INTERLOCKED_COMPARE_EXCHANGE

/* Prefer C11 atomic intrinsics over assembly-based implementation even in
   case of inefficient implementation (do not use assembly for any atomic_ops
   primitive if C11/GCC atomic intrinsics available) */
#cmakedefine AO_PREFER_BUILTIN_ATOMICS

/* Prefer generalized definitions to direct assembly-based ones */
#cmakedefine AO_PREFER_GENERALIZED

/* Trace AO_malloc/free calls (for debug only) */
#cmakedefine AO_TRACE_MALLOC

/* Assume single-core target (only arm v6+) */
#cmakedefine AO_UNIPROCESSOR

/* Assume Win32 _Interlocked primitives available as intrinsics (only VC/arm)
   */
#cmakedefine AO_USE_INTERLOCKED_INTRINSICS

/* Use nanosleep() instead of select() (only if atomic operations are
   emulated) */
#cmakedefine AO_USE_NANOSLEEP

/* Do not block signals in compare_and_swap (only if atomic operations are
   emulated) */
#cmakedefine AO_USE_NO_SIGNALS

/* Use Pentium 4 'mfence' instruction (only x86) */
#cmakedefine AO_USE_PENTIUM4_INSTRS

/* Emulate atomic operations via slow and async-signal-unsafe pthread locking
   */
#cmakedefine AO_USE_PTHREAD_DEFS

/* Prefer GCC built-in CAS intrinsics in favor of inline assembly (only
   gcc/x86, gcc/x86_64) */
#cmakedefine AO_USE_SYNC_CAS_BUILTIN

/* Use Win32 Sleep() instead of select() (only if atomic operations are
   emulated) */
#cmakedefine AO_USE_WIN32_PTHREADS

/* Emulate double-width CAS via pthread locking in case of no hardware support
   (only gcc/x86_64, the emulation is unsafe) */
#cmakedefine AO_WEAK_DOUBLE_CAS_EMULATION

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H

/* Define to 1 if you have the `getpagesize' function. */
#cmakedefine HAVE_GETPAGESIZE

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H

/* Define to 1 if you have a working `mmap' system call. */
#cmakedefine HAVE_MMAP

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H

/* Define to 1 if you have the <sys/param.h> header file. */
#cmakedefine HAVE_SYS_PARAM_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#cmakedefine LT_OBJDIR

/* Define to disable assertion checking. */
#cmakedefine NDEBUG

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

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS

/* Version number of package */
#cmakedefine VERSION

/* Indicates the use of pthreads (NetBSD). */
#cmakedefine _PTHREADS

/* Required define if using POSIX threads. */
#cmakedefine _REENTRANT
