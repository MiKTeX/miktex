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

#include <sstream>
#include <unistd.h>
#include <algorithm>

#include "genv.h"
#include "env.h"
#include "dec.h"
#include "stm.h"
#include "types.h"
#include "settings.h"
#include "runtime.h"
#include "parser.h"
#include "locate.h"
#include "interact.h"
#include "builtin.h"

using namespace types;
using settings::getSetting;
using settings::Setting;
using vm::importIndex_t;

// Dynamic loading of external libraries.
types::record *transExternalModule(
    trans::genv& ge, string filename, symbol id
);

namespace trans {

genv::genv()
  : imap()
{
  // Add settings as a module.  This is so that the init file ~/.asy/config.asy
  // can set settings.
  imap[importIndex_t("settings","")]=settings::getSettingsModule();

  // Translate plain in advance, if we're using autoplain.
  if(getSetting<bool>("autoplain")) {
    Setting("autoplain")=false;

    // Translate plain without autoplain.
    getModule(symbol::trans("plain"), "plain");

    Setting("autoplain")=true;
  }
#ifdef HAVE_LIBGSL
  imap[importIndex_t("gsl","")]=trans::getGSLModule();
#endif
}

bool endswith(string suffix, string str)
{
  return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

record *genv::loadModule(symbol id, string filename) {
  // Hackish way to load an external library.
#if 0
  if (endswith(".so", filename)) {
    return transExternalModule(*this, filename, id);
  }
#endif

  // Get the abstract syntax tree.
  absyntax::file *ast = parser::parseFile(filename,"Loading");

  inTranslation.push_front(filename);

  em.sync();

  record *r=ast->transAsFile(*this, id);

  inTranslation.remove(filename);

  return r;
}

record *genv::loadTemplatedModule(symbol id, string filename,
                                  mem::vector<absyntax::namedTyEntry*> *args)
{
  // Hackish way to load an external library.
#if 0
  if (endswith(".so", filename)) {
    return transExternalModule(*this, filename, id);
  }
#endif

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
    em.sync();
    throw handled_error();
  }
}

record *genv::getModule(symbol id, string filename) {
  checkRecursion(filename);

  importIndex_t Index(filename,"");
  record *r=imap[Index];
  if (r)
    return r;
  else {
    record *r=loadModule(id, filename);
    // Don't add an erroneous module to the dictionary in interactive mode, as
    // the user may try to load it again.
    if (!interact::interactive || !em.errors())
      imap[Index]=r;

    return r;
  }

}

record *genv::getTemplatedModule(symbol id, string filename, string sigHandle,
                                 mem::vector<absyntax::namedTyEntry*>* args)
{
  checkRecursion(filename);
  importIndex_t Index(filename,sigHandle);
  record *r=loadTemplatedModule(id, filename, args);

  // Don't add an erroneous module to the dictionary in interactive mode, as
  // the user may try to load it again.
  if (!interact::interactive || !em.errors()) {
    imap[Index]=r;
  }

  return r;

}


typedef vm::stack::importInitMap importInitMap;

importInitMap *genv::getInitMap()
{
  struct initMap : public importInitMap, public gc {
    genv &ge;
    initMap(genv &ge)
      : ge(ge) {}
    lambda *operator[](importIndex_t s) {
      record *r=ge.imap[s];
      return r ? r->getInit() : 0;
    }
  };

  return new initMap(*this);
}

} // namespace trans
