#ifndef __ZZIP_INTERNAL_ERRNO_H
#define __ZZIP_INTERNAL_ERRNO_H

#include <errno.h>

/* Mingw cross compile fix */
#ifndef EBADMSG
#define EBADMSG 74
#endif

#endif
