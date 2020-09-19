
#include "utilmemheap.h"
#include "utilmemallc.h"

#define pyre_alloc8(heap,  space) ((pyre8 *)((heap->flags  & HEAP_ZERO) ? util_calloc(1, sizeof(pyre8)  + space * sizeof(uint8_t)) : util_malloc(sizeof(pyre8)  + space * sizeof(uint8_t))))
#define pyre_alloc16(heap, space) ((pyre16 *)((heap->flags & HEAP_ZERO) ? util_calloc(1, sizeof(pyre16) + space * sizeof(uint8_t)) : util_malloc(sizeof(pyre16) + space * sizeof(uint8_t))))
#define pyre_alloc32(heap, space) ((pyre32 *)((heap->flags & HEAP_ZERO) ? util_calloc(1, sizeof(pyre32) + space * sizeof(uint8_t)) : util_malloc(sizeof(pyre32) + space * sizeof(uint8_t))))
#define pyre_alloc64(heap, space) ((pyre64 *)((heap->flags & HEAP_ZERO) ? util_calloc(1, sizeof(pyre64) + space * sizeof(uint8_t)) : util_malloc(sizeof(pyre64) + space * sizeof(uint8_t))))

#define pyre_free(pyre) util_free(pyre)

/* block reset */

#define reset_heap_head8(heap, pyre, used) \
  ((used = block_used8(pyre)), (pyre->data -= used), ((heap->flags & HEAP_ZERO) ? (memset(pyre->data, 0, used), 0) : 0), (pyre->left += (uint8_t)used))
#define reset_heap_head16(heap, pyre, used) \
  ((used = block_used16(pyre)), (pyre->data -= used), ((heap->flags & HEAP_ZERO) ? (memset(pyre->data, 0, used), 0) : 0), (pyre->left += (uint16_t)used))
#define reset_heap_head32(heap, pyre, used) \
  ((used = block_used32(pyre)), (pyre->data -= used), ((heap->flags & HEAP_ZERO) ? (memset(pyre->data, 0, used), 0) : 0), (pyre->left += (uint32_t)used))
#define reset_heap_head64(heap, pyre, used) \
  ((used = block_used64(pyre)), (pyre->data -= used), ((heap->flags & HEAP_ZERO) ? (memset(pyre->data, 0, used), 0) : 0), (pyre->left += (uint64_t)used))

/* init heap */

heap8 * heap8_init (heap8 *heap, uint8_t space, uint8_t large, uint8_t flags) 
{ 
  align_space8(space);
  if (large > space) large = space;
  heap->head = NULL; 
  heap->space = space; 
  heap->large = large; 
  heap->flags = flags; 
  return heap; 
}

heap16 * heap16_init (heap16 *heap, uint16_t space, uint16_t large, uint8_t flags) 
{ 
  align_space16(space);
  if (large > space) large = space;
  heap->head = NULL; 
  heap->space = space; 
  heap->large = large; 
  heap->flags = flags; 
  return heap; 
}

heap32 * heap32_init (heap32 *heap, uint32_t space, uint32_t large, uint8_t flags) 
{ 
  align_space32(space);
  if (large > space) large = space;
  heap->head = NULL; 
  heap->space = space; 
  heap->large = large; 
  heap->flags = flags; 
  return heap; 
}

heap64 * heap64_init (heap64 *heap, uint64_t space, uint64_t large, uint8_t flags) 
{ 
  align_space64(space);
  if (large > space) large = space;
  heap->head = NULL; 
  heap->space = space; 
  heap->large = large; 
  heap->flags = flags; 
  return heap; 
}

/* free heap */

void heap8_free (heap8 *heap)
{
  pyre8 *pyre, *prev;
  pyre = heap->head;
  heap->head = NULL;
  while (pyre != NULL)
  {
    prev = pyre->prev;
    pyre_free(pyre);
    pyre = prev;
  }
}

void heap16_free (heap16 *heap)
{
  pyre16 *pyre, *prev;
  pyre = heap->head;
  heap->head = NULL;
  while (pyre != NULL)
  {
    prev = pyre->prev;
    pyre_free(pyre);
    pyre = prev;
  }
}

void heap32_free (heap32 *heap)
{
  pyre32 *pyre, *prev;
  pyre = heap->head;
  heap->head = NULL;
  while (pyre != NULL)
  {
    prev = pyre->prev;
    pyre_free(pyre);
    pyre = prev;
  }
}

