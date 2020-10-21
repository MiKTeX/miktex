/*****
 * process.cc
 * Andy Hammerlindl 2006/08/19
 *
 * Handles processing blocks of code (including files, strings, and the
 * interactive prompt, for listing and parse-only modes as well as actually
 * running it.
 *****/

#include "types.h"
#include "errormsg.h"
#include "genv.h"
#include "stm.h"
#include "settings.h"
#include "vm.h"
#include "program.h"
#include "interact.h"
#include "envcompleter.h"
#include "parser.h"
#include "fileio.h"

#include "stack.h"
#include "runtime.h"
#include "texfile.h"

#include "process.h"

namespace camp {
pen& defaultpen() {
  return processData().defaultpen;
}
}

unsigned int count=0;

namespace run {
void cleanup();
void exitFunction(vm::stack *Stack);
void updateFunction(vm::stack *Stack);
void purge(Int divisor=0);
}

namespace vm {
bool indebugger;
}

using namespace settings;

using absyntax::file;
using trans::genv;
using trans::coenv;
using trans::env;
using trans::coder;
using types::record;

using interact::interactive;
using interact::uptodate;

using absyntax::runnable;
using absyntax::block;

mem::stack<processDataStruct *> processDataStack;

// Exception-safe way to push and pop the process data.
class withProcessData {
  // Do not let this object be dynamically allocated.
  void *operator new(size_t);

  processDataStruct *pd_ptr;
public:

  withProcessData(processDataStruct *pd_ptr) : pd_ptr(pd_ptr)
  {
    processDataStack.push(pd_ptr);
  }

  ~withProcessData()
  {
    assert(processDataStack.top() == pd_ptr);
    processDataStack.pop();
  }
};

processDataStruct &processData() {
  assert(!processDataStack.empty());
  return *processDataStack.top();
}

// A process environment.  Basically this just serves as short-hand to start a
// new global environment (genv) and new process data at the same time.  When
// it goes out of scope, the process data is popped off the stack.  This also
// ensures that the data is popped even if an exception is thrown.
class penv {
  genv *_ge;
  processDataStruct _pd;

  // Do not let this object be dynamically allocated.
  void *operator new(size_t);

public:
  penv() : _ge(0), _pd() {
    // Push the processData first, as it needs to be on the stack before genv
    // is initialized.
    processDataStack.push(&_pd);
    _ge = new genv;
  }

  virtual ~penv() {
    processDataStack.pop();
    delete _ge;
  }

  genv &ge() { return *_ge; }

  processDataStruct *pd() { return &_pd; }
};


void init(bool resetpath=true)
{
  vm::indebugger=false;
  uptodate=false;
  if(resetpath)
    setPath("");  /* On second and subsequent calls, sets the path
                     to what it was when the program started. */
}

// This helper class does nothing but call the interactiveTrans method of the
// base object in place of trans, so that the runnable can exhibit special
// behaviour when run at the interactive prompt.
class interactiveRunnable : public runnable {
  runnable *base;

public:
  interactiveRunnable(runnable *base)
    : runnable(base->getPos()), base(base) {}

  void prettyprint(ostream &out, Int indent) {
    absyntax::prettyname(out, "interactiveRunnable", indent);
    base->prettyprint(out, indent+1);
  }

  void trans(coenv &e) {
    base->interactiveTrans(e);
  }

  void transAsField(coenv &e, types::record *r) {
    // There is no interactiveTransAsField, as fields aren't declared at the top
    // level of the interactive prompt.
    base->transAsField(e, r);
  }
};

enum transMode {
  TRANS_INTERACTIVE,
  TRANS_NORMAL
};


// How to run a runnable in runnable-at-a-time mode.
bool runRunnable(runnable *r, coenv &e, istack &s, transMode tm=TRANS_NORMAL) {
  e.e.beginScope();

  lambda *codelet= tm==TRANS_INTERACTIVE ?
    interactiveRunnable(r).transAsCodelet(e) :
    r->transAsCodelet(e);
  em.sync();
  if(!em.errors()) {
    if(getSetting<bool>("translate")) print(cout,codelet->code);
    s.run(codelet);

    // Commits the changes made to the environment.
    e.e.collapseScope();
  } else {
    e.e.endScope(); // Remove any changes to the environment.

    // Should an interactive error hurt the status?
    em.statusError();

    return false;
  }
  return true;
}

