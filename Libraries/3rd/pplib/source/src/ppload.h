
#ifndef PP_LOAD_H
#define PP_LOAD_H

typedef struct {
  ppobj *buf;     // ppobjects buffer (allocated, not from our heap)
  ppobj *pos;     // current ppobj *
  size_t size;    // stack size
  size_t space;   // available space
  ppheap *heap;  // allocator (parent pdf->stack->heap or parent context)
} ppstack;

typedef struct {
  ppobj *current;
  ppobj *sentinel;
} ppkids;

#define PPPAGES_STACK_DEPTH 4

typedef struct {
  ppkids buffer[PPPAGES_STACK_DEPTH];
  ppkids *root;
  ppkids *parent;
  ppuint depth;
  ppuint space;
} pppages;

struct ppdoc {
  /* input */
  iof_file input;
  iof reader;
  uint8_t *buffer;
  size_t filesize;
  /* heap */
  ppheap heap;
  ppstack stack;
  /* output struct */
  ppxref *xref;
  pppages pages;
  ppcrypt *crypt;
  ppcrypt_status cryptstatus;
  int flags;
  char version[5];
};

#define PPDOC_LINEARIZED (1 << 0)

ppobj * ppdoc_load_entry (ppdoc *pdf, ppref *ref);
#define ppobj_preloaded(pdf, obj) ((obj)->type != PPREF ? (obj) : ((obj)->ref->object.type == PPNONE ? ppdoc_load_entry(pdf, (obj)->ref) : &(obj)->ref->object))

ppstring * ppstring_internal (const void *data, size_t size, ppheap *heap);

struct ppcontext {
  ppheap heap;
  ppstack stack;
};

#endif