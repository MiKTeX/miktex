/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
#cmakedefine CRAY_STACKSEG_END

/* Define to 1 if using `alloca.c'. */
#cmakedefine C_ALLOCA 1

/* Define to 1 if you have `alloca', as a function or macro. */
#cmakedefine HAVE_ALLOCA 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#cmakedefine HAVE_ALLOCA_H 1

/* Define if denormalized floats work. */
#cmakedefine HAVE_DENORMS

/* Define to 1 if the system has the type `intmax_t'. */
#cmakedefine HAVE_INTMAX_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define one of the following to 1 for the format of a `long double'.
   If your format is not among these choices, or you don't know what it is,
   then leave all undefined.
   IEEE_EXT is the 10-byte IEEE extended precision format.
   IEEE_QUAD is the 16-byte IEEE quadruple precision format.
   LITTLE or BIG is the endianness.  */
#cmakedefine HAVE_LDOUBLE_IEEE_EXT_LITTLE 1
#cmakedefine HAVE_LDOUBLE_IEEE_QUAD_LITTLE 1
#cmakedefine HAVE_LDOUBLE_IEEE_QUAD_BIG 1

/* Define if compiler supports long long */
#cmakedefine HAVE_LONG_LONG

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define if stdarg */
#cmakedefine HAVE_STDARG 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the `va_copy' function. */
#cmakedefine HAVE_VA_COPY 1

/* Define to 1 if you have the <wchar.h> header file. */
#cmakedefine HAVE_WCHAR_H 1

/* Define to 1 if you have the `__va_copy' function. */
#cmakedefine HAVE___VA_COPY 1

/* Define if the FP division by 0 fails. */
#cmakedefine MPFR_ERRDIVZERO

/* Define if you have the `fesetround' function via the <fenv.h> header file.
   */
#cmakedefine MPFR_HAVE_FESETROUND

/* Define if you have a working INTMAX_MAX. */
#cmakedefine MPFR_HAVE_INTMAX_MAX

/* Define if NAN == NAN. */
#cmakedefine MPFR_NANISNAN

/* Charset is not consecutive */
#cmakedefine MPFR_NO_CONSECUTIVE_CHARSET

/* gmp_printf cannot use `hh' length modifier */
#cmakedefine NPRINTF_HH

/* gmp_printf cannot read intmax_t */
#cmakedefine NPRINTF_J

/* gmp_printf cannot read long double */
#cmakedefine NPRINTF_L

/* gmp_printf cannot read long long int */
#cmakedefine NPRINTF_LL

/* gmp_printf cannot read ptrdiff_t */
#cmakedefine NPRINTF_T

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

/* Define to `((size_t)-1)' if <stdint.h> does not define it. */
#cmakedefine SIZE_MAX

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
#cmakedefine STACK_DIRECTION

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME 1

/* Version number of package */
#cmakedefine VERSION

/* Define to `unsigned int' if <sys/types.h> does not define. */
#cmakedefine size_t