void runAutoplain(coenv &e, istack &s) {
  absyntax::runnable *r=absyntax::autoplainRunnable();
  runRunnable(r,e,s);
}

// Abstract base class for the core object being run in line-at-a-time mode, it
// may be a block of code, file, or interactive prompt.
struct icore {
  virtual ~icore() {}

  virtual void doParse() = 0;
  virtual void doList() = 0;

public:

  // preRun and postRun are the optional activities that take place before and
  // after running the code specified.  They can be overridden by a derived
  // class that wishes different behaviour.
  virtual void preRun(coenv &e, istack &s) {
    if(getSetting<bool>("autoplain"))
      runAutoplain(e,s);
  }

  virtual void run(coenv &e, istack &s, transMode tm=TRANS_NORMAL) = 0;

  virtual void postRun(coenv &, istack &s) {
    run::exitFunction(&s);
  }

  virtual void doRun(bool purge=false, transMode tm=TRANS_NORMAL) {
    em.sync();
    if(em.errors())
      return;

    try {
      if(purge) run::purge();

      penv pe;
      env base_env(pe.ge());
      coder base_coder(nullPos, "icore::doRun");
      coenv e(base_coder,base_env);

      vm::interactiveStack s;
      s.setInitMap(pe.ge().getInitMap());
      s.setEnvironment(&e);

      preRun(e,s);

      if(purge) run::purge();

      // Now that everything is set up, run the core.
      run(e,s,tm);

      postRun(e,s);

    } catch(std::bad_alloc&) {
      outOfMemory();
    } catch(quit) {
      // Exception to quit running the current code. Nothing more to do.
    } catch(handled_error) {
      em.statusError();
    }

    run::cleanup();

    em.clear();
  }

  virtual void process(bool purge=false) {
    if (!interactive && getSetting<bool>("parseonly"))
      doParse();
    else if (getSetting<bool>("listvariables"))
      doList();
    else
      doRun(purge);
  }
};

// Abstract base class for one-time processing of an abstract syntax tree.
class itree : public icore {
  string name;

  block *cachedTree;
public:
  itree(string name="<unnamed>")
    : name(name), cachedTree(0) {}

  // Build the tree, possibly throwing a handled_error if it cannot be built.
  virtual block *buildTree() = 0;

  virtual block *getTree() {
    if (cachedTree==0) {
      try {
        cachedTree=buildTree();
      } catch(handled_error) {
        em.statusError();
        return 0;
      }
    }
    return cachedTree;
  }

  virtual string getName() {
    return name;
  }

  void doParse() {
    block *tree=getTree();
    em.sync();
    if(tree && !em.errors())
      tree->prettyprint(cout, 0);
  }

  void doList() {
    block *tree=getTree();
    if (tree) {
      penv pe;
      record *r=tree->transAsFile(pe.ge(), symbol::trans(getName()));
      r->e.list(r);
    }
  }

  void run(coenv &e, istack &s, transMode tm=TRANS_NORMAL) {
    block *tree=getTree();
    if (tree) {
      for(mem::list<runnable *>::iterator r=tree->stms.begin();
          r != tree->stms.end(); ++r) {
        processData().fileName=(*r)->getPos().filename();
        if(!em.errors() || getSetting<bool>("debug"))
          runRunnable(*r,e,s,tm);
      }
    }
  }

  void doExec(transMode tm=TRANS_NORMAL) {
    // Don't prepare an environment to run the code if there isn't any code.
    if (getTree())
      icore::doRun(false,tm);
  }
};

class icode : public itree {
  block *tree;

public:
  icode(block *tree, string name="<unnamed>")
    : itree(name), tree(tree) {}

  block *buildTree() {
    return tree;
  }
};

class istring : public itree {
  string str;

public:
  istring(const string& str, string name="<eval>")
    : itree(name), str(str) {}

  block *buildTree() {
    return parser::parseString(str, getName());
  }
};

void printGreeting(bool interactive) {
  if(!getSetting<bool>("quiet")) {
    cout << "Welcome to " << PROGRAM << " version " << REVISION;
    if(interactive)
      cout << " (to view the manual, type help)";
    cout << endl;
  }
}

