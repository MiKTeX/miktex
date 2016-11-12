#ifndef _UTL_H_
#define _UTL_H_
#include <stdlib.h>

void *utl_malloc(size_t size);
void *utl_realloc(void *ptr,size_t size);
void utl_free(void *ptr);

#endif
