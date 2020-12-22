/* print stats; common for heap, stock and pool */

#include <stdio.h>

#include "utilmeminfo.h"

#define UINT(i) ((unsigned long)(i))

void show_mem_info (mem_info *info)
{
  size_t totalwaste, totalmem, averagechunk, singlemem;
  double ftotalwaste, fblockwaste, fghostwaste, ftailwaste, fsinglewaste;
  double funused, fsingles, fsinglemem, fsingleeff;

  totalwaste = info->ghosts + info->blockghosts + info->left;
  totalmem = info->used + totalwaste;

  ftotalwaste = totalmem > 0 ? totalwaste * 100.0 / totalmem : 0;
  fblockwaste = totalmem > 0 ? (info->blockghosts - info->singleghosts) * 100.0 / totalmem : 0;
  fsinglewaste = totalmem > 0 ? info->singleghosts * 100.0 / totalmem : 0;
  fghostwaste = totalmem > 0 ? info->ghosts * 100.0 / totalmem : 0;
  ftailwaste = totalmem > 0 ? info->left * 100.0 / totalmem : 0;

  averagechunk = info->chunks > 0 ? info->used / info->chunks : 0;
  funused = info->chunks > 0 ? info->unused * 100.0 / info->chunks : 0.0;

  fsingles = info->blocks > 0 ? info->singles * 100.0 / info->blocks : 0;
  fsinglemem = info->used > 0 ? info->singleused * 100.0 / info->used : 0;
  singlemem = info->singleused + info->singleghosts;
  fsingleeff = singlemem > 0 ? info->singleused * 100.0 / singlemem : 0;

  printf("total: %lu + %lu = %lu\n", UINT(info->used), UINT(totalwaste), UINT(totalmem));
  printf("chunks: %lu of average size %lu, unused %lu[%.2f%%]\n", UINT(info->chunks), UINT(averagechunk), UINT(info->unused), funused);
  printf("blocks: %lu, singles %lu[%.2f%%], %.2f%% of allocs, efficiency %.2f%%\n", 
    UINT(info->blocks), UINT(info->singles), fsingles, fsinglemem, fsingleeff);
  printf("waste: %lu[%0.2f%%], block ghosts %0.2f%%, single ghosts %.2f%%, chunk ghosts %0.2f%%, tails %0.2f%%\n\n",
    UINT(totalwaste), ftotalwaste, fblockwaste, fsinglewaste, fghostwaste, ftailwaste);
}
