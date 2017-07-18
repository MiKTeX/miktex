/*****
 * coder.cc
 * Andy Hammerlindl 2004/11/06
 *
 * Handles encoding of syntax into programs.  It's methods are called by
 * abstract syntax objects during translation to construct the virtual machine
 * code.
 *****/

#include <utility>

#include "errormsg.h"
#include "coder.h"
#include "genv.h"
#include "entry.h"
#include "builtin.h"

using namespace sym;
using namespace types;

namespace trans {

namespace {
function *inittype();
function *bootuptype();
}

vm::lambda *newLambda(string name) {
  assert(!name.empty());
  vm::lambda *l = new vm::lambda;
#ifdef DEBUG_FRAME
  l->name = name;
#endif
  return l;
}


// Used purely for global variables and static code blocks of file
// level modules.
coder::coder(position pos, string name, modifier sord)
#if SIMPLE_FRAME
  : level(frame::indirect_frame(name)),
#else
  : level(new frame(name, 0, 0)),
#endif
    recordLevel(0),
    recordType(0),
    isCodelet(false),
    l(newLambda(name)),
    funtype(bootuptype()),
    parent(0),
    sord(sord),
    perm(DEFAULT_PERM),
    program(new vm::program),
    curPos(pos)
{
  sord_stack.push(sord);
}

// Defines a new function environment.
coder::coder(position pos, string name, function *t, coder *parent,
             modifier sord, bool reframe)
  : level(reframe ? new frame(name,
                              parent->getFrame(),
                              t->sig.getNumFormals()) :
                    parent->getFrame()),
    recordLevel(parent->recordLevel),
    recordType(parent->recordType),
    isCodelet(!reframe),
    l(newLambda(name)),
    funtype(t),
    parent(parent),
    sord(sord),
    perm(DEFAULT_PERM),
    program(new vm::program),
    curPos(pos)
{
  sord_stack.push(sord);
}

// Start encoding the body of the record.  The function being encoded
// is the record's initializer.
coder::coder(position pos, record *t, coder *parent, modifier sord)
  : level(t->getLevel()),
    recordLevel(t->getLevel()),
    recordType(t),
    isCodelet(false),
    l(t->getInit()),
    funtype(inittype()),
    parent(parent),
    sord(sord),
    perm(DEFAULT_PERM),
    program(new vm::program),
    curPos(pos)
{
  sord_stack.push(sord);
}

coder coder::newFunction(position pos, string name, function *t, modifier sord)
{
  return coder(pos, name, t, this, sord);
}

coder coder::newCodelet(position pos)
{
  return coder(pos, "<codelet>", new function(primVoid()), this,
               DEFAULT_DYNAMIC, false);
}

record *coder::newRecord(symbol id)
{
  frame *underlevel = getFrame();

  frame *level = new frame(id, underlevel, 0);
  
  record *r = new record(id, level);

