/*****
 * errormsg.cc
 * Andy Hammerlindl 2002/06/17
 *
 * Used in all phases of the compiler to give error messages.
 *****/

#if defined(MIKTEX)
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif
#include <miktex/ExitThrows>
#endif
#include <cstdio>
#include <cstdlib>
#include <regex>

#include "errormsg.h"
#include "interact.h"
#include "fileio.h"

errorstream em;

position nullPos;
static nullPosInitializer nullPosInit;

bool errorstream::interrupt=false;

using camp::newl;

ostream& operator<< (ostream& out, const position& pos)
{
  if (!pos)
    return out;

  string filename=pos.file->name();

  if(filename != "-" && !settings::getSetting<bool>("quiet")) {
#if defined(MIKTEX_WINDOWS)
    std::ifstream fin(UW_(filename));
#else
    std::ifstream fin(filename.c_str());
#endif
    string s;
    size_t count=pos.line;
    while(count > 0 && getline(fin,s)) {
      count--;
    }
    s=std::regex_replace(s,std::regex("\t")," ");
    out << s << endl;
    for(size_t i=1; i < pos.column; ++i)
      out << " ";
    out << "^" << endl;
  }

  out << filename << ": ";
  out << pos.line << "." << pos.column;

  if(settings::xasy) {
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
  out << pos << ": " << s;
  floating = true;
}

void errorstream::compiler(position pos)
{
  message(pos,"Compiler bug; report to https://github.com/vectorgraphics/asymptote/issues:\n");
  anyErrors = true;
}

void errorstream::compiler()
{
  compiler(nullPos);
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

void errorstream::sync(bool reportTraceback)
{
  if (floating) out << endl;

  if(reportTraceback && traceback.size()) {
    bool first=true;
    for(auto p=this->traceback.rbegin(); p != this->traceback.rend(); ++p) {
      if(p->filename() != "-") {
        if(first) {
          out << newl << "TRACEBACK:";
          first=false;
        }
        cout << newl << (*p) << endl;
      }
    }
    traceback.clear();
  }

  floating = false;
}

void outOfMemory()
{
  cerr << "error: out of memory" << endl;
  exit(1);
}