void heap64_free (heap64 *heap)
{
  pyre64 *pyre, *prev;
  pyre = heap->head;
  heap->head = NULL;
  while (pyre != NULL)
  {
    prev = pyre->prev;
    pyre_free(pyre);
    pyre = prev;
  }
}

/* clear heap */

void heap8_clear (heap8 *heap)
{
  pyre8 *pyre, *prev;
  size_t used;
  if ((pyre = heap->head) == NULL)
    return;
  prev = pyre->prev;
  pyre->prev = NULL;
  reset_heap_head8(heap, pyre, used);
  for (; prev != NULL; prev = pyre)
  {
    pyre = prev->prev;
    pyre_free(prev);
  }
}

void heap16_clear (heap16 *heap)
{
  pyre16 *pyre, *prev;
  size_t used;
  if ((pyre = heap->head) == NULL)
    return;
  prev = pyre->prev;
  pyre->prev = NULL;
  reset_heap_head16(heap, pyre, used);
  for (; prev != NULL; prev = pyre)
  {
    pyre = prev->prev;
    pyre_free(prev);
  }
}

void heap32_clear (heap32 *heap)
{
  pyre32 *pyre, *prev;
  size_t used;
  if ((pyre = heap->head) == NULL)
    return;
  prev = pyre->prev;
  pyre->prev = NULL;
  reset_heap_head32(heap, pyre, used);
  for (; prev != NULL; prev = pyre)
  {
    pyre = prev->prev;
    pyre_free(prev);
  }
}

void heap64_clear (heap64 *heap)
{
  pyre64 *pyre, *prev;
  size_t used;
  if ((pyre = heap->head) == NULL)
    return;
  prev = pyre->prev;
  pyre->prev = NULL;
  reset_heap_head64(heap, pyre, used);
  for (; prev != NULL; prev = pyre)
  {
    pyre = prev->prev;
    pyre_free(prev);
  }
}

/* heap head */

void heap8_head (heap8 *heap)
{
  pyre8 *pyre;
  heap->head = pyre = pyre_alloc8(heap, heap->space);
  pyre->prev = NULL;
  pyre->data = block_edge8(pyre);
  pyre->left = block_left8(pyre, heap->space);
  pyre->chunks = 0;
}

void heap16_head (heap16 *heap)
{
  pyre16 *pyre;
  heap->head = pyre = pyre_alloc16(heap, heap->space);
  pyre->prev = NULL;
  pyre->data = block_edge16(pyre);
  pyre->left = block_left16(pyre, heap->space);
  pyre->chunks = 0;
}

void heap32_head (heap32 *heap)
{
  pyre32 *pyre;
  heap->head = pyre = pyre_alloc32(heap, heap->space);
  pyre->prev = NULL;
  pyre->data = block_edge32(pyre);
  pyre->left = block_left32(pyre, heap->space);
  pyre->chunks = 0;
}

void heap64_head (heap64 *heap)
{
  pyre64 *pyre;
  heap->head = pyre = pyre_alloc64(heap, heap->space);
  pyre->prev = NULL;
  pyre->data = block_edge64(pyre);
  pyre->left = block_left64(pyre, heap->space);
  pyre->chunks = 0;
}

/* next heap head */

static pyre8 * heap8_new (heap8 *heap)
{
  pyre8 *pyre;
  pyre = pyre_alloc8(heap, heap->space);
  pyre->prev = heap->head;
  heap->head = pyre;
  pyre->data = block_edge8(pyre);
  pyre->left = block_left8(pyre, heap->space);
  pyre->chunks = 0;
  return pyre;
}

static pyre16 * heap16_new (heap16 *heap)
{
  pyre16 *pyre;
  pyre = pyre_alloc16(heap, heap->space);
  pyre->prev = heap->head;
  heap->head = pyre;
  pyre->data = block_edge16(pyre);
  pyre->left = block_left16(pyre, heap->space);
  pyre->chunks = 0;
  return pyre;
}

static pyre32 * heap32_new (heap32 *heap)
{
  pyre32 *pyre;
  pyre = pyre_alloc32(heap, heap->space);
  pyre->prev = heap->head;
  heap->head = pyre;
  pyre->data = block_edge32(pyre);
  pyre->left = block_left32(pyre, heap->space);
  pyre->chunks = 0;
  return pyre;
}

