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
#include "interact.h"
#include "fileio.h"

errorstream em;

position nullPos;
static nullPosInitializer nullPosInit;

bool errorstream::interrupt=false;

ostream& operator<< (ostream& out, const position& pos)
{
  if (!pos)
    return out;

  string filename=pos.file->name();

  if(filename != "-" && !settings::getSetting<bool>("quiet")) {
    std::ifstream fin(filename.c_str());
    string s;
    size_t count=pos.line;
    while(count > 0 && getline(fin,s)) {
      count--;
    }
    out << s << endl;
    for(size_t i=1; i < pos.column; ++i)
      out << " ";
    out << "^" << endl;
  }

  out << filename << ": ";
  out << pos.line << "." << pos.column << ": ";

  if(settings::getSetting<bool>("xasy")) {
    camp::openpipeout();
    fprintf(camp::pipeout,"Error\n");
    fflush(camp::pipeout);
  }

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
