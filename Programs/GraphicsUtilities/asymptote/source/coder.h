/*****
 * coder.h
 * Andy Hammerlindl 2004/11/06
 *
 * Handles encoding of syntax into programs.  It's methods are called by
 * abstract syntax objects during translation to construct the virtual machine
 * code.
 *****/

#ifndef CODER_H
#define CODER_H

#include "errormsg.h"
#include "entry.h"
#include "types.h"
#include "record.h"
#include "frame.h"
#include "program.h"
#include "util.h"
#include "modifier.h"
#include "inst.h"

namespace trans {

using sym::symbol;
using types::ty;
using types::function;
using types::record;

using vm::bltin;
using vm::inst;
using vm::item;

#ifdef DEBUG_BLTIN
void assertBltinLookup(inst::opcode op, item it);
#endif

// Labels used by the coder class to denote where in the code a jump
// instruction should go to.  Label can be used before their exact location is
// known.
// Declared outside of the coder class, so that it can be declared in exp.h.
struct label_t : public gc {
  vm::program::label location;
  vm::program::label firstUse;

  // Most labels are used only once, and so we optimize for that case.  We do,
  // however, have to handle labels which are used multiple times (such as
  // break locations in a loop), and so a useVector is allocated to store
  // these if necessary.
  typedef mem::vector<vm::program::label> useVector;
  useVector *moreUses;

  // Only the constructor is defined.  Everything else is handles by methods
  // of the coder class.
  label_t() : location(), firstUse(), moreUses(0) {}
};
typedef label_t *label;

class coder {
  // The frame of the function we are currently encoding.  This keeps
  // track of local variables, and parameters with respect to the stack.
  frame *level;

  // The frame of the enclosing record that the "this" expression yields.  ie.
  // the highest frame that is a record, not a function. 
  frame *recordLevel;

  // The type of the enclosing record.  Also needed for the "this" expression.
  record *recordType;
  
  // Are we translating a codelet?
  bool isCodelet;

  // The lambda being constructed. In some cases, this lambda is needed
  // before full translation of the function, so it is stored,
  // incomplete, here.
  vm::lambda *l;

  // The type of the function being translated.
  const function *funtype;

  // The enclosing environment.  Null if this is a file-level module.
  coder *parent;

  // The mode of encoding, either static or dynamic. sord is used as an
  // acronym for Static OR Dynamic.
  // Once something is static, no amount of dynamic modifiers can change
  // that, so once a stack is EXPLICIT_STATIC, additional modifiers will
  // be pushed on as EXPLICIT_STATIC.
  modifier sord;
  std::stack<modifier> sord_stack;

  // What permissions will be given to a new access.
  // TODO: Ensure private fields don't show up calling lookup for a
  // record.
  permission perm;
  
  // The function code as its being written.  Code points to next place in
  // array to write.
  vm::program *program;

  // Some loops allocate nested frames, in case variables in an
  // iteration escape in a closure.  This stack keeps track of where the
  // pushframe instructions are, so the size of the frame can be encoded.
  std::stack<vm::program::label> pushframeLabels;

  // Loops need to store labels to where break and continue statements
  // should pass control.  Since loops can be nested, this needs to
  // be stored as a stack.  We also store which of the loops are being encoded
  // with an additional frame for variables.  This is needed to know if the
  // break and continue statements need to pop the frame.
  struct loopdata_t : gc {
    label continueLabel;
    label breakLabel;
    bool pushedFrame;

    loopdata_t(label c, label b)
      : continueLabel(c), breakLabel(b), pushedFrame(false) {}
  };
  mem::stack<loopdata_t> loopdata;

  // Current File Position
  position curPos;

public:
  // Define a new function coder.  If reframe is true, this gives the function
  // its own frame, which is the usual (sensible) thing to do.  It is set to
  // false for a line-at-a-time codelet, where variables should be allocated in
  // the lower frame.
  coder(position pos,
        string name, function *t, coder *parent,
        modifier sord = DEFAULT_DYNAMIC,
        bool reframe=true);

  // Start encoding the body of the record.  The function being encoded
  // is the record's initializer.
  coder(position pos,
        record *t, coder *parent, modifier sord = DEFAULT_DYNAMIC);

  coder(position pos,
        string name, modifier sord = DEFAULT_DYNAMIC);
  
  coder(const coder&);
  
  /* Add a static or dynamic modifier. */
  void pushModifier(modifier s)
  {
    /* Default setting should only be used in the constructor. */
    assert(s != DEFAULT_STATIC && s != DEFAULT_DYNAMIC);

    /* Non-default static overrules. */
    if (sord != EXPLICIT_STATIC)
      sord = s;

    sord_stack.push(sord);
  }

  /* Tests if encoding mode is currently static. */
  bool isStatic()
  {
    switch(sord) {
      case DEFAULT_STATIC:
      case EXPLICIT_STATIC:
        return true;
      case DEFAULT_DYNAMIC:
      case EXPLICIT_DYNAMIC:
        return false;
      default:
        assert(False);
        return false;
    }
  }


  /* Remove a modifier. */
  void popModifier()
  {
    assert(!sord_stack.empty());
    sord_stack.pop();

    assert(!sord_stack.empty());
    sord = sord_stack.top();
  }

  /* Set/get/clear permissions. */
  void setPermission(permission p)
  {
    perm = p;
  }
  permission getPermission()
  {
    return perm;
  }
  void clearPermission()
  {
    perm = DEFAULT_PERM;
  }
    

  // Says what the return type of the function is.
  ty *getReturnType() {
    return funtype->result;
  }

  bool isRecord();
  
