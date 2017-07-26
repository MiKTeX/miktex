#ifndef __lexical_h__
#define __lexical_h__ 1

#include <sstream>

#include "common.h"

namespace lexical {

class bad_cast {};

template <typename T> 
T cast(const string& s, bool tolerant=false) 
{
  istringstream is(s);
  T value;
  if(is && is >> value && ((is >> std::ws).eof() || tolerant)) return value;
  throw bad_cast();
} 

}

#endif
