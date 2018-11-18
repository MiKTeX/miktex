
#include "pplib.h"

#define PPXREF_MAP_INIT 16 // number of xref sections

ppxref * ppxref_create (ppdoc *pdf, size_t initsize, size_t xrefoffset)
{
  ppxref *xref;

  if (initsize == 0) // unknown
    initsize = PPXREF_MAP_INIT;
  xref = (ppxref *)ppheap_take(&pdf->heap, sizeof(ppxref) + initsize * sizeof(ppxsec));
  xref->sects = (ppxsec *)(xref + 1);
  xref->size = 0;
  xref->space = initsize;
  xref->count = 0;
  xref->trailer.type = PPNONE;
  xref->trailer.dict = NULL;
  xref->prev = NULL;
  xref->pdf = pdf;
  xref->offset = xrefoffset;
  //xref->crypt = NULL;
  return xref;
}

ppxsec * ppxref_push_section (ppxref *xref, ppheap **pheap)
{
  ppxsec *sects;
  if (xref->size < xref->space)
    return &xref->sects[xref->size++];
  xref->space <<= 1;
  sects = xref->sects;
  xref->sects = (ppxsec *)ppheap_take(pheap, xref->space * sizeof(ppxsec)); // waste but rare
  memcpy(xref->sects, sects, xref->size * sizeof(ppxsec));
  return &xref->sects[xref->size++];
}

static void ppxref_sort_sects (ppxsec *left, ppxsec *right)
{
  ppxsec *l, *r, *m, t;
  ppuint first, last;
  l = left, r = right, m = l + ((r - l) / 2);
  first = m->first, last = m->last;
  do
  { // don't take first/last from pointer
    while (l->first < first) ++l;
    while (r->first > last) --r;
    if (l <= r)
    {
      t = *l;
      *l = *r;
      *r = t;
      ++l, --r;
    }
  } while (l <= r);
  if (l < right)
    ppxref_sort_sects(l, right);
  if (r > left)
    ppxref_sort_sects(left, r);
}

int ppxref_sort (ppxref *xref)
{
  if (xref->size == 0)
    return 0;
  ppxref_sort_sects(xref->sects, xref->sects + xref->size - 1);
  return 1;
}

ppref * ppxref_find_local (ppxref *xref, ppuint refnumber)
{
  ppxsec *left, *right, *mid;
  //if (xref->size == 0) // we don't allow that
  //  return NULL;
  left = xref->sects;
  right = xref->sects + xref->size - 1;
  do
  {
    mid = left + ((right - left) / 2);
    if (refnumber > mid->last)
      left = mid + 1;
    else if (refnumber < mid->first)
      right = mid - 1;
    else
      return &mid->refs[refnumber - mid->first];
  } while (left <= right);
  return NULL;
}

/*
PJ 20180910

So far we were resolving references in the context of the current xref:

- if a given object is found in this xref, than this is the object
- otherwise older xrefs are queried in order
- only in linearized documents older body may refer to object from newer xref

Hans sent a document where an incremental update (newer body) has only an updated page object
(plus /Metadata and /Info), but /Root (catalog) and /Pages dict refs are defined only in the older body.
If we resolve references using the approach so far, we actually drop the update; newer objects are parsed
and linked to the newest xref, but never linked to objects tree. Assuming we will never need to interpret
older versions, makes sense to assume, that the newest object version is always the correct version.

*/

#if 0

ppref * ppxref_find (ppxref *xref, ppuint refnumber)
{
  ppref *ref;
  ppxref *other;

  if ((ref = ppxref_find_local(xref, refnumber)) != NULL)
    return ref;
  if (xref->pdf->flags & PPDOC_LINEARIZED)
  {
    for (other = xref->pdf->xref; other != NULL; other = other->prev)
      if (other != xref && (ref = ppxref_find_local(other, refnumber)) != NULL)
        return ref;
  }
  else
  {
    for (other = xref->prev; other != NULL; other = other->prev)
      if ((ref = ppxref_find_local(other, refnumber)) != NULL)
        return ref;
    /* This shouldn't happen, but I've met documents that have no linearized dict,
       but their xrefs are prepared as for linearized; with "older" xrefs referring
       to "newer". */
    for (other = xref->pdf->xref; other != NULL && other != xref; other = other->prev)
      if ((ref = ppxref_find_local(other, refnumber)) != NULL)
        return ref;
  }
  return NULL;
}

#else

ppref * ppxref_find (ppxref *xref, ppuint refnumber)
{
  ppref *ref;
  ppxref *other;

  for (other = xref->pdf->xref; other != NULL; other = other->prev)
    if ((ref = ppxref_find_local(other, refnumber)) != NULL)
      return ref;
  return NULL;
}

#endif

ppdict * ppxref_trailer (ppxref *xref)
{
  switch (xref->trailer.type)
  {
    case PPDICT:
      return xref->trailer.dict;
    case PPSTREAM:
      return xref->trailer.stream->dict;
    default:
      break;
  }
  return NULL;
}

ppxref * ppdoc_xref (ppdoc *pdf)
{
	return pdf->xref;
}

ppxref * ppxref_prev (ppxref *xref)
{
	return xref->prev;
}

ppdict * ppxref_catalog (ppxref *xref)
{
	ppdict *trailer;
	return (trailer = ppxref_trailer(xref)) != NULL ? ppdict_rget_dict(trailer, "Root") : NULL;
}

ppdict * ppxref_info (ppxref *xref)
{
	ppdict *trailer;
	return (trailer = ppxref_trailer(xref)) != NULL ? ppdict_rget_dict(trailer, "Info") : NULL;
}

ppref * ppxref_pages (ppxref *xref)
{
  ppdict *dict;
  ppref *ref;

  if ((dict = ppxref_catalog(xref)) == NULL || (ref = ppdict_get_ref(dict, "Pages")) == NULL)
    return NULL;
  return ref->object.type == PPDICT ? ref : NULL;
}
