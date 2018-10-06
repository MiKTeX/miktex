
#ifndef PP_LOAD_H
#define PP_LOAD_H

typedef struct {
  ppobj *buf;     // ppobjects buffer (allocated, not from our heap)
  ppobj *pos;     // current ppobj *
  size_t size;    // stack size
  size_t space;   // available space
  ppheap **pheap; // allocator (parent pdf->stack->pheap or own)
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
  char version[5];
  iof_file input;
  iof reader;
  uint8_t *buffer;
  size_t filesize;
  ppxref *xref;
  ppheap *heap;
  ppstack stack;
  pppages pages;
  int flags;
  ppcrypt *crypt;
  ppcrypt_status cryptstatus;
};

#define PPDOC_LINEARIZED (1 << 0)

ppobj * ppdoc_load_entry (ppdoc *pdf, ppref *ref);
#define ppobj_preloaded(pdf, obj) ((obj)->type != PPREF ? (obj) : ((obj)->ref->object.type == PPNONE ? ppdoc_load_entry(pdf, (obj)->ref) : &(obj)->ref->object))
ppstring ppstring_internal (const void *data, size_t size, ppheap **pheap);

struct ppcontext {
  ppheap *heap;
  ppstack stack;
};

#endif