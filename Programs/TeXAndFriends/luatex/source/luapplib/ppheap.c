
#include "pplib.h"

#define PPHEAP_BUFFER 0xFFFF
#define PPHEAP_WASTE 0x00FF

#define ppheap_head(heap) ((uint8_t *)((heap) + 1))


#if ARM_COMPLIANT
# define PPHEAP_ARCH_ARM
# define PPHEAP_NEED_ALIGNMENT
#endif

#if defined(__sun) && defined(__SVR4)
# define PPHEAP_NEED_ALIGNMENT
#endif
 
#ifdef PPHEAP_NEED_ALIGNMENT 
/* Tests has shown that long double seems to work: */
/* for 32bit aligned_data has  algn: 64 as ppxref and ppref, */
/* the other algns divide algn: 64, so it's ok.*/
/* Hopefully it's ok for aarch64 too */

/* These data are stored in ppheap.c.001t.tu */
/* made with gcc -fdump-tree-all-fdump-tree-all */


/* @2586   identifier_node  strg: ppxref   lngt: 6        */
/* @2565   record_type      name: @2586    size: @679     algn: 64       */
/*                          tag : struct   flds: @2587    */


/* @2672   identifier_node  strg: ppxsec   lngt: 6        */
/* @2648   type_decl        name: @2672    type: @2623    scpe: @221     */
/*                          srcp: ppxref.h:16             chain: @2673    */
/* @2623   record_type      name: @2648    unql: @2649    size: @593     */
/*                          algn: 32       tag : struct   flds: @2650 */


/* @2642   identifier_node  strg: ppstream lngt: 8 */
/* @2615   type_decl        name: @2642    type: @2643    scpe: @221     */
/*                          srcp: ppapi.h:54              chain: @2644    */
/* @2643   record_type      name: @2615    unql: @2614    size: @634     */
/*                          algn: 32       tag : struct   flds: @2641    */


/* @2765   identifier_node  strg: ppkids   lngt: 6 */
/* @2743   type_decl        name: @2765    type: @2766    scpe: @221     */
/*                          srcp: ppload.h:16             chain: @2767    */
/* @2766   record_type      name: @2743    unql: @2742    size: @19      */
/*                          algn: 32       tag : struct   flds: @2764    */


/* @2625   identifier_node  strg: ppdoc    lngt: 5        */
/* @2605   record_type      name: @2625    size: @2626    algn: 32       */
/*                          tag : struct   flds: @2627    */

/* @2526   identifier_node  strg: ppref    lngt: 5        */
/* @2513   record_type      name: @2526    size: @662     algn: 64       */
/*                          tag : struct   flds: @2527    */


/* @2595   identifier_node  strg: ppdict   lngt: 6 */
/* @2576   type_decl        name: @2595    type: @2596    scpe: @221     */
/*                          srcp: ppapi.h:45              chain: @2597    */
/* @2596   record_type      name: @2576    unql: @2575    size: @593     */
/*                          algn: 32       tag : struct   flds: @2594    */


/* @2769   identifier_node  strg: ppcrypt_status          lngt: 14 */
/* @2752   type_decl        name: @2769    type: @2770    scpe: @221     */
/*                          srcp: ppapi.h:295             chain: @2771 */
/* @2770   enumeral_type    name: @2752    unql: @2655    size: @5       */
/*                          algn: 32       prec: 32       sign: signed   */
/*                          min : @6       max : @7       csts: @2681    */


/* @2558   identifier_node  strg: pparray  lngt: 7 */
/* @2541   type_decl        name: @2558    type: @2559    scpe: @221     */
/*                          srcp: ppapi.h:39              chain: @2560    */
/* @2559   record_type      name: @2541    unql: @2540    size: @19      */
/*                          algn: 32       tag : struct   flds: @2557 */


/* @2817   identifier_node  strg: aligned_data */
/* @2801   type_decl        name: @2817    type: @2818    scpe: @221     */
/*                          srcp: ppheap.c:22             chain: @2819    */
/* @2818   real_type        name: @2801    unql: @99      size: @19      */
/*                          algn: 64       prec: 64       */


typedef long double aligned_data;


#define ALIGN_BUFF_BUCKET_SIZE  0x3000 /* heuristic value, found by running few tests */
#ifdef __SIZEOF_POINTER__ 
#define SIZE_OF_POINTER  __SIZEOF_POINTER__ 
#else
#define SIZE_OF_POINTER  (sizeof(void *))
#endif 

typedef struct _simplereg {
  size_t bucket_pos;
  size_t bucket_size;
  size_t heap_instance;
  aligned_data **align_data_set ;
} simplereg;

/* By default static vars are initialized to  NULL, but to be clear.. */
static simplereg *align_set = NULL ; 

