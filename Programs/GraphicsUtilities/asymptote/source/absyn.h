/****
 * absyn.h
 * Andy Hammerlindl 2002/07/14
 *
 * Defines the basic types of abstract syntax objects using forward
 * class declarations.
 *****/

#ifndef ABSYN_H
#define ABSYN_H

#include "common.h"
#include "errormsg.h" // For position

// Forward declaration for markPos.
namespace trans {
class coenv;
}

namespace absyntax {

class absyn : public gc {
protected:
  const position pos;

  void markPos(trans::coenv& c);

public:
  absyn(position pos)
    : pos(pos) {}

  virtual ~absyn();

  position getPos() const
  {
    return pos;
  }

  virtual void prettyprint(ostream &out, Int indent) = 0;
private:  // Non-copyable
  void operator=(const absyn&);
  absyn(const absyn&);
};

void prettyindent(ostream &out, Int indent);
void prettyname(ostream &out, string name, Int indent);

class name;
class ty;
class varinit;
class exp;
class runnable;
class stm;
class dec;
class block;

typedef block file;

// This is the abstract syntax tree of a file, assigned to when running
// yyparse.
extern file *root;

}

#endif
