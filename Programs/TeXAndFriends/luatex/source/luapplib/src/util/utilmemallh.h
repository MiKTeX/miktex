
#ifndef UTIL_MEM_ALLH_H
#define UTIL_MEM_ALLH_H

#include <stddef.h> // size_t
#include <stdint.h>

#include "utildecl.h"

typedef struct ghost8 ghost8;
typedef struct ghost16 ghost16;
typedef struct ghost32 ghost32;
typedef struct ghost64 ghost64;

#define aligned_size8(size) (size)
#define aligned_size16(size) ((((size) + 1) >> 1) << 1)
#define aligned_size32(size) ((((size) + 3) >> 2) << 2)
#define aligned_size64(size) ((((size) + 7) >> 3) << 3)

#define aligned_space8(size) (size)
#define aligned_space16(size) (((size) & 1) ? ((size) < 0xFFFF ? ((size) + 1) : ((size) - 1)) : (size))
#define aligned_space32(size) (((size) & 3) ? ((size) < 0xFFFFFFFD ? ((size) - ((size) & 3) + 4) : (size) - ((size) & 3)) : (size))
#define aligned_space64(size) (((size) & 7) ? ((size) < 0xFFFFFFFFFFFFFFF8ULL ? ((size) - ((size) & 7) + 8) : (size) - ((size) & 7)) : (size))

/* info stub */

typedef struct {
  size_t blocks, singles;
  size_t chunks, unused;
  size_t used, singleused, left;
  size_t ghosts, blockghosts, singleghosts;
} mem_info;

#define MEM_INFO_INIT() = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

#endif