  return r;
}

coder coder::newRecordInit(position pos, record *r, modifier sord)
{
  return coder(pos, r, this, sord);
}

#ifdef DEBUG_BLTIN
void assertBltinLookup(inst::opcode op, item it)
{
  if (op == inst::builtin) {
    string name=lookupBltin(vm::get<vm::bltin>(it));
    assert(!name.empty());
  }
}
#endif


void coder::encodePop()
{
  if (isStatic() && !isTopLevel()) {
    assert(parent);
    parent->encodePop();
  }
  else {
#ifdef COMBO
    vm::program::label end = program->end();
    --end;
    inst& lastInst = *end;
    if (lastInst.op == inst::varsave) {
      lastInst.op = inst::varpop;
      return;
    }
    if (lastInst.op == inst::fieldsave) {
      lastInst.op = inst::fieldpop;
      return;
    }
    // TODO: push+pop into no op.
#endif

    // No combo applicable.  Just encode a usual pop.
    encode(inst::pop);
  }
}


bool coder::encode(frame *f)
{
  frame *toplevel = getFrame();
  
  if (f == 0) {
    encode(inst::constpush,(item)0);
    return true;
  }
  else if (f == toplevel) {
    encode(inst::pushclosure);
    return true;
  }
  else {
    encode(inst::varpush,toplevel->parentIndex());
    return encode(f, toplevel->getParent());
  }
}

bool coder::encode(frame *dest, frame *top)
{
  if (dest == 0) {
    // Change to encodePop?
    encode(inst::pop);
    encode(inst::constpush,(item)0);
  }
  else {
    frame *level = top;
    while (level != dest) {
      if (level == 0) {
        // Frame request was in an improper scope.
        return false;
      }

      encode(inst::fieldpush, level->parentIndex());

      level = level->getParent();
    }
  }

  //cerr << "succeeded\n";
  return true;
}

vm::program::label coder::encodeEmptyJump(inst::opcode op)
{
  // Get the end position before encoding the label.  Once encoded, this will
  // point to the instruction.
  vm::program::label pos = program->end();

  encode(op);

  return pos;
}

void replaceEmptyJump(vm::program::label from, vm::program::label to)
{
  from->ref = to;
}

label coder::defNewLabel()
{
  if (isStatic())
    return parent->defNewLabel();
  
  label l = new label_t();
  assert(!l->location.defined());
  assert(!l->firstUse.defined());
  return defLabel(l);
}

label coder::defLabel(label label)
{
  if (isStatic())
    return parent->defLabel(label);

  //cout << "defining label " << label << endl;

  assert(!label->location.defined());
  //vm::program::label here = program->end();
  label->location = program->end();
  assert(label->location.defined());

  if (label->firstUse.defined()) {
    replaceEmptyJump(label->firstUse, program->end());
    //vm::printInst(cout, label->firstUse, program->begin());
    //cout << endl;

    if (label->moreUses) {
      typedef label_t::useVector useVector;
      useVector& v = *label->moreUses;
      for (useVector::iterator p = v.begin(); p != v.end(); ++p) {
        replaceEmptyJump(*p, program->end());
      }
    }
  }

  return label;
}


void coder::useLabel(inst::opcode op, label label)
{
  if (isStatic())
    return parent->useLabel(op,label);
  
  if (label->location.defined()) {
    encode(op, label->location);
  } else {
    if (label->firstUse.defined()) {
      // Store additional uses in the moreUses array.
      if (!label->moreUses)
        label->moreUses = new label_t::useVector;
      label->moreUses->push_back(encodeEmptyJump(op));
    }
    else {
      label->firstUse = encodeEmptyJump(op);
      assert(label->firstUse.defined());
      assert(!label->location.defined());
    }
  }
}
label coder::fwdLabel()
{
  if (isStatic())
    return parent->fwdLabel();
  
  // Create a new label without specifying its position.
  label l = new label_t();
  assert(!l->location.defined());
  assert(!l->firstUse.defined());

  //cout << "forward label " << l << endl;

  return l;
}

bool coder::usesClosureSinceLabel(label l)
{
  assert(l->location.defined());
  for (vm::program::label i = l->location; i != program->end(); ++i)
    if (i->op == inst::pushclosure)
      return true;
  return false;
}

void coder::encodePatch(label from, label to)
{
  assert(from->location.defined());
  assert(to->location.defined());

  assert(from->location->op == inst::nop);

  from->location->op = inst::jmp;
  from->location->ref = to->location;
}

void coder::markPos(position pos)
{
  curPos = pos;
}

// When translating the function is finished, this ties up loose ends
// and returns the lambda.
vm::lambda *coder::close() {
  // These steps must be done dynamically, not statically.
  sord = EXPLICIT_DYNAMIC;
  sord_stack.push(sord);

  // Add a return for void types; may be redundant.
  if (funtype->result->kind == types::ty_void)
    encode(inst::ret);

  l->code = program;

  l->parentIndex = level->parentIndex();

  l->framesize = level->size();

  sord_stack.pop();
  sord = sord_stack.top();

  return l;
}

void coder::closeRecord()
{
  // Put record into finished state.
  encode(inst::pushclosure);
  close();
}

bool coder::isRecord()
{
  return (funtype==inittype());
}

namespace {
function *inittype()
{
  static function t(types::primVoid());
  return &t;
}

function *bootuptype()
{
  static function t(types::primVoid());
  return &t;
}
} // private

} // namespace trans

