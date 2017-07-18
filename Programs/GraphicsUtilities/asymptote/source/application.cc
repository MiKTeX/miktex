/*****
 * application.cc
 * Andy Hammerlindl 2005/05/20
 *
 * An application is a matching of arguments in a call expression to formal
 * parameters of a function.  Since the language allows default arguments,
 * keyword arguments, rest arguments, and anything else we think of, this
 * is not a simple mapping.
 *****/

#include "application.h"
#include "exp.h"
#include "coenv.h"
#include "runtime.h"
#include "runarray.h"

using namespace types;
using absyntax::varinit;
using absyntax::arrayinit;
using absyntax::arglist;

namespace trans {

// Lower scores are better.  Packed is added onto the other qualifiers so
// we may score both exact and casted packed arguments.
const score FAIL=0, EXACT=1, CAST=2;
const score PACKED=2;

bool castable(env &e, formal& target, formal& source) {
  return target.Explicit ? equivalent(target.t,source.t)
    : e.castable(target.t,source.t, symbol::castsym);
}

score castScore(env &e, formal& target, formal& source) {
  return equivalent(target.t,source.t) ? EXACT :
    (!target.Explicit &&
     e.fastCastable(target.t,source.t)) ? CAST : FAIL;
}


void restArg::transMaker(coenv &e, Int size, bool rest) {
  // Push the number of cells and call the array maker.
  e.c.encode(inst::intpush, size);
  e.c.encode(inst::builtin, rest ? run::newAppendedArray :
             run::newInitializedArray);
}

void restArg::trans(coenv &e, temp_vector &temps)
{
  // Push the values on the stack.
  for (mem::list<arg *>::iterator p = inits.begin(); p != inits.end(); ++p)
    (*p)->trans(e, temps);

  if (rest)
    rest->trans(e, temps);
  
  transMaker(e, (Int)inits.size(), (bool)rest);
}

class maximizer {
  app_list l;

  // Tests if x is as good (or better) an application as y.
  bool asgood(application *x, application *y) {
    // Matches to open signatures are always worse than matches to normal
    // signatures.
    if (x->sig->isOpen)
      return y->sig->isOpen;
    else if (y->sig->isOpen)
      return true;

    assert (x->scores.size() == y->scores.size());

    // Test if each score in x is no higher than the corresponding score in
    // y.
    return std::equal(x->scores.begin(), x->scores.end(), y->scores.begin(),
                      std::less_equal<score>());
  }

  bool better(application *x, application *y) {
    return asgood(x,y) && !asgood(y,x);
  }

  // Add an application that has already been determined to be maximal.
  // Remove any other applications that are now not maximal because of its
  // addition.
  void addMaximal(application *x) {
    app_list::iterator y=l.begin();
    while (y!=l.end())
      if (better(x,*y))
        y=l.erase(y);
      else
        ++y;
    l.push_front(x);
  }
  
  // Tests if x is maximal.
  bool maximal(application *x) {
    for (app_list::iterator y=l.begin(); y!=l.end(); ++y)
      if (better(*y,x))
        return false;
    return true;
  }

public:
  maximizer() {}

  void add(application *x) {
    if (maximal(x))
      addMaximal(x);
  }

