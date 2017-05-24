/* Guard against double inclusion. */
#ifndef FF_CONFIG_H
#define FF_CONFIG_H

#define _NO_FFSCRIPT 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the <langinfo_h> header file. */
#cmakedefine HAVE_LANGINFO_H 1

/* Define to 1 if the system has the type `long double'. */
#cmakedefine HAVE_LONG_DOUBLE 1

/* Define to 1 if the system has the type `long long int'. */
#cmakedefine HAVE_LONG_LONG_INT 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

#ifdef HAVE_LONG_LONG_INT
# define _HAS_LONGLONG 1
#endif

#endif /* !FF_CONFIG_H */
