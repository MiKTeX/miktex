/*****
 * errormsg.h
 * Andy Hammerlindl 2002/06/17
 *
 * Used in all phases of the compiler to give error messages.
 *****/

#ifndef ERRORMSG_H
#define ERRORMSG_H

#include <iostream>
#include <exception>
#include "common.h"
#include "settings.h"
#include "symbolmaps.h"

using std::ostream;

struct handled_error : std::exception {}; // Exception to process next file.
struct interrupted : std::exception {};   // Exception to interrupt execution.
struct quit : std::exception {};          // Exception to quit current operation.
#if defined(MIKTEX)
struct eof_exception :
    std::exception
{
};
#else
struct eof : std::exception {};           // Exception to exit interactive mode.
#endif

class fileinfo : public gc {
  string filename;
  size_t lineNum;

public:
  fileinfo(string filename, size_t lineNum=1)
    : filename(filename), lineNum(lineNum) {}

  size_t line() const
  {
    return lineNum;
  }

  string name() const {
    return filename;
  }

  // The filename without the directory and without the '.asy' suffix.
  // Note that this assumes name are separated by a forward slash.
  string moduleName() const {
    size_t start = filename.rfind('/');
    if (start == filename.npos)
      start = 0;
    else
      // Step over slash.
      ++start;

    size_t end = filename.rfind(".asy");
    if (end != filename.size() - 4)
      end = filename.size();

    return filename.substr(start, end-start);
  }

  // Specifies a newline symbol at the character position given.
  void newline() {
    ++lineNum;
  }

};

inline bool operator == (const fileinfo& a, const fileinfo& b)
{
  return a.line() == b.line() && a.name() == b.name();
}

class position : public gc {
  fileinfo *file;
  size_t line;
  size_t column;

public:
  void init(fileinfo *f, Int p) {
    file = f;
    if (file) {
      line = file->line();
      column = p;
    } else {
      line = column = 0;
    }
  }

  string filename() const {
    return file ? file->name() : "";
  }

  size_t Line() const {
    return line;
  }

  size_t Column() const {
    return column;
  }

  position shift(unsigned int offset) const {
    position P=*this;
    P.line -= offset;
    return P;
  }

  std::pair<size_t,size_t>LineColumn() const {
    return std::pair<size_t,size_t>(line,column);
  }

  bool match(const string& s) {
    return file && file->name() == s;
  }

  bool match(size_t l) {
    return line == l;
  }

  bool matchColumn(size_t c) {
    return column == c;
  }

  bool operator! () const
  {
    return (file == 0);
  }

  friend ostream& operator << (ostream& out, const position& pos);

  typedef std::pair<size_t, size_t> posInFile;
  typedef std::pair<std::string, posInFile> filePos;

  explicit operator AsymptoteLsp::filePos()
  {
    return std::make_pair((std::string) file->name().c_str(),LineColumn());
  }

  void print(ostream& out) const
  {
    if (file) {
      out << file->name() << ":" << line << "." << column;
    }
  }

  // Write out just the module name and line number.
  void printTerse(ostream& out) const
  {
    if (file) {
      out << file->moduleName() << ":" << line;
    }
  }
};

extern position nullPos;

struct nullPosInitializer {
  nullPosInitializer() {nullPos.init(NULL,0);}
};

inline bool operator == (const position& a, const position& b)
{
  return a.Line() == b.Line() && a.Column() == b.Column() &&
    a.filename() == b.filename();
}

string warning(string s);

class errorstream {
  ostream& out;
  bool anyErrors;
  bool anyWarnings;
  bool floating;        // Was a message output without a terminating newline?

  // Is there an error that warrants the asy process to return 1 instead of 0?
  bool anyStatusErrors;

public:
  static bool interrupt; // Is there a pending interrupt?

  errorstream(ostream& out = cerr)
    : out(out), anyErrors(false), anyWarnings(false), floating(false),
      anyStatusErrors(false) {}


  void clear();

  void message(position pos, const string& s);

  void Interrupt(bool b) {
    interrupt=b;
  }

  // An error is encountered, not in the user's code, but in the way the
  // compiler works!  This may be augmented in the future with a message
  // to contact the compiler writers.
  void compiler();
  void compiler(position pos);

  // An error encountered when running compiled code.  This method does
  // not stop the executable, but the executable should be stopped
  // shortly after calling this method.
  void runtime(position pos);

  // Errors encountered when compiling making it impossible to run the code.
  void error(position pos);

  // Indicate potential problems in the code, but the code is still usable.
  void warning(position pos);
  void warning(position pos, string s);

  // Single a fatal error and execute the main process.
  void fatal(position pos);

  // Print out position in code to aid debugging.
  void trace(position pos);

  // Sends stuff to out to print.
  // NOTE: May later make it do automatic line breaking for long messages.
  template<class T>
  errorstream& operator << (const T& x) {
    flush(out);
    out << x;
    return *this;
  }

  // Reporting errors to the stream may be incomplete.  This draws the
  // appropriate newlines or file excerpts that may be needed at the end.
  void sync();

  void cont();

  bool errors() const {
    return anyErrors;
  }

  bool warnings() const {
    return anyWarnings || errors();
  }

  void statusError() {
    anyStatusErrors=true;
  }

  // Returns true if no errors have occured that should be reported by the
  // return value of the process.
  bool processStatus() const {
    return !anyStatusErrors;
  }
};

extern errorstream em;
void outOfMemory();

GC_DECLARE_PTRFREE(nullPosInitializer);

#endif
