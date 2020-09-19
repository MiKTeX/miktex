/*****
 * stack.cc
 * Andy Hammerlindl 2002/06/27
 *
 * The general stack machine used to run compiled camp code.
 *****/

#include <fstream>
#include <sstream>

#include "stack.h"
#include "program.h"
#include "callable.h"
#include "errormsg.h"
#include "util.h"
#include "runtime.h"
#include "process.h"

#include "profiler.h"

#ifdef DEBUG_STACK
#include <iostream>

namespace vm {
void draw(ostream& out, frame *v);
}
#endif

namespace run {
void breakpoint(vm::stack *Stack, absyntax::runnable *r);
}

namespace vm {

mem::list<bpinfo> bplist;
  
namespace {
position curPos = nullPos;
const program::label nulllabel;
}

inline stack::vars_t base_frame(
    size_t size,
    size_t parentIndex,
    stack::vars_t closure
#ifdef DEBUG_FRAME
    , string name
#endif
    )
{
  stack::vars_t vars;
#ifdef SIMPLE_FRAME
  vars = new item[size];
  vars[parentIndex] = closure;
#else
#  ifdef DEBUG_FRAME
  assert(!name.empty());
  vars = new frame(name, parentIndex, size);
#  else
  vars = new frame(size);
#  endif
  (*vars)[parentIndex] = closure;
#endif

  // TODO: Re-factor closure.
  return vars;
}

#ifdef DEBUG_FRAME
#define BASEFRAME(s,p,c,n) (base_frame((s), (p), (c), (n)))
#else
#define BASEFRAME(s,p,c,n) (base_frame((s), (p), (c)))
#endif

// Abstractions needed.
//accessor(frame_handle)
// operator[] for accessor


inline stack::vars_t make_frame(lambda *l, stack::vars_t closure)
{
  return BASEFRAME(l->framesize, l->parentIndex, closure, l->name);
}

inline stack::vars_t make_pushframe(size_t size, stack::vars_t closure)
{
  assert(size >= 1);
  return BASEFRAME(size, 0, closure, "<pushed frame>");
}

stack::vars_t make_dummyframe(string name)
{
  return BASEFRAME(1, 0, 0, "<dummy frame for "+name+">");
}

inline stack::vars_t make_globalframe(size_t size)
{
  assert(size > 0);
#if SIMPLE_FRAME
  // The global frame is an indirect frame.  It holds one item, which is the
  // link to another frame.
  stack::vars_t direct = new item[1];
  stack::vars_t indirect = new item[size];
  direct[0] = indirect;
  return direct;
#else
  return BASEFRAME(size, 0, 0, "<globals>");
#if 0
#ifdef DEBUG_FRAME
  stack::vars_t vars = new frame("<pushed frame>", 0, size);
#else
  stack::vars_t vars = new frame(size);
#endif
  return vars;
#endif
#endif

}

inline void resize_frame(frame *f, size_t oldsize, size_t newsize)
{
  //assert("Need to fix this" == 0);
  assert(newsize > oldsize);
#if SIMPLE_FRAME
  frame *old_indirect = get<frame *>(f[0]);
  frame *new_indirect = new item[newsize];
  std::copy(old_indirect, old_indirect+oldsize, new_indirect);
  f[0] = new_indirect;
#else
  f->extend(newsize);
#endif
}




void run(lambda *l)
{
  func f;
  f.body = l;
  stack s;
  s.run(&f);
}

// Move arguments from stack to frame.
void stack::marshall(size_t args, stack::vars_t vars)
{
  for (size_t i = args; i > 0; --i)
#if SIMPLE_FRAME
    vars[i-1] = pop();
#else
    (*vars)[i-1] = pop();
#endif
}

#ifdef PROFILE

#ifndef DEBUG_FRAME
#warning "profiler needs DEBUG_FRAME for function names"
#endif
#ifndef DEBUG_BLTIN
#warning "profiler needs DEBUG_BLTIN for builtin function names"
#endif

profiler prof;

void dumpProfile() {
  std::ofstream out("asyprof");
  if (!out.fail())
    prof.dump(out);
}
#endif

void assessClosure(lambda *body) {
  // If we have already determined if it needs closure, just return.
  if (body->closureReq != lambda::MAYBE_NEEDS_CLOSURE)
    return;

  for (program::label l = body->code->begin(); l != body->code->end(); ++l)
    if (l->op == inst::pushclosure ||
        l->op == inst::pushframe) {
      body->closureReq = lambda::NEEDS_CLOSURE;
      return;
    }

  body->closureReq = lambda::DOESNT_NEED_CLOSURE;
}

void stack::run(func *f)
{
  lambda *body = f->body;

#ifdef PROFILE
  prof.beginFunction(body);
#endif

#ifdef DEBUG_STACK
#ifdef DEBUG_FRAME
  cout << "running lambda " + body->name + ": \n";
#else
  cout << "running lambda: \n";
#endif
  print(cout, body->code);
  cout << endl;
#endif

  runWithOrWithoutClosure(body, 0, f->closure);

#ifdef PROFILE
  prof.endFunction(body);
#endif
}

void stack::breakpoint(absyntax::runnable *r) 
{
  lastPos=curPos;
  indebugger=true;
  ::run::breakpoint(this,r);
  string s=vm::pop<string>(this);
  debugOp=(s.length() > 0) ? s[0] : (char) 0;
  indebugger=false;
}
  
void stack::debug() 
{
  if(!curPos) return;
  if(indebugger) {em.clear(); return;}
  
  switch(debugOp) {
    case 'i': // inst
      breakpoint();
      break;
    case 's': // step
      if((!curPos.match(lastPos.filename()) || !curPos.match(lastPos.Line())))
        breakpoint();
      break;
    case 'n': // next
      if(curPos.match(lastPos.filename()) && !curPos.match(lastPos.Line()))
        breakpoint();
      break;
    case 'f': // file
      if(!curPos.match(lastPos.filename()))
        breakpoint();
      break;
    case 'r': // return
      if(curPos.match(breakPos.filename()))
        breakpoint();
      break;
    case 'c': // continue
    default:
      for(mem::list<bpinfo>::iterator p=bplist.begin(); p != bplist.end(); ++p) {
        if(curPos.match(p->f.name()) && curPos.match(p->f.line()) &&
           (newline || !curPos.match(breakPos.filename()) ||
            !curPos.match(breakPos.Line()))) {
          breakPos=curPos;
          breakpoint(p->r);
          newline=false;
          break;
        }
        if(!newline && 
           (curPos.match(lastPos.filename()) && !curPos.match(lastPos.Line())))
          newline=true;
      }
      break;
  }
}


void stack::runWithOrWithoutClosure(lambda *l, vars_t vars, vars_t parent)
{
  // The size of the frame (when running without closure).
  size_t frameSize = l->parentIndex;

#ifdef SIMPLE_FRAME
  // Link to the variables, be they in a closure or on the stack.
  frame *varlink;

#  define SET_VARLINK assert(vars); varlink = vars;
#  define VAR(n) ( (varlink)[(n) + frameStart] )
#  define FRAMEVAR(frame,n) (frame[(n)])
#else
  // Link to the variables, be they in a closure or on the stack.
  mem::vector<item> *varlink=NULL;

#  define SET_VARLINK assert(vars); varlink = &vars->vars
#  define VAR(n) ( (*varlink)[(n) + frameStart] )
#  define FRAMEVAR(frame,n) ((*frame)[(n)])
#endif

  size_t frameStart = 0;

  // Set up the closure, if necessary.
  if (vars == 0)
  {
#ifndef SIMPLE_FRAME
    assessClosure(l);
    if (l->closureReq == lambda::NEEDS_CLOSURE)
#endif
    {
      /* make new activation record */
      vars = vm::make_frame(l, parent);
      assert(vars);
    }
#ifndef SIMPLE_FRAME
    else 
    {
      assert(l->closureReq == lambda::DOESNT_NEED_CLOSURE);

      // Use the stack to store variables.
      varlink = &theStack;

      // Record where the parameters start on the stack.
      frameStart = theStack.size() - frameSize;

      // Add the parent's closure to the frame.
      push(parent);
      ++frameSize;

      size_t newFrameSize = (size_t)l->framesize;

      if (newFrameSize > frameSize) {
        theStack.resize(frameStart + newFrameSize);
        frameSize = newFrameSize;
      }
    }
#endif
  }

  if (vars) {
      marshall(l->parentIndex, vars);

      SET_VARLINK;
  }

  /* start the new function */
  program::label ip = l->code->begin();
  position& topPos=processData().topPos;
  string& fileName=processData().fileName;

  try {
    for (;;) {
      const inst &i = *ip;
      curPos = i.pos;
      
  if(curPos.filename() == fileName)
    topPos=curPos;
  
#ifdef PROFILE
      prof.recordInstruction();
#endif

#ifdef DEBUG_STACK
      printInst(cout, ip, l->code->begin());
      cout << "    (";
			i.pos.printTerse(cout);
			cout << ")\n";
#endif

      if(settings::verbose > 4) em.trace(curPos);
      
      if(!bplist.empty()) debug();
      
      if(errorstream::interrupt) throw interrupted();
      
      switch (i.op)
        {
          case inst::varpush:
            push(VAR(get<Int>(i)));
            break;

          case inst::varsave:
            VAR(get<Int>(i)) = top();
            break;
        
#ifdef COMBO
          case inst::varpop:
            VAR(get<Int>(i)) = pop();
            break;
#endif

          case inst::ret: {
            if (vars == 0)
              // Delete the frame from the stack.
              // TODO: Optimize for common cases.
              theStack.erase(theStack.begin() + frameStart,
                             theStack.begin() + frameStart + frameSize);
            return;
          }

          case inst::pushframe:
          {
            assert(vars);
            Int size = get<Int>(i);
            vars=make_pushframe(size, vars);

            SET_VARLINK;

            break;
          }

          case inst::popframe:
          {
            assert(vars);
            vars=get<frame *>(VAR(0));

            SET_VARLINK;

            break;
          }

          case inst::pushclosure:
            assert(vars);
            push(vars);
            break; 

          case inst::nop:
            break;

          case inst::pop:
            pop();
            break;
        
          case inst::intpush:
          case inst::constpush:
            push(i.ref);
            break;
        
          case inst::fieldpush: {
            vars_t frame = pop<vars_t>();
            if (!frame)
              error("dereference of null pointer");
            push(FRAMEVAR(frame, get<Int>(i)));
            break;
          }
        
          case inst::fieldsave: {
            vars_t frame = pop<vars_t>();
            if (!frame)
              error("dereference of null pointer");
            FRAMEVAR(frame, get<Int>(i)) = top();
            break;
          }

#if COMBO
          case inst::fieldpop: {
#error NOT REIMPLEMENTED
            vars_t frame = pop<vars_t>();
            if (!frame)
              error("dereference of null pointer");
            FRAMEVAR(get<Int>(i)) = pop();
            break;
          }
#endif
        
        
          case inst::builtin: {
            bltin func = get<bltin>(i);
#ifdef PROFILE
            prof.beginFunction(func);
#endif
            func(this);
#ifdef PROFILE
            prof.endFunction(func);
#endif
            break;
          }

          case inst::jmp:
            ip = get<program::label>(i);
            continue;

          case inst::cjmp:
            if (pop<bool>()) { ip = get<program::label>(i); continue; }
            break;

          case inst::njmp:
            if (!pop<bool>()) { ip = get<program::label>(i); continue; }
            break;

          case inst::jump_if_not_default:
            if (!isdefault(pop())) { ip = get<program::label>(i); continue; }
            break;

#ifdef COMBO
          case inst::gejmp: {
            Int y = pop<Int>();
            Int x = pop<Int>();
            if (x>=y)
              { ip = get<program::label>(i); continue; }
            break;
          }

#if 0
          case inst::jump_if_func_eq: {
            callable * b=pop<callable *>();
            callable * a=pop<callable *>();
            if (a->compare(b))
              { ip = get<program::label>(i); continue; }
            break;
          }

          case inst::jump_if_func_neq: {
            callable * b=pop<callable *>();
            callable * a=pop<callable *>();
            if (!a->compare(b))
              { ip = get<program::label>(i); continue; }
            break;
          }
#endif
#endif

          case inst::push_default:
            push(Default);
            break;

          case inst::popcall: {
            /* get the function reference off of the stack */
            callable* f = pop<callable*>();
            f->call(this);
            break;
          }

          case inst::makefunc: {
            func *f = new func;
            f->closure = pop<vars_t>();
            f->body = get<lambda*>(i);

            push((callable*)f);
            break;
          }
        
          default:
            error("Internal VM error: Bad stack operand");
        }

#ifdef DEBUG_STACK
      draw(cerr);
      vm::draw(cerr,vars);
      cerr << "\n";
#endif
            
      ++ip;
    }
  } catch (bad_item_value&) {
    error("Trying to use uninitialized value.");
  }

#undef SET_VARLINK
#undef VAR
#undef FRAMEVAR
}

void stack::load(string index) {
  frame *inst=instMap[index];
  if (inst)
    push(inst);
  else {
    func f;
    assert(initMap);
    f.body=(*initMap)[index];
    assert(f.body);
    run(&f);
    instMap[index]=get<frame *>(top());
  }
}


#ifdef DEBUG_STACK

const size_t MAX_ITEMS=20;

void stack::draw(ostream& out)
{
//  out.setf(out.hex);

  out << "operands:";
  stack_t::const_iterator left = theStack.begin();
  if (theStack.size() > MAX_ITEMS) {
    left = theStack.end()-MAX_ITEMS;
    out << " ...";
  }
  else
    out << " ";
  
  while (left != theStack.end())
    {
      if (left != theStack.begin())
        out << " | " ;
      out << *left;
      left++;
    }
  out << "\n";
}

void draw(ostream& out, frame* v)
{
  out << "vars:" << endl;
  
  while (!!v) {
    item link=(*v)[v->getParentIndex()];

    out << "  " <<  v->getName() << ":  ";

    for (size_t i = 0; i < MAX_ITEMS && i < v->size(); i++) {
      if (i > 0)
        out << " | ";
      out << i << ": ";

      if (i == v->getParentIndex()) {
        try {
          frame *parent = get<frame *>(link);
          out << (parent ? "link" :  "----");
        } catch (bad_item_value&) {
          out << "non-link " << (*v)[0];
        }
      } else {
        out << (*v)[i];
      }
    }

    if (v->size() > MAX_ITEMS)
      out << "...";
    out << "\n";


    frame *parent;
    try {
      parent = get<frame *>(link);
    } catch (bad_item_value&) {
      parent = 0;
    }

    v = parent;
  }
}
#endif // DEBUG_STACK

position getPos() {
  return curPos;
}

void errornothrow(const char* message)
{
  em.error(curPos);
  em << message;
  em.sync();
}
  
void error(const char* message)
{
  errornothrow(message);
  throw handled_error();
}
  
void error(const ostringstream& message)
{
  const string& s=message.str();
  error(s.c_str());
}

const size_t STARTING_GLOBALS_SIZE = 1;
interactiveStack::interactiveStack()
  : globals(make_globalframe(STARTING_GLOBALS_SIZE)),
    globals_size(STARTING_GLOBALS_SIZE) {}

void interactiveStack::run(lambda *codelet) {
  if (globals_size < codelet->framesize) {
    resize_frame(globals, globals_size, codelet->framesize);
    globals_size = codelet->framesize;
  }
  stack::runWithOrWithoutClosure(codelet, globals, 0);
}

} // namespace vm
