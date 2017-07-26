/*****
 * coenv.h
 * Andy Hammerlindl 2004/11/18
 *
 * Bundles the env and coder classes needed in translating the abstract syntax
 * tree.  It also also implements some functions that involve both the env and
 * coder, such as implicitCast().
 *****/

#ifndef COENV_H
#define COENV_H

#include "env.h"
#include "coder.h"

namespace trans {

class coenv {
public:
  coder &c;
  env &e;

  coenv(coder &c, env &e)
    : c(c), e(e) {}

  // This is used when an expression of type source needs to be an
  // expression of type target.
  // If it is allowed, the casting instructions (if any) will be added.
  // Otherwise, an appropriate error message will be printed.
  bool implicitCast(position pos, ty *target, ty *source);
  bool explicitCast(position pos, ty *target, ty *source);

  void add(protoenv &source, varEntry *qualifier) {
    e.add(source, qualifier, c);
  }
};

} // namespace trans

#endif
