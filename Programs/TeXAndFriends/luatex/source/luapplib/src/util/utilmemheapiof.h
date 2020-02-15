
#ifndef UTIL_MEM_HEAP_IOF_H
#define UTIL_MEM_HEAP_IOF_H

#include "utilmemheap.h"
#include "utiliof.h"

UTILAPI iof * heap8_buffer_init (heap8 *heap, iof *O);
UTILAPI iof * heap16_buffer_init (heap16 *heap, iof *O);
UTILAPI iof * heap32_buffer_init (heap32 *heap, iof *O);
UTILAPI iof * heap64_buffer_init (heap64 *heap, iof *O);

UTILAPI iof * heap8_buffer_some (heap8 *heap, iof *O, size_t atleast);
UTILAPI iof * heap16_buffer_some (heap16 *heap, iof *O, size_t atleast);
UTILAPI iof * heap32_buffer_some (heap32 *heap, iof *O, size_t atleast);
UTILAPI iof * heap64_buffer_some (heap64 *heap, iof *O, size_t atleast);

#endif