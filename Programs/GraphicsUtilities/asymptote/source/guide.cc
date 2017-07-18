/*****
 * guide.cc
 * Andy Hammerlindl 2005/02/23
 *
 *****/

#include "guide.h"

namespace camp {

multiguide::multiguide(guidevector& v)
{
    // This constructor tests if the first subguide is also a multiguide and,
    // if possible, uses the same base, extending it beyond what is used.
    multiguide *rg = v.empty() ? 0 : dynamic_cast<multiguide *>(v[0]);
    if (rg && rg->base->size() == rg->length) {
        base = rg->base;
        base->insert(base->end(), v.begin()+1, v.end());
    }
    else
        base = new guidevector(v);

    length = base->size();
}

void multiguide::flatten(flatguide& g, bool allowsolve)
{
  size_t n=length;
  if(n > 0) {
    for(size_t i=0; i+1 < n; ++i) {
      subguide(i)->flatten(g,allowsolve);
      if(!allowsolve && subguide(i)->cyclic()) {
        g.precyclic(true);
        g.resolvecycle();
      }
    }
    subguide(n-1)->flatten(g,allowsolve);
  }
}

void multiguide::print(ostream& out) const
{
  side lastLoc=JOIN;
  for(size_t i=0; i < length; ++i) {
    guide *g = subguide(i);
    side loc = g->printLocation();
    adjustLocation(out,lastLoc,loc);
    g->print(out);
    lastLoc=loc;
  }
}

} // namespace camp
