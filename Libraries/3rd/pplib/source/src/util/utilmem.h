
#ifndef UTIL_MEM_H
#define UTIL_MEM_H

#include <stdlib.h> // for size_t and alloc functions
#include "utildecl.h"

UTILAPI void * util_malloc (size_t size);
UTILAPI void * util_calloc (size_t num, size_t size);
UTILAPI void * util_realloc (void *m, size_t size);

void util_resize (void **data, size_t unit, size_t size, size_t extra, size_t *space, int allocated);

#define util_free free // not a call, might be used as identifier

#endif
