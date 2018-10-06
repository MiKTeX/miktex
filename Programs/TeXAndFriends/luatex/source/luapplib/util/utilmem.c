
#include <string.h> // for memcpy

#include "utilmem.h"
#include "utillog.h"

#ifndef util_memerr
#  if defined(_WIN64) || defined(__MINGW32__)
#    define util_memerr(size) { loggerf("ooops, not enough memory (%I64u)", ((unsigned long long)(size))); abort(); }
#  else
#    define util_memerr(size) { loggerf("ooops, not enough memory (%llu)", ((unsigned long long)(size))); abort(); }
#  endif
#endif

void * util_malloc (size_t size)
{
  void *m;
  if ((m = malloc(size)) == NULL)
    util_memerr(size);
  return m;
}

void * util_calloc (size_t num, size_t size)
{
  void *m;
  if ((m = calloc(num, size)) == NULL)
    util_memerr(size);
  return m;
}

void * util_realloc (void *m, size_t size)
{
  if ((m = realloc(m, size)) == NULL)
    util_memerr(size);
  return m;
}

/* common array resizer

data -- the beginning of array
unit -- sizeof array element
size -- current array size
extra -- requested extra size
space -- pointer to available space
allocated -- flag indicating if *data has been allocated (with malloc)

*/

void util_resize (void **data, size_t unit, size_t size, size_t extra, size_t *space, int allocated)
{
  if (*space == 0)
    *space = 4; // ... better keep *space non-zero to avoid it
  do { *space <<= 1; }
  while (size + extra > *space);

  if (allocated)
  {
    *data = util_realloc(*data, *space * unit);
  }
  else
  {
    void *newdata = util_malloc(*space * unit);
    if (*data != NULL)
      memcpy(newdata, *data, size * unit);
    *data = newdata;
  }
}