static pyre64 * heap64_new (heap64 *heap)
{
  pyre64 *pyre;
  pyre = pyre_alloc64(heap, heap->space);
  pyre->prev = heap->head;
  heap->head = pyre;
  pyre->data = block_edge64(pyre);
  pyre->left = block_left64(pyre, heap->space);
  pyre->chunks = 0;
  return pyre;
}

/* next heap sole */

static pyre8 * heap8_sole (heap8 *heap, size_t size)
{
  pyre8 *pyre, *head, *prev;
  pyre = pyre_alloc8(heap, size);
  head = heap->head;
  prev = head->prev;
  pyre->prev = prev;
  head->prev = pyre;
  pyre->data = block_edge8(pyre);
  pyre->left = 0; // (uint8_t)size makes no sense, even with buffer api it will finally become 0
  return pyre;
}

static pyre16 * heap16_sole (heap16 *heap, size_t size)
{
  pyre16 *pyre, *head, *prev;
  pyre = pyre_alloc16(heap, size);
  head = heap->head;
  prev = head->prev;
  pyre->prev = prev;
  head->prev = pyre;
  pyre->data = block_edge16(pyre);
  pyre->left = 0;
  return pyre;
}

static pyre32 * heap32_sole (heap32 *heap, size_t size)
{
  pyre32 *pyre, *head, *prev;
  pyre = pyre_alloc32(heap, size);
  head = heap->head;
  prev = head->prev;
  pyre->prev = prev;
  head->prev = pyre;
  pyre->data = block_edge32(pyre);
  pyre->left = 0;
  return pyre;
}

static pyre64 * heap64_sole (heap64 *heap, size_t size)
{
  pyre64 *pyre, *head, *prev;
  pyre = pyre_alloc64(heap, size);
  head = heap->head;
  prev = head->prev;
  pyre->prev = prev;
  head->prev = pyre;
  pyre->data = block_edge64(pyre);
  pyre->left = 0;
  return pyre;
}

/* take from heap */

#define pyre_next8(d, pyre, size) (d = pyre->data, pyre->data += size, pyre->left -= (uint8_t)size, ++pyre->chunks)
#define pyre_next16(d, pyre, size) (d = pyre->data, pyre->data += size, pyre->left -= (uint16_t)size, ++pyre->chunks)
#define pyre_next32(d, pyre, size) (d = pyre->data, pyre->data += size, pyre->left -= (uint32_t)size, ++pyre->chunks)
#define pyre_next64(d, pyre, size) (d = pyre->data, pyre->data += size, pyre->left -= (uint64_t)size, ++pyre->chunks)

// for sole blocks, block->left is permanently 0, we can't store size_t there
#define pyre_last8(d, pyre, size)  (d = pyre->data, pyre->data += size, pyre->chunks = 1)
#define pyre_last16(d, pyre, size) (d = pyre->data, pyre->data += size, pyre->chunks = 1)
#define pyre_last32(d, pyre, size) (d = pyre->data, pyre->data += size, pyre->chunks = 1)
#define pyre_last64(d, pyre, size) (d = pyre->data, pyre->data += size, pyre->chunks = 1)

void * _heap8_take (heap8 *heap, size_t size)
{
  pyre8 *pyre;
  void *data;
  pyre = heap->head;
  align_size8(size);
  if (size <= pyre->left)
  {
    pyre_next8(data, pyre, size);
  }
  else if (take_new_block8(heap, pyre8, pyre, size))
  {
    pyre = heap8_new(heap);
    pyre_next8(data, pyre, size);
  }
  else
  {
    pyre = heap8_sole(heap, size);
    pyre_last8(data, pyre, size);
  }
  return data;
}

void * _heap16_take (heap16 *heap, size_t size)
{
  pyre16 *pyre;
  void *data;
  pyre = heap->head;
  align_size16(size);
  if (size <= pyre->left)
  {
    pyre_next16(data, pyre, size);
  }
  else if (take_new_block16(heap, pyre16, pyre, size))
  {
    pyre = heap16_new(heap);
    pyre_next16(data, pyre, size);
  }
  else
  {
    pyre = heap16_sole(heap, size);
    pyre_last16(data, pyre, size);
  }
  return data;
}

