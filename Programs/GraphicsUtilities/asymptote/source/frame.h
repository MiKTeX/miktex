/*****
 * frame.h
 * Andy Hammerlindl 2002/07/22
 *
 * Describes the frame levels for the functions of the language.
 * Also creates accesses for the variable for automated loading
 * and saving of variables.
 *****/

#ifndef FRAME_H
#define FRAME_H

#include <cassert>

#include "access.h"

namespace trans {

class frame : public gc {
  frame *parent;
  size_t numFormals;
  Int numLocals;

  // With the SIMPLE_FRAME flag, the size of frames cannot be changed at
  // runtime.  This is an issue for runnable-at-a-time mode, where global
  // variables can be continually added.  To handle this, the frame for
  // global variables is an "indirect" frame.  It holds one variable, which is
  // a link to another frame.  When the subframe is too small, a larger
  // runtime array is allocated, and the link is changed.
  enum {DIRECT_FRAME, INDIRECT_FRAME} style;

#ifdef DEBUG_FRAME
  string name;
#endif

  frame(string name)
    : parent(new frame("<subframe of " + name + ">", 0, 0)),
      numFormals(0), numLocals(1), style(INDIRECT_FRAME)
#ifdef DEBUG_FRAME
    , name(name)
#endif
  {}

public:
  frame(string name, frame *parent, size_t numFormals)
    : parent(parent), numFormals(numFormals), numLocals(0),
      style(DIRECT_FRAME)
#ifdef DEBUG_FRAME
    , name(name)
#endif
  {}

  static frame *indirect_frame(string name) {
    return new frame(name);
  }

  frame *getParent() {
    return parent;
  }

  // Which variable stores the link to the parent frame.
  Int parentIndex() {
    return numFormals;
  }

  Int size() {
    if (style == DIRECT_FRAME)
      return (Int) (1+numFormals+numLocals);
    else
      return parent->size();
  }

  access *accessFormal(size_t index) {
    assert(index < numFormals);
    assert(style == DIRECT_FRAME);
    return new localAccess((Int) (index), this);
  }

  access *allocLocal() {
    if (style == DIRECT_FRAME)
      return new localAccess((Int) (1 + numFormals + numLocals++), this);
    else
      return parent->allocLocal();
  }
};

inline void print(ostream& out, frame *f) {
  out << f;
  if (f != 0) {
    out << " -> ";
    print(out, f->getParent());
  }
}

} // namespace trans

#endif

