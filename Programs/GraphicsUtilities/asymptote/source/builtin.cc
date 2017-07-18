/*****
 * builtin.cc
 * Tom Prince 2004/08/25
 *
 * Initialize builtins.
 *****/

#include <cmath>

#include "builtin.h"
#include "entry.h"

#include "runtime.h"
#include "runpicture.h"
#include "runlabel.h"
#include "runhistory.h"
#include "runarray.h"
#include "runfile.h"
#include "runsystem.h"
#include "runstring.h"
#include "runpair.h"
#include "runtriple.h"
#include "runpath.h"
#include "runpath3d.h"
#include "runmath.h"

#include "types.h"

#include "castop.h"
#include "mathop.h"
#include "arrayop.h"
#include "vm.h"

#include "coder.h"
#include "exp.h"
#include "refaccess.h"
#include "settings.h"

#include "opsymbols.h"

#ifndef NOSYM
#include "builtin.symbols.h"
#endif

namespace vm {
  // Defined in stack.cc
  extern vm::frame *make_dummyframe(string name);
}

using namespace types;
using namespace camp;
using namespace vm;  

namespace trans {
using camp::transform;
using camp::pair;
using vm::bltin;
using run::divide;
using run::less;
using run::greater;
using run::plus;
using run::minus;

using namespace run;  
  
void gen_runtime_venv(venv &ve);
void gen_runbacktrace_venv(venv &ve);
void gen_runpicture_venv(venv &ve);
void gen_runlabel_venv(venv &ve);
void gen_runhistory_venv(venv &ve);
void gen_runarray_venv(venv &ve);
void gen_runfile_venv(venv &ve);
void gen_runsystem_venv(venv &ve);
void gen_runstring_venv(venv &ve);
void gen_runpair_venv(venv &ve);
void gen_runtriple_venv(venv &ve);
void gen_runpath_venv(venv &ve);
void gen_runpath3d_venv(venv &ve);
void gen_runmath_venv(venv &ve);
void gen_rungsl_venv(venv &ve);

void addType(tenv &te, symbol name, ty *t)
{
  te.enter(name, new tyEntry(t,0,0,position()));
}

// The base environments for built-in types and functions
void base_tenv(tenv &te)
{
#define PRIMITIVE(name,Name,asyName) \
        addType(te, symbol::trans(#asyName), prim##Name());
#include "primitives.h"
#undef PRIMITIVE
}

const formal noformal(0);  

function *functionFromFormals(ty *result,
             formal f1=noformal, formal f2=noformal, formal f3=noformal,
             formal f4=noformal, formal f5=noformal, formal f6=noformal,
             formal f7=noformal, formal f8=noformal, formal f9=noformal,
             formal fA=noformal, formal fB=noformal, formal fC=noformal,
             formal fD=noformal, formal fE=noformal, formal fF=noformal,
             formal fG=noformal, formal fH=noformal, formal fI=noformal)
{
  function *fun = new function(result);

  if (f1.t) fun->add(f1);
  if (f2.t) fun->add(f2);
  if (f3.t) fun->add(f3);
  if (f4.t) fun->add(f4);
  if (f5.t) fun->add(f5);
  if (f6.t) fun->add(f6);
  if (f7.t) fun->add(f7);
  if (f8.t) fun->add(f8);
  if (f9.t) fun->add(f9);
  if (fA.t) fun->add(fA);
  if (fB.t) fun->add(fB);
  if (fC.t) fun->add(fC);
  if (fD.t) fun->add(fD);
  if (fE.t) fun->add(fE);
  if (fF.t) fun->add(fF);
  if (fG.t) fun->add(fG);
  if (fH.t) fun->add(fH);
  if (fI.t) fun->add(fI);

  return fun;
}

void addFunc(venv &ve, access *a, ty *result, symbol id,
             formal f1=noformal, formal f2=noformal, formal f3=noformal,
             formal f4=noformal, formal f5=noformal, formal f6=noformal,
             formal f7=noformal, formal f8=noformal, formal f9=noformal,
             formal fA=noformal, formal fB=noformal, formal fC=noformal,
             formal fD=noformal, formal fE=noformal, formal fF=noformal,
             formal fG=noformal, formal fH=noformal, formal fI=noformal)
{
  function *fun = functionFromFormals(result,f1,f2,f3,f4,f5,f6,f7,f8,f9,
      fA,fB,fC,fD,fE,fF,fG,fH,fI);

  // NOTE: If the function is a field, we should encode the defining record in
  // the entry
  varEntry *ent = new varEntry(fun, a, 0, position());
  
  ve.enter(id, ent);
}

// Add a function with one or more default arguments.
void addFunc(venv &ve, bltin f, ty *result, symbol name, 
             formal f1, formal f2, formal f3, formal f4, formal f5, formal f6,
             formal f7, formal f8, formal f9, formal fA, formal fB, formal fC,
             formal fD, formal fE, formal fF, formal fG, formal fH, formal fI)
{
#ifdef DEBUG_BLTIN
  // If the function is an operator, print out the whole signature with the
  // types, as operators are heavily overloaded.  min and max are also heavily
  // overloaded, so we check for them too.  Many builtin functions have so
  // many arguments that it is noise to print out their full signatures.
  string s = name;
  if (s.find("operator ", 0) == 0 || s == "min" || s == "max") 
  {
    function *fun = functionFromFormals(result,f1,f2,f3,f4,f5,f6,f7,f8,f9,
        fA,fB,fC,fD,fE,fF,fG,fH,fI);
    ostringstream out;
    fun->printVar(out, name);
    REGISTER_BLTIN(f, out.str());
  }
  else {
    REGISTER_BLTIN(f, name);
  }
#endif

  access *a = new bltinAccess(f);
  addFunc(ve,a,result,name,f1,f2,f3,f4,f5,f6,f7,f8,f9,
      fA,fB,fC,fD,fE,fF,fG,fH,fI);
}

void addOpenFunc(venv &ve, bltin f, ty *result, symbol name)
{
  function *fun = new function(result, signature::OPEN);

  REGISTER_BLTIN(f, name);
  access *a= new bltinAccess(f);

  varEntry *ent = new varEntry(fun, a, 0, position());
  
  ve.enter(name, ent);
}


// Add a rest function with zero or more default/explicit arguments.
void addRestFunc(venv &ve, bltin f, ty *result, symbol name, formal frest,
                 formal f1=noformal, formal f2=noformal, formal f3=noformal,
                 formal f4=noformal, formal f5=noformal, formal f6=noformal,
                 formal f7=noformal, formal f8=noformal, formal f9=noformal)
{
  REGISTER_BLTIN(f, name);
  access *a = new bltinAccess(f);
  function *fun = new function(result);

  if (f1.t) fun->add(f1);
  if (f2.t) fun->add(f2);
  if (f3.t) fun->add(f3);
  if (f4.t) fun->add(f4);
  if (f5.t) fun->add(f5);
  if (f6.t) fun->add(f6);
  if (f7.t) fun->add(f7);
  if (f8.t) fun->add(f8);
  if (f9.t) fun->add(f9);

  if (frest.t) fun->addRest(frest);

  varEntry *ent = new varEntry(fun, a, 0, position());

  ve.enter(name, ent);
}

void addRealFunc0(venv &ve, bltin fcn, symbol name)
{
  addFunc(ve, fcn, primReal(), name);
}

template<double (*fcn)(double)>
void addRealFunc(venv &ve, symbol name)
{
  addFunc(ve, realReal<fcn>, primReal(), name, formal(primReal(),SYM(x)));
  addFunc(ve, arrayFunc<double,double,fcn>, realArray(), name,
          formal(realArray(),SYM(a)));
}

#define addRealFunc(fcn, sym) addRealFunc<fcn>(ve, sym);
  
void addRealFunc2(venv &ve, bltin fcn, symbol name)
{
  addFunc(ve,fcn,primReal(),name,formal(primReal(),SYM(a)),
          formal(primReal(),SYM(b)));
}

template <double (*func)(double, int)>
void realRealInt(vm::stack *s) {
  Int n = pop<Int>(s);
  double x = pop<double>(s);
  s->push(func(x, intcast(n)));
}

template<double (*fcn)(double, int)>
void addRealIntFunc(venv& ve, symbol name, symbol arg1,
                    symbol arg2) {
  addFunc(ve, realRealInt<fcn>, primReal(), name, formal(primReal(), arg1),
          formal(primInt(), arg2));
}
  
void addInitializer(venv &ve, ty *t, access *a)
{
  addFunc(ve, a, t, symbol::initsym);
}

void addInitializer(venv &ve, ty *t, bltin f)
{
#ifdef DEBUG_BLTIN
  ostringstream s;
  s << "initializer for " << *t;
  REGISTER_BLTIN(f, s.str());
#endif
  access *a = new bltinAccess(f);
  addInitializer(ve, t, a);
}

// Specifies that source may be cast to target, but only if an explicit
// cast expression is used.
void addExplicitCast(venv &ve, ty *target, ty *source, access *a) {
  addFunc(ve, a, target, symbol::ecastsym, source);
}

// Specifies that source may be implicitly cast to target by the
// function or instruction stores at a.
void addCast(venv &ve, ty *target, ty *source, access *a) {
  //addExplicitCast(target,source,a);
  addFunc(ve, a, target, symbol::castsym, source);
}

void addExplicitCast(venv &ve, ty *target, ty *source, bltin f) {
#ifdef DEBUG_BLTIN
  ostringstream s;
  s << "explicit cast from " << *source << " to " << *target;
  REGISTER_BLTIN(f, s.str());
#endif
  addExplicitCast(ve, target, source, new bltinAccess(f));
}

void addCast(venv &ve, ty *target, ty *source, bltin f) {
#ifdef DEBUG_BLTIN
  ostringstream s;
  s << "cast from " << *source << " to " << *target;
  REGISTER_BLTIN(f, s.str());
#endif
  addCast(ve, target, source, new bltinAccess(f));
}

template<class T>
void addVariable(venv &ve, T *ref, ty *t, symbol name,
                 record *module=settings::getSettingsModule()) {
  access *a = new refAccess<T>(ref);
  varEntry *ent = new varEntry(t, a, PUBLIC, module, 0, position());
  ve.enter(name, ent);
}

template<class T>
void addVariable(venv &ve, T value, ty *t, symbol name,
                 record *module=settings::getSettingsModule(),
                 permission perm=PUBLIC) {
  item* ref=new item;
  *ref=value;
  access *a = new itemRefAccess(ref);
  varEntry *ent = new varEntry(t, a, perm, module, 0, position());
  ve.enter(name, ent);
}

template<class T>
void addConstant(venv &ve, T value, ty *t, symbol name,
                 record *module=settings::getSettingsModule()) {
  addVariable(ve,value,t,name,module,RESTRICTED);
}

// The identity access, i.e. no instructions are encoded for a cast or
// operation, and no functions are called.
identAccess id;

function *IntRealFunction()
{
  return new function(primInt(),primReal());
}

function *realPairFunction()
{
  return new function(primReal(),primPair());
}

function *voidFileFunction()
{
  return new function(primVoid(),primFile());
}

void addInitializers(venv &ve)
{
  addInitializer(ve, primBoolean(), boolFalse);
  addInitializer(ve, primInt(), IntZero);
  addInitializer(ve, primReal(), realZero);

  addInitializer(ve, primString(), emptyString);
  addInitializer(ve, primPair(), pairZero);
  addInitializer(ve, primTriple(), tripleZero);
  addInitializer(ve, primTransform(), transformIdentity);
  addInitializer(ve, primGuide(), nullGuide);
  addInitializer(ve, primPath(), nullPath);
  addInitializer(ve, primPath3(), nullPath3);
  addInitializer(ve, primPen(), newPen);
  addInitializer(ve, primPicture(), newPicture);
  addInitializer(ve, primFile(), nullFile);
}

void addCasts(venv &ve)
{
  addExplicitCast(ve, primString(), primInt(), stringCast<Int>);
  addExplicitCast(ve, primString(), primReal(), stringCast<double>);
  addExplicitCast(ve, primString(), primPair(), stringCast<pair>);
  addExplicitCast(ve, primString(), primTriple(), stringCast<triple>);
  addExplicitCast(ve, primInt(), primString(), castString<Int>);
  addExplicitCast(ve, primReal(), primString(), castString<double>);
  addExplicitCast(ve, primPair(), primString(), castString<pair>);
  addExplicitCast(ve, primTriple(), primString(), castString<triple>);

  addExplicitCast(ve, primInt(), primReal(), castDoubleInt);

  addCast(ve, primReal(), primInt(), cast<Int,double>);
  addCast(ve, primPair(), primInt(), cast<Int,pair>);
  addCast(ve, primPair(), primReal(), cast<double,pair>);
  
  addCast(ve, primPath(), primPair(), cast<pair,path>);
  addCast(ve, primGuide(), primPair(), pairToGuide);
  addCast(ve, primGuide(), primPath(), pathToGuide);
  addCast(ve, primPath(), primGuide(), guideToPath);

  addCast(ve, primFile(), primNull(), nullFile);
  
  // Vectorized casts.
  addExplicitCast(ve, IntArray(), realArray(), arrayToArray<double,Int>);
  
  addCast(ve, realArray(), IntArray(), arrayToArray<Int,double>);
  addCast(ve, pairArray(), IntArray(), arrayToArray<Int,pair>);
  addCast(ve, pairArray(), realArray(), arrayToArray<double,pair>);
  
  addCast(ve, realArray2(), IntArray2(), array2ToArray2<Int,double>);
  addCast(ve, pairArray2(), IntArray2(), array2ToArray2<Int,pair>);
  addCast(ve, pairArray2(), realArray2(), array2ToArray2<double,pair>);
}

void addTupleOperators(venv &ve)
{
  addFunc(ve, realRealToPair, primPair(), SYM_TUPLE,
          formal(primReal(), SYM(x)),
          formal(primReal(), SYM(y)));
  addFunc(ve, realRealRealToTriple, primTriple(), SYM_TUPLE,
          formal(primReal(), SYM(x)),
          formal(primReal(), SYM(y)),
          formal(primReal(), SYM(z)));
  addFunc(ve, real6ToTransform, primTransform(), SYM_TUPLE,
          formal(primReal(), SYM(x)),
          formal(primReal(), SYM(y)),
          formal(primReal(), SYM(xx)),
          formal(primReal(), SYM(xy)),
          formal(primReal(), SYM(yx)),
          formal(primReal(), SYM(yy)));
}

void addGuideOperators(venv &ve)
{
  // The guide operators .. and -- take an array of guides, and turn them
  // into a single guide.
  addRestFunc(ve, dotsGuide, primGuide(), SYM_DOTS, guideArray());
  addRestFunc(ve, dashesGuide, primGuide(), SYM_DASHES, guideArray());
}

/* Avoid typing the same type three times. */
void addSimpleOperator(venv &ve, bltin f, ty *t, symbol name)
{
  addFunc(ve,f,t,name,formal(t,SYM(a)),formal(t,SYM(b)));
}
void addBooleanOperator(venv &ve, bltin f, ty *t, symbol name)
{
  addFunc(ve,f,primBoolean(),name,formal(t,SYM(a)),formal(t,SYM(b)));
}

template<class T, template <class S> class op>
void addArray2Array2Op(venv &ve, ty *t3, symbol name)
{
  addFunc(ve,array2Array2Op<T,op>,t3,name,formal(t3,SYM(a)),formal(t3,SYM(b)));
}

template<class T, template <class S> class op>
void addOpArray2(venv &ve, ty *t1, symbol name, ty *t3)
{
  addFunc(ve,opArray2<T,T,op>,t3,name,formal(t1,SYM(a)),formal(t3,SYM(b)));
}

template<class T, template <class S> class op>
void addArray2Op(venv &ve, ty *t1, symbol name, ty *t3)
{
  addFunc(ve,array2Op<T,T,op>,t3,name,formal(t3,SYM(a)),formal(t1,SYM(b)));
}

template<class T, template <class S> class op>
void addOps(venv &ve, ty *t1, symbol name, ty *t2)
{
  addSimpleOperator(ve,binaryOp<T,op>,t1,name);
  addFunc(ve,opArray<T,T,op>,t2,name,formal(t1,SYM(a)),formal(t2,SYM(b)));
  addFunc(ve,arrayOp<T,T,op>,t2,name,formal(t2,SYM(a)),formal(t1,SYM(b)));
  addSimpleOperator(ve,arrayArrayOp<T,op>,t2,name);
}

template<class T, template <class S> class op>
void addBooleanOps(venv &ve, ty *t1, symbol name, ty *t2)
{
  addBooleanOperator(ve,binaryOp<T,op>,t1,name);
  addFunc(ve,opArray<T,T,op>,
      booleanArray(),name,formal(t1,SYM(a)),formal(t2,SYM(b)));
  addFunc(ve,arrayOp<T,T,op>,
      booleanArray(),name,formal(t2,SYM(a)),formal(t1,SYM(b)));
  addFunc(ve,arrayArrayOp<T,op>,booleanArray(),name,formal(t2,SYM(a)),
          formal(t2,SYM(b)));
}

void addWrite(venv &ve, bltin f, ty *t1, ty *t2)
{
  addRestFunc(ve,f,primVoid(),SYM(write),t2,
              formal(primFile(),SYM(file),true),
              formal(primString(),SYM(s),true),
              formal(t1,SYM(x)),formal(voidFileFunction(),SYM(suffix),true));
}

template<class T>
void addUnorderedOps(venv &ve, ty *t1, ty *t2, ty *t3, ty *t4)
{
  addBooleanOps<T,equals>(ve,t1,SYM_EQ,t2);
  addBooleanOps<T,notequals>(ve,t1,SYM_NEQ,t2);
   
  addFunc(ve, run::array2Equals<T>, primBoolean(), SYM_EQ, formal(t3, SYM(a)),
          formal(t3, SYM(b)));
  addFunc(ve, run::array2NotEquals<T>, primBoolean(),
          SYM_NEQ, formal(t3, SYM(a)),
          formal(t3, SYM(b)));
  
  addCast(ve,t1,primFile(),read<T>);
  addCast(ve,t2,primFile(),readArray1<T>);
  addCast(ve,t3,primFile(),readArray2<T>);
  addCast(ve,t4,primFile(),readArray3<T>);
  
  addWrite(ve,write<T>,t1,t2);
  addRestFunc(ve,writeArray<T>,primVoid(),SYM(write),t3,
              formal(primFile(),SYM(file),true),
              formal(primString(),SYM(s),true),
              formal(t2,SYM(a),false,true));
  addFunc(ve,writeArray2<T>,primVoid(),SYM(write),
          formal(primFile(),SYM(file),true),t3);
  addFunc(ve,writeArray3<T>,primVoid(),SYM(write),
          formal(primFile(),SYM(file),true),t4);
}

inline double abs(pair z) {
  return z.length();
}

inline double abs(triple v) {
  return v.length();
}

inline pair conjugate(pair z) {
  return conj(z);
}

template<class T>
inline T negate(T x) {
  return -x;
}

template<class T, template <class S> class op>
void addBinOps(venv &ve, ty *t1, ty *t2, ty *t3, ty *t4, symbol name)
{
  addFunc(ve,binopArray<T,op>,t1,name,formal(t2,SYM(a)));
  addFunc(ve,binopArray2<T,op>,t1,name,formal(t3,SYM(a)));
  addFunc(ve,binopArray3<T,op>,t1,name,formal(t4,SYM(a)));
}

template<class T>
void addOrderedOps(venv &ve, ty *t1, ty *t2, ty *t3, ty *t4)
{
  addBooleanOps<T,less>(ve,t1,SYM_LT,t2);
  addBooleanOps<T,lessequals>(ve,t1,SYM_LE,t2);
  addBooleanOps<T,greaterequals>(ve,t1,SYM_GE,t2);
  addBooleanOps<T,greater>(ve,t1,SYM_GT,t2);
  
  addOps<T,run::min>(ve,t1,SYM(min),t2);
  addOps<T,run::max>(ve,t1,SYM(max),t2);
  addBinOps<T,run::min>(ve,t1,t2,t3,t4,SYM(min));
  addBinOps<T,run::max>(ve,t1,t2,t3,t4,SYM(max));
    
  addFunc(ve,sortArray<T>,t2,SYM(sort),formal(t2,SYM(a)));
  addFunc(ve,sortArray2<T>,t3,SYM(sort),formal(t3,SYM(a)));
  
  addFunc(ve,searchArray<T>,primInt(),SYM(search),formal(t2,SYM(a)),
          formal(t1,SYM(key)));
}

template<class T>
void addBasicOps(venv &ve, ty *t1, ty *t2, ty *t3, ty *t4, bool integer=false,
                 bool Explicit=false)
{
  addOps<T,plus>(ve,t1,SYM_PLUS,t2);
  addOps<T,minus>(ve,t1,SYM_MINUS,t2);
  addFunc(ve,initialized,primBoolean(),SYM(initialized),formal(t1,SYM(a)));
  
  addArray2Array2Op<T,plus>(ve,t3,SYM_PLUS);
  addArray2Array2Op<T,minus>(ve,t3,SYM_MINUS);

  addFunc(ve,&id,t1,SYM_PLUS,formal(t1,SYM(a)));
  addFunc(ve,&id,t2,SYM_PLUS,formal(t2,SYM(a)));
  addFunc(ve,Negate<T>,t1,SYM_MINUS,formal(t1,SYM(a)));
  addFunc(ve,arrayFunc<T,T,negate>,t2,SYM_MINUS,formal(t2,SYM(a)));
  addFunc(ve,arrayFunc2<T,T,negate>,t3,SYM_MINUS,formal(t3,SYM(a)));
  if(!integer) addFunc(ve,interp<T>,t1,SYM(interp),
                       formal(t1,SYM(a),false,Explicit),
                       formal(t1,SYM(b),false,Explicit),
                       formal(primReal(),SYM(t)));
  
  addFunc(ve,sumArray<T>,t1,SYM(sum),formal(t2,SYM(a)));
  addUnorderedOps<T>(ve,t1,t2,t3,t4);
}

template<class T>
void addOps(venv &ve, ty *t1, ty *t2, ty *t3, ty *t4, bool integer=false,
            bool Explicit=false)
{
  addBasicOps<T>(ve,t1,t2,t3,t4,integer,Explicit);
  
  addOps<T,times>(ve,t1,SYM_TIMES,t2);
  addOpArray2<T,times>(ve,t1,SYM_TIMES,t3);
  addArray2Op<T,times>(ve,t1,SYM_TIMES,t3);
  
  if(!integer) {
    addOps<T,run::divide>(ve,t1,SYM_DIVIDE,t2);
    addArray2Op<T,run::divide>(ve,t1,SYM_DIVIDE,t3);
  }
      
  addOps<T,power>(ve,t1,SYM_CARET,t2);
}


// Adds standard functions for a newly added array type.
void addArrayOps(venv &ve, types::array *t)
{
  ty *ct = t->celltype;

  // Check for the alias function to see if these operation have already been
  // added, if they have, don't add them again.
  static types::function aliasType(primBoolean(), primVoid(), primVoid());
  aliasType.sig.formals[0].t = t;
  aliasType.sig.formals[1].t = t;

  if (ve.lookByType(SYM(alias), &aliasType))
    return;
  
  addFunc(ve, run::arrayAlias,
          primBoolean(), SYM(alias), formal(t, SYM(a)), formal(t, SYM(b)));

  size_t depth=(size_t) t->depth();

  // Define an array constructor.  This needs to know the depth of the array,
  // which may not be known at runtime.  Therefore, the depth, which is known
  // here at compile-time, is pushed on the stack beforehand by use of a
  // thunk.
  callable *copyValueFunc = new thunk(new vm::bfunc(run::copyArrayValue),(Int) depth-1);
  addFunc(ve, new callableAccess(copyValueFunc),
          t, SYM(array), formal(primInt(), SYM(n)),
          formal(ct, SYM(value)),
          formal(primInt(), SYM(depth), true));
  
  callable *copyFunc = new thunk(new vm::bfunc(run::copyArray),(Int) depth);
  addFunc(ve, new callableAccess(copyFunc),
          t, SYM(copy), formal(t, SYM(a)), formal(primInt(), SYM(depth), true));

  addFunc(ve, run::arrayFunction,
          t, SYM(map), formal(new function(ct, ct), SYM(f)), formal(t, SYM(a)));
  
  addFunc(ve, run::arraySequence,
          t, SYM(sequence), formal(new function(ct, primInt()), SYM(f)),
          formal(primInt(), SYM(n)));
  
  addFunc(ve, run::arraySort,
          t, SYM(sort), formal(t, SYM(a)),
          formal(new function(primBoolean(), ct, ct), SYM(less)));
      
  switch (depth) {
    case 1:
      addRestFunc(ve, run::arrayConcat, t, SYM(concat), new types::array(t));
      addFunc(ve, run::arraySearch,
              primInt(), SYM(search), formal(t, SYM(a)), formal(ct, SYM(key)),
              formal(new function(primBoolean(), ct, ct), SYM(less)));
      break;
    case 2:
      addFunc(ve, run::array2Transpose, t, SYM(transpose), formal(t, SYM(a)));
      break;
    case 3:
      addFunc(ve, run::array3Transpose, t, SYM(transpose), formal(t, SYM(a)),
              formal(IntArray(),SYM(perm)));
      break;
    default:
      break;
  }
}

void addRecordOps(venv &ve, record *r)
{
  addFunc(ve, run::boolMemEq, primBoolean(), SYM(alias), formal(r, SYM(a)),
          formal(r, SYM(b)));
  addFunc(ve, run::boolMemEq, primBoolean(), SYM_EQ, formal(r, SYM(a)),
          formal(r, SYM(b)));
  addFunc(ve, run::boolMemNeq, primBoolean(), SYM_NEQ, formal(r, SYM(a)),
          formal(r, SYM(b)));
}

void addFunctionOps(venv &ve, function *f)
{
  // No function ops.
}


void addOperators(venv &ve) 
{
  addSimpleOperator(ve,binaryOp<string,plus>,primString(),SYM_PLUS);
  
  addBooleanOps<bool,And>(ve,primBoolean(),SYM_AMPERSAND,booleanArray());
  addBooleanOps<bool,Or>(ve,primBoolean(),SYM_BAR,booleanArray());
  addBooleanOps<bool,Xor>(ve,primBoolean(),SYM_CARET,booleanArray());
  
  addUnorderedOps<bool>(ve,primBoolean(),booleanArray(),booleanArray2(),
                        booleanArray3());
  addOps<Int>(ve,primInt(),IntArray(),IntArray2(),IntArray3(),true);
  addOps<double>(ve,primReal(),realArray(),realArray2(),realArray3());
  addOps<pair>(ve,primPair(),pairArray(),pairArray2(),pairArray3(),false,true);
  addBasicOps<triple>(ve,primTriple(),tripleArray(),tripleArray2(),
                      tripleArray3());
  addFunc(ve,opArray<double,triple,times>,tripleArray(),SYM_TIMES,
          formal(primReal(),SYM(a)),formal(tripleArray(),SYM(b)));
  addFunc(ve,opArray2<double,triple,timesR>,tripleArray2(),SYM_TIMES,
          formal(primReal(),SYM(a)),formal(tripleArray2(),SYM(b)));
  addFunc(ve,arrayOp<triple,double,timesR>,tripleArray(),SYM_TIMES,
          formal(tripleArray(),SYM(a)),formal(primReal(),SYM(b)));
  addFunc(ve,array2Op<triple,double,timesR>,tripleArray2(),SYM_TIMES,
          formal(tripleArray2(),SYM(a)),formal(primReal(),SYM(b)));
  addFunc(ve,arrayOp<triple,double,divide>,tripleArray(),SYM_DIVIDE,
          formal(tripleArray(),SYM(a)),formal(primReal(),SYM(b)));

  addUnorderedOps<string>(ve,primString(),stringArray(),stringArray2(),
                          stringArray3());
  
  addSimpleOperator(ve,binaryOp<pair,minbound>,primPair(),SYM(minbound));
  addSimpleOperator(ve,binaryOp<pair,maxbound>,primPair(),SYM(maxbound));
  addSimpleOperator(ve,binaryOp<triple,minbound>,primTriple(),SYM(minbound));
  addSimpleOperator(ve,binaryOp<triple,maxbound>,primTriple(),SYM(maxbound));
  addBinOps<pair,minbound>(ve,primPair(),pairArray(),pairArray2(),pairArray3(),
                           SYM(minbound));
  addBinOps<pair,maxbound>(ve,primPair(),pairArray(),pairArray2(),pairArray3(),
                           SYM(maxbound));
  addBinOps<triple,minbound>(ve,primTriple(),tripleArray(),tripleArray2(),
                             tripleArray3(),SYM(minbound));
  addBinOps<triple,maxbound>(ve,primTriple(),tripleArray(),tripleArray2(),
                             tripleArray3(),SYM(maxbound));
  
  addFunc(ve,arrayFunc<double,pair,abs>,realArray(),SYM(abs),
          formal(pairArray(),SYM(a)));
  addFunc(ve,arrayFunc<double,triple,abs>,realArray(),SYM(abs),
          formal(tripleArray(),SYM(a)));
  
  addFunc(ve,arrayFunc<pair,pair,conjugate>,pairArray(),SYM(conj),
          formal(pairArray(),SYM(a)));
  addFunc(ve,arrayFunc2<pair,pair,conjugate>,pairArray2(),SYM(conj),
          formal(pairArray2(),SYM(a)));
  
  addFunc(ve,binaryOp<Int,divide>,primReal(),SYM_DIVIDE,
          formal(primInt(),SYM(a)),formal(primInt(),SYM(b)));
  addFunc(ve,arrayOp<Int,Int,divide>,realArray(),SYM_DIVIDE,
          formal(IntArray(),SYM(a)),formal(primInt(),SYM(b)));
  addFunc(ve,opArray<Int,Int,divide>,realArray(),SYM_DIVIDE,
          formal(primInt(),SYM(a)),formal(IntArray(),SYM(b)));
  addFunc(ve,arrayArrayOp<Int,divide>,realArray(),SYM_DIVIDE,
          formal(IntArray(),SYM(a)),formal(IntArray(),SYM(b)));
  
  addOrderedOps<Int>(ve,primInt(),IntArray(),IntArray2(),IntArray3());
  addOrderedOps<double>(ve,primReal(),realArray(),realArray2(),realArray3());
  addOrderedOps<string>(ve,primString(),stringArray(),stringArray2(),
                        stringArray3());
  
  addOps<Int,mod>(ve,primInt(),SYM_MOD,IntArray());
  addOps<Int,quotient>(ve,primInt(),SYM_QUOTIENT,IntArray());
  addOps<double,mod>(ve,primReal(),SYM_MOD,realArray());
  
  addRestFunc(ve,diagonal<Int>,IntArray2(),SYM(diagonal),IntArray());
  addRestFunc(ve,diagonal<double>,realArray2(),SYM(diagonal),realArray());
  addRestFunc(ve,diagonal<pair>,pairArray2(),SYM(diagonal),pairArray());
}

dummyRecord *createDummyRecord(venv &ve, symbol name)
{
  dummyRecord *r=new dummyRecord(name);
  vm::frame *f = make_dummyframe(name);
  addConstant(ve, f, r, name);
  addRecordOps(ve, r);
  return r;
}

double identity(double x) {return x;}
double pow10(double x) {return run::pow(10.0,x);}

// An example of an open function.
#ifdef OPENFUNCEXAMPLE
void openFunc(stack *Stack)
{
  vm::array *a=vm::pop<vm::array *>(Stack);
  size_t numArgs=checkArray(a);
  for (size_t k=0; k<numArgs; ++k)
    cout << k << ": " << (*a)[k];
  
  Stack->push<Int>((Int)numArgs);
}
#endif

// A function accessible in asy code print the bytecode of a function.
void printBytecode(stack *Stack)
{
  // As arbitrary addresses can be sent to printBytecode, it should not be run
  // in safe mode. 
  if (settings::safe) {
    cerr << "use -nosafe flag to enable printBytecode" << endl;
    return;
  }

  vm::array *a=vm::pop<vm::array *>(Stack);
  size_t numArgs=checkArray(a);
  if (numArgs != 1)
    cerr << "printBytecode takes one argument" << endl;

  // TODO: Add a reliable test for the object being a func.
  callable *c = a->read<callable *>(0);
  if (func *f = dynamic_cast<func *>(c))
    print(cout, f->body->code);
  else
    cout << "callable is not a standard function";
}




// NOTE: We should move all of these into a "builtin" module.
void base_venv(venv &ve)
{
  // Register the name of arrayDeleteHelper for debugging in "asy -s" mode.
  // This is done automatically for other function, but because
  // arrayDeleteHelper is not defined in the usual way, it must be done
  // explicitly, and here is as good a place as any.
  REGISTER_BLTIN(arrayDeleteHelper, "arrayDeleteHelper");

  addInitializers(ve);
  addCasts(ve);
  addOperators(ve);
  addTupleOperators(ve);
  addGuideOperators(ve);
  
  addRealFunc(sin,SYM(sin));
  addRealFunc(cos,SYM(cos));
  addRealFunc(tan,SYM(tan));
  addRealFunc(asin,SYM(asin));
  addRealFunc(acos,SYM(acos));
  addRealFunc(atan,SYM(atan));
  addRealFunc(exp,SYM(exp));
  addRealFunc(log,SYM(log));
  addRealFunc(log10,SYM(log10));
  addRealFunc(sinh,SYM(sinh));
  addRealFunc(cosh,SYM(cosh));
  addRealFunc(tanh,SYM(tanh));
  addRealFunc(asinh,SYM(asinh));
  addRealFunc(acosh,SYM(acosh));
  addRealFunc(atanh,SYM(atanh));
  addRealFunc(sqrt,SYM(sqrt));
  addRealFunc(cbrt,SYM(cbrt));
  addRealFunc(fabs,SYM(fabs));
  addRealFunc<fabs>(ve,SYM(abs));
  addRealFunc(expm1,SYM(expm1));
  addRealFunc(log1p,SYM(log1p));
  addRealIntFunc<ldexp>(ve, SYM(ldexp), SYM(x), SYM(e));

  addRealFunc(pow10,SYM(pow10));
  addRealFunc(identity,SYM(identity));
  
#ifdef STRUCTEXAMPLE
  dummyRecord *fun=createDummyRecord(ve, SYM(test));
  addFunc(fun->e.ve,realReal<sin>,primReal(),SYM(f),formal(primReal(),SYM(x)));
  addVariable<Int>(fun->e.ve,1,primInt(),SYM(x));
#endif
  
  addFunc(ve,writestring,primVoid(),SYM(write),
          formal(primFile(),SYM(file),true),
          formal(primString(),SYM(s)),
          formal(voidFileFunction(),SYM(suffix),true));
  
  addWrite(ve,write<transform>,primTransform(),transformArray());
  addWrite(ve,write<guide *>,primGuide(),guideArray());
  addWrite(ve,write<pen>,primPen(),penArray());
  addFunc(ve,arrayArrayOp<pen,equals>,booleanArray(),SYM_EQ,
          formal(penArray(),SYM(a)),formal(penArray(),SYM(b)));
  addFunc(ve,arrayArrayOp<pen,notequals>,booleanArray(),SYM_NEQ,
          formal(penArray(),SYM(a)),formal(penArray(),SYM(b)));

  addFunc(ve,arrayFunction,realArray(),SYM(map),
          formal(realPairFunction(),SYM(f)),
          formal(pairArray(),SYM(a)));
  addFunc(ve,arrayFunction,IntArray(),SYM(map),
          formal(IntRealFunction(),SYM(f)),
          formal(realArray(),SYM(a)));
  
  addConstant<Int>(ve, Int_MAX, primInt(), SYM(intMax));
  addConstant<Int>(ve, Int_MIN, primInt(), SYM(intMin));
  addConstant<double>(ve, HUGE_VAL, primReal(), SYM(inf));
  addConstant<double>(ve, run::infinity, primReal(), SYM(infinity));
  addConstant<double>(ve, nan(""), primReal(), SYM(nan));
  addConstant<double>(ve, DBL_MAX, primReal(), SYM(realMax));
  addConstant<double>(ve, DBL_MIN, primReal(), SYM(realMin));
  addConstant<double>(ve, DBL_EPSILON, primReal(), SYM(realEpsilon));
  addConstant<Int>(ve, DBL_DIG, primInt(), SYM(realDigits));
  addConstant<Int>(ve, RANDOM_MAX, primInt(), SYM(randMax));
  addConstant<double>(ve, PI, primReal(), SYM(pi));
  addConstant<string>(ve, string(REVISION),primString(),SYM(VERSION));

  addVariable<pen>(ve, &processData().currentpen, primPen(), SYM(currentpen));

#ifdef OPENFUNCEXAMPLE
  addOpenFunc(ve, openFunc, primInt(), SYM(openFunc));
#endif

  addOpenFunc(ve, printBytecode, primVoid(), SYM(printBytecode));

  gen_runtime_venv(ve);
  gen_runbacktrace_venv(ve);
  gen_runpicture_venv(ve);
  gen_runlabel_venv(ve);
  gen_runhistory_venv(ve);
  gen_runarray_venv(ve);
  gen_runfile_venv(ve);
  gen_runsystem_venv(ve);
  gen_runstring_venv(ve);
  gen_runpair_venv(ve);
  gen_runtriple_venv(ve);
  gen_runpath_venv(ve);
  gen_runpath3d_venv(ve);
  gen_runmath_venv(ve);
  
#ifdef HAVE_LIBGSL
  gen_rungsl_venv(ve);
#endif
}

} //namespace trans

namespace run {

double infinity=cbrt(DBL_MAX); // Reduced for tension atleast infinity

void arrayDeleteHelper(stack *Stack)
{
  array *a=pop<array *>(Stack);
  item itj=pop(Stack);
  bool jdefault=isdefault(itj);
  item iti=pop(Stack);
  Int i,j;
  if(isdefault(iti)) {
    if(jdefault) {
    (*a).clear();
    return;
    } else i=j=get<Int>(itj);
  } else {
    i=get<Int>(iti);
    j=jdefault ? i : get<Int>(itj);
  }

  size_t asize=checkArray(a);
  if(a->cyclic() && asize > 0) {
    if(j-i+1 >= (Int) asize) {
      (*a).clear();
      return;
    }
    i=imod(i,asize);
    j=imod(j,asize);
    if(j >= i) 
      (*a).erase((*a).begin()+i,(*a).begin()+j+1);
    else {
      (*a).erase((*a).begin()+i,(*a).end());
      (*a).erase((*a).begin(),(*a).begin()+j+1);
    }
    return;
  }
  
  if(i < 0 || i >= (Int) asize || i > j || j >= (Int) asize) {
    ostringstream buf;
    buf << "delete called on array of length " << (Int) asize 
        << " with out-of-bounds index range [" << i << "," << j << "]";
    error(buf);
  }

  (*a).erase((*a).begin()+i,(*a).begin()+j+1);
}

// Used by coder to optimize conditional jumps.
const bltin intLess = binaryOp<Int,less>;
const bltin intGreater = binaryOp<Int,greater>;

}
