
#ifndef PP_CONF_H
#define PP_CONF_H

//#include "utilarm.h" // keep in sync
#if defined __arm__ || defined __ARM__ || defined ARM || defined __ARM || defined __arm || defined __ARM_ARCH ||defined __aarch64__ ||( defined(__sun) && defined(__SVR4))
#  define ARM_COMPLIANT 1
#else
#  define ARM_COMPLIANT 0
#endif

/*
Aux flags:
  PPDLL -- indicates a part of a shared library
  PPEXE -- indicates a host program using shared library functions
*/

#if defined(_WIN32) || defined(_WIN64)
#  ifdef PPDLL
#    define PPAPI __declspec(dllexport)
#    define PPDEF __declspec(dllexport)
#  else
#    ifdef PPEXE
#      define PPAPI __declspec(dllimport)
#      define PPDEF
#    else
#      define PPAPI
#      define PPDEF
#    endif
#  endif
#else
#  define PPAPI
#  define PPDEF
#endif

/* platform vs integers */

#if defined(_WIN32) || defined(WIN32)
#  ifdef _MSC_VER
#    if defined(_M_64) || defined(_WIN64)
#      define MSVC64
#    else
#      define MSVC32
#    endif
#  else
#    if defined(__MINGW64__)
#      define MINGW64
#    else
#      if defined(__MINGW32__)
#        define MINGW32
#      endif
#    endif
#  endif
#endif

#if defined(_WIN64) || defined(__MINGW32__)
#  define PPINT64F "%I64d"
#  define PPUINT64F "%I64u"
#else
#  define PPINT64F "%lld"
#  define PPUINT64F "%llu"
#endif

#if defined(MSVC64)
#  define PPINT(N) N##I64
#  define PPUINT(N) N##UI64
#  define PPINTF PPINT64F
#  define PPUINTF PPUINT64F
#elif defined(MINGW64)
#  define PPINT(N) N##LL
#  define PPUINT(N) N##ULL
#  define PPINTF PPINT64F
#  define PPUINTF PPUINT64F
#else // 32bit or sane 64bit (LP64, where long is long indeed)
#  define PPINT(N) N##L
#  define PPUINT(N) N##UL
#  define PPINTF "%ld"
#  define PPUINTF "%lu"
#endif

#define PPSIZEF PPUINTF

#endif
