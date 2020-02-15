
#ifndef UTIL_MEM_ALLH_H
#define UTIL_MEM_ALLH_H

#include <stddef.h> // size_t
#include <stdint.h>

#include "utildecl.h"

typedef struct ghost8 ghost8;
typedef struct ghost16 ghost16;
typedef struct ghost32 ghost32;
typedef struct ghost64 ghost64;

/* info stub */

typedef struct {
  size_t blocks, singles;
  size_t chunks, unused;
  size_t used, singleused, left;
  size_t ghosts, blockghosts, singleghosts;
} mem_info;

#define MEM_INFO_INIT() = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

#endif