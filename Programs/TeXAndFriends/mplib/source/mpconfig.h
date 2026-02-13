#ifndef METAPOST_MPCONFIG_H
#define METAPOST_MPCONFIG_H

#include <stdint.h>

/* For the moment distinct from integer64 but same type */

#if defined(WIN32)
typedef __int64 mpinteger64;
#else
typedef int64_t mpinteger64;
#endif


/* In C23 true and false are keywords */
/* and  the value of the macro  __STDC_VERSION__ is 202311L */
#undef true
#undef false
#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ <202311L) /* before C23 */
# define true 1
# define false 0
#endif




#endif /* not METAPOST_MPCONFIG_H */
