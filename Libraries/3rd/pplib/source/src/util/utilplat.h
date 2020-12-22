
#ifndef UTIL_PLAT_H
#define UTIL_PLAT_H

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

#ifdef __GNUC__
//#  define FALLTHRU [[fallthrough]] // c++17
//#  define FALLTHRU [[gnu:fallthrough]] // c++14
#  define FALLTHRU __attribute__((fallthrough)); // C and C++03
#else
#  define FALLTHRU
#endif

#endif