/*****
 * genv.cc
 * Andy Hammerlindl 2002/08/29
 *
 * This is the global environment for the translation of programs.  In
 * actuality, it is basically a module manager.  When a module is
 * requested, it looks for the corresponding filename, and if found,
 * parses and translates the file, returning the resultant module.
 *
 * genv sets up the basic type bindings and function bindings for
 * builtin functions, casts and operators, and imports plain (if set),
 * but all other initialization is done by the local environment defined
 * in env.h.
 *****/

/* Implementation of templated modules:
 *
 *    Translating an access declaration:
 *
 *    access Map(Key=A, Value=B) as MapAB;
 *
 *    run encodeLevel for both A and B
 *    this should give the parent records for each struct
 *    encode pushing the *number* of parents on the stack (i.e., push a
 *        single int)
 *    encode pushing the string "Map/1234567" on the stack
 *    encode call to builtin loadTemplatedModule
 *    also save into MapAB (varinit)
 *
 *    build list of types (or tyEntry?)
 *
 *    also ensure names match
 *
 *    *****
 *
 *    At runtime, loadTemplatedModule pops the string
 *
 *    if the module is already loaded, it pops the levels
 *    and returns the already loaded module.
 *
 *    if the module is not loaded, it leaves the levels on the stack
 *    and calls the initializer for the templated module
 *
 *    it might be easiest to give the number of pushed params as an argument
 *    to loadTemplatedModule (ints and strings have no push/pop)
 *
 *    *****
 *
 *    Translating a templated module
 *
 *    we start translating a file with a list of (name, type) pairs
 *
 *    for each record type,
 *    build variables for each parent level
 *    and encode bytecode to pop the parents off the stack into these vars
 *
 *    build tyEntry for each templated type
 *    if its a record, then use the above variables as ent->v
 *
 *    from here,
 *    translate the file as a module as usual
 *
 *
 */

#include <sstream>

#include <algorithm>

#include "genv.h"
#include "env.h"
#include "dec.h"
#include "stm.h"
#include "types.h"
#include "settings.h"
#include "runtime.h"
#include "asyparser.h"
#include "locate.h"
#include "interact.h"
#include "builtin.h"

#if !defined(_WIN32)
#include <unistd.h>
#endif

using namespace types;
using settings::getSetting;
using settings::Setting;

namespace trans {

genv::genv()
  : imap()
{
  // Add settings as a module.  This is so that the init file ~/.asy/config.asy
  // can set settings.
  imap[symbol::literalTrans("settings")]=settings::getSettingsModule();

  // Translate plain in advance, if we're using autoplain.
  if(getSetting<bool>("autoplain")) {
    Setting("autoplain")=false;

    // Translate plain without autoplain.
    getModule(symbol::trans("plain"), "plain");

    Setting("autoplain")=true;
  }
#ifdef HAVE_LIBGSL
  imap[symbol::literalTrans("gsl")]=trans::getGSLModule();
#endif
}

bool endswith(string suffix, string str)
{
  return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

record *genv::loadModule(symbol id, string filename) {
  // Get the abstract syntax tree.
  absyntax::file *ast = parser::parseFile(filename,"Loading");

  inTranslation.push_front(filename);

  em.sync();

  record *r=ast->transAsFile(*this, id);

  inTranslation.remove(filename);

  return r;
}

record *genv::loadTemplatedModule(
      symbol id,
      string filename,
      mem::vector<absyntax::namedTy*> *args
) {

  // Get the abstract syntax tree.
  absyntax::file *ast = parser::parseFile(filename,"Loading");

  inTranslation.push_front(filename);

  em.sync();

  record *r=ast->transAsTemplatedFile(*this, id, args);

  inTranslation.remove(filename);

  return r;
}


void genv::checkRecursion(string filename) {
  if (find(inTranslation.begin(), inTranslation.end(), filename) !=
      inTranslation.end()) {
    em.sync();
    em << "error: recursive loading of module '" << filename << "'\n";
    em.sync(true);
    throw handled_error();
  }
}

record *genv::getModule(symbol id, string filename) {
  checkRecursion(filename);

  symbol index=symbol::literalTrans(filename);
  record *r=imap[index];
  if (r)
    return r;
  else {
    r=loadModule(id, filename);
    // Don't add an erroneous module to the dictionary in interactive mode, as
    // the user may try to load it again.
    if (!interact::interactive || !em.errors()) {
      imap[index]=r;
    }

    return r;
  }
}

record *genv::getTemplatedModule(
    string filename,
    mem::vector<absyntax::namedTy*>* args
) {
  checkRecursion(filename);

  types::signature* sig = new types::signature();
  stringstream buf;
  buf << filename << "/";
  for (auto arg : *args) {
    sig->add(formal(arg->t, arg->dest));
    buf << arg->dest << "/";
  }
  buf << sig->handle() << "/";
  symbol index=symbol::literalTrans(buf.str());

  record *r=imap[index];
  if (r)
    return r;
  else {
    r=loadTemplatedModule(index, filename, args);
    // Don't add an erroneous module to the dictionary in interactive mode, as
    // the user may try to load it again.
    if (!interact::interactive || !em.errors()) {
      imap[index]=r;
    }

    return r;
  }
}

record *genv::getLoadedModule(symbol id) {
  return imap[id];
}

typedef vm::stack::importInitMap importInitMap;

importInitMap *genv::getInitMap()
{
  struct initMap : public importInitMap, public gc {
    genv &ge;
    initMap(genv &ge)
      : ge(ge) {}
    lambda *operator[](string s) {
      record *r=ge.imap[symbol::literalTrans(s)];
      return r ? r->getInit() : 0;
    }
  };

  return new initMap(*this);
}

} // namespace trans
