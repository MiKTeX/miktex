/*****
 * application.h
 * Andy Hammerlindl 2005/05/20
 *
 * An application is a matching of arguments in a call expression to formal
 * parameters of a function.  Since the language allows default arguments,
 * keyword arguments, rest arguments, and anything else we think of, this
 * is not a simple mapping.
 *****/

#ifndef APPLICATION_H
#define APPLICATION_H

#include "common.h"
#include "types.h"
#include "coenv.h"
#include "exp.h"

// Defined in runtime.in:
namespace run {
void pushDefault(vm::stack *Stack);
}

using absyntax::arglist;
using absyntax::varinit;
using absyntax::arrayinit;
using absyntax::tempExp;

// This is mid-way between trans and absyntax.
namespace trans {

typedef Int score;

typedef mem::vector<score> score_vector;

// This is used during the translation of arguments to store temporary
// expressions for arguments that need to be translated for side-effects at a
// certain point but used later on.  The invariant maintained is that if the
// vector has n elements, then the side-effects for the first n arguments have
// been translated.  Null is pushed onto the vector to indicate that the
// expression was evaluated directly onto the stack, without the use of a
// temporary.
typedef mem::vector<tempExp *> temp_vector;

struct arg : public gc {
  types::ty *t;

  arg(types::ty *t)
    : t(t) {}
  virtual ~arg() {}

  virtual void trans(coenv &e, temp_vector &) = 0;
};

struct varinitArg : public arg {
  varinit *v;

  varinitArg(varinit *v, types::ty *t)
    : arg(t), v(v)  {}

  virtual void trans(coenv &e, temp_vector &) {
    // Open signatures can match overloaded variables, but there is no way to
    // translate the result, so report an error.
    if (t->kind == types::ty_overloaded) {
      em.error(v->getPos());
      em << "overloaded argument in function call";
    }
    else
      v->transToType(e, t);
  }
};


// Pushes a default argument token on the stack as a placeholder for the
// argument.
struct defaultArg : public arg {
  defaultArg(types::ty *t)
    : arg(t) {}

  virtual void trans(coenv &e, temp_vector &) {
    //e.c.encode(inst::builtin, run::pushDefault);
    e.c.encode(inst::push_default);
  }
};

// Handles translation of all the arguments matched to the rest formal.
// NOTE: This code duplicates a lot of arrayinit.
struct restArg : public gc {
  mem::list<arg *> inits;

  arg *rest;
public:
  restArg()
    : rest(0) {}

  virtual ~restArg()
  {}

  // Encodes the instructions to make an array from size elements on the stack.
  static void transMaker(coenv &e, Int size, bool rest);

  void trans(coenv &e, temp_vector &temps);

  void add(arg *init) {
    inits.push_back(init);
  }

  void addRest(arg *init) {
    rest=init;
  }
};

// This class generates sequenced args, args whose side-effects occur in order
// according to their index, regardless of the order they are called.  This is
// used to ensure left-to-right order of evaluation of keyword arguments, even
// if they are given out of the order specified in the declaration.
class sequencer {
  struct sequencedArg : public varinitArg {
    sequencer &parent;
    size_t i;
    sequencedArg(varinit *v, types::ty *t, sequencer &parent, size_t i)
      : varinitArg(v, t), parent(parent), i(i) {}

    void trans(coenv &e, temp_vector &temps) {
      parent.trans(e, i, temps);
    }
  };

  typedef mem::vector<sequencedArg *> sa_vector;
  sa_vector args;

  // Makes a temporary for the next argument in the sequence.
  void alias(coenv &e, temp_vector &temps) {
    size_t n=temps.size();
    assert(n < args.size());
    sequencedArg *sa=args[n];
    assert(sa);

    temps.push_back(new tempExp(e, sa->v, sa->t));
  }

  // Get in a state to translate the i-th argument, aliasing any arguments that
  // occur before it in the sequence.
  void advance(coenv &e, size_t i, temp_vector &temps) {
    while (temps.size() < i)
      alias(e,temps);
  }

