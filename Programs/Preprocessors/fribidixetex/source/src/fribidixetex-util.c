#include <stdlib.h>
#include <stdio.h>
#include "fribidixetex-util.h"

void *utl_malloc(size_t size)
{
	void *ptr;
	ptr=malloc(size);
	if(!ptr) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	return ptr;
}
void *utl_realloc(void *inptr,size_t size)
{
	void *ptr;
	ptr=realloc(inptr,size);
	if(!ptr) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	return ptr;
}
void utl_free(void *ptr)
{
	free(ptr);
}