void * _heap32_take (heap32 *heap, size_t size)
{
  pyre32 *pyre;
  void *data;
  pyre = heap->head;
  align_size32(size);
  if (size <= pyre->left)
  {
    pyre_next32(data, pyre, size);
  }
  else if (take_new_block32(heap, pyre32, pyre, size))
  {
    pyre = heap32_new(heap);
    pyre_next32(data, pyre, size);
  }
  else
  {
    pyre = heap32_sole(heap, size);
    pyre_last32(data, pyre, size);
  }
  return data;
}

void * _heap64_take (heap64 *heap, size_t size)
{
  pyre64 *pyre;
  void *data;
  pyre = heap->head;
  align_size64(size);
  if (size <= pyre->left)
  {
    pyre_next64(data, pyre, size);
  }
  else if (take_new_block64(heap, pyre64, pyre, size))
  {
    pyre = heap64_new(heap);
    pyre_next64(data, pyre, size);
  }
  else
  {
    pyre = heap64_sole(heap, size);
    pyre_last64(data, pyre, size);
  }
  return data;
}

void * _heap8_take0 (heap8 *heap, size_t size)
{
  return memset(_heap8_take(heap, size), 0, size);
}

void * _heap16_take0 (heap16 *heap, size_t size)
{
  return memset(_heap16_take(heap, size), 0, size);
}

void * _heap32_take0 (heap32 *heap, size_t size)
{
  return memset(_heap32_take(heap, size), 0, size);
}

void * _heap64_take0 (heap64 *heap, size_t size)
{
  return memset(_heap64_take(heap, size), 0, size);
}

/* pop last heap chunk */

#define taken_from_head(taken, head) (byte_data(taken) == head->data)
#define taken_from_sole(taken, head, sole) ((sole = head->prev) != NULL && byte_data(taken) == sole->data)

#define taken_prev_head(taken, head, size) (byte_data(taken) == head->data - size)
#define taken_prev_sole(taken, head, sole, size) ((sole = head->prev) != NULL && byte_data(taken) == sole->data - size)

void heap8_pop (heap8 *heap, void *taken, size_t size)
{
  pyre8 *pyre, *head;
  head = heap->head;
  align_size8(size);
  if (taken_prev_head(taken, head, size))
  {
    
    head->data -= size;
    head->left += (uint8_t)size;
    --head->chunks;
  }
  else if (taken_prev_sole(taken, head, pyre, size))
  {
    head->prev = pyre->prev;
    pyre_free(pyre);
  }
  else
  {
    ASSERT8(0);
  }
}

void heap16_pop (heap16 *heap, void *taken, size_t size)
{
  pyre16 *pyre, *head;
  head = heap->head;
  align_size16(size);
  if (taken_prev_head(taken, head, size))
  {
    
    head->data -= size;
    head->left += (uint16_t)size;
    --head->chunks;
  }
  else if (taken_prev_sole(taken, head, pyre, size))
  {
    head->prev = pyre->prev;
    pyre_free(pyre);
  }
  else
  {
    ASSERT16(0);
  }
}

void heap32_pop (heap32 *heap, void *taken, size_t size)
{
  pyre32 *pyre, *head;
  head = heap->head;
  align_size32(size);
  if (taken_prev_head(taken, head, size))
  {
    
    head->data -= size;
    head->left += (uint32_t)size;
    --head->chunks;
  }
  else if (taken_prev_sole(taken, head, pyre, size))
  {
    head->prev = pyre->prev;
    pyre_free(pyre);
  }
  else
  {
    ASSERT32(0);
  }
}

void heap64_pop (heap64 *heap, void *taken, size_t size)
{
  pyre64 *pyre, *head;
  head = heap->head;
  align_size64(size);
  if (taken_prev_head(taken, head, size))
  {
    
    head->data -= size;
    head->left += (uint64_t)size;
    --head->chunks;
  }
  else if (taken_prev_sole(taken, head, pyre, size))
  {
    head->prev = pyre->prev;
    pyre_free(pyre);
  }
  else
  {
    ASSERT64(0);
  }
}

/* heap buffer */

