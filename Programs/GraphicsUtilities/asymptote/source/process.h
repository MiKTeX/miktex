/*****
 * process.h
 * Andy Hammerlindl 2006/08/19
 *
 * Handles processing blocks of code (including files, strings, and the
 * interactive prompt, for listing and parse-only modes as well as actually
 * running it.
 *****/

#ifndef PROCESS_H
#define PROCESS_H


#include "common.h"
#include "stm.h"
#include "stack.h"
#include "pipestream.h"
#include "callable.h"
#include "pen.h"
#include "dec.h"
#include "transform.h"
#include "parser.h"

#ifdef HAVE_RPC_RPC_H
#include "xstream.h"
#endif

// Process the code respecting the parseonly and listvariables flags of
// settings.
void processCode(absyntax::block *code);
void processFile(const string& filename, bool purge=false);
void processPrompt();

// Run the code in its own environment.
void runCode(absyntax::block *code);
void runString(const string& s, bool interactiveWrite=false);
void runFile(const string& filename);
void runPrompt();

// Run the code in a given run-time environment.
typedef vm::interactiveStack istack;
void runCodeEmbedded(absyntax::block *code, trans::coenv &e, istack &s);
void runStringEmbedded(const string& str, trans::coenv &e, istack &s);
void runPromptEmbedded(trans::coenv &e, istack &s);

// Basic listing.
void doUnrestrictedList();

template<class T>
class terminator {
public:
  typedef mem::vector<T *> Pointer;
  Pointer pointer;

  // Return first available index
  size_t available() {
    size_t index=0;
    for(typename Pointer::iterator p=pointer.begin(); p != pointer.end(); ++p) {
      if(*p == NULL) {return index;}
      ++index;
    }
    pointer.push_back(NULL);
    return index;
  }

  size_t add(T *p) {
    size_t index=available();
    pointer[index]=p;
    return index;
  }

  void remove(size_t index) {
    pointer[index]=NULL;
  }

  ~terminator() {
    for(typename Pointer::iterator p=pointer.begin(); p != pointer.end(); ++p) {
      if(*p != NULL) {
        (*p)->~T();
        (*p)=NULL;
      }
    }
  }
};

class texstream : public iopipestream {
public:
  ~texstream();
};

typedef std::pair<size_t,size_t> linecolumn;
typedef mem::map<CONST linecolumn,string> xkey_t;
typedef mem::deque<camp::transform> xtransform_t;
typedef mem::map<CONST string,xtransform_t> xmap_t;

struct processDataStruct {
  texstream tex; // Bi-directional pipe to latex (to find label bbox)
  mem::list<string> TeXpipepreamble;
  mem::list<string> TeXpreamble;
  vm::callable *atExitFunction;
  vm::callable *atUpdateFunction;
  vm::callable *atBreakpointFunction;
  camp::pen defaultpen;
  camp::pen currentpen;

  // For xasy:
  string fileName;
  position topPos;
  string KEY;
  xkey_t xkey;
  xmap_t xmap;
  unsigned int xmapCount;

  terminator<std::ofstream> ofile;
  terminator<std::fstream> ifile;
#ifdef HAVE_RPC_RPC_H
  terminator<xdr::ixstream> ixfile;
  terminator<xdr::oxstream> oxfile;
#endif

  processDataStruct() : xmapCount(0) {
    atExitFunction=NULL;
    atUpdateFunction=NULL;
    atBreakpointFunction=NULL;
    defaultpen=camp::pen::initialpen();
    currentpen=camp::pen();
  }

};

enum transMode {
  TRANS_INTERACTIVE,
  TRANS_NORMAL
};

// Abstract base class for the core object being run in line-at-a-time mode, it
// may be a block of code, file, or interactive prompt.
struct icore {
  virtual ~icore() {}
  virtual void doParse() = 0;
  virtual void doList() = 0;

public:
  virtual void preRun(trans::coenv &e, istack &s);
  virtual void run(trans::coenv &e, istack &s, transMode tm=TRANS_NORMAL) = 0;
  virtual void postRun(trans::coenv &, istack &s);
  virtual void doRun(bool purge=false, transMode tm=TRANS_NORMAL);
  virtual void process(bool purge=false);
};

// Abstract base class for one-time processing of an abstract syntax tree.
class itree : public icore {
  string name;
  absyntax::block *cachedTree;
public:
  itree(string name="<unnamed>");
  virtual absyntax::block *buildTree() = 0;

  // Build the tree, possibly throwing a handled_error if it cannot be built.
  virtual absyntax::block *getTree();
  virtual string getName();

  void doParse();
  void doList();
  void run(trans::coenv &e, istack &s, transMode tm=TRANS_NORMAL);
  void doExec(transMode tm=TRANS_NORMAL);
};

class ifile : public itree {
  string filename;
  string outname;
  string outname_save;

public:
  ifile(const string& filename);
  absyntax::block *buildTree();

  void preRun(trans::coenv& e, istack& s);
  void postRun(trans::coenv &e, istack& s);
  void process(bool purge=false);
};

class icode : public itree {
  absyntax::block *tree;

public:
  icode(absyntax::block *tree, string name="<unnamed>")
          : itree(name), tree(tree) {}

  absyntax::block *buildTree() {
    return tree;
  }
};

class istring : public itree {
  string str;

public:
  istring(const string& str, string name="<eval>")
          : itree(name), str(str) {}

  absyntax::block *buildTree() {
    return parser::parseString(str, getName());
  }
};

processDataStruct &processData();

#endif
