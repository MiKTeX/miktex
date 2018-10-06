
#ifndef UTIL_DECL_H
#define UTIL_DECL_H

/*
UTILDLL - when building .dll
UTILEXE - when building .exe to import symbols from .dll
*/

#if defined (_WIN32) || defined(_WIN64)
#  ifdef UTILDLL
#    define UTILAPI __declspec(dllexport)
#    define UTILDEF __declspec(dllexport) 
#  else
#    ifdef UTILEXE
#      define UTILAPI __declspec(dllimport)
#      define UTILDEF
#    else
#      define UTILAPI
#      define UTILDEF
#    endif
#  endif
#else
#  define UTILAPI
#  define UTILDEF
#endif

#endif