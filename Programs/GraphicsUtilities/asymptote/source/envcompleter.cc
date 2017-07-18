/*****
 * envcompleter.cc
 * Andy Hammerlindl 2006/07/31
 *
 * Implements a text completion function for readline based on symbols in the
 * environment.
 *****/

#include <fstream>

#include "table.h"
#include "envcompleter.h"

namespace trans {

bool basicListLoaded=false;
envCompleter::symbol_list basicList;

static void loadBasicList() {
  assert(basicListLoaded==false);

#define ADD(word) basicList.push_back(symbol::literalTrans(#word))
#include "keywords.cc"
#undef ADD

  basicListLoaded=true;
}

void envCompleter::basicCompletions(symbol_list &l, string start) {
  if (!basicListLoaded)
    loadBasicList();

  for (symbol_list::iterator p = basicList.begin(); p != basicList.end(); ++p)
    if (prefix(start, *p))
      l.push_back(*p);
}

void envCompleter::makeList(const char *text) {
  l.clear();
  basicCompletions(l, text);
  e.completions(l, text);
  index=l.begin();
}

char *envCompleter::operator () (const char *text, int state) {
  if (state==0)
    makeList(text);

  if (index==l.end())
    return 0;
  else {
    symbol name=*index;
    ++index;
    return StrdupMalloc((string)name);
  }
}

} // namespace trans
