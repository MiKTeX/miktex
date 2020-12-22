
#ifndef PP_HEAP_H
#define PP_HEAP_H

#include "utilmem.h"

#define pp_malloc util_malloc
//#define pp_callic util_calloc
//#define pp_realloc util_realloc
#define pp_free util_free

#include "utilmemheapiof.h"
//#include "utilmeminfo.h"

#define ppbytes_heap heap16
#define ppbytes_heap_init(heap, space, large, flags) (heap16_init(&(heap)->bytesheap, space, large, flags), heap16_head(&(heap)->bytesheap))
//#define ppbytes_heap_some(heap, size, pspace) _heap16_some(&(heap)->bytesheap, size, pspace)
#define ppbytes_heap_done(heap, data, written) heap16_done(&(heap)->bytesheap, data, written)
#define ppbytes_heap_clear(heap) heap16_clear(&(heap)->bytesheap)
#define ppbytes_heap_free(heap) heap16_free(&(heap)->bytesheap)
#define ppbytes_heap_info(heap, info, append) heap16_stats(&(heap)->bytesheap, info, append)

#define ppbytes_take(heap, size) _heap16_take(&(heap)->bytesheap, size)
#define ppbytes_buffer_init(heap) heap16_buffer_init(&(heap)->bytesheap, &(heap)->bytesbuffer)
#define ppbytes_buffer(heap, atleast) _heap16_buffer_some(&(heap)->bytesheap, &(heap)->bytesbuffer, atleast)

#define ppstruct_heap heap64
#define ppstruct_heap_init(heap, space, large, flags) (heap64_init(&(heap)->structheap, space, large, flags), heap64_head(&(heap)->structheap))
#define ppstruct_heap_clear(heap) heap64_clear(&(heap)->structheap)
#define ppstruct_heap_free(heap) heap64_free(&(heap)->structheap)
#define ppstruct_heap_info(heap, info, append) heap64_stats(&(heap)->structheap, info, append)
#define ppstruct_take(heap, size) _heap64_take(&(heap)->structheap, size)

typedef struct {
  ppbytes_heap bytesheap;
  ppstruct_heap structheap;
  iof bytesbuffer;
} ppheap;

ppbyte * ppbytes_flush (ppheap *heap, iof *O, size_t *psize);

void ppheap_init (ppheap *heap);
void ppheap_free (ppheap *heap);
void ppheap_renew (ppheap *heap);

#endif