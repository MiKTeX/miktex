/* touch.cpp:

   Copyright (C) 2008-2016 Christian Schenk

   This file is part of miktex-touch.

   miktex-touch is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   miktex-touch is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.
   
   You should have received a copy of the GNU General Public License
   along with miktex-touch; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Session>
#include <miktex/Util/StringUtil>
#include <miktex/Wrappers/PoptWrapper>

#include "touch-version.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <locale>

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#define T_(x) MIKTEXTEXT(x)

enum Option
{
  OPT_AAA = 1000,
  OPT_TIME_STAMP,
  OPT_VERSION,
};

const struct poptOption aoption[] = {
  {
    "time-stamp", 0, POPT_ARG_STRING, nullptr, OPT_TIME_STAMP,
    T_("Use TIMESTAMP (elapsed seconds since 1970) instead of current time."),
    T_("TIMESTAMP")
  },
  {
    "version", 0, POPT_ARG_NONE, nullptr, OPT_VERSION,
    T_("Show version information and exit."),
    nullptr
  },
  POPT_AUTOHELP
  POPT_TABLEEND
};

void FatalError(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  cerr << Utils::GetExeName() << ": "
    << StringUtil::FormatString(lpszFormat, arglist)
    << endl;
  va_end(arglist);
  throw 1;
}

void Main(int argc, const char ** argv)
{
  int option;
  time_t timeStamp = time(nullptr);
  PoptWrapper popt(argc, argv, aoption);
  popt.SetOtherOptionHelp(T_("[OPTION...] [FILE...]"));
  while ((option = popt.GetNextOpt()) >= 0)
  {
    switch (option)
    {
    case OPT_TIME_STAMP:
      timeStamp = static_cast<time_t>(std::stoll(popt.GetOptArg()));
      break;
    case OPT_VERSION:
      cout
        << Utils::MakeProgramVersionString(Utils::GetExeName().c_str(), VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0))
        << "Copyright (C) 2008-2016 Christian Schenk" << endl
        << "This is free software; see the source for copying conditions.  There is NO" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
      return;
    }
  }
  if (option != -1)
  {
    string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
    msg += ": ";
    msg += popt.Strerror(option);
    FatalError("%s", msg.c_str());
  }
  vector<string> leftovers = popt.GetLeftovers();
  if (leftovers.empty())
  {
    FatalError(T_("no file name arguments"));
  }
  for (const string & fileName : leftovers)
  {
    if (Directory::Exists(fileName))
    {
      Directory::SetTimes(fileName, timeStamp, timeStamp, timeStamp);
    }
    else
    {
      File::SetTimes(fileName, timeStamp, timeStamp, timeStamp);
    }
  }
}

int main(int argc, const char ** argv)
{
  int exitCode;
  try
  {    
    Session::InitInfo initInfo;
    initInfo.SetProgramInvocationName(argv[0]);
    shared_ptr<Session> session = Session::Create(initInfo);
    Main(argc, argv);
    exitCode = 0;
  }
  catch (const MiKTeXException & e)
  {
    Utils::PrintException(e);
    exitCode = 1;
  }
  catch (int r)
  {
    exitCode = r;
  }
  return exitCode;
}
