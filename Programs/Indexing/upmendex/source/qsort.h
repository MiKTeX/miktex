/*
 *
 *  This file is part of
 *	MakeIndex - A formatter and format independent index processor
 *
 *  This file is public domain software donated by
 *  Nelson Beebe (beebe@science.utah.edu).
 *
 */
#include <string.h>	/* for size_t */

typedef int (*qq_compar_fn_t)(const void *, const void *);

extern void qqsort (void *base, size_t n, size_t size, qq_compar_fn_t compar);

#define qsort qqsort
