/*****
 * envcompleter.h
 * Andy Hammerlindl 2006/07/31
 *
 * Implements a text completion function for readline based on symbols in the
 * environment.
 *****/

#ifndef ENVCOMPLETER_H
#define ENVCOMPLETER_H

#include "env.h"
#include "interact.h"

namespace trans {

class envCompleter : public interact::completer {
public:
  typedef protoenv::symbol_list symbol_list;

private:
  protoenv &e;
  symbol_list l;
  symbol_list::iterator index;

  // These are completions that don't come from the environment, such as
  // keywords.  They are read from the keywords file.
  static void basicCompletions(symbol_list &l, string start);

  void makeList(const char *text);

public:
  envCompleter(protoenv &e)
    : e(e), l(), index(l.end()) {}

  char *operator () (const char *text, int state);
};

} // namespace trans

#endif // ENVCOMPLETER_H