  app_list result() {
    return l;
  }
};

ty *restCellType(signature *sig) {
  formal& f=sig->getRest();
  if (f.t) {
    array *a=dynamic_cast<array *>(f.t);
    if (a)
      return a->celltype;
  }

  return 0;
}

void application::initRest() {
  formal& f=sig->getRest();
  if (f.t) {
    ty *ct = restCellType(sig);
    if (!ct)
      vm::error("formal rest argument must be an array");

    rf=formal(ct, symbol::nullsym, false, f.Explicit);
  }
  if (f.t || sig->isOpen) {
    rest=new restArg();
  }
}

//const Int REST=-1; 
const Int NOMATCH=-2;

Int application::find(symbol name) {
  formal_vector &f=sig->formals;
  for (size_t i=index; i<f.size(); ++i)
    if (f[i].name==name && args[i]==0)
      return (Int)i;
  return NOMATCH;
}

bool application::matchDefault() {
  if (index==args.size())
    return false;
  else {
    formal &target=getTarget();
    if (target.defval) {
      args[index]=new defaultArg(target.t);
      advanceIndex();
      return true;
    }
    else
      return false;
  }
}

bool application::matchArgumentToRest(env &e, formal &source,
                                      varinit *a, size_t evalIndex)
{
  if (rest) {
    score s=castScore(e, rf, source);
    if (s!=FAIL) {
      rest->add(seq.addArg(a, rf.t, evalIndex));
      scores.push_back(s+PACKED);
      return true;
    }
  }
  return false;
}

bool application::matchAtSpot(size_t spot, env &e, formal &source,
                              varinit *a, size_t evalIndex)
{
  formal &target=sig->getFormal(spot);
  score s=castScore(e, target, source);

  if (s == FAIL)
    return false;
  else if (sig->formalIsKeywordOnly(spot) && source.name == symbol::nullsym)
    return false;
  else {
    // The argument matches.
    args[spot]=seq.addArg(a, target.t, evalIndex);
    if (spot==index)
      advanceIndex();
    scores.push_back(s);
    return true;
  }
}

bool application::matchArgument(env &e, formal &source,
                                varinit *a, size_t evalIndex)
{
  assert(!source.name);

  if (index==args.size())
    // Try to pack into the rest array.
    return matchArgumentToRest(e, source, a, evalIndex);
  else
    // Match here, or failing that use a default and try to match at the next
    // spot.
    return matchAtSpot(index, e, source, a, evalIndex) ||
      (matchDefault() && matchArgument(e, source, a, evalIndex));
}

bool application::matchNamedArgument(env &e, formal &source,
                                     varinit *a, size_t evalIndex)
{
  assert(source.name);

  Int spot=find(source.name);
  return spot!=NOMATCH && matchAtSpot(spot, e, source, a, evalIndex);
}

bool application::complete() {
  if (index==args.size())
    return true;
  else if (matchDefault())
    return complete();
  else
    return false;
}

bool application::matchRest(env &e, formal &source, varinit *a,
                            size_t evalIndex) {
  // First make sure all non-rest arguments are matched (matching to defaults
  // if necessary).
  if (complete())
    // Match rest to rest.
    if (rest) {
      formal &target=sig->getRest();
      score s=castScore(e, target, source);
      if (s!=FAIL) {
        rest->addRest(seq.addArg(a, target.t, evalIndex));
        scores.push_back(s);
        return true;
      }
    }
  return false;
}
  
// When the argument should be evaluated, possibly adjusting for a rest
// argument which occurs before named arguments.
size_t adjustIndex(size_t i, size_t ri)
{
  return i < ri ? i : i+1;
}

bool application::matchSignature(env &e, types::signature *source,
                                 arglist &al) {
  formal_vector &f=source->formals;

#if 0
  cout << "num args: " << f.size() << endl;
  cout << "num keyword-only: " << sig->numKeywordOnly << endl;
#endif

  size_t ri = al.rest.val ? al.restPosition : f.size();

  // First, match all of the named (non-rest) arguments.
  for (size_t i=0; i<f.size(); ++i)
    if (f[i].name)
      if (!matchNamedArgument(e, f[i], al[i].val, adjustIndex(i,ri)))
        return false;

  // Then, the unnamed.
  for (size_t i=0; i<f.size(); ++i)
    if (!f[i].name)
      if (!matchArgument(e, f[i], al[i].val, adjustIndex(i,ri)))
        return false;

  // Then, the rest argument.
  if (source->hasRest())
    if (!matchRest(e, source->getRest(), al.rest.val, ri))
      return false;

  // Fill in any remaining arguments with their defaults.
  return complete();
}
       
bool application::matchOpen(env &e, signature *source, arglist &al) {
  assert(rest);

  // Pack all given parameters into the rest argument.
  formal_vector &f=source->formals;
  for (size_t i = 0; i < f.size(); ++i)
    if (al[i].name)
      // Named arguments are not handled by open signatures.
      return false;
    else
      rest->add(seq.addArg(al[i].val, f[i].t, i));

  if (source->hasRest())
    rest->addRest(new varinitArg(al.rest.val, source->getRest().t));

  return true;
}

application *application::match(env &e, function *t, signature *source,
                                arglist &al) {
  assert(t->kind==ty_function);
  application *app=new application(t);

  bool success = t->getSignature()->isOpen ?
                     app->matchOpen(e, source, al) :
                     app->matchSignature(e, source, al);

  //cout << "MATCH " << success << endl;

  return success ? app : 0;
}

void application::transArgs(coenv &e) {
  temp_vector temps;

  for(arg_vector::iterator a=args.begin(); a != args.end(); ++a)
    (*a)->trans(e,temps);

  if (rest)
    rest->trans(e,temps);
}

bool application::exact() {
  if (sig->isOpen)
    return false;
  for (score_vector::iterator p = scores.begin(); p != scores.end(); ++p)
    if (*p != EXACT)
      return false;
  return true;
}

bool application::halfExact() {
  if (sig->isOpen)
    return false;
  if (scores.size() != 2)
    return false;
  if (scores[0] == EXACT && scores[1] == CAST)
    return true;
  if (scores[0] == CAST && scores[1] == EXACT)
    return true;
  return false;
}

// True if any of the formals have names.
bool namedFormals(signature *sig)
{
  formal_vector& formals = sig->formals;
  size_t n = formals.size();
  for (size_t i = 0; i < n; ++i) {
    if (formals[i].name)
      return true;
  }
  return false;
}

// Tests if arguments in the source signature can be matched to the formals
// in the target signature with no casting or packing.
// This allows overloaded args, but not named args.
bool exactMightMatch(signature *target, signature *source)
{
  // Open signatures never exactly match.
  if (target->isOpen)
    return false;

#if 0
  assert(!namedFormals(source));
#endif

  formal_vector& formals = target->formals;
  formal_vector& args = source->formals;

  // Sizes of the two lists.
  size_t fn = formals.size(), an = args.size();

  // Indices for the two lists.
  size_t fi = 0, ai = 0;

  while (fi < fn && ai < an) {
    if (equivalent(formals[fi].t, args[ai].t)) {
      // Arguments match, move to the next.
      ++fi; ++ai;
    } else if (formals[fi].defval) {
      // Match formal to default value.
      ++fi;
    } else {
      // Failed to match formal.
      return false;
    }
  }

  assert(fi == fn || ai == an);

  // Packing array arguments into the rest formal is inexact.  Do not allow it
  // here.
  if (ai < an)
    return false;

  assert(ai == an);

  // Match any remaining formal to defaults.
  while (fi < fn)
    if (formals[fi].defval) {
      // Match formal to default value.
      ++fi;
    } else {
      // Failed to match formal.
      return false;
    }

  // Non-rest arguments have matched.
  assert(fi == fn && ai == an);

  // Try to match the rest argument if given.
  if (source->hasRest()) {
    if (!target->hasRest())
      return false;
    
    if (!equivalent(source->getRest().t, target->getRest().t))
      return false;
  }

  // All arguments have matched.
  return true;
}

// Tries to match applications without casting.  If an application matches
// here, we need not attempt to match others with the slower, more general
// techniques.
app_list exactMultimatch(env &e,
                         types::overloaded *o,
                         types::signature *source,
                         arglist &al)
{
  assert(source);

  app_list l;

  // This can't handle named arguments.
  if (namedFormals(source))
    return l; /* empty */

  for (ty_vector::iterator t=o->sub.begin(); t!=o->sub.end(); ++t)
  {
    if ((*t)->kind != ty_function)
      continue;

    function *ft = (function *)*t;

    // First we run a test to see if all arguments could be exactly matched.
    // If this returns false, no such match is possible.
    // If it returns true, an exact match may or may not be possible.
    if (!exactMightMatch(ft->getSignature(), source))
      continue;

    application *a=application::match(e, ft, source, al);

    // Consider calling
    //   void f(A a=new A, int y)
    // with
    //   f(3)
    // This matches exactly if there is no implicit cast from int to A.
    // Otherwise, it does not match.
    // Thus, there is no way to know if the
    // match truly is exact without looking at the environment.
    // In such a case, exactMightMatch() must return true, but there is no
    // exact match.  Such false positives are eliminated here.
    // 
    // Consider calling
    //   void f(int x, real y=0.0, int z=0)
    // with
    //   f(1,2)
    // exactMightMatch() will return true, matching 1 to x and 2 to z, but the
    // application::match will give an inexact match of 1 to x to 2 to y, due
    // to the cast from int to real.  Therefore, we must test for exactness
    // even after matching.
    if (a && a->exact())
      l.push_back(a);
  }

  //cout << "EXACTMATCH " << (!l.empty()) << endl;
  return l;
}

bool halfExactMightMatch(env &e,
                         signature *target, types::ty *t1, types::ty *t2)
{
  formal_vector& formals = target->formals;
  if (formals.size() < 2)
    return false;
  if (formals.size() > 2) {
    // We should probably abort the whole matching in this case.  For now,
    // return true and let the usual matching handle it.
    return true;
  }

  assert(formals[0].t);
  assert(formals[1].t);

  // These casting tests if successful will be repeated again by
  // application::match.  It would be nice to avoid this somehow, but the
  // additional complexity is probably not worth the minor speed improvement.
  if (equivalent(formals[0].t, t1))
     return e.fastCastable(formals[1].t, t2);
  else 
    return equivalent(formals[1].t, t2) && e.fastCastable(formals[0].t, t1);
}

// Most common after exact matches are cases such as
//   2 + 3.4   (int, real) --> (real, real)
// that is, binary operations where one of the operands matches exactly and the
// other does not.  This function searches for these so-called "half-exact"
// matches.  This should only be called after exactMultimatch has failed.
app_list halfExactMultimatch(env &e,
                             types::overloaded *o,
                             types::signature *source,
                             arglist &al)
{
  assert(source);

  app_list l;


  // Half exact is only in the case of two arguments.
  formal_vector& formals = source->formals;
  if (formals.size() != 2 || source->hasRest())
    return l; /* empty */

  // This can't handle named arguments.
  if (namedFormals(source))
    return l; /* empty */

  // Alias the two argument types.
  types::ty *t1 = formals[0].t;
  types::ty *t2 = formals[1].t;

  assert(t1); assert(t2);

  for (ty_vector::iterator t=o->sub.begin(); t!=o->sub.end(); ++t)
  {
    if ((*t)->kind != ty_function)
      continue;

    function *ft = (function *)*t;

#if 1
    if (!halfExactMightMatch(e, ft->getSignature(), t1, t2))
      continue;
#endif

    application *a=application::match(e, ft, source, al);

#if 1
    if (a && a->halfExact())
      l.push_back(a);
#endif
  }

  return l;
}

// Simple check if there are too many arguments to match the candidate
// function.
// A "tooFewArgs" variant was also implemented at some point, but did
// not give any speed-up.
bool tooManyArgs(types::signature *target, types::signature *source) {
  return source->getNumFormals() > target->getNumFormals() &&
         !target->hasRest();
}

// The full overloading resolution system, which handles casting of arguments,
// packing into rest arguments, named arguments, etc.
app_list inexactMultimatch(env &e,
                           types::overloaded *o,
                           types::signature *source,
                           arglist &al)
{
  assert(source);

  app_list l;


#define DEBUG_GETAPP 0
#if DEBUG_GETAPP
  //cout << "source: " << *source << endl;
  //cout << "ARGS: " << source->getNumFormals() << endl;
  bool perfect=false;
  bool exact=false;
  bool halfExact=false;
#endif

  for(ty_vector::iterator t=o->sub.begin(); t!=o->sub.end(); ++t) {
    if ((*t)->kind==ty_function) {
#if DEBUG_GETAPP
      function *ft = dynamic_cast<function *>(*t);
      signature *target = ft->getSignature();
      if (equivalent(target, source))
        perfect = true;
#endif

      // Check if there are two many arguments to match.
      if (tooManyArgs((*t)->getSignature(), source))
        continue;

      application *a=application::match(e, (function *)(*t), source, al);
      if (a)
        l.push_back(a);

#if DEBUG_GETAPP
      if (a && !namedFormals(source)) {
        assert(a->exact() == exactlyMatchable(ft->getSignature(), source));
        if (a->halfExact() && !namedFormals(source)) {
          assert(halfExactMightMatch(e, target, source->getFormal(0).t,
                                                source->getFormal(1).t));
        }
          
      }
      if (a && a->exact())
        exact = true;
      if (a && a->halfExact())
        halfExact = true;
#endif
    }
  }

#if DEBUG_GETAPP
  cout << (perfect     ? "PERFECT" :
           exact       ? "EXACT" :
           halfExact   ? "HALFEXACT" :
                         "IMPERFECT")
       << endl;
#endif

  if (l.size() > 1) {
    // Return the most specific candidates.
    maximizer m;
    for (app_list::iterator x=l.begin(); x!=l.end(); ++x) {
      assert(*x);
      m.add(*x);
    }
    return m.result();
  }
  else
    return l;
}

enum testExactType {
  TEST_EXACT,
  DONT_TEST_EXACT,
};

// Sanity check for multimatch optimizations.
void sameApplications(app_list a, app_list b, testExactType te) {
  assert(a.size() == b.size());

  if (te == TEST_EXACT) {
    for (app_list::iterator i = a.begin(); i != a.end(); ++i) {
      if (!(*i)->exact()) {
        cout << *(*i)->getType() << endl;
      }
      assert((*i)->exact());
    }
    for (app_list::iterator i = b.begin(); i != b.end(); ++i)
      assert((*i)->exact());
  }

  if (a.size() == 1)
    assert(equivalent(a.front()->getType(), b.front()->getType()));
}

app_list multimatch(env &e,
                    types::overloaded *o,
                    types::signature *source,
                    arglist &al)
{
  app_list a = exactMultimatch(e, o, source, al);
  if (!a.empty()) {
#if DEBUG_CACHE
    // Make sure that exactMultimatch and the fallback return the same
    // application(s).
    sameApplications(a, inexactMultimatch(e, o, source, al), TEST_EXACT);
#endif

    return a;
  }

  a = halfExactMultimatch(e, o, source, al);
  if (!a.empty()) {
#if DEBUG_CACHE
    sameApplications(a, inexactMultimatch(e, o, source, al), DONT_TEST_EXACT);
#endif

    return a;
  }

  // Slow but most general method.
  return inexactMultimatch(e, o, source, al);
}

} // namespace trans
