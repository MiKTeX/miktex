#ifndef LCDF_TYPETOOLS_CONFIG_H
#define LCDF_TYPETOOLS_CONFIG_H 1

#include <autoconf.h>
#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif

/* Allow compilation on Windows (thanks, Fabrice Popineau). */
#if defined(MIKTEX_WINDOWS)
# define CDECL
#else
#ifdef WIN32
# ifdef __MINGW32__
#  include <winsock2.h>
# else
#  include <win32lib.h>
# endif
#else
# include <stddef.h>
# define CDECL /* nothing */
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Prototype strerror if necessary. */
#if !HAVE_STRERROR
char *strerror(int errno);
#endif

/* Prototype strnlen if necessary. */
#if !HAVE_DECL_STRNLEN
size_t strnlen(const char *s, size_t maxlen);
#endif

/* Prototype good_strtod if necessary. */
#if HAVE_BROKEN_STRTOD
double good_strtod(const char *s, char **endptr);
#endif

#ifdef __cplusplus
}
/* Get rid of a possible inline macro under C++. */
#if !defined(MIKTEX)
# define inline inline
#endif

#if !defined(MIKTEX)
/* Ignore `noexcept` if compiler is too old. */
#if __cplusplus < 201103L
#define noexcept
#endif
#endif
#endif

#endif /* LCDF_TYPETOOLS_CONFIG_H */
