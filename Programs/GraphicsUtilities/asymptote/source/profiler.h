/*****
 * profiler.h
 * Andy Hammerlindl 2010/07/24
 *
 * Profiler for the execution of the virtual machine bytecode.
 *****/

#ifndef PROFILER_H
#define PROFILER_H

#include <sys/time.h>
#if defined(MIKTEX_WINDOWS)
#include <WinSock2.h>
#endif

#include <iostream>

#include "inst.h"

namespace vm {

#ifdef DEBUG_BLTIN
string lookupBltin(bltin b);
#endif

inline position positionFromLambda(lambda *func) {
  if (func == 0)
    return position();

  program& code = *func->code;

  // Check for empty program.
  if (code.begin() == code.end())
    return position();

  return code.begin()->pos;
}

inline void printNameFromLambda(ostream& out, lambda *func) {
  if (!func) {
    out << "<top level>";
    return;
  }

#ifdef DEBUG_FRAME
  string name = func->name;
#else
  string name = "";
#endif

  // If unnamed, use the pointer address.
  if (name.empty())
    out << func;
  else
    out << name;

  out << " at ";
  positionFromLambda(func).printTerse(out);
}

inline void printNameFromBltin(ostream& out, bltin b) {
#ifdef DEBUG_BLTIN
  string name = lookupBltin(b);
#else
  string name = "";
#endif

  if (!name.empty())
    out << name << " ";
  out << "(builtin at " << (void *)b << ")";
}

class profiler : public gc {
  // To do call graph analysis, each call stack that occurs in practice is
  // represented by a node.  For instance, if f and g are functions, then
  //   f -> g -> g
  // is represented by a node and
  //   g -> f -> g
  // is represented by a different one.
  struct node {
    // The top-level function of the call stack.  It is either an asymptote
    // function with a given lambda, or a builtin function, with a given
    // bltin.
    lambda *func;
    bltin cfunc;

    // The number of times the top-level function has been called resulting in
    // this specific call stack.
    int calls;

    // The number of bytecode instructions executed with this exact call stack.
    // It does not include time spent in called function.
    int instructions;

    // Number of instructions spent in this function or its children.  This is
    // computed by computeTotals.
    int instTotal;

    // The number of real-time nanoseconds spent in this node.  WARNING: May
    // be wildly inaccurate.
    long long nsecs;

    // Total including children.
    long long nsecsTotal;

    // Call stacks resulting from calls during this call stack.
    mem::vector<node> children;

    node()
      : func(0), cfunc(0), calls(0),
        instructions(0), instTotal(0),
        nsecs(0), nsecsTotal(0) {}

    node(lambda *func)
      : func(func), cfunc(0), calls(0),
        instructions(0), instTotal(0),
        nsecs(0), nsecsTotal(0) {}

    node(bltin b)
      : func(0), cfunc(b), calls(0),
        instructions(0), instTotal(0),
        nsecs(0), nsecsTotal(0) {}

    // Return the call stack resulting from a call to func when this call
    // stack is current.
    node *getChild(lambda *func) {
      size_t n = children.size();
      for (size_t i = 0; i < n; ++i)
        if (children[i].func == func)
          return &children[i];

      // Not found, create a new one.
      children.push_back(node(func));
      return &children.back();
    }
    node *getChild(bltin func) {
      size_t n = children.size();
      for (size_t i = 0; i < n; ++i)
        if (children[i].cfunc == func)
          return &children[i];

      // Not found, create a new one.
      children.push_back(node(func));
      return &children.back();
    }

    void computeTotals() {
      instTotal = instructions;
      nsecsTotal = nsecs;
      size_t n = children.size();
      for (size_t i = 0; i < n; ++i) {
        children[i].computeTotals();
        instTotal += children[i].instTotal;
        nsecsTotal += children[i].nsecsTotal;
      }
    }



    void pydump(ostream& out) {
#ifdef DEBUG_FRAME
      string name = func ? func->name : "<top level>";
#else
      string name = "";
#endif

      out << "dict(\n"
          << "    name = '" << name << " " << func << "',\n"
          << "    pos = '" << positionFromLambda(func) << "',\n"
          << "    calls = " << calls << ",\n"
          << "    instructions = " << instructions << ",\n"
          << "    nsecs = " << nsecs << ",\n"
          << "    children = [\n";

      size_t n = children.size();
      for (size_t i = 0; i < n; ++i) {
        children[i].pydump(out);
        out << ",\n";
      }

      out << "    ])\n";
    }
  };

  // An empty call stack.
  node emptynode;

  // All of the callstacks.
  std::stack<node *> callstack;

  node &topnode() {
    return *callstack.top();
  }

  // Arc representing one function calling another.  Used only for building
  // the output for kcachegrind.
  struct arc : public gc {
    int calls;
    int instTotal;
    long long nsecsTotal;

    arc() : calls(0), instTotal(0), nsecsTotal(0) {}

    void add(node& n) {
      calls += n.calls;
      instTotal += n.instTotal;
      nsecsTotal += n.nsecsTotal;
    }
  };

  // Representing one function and its calls to other functions.
  struct fun : public gc {
    int instructions;
    long long nsecs;
    mem::map<lambda *, arc> arcs;
    mem::map<bltin, arc> carcs;

