
#ifndef UTIL_MEM_HEAP_H
#define UTIL_MEM_HEAP_H

#include "utilmemallh.h"

typedef struct pyre8 pyre8;
typedef struct pyre16 pyre16;
typedef struct pyre32 pyre32;
typedef struct pyre64 pyre64;

struct pyre8 {
  pyre8 *prev;
  uint8_t *data;
  uint8_t left;
  uint8_t chunks;
#ifdef BIT32
  uint8_t dummy[2]; // 10->12
#else
  uint8_t dummy[6]; // 18->24
#endif
};

struct pyre16 {
  pyre16 *prev;
  uint8_t *data;
  uint16_t left;
  uint16_t chunks;
#ifdef BIT32
  //uint8_t dummy[0]; // 12->12
#else
  uint8_t dummy[4]; // 20->24
#endif
};

struct pyre32 {
  pyre32 *prev;
  uint8_t *data;
  uint32_t left;
  uint32_t chunks;
#ifdef BIT32
  //uint8_t dummy[0]; // 16->16
#else
  //uint8_t dummy[0]; // 24->24
#endif
};

struct pyre64 {
  pyre64 *prev;
  uint8_t *data;
  uint64_t left;
  uint64_t chunks;
#ifdef BIT32
  //uint8_t dummy[0]; // 24->24
#else
  //uint8_t dummy[0]; // 32->32
#endif
};

/* heaps */

typedef struct heap8 heap8;
typedef struct heap16 heap16;
typedef struct heap32 heap32;
typedef struct heap64 heap64;

struct heap8 {
  pyre8 *head;
  uint8_t space;
  uint8_t large;
  uint8_t flags;
};

struct heap16 {
  pyre16 *head;
  uint16_t space;
  uint16_t large;
  uint8_t flags;
};

struct heap32 {
  pyre32 *head;
  uint32_t space;
  uint32_t large;
  uint8_t flags;
};

struct heap64 {
  pyre64 *head;
  uint64_t space;
  uint64_t large;
  uint8_t flags;
};

#define HEAP_ZERO (1 << 0)
#define HEAP_DEFAULTS 0

#define HEAP8_INIT(space, large, flags) { NULL, aligned_space8(space), large, flags }
#define HEAP16_INIT(space, large, flags) { NULL, aligned_space16(space), large, flags }
#define HEAP32_INIT(space, large, flags) { NULL, aligned_space32(space), large, flags }
#define HEAP64_INIT(space, large, flags) { NULL, aligned_space64(space), large, flags }

UTILAPI heap8 * heap8_init (heap8 *heap, uint8_t space, uint8_t large, uint8_t flags);
UTILAPI heap16 * heap16_init (heap16 *heap, uint16_t space, uint16_t large, uint8_t flags);
UTILAPI heap32 * heap32_init (heap32 *heap, uint32_t space, uint32_t large, uint8_t flags);
UTILAPI heap64 * heap64_init (heap64 *heap, uint64_t space, uint64_t large, uint8_t flags);

UTILAPI void heap8_head (heap8 *heap);
UTILAPI void heap16_head (heap16 *heap);
UTILAPI void heap32_head (heap32 *heap);
UTILAPI void heap64_head (heap64 *heap);

#define heap8_ensure_head(heap) ((void)((heap)->head != NULL || (heap8_head(heap), 0)))
#define heap16_ensure_head(heap) ((void)((heap)->head != NULL || (heap16_head(heap), 0)))
#define heap32_ensure_head(heap) ((void)((heap)->head != NULL || (heap32_head(heap), 0)))
#define heap64_ensure_head(heap) ((void)((heap)->head != NULL || (heap64_head(heap), 0)))

UTILAPI void heap8_free (heap8 *heap);
UTILAPI void heap16_free (heap16 *heap);
UTILAPI void heap32_free (heap32 *heap);
UTILAPI void heap64_free (heap64 *heap);

UTILAPI void heap8_clear (heap8 *heap);
UTILAPI void heap16_clear (heap16 *heap);
UTILAPI void heap32_clear (heap32 *heap);
UTILAPI void heap64_clear (heap64 *heap);