static void align_init_set(void){
   size_t size ;

   if (align_set) {
     align_set->heap_instance++;
     return ;
   }

   align_set = malloc(sizeof(simplereg)); 
   if (!align_set) {
         fprintf(stderr,"! fatal error: unable to setup master register for  aligned pointers\n");
         exit(EXIT_FAILURE);
     }

   size = SIZE_OF_POINTER*ALIGN_BUFF_BUCKET_SIZE;
   align_set->align_data_set = malloc(size);
   if (!align_set->align_data_set) {
         fprintf(stderr,"! fatal error: unable to setup register for aligned pointers\n");
         exit(EXIT_FAILURE);
     }

   align_set->bucket_pos = 0;
   align_set->bucket_size = ALIGN_BUFF_BUCKET_SIZE;
   align_set->heap_instance = 1;
   memset(align_set->align_data_set, 0UL,size);
}

static void align_save_into_set(aligned_data *p){
   if (align_set->bucket_pos >= align_set->bucket_size) {
     size_t new_size;
     aligned_data **align_data_set_new; 

     if(!align_set->align_data_set){
         fprintf(stderr,"! fatal error: unable to save aligned pointer,corrupted set\n");
         exit(EXIT_FAILURE);
     }
     new_size = (ALIGN_BUFF_BUCKET_SIZE+align_set->bucket_size)*SIZE_OF_POINTER;
     align_data_set_new = malloc(new_size);
     if (!align_data_set_new) {
         fprintf(stderr,"! fatal error: unable to save aligned pointer\n");
         exit(EXIT_FAILURE);
     }
     memset(align_data_set_new,0,new_size);
     memcpy(align_data_set_new, align_set->align_data_set, align_set->bucket_size*SIZE_OF_POINTER);
     free(align_set->align_data_set);
     align_set->align_data_set = align_data_set_new ;
     align_set->bucket_size += ALIGN_BUFF_BUCKET_SIZE;
   }
   if (align_set->bucket_pos>align_set->bucket_size){
     fprintf(stderr,"! fatal error: unable to save aligned pointer, wrong position\n");
     exit(EXIT_FAILURE);
   }
   align_set->align_data_set[align_set->bucket_pos] = p ; 
   align_set->bucket_pos++;

}

static void align_free_set(void){
  /* We don't know what heap does with its data, so free here is not secure */

  if(align_set){
    if (align_set->heap_instance>1) {
      align_set->heap_instance--;
    } else if (align_set->heap_instance ==1) {
      if (align_set->align_data_set){
	size_t p;
	for(p=1;p<align_set->bucket_pos;p++){
	  if (align_set->align_data_set[p]) {
	    free(align_set->align_data_set[p]);
	  }
	}
	free(align_set->align_data_set);
	align_set->align_data_set = NULL;
      }
      align_set->heap_instance=0;
      free(align_set);
      align_set = NULL ;
    }
  }
}

#endif /* PPHEAP_NEED_ALIGNMENT */


static ppheap * ppheap_create (size_t size)
{
	ppheap *heap;
	heap = (ppheap *)pp_malloc(sizeof(ppheap) + size * sizeof(uint8_t));
	heap->size = size;
	heap->space = size;
	heap->data = ppheap_head(heap);
	heap->prev = NULL;
	return heap;
}

ppheap * ppheap_new (void)
{
#ifdef PPHEAP_NEED_ALIGNMENT
        align_init_set();
#endif  
        return ppheap_create(PPHEAP_BUFFER);
}

void ppheap_free (ppheap *heap)
{
  ppheap *prev;
  do
  {
    prev = heap->prev;
    pp_free(heap);
    heap = prev;
  } while (heap != NULL);
#ifdef PPHEAP_NEED_ALIGNMENT
    align_free_set();
#endif  

}

void ppheap_renew (ppheap *heap)
{ // free all but first
  ppheap *prev;
  if ((prev = heap->prev) != NULL)
  {
    heap->prev = NULL;
    ppheap_free(prev);
  }
  heap->size = heap->space;
  heap->data = ppheap_head(heap);
}

static ppheap * ppheap_insert_top (ppheap **pheap, size_t size)
{
  ppheap *heap;
  heap = ppheap_create(size);
  heap->prev = (*pheap);
  *pheap = heap;
  return heap;
}

static ppheap * ppheap_insert_sub (ppheap **pheap, size_t size)
{
  ppheap *heap;
  heap = ppheap_create(size);
  heap->prev = (*pheap)->prev;
  (*pheap)->prev = heap;
  return heap;
}

void * ppheap_take (ppheap **pheap, size_t size)
{
	ppheap *heap;
	uint8_t *data;
#ifdef PPHEAP_NEED_ALIGNMENT
	aligned_data *p_aligned_data;
#endif  
	heap = *pheap;
	if (size <= heap->size)
	  ;
	else if (heap->size <= PPHEAP_WASTE && size <= (PPHEAP_BUFFER >> 1))
	  heap = ppheap_insert_top(pheap, PPHEAP_BUFFER);
	else
        heap = ppheap_insert_sub(pheap, size);
 	data = heap->data;
 	heap->data += size;
 	heap->size -= size;
#ifdef PPHEAP_NEED_ALIGNMENT
	/* Todo: only if data%sizeof(aligned_data) != 0 */
	p_aligned_data = malloc(size);
	if (!p_aligned_data) {
         fprintf(stderr,"! fatal error: unable to setup aligned pointer for ppheap_take\n");
         exit(EXIT_FAILURE);
	}
	memcpy(p_aligned_data,data,size);
	align_save_into_set(p_aligned_data);
	return (void *)p_aligned_data;
#else
 	return (void *)data;
#endif  

}


