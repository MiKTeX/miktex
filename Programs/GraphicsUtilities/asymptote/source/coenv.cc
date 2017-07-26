/*****
 * coenv.cc
 * Andy Hammerlindl 2004/11/18
 *
 * Bundles the env and coder classes needed in translating the abstract syntax
 * tree.  It also also implements some functions that involve both the env and
 * coder, such as implicitCast().
 *****/

#include "coenv.h"

namespace trans {

// Prints out error messages for the cast methods.
static inline void castError(position pos, ty *target, ty *source)
{
  em.error(pos);
  em << "cannot convert \'" << *source
     << "\' to \'" << *target << "\'";
}

static inline bool accessCast(position pos, ty *target, ty *source,
                              access *a, coder& c)
{
  if (a) {
    a->encode(CALL, pos, c);
    return true;
  }
  else {
    castError(pos, target, source);
    return false;
  }
}

bool coenv::implicitCast(position pos, ty *target, ty *source)
{
  return accessCast(pos, target, source,
                    e.lookupCast(target, source, symbol::castsym), c);
}

bool coenv::explicitCast(position pos, ty *target, ty *source)
{
  return accessCast(pos, target, source,
                    e.lookupCast(target, source, symbol::ecastsym), c);
}

}