class ifile : public itree {
  string filename;
  string outname;
  string outname_save;

public:
  ifile(const string& filename)
    : itree(filename),
      filename(filename),
      outname(stripDir(stripExt(string(filename == "-" ? settings::outname() : filename), suffix))) {}

  block *buildTree() {
    return !filename.empty() ? parser::parseFile(filename,"Loading") : 0;
  }

  void preRun(coenv& e, istack& s) {
    outname_save=getSetting<string>("outname");
    if(stripDir(outname_save).empty())
      Setting("outname")=outname_save+outname;

    itree::preRun(e, s);
  }

  void postRun(coenv &e, istack& s) {
    itree::postRun(e, s);

    Setting("outname")=outname_save;
  }

  void process(bool purge=false) {
    if(verbose > 1) printGreeting(false);
    try {
      init();
    } catch(handled_error) {
    }

    if (verbose >= 1)
      cout << "Processing " << outname << endl;

    try {
      icore::process(purge);
    }
    catch(handled_error) {
      em.statusError();
    }
  }
};

// Add a semi-colon terminator, if one is not there.
string terminateLine(const string line) {
  return (!line.empty() && *(line.rbegin())!=';') ? (line+";") : line;
}

// cleanLine changes a C++ style comment (//) into a C-style comment (/* */) so
// that comments don't absorb subsequent lines of code when multiline input is
// collapsed to a single line in the history.
//
// ex.  if (x==1) // test x
//        x=2;
// becomes
//      if (x==1) /* test x */ x=2    (all on one line)
//
// cleanLine is a mess because we have to check that the // is not in a string
// or c-style comment, which entails re-inventing much of the lexer.  The
// routine handles most cases, but multiline strings lose their newlines when
// recorded in the history.
typedef string::size_type size_type;
const size_type npos=string::npos;

inline size_type min(size_type a, size_type b) {
  return a < b ? a : b;
}

// If start is an offset somewhere within a string, this returns the first
// offset outside of the string. sym is the character used to start the string,
// ' or ".
size_type endOfString(const char sym, const string line, size_type start) {
  size_type endString=line.find(sym, start);
  if (endString == npos)
    return npos;

  size_type escapeInString=min(line.find(string("\\")+sym, start),
                               line.find("\\\\", start));
  if (endString < escapeInString)
    return endString+1;
  else
    return endOfString(sym, line, escapeInString+2);
}

// If start is an offset somewhere within a C-style comment, this returns the
// first offset outside of the comment.
size_type endOfComment(const string line, size_type start) {
  size_type endComment=line.find("*/", start);
  if (endComment == npos)
    return npos;
  else
    return endComment+2;
}

// Find the start of a string literal in the line.
size_type stringPos(const string line, size_type start) {
  if (start == npos)
    return npos;

  size_type pos=line.find_first_of("\'\"", start);
  if (pos == npos)
    return npos;

  // Skip over comments /*  */ and ignore anything after //
  size_type startComment=line.find("/*", start);
  size_type startLineComment=line.find("//", start);

  if (min(startComment,startLineComment) < pos)
    return stringPos(line,
                     startComment < startLineComment ?
                     endOfComment(line, startComment+2) :
                     npos);
  else
    // Nothing to skip over - the symbol actually starts a string.
    return pos;
}

// A multiline string should retain its newlines when collapsed to a single
// line.  This converts
//   'hello
//   there'
// to
//   'hello\nthere'
// and
//   "hello
//   there"
// to
//   "hello" '\n' "there"
// If the line doesn't end mid-string, this adds a space to the end to preserve
// whitespace, since it is the last function to touch the line.
string endString(const string line, size_type start) {
  assert(start!=npos);

  size_type pos=stringPos(line, start);

  if (pos==npos)
    // String ends in normal code.
    return line+" ";
  else {
    char sym=line[pos];
    size_type eos=endOfString(sym, line, pos+1);
    if (eos==npos) {
      // Line ends while in a string, attach a newline symbol.
      switch (line[pos]) {
        case '\'':
          return line+"\\n";
        case '\"':
          return line+"\" \'\\n\' \"";
        default:
          assert(False);
          return line;
      }
    }
    else {
      return endString(line, eos+1);
    }
  }
}

