/************
 *
 *   This file is part of the vector graphics language Asymptote
 *   Copyright (C) 2004 Andy Hammerlindl, John C. Bowman, Tom Prince
 *                 https://asymptote.sourceforge.io
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

#ifdef __CYGWIN__
#define _POSIX_C_SOURCE 200809L
#endif

#if defined(MIKTEX)
#include <miktex/asy-first.h>
#include <miktex/asy.h>
#endif
#include <iostream>
#include <cstdlib>
#include <cerrno>
#if !defined(MIKTEX_WINDOWS)
#include <sys/wait.h>
#endif
#include <sys/types.h>

#define GC_PTHREAD_SIGMASK_NEEDED

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
#include "fileio.h"

#ifdef HAVE_LSP
#include "lspserv.h"
#endif

#include "stack.h"

using namespace settings;

using interact::interactive;

namespace gl {
extern bool glexit;
}

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

void setsignal(void (*handler)(int))
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

bool hangup=false;
void hangup_handler(int sig)
{
  hangup=true;
}

struct Args
{
  int argc;
  char **argv;
  Args(int argc, char **argv) : argc(argc), argv(argv) {}
};

int returnCode()
{
  return em.processStatus() || interact::interactive ? 0 : 1;
}

void *asymain(void *A)
{
  setsignal(signalHandler);
  Args *args=(Args *) A;
  fpu_trap(trap());

  if(interactive) {
    Signal(SIGINT,interruptHandler);
#ifdef HAVE_LSP
    if (getSetting<bool>("lsp")) {
      AsymptoteLsp::LspLog log;
      auto jsonHandler=std::make_shared<lsp::ProtocolJsonHandler>();
      auto endpoint=std::make_shared<GenericEndpoint>(log);

      unique_ptr<AsymptoteLsp::AsymptoteLspServer> asylsp;

      if(getSetting<string>("lspport") != "") {
        asylsp=make_unique<AsymptoteLsp::TCPAsymptoteLSPServer>(
          (std::string)getSetting<string>("lsphost").c_str(),
          (std::string)getSetting<string>("lspport").c_str(),
          jsonHandler, endpoint, log);
      } else {
        asylsp=make_unique<AsymptoteLsp::AsymptoteLspServer>(jsonHandler,
                                                                  endpoint,
                                                                  log);
      }
      asylsp->start();
    } else
#endif
      processPrompt();
  } else if (getSetting<bool>("listvariables") && numArgs()==0) {
    try {
      doUnrestrictedList();
    } catch(handled_error const&) {
      em.statusError();
    }
  } else {
    int n=numArgs();
    if(n == 0) {
      int inpipe=intcast(settings::getSetting<Int>("inpipe"));
      if(inpipe >= 0) {
#if !defined(MIKTEX_WINDOWS)
        // TODO
        Signal(SIGHUP,hangup_handler);
#endif
        camp::openpipeout();
        fprintf(camp::pipeout,"\n");
        fflush(camp::pipeout);
      }
      while(true) {
        processFile("-",true);
        try {
          setOptions(args->argc,args->argv);
        } catch(handled_error const&) {
          em.statusError();
        }
        if(inpipe < 0) break;
      }
    } else {
      for(int ind=0; ind < n; ind++) {
        string name=(getArg(ind));
        if(name == stripExt(name)+".v3d") {
          interact::uptodate=false;
          runString("import v3d; defaultfilename=\""+stripDir(name)+
                    "\"; importv3d(\""+name+"\");");
        } else
          processFile(name,n > 1);
        try {
          if(ind < n-1)
            setOptions(args->argc,args->argv);
        } catch(handled_error const&) {
          em.statusError();
        }
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
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  if(gl::glthread) {
#ifdef __MSDOS__ // Signals are unreliable in MSWindows
    gl::glexit=true;
#else
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
    MiKTeX::Aymptote::exitRequested = true;
    gl::initSignal.notify_one();
#else
    pthread_kill(gl::mainthread,SIGURG);
    pthread_join(gl::mainthread,NULL);
#endif
#endif
  }
#endif
#endif
  exit(returnCode());
}
#if defined(MIKTEX)
void BackgroundThread(void* ptr)
{
  try
  {
    asymain(ptr);
  }
  catch (const std::exception& /*ex*/)
  {
    // MIKTEX-TODO
  }
  catch (int /*code*/)
  {
    // MIKTEX-TODO
  }
}
#endif

void exitHandler(int)
{
  exit(returnCode());
}

#if defined(MIKTEX)
int main(int argc, char** argv)
#else
int main(int argc, char *argv[])
#endif
{
#if defined(MIKTEX)
  MIKTEX_SESSION()->PushAppName("asymptote");
  MiKTeX::Util::PathName plainAsy;
  if (!MIKTEX_SESSION()->FindFile("plain.asy", MiKTeX::Core::FileType::PROGRAMTEXTFILE, plainAsy))
  {
    cerr
      << "warning: package 'asymptote' is not installed\n"
      << "         run 'mpm --install asymptote' to install it\n";
  }
#endif
#ifdef HAVE_LIBGSL
  unsetenv("GSL_RNG_SEED");
  unsetenv("GSL_RNG_TYPE");
#endif
  setsignal(signalHandler);

  try {
    setOptions(argc,argv);
  } catch(handled_error const&) {
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
#ifndef HAVE_LIBOSMESA
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
          Signal(SIGURG,exitHandler);
          camp::glrenderWrapper();
          gl::initialize=true;
        }
      } else gl::glthread=false;
    } catch(std::bad_alloc&) {
      outOfMemory();
    }
  }
#endif
#elif defined(MIKTEX)
  if (gl::glthread)
  {
    std::thread(BackgroundThread, &args).detach();
    while (true)
    {
      MiKTeX::Aymptote::RequestHandler();
      camp::glrenderWrapper();
      gl::initialize = true;
    }
  }
#endif
  gl::glthread=false;
#endif
  asymain(&args);
#if defined(MIKTEX)
  // MIKTEX-UNEXPECTED: unreachable code
  return 0;
#endif
}

#ifdef USEGC
GC_API void GC_CALL GC_throw_bad_alloc() {
  std::bad_alloc();
}
#endif
#if defined(MIKTEX)
#include "types.h"
const types::signature::OPEN_t types::signature::OPEN;
#endif

