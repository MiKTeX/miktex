/* makebase.cpp:

   Copyright (C) 1998-2016 Christian Schenk

   This file is part of the MiKTeX Maker Library.

   The MiKTeX Maker Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Maker Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Maker Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "internal.h"

enum {
  OPT_AAA = 1, OPT_DESTNAME, OPT_ENGINE_OPTION, OPT_NO_DUMP
};

class MakeBase : public MakeUtility
{
public:
  virtual void Run(int argc, const char ** argv);

private:
  virtual void Usage();

private:
  virtual void CreateDestinationDirectory();

private:
  BEGIN_OPTION_MAP(MakeBase)
    OPTION_ENTRY_SET(OPT_DESTNAME, destinationName)
    OPTION_ENTRY(OPT_ENGINE_OPTION, AppendEngineOption(lpszOptArg))
    OPTION_ENTRY_TRUE(OPT_NO_DUMP, noDumpPrimitive)
  END_OPTION_MAP();

private:
  void AppendEngineOption(const char * lpszOption)
  {
    engineOptions.Append(lpszOption);
  }

private:
  PathName destinationName;

private:
  bool noDumpPrimitive = false;

private:
  Argv engineOptions;
};

void MakeBase::Usage()
{
  OUT__
    << T_("Usage:") << " " << Utils::GetExeName() << " " << T_("[OPTION]... NAME") << endl
    << endl
    << T_("This program makes a METAFONT base file.") << endl
    << endl
    << T_("NAME is the name of the base file, such as 'mf'.") << endl
    << endl
    << T_("Options:") << endl
    << "--debug, -d " << T_("Print debugging information.") << endl
    << "--dest-name NAME " << T_("Destination file name.") << endl
    << "--disable-installer " << T_("Disable the package installer.") << endl
    << "--enable-installer " << T_("Enable the package installer.") << endl
    << "--engine-option=OPTION " << T_("Add an engine option.") << endl
    << "--help, -h " << T_("Print this help screen and exit.") << endl
    << "--no-dump " << T_("Don't issue the dump command.") << endl
    << "--print-only, -n " << T_("Print what commands would be executed.") << endl
    << "--verbose, -v " << T_("Print information on what is being done.") << endl
    << "--version, -V " << T_("Print the version number and exit.") << endl;
}

namespace
{
  const struct option aLongOptions[] =
  {
    COMMON_OPTIONS,
    "dest-name", required_argument, nullptr, OPT_DESTNAME,
    "engine-option", required_argument, nullptr, OPT_ENGINE_OPTION,
    "no-dump", no_argument, nullptr, OPT_NO_DUMP,
    nullptr, no_argument, nullptr, 0,
  };
}

void MakeBase::CreateDestinationDirectory()
{
  destinationDirectory = CreateDirectoryFromTemplate(session->GetConfigValue(MIKTEX_REGKEY_MAKEBASE, MIKTEX_REGVAL_DESTDIR, "%R/" MIKTEX_PATH_BASE_DIR));
}

void MakeBase::Run(int argc, const char ** argv)
{
  // get options and file name
  int optionIndex = 0;
  GetOptions(argc, argv, aLongOptions, optionIndex);
  if (argc - optionIndex != 1)
  {
    FatalError(T_("Invalid command-line."));
  }
  name = argv[optionIndex];

  if (destinationName.Empty())
  {
    destinationName = name;
    destinationName.SetExtension(0);
  }

  // create destination directory
  CreateDestinationDirectory();

  // make the base file name
  PathName baseFile(name);
  baseFile.SetExtension(".base");

  // make fully qualified destination file name
  PathName pathDest(destinationDirectory, destinationName, ".base");

  Verbose(T_("Creating the %s base file..."), Q_(destinationName));

  // create a temporary working directory
  unique_ptr<TemporaryDirectory> wrkDir = TemporaryDirectory::Create();

  // invoke METAFONT to make the base file
  CommandLineBuilder arguments;
  arguments.AppendOption("--initialize");
  arguments.AppendOption("--interaction=", "nonstopmode");
  arguments.AppendOption("--halt-on-error");
  switch (GetEnableInstaller())
  {
  case TriState::False:
    arguments.AppendOption("--disable-installer");
    break;
  case TriState::True:
    arguments.AppendOption("--enable-installer");
    break;
  default:
    break;
  }
  arguments.AppendArguments(engineOptions);
  if (!noDumpPrimitive)
  {
    arguments.AppendArgument(name + T_("; input modes; dump"));
  }
  else
  {
    arguments.AppendArgument(name);
  }
  if (!RunProcess(MIKTEX_MF_EXE, arguments.ToString(), wrkDir->GetPathName()))
  {
    FatalError(T_("METAFONT failed on %s."), Q_(name));
  }

  // install the result
  Install(wrkDir->GetPathName() / baseFile, pathDest);
}

MKTEXAPI(makebase) (int argc, const char ** argv)
{
  try
  {
    MakeBase app;
    app.Init(Session::InitInfo(argv[0]));
    app.Run(argc, argv);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException & e)
  {
    Application::Sorry("makebase", e);
    return 1;
  }
  catch (const exception & e)
  {
    Application::Sorry("makebase", e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