// Find the first // that isn't in a C-style comment or a string.
size_type slashPos(const string line, size_type start) {
  if (start == npos)
    return npos;

  size_type pos=line.find("//", start);
  if (pos == npos)
    return npos;

  // Skip over comments /*  */ and strings both "   " and '   '
  size_type startComment=line.find("/*", start);
  size_type startString=line.find_first_of("\'\"", start);

  if (min(startComment,startString) < pos)
    return slashPos(line,
                    startComment < startString ?
                    endOfComment(line, startComment+2) :
                    endOfString(line[startString], line, startString+1));
  else
    // Nothing to skip over - the // actually starts a comment.
    return pos;
}

string endCommentOrString(const string line) {
  size_type pos=slashPos(line, 0);
  if (pos == npos)
    return endString(line, 0);
  else {
    string sub=line;

    // Replace the first // by /*
    sub[pos+1]='*';

    // Replace any */ in the comment by *!
    while ((pos = line.find("*/", pos+2)) != npos)
      sub[pos+1]='!';

    // Tack on a */ at the end.
    sub.append(" */ ");
    return sub;
  }
}

bool isSlashed(const string line) {
  // NOTE: This doesn't fully handle escaped slashed in a string literal.
  unsigned n=line.size();
  return n > 0 ? line[line.size()-1] == '\\' : false;
}
string deslash(const string line) {
  return isSlashed(line) ? line.substr(0,line.size()-1) : line;
}

// This transforms a line in to the history, so that when more code is added
// after it, the code behaves the same as if there was a newline between the
// two lines.  This involves changing // style comments to /* */ style comments,
// and adding explicit newlines to multiline strings.
string cleanLine(const string line) {
  // First remove a trailing slash, if there is one.
  return endCommentOrString(deslash(line));
}

class iprompt : public icore {
  // Flag that is set to false to signal the prompt to exit.
  bool running;

  // Flag that is set to restart the main loop once it has exited.
  bool restart;

  // Code ran at start-up.
  string startline;

  void postRun(coenv &, istack &) {
  }

  // Commands are chopped into the starting word and the rest of the line.
  struct commandLine {
    string line;
    string word;
    string rest;

    commandLine(string line) : line(line) {
      string::iterator c=line.begin();

      // Skip leading whitespace
      while (c != line.end() && isspace(*c))
        ++c;

      // Only handle identifiers starting with a letter.
      if (c != line.end() && isalpha(*c)) {
        // Store the command name.
        while (c != line.end() && (isalnum(*c) || *c=='_')) {
          word.push_back(*c);
          ++c;
        }
      }

      // Copy the rest to rest.
      while (c != line.end()) {
        rest.push_back(*c);
        ++c;
      }

#if 0
      cerr << "line: " << line << endl;
      cerr << "word: " << word << endl;
      cerr << "rest: " << rest << endl;
      cerr << "simple: " << simple() << endl;
#endif
    }

    // Simple commands have at most spaces or semicolons after the command word.
    bool simple() {
      for (string::iterator c=rest.begin(); c != rest.end(); ++c)
        if (!isspace(*c) && *c != ';')
          return false;
      return true;
    }
  };


  // The interactive prompt has special functions that cannot be implemented as
  // normal functions.  These special funtions take a commandLine as an argument
  // and return true if they can handle the command.  If false is returned, the
  // line is treated as a normal line of code.
  // commands is a map of command names to methods which implement the commands.
  typedef bool (iprompt::*command)(coenv &, istack &, commandLine);
  typedef mem::map<CONST string, command> commandMap;
  commandMap commands;

  bool exit(coenv &, istack &, commandLine cl) {
    if (cl.simple()) {
      // Don't store exit commands in the history file.
      interact::deleteLastLine();
      running=false;
      return true;
    }
    else return false;
  }

  bool q(coenv &e, istack &s, commandLine cl) {
    if(e.e.ve.getType(symbol::trans("q"))) return false;
    return exit(e,s,cl);
  }

  bool reset(coenv &, istack &, commandLine cl) {
    if (cl.simple()) {
      running=false;
      restart=true;
      startline="";
      run::purge();
      return true;
    }
    else return false;
  }

  bool help(coenv &, istack &, commandLine cl) {
    if (cl.simple()) {
      popupHelp();
      return true;
    }
    else return false;
  }

