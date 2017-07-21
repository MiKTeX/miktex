/************
*
*   This file is part of the vector graphics language Asymptote
*   Copyright (C) 2004 Andy Hammerlindl, John C. Bowman, Tom Prince
*                 http://asymptote.sourceforge.net
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*************/

#include <iostream>
#include <cstdlib>
#include <cerrno>
#if !defined(MIKTEX_WINDOWS)
#include <sys/wait.h>
#endif
#include <sys/types.h>

#include "common.h"
#if defined(MIKTEX)
#  include <miktex/Core/Session>
#endif

#ifdef HAVE_LIBSIGSEGV
#include <sigsegv.h>
#endif

#include "errormsg.h"
#include "fpu.h"
#include "settings.h"
#include "locate.h"
#include "interact.h"
#include "process.h"

#include "stack.h"

using namespace settings;

using interact::interactive;

namespace run {
void purge();
}
  
#ifdef PROFILE
namespace vm {
extern void dumpProfile();
};
#endif

#ifdef HAVE_LIBSIGSEGV
void stackoverflow_handler (int, stackoverflow_context_t)
{
  em.runtime(vm::getPos());
  cerr << "Stack overflow" << endl;
  abort();
}

int sigsegv_handler (void *, int emergency)
{
  if(!emergency) return 0; // Really a stack overflow
  em.runtime(vm::getPos());
#ifdef HAVE_GL
  if(gl::glthread)
    cerr << "Stack overflow or segmentation fault: rerun with -nothreads"
         << endl;
  else
#endif    
    cerr << "Segmentation fault" << endl;
  abort();
}
#endif 

void setsignal(RETSIGTYPE (*handler)(int))
{
#ifdef HAVE_LIBSIGSEGV
  char mystack[16384];
  stackoverflow_install_handler(&stackoverflow_handler,
                                mystack,sizeof (mystack));
  sigsegv_install_handler(&sigsegv_handler);
#endif
#if !defined(MIKTEX_WINDOWS)
  Signal(SIGBUS,handler);
  Signal(SIGFPE,handler);
#endif
}

void signalHandler(int)
{
  // Print the position and trust the shell to print an error message.
  em.runtime(vm::getPos());

#if !defined(MIKTEX_WINDOWS)
  Signal(SIGBUS,SIG_DFL);
  Signal(SIGFPE,SIG_DFL);
#endif
}

void interruptHandler(int)
{
  em.Interrupt(true);
}

struct Args 
{
  int argc;
  char **argv;
  Args(int argc, char **argv) : argc(argc), argv(argv) {}
};

#if defined(MIKTEX)
#define exit(x) throw x
#endif
void *asymain(void *A)
{
  setsignal(signalHandler);
  
  Args *args=(Args *) A;
  fpu_trap(trap());

  if(interactive) {
    Signal(SIGINT,interruptHandler);
    processPrompt();
  } else if (getSetting<bool>("listvariables") && numArgs()==0) {
    try {
      doUnrestrictedList();
    } catch(handled_error) {
      em.statusError();
    } 
  } else {
    int n=numArgs();
    if(n == 0) 
      processFile("-");
    else
      for(int ind=0; ind < n; ind++) {
        processFile(string(getArg(ind)),n > 1);
        try {
          if(ind < n-1)
            setOptions(args->argc,args->argv);
        } catch(handled_error) {
          em.statusError();
        } 
      }
  }

#ifdef PROFILE
  vm::dumpProfile();
#endif

  if(getSetting<bool>("wait")) {
#if defined(MIKTEX_WINDOWS)
    // MIKTEX-TODO
#else
    int status;
    while(wait(&status) > 0);
#endif
  }
#ifdef HAVE_GL
#ifdef HAVE_PTHREAD
  if(gl::glthread && !getSetting<bool>("offscreen")) {
    pthread_kill(gl::mainthread,SIGUSR2);
    pthread_join(gl::mainthread,NULL);
  }
#endif
#endif
  exit(em.processStatus() || interact::interactive ? 0 : 1);  
}

void exitHandler(int)
{
  exit(0);
}

#if defined(MIKTEX)
int main(int argc, char **argv)
#else
int main(int argc, char *argv[]) 
#endif
{
#if defined(MIKTEX)
  MiKTeX::Core::Session::Get()->PushBackAppName("asymptote");
#endif
#ifdef HAVE_LIBGSL  
  unsetenv("GSL_RNG_SEED");
  unsetenv("GSL_RNG_TYPE");
#endif  
  setsignal(signalHandler);

  try {
    setOptions(argc,argv);
  } catch(handled_error) {
    em.statusError();
  }
  
  Args args(argc,argv);
#ifdef HAVE_GL
#ifdef __APPLE__
  bool usethreads=true;
#else
  bool usethreads=view();
#endif  
  gl::glthread=usethreads ? getSetting<bool>("threads") : false;
#if HAVE_PTHREAD
  
  if(gl::glthread) {
    pthread_t thread;
    try {
      if(pthread_create(&thread,NULL,asymain,&args) == 0) {
        gl::mainthread=pthread_self();
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGCHLD);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
        while(true) {
          Signal(SIGUSR2,exitHandler);
          camp::glrenderWrapper();
          gl::initialize=true;
        }
      } else gl::glthread=false;
    } catch(std::bad_alloc&) {
      outOfMemory();
    }
  }
#endif
  gl::glthread=false;
#endif  
  asymain(&args);
#if defined(MIKTEX)
  return 0;
#endif
}
#if defined(MIKTEX)
#include "types.h"
const types::signature::OPEN_t types::signature::OPEN;
#endif
