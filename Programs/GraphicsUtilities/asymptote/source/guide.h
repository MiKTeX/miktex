/*****
 * guide.h
 * Andy Hammerlindl 2005/02/23
 *
 *****/

#ifndef GUIDE_H
#define GUIDE_H

#include <iostream>
#include "knot.h"
#include "flatguide.h"
#include "settings.h"

namespace camp {

// Abstract base class for guides.
class guide : public gc {
protected:
public:
  virtual ~guide() {}

  // Returns the path that the guide represents.
  virtual path solve() {
    return path();
  }

  // Add the information in the guide to the flatguide, so that it can be
  // solved via the knotlist solving routines.
  // Returns true if guide has an interior cycle token.
  virtual void flatten(flatguide&, bool allowsolve=true)=0;

  virtual bool cyclic() {return false;}

  virtual void print(ostream& out) const {
    out << "nullpath";
  }

  // Needed so that multiguide can know where to put in ".." symbols.
  virtual side printLocation() const {
    return END;
  }

};

inline ostream& operator<< (ostream& out, const guide& g)
{
  g.print(out);
  return out;
}

// Draws dots between two printings of guides, if their locations are such that
// the dots are necessary.
inline void adjustLocation(ostream& out, side l1, side l2)
{
  if (l1 == END)
    out << endl;
  if ((l1 == END || l1 == OUT) && (l2 == IN || l2 == END))
    out << "..";
}

// A guide representing a pair.
class pairguide : public guide {
  pair z;

public:
  void flatten(flatguide& g, bool=true) {
    g.add(z);
  }

  pairguide(pair z)
    : z(z) {}

  path solve() {
    return path(z);
  }

  void print(ostream& out) const {
    out << z;
  }

  side printLocation() const {
    return END;
  }
};


// A guide representing a path.
class pathguide : public guide {
  path p;

public:
  void flatten(flatguide& g, bool allowsolve=true) {
    g.add(p,allowsolve);
  }

  pathguide(path p)
    : p(p) {}

  path solve() {
    return p;
  }

  bool cyclic() {return p.cyclic();}

  void print(ostream& out) const {
    out << p;
  }

  side printLocation() const {
    return END;
  }
};

// Tension expressions are evaluated to this class before being cast to a guide,
// so that they can be cast to other types (such as guide3) instead.
class tensionSpecifier : public gc {
  double out,in;
  bool atleast;

public:
  tensionSpecifier(double val, bool atleast=false)
    : out(val), in(val), atleast(atleast) {}
  tensionSpecifier(double out, double in, bool atleast=false)
    : out(out), in(in), atleast(atleast) {}

  double getOut() const { return out; }
  double getIn() const { return in; }
  bool getAtleast() const { return atleast; }
};


// A guide giving tension information (as part of a join).
class tensionguide : public guide {
  tension tout,tin;

public:
  void flatten(flatguide& g, bool=true) {
    g.setTension(tin,IN);
    g.setTension(tout,OUT);
  }

  tensionguide(tensionSpecifier spec)
    : tout(spec.getOut(), spec.getAtleast()),
      tin(spec.getIn(), spec.getAtleast()) {}

  void print(ostream& out) const {
    out << (tout.atleast ? ".. tension atleast " : ".. tension ")
        << tout.val << " and " << tin.val << " ..";
  }

  side printLocation() const {
    return JOIN;
  }
};

// Similar to tensionSpecifier, curl expression are evaluated to this type
// before being cast to guides.
class curlSpecifier : public gc {
  double value;
  side s;

public:
  curlSpecifier(double value, side s)
    : value(value), s(s) {}

  double getValue() const { return value; }
  side getSide() const { return s; }
};

// A guide giving a specifier.
class specguide : public guide {
  spec *p;
  side s;

public:
  void flatten(flatguide& g, bool=true) {
    g.setSpec(p,s);
  }

  specguide(spec *p, side s)
    : p(p), s(s) {}

  specguide(curlSpecifier spec)
    : p(new curlSpec(spec.getValue())), s(spec.getSide()) {}

  void print(ostream& out) const {
    out << *p;
  }

  side printLocation() const {
    return s;
  }
};

// A guide for explicit control points between two knots.  This could be done
// with two specguides, instead, but this prints nicer, and is easier to encode.
class controlguide : public guide {
  pair zout, zin;

public:
  void flatten(flatguide& g, bool=true) {
    g.setSpec(new controlSpec(zout), OUT);
    g.setSpec(new controlSpec(zin), IN);
  }

  controlguide(pair zout,pair zin)
    : zout(zout),zin(zin) {}
  controlguide(pair z)
    : zout(z),zin(z) {}

  void print(ostream& out) const {
    out << ".. controls "
        << zout << " and " << zin << " ..";
  }

  side printLocation() const {
    return JOIN;
  }
};

// A guide that is a sequence of other guides.  This is used, for instance is
// joins, where we have the left and right guide, and possibly specifiers and
// tensions in between.
typedef mem::vector<guide *> guidevector;

// A multiguide represents a guide given by the first "length" items of
// the vector pointed to by "base".
// The constructor, if given another multiguide as a first argument,
// will try to avoid allocating a new "base" array.
class multiguide : public guide {
  guidevector *base;
  size_t length;

  guide *subguide(size_t i) const
  {
    assert(i < length);
    assert(length <= base->size());
    return (*base)[i];
  }

public:

  multiguide(guidevector& v);

  void flatten(flatguide&, bool=true);

  bool cyclic() {
    size_t n=length;
    if(n < 1) return false;
    return subguide(n-1)->cyclic();
  }

  path solve() {
    if (settings::verbose>3) {
      cerr << "solving guide:\n";
      print(cerr); cerr << "\n\n";
    }

    flatguide g;
    this->flatten(g);
    path p=g.solve(false);

    if (settings::verbose>3)
      cerr << "solved as:\n" << p << "\n\n";

    return p;
  }

  void print(ostream& out) const;

  side printLocation() const {
    int n = length;
    return subguide(n-1)->printLocation();
  }
};

struct cycleToken : public gc {};

// A guide representing the cycle token.
class cycletokguide : public guide {
public:
  void flatten(flatguide& g, bool allowsolve=true) {
    // If cycles occur in the midst of a guide, the guide up to that point
    // should be solved as a path.  Any subsequent guide will work with that
    // path locked in place.
    if(allowsolve)
      g.solve(true);
    else
      g.close();
  }

  bool cyclic() {return true;}

  path solve() {
    // Just a cycle on it's own makes an empty guide.
    return path();
  }

  void print(ostream& out) const {
    out << "cycle";
  }

  side printLocation() const {
    return END;
  }
};

} // namespace camp

GC_DECLARE_PTRFREE(camp::pairguide);
GC_DECLARE_PTRFREE(camp::tensionSpecifier);
GC_DECLARE_PTRFREE(camp::tensionguide);
GC_DECLARE_PTRFREE(camp::curlSpecifier);
GC_DECLARE_PTRFREE(camp::controlguide);
GC_DECLARE_PTRFREE(camp::cycleToken);
GC_DECLARE_PTRFREE(camp::cycletokguide);

#endif // GUIDE_H
