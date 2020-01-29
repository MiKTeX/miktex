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
#include "transform.h"

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
  
  terminator<std::ofstream> ofile;
  terminator<std::fstream> ifile;
#ifdef HAVE_RPC_RPC_H
  terminator<xdr::ioxstream> ixfile;
  terminator<xdr::oxstream> oxfile;
#endif  
  
  processDataStruct() {
    atExitFunction=NULL;
    atUpdateFunction=NULL;
    atBreakpointFunction=NULL;
    defaultpen=camp::pen::initialpen();
    currentpen=camp::pen();
  }
  
};

processDataStruct &processData();

#endif
