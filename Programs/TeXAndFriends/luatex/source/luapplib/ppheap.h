
#ifndef PP_HEAP_H
#define PP_HEAP_H

#include "utilmem.h"

#define pp_malloc util_malloc
#define pp_callic util_calloc
#define pprealloc util_realloc
#define pp_free util_free

typedef struct ppheap ppheap;
struct ppheap {
	size_t size;
	size_t space;
	uint8_t *data;
	ppheap *prev;
};

ppheap * ppheap_new (void);
void ppheap_free (ppheap *heap);
void ppheap_renew (ppheap *heap);
void * ppheap_take (ppheap **pheap, size_t size);
iof * ppheap_buffer (ppheap **pheap, size_t objectsize, size_t initsize);
#define ppheap_buffer_size(O, heap) (size_t)((O)->pos - (heap)->data) // == (size_t)(O->buf - heap->data) + (size_t)(O->pos - O->buf);
//void * ppheap_buffer_data (iof *O, size_t *psize);
void * ppheap_flush (iof *O, size_t *psize);

#endif