void * _heap8_some (heap8 *heap, size_t size, size_t *pspace)
{
  pyre8 *pyre;
  pyre = heap->head;
  align_size8(size);
  if (size <= pyre->left)
  {
    *pspace = pyre->left;
  }
  else if (take_new_block8(heap, pyre8, pyre, size))
  {
    pyre = heap8_new(heap);
    *pspace = pyre->left;
  }
  else
  {
    pyre = heap8_sole(heap, size);
    *pspace = size;
  }
  return void_data(pyre->data);
}

void * _heap16_some (heap16 *heap, size_t size, size_t *pspace)
{
  pyre16 *pyre;
  pyre = heap->head;
  align_size16(size);
  if (size <= pyre->left)
  {
    *pspace = pyre->left;
  }
  else if (take_new_block16(heap, pyre16, pyre, size))
  {
    pyre = heap16_new(heap);
    *pspace = pyre->left;
  }
  else
  {
    pyre = heap16_sole(heap, size);
    *pspace = size;
  }
  return void_data(pyre->data);
}

void * _heap32_some (heap32 *heap, size_t size, size_t *pspace)
{
  pyre32 *pyre;
  pyre = heap->head;
  align_size32(size);
  if (size <= pyre->left)
  {
    *pspace = pyre->left;
  }
  else if (take_new_block32(heap, pyre32, pyre, size))
  {
    pyre = heap32_new(heap);
    *pspace = pyre->left;
  }
  else
  {
    pyre = heap32_sole(heap, size);
    *pspace = size;
  }
  return void_data(pyre->data);
}

void * _heap64_some (heap64 *heap, size_t size, size_t *pspace)
{
  pyre64 *pyre;
  pyre = heap->head;
  align_size64(size);
  if (size <= pyre->left)
  {
    *pspace = pyre->left;
  }
  else if (take_new_block64(heap, pyre64, pyre, size))
  {
    pyre = heap64_new(heap);
    *pspace = pyre->left;
  }
  else
  {
    pyre = heap64_sole(heap, size);
    *pspace = size;
  }
  return void_data(pyre->data);
}

void * heap8_more (heap8 *heap, void *taken, size_t written, size_t size, size_t *pspace)
{
  pyre8 *pyre, *prev;
  pyre = heap->head;
  align_size8(size);
  if (taken_from_head(taken, pyre))
  {
    if (size <= pyre->left)
    {
      *pspace = pyre->left;
    }
    else if (take_new_block8(heap, pyre8, pyre, size))
    {
      pyre = heap8_new(heap);
      memcpy(pyre->data, taken, written);
      *pspace = pyre->left;
    }
    else
    {
      pyre = heap8_sole(heap, size);
      memcpy(pyre->data, taken, written);
      *pspace = size;
    }
  }
  else if (taken_from_sole(taken, pyre, prev))
  {
    pyre = heap8_sole(heap, size);
    memcpy(pyre->data, taken, written);
    *pspace = size;
    pyre->prev = prev->prev;
    pyre_free(prev);
  }
  else
  {
    ASSERT8(0);
    *pspace = 0;
    return NULL;
  }
  return void_data(pyre->data);
}

void * heap16_more (heap16 *heap, void *taken, size_t written, size_t size, size_t *pspace)
{
  pyre16 *pyre, *prev;
  pyre = heap->head;
  align_size16(size);
  if (taken_from_head(taken, pyre))
  {
    if (size <= pyre->left)
    {
      *pspace = pyre->left;
    }
    else if (take_new_block16(heap, pyre16, pyre, size))
    {
      pyre = heap16_new(heap);
      memcpy(pyre->data, taken, written);
      *pspace = pyre->left;
    }
    else
    {
      pyre = heap16_sole(heap, size);
      memcpy(pyre->data, taken, written);
      *pspace = size;
    }
  }
  else if (taken_from_sole(taken, pyre, prev))
  {
    pyre = heap16_sole(heap, size);
    memcpy(pyre->data, taken, written);
    *pspace = size;
    pyre->prev = prev->prev;
    pyre_free(prev);
  }
  else
  {
    ASSERT16(0);
    *pspace = 0;
    return NULL;
  }
  return void_data(pyre->data);
}

