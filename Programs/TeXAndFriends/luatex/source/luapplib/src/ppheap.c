
#include "pplib.h"

#define PPBYTES_HEAP_BLOCK 0xFFF
#define PPBYTES_HEAP_LARGE (PPBYTES_HEAP_BLOCK >> 2)
#define PPSTRUCT_HEAP_BLOCK 0xFFF
#define PPSTRUCT_HEAP_LARGE (PPSTRUCT_HEAP_BLOCK >> 2)

void ppheap_init (ppheap *heap)
{
  ppstruct_heap_init(heap, PPSTRUCT_HEAP_BLOCK, PPSTRUCT_HEAP_LARGE, 0);
  ppbytes_heap_init(heap, PPBYTES_HEAP_BLOCK, PPBYTES_HEAP_LARGE, 0);
}

void ppheap_free (ppheap *heap)
{
  ppstruct_heap_free(heap);
  ppbytes_heap_free(heap);
}

void ppheap_renew (ppheap *heap)
{
  ppstruct_heap_clear(heap);
  ppbytes_heap_clear(heap);
  ppbytes_buffer_init(heap);
}

ppbyte * ppbytes_flush (ppheap *heap, iof *O, size_t *psize)
{
  ppbyte *data;
  size_t size;
  
  //ASSERT(&heap->bytesheap == O->link);
  iof_put(O, '\0');
  data = (ppbyte *)O->buf;
  size = (size_t)iof_size(O);
  ppbytes_heap_done(heap, data, size);
  *psize = size - 1;
  return data;
}