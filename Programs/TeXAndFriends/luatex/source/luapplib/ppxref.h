
#ifndef PP_XREF_H
#define PP_XREF_H

/*
What we call xref is actually "xref section" in PDF spec and what we call section is "xref subsection".
Our ppxref is a list of sections, sorted by xrefsection->first and xrefsection->last bounds. Every section
keeps a list of ppref *refs, enumerated from xrefsection->first to xrefsection->last. To find a reference
by number we make a binary search over sections bounds, then jump to the proper ppref *ref.
*/

typedef struct {
  ppuint first;     // first reference number in section
  ppuint last;      // last reference number in section
  ppref *refs;      // references list
} ppxsec;

struct ppxref {
  ppxsec *sects;    // subsections list
  size_t size;      // actual sections size
  size_t space;     // available sections space
  ppobj trailer;    // trailer dict or stream
  ppuint count;     // count of references in all sections
  ppxref *prev;     // previous xref
  ppdoc *pdf;       // parent pdf to access entries in linearized docs
  size_t offset;    // file offset of xref
  //ppcrypt *crypt;   // per xref encryption state? 
};

ppxref * ppxref_create (ppdoc *pdf, size_t initsize, size_t xrefoffset);
ppxsec * ppxref_push_section (ppxref *xref, ppheap **pheap);
int ppxref_sort (ppxref *xref);
ppref * ppxref_find_local (ppxref *xref, ppuint refnumber);

#endif