  bool erase(coenv &e, istack &s, commandLine cl) {
    if (cl.simple()) {
      runLine(e,s,"erase();");
      return true;
    }
    else return false;
  }

  bool input(coenv &, istack &, commandLine cl) {
    running=false;
    restart=true;
    startline="include "+cl.rest;
    return true;
  }

  void initCommands() {
#define ADDCOMMAND(name, func)                  \
    commands[#name]=&iprompt::func

    // keywords.pl looks for ADDCOMMAND to identify special commands in the
    // auto-completion.
    ADDCOMMAND(quit,exit);
    ADDCOMMAND(q,q);
    ADDCOMMAND(exit,exit);
    ADDCOMMAND(reset,reset);
    ADDCOMMAND(erase,erase);
    ADDCOMMAND(help,help);
    ADDCOMMAND(input,input);

#undef ADDCOMMAND
  }

  bool handleCommand(coenv &e, istack &s, string line) {
    commandLine cl(line);
    if (cl.word != "") {
      commandMap::iterator p=commands.find(cl.word);
      if (p != commands.end()) {
        // Execute the command.
        command &com=p->second;
        return (this->*com)(e,s,cl);
      }
      else
        return false;
    }
    else
      return false;
  }

  void addToHistory(string line) {
    interact::addToHistory(line);
  }

  void addToLastLine(string line) {
    // Here we clean a line at the last possible point, when we know that more
    // code is going to be appended to it.
    string last=interact::getLastHistoryLine();
    interact::setLastHistoryLine(cleanLine(last)+line);
  }

  void terminateLastHistoryLine() {
    string last=interact::getLastHistoryLine();
    interact::setLastHistoryLine(terminateLine(last));
  }

  // Get input from the interactive prompt.  Such input may be over several
  // user-typed lines if he/she ends a line a with backslash to continue input
  // on the next line.  If continuation is true, the input started on a previous
  // line and is being continued (either because of a backslash or the parser
  // detecting it in multiline mode).
  string getline(bool continuation) {
    string prompt;
    if(!getSetting<bool>("xasy"))
      prompt=getSetting<string>(continuation ? "prompt2" : "prompt");
    string line=interact::simpleline(prompt);

    if (continuation)
      addToLastLine(line);
    else
      addToHistory(line);

    // If the line ends in a slash, get more input.
    return isSlashed(line) ? line+"\n"+getline(true) :
      line;
  }

  // Continue taking input for a line until it properly parses, or a syntax
  // error occurs.  Returns the parsed code on success, and throws a
  // handled_error exception on failure.
  block *parseExtendableLine(string line) {
    block *code=parser::parseString(line, "-", true);
    if (code) {
      return code;
    } else {
      string nextline=getline(true);
      return parseExtendableLine(line+"\n"+nextline);
    }
  }

  // Continue taking input until a termination command is received from xasy.
  block *parseXasyLine(string line) {

#ifdef __MSDOS__
    const string EOT="\x04\r\n";
#else
    const string EOT="\x04\n";
#endif
    string s;
    while((s=getline(true)) != EOT)
      line += s;
    return parser::parseString(line, "-", true);
  }

  void runLine(coenv &e, istack &s, string line) {
    try {
      if(getSetting<bool>("multiline")) {
        block *code=parseExtendableLine(line);

        icode i(code);
        i.run(e,s,TRANS_INTERACTIVE);
      } else if(getSetting<bool>("xasy")) {
        block *code=parseXasyLine(line);

        icode i(code);
        i.run(e,s,TRANS_INTERACTIVE);
      } else {
        // Add a semi-colon to the end of the line if one is not there.  Do this
        // to the history as well, so the transcript can be run as regular asy
        // code.  This also makes the history work correctly if the multiline
        // setting is changed within an interactive session.
        // It is added to the history at the last possible moment to avoid
        // tampering with other features, such as using a slash to extend a
        // line.
        terminateLastHistoryLine();
        istring i(terminateLine(line), "-");
        i.run(e,s,TRANS_INTERACTIVE);
      }

      run::updateFunction(&s);
      uptodate=false;

    } catch(handled_error) {
      vm::indebugger=false;
    } catch(interrupted&) {
      // Turn off the interrupted flag.
      em.Interrupt(false);
      uptodate=true;
      cout << endl;
    } catch(quit&) {
    }

    // Ignore errors from this line when trying to run subsequent lines.
    em.clear();
  }

  void runStartCode(coenv &e, istack &s) {
    if (!startline.empty())
      runLine(e, s, startline);
  }

public:
  iprompt() : running(false), restart(false), startline("") {
    initCommands();
  }

  void doParse() {}

  void doList() {}

  void run(coenv &e, istack &s, transMode=TRANS_NORMAL) {
    running=true;
    interact::setCompleter(new trans::envCompleter(e.e));

    runStartCode(e, s);

    while (running) {
      // Read a line from the prompt.
      string line=getline(false);
      // Check if it is a special command.
      if (handleCommand(e,s,line))
        continue;
      else
        runLine(e, s, line);
    }
  }

  void process(bool purge=false) {
    printGreeting(true);
    interact::init_interactive();
    try {
      setPath("",true);
    } catch(handled_error) {
    }

    do {
      try {
        init(false);
        restart=false;
        icore::process();
      } catch(interrupted&) {
        em.Interrupt(false);
        restart=true;
#if defined(MIKTEX)
      } catch(eof_exception&) {
#else
      } catch(eof&) {
#endif
        restart=false;
      }
    } while(restart);

    interact::cleanup_interactive();
  }
};

void processCode(absyntax::block *code) {
  icode(code).process();
}
void processFile(const string& filename, bool purge) {
  ifile(filename).process(purge);
}
void processPrompt() {
  iprompt().process();
}

void runCode(absyntax::block *code) {
  icode(code).doExec();
}
void runString(const string& s, bool interactiveWrite) {
  istring(s).doExec(interactiveWrite ? TRANS_INTERACTIVE : TRANS_NORMAL);
}
void runFile(const string& filename) {
  ifile(filename).doExec();
}
void runPrompt() {
  iprompt().doRun();
}

void runCodeEmbedded(absyntax::block *code, trans::coenv &e, istack &s) {
  icode(code).run(e,s);
}
void runStringEmbedded(const string& str, trans::coenv &e, istack &s) {
  istring(str).run(e,s);
}
void runPromptEmbedded(trans::coenv &e, istack &s) {
  iprompt().run(e,s);
}

void doUnrestrictedList() {
  penv pe;
  env base_env(pe.ge());
  coder base_coder(nullPos, "doUnrestictedList");
  coenv e(base_coder,base_env);

  if (getSetting<bool>("autoplain"))
    absyntax::autoplainRunnable()->trans(e);

  e.e.list(0);
}

// Environment class used by external programs linking to the shared library.
class fullenv : public gc {
  penv pe;
  env base_env;
  coder base_coder;
  coenv e;

