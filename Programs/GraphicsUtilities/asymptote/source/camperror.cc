/*****
 * camperror.cc
 * 2003/02/25 Andy Hammerlindl
 *
 * Provides a way for the classes in camp to report errors in
 * computation elegantly.  After running a method on a camp object that
 * could encounter an error, the program should call camp::errors to see
 * if any errors were encountered.
 *****/

#include <cassert>
#include <sstream>

#include "camperror.h"
#include "vm.h"
#include "errormsg.h"

namespace camp {

// Used internally to report an error in an operation.
void reportError(const string& desc)
{
  em.runtime(vm::getPos());
  em << desc;
  em.sync();
  throw handled_error(); 
}

// Used internally to report a warning in an operation.
void reportWarning(const string& desc)
{
  em.warning(vm::getPos());
  em << desc;
  em.sync();
}

void reportFatal(const string& desc)
{
  em.fatal(vm::getPos());
  em << desc;
  em.sync();
  em.statusError();
  try {
    throw quit(); 
  } catch(handled_error) {
  }
}

void reportError(const ostringstream& desc)
{
  reportError(desc.str());
}
  
void reportWarning(const ostringstream& desc)
{
  reportWarning(desc.str());
}
  
void reportFatal(const ostringstream& desc)
{
  reportFatal(desc.str());
}
  
} // namespace camp
