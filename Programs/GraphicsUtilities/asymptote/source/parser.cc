/*****
 * parser.cc
 * Tom Prince 2004/01/10
 *
 *****/

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include <fstream>
#include <sstream>

#include "common.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if defined(MIKTEX_WINDOWS)
#  include <miktex/unxemu.h>
#  include <miktex/Util/CharBuffer>
#  define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

#include "interact.h"
#include "locate.h"
#include "errormsg.h"
#include "parser.h"

// The lexical analysis and parsing functions used by parseFile.
void setlexer(size_t (*input) (char* bif, size_t max_size), string filename);
#if defined(MIKTEX)
extern int yyparse(void);
#else
extern bool yyparse(void);
#endif
extern int yydebug;
extern int yy_flex_debug;
static const int YY_NULL = 0;
extern bool lexerEOF();
extern void reportEOF();

namespace parser {

namespace yy { // Lexers

std::streambuf *sbuf = NULL;

size_t stream_input(char *buf, size_t max_size)
{
  size_t count= sbuf ? sbuf->sgetn(buf,max_size) : 0;
  return count ? count : YY_NULL;
}

} // namespace yy

void debug(bool state)
{
  // For debugging the machine-generated lexer and parser.
  yy_flex_debug = yydebug = state;
}

namespace {
void error(const string& filename)
{
  em.sync();
  em << "error: could not load module '" << filename << "'\n";
  em.sync();
  throw handled_error();
}
}

absyntax::file *doParse(size_t (*input) (char* bif, size_t max_size),
                        const string& filename, bool extendable=false)
{
  setlexer(input,filename);
  absyntax::file *root = yyparse() == 0 ? absyntax::root : 0;
  absyntax::root = 0;
  yy::sbuf = 0;

  if (!root) {
    if (lexerEOF()) {
      if (extendable) {
        return 0;
      } else {
        // Have the lexer report the error.
        reportEOF();
      }
    }

    em.error(nullPos);
    if(!interact::interactive)
      error(filename);
    else
      throw handled_error();
  }

  return root;
}

absyntax::file *parseStdin()
{
  debug(false);
  yy::sbuf = cin.rdbuf();
  return doParse(yy::stream_input,"-");
}

absyntax::file *parseFile(const string& filename,
                          const char *nameOfAction)
{
  if(filename == "-")
    return parseStdin();
  
  string file = settings::locateFile(filename);

  if(file.empty())
    error(filename);

  if(nameOfAction && settings::verbose > 1)
    cerr << nameOfAction << " " <<  filename << " from " << file << endl;
  
  debug(false); 

  std::filebuf filebuf;
#if defined(MIKTEX_WINDOWS)
  if (!filebuf.open(UW_(file.c_str()), std::ios::in))
#else
  if(!filebuf.open(file.c_str(),std::ios::in))
#endif
    error(filename);
  
#ifdef HAVE_SYS_STAT_H
  // Check that the file is not a directory.
  static struct stat buf;
  if(stat(file.c_str(),&buf) == 0) {
    if(S_ISDIR(buf.st_mode))
      error(filename);
  }
#endif
  
  // Check that the file can actually be read.
  try {
    filebuf.sgetc();
  } catch (...) {
    error(filename);
  }
  
  yy::sbuf = &filebuf;
  return doParse(yy::stream_input,file);
}

absyntax::file *parseString(const string& code,
                            const string& filename,
                            bool extendable)
{
  debug(false);
  stringbuf buf(code);
  yy::sbuf = &buf;
  return doParse(yy::stream_input,filename,extendable);
}

} // namespace parser

