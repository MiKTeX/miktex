/* dvipdft.cpp: run dvipdfm-gswin32-dvipdfm

   Copyright (C) 2000-2017 Christian Schenk

   This file is part of dvipdm.

   dvipdfm is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   dvipdfm is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with dvipdfm; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include <cstdlib>
#include <cstdarg>
#include <string>

#if defined(MIKTEX_WINDOWS)
#  include <direct.h>
#  include <io.h>
#endif

#include <miktex/App/Application>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/Process>
#include <miktex/Core/TemporaryDirectory>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX;
using namespace std;
using namespace std::string_literals;

void FatalError(const char* lpszFormat, ...)
{
  fprintf(stderr, "%s: ", Utils::GetExeName().c_str());
  va_list marker;
  va_start(marker, lpszFormat);
  vfprintf(stderr, lpszFormat, marker);
  va_end(marker);
  fputc('\n', stderr);
  throw 1;
}

void FatalFileError(const char* lpszPath)
{
  fprintf(stderr, "%s: ", Utils::GetExeName().c_str());
  perror(lpszPath);
  throw 1;
}

void BadUsage()
{
  FatalError(MIKTEXTEXT("No DVI file name specified."));
}

void dvipdft(int argc, char** argv)
{
  shared_ptr<Session> session = Session::Get();

  // must have at least one argument: the DVI file name
  if (argc == 1)
  {
    BadUsage();
  }

  PathName gsExe = session->GetGhostscript(nullptr);

  PathName dvipdfmExe;
  if (!session->FindFile("dvipdfm", FileType::EXE, dvipdfmExe))
  {
    FatalError(MIKTEXTEXT("The Dvipdfm executable could not be found."));
  }

  // create a temporary directory
  unique_ptr<TemporaryDirectory> tempDir;

  vector<string> arguments{ dvipdfmExe.GetFileNameWithoutExtension().ToString() };
  PathName fileNoExt;

  // loop over all arguments except the last one
  for (int i = 1; i < argc - 1; ++i)
  {
    arguments.push_back(argv[i]);
    if (strcmp(argv[i], "-o") == 0 && i + 1 < argc - 1)
    {
      ++i;
      arguments.push_back(argv[i]);
      fileNoExt = argv[i];
      fileNoExt.SetExtension(nullptr);
    }
  }

  const char* lpszUserFilename = argv[argc - 1];
  if (fileNoExt.Empty())
  {
    fileNoExt = lpszUserFilename;
    fileNoExt.SetExtension(nullptr);
  }

  // run dvipdfm with the fastest options for the first pass
  arguments.push_back("-e");
  arguments.push_back("-z"s + "0");
  arguments.push_back(lpszUserFilename);
  int exitCode = 0;
  if (!Process::Run(dvipdfmExe, arguments, nullptr, &exitCode, nullptr))
  {
    FatalError(MIKTEXTEXT("%s could not be started."), dvipdfmExe.GetData());
  }
  if (exitCode != 0)
  {
    throw exitCode;
  }

  // run GhostScript to create thumbnails
  PathName outFileTemplate = tempDir->GetPathName() / fileNoExt;
  outFileTemplate.AppendExtension(".%d");
  arguments = { gsExe.GetFileNameWithoutExtension().ToString() };
  arguments.push_back("-r"s + "10");
  arguments.push_back("-dNOPAUSE");
  arguments.push_back("-dBATCH");
  arguments.push_back("-sDEVICE:"s + "png256");
  arguments.push_back("-sOutputFile:"s + outFileTemplate.ToString());
  arguments.push_back(PathName(fileNoExt).AppendExtension(".pdf").ToString());
  if (!Process::Run(gsExe, arguments, nullptr, &exitCode, nullptr))
  {
    FatalError(MIKTEXTEXT("%s could not be started."), gsExe.GetData());
  }
  if (exitCode != 0)
  {
    throw exitCode;
  }

  // run dvipdfm with the users specified options for the last pass
  arguments = { dvipdfmExe.GetFileNameWithoutExtension().ToString() };
  arguments.push_back("-dt");
  for (int idx = 1; idx < argc; ++idx)
  {
    arguments.push_back(argv[idx]);
  }
  printf("%s\n", CommandLineBuilder(arguments).ToString().c_str());
  Utils::SetEnvironmentString("MIKTEX_TEMP", tempDir->GetPathName().ToString());
  if (!Process::Run(dvipdfmExe, arguments, nullptr, &exitCode, nullptr))
  {
    FatalError(MIKTEXTEXT("%s could not be started."), dvipdfmExe.GetData());
  }
  if (exitCode != 0)
  {
    throw exitCode;
  }
}

// TODO: _UNICODE
int main(int argc, char** argv)
{
  try
  {
    MiKTeX::App::Application app;
    vector<char*> newargv{ argv, argv + argc + 1 };
    app.Init(newargv);
    dvipdft(newargv.size() - 1, &newargv[0]);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException& e)
  {
    Utils::PrintException(e);
    return 1;
  }
  catch (const exception& e)
  {
    Utils::PrintException(e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
