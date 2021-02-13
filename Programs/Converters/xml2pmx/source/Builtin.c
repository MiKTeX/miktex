
#include "obx.h"
#include <ctype.h>
#include <math.h>
#include <string.h>

char *lib_version = 
  "Oxford Oberon-2 library version 3.2beta1 [build hg-f2fdbc915d42]";

value *NEW(value *bp) {
     word desc = bp[HEAD+0].a;
     int size = bp[HEAD+1].i+4;
     value *sp = bp;
     word p = gc_alloc(size, bp);
     value *q = ptrcast(value, p);
     q[0].a = desc;
     (*--sp).a = p+4;
     return sp;
}

/*
Layout for flexible arrays:

q:	desc

	element 0
	...
	element n-1

desc:	map			desc = q + align(n*elsize) + 4
	dim 1
        ...
	dim k

map:	-4
	desc-(q+4)
[	GC_REPEAT		if elmap != 0
	0
	n
	elsize
	GC_MAP
	elmap
	GC_END          ]		
	GC_END

size = align(n*elsize) + 4*k + 20 + (28 if elmap != 0)

Parameters NEWFLEX(elmap, elsize, k: INTEGER;
			dim_0, ..., dim_{k-1}: INTEGER): SYSTEM.PTR
*/

value *NEWFLEX(value *bp) {
     word elmap = bp[HEAD+0].a;
     int elsize = bp[HEAD+1].i;
     int k = bp[HEAD+2].i;
     value *dim = &bp[HEAD+3];		/* Array of bounds */
     value *sp = bp;

     int size, arsize, i, n;
     word p, desc, map;
     value *q, *pdesc, *pmap;

     /* Compute no. of elements */
     n = 1;
     for (i = 0; i < k; i++) n *= dim[i].i;
     if (n < 0) liberror("allocating negative size");
     arsize = align(n * elsize, 4); 
     if (n == 0) elmap = 0;

     /* Allocate the space */
     size = arsize + 4*k + 20;
     if (elmap != 0) size += 28;

     p = gc_alloc(size, bp);
     desc = p + arsize + 4;
     map = desc + 4*k + 4;

     q = ptrcast(value, p);
     pdesc = ptrcast(value, desc);
     pmap = ptrcast(value, map);
     q[0].a = desc;

     /* Fill in the descriptor */
     pdesc[DESC_MAP].a = map;
     for (i = 0; i < k; i++) pdesc[DESC_BOUND+i].i = dim[i].i;

     /* Fill in the map */
     pmap[0].i = -4;
     pmap[1].i = desc-(p+4);
     if (elmap == 0) 
          pmap[2].i = GC_END;
     else {
          pmap[2].i = GC_REPEAT;
          pmap[3].i = 0;
          pmap[4].i = n;
          pmap[5].i = elsize;
          pmap[6].i = GC_MAP;
          pmap[7].i = elmap;
          pmap[8].i = GC_END;
          pmap[9].i = GC_END;
     }

     (*--sp).a = p+4; 
     return sp;
}

value *COMPARE(value *bp) {
     uchar *s1 = pointer(bp[HEAD+0]), *s2 = pointer(bp[HEAD+2]);
     int i = 0, n = min(bp[HEAD+1].i, bp[HEAD+3].i);
     value *sp = bp;

     while (i < n && s1[i] != '\0' && s1[i] == s2[i]) i++;
     if (i >= n) liberror("string is not null-terminated");
     (*--sp).i = s1[i] - s2[i]; 
     return sp;
}

value *COPY(value *bp) {
     obcopy((char * ) pointer(bp[HEAD+2]), bp[HEAD+3].i,
            (char * ) pointer(bp[HEAD+0]), bp[HEAD+1].i, bp);
     return bp;                                           
}

value *FLEXASSIGN(value *bp) {
     int size = bp[HEAD+0].i;
     int dim = bp[HEAD+1].i;
     uchar *src = pointer(bp[HEAD+2]), *dst = pointer(bp[HEAD+dim+3]);
     value *sbound = &bp[HEAD+3], *dbound = &bp[HEAD+dim+4];
     int i;

     for (i = 0; i < dim; i++) {
	  int sb = sbound[i].i, db = dbound[i].i;
	  if (sb > db || (i > 0 && sb < db)) 
	       liberror("bound mismatch in open array assignment");
	  size *= sb;
     }

     memmove(dst, src, size);
     return bp;
}

value *sys_move(value *bp) {
     memmove(pointer(bp[HEAD+1]), pointer(bp[HEAD+0]), bp[HEAD+2].i);
     return bp;
}

value *sys_liberror(value *bp) {
     value *bp1 = valptr(bp[BP]);
     error_stop((char * ) pointer(bp[HEAD+0]), 0, 0, bp1, NULL);
     return bp;
}

value *EASSERT(value *bp) {
     int line = bp[HEAD+0].i;
     int arg = bp[HEAD+1].i;
     value *bp1 = valptr(bp[BP]);
     uchar *pc = pointer(bp[PC]);
     error_stop("assertion failed (%d)", arg, line, bp1, pc);
     return bp;
}
