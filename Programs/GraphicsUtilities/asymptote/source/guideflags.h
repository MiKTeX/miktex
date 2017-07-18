/*****
 * guideflags.h
 * Tom Prince 2004/5/12
 *
 * These flags are used to indicate what specifications of the join are
 * put on the stack.
 *****/
#ifndef GUIDEFLAGS_H
#define GUIDEFLAGS_H

namespace camp {

#undef OUT
#undef IN
  
enum side { OUT, IN, END, JOIN };
  
}

#endif //GUIDEFLAGS_H