UTILAPI void * _heap8_take (heap8 *heap, size_t size);
UTILAPI void * _heap16_take (heap16 *heap, size_t size);
UTILAPI void * _heap32_take (heap32 *heap, size_t size);
UTILAPI void * _heap64_take (heap64 *heap, size_t size);

UTILAPI void * _heap8_take0 (heap8 *heap, size_t size);
UTILAPI void * _heap16_take0 (heap16 *heap, size_t size);
UTILAPI void * _heap32_take0 (heap32 *heap, size_t size);
UTILAPI void * _heap64_take0 (heap64 *heap, size_t size);

#define heap8_take(heap, size) (heap8_ensure_head(heap), _heap8_take(heap, size))
#define heap16_take(heap, size) (heap16_ensure_head(heap), _heap16_take(heap, size))
#define heap32_take(heap, size) (heap32_ensure_head(heap), _heap32_take(heap, size))
#define heap64_take(heap, size) (heap64_ensure_head(heap), _heap64_take(heap, size))

#define heap8_take0(heap, size) (heap8_ensure_head(heap), _heap8_take0(heap, size))
#define heap16_take0(heap, size) (heap16_ensure_head(heap), _heap16_take0(heap, size))
#define heap32_take0(heap, size) (heap32_ensure_head(heap), _heap32_take0(heap, size))
#define heap64_take0(heap, size) (heap64_ensure_head(heap), _heap64_take0(heap, size))

UTILAPI void heap8_pop (heap8 *heap, void *taken, size_t size);
UTILAPI void heap16_pop (heap16 *heap, void *taken, size_t size);
UTILAPI void heap32_pop (heap32 *heap, void *taken, size_t size);
UTILAPI void heap64_pop (heap64 *heap, void *taken, size_t size);

UTILAPI void * _heap8_some (heap8 *heap, size_t size, size_t *pspace);
UTILAPI void * _heap16_some (heap16 *heap, size_t size, size_t *pspace);
UTILAPI void * _heap32_some (heap32 *heap, size_t size, size_t *pspace);
UTILAPI void * _heap64_some (heap64 *heap, size_t size, size_t *pspace);

#define heap8_some(heap, size, pspace) (heap8_ensure_head(heap), _heap8_some(heap, size, pspace))
#define heap16_some(heap, size, pspace) (heap16_ensure_head(heap), _heap16_some(heap, size, pspace))
#define heap32_some(heap, size, pspace) (heap32_ensure_head(heap), _heap32_some(heap, size, pspace))
#define heap64_some(heap, size, pspace) (heap64_ensure_head(heap), _heap64_some(heap, size, pspace))

UTILAPI void * heap8_more (heap8 *heap, void *taken, size_t written, size_t size, size_t *pspace);
UTILAPI void * heap16_more (heap16 *heap, void *taken, size_t written, size_t size, size_t *pspace);
UTILAPI void * heap32_more (heap32 *heap, void *taken, size_t written, size_t size, size_t *pspace);
UTILAPI void * heap64_more (heap64 *heap, void *taken, size_t written, size_t size, size_t *pspace);

UTILAPI void heap8_done (heap8 *heap, void *taken, size_t written);
UTILAPI void heap16_done (heap16 *heap, void *taken, size_t written);
UTILAPI void heap32_done (heap32 *heap, void *taken, size_t written);
UTILAPI void heap64_done (heap64 *heap, void *taken, size_t written);

UTILAPI void heap8_giveup (heap8 *heap, void *taken);
UTILAPI void heap16_giveup (heap16 *heap, void *taken);
UTILAPI void heap32_giveup (heap32 *heap, void *taken);
UTILAPI void heap64_giveup (heap64 *heap, void *taken);

UTILAPI int heap8_empty (heap8 *heap);
UTILAPI int heap16_empty (heap16 *heap);
UTILAPI int heap32_empty (heap32 *heap);
UTILAPI int heap64_empty (heap64 *heap);

UTILAPI void heap8_stats (heap8 *heap, mem_info *info, int append);
UTILAPI void heap16_stats (heap16 *heap, mem_info *info, int append);
UTILAPI void heap32_stats (heap32 *heap, mem_info *info, int append);
UTILAPI void heap64_stats (heap64 *heap, mem_info *info, int append);

#endif