/*****
 * errormsg.cc
 * Andy Hammerlindl 2002/06/17
 *
 * Used in all phases of the compiler to give error messages.
 *****/

#if defined(MIKTEX)
#include <miktex/ExitThrows>
#endif
#include <cstdio>
#include <cstdlib>

#include "errormsg.h"

errorstream em;

position nullPos;
static nullPosInitializer nullPosInit;

bool errorstream::interrupt=false;

ostream& operator<< (ostream& out, const position& pos)
{
  if (!pos)
    return out;

  out << pos.file->name() << ": ";
  out << pos.line << "." << pos.column << ": ";
  return out;
}

void errorstream::clear()
{
  sync();
  anyErrors = anyWarnings = false;
}

void errorstream::message(position pos, const string& s)
{
  if (floating) out << endl;
  out << pos << s;
  floating = true;
}

void errorstream::compiler(position pos)
{
  message(pos,"compiler: ");
  anyErrors = true;
}

void errorstream::compiler()
{
  message(nullPos,"compiler: ");
  anyErrors = true;
}

void errorstream::runtime(position pos)
{
  message(pos,"runtime: ");
  anyErrors = true;
}

void errorstream::error(position pos)
{
  message(pos,"");
  anyErrors = true;
}

void errorstream::warning(position pos, string s)
{
  message(pos,"warning ["+s+"]: ");
  anyWarnings = true;
}

void errorstream::warning(position pos)
{
  message(pos,"warning: ");
  anyWarnings = true;
}

void errorstream::fatal(position pos)
{
  message(pos,"abort: ");
  anyErrors = true;
}

void errorstream::trace(position pos)
{
  static position lastpos;
  if(!pos || (pos.match(lastpos.filename()) && pos.match(lastpos.Line()))) 
    return;
  lastpos=pos;
  message(pos,"");
  sync();
}

void errorstream::cont()
{
  floating = false;
}

void errorstream::sync()
{
  if (floating) out << endl;
  floating = false;
}

void outOfMemory() 
{
  cerr << "error: out of memory" << endl;
  exit(1);
}