/* iof buffer tied to a heap */

static ppheap * ppheap_resize  (ppheap **pheap, size_t size)
{
  ppheap *heap;
  heap = ppheap_create(size);
  heap->prev = (*pheap)->prev;
  memcpy(heap->data, (*pheap)->data, (*pheap)->space); // (*pheap)->size is irrelevant
  pp_free(*pheap);
  *pheap = heap;
  return heap;
}

static size_t ppheap_buffer_handler (iof *O, iof_mode mode)
{
  ppheap **pheap, *heap;
  size_t objectsize, buffersize, neededsize;
  uint8_t *copyfrom;
  switch (mode)
  {
    case IOFWRITE:
      /* apparently more space needed then assumed initsize */
      pheap = (ppheap **)O->link;
      heap = *pheap;
      objectsize = (size_t)(O->buf - heap->data);
      buffersize = (size_t)(O->pos - O->buf);
      neededsize = objectsize + (buffersize << 1);
      if (ppheap_head(heap) < heap->data)
      {
        if (heap->size <= PPHEAP_WASTE && neededsize <= (PPHEAP_BUFFER >> 1))
        {
          heap = ppheap_insert_top(pheap, PPHEAP_BUFFER);
          copyfrom = heap->prev->data;
        }
        else
        {
          heap = ppheap_insert_sub(pheap, neededsize);
          copyfrom = (*pheap)->data;
          O->link = (void *)(&(*pheap)->prev);
        }
        memcpy(heap->data, copyfrom, objectsize + buffersize);
      }
      else
      { /* the buffer was (re)initialized from a new empty heap and occupies its entire space */
        // ASSERT(ppheap_head(heap) == heap->data);
        heap = ppheap_resize(pheap, neededsize);
      }
      O->buf = heap->data + objectsize;
      O->pos = O->buf + buffersize;
      O->end = heap->data + heap->size;
      return (size_t)(O->end - O->pos);
    case IOFFLUSH:
      return 0;
    case IOFCLOSE:
      // O->buf = O->pos = O->end = NULL;
      // O->link = NULL;
      return 0;
    default:
      break;
  }
  return 0;
}

iof * ppheap_buffer (ppheap **pheap, size_t objectsize, size_t initsize)
{
  static iof ppheap_buffer_iof = IOF_WRITER_STRUCT(ppheap_buffer_handler, NULL, NULL, 0, 0);
  ppheap *heap;
  size_t size;
  size = objectsize + initsize;
  heap = *pheap;
  if (size <= heap->size)
    ;
  else if (heap->size <= PPHEAP_WASTE && size <= (PPHEAP_BUFFER >> 1))
  {
    heap = ppheap_create(PPHEAP_BUFFER);
    heap->prev = (*pheap);
    *pheap = heap;
  }
  else
  {
    heap = ppheap_create(size);
    heap->prev = (*pheap)->prev;
    (*pheap)->prev = heap;
    pheap = &(*pheap)->prev; // passed to ppheap_buffer_iof.link
  }
  ppheap_buffer_iof.buf = ppheap_buffer_iof.pos = heap->data + objectsize;
  ppheap_buffer_iof.end = heap->data + heap->size;
  ppheap_buffer_iof.link = pheap; // ASSERT(*pheap == heap)
  return &ppheap_buffer_iof;
}

/*
void * ppheap_buffer_data (iof *O, size_t *psize)
{
  ppheap *heap;
  heap = *((ppheap **)(O->link));
  *psize = ppheap_buffer_size(O, heap);
  return heap->data;
}
*/

void * ppheap_flush (iof *O, size_t *psize) // not from explicit ppheap ** !!!
{
  ppheap *heap;
  uint8_t *data;
  size_t size;
#ifdef PPHEAP_NEED_ALIGNMENT
  aligned_data *p_aligned_data;
#endif  
  heap = *((ppheap **)(O->link));
  *psize = ppheap_buffer_size(O, heap);
  size = *psize;
  data = heap->data;
/*  heap->data += *psize;
  heap->size -= *psize;
*/
  heap->data += size;
  heap->size -= size;
  // O->buf = O->pos = O->end = NULL;
  // O->link = NULL;
  // iof_close(O);
#ifdef PPHEAP_NEED_ALIGNMENT
  /* Todo: only if data%sizeof(aligned_data) != 0 */
  p_aligned_data = malloc(size);
  if (!p_aligned_data) {
    fprintf(stderr,"! fatal error: unable to setup aligned pointer for ppheap_flush\n");
    exit(EXIT_FAILURE);
  }
  memcpy(p_aligned_data,data,size);
  align_save_into_set(p_aligned_data);
  return (void *)p_aligned_data;
#else
  return data;

#endif  


}