  void trans(coenv &e, size_t i, temp_vector &temps) {
    if (i < temps.size()) {
      // Already translated, use the alias.
      assert(temps[i]);
      temps[i]->trans(e);
    }
    else {
      // Alias earlier args if necessary.
      advance(e, i, temps);

      // Translate using the base method.
      args[i]->varinitArg::trans(e,temps);

      // Push null to indicate the argument has been translated.
      temps.push_back(0);
    }
  }

public:
  arg *addArg(varinit *v, types::ty *t, size_t i) {
    if (args.size() <= i)
      args.resize(i+1);
    return args[i]=new sequencedArg(v, t, *this, i);
  }
};


class application : public gc {
  types::signature *sig;
  types::function *t;

  // Sequencer to ensure given arguments are evaluated in the proper order.
  // Use of this sequencer means that transArgs can only be called once.
  sequencer seq;

  typedef mem::vector<arg *> arg_vector;
  arg_vector args;
  restArg *rest;

  // Target formal to match with arguments to be packed into the rest array.
  types::formal rf;

  // During the matching of arguments to an application, this stores the index
  // of the first unmatched formal.
  size_t index;

  // To resolve which is the best application in case of multiple matches of
  // overloaded functions, a score is kept for every source argument matched,
  // and an application with higher-scoring matches is chosen.
  score_vector scores;

  void initRest();

  application(types::signature *sig)
    : sig(sig),
      t(0),
      args(sig->formals.size()),
      rest(0),
      rf(0),
      index(0)
  { assert(sig); initRest(); }

  application(types::function *t)
    : sig(t->getSignature()),
      t(t),
      args(sig->formals.size()),
      rest(0),
      rf(0),
      index(0)
  { assert(sig); initRest(); }

  types::formal &getTarget() {
    return sig->getFormal(index);
  }

  // Move the index forward one, then keep going until we're at an unmatched
  // argument.
  void advanceIndex() {
    do {
      ++index;
    } while (index < args.size() && args[index]!=0);
  }

  // Finds the first unmatched formal of the given name, returning the index.
  // The rest formal is not tested.  This function returns FAIL if no formals
  // match.
  Int find(symbol name);

  // Match the formal at index to its default argument (if it has one).
  bool matchDefault();

  // Match the argument to the formal indexed by spot.
  bool matchAtSpot(size_t spot, env &e, types::formal &source,
                   varinit *a, size_t evalIndex);

  // Match the argument to be packed into the rest array, if possible.
  bool matchArgumentToRest(env &e, types::formal& source,
                           varinit *a, size_t evalIndex);

  // Matches the argument to a formal in the target signature (possibly causing
  // other formals in the target to be matched to default values), and updates
  // the matchpoint accordingly.
  bool matchArgument(env &e, types::formal& source,
                     varinit *a, size_t evalIndex);

  // Match an argument bound to a name, as in f(index=7).
  bool matchNamedArgument(env &e, types::formal& source,
                          varinit *a, size_t evalIndex);

  // After all source formals have been matched, checks if the match is
  // complete (filling in final default values if necessary).
  bool complete();

  // Match a rest argument in the calling expression.
  bool matchRest(env &e, types::formal& f, varinit *a, size_t evalIndex);

  // Match the argument represented in signature to the target signature.  On
  // success, all of the arguments in args will be properly set up.
  bool matchSignature(env &e, types::signature *source, arglist &al);

  // Match a signature which is open, meaning that any sequence of arguments is
  // matched.
  bool matchOpen(env &e, signature *source, arglist &al);

  friend class maximizer;
public:
  // Attempt to build an application given the target signature and the source
  // signature (representing the given arguments).  Return 0 if they cannot be
  // matched.
  static application *match(env &e,
                            types::function *t,
                            types::signature *source,
                            arglist &al);

  // Translate the arguments so they appear in order on the stack in
  // preparation for a call.
  void transArgs(coenv &e);

  types::function *getType() {
    return t;
  }

  // This returns true in the special case that the arguments matched without
  // casting or packing into the rest formal.
  bool exact();

  // The next best thing (score-wise) to an exact match.  This returns true if
  // there are two arguments, one of which is cast and one is matched exactly
  // and neither are packed into the rest argument.
  bool halfExact();
};

typedef mem::list<application *> app_list;

// Given an overloaded list of types, determines which type to call.  If none
// are applicable, returns an empty vector, if there is ambiguity, several will
// be returned.
app_list multimatch(env &e,
                    types::overloaded *o,
                    types::signature *source,
                    arglist &al);

}  // namespace trans

#endif