    fun() : instructions(0), nsecs(0) {}

    void addChildTime(node& n) {
      if (n.cfunc)
        carcs[n.cfunc].add(n);
      else
        arcs[n.func].add(n);
    }

    void analyse(node& n) {
      instructions += n.instructions;
      nsecs += n.nsecs;
      size_t numChildren = n.children.size();
      for (size_t i = 0; i < numChildren; ++i)
        addChildTime(n.children[i]);
    }

    void dump(ostream& out) {
      // The unused line number needed by kcachegrind.
      static const string POS = "1";

      out << POS << " " << instructions << " " << nsecs << "\n";
      for (mem::map<lambda *, arc>::iterator i = arcs.begin();
           i != arcs.end();
           ++i)
        {
          lambda *l = i->first;
          arc& a = i->second;

          out << "cfl=" << positionFromLambda(l) << "\n";

          out << "cfn=";
          printNameFromLambda(out, l);
          out << "\n";

          out << "calls=" << a.calls << " " << POS << "\n";
          out << POS << " " << a.instTotal << " " << a.nsecsTotal << "\n";
        }
      for (mem::map<bltin, arc>::iterator i = carcs.begin();
           i != carcs.end();
           ++i)
        {
          bltin b = i->first;
          arc& a = i->second;

          out << "cfl=C++ code" << endl;

          out << "cfn=";
          printNameFromBltin(out, b);
          out << "\n";

          out << "calls=" << a.calls << " " << POS << "\n";
          out << POS << " " << a.instTotal << " " << a.nsecsTotal << "\n";
        }
    }
  };

  // The data for each asymptote function.
  mem::map<lambda *, fun> funs;

  // The data for each C++ function.
  mem::map<bltin, fun> cfuns;

  void analyseNode(node& n) {
    fun& f = n.cfunc ? cfuns[n.cfunc] :
      funs[n.func];

    f.analyse(n);

    size_t numChildren = n.children.size();
    for (size_t i = 0; i < numChildren; ++i)
      analyseNode(n.children[i]);
  }

  // Convert data in the tree of callstack nodes into data for each function.
  void analyseData() {
    emptynode.computeTotals();
    analyseNode(emptynode);
  }


  // Timing data.
  struct timeval timestamp;

  void startLap() {
    gettimeofday(&timestamp, 0);
  }

  long long timeAndResetLap() {
    struct timeval now;
    gettimeofday(&now, 0);
    long long nsecs = 1000000000LL * (now.tv_sec - timestamp.tv_sec) +
      1000LL * (now.tv_usec - timestamp.tv_usec);
    timestamp = now;
    return nsecs;
  }

  // Called whenever the stack is about to change, in order to record the time
  // duration for the current node.
  void recordTime() {
    topnode().nsecs += timeAndResetLap();
  }

public:
  profiler();

  void beginFunction(lambda *func);
  void endFunction(lambda *func);
  void beginFunction(bltin func);
  void endFunction(bltin func);
  void recordInstruction();

  // TODO: Add position, type of instruction info to profiling.

  // Dump all of the data out in a format that can be read into Python.
  void pydump(ostream &out);

  // Dump all of the data in a format for kcachegrind.
  void dump(ostream& out);

};

inline profiler::profiler()
  : emptynode()
{
  callstack.push(&emptynode);
  startLap();
}

inline void profiler::beginFunction(lambda *func) {
  assert(func);
  assert(!callstack.empty());

  recordTime();

  callstack.push(topnode().getChild(func));
  ++topnode().calls;
}

inline void profiler::endFunction(lambda *func) {
  assert(func);
  assert(!callstack.empty());
  assert(topnode().func == func);

  recordTime();

  callstack.pop();
}

inline void profiler::beginFunction(bltin cfunc) {
  assert(cfunc);
  assert(!callstack.empty());

  recordTime();

  callstack.push(topnode().getChild(cfunc));
  ++topnode().calls;
}

inline void profiler::endFunction(bltin cfunc) {
  assert(cfunc);
  assert(!callstack.empty());
  assert(topnode().cfunc == cfunc);

  recordTime();

  callstack.pop();
}

inline void profiler::recordInstruction() {
  assert(!callstack.empty());
  ++topnode().instructions;
}

inline void profiler::pydump(ostream& out) {
  out << "profile = ";
  emptynode.pydump(out);
}

inline void profiler::dump(ostream& out) {
  analyseData();

  out << "events: Instructions Nanoseconds\n";

  for (mem::map<lambda *, fun>::iterator i = funs.begin();
       i != funs.end();
       ++i)
    {
      lambda *l = i->first;
      fun& f = i->second;

      out << "fl=" << positionFromLambda(l) << "\n";

      out << "fn=";
      printNameFromLambda(out, l);
      out << "\n";

      f.dump(out);
    }
  for (mem::map<bltin, fun>::iterator i = cfuns.begin();
       i != cfuns.end();
       ++i)
    {
      bltin b = i->first;
      fun& f = i->second;

      out << "fl=C++ code\n";

      out << "fn=";
      printNameFromBltin(out, b);
      out << "\n";

      f.dump(out);
    }
}


} // namespace vm

#endif // PROFILER_H
