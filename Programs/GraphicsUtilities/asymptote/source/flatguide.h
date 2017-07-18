/*****
 * flatguide.h
 * Andy Hammerlindl 2005/02/23
 *
 * The data structure that builds up a knotlist.  This is done by calling in
 * order the methods to set knots, specifiers, and tensions.
 * Used by the guide solving routines.
 *
 * NOTE: figure out how nullpath{}..a should be handled.
 *****/

#ifndef FLATGUIDE_H
#define FLATGUIDE_H

#include "knot.h"
#include "guideflags.h"

namespace camp {

class flatguide
{
  // A cached solution of the path.  When traversing through a tree of guides,
  // if a cycle tag is encountered, then the path is solved up to that point.
  // If the guide continues from there (which rarely occurs in practice), all of
  // the control points solved are added as control specifiers, and then solved
  // into a path again.  In the (usual) case that a cycle ends a path, the
  // cached path avoids this second pass.
  bool solved;
  
  // Used by reverse(guide) to indicate the presence of an unresolved
  // interior cycle.
  bool precycle;
  
  path p;

  cvector<knot> nodes;

  // Information before the first knot.  For a non-cyclic guide, this is
  // ignored.  For a cyclic guide, it may be useful, but I can't determine a
  // sensible way to use it yet.
  tension tout;
  spec *out;

  // Information for the next knot to come.
  tension tin;
  spec *in;

  static spec open;

  tension& tref(side s)
  {
    switch (s) {
      case OUT:
        return nodes.empty() ? tout : nodes.back().tout;
      case IN:
      default:
        return tin;
    }
  }

  // Returns a reference to a spec* so that it may be assigned.
  spec*& sref(side s)
  {
    switch (s) {
      case OUT:
        return nodes.empty() ? out : nodes.back().out;
      case IN:
      default:
        return in;
    }
  }

  void addPre(path& p, Int j);
  void addPoint(path& p, Int j);
  void addPost(path& p, Int j);

  void clearNodes() {
    nodes.clear();
    in=&open;
    tin=tension();
  }
  void clearPath() {
    p=path();
    solved=false;
  }

  void uncheckedAdd(path p, bool allowsolve=true);

  // Sets solved to false, indicating that the path has been updated since last
  // being solved.  Also, copies a solved path back in as knots and control
  // specifiers, as it will have to be solved again.
  void update() {
    if (solved) {
      solved=false;
      clearNodes();
      add(p);
      clearPath();
    }
  }
      
public:
  flatguide()
    : solved(true), precycle(false), p(), out(&open), in(&open) {}

  Int size() const {
    return (Int) nodes.size();
  }
  
  knot Nodes(Int i) const {
    return nodes[i];
  }
  
  void setTension(tension t, side s) {
    update();
    tref(s)=t;
  }
  void setSpec(spec *p, side s) {
    assert(p);
    update();
    spec *&ref=sref(s);
    // Control specifiers trump normal direction specifiers.
    if (!ref || !ref->controlled() || p->controlled())
      ref=p;
  }

  void add(pair z) {
    update();
    // Push the pair onto the vector as a knot, using the current in-specifier
    // and in-tension for the in side for the knot. Use default values for the
    // out side, as those will be set after the point is added.
    nodes.push_back(knot(z,in,&open,tin,tension()));

    // Reset the in-spec and in-tension to defaults;
    tin=tension();
    in=&open;
  }

  // Reverts to an empty state.
  void add(path p, bool allowsolve=true) {
    update();
    uncheckedAdd(p,allowsolve);
  }

  void clear() {
    clearNodes();
    clearPath();
  }

  void close() {
    if(!nodes.empty()) {
      nodes.front().in=in;
      nodes.front().tin=tin;
    }
  }
  
  void resolvecycle() {
    if(!nodes.empty())
      nodes.push_back(nodes.front());
  }
  
  void precyclic(bool b) {
    precycle=b;
  }
  
  bool precyclic() {
    return precycle;
  }
  
  // Once all information has been added, release the flat result.
  simpleknotlist list(bool cycles=false) {
    if(cycles && !nodes.empty()) close();
    return simpleknotlist(nodes,cycles);
  }

  // Yield a path from the guide as represented here.
  path solve(bool cycles=false) {
    if (solved)
      return p;
    else {
      simpleknotlist l=list(cycles);
      p=camp::solve(l);
      solved=true;
      return p;
    }
  }
};

} // namespace camp

#endif // FLATGUIDE_H
