/*****
 * flatguide.cc
 * Andy Hammerlindl 2005/02/23
 *
 * The data structure that builds up a knotlist.  This is done by calling in
 * order the methods to set knots, specifiers, and tensions.
 * Used by the guide solving routines.
 *****/

#include "flatguide.h"

namespace camp {

void flatguide::addPre(path& p, Int j)
{
  setSpec(new controlSpec(p.precontrol(j),p.straight(j-1)),IN);
}
void flatguide::addPoint(path& p, Int j)
{
  add(p.point(j));
}
void flatguide::addPost(path& p, Int j)
{
  setSpec(new controlSpec(p.postcontrol(j),p.straight(j)),OUT);
}

void flatguide::uncheckedAdd(path p, bool allowsolve)
{
  Int n=p.length();
  if(n < 0) return;
  if(n == 0) {
    addPoint(p,0);
    return;
  }
  int nminus1=n-1;
  if(!allowsolve && p.cyclic()) addPre(p,0);
  for(Int i=0; i < nminus1;) {
    addPoint(p,i);
    addPost(p,i);
    ++i;
    addPre(p,i);
  }
  addPoint(p,nminus1);
  addPost(p,nminus1);
  if(allowsolve || !p.cyclic()) {
    addPre(p,n);
    addPoint(p,n);
  }
}

spec flatguide::open;

}