  vm::interactiveStack s;

public:
  fullenv() :
    pe(), base_env(pe.ge()), base_coder(nullPos, "fullenv"),
    e(base_coder, base_env), s()
  {
    s.setInitMap(pe.ge().getInitMap());
    s.setEnvironment(&e);

    // TODO: Add way to not run autoplain.
    runAutoplain(e, s);
  }

  coenv &getCoenv()
  {
    return e;
  }

  void runRunnable(runnable *r)
  {
    assert(!em.errors()); // TODO: Decide how to handle prior errors.

    try {
      { withProcessData token(pe.pd());
        ::runRunnable(r, e, s, TRANS_INTERACTIVE);
      }
    } catch(std::bad_alloc&) {
      // TODO: give calling application useful message.
      cerr << "out of memory" << endl;
    } catch (quit) {
      // I'm not sure whether this counts as successfully running the code or
      // not.
      cerr << "quit exception" << endl;
    } catch (handled_error) {
      // Normally, this is the result of an error that changes the return code
      // of the free-standing asymptote program.
      // An error should probably be reported to the application calling the
      // asymptote library.
      cerr << "handled error" << endl;
    }

    em.clear();
  }
};

fullenv &getFullEnv()
{
  static fullenv fe;
  return fe;
}

coenv &coenvInOngoingProcess()
{
  return getFullEnv().getCoenv();
}

void runInOngoingProcess(runnable *r)
{
  getFullEnv().runRunnable(r);
}
