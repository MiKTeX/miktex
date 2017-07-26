/* mkocp.cpp:

   Copyright (C) 2002-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/Process>
#include <miktex/Core/Session>

using namespace MiKTeX::Core;
using namespace std;

#define T_(x) MIKTEXTEXT(x)

#if defined(MIKTEX_UNICODE)
#  define tcout wcout
#  define tcerr wcerr
#else
#  define tcout cout
#  define tcerr cerr
#endif

/* _________________________________________________________________________

   main

   <original_shell_script>
   #!/bin/sh
   # Initial definition.  Will leave the .ocp file in the local directory.
   (
   otp2ocp `basename $1 .ocp` || exit 1
   ) 1>&2 </dev/null
   echo `basename $1 .ocp`.ocp
   </original_shell_script>
   _________________________________________________________________________ */

int main(int argc, char ** argv)
{
  try
  {
    shared_ptr<Session> pSession;
    Session::InitInfo initInfo;
    initInfo.SetProgramInvocationName(argv[0]);
    pSession = Session::Create(initInfo);
    if (argc != 2)
    {
      tcerr << T_("Usage: mkocp OCPFILE") << endl;
      throw 1;
    }
    PathName otp2ocp;
    if (!pSession->FindFile(T_("otp2ocp"), FileType::EXE, otp2ocp))
    {
      tcerr << T_("mkocp: otp2ocp executable could not be found.") << endl;
      throw 1;
    }
    PathName argv1 = argv[1];
    PathName outputName;
    if (PathName::Compare(argv1.GetExtension(), ".ocp") == 0)
    {
      outputName = argv1.GetFileNameWithoutExtension();
    }
    else
    {
      outputName = argv1.GetFileName();
    }
    Process::Run(otp2ocp, { otp2ocp.GetFileNameWithoutExtension().ToString(), outputName.ToString() });
    tcout << outputName.ToString() << ".ocp" << endl;
    return 0;
  }
  catch (const MiKTeXException & e)
  {
    Utils::PrintException(e);
    return 1;
  }
  catch (const exception & e)
  {
    Utils::PrintException(e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