  // Creates a new coder to handle the translation of a new function.
  coder newFunction(position pos,
                    string name, function *t, modifier sord=DEFAULT_DYNAMIC);

  // Creates a new record type.
  record *newRecord(symbol id);

  // Create a coder for the initializer of the record.
  coder newRecordInit(position pos, record *r, modifier sord=DEFAULT_DYNAMIC);

  // Create a coder for translating a small piece of code.  Used for
  // line-at-a-time mode.
  coder newCodelet(position pos);

  frame *getFrame()
  {
    if (isStatic() && !isTopLevel()) {
      assert(parent->getFrame());
      return parent->getFrame();
    }
    else
      return level;
  }

  // Tests if the function or record with the given frame is currently under
  // translation (either by this coder or an ancestor).
  bool inTranslation(frame *f) {
    frame *level=this->level;
    while (level) {
      if (f==level)
        return true;
      level=level->getParent();
    }
    return parent && parent->inTranslation(f);
  }

  // Allocates space in the function or record frame for a new local variable.
  access *allocLocal()
  {
    return getFrame()->allocLocal();
  }

  // Get the access in the frame for a specified formal parameter.
  access *accessFormal(Int index)
  {
    // NOTE: This hasn't been extended to handle frames for loops, but is
    // currently only called when starting to translate a function, where there
    // can be no loops.
    return level->accessFormal(index);
  }

  // Checks if we are at the top level, which is true for a file-level module or
  // a codelet.
  bool isTopLevel() {
    return parent==0 || isCodelet;
  }

  // The encode functions add instructions and operands on to the code array.
private:
  void encode(inst i)
  {
    i.pos = curPos;
    // Static code is put into the enclosing coder, unless we are translating a
    // codelet.
    if (isStatic() && !isTopLevel()) {
      assert(parent);
      parent->encode(i);
    }
    else {
      program->encode(i);
    }
  }

  // Encode a jump to a not yet known location.
  vm::program::label encodeEmptyJump(inst::opcode op);

public:
  void encode(inst::opcode op)
  {
    inst i; i.op = op; i.pos = nullPos;
    encode(i);
  }
  void encode(inst::opcode op, item it)
  {
#ifdef DEBUG_BLTIN
    assertBltinLookup(op, it);
#endif
    inst i; i.op = op; i.pos = nullPos; i.ref = it;
    encode(i);
  }

  // Encodes a pop instruction, or merges the pop into the previous
  // instruction (ex. varsave+pop becomes varpop).
  void encodePop();

  // Puts the requested frame on the stack.  If the frame is not that of
  // this coder or its ancestors, false is returned.
  bool encode(frame *f);

  // Puts the frame corresponding to the expression "this" on the stack.
  bool encodeThis()
  {
    assert(recordLevel);
    return encode(recordLevel);
  }

  // An access that encodes the frame corresponding to "this".
  access *thisLocation()
  {
    assert(recordLevel);
    return new frameAccess(recordLevel);
  }

  // Returns the type of the enclosing record.
  record *thisType()
  {
    return recordType;
  }

  // Puts the 'dest' frame on the stack, assuming the frame 'top' is on
  // top of the stack.  If 'dest' is not an ancestor frame of 'top',
  // false is returned.
  bool encode(frame *dest, frame *top);


  // Assigns a handle to the current point in the list of bytecode
  // instructions and returns that handle.
  label defNewLabel();

  // Sets the handle given by label to the current point in the list of
  // instructions.
  label defLabel(label label);

  // Encodes the address pointed to by the handle label into the
  // sequence of instructions.  This is useful for a jump instruction to
  // jump to where a label was defined.
  void useLabel(inst::opcode op, label label);

  // If an address has to be used for a jump instruction before it is
  // actually encoded, a handle can be given to it by this function.
  // When that handle's label is later defined, the proper address will
  // be inserted into the code where the handle was used. 
  label fwdLabel();

  void pushLoop(label c, label b) {
    loopdata.push(loopdata_t(c,b));
  }
  void popLoop() {
    loopdata.pop();
  }
  void loopPushesFrame()
  {
    assert(!loopdata.empty());
    loopdata_t& d = loopdata.top();
    d.pushedFrame = true;
  }
  bool encodeBreak() {
    if (loopdata.empty())
      return false;
    else {
      loopdata_t& d = loopdata.top();
      if (d.pushedFrame)
        encode(inst::popframe);
      useLabel(inst::jmp,d.breakLabel);
      return true;
    }
  }
  bool encodeContinue() {
    if (loopdata.empty())
      return false;
    else {
      loopdata_t& d = loopdata.top();
      if (d.pushedFrame)
        encode(inst::popframe);
      useLabel(inst::jmp,d.continueLabel);
      return true;
    }
  }

  // Returns true if a pushclosure has been encoded since the definition of
  // the label.
  bool usesClosureSinceLabel(label l);

  // Turn a no-op into a jump to bypass incorrect code.
  void encodePatch(label from, label to);

public:
  void encodePushFrame() {
    pushframeLabels.push(program->end());
    encode(inst::pushframe, (Int)0);

    level = new frame("encodePushFrame", level, 0);
  }

  void encodePopFrame() {
    pushframeLabels.top()->ref = level->size();
    pushframeLabels.pop();

    encode(inst::popframe);

    level = level->getParent();
  }

  // Adds an entry into the position list, linking the given point in the
  // source code to the current position in the virtual machine code.  This is
  // used to print positions at runtime.
  void markPos(position pos);

  // When translation of the function is finished, this ties up loose ends
  // and returns the lambda.
  vm::lambda *close();

  // Finishes translating the initializer of a record.
  void closeRecord();

private: // Non-copyable
  void operator=(const coder&);
};

} // namespace trans

#endif
