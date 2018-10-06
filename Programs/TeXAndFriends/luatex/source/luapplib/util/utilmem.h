
#ifndef UTIL_MEM_H
#define UTIL_MEM_H

#include <stdlib.h> // for size_t and alloc functions
#include "utildecl.h"

UTILAPI void * util_malloc (size_t size);
UTILAPI void * util_calloc (size_t num, size_t size);
UTILAPI void * util_realloc (void *m, size_t size);

#define util_free free // not a call

/* arrays */

#define array_header(element_type, integer_type) \
    element_type *data; integer_type size; integer_type space

#define array_of(element_type) array_header(element_type, size_t)

#define array_data_is_allocated(array) ((array)->data != (void *)((array) + 1))

#define array_create(array, array_type, element_type, init) \
  (array = (array_type *)util_malloc(sizeof(array_type) + ((init) > 0 ? ((init) * sizeof(element_type)) : 1)), \
   array_init(array, element_type, init))

#define array_init(array, element_type, init) \
  ((array)->data = (element_type *)((array) + 1), (array)->size = 0, (array)->space = init)

#define array_init_data(array, element_type, init) \
  ((array)->data = (element_type *)util_malloc((init) * sizeof(element_type)), (array)->size = 0, (array)->space = init)

#define array_free_data(array) \
  ((void)(array_data_is_allocated(array) && (util_free((array)->data), 0)))

#define array_free(array) \
  ((void)(array_free_data(array), (util_free(array), 0)))

void util_resize (void **data, size_t unit, size_t size, size_t extra, size_t *space, int allocated);

#define array_ensure(array, unit, extra) \
  ((void)((array)->size + (extra) > (array)->space && \
         (util_resize((void **)(&(array)->data), unit, (array)->size, extra, &(array)->space, array_data_is_allocated(array)), 0)))

#define array_ensure_alloc(array, unit, extra, isalloc) \
  ((void)((array)->size + (extra) > (array)->space && \
         (util_resize((void **)(&(array)->data), unit, (array)->size, extra, &(array)->space, isalloc), 0)))

#define array_at(array, index) ((array)->data + index)
#define array_index(array, index) (index < (array)->size ? array_at(array, index) : NULL)
#define array_top(array) ((array)->data + (array)->size - 1)

#define array_from_bottom(array, index) array_at(array, index - 1)
#define array_from_top(array, negindex) array_at(array, (array)->size + (negindex))

#define array_push(array) ((array)->data + ((array)->size)++)
#define array_pop(array) ((array)->data + --((array)->size))

#endif
