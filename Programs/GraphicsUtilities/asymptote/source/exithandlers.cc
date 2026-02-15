/**
 * @file exithandlers.cc
 * @brief Definitions for exit handlers
 */

#include "exithandlers.h"

#include <csignal>

#include "common.h"
#include "util.h"
#include "interact.h"
#include "errormsg.h"
#include "vm.h"

#ifdef HAVE_LIBFFTW3
#include "fftw++.h"
#endif

void interruptHandler(int)
{
#ifdef HAVE_LIBFFTW3
  fftwpp::saveWisdom();
#endif
  em.Interrupt(true);
}

bool hangup=false;

int returnCode()
{
  return em.processStatus() || interact::interactive ? 0 : 1;
}

void exitHandler(int)
{
#if defined(HAVE_READLINE) && defined(HAVE_LIBCURSES)
  rl_cleanup_after_signal();
#endif
  exit(returnCode());
}

void signalHandler(int)
{
  // Print the position and trust the shell to print an error message.
  em.runtime(vm::getPos());

#if !defined(_WIN32)
  Signal(SIGBUS,SIG_DFL);
#endif
  Signal(SIGFPE,SIG_DFL);
}

void hangup_handler(int sig)
{
  hangup=true;
}