void * heap32_more (heap32 *heap, void *taken, size_t written, size_t size, size_t *pspace)
{
  pyre32 *pyre, *prev;
  pyre = heap->head;
  align_size32(size);
  if (taken_from_head(taken, pyre))
  {
    if (size <= pyre->left)
    {
      *pspace = pyre->left;
    }
    else if (take_new_block32(heap, pyre32, pyre, size))
    {
      pyre = heap32_new(heap);
      memcpy(pyre->data, taken, written);
      *pspace = pyre->left;
    }
    else
    {
      pyre = heap32_sole(heap, size);
      memcpy(pyre->data, taken, written);
      *pspace = size;
    }
  }
  else if (taken_from_sole(taken, pyre, prev))
  {
    pyre = heap32_sole(heap, size);
    memcpy(pyre->data, taken, written);
    *pspace = size;
    pyre->prev = prev->prev;
    pyre_free(prev);
  }
  else
  {
    ASSERT32(0);
    *pspace = 0;
    return NULL;
  }
  return void_data(pyre->data);
}

void * heap64_more (heap64 *heap, void *taken, size_t written, size_t size, size_t *pspace)
{
  pyre64 *pyre, *prev;
  pyre = heap->head;
  align_size64(size);
  if (taken_from_head(taken, pyre))
  {
    if (size <= pyre->left)
    {
      *pspace = pyre->left;
    }
    else if (take_new_block64(heap, pyre64, pyre, size))
    {
      pyre = heap64_new(heap);
      memcpy(pyre->data, taken, written);
      *pspace = pyre->left;
    }
    else
    {
      pyre = heap64_sole(heap, size);
      memcpy(pyre->data, taken, written);
      *pspace = size;
    }
  }
  else if (taken_from_sole(taken, pyre, prev))
  {
    pyre = heap64_sole(heap, size);
    memcpy(pyre->data, taken, written);
    *pspace = size;
    pyre->prev = prev->prev;
    pyre_free(prev);
  }
  else
  {
    ASSERT64(0);
    *pspace = 0;
    return NULL;
  }
  return void_data(pyre->data);
}

void heap8_done (heap8 *heap, void *taken, size_t written)
{
  pyre8 *pyre;
  pyre = heap->head;
  align_size8(written);
  if (taken_from_head(taken, pyre))
  {
    pyre->data += written;
    pyre->left -= (uint8_t)written;
    ++pyre->chunks;
  }
  else if (taken_from_sole(taken, pyre, pyre))
  {
    pyre->data += written;
    pyre->chunks = 1;
  }
  else
  {
    ASSERT8(0);
  }
}

void heap16_done (heap16 *heap, void *taken, size_t written)
{
  pyre16 *pyre;
  pyre = heap->head;
  align_size16(written);
  if (taken_from_head(taken, pyre))
  {
    pyre->data += written;
    pyre->left -= (uint16_t)written;
    ++pyre->chunks;
  }
  else if (taken_from_sole(taken, pyre, pyre))
  {
    pyre->data += written;
    pyre->chunks = 1;
  }
  else
  {
    ASSERT16(0);  
  }
}

void heap32_done (heap32 *heap, void *taken, size_t written)
{
  pyre32 *pyre;
  pyre = heap->head;
  align_size32(written);
  if (taken_from_head(taken, pyre))
  {
    pyre->data += written;
    pyre->left -= (uint32_t)written;
    ++pyre->chunks;
  }
  else if (taken_from_sole(taken, pyre, pyre))
  {
    pyre->data += written;
    pyre->chunks = 1;
  }
  else
  {
    ASSERT32(0);  
  }
}

void heap64_done (heap64 *heap, void *taken, size_t written)
{
  pyre64 *pyre;
  pyre = heap->head;
  align_size64(written);
  if (taken_from_head(taken, pyre))
  {
    pyre->data += written;
    pyre->left -= (uint64_t)written;
    ++pyre->chunks;
  }
  else if (taken_from_sole(taken, pyre, pyre))
  {
    pyre->data += written;
    pyre->chunks = 1;
  }
  else
  {
    ASSERT64(0);
  }
}

/* giveup */

void heap8_giveup (heap8 *heap, void *taken)
{
  pyre8 *head, *pyre;
  head = heap->head;
  if (taken_from_sole(taken, head, pyre))
  {
    head->prev = pyre->prev;
    pyre_free(pyre);
  }
}

void heap16_giveup (heap16 *heap, void *taken)
{
  pyre16 *head, *pyre;
  head = heap->head;
  if (taken_from_sole(taken, head, pyre))
  {
    head->prev = pyre->prev;
    pyre_free(pyre);
  }
}

