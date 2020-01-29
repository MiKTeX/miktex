/*****
 * vm.h
 * Tom Prince 2005/06/17
 * 
 * Interface to the virtual machine.
 *****/

#ifndef VM_H
#define VM_H

#include "errormsg.h"

namespace vm {

struct lambda; class stack;
typedef void (*bltin)(stack *s);

#ifdef DEBUG_BLTIN
// This associates names to bltin functions, so that the output of 'asy -s'
// can print the names of the bltin functions that appear in the bytecode.
void registerBltin(bltin b, string s);
string lookupBltin(bltin b);

#define REGISTER_BLTIN(b, s) \
    registerBltin((b), (s))
#else
#define REGISTER_BLTIN(b, s)
#endif

void run(lambda *l);
position getPos();

void errornothrow(const char* message);
void error(const char* message);
void error(const ostringstream& message);

} // namespace vm

#endif // VM_H
