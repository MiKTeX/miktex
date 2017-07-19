/*****
 * program.cc
 * Tom Prince
 * 
 * The list of instructions used by the virtual machine.
 *****/

#include <iostream>
#include "util.h"
#include "callable.h"
#include "program.h"


namespace vm {

static const char* opnames[] = {
#define OPCODE(name, type) #name,
#include "opcodes.h"
#undef OPCODE
};
static const Int numOps = (Int)(sizeof(opnames)/sizeof(char *));

static const char optypes[] = {
#define OPCODE(name, type) type,
#include "opcodes.h"
#undef OPCODE
};

#ifdef DEBUG_BLTIN
mem::map<bltin,string> bltinRegistry;

void registerBltin(bltin b, string s) {
  bltinRegistry[b] = s;
}
string lookupBltin(bltin b) {
  return bltinRegistry[b];
}
#endif


ostream& operator<< (ostream& out, const item& i)
{
  if (i.empty())
    return out << "empty";

  if (isdefault(i))
    return out << "default";

#if COMPACT
  // TODO: Try to guess the type from the value.
  Int n = get<Int>(i);
  double x = get<double>(i);
  void *p = get<void *>(i);

  if (n == BoolTruthValue)
    return out << "true";
  if (n == BoolFalseValue)
    return out << "false";

  if (std::abs(n) < 1000000)
    return out << n;

#if defined(MIKTEX) && !defined(and)
#define and &&
#endif
  if (fabs(x) < 1e30 and fabs(x) > 1e-30)
    return out << x;

  return out << "<item " << p << ">";
#else
  // TODO: Make a data structure mapping typeids to print functions.
  else if (i.type() == typeid(Int))
    out << "Int, value = " << get<Int>(i);
  else if (i.type() == typeid(double))
    out << "real, value = " << get<double>(i);
  else if (i.type() == typeid(string))
    out << "string, value = " << get<string>(i);
  else if (i.type() == typeid(callable))
    out << *(get<callable *>(i));
  else if (i.type() == typeid(frame)) {
    out << "frame";
#  ifdef DEBUG_FRAME
    {
      frame *f = get<frame *>(i);
      if (f)
        out << " " << f->getName();
      else
        out << " <null>";
    }
#  endif
  }
  else
    out << "type " << demangle(i.type().name());
#endif

  return out;
}

void printInst(ostream& out, const program::label& code,
               const program::label& base)
{
  out.width(4);
  out << offset(base,code) << " ";
  
  Int i = (Int)code->op;
  
  if (i < 0 || i >= numOps) {
    out << "<<invalid op>>" << i;
    return;
  }
  out << opnames[i];

  switch (optypes[i]) {
    case 'n':
    {
      out << " " << get<Int>(*code);
      break;
    }

    case 't':
    {
      item c = code->ref;
      out << " " << c;
      break;
    }

    case 'b':
    {
#ifdef DEBUG_BLTIN
      string s=lookupBltin(get<bltin>(*code));
      out << " " << (!s.empty() ? s : "<unnamed>") << " ";
#endif
      break;
    }

    case 'o':
    {
      char f = out.fill('0');
      out << " i";
      out.width(4);
      out << offset(base,get<program::label>(*code));
      out.fill(f);
      break;
    }

    case 'l':
    {
#ifdef DEBUG_FRAME
      out << " " << get<lambda*>(*code)->name << " ";
#endif
      break;
    }
    
    default: {
      /* nothing else to do */
      break;
    }
  };
}

void print(ostream& out, program *base)
{
  program::label code = base->begin();
  bool active = true;
  while (active) {
    if (code->op == inst::ret || 
        code->op < 0 || code->op >= numOps)
      active = false;
    printInst(out, code, base->begin());
    out << '\n';
    ++code;
  }
}

} // namespace vm