void heap32_giveup (heap32 *heap, void *taken)
{
  pyre32 *head, *pyre;
  head = heap->head;
  if (taken_from_sole(taken, head, pyre))
  {
    head->prev = pyre->prev;
    pyre_free(pyre);
  }
}

void heap64_giveup (heap64 *heap, void *taken)
{
  pyre64 *head, *pyre;
  head = heap->head;
  if (taken_from_sole(taken, head, pyre))
  {
    head->prev = pyre->prev;
    pyre_free(pyre);
  }
}

/* heap empty */

int heap8_empty (heap8 *heap)
{
  pyre8 *pyre;
  return head_block_empty(heap, pyre);
}

int heap16_empty (heap16 *heap)
{
  pyre16 *pyre;
  return head_block_empty(heap, pyre);
}

int heap32_empty (heap32 *heap)
{
  pyre32 *pyre;
  return head_block_empty(heap, pyre);
}

int heap64_empty (heap64 *heap)
{
  pyre64 *pyre;
  return head_block_empty(heap, pyre);
}

/* heap stats */

void heap8_stats (heap8 *heap, mem_info *info, int append)
{
  pyre8 *pyre;
  size_t used, chunks = 0, blocks = 0, singles = 0;
  if (!append)
    memset(info, 0, sizeof(mem_info));
  for (pyre = heap->head; pyre != NULL; pyre = pyre->prev)
  {
    ++blocks;
    chunks += pyre->chunks;
    used = block_used8(pyre);
    info->used += used;
    info->left += pyre->left;
    if (pyre->chunks == 1 && pyre->left == 0)
    {
      ++singles;
      info->singleused += used;
    }
  }
  info->chunks += chunks;
  info->blocks += blocks;
  info->blockghosts += blocks * sizeof(pyre8);
  info->singles += singles;
  info->singleghosts += singles * sizeof(pyre8);
}

void heap16_stats (heap16 *heap, mem_info *info, int append)
{
  pyre16 *pyre;
  size_t used, chunks = 0, blocks = 0, singles = 0;
  if (!append)
    memset(info, 0, sizeof(mem_info));
  for (pyre = heap->head; pyre != NULL; pyre = pyre->prev)
  {
    ++blocks;
    chunks += pyre->chunks;
    used = block_used16(pyre);
    info->used += used;
    info->left += pyre->left;
    if (pyre->chunks == 1 && pyre->left == 0)
    {
      ++singles;
      info->singleused += used;
    }
  }
  info->chunks += chunks;
  info->blocks += blocks;
  info->blockghosts += blocks * sizeof(pyre16);
  info->singles += singles;
  info->singleghosts += singles * sizeof(pyre16);
}

void heap32_stats (heap32 *heap, mem_info *info, int append)
{
  pyre32 *pyre;
  size_t used, chunks = 0, blocks = 0, singles = 0;
  if (!append)
    memset(info, 0, sizeof(mem_info));
  for (pyre = heap->head; pyre != NULL; pyre = pyre->prev)
  {
    ++blocks;
    chunks += pyre->chunks;
    used = block_used32(pyre);
    info->used += used;
    info->left += pyre->left;
    if (pyre->chunks == 1 && pyre->left == 0)
    {
      ++singles;
      info->singleused += used;
    }
  }
  info->chunks += chunks;
  info->blocks += blocks;
  info->blockghosts += blocks * sizeof(pyre32);
  info->singles += singles;
  info->singleghosts += singles * sizeof(pyre32);
}

void heap64_stats (heap64 *heap, mem_info *info, int append)
{
  pyre64 *pyre;
  size_t used, chunks = 0, blocks = 0, singles = 0;
  if (!append)
    memset(info, 0, sizeof(mem_info));
  for (pyre = heap->head; pyre != NULL; pyre = pyre->prev)
  {
    ++blocks;
    chunks += pyre->chunks;
    used = block_used64(pyre);
    info->used += used;
    info->left += pyre->left;
    if (pyre->chunks == 1 && pyre->left == 0)
    {
      ++singles;
      info->singleused += used;
    }
  }
  info->chunks += chunks;
  info->blocks += blocks;
  info->blockghosts += blocks * sizeof(pyre64);
  info->singles += singles;
  info->singleghosts += singles * sizeof(pyre64);
}
