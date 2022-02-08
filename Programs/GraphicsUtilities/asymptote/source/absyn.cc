/****
 * absyn.cc
 * Tom Prince 2004/05/12
 *
 * Utility functions for syntax trees.
 *****/

#include "absyn.h"
#include "coenv.h"

namespace absyntax {

void absyn::markPos(trans::coenv& e)
{
  e.c.markPos(getPos());
}

absyn::~absyn()
{}

void prettyindent(ostream &out, Int indent)
{
  for (Int i = 0; i < indent; i++) out << " ";
}
void prettyname(ostream &out, string name, Int indent, position pos) {
  pos.print(out);
  prettyindent(out,indent);
  out << name << "\n";
}

} // namespace absyntax
