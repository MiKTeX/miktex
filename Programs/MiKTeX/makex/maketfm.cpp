/* maketfm.cpp:

   Copyright (C) 1998-2021 Christian Schenk

   This file is part of MiKTeX MakeTFM.

   MiKTeX MakeTFM is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX MakeTFM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX MakeTFM; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "config.h"

#include "maketfm-version.h"

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/TemporaryDirectory>

#include "MakeUtility.h"

using namespace std;

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("maketfm"));

class MakeTfm :
  public MakeUtility
{
public:
  void Run(int argc, const char** argv) override;

private:
  void CreateDestinationDirectory() override;

private:
  void Usage() override;

private:
#if defined(_MSC_VER)
#  pragma warning (disable: 4065)
#endif
  BEGIN_OPTION_MAP(MakeTfm)
  END_OPTION_MAP();
#if defined(_MSC_VER)
#  pragma warning (default: 4065)
#endif

private:
  bool MakeFromHBF(const char* name, const PathName& workingDirectory);
};

void MakeTfm::Usage()
{
  OUT__
    << T_("Usage:") << " " << Utils::GetExeName() << " " << T_("[OPTION]... name") << "\n"
    << "\n"
    << T_("This program makes a TeX font metric file.") << "\n"
    << "\n"
    << T_("NAME is the name of the font, such as 'cmr10'.") << "\n"
    << "\n"
    << T_("Options:") << "\n"
    << "--debug, -d " << T_("Print debugging information.") << "\n"
    << "--disable-installer " << T_("Disable the package installer.") << "\n"
    << "--enable-installer " << T_("Enable the package installer.") << "\n"
    << "--help, -h " << T_("Print this help screen and exit.") << "\n"
    << "--print-only, -n " << T_("Print what commands would be executed.") << "\n"
    << "--verbose, -v " << T_("Print information on what is being done.") << "\n"
    << "--version, -V " << T_("Print the version number and exit.") << "\n";
}

namespace {
  const struct option aLongOptions[] =
  {
    COMMON_OPTIONS,
    {nullptr, no_argument, nullptr, 0}
  };
}

void MakeTfm::CreateDestinationDirectory()
{
  // get basic font information (supplier & typeface)
  string supplier;
  string typeface;
  if (!session->GetFontInfo(name, supplier, typeface, nullptr))
  {
    supplier = "public";
    typeface = "misc";
  }

  // get destination path template
  string templ1 = session->GetConfigValue(MIKTEX_CONFIG_SECTION_MAKETFM, MIKTEX_CONFIG_VALUE_DESTDIR).GetString();

  string templ2;
  for (const char* lpsz = templ1.c_str(); *lpsz != 0; ++lpsz)
  {
    if (lpsz[0] == '%')
    {
      switch (lpsz[1])
      {
      case '%':
        templ2 += '%';
        break;
      case 'R':
        templ2 += "%R";
        break;
      case 's':
        templ2 += supplier;
        break;
      case 't':
        templ2 += typeface;
        break;
      }
      ++lpsz;
    }
    else
    {
      templ2 += *lpsz;
    }
  }

  // create destination directory
  destinationDirectory = CreateDirectoryFromTemplate(templ2);
}

bool MakeTfm::MakeFromHBF(const char* name, const PathName& workingDirectory)
{
  // run hbf2gf to make a .pl file
  vector<string> arguments;
  if (debug)
  {
    arguments.push_back("-q");
  }
  arguments.push_back("-g");
  arguments.push_back(name);
  arguments.push_back(std::to_string(300));
  if (!RunProcess(MIKTEX_HBF2GF_EXE, arguments, workingDirectory))
  {
    return false;
  }

  // run PLtoTF
  arguments.clear();
  arguments.push_back(PathName(name).AppendExtension(".pl").ToString());
  arguments.push_back(PathName(name).AppendExtension(".tfm").ToString());
  if (!RunProcess(MIKTEX_PLTOTF_EXE, arguments, workingDirectory))
  {
    FatalError(fmt::format(T_("PLtoTF failed on {0}."), Q_(name)));
  }

  return true;
}

void MakeTfm::Run(int argc, const char** argv)
{
  // get command line options and name
  int optionIndex = 0;
  GetOptions(argc, argv, aLongOptions, optionIndex);
  if (argc - optionIndex != 1)
  {
    FatalError(T_("Invalid command-line."));
  }
  name = argv[optionIndex];

  // create a temporary working directory
  unique_ptr<TemporaryDirectory> wrkDir = TemporaryDirectory::Create();

  // create destination directory
  CreateDestinationDirectory();

  // make TFM file name
  PathName pathTFMName(name + ".tfm");

  // make fully qualified destination path name
  PathName pathDest(destinationDirectory, pathTFMName);

  // quit, if destination font file already exists
  if (File::Exists(pathDest))
  {
    Message(fmt::format(T_("TFM file {0} already exists."), Q_(pathDest)));
    return;
  }

  // try to create the METAFONT if it does not exist
  PathName mfPath;
  bool done = false;
  if (!session->FindFile(name, FileType::MF, mfPath))
  {
    vector<string> arguments;
    if (debug)
    {
      arguments.push_back("--debug");
    }
    if (verbose)
    {
      arguments.push_back("--verbose");
    }
    if (printOnly)
    {
      arguments.push_back("--print-only");
    }
    arguments.push_back(name);
    if (!RunProcess(MIKTEX_MAKEMF_EXE, arguments, wrkDir->GetPathName()))
    {
      // no METAFONT input file; try to make from HBF file
      if (!MakeFromHBF(name.c_str(), wrkDir->GetPathName()))
      {
        FatalError(fmt::format(T_("No creation rule for font {0}."), Q_(name)));
      }
      done = true;
    }
  }

  // run METAFONT, if an appropriate input file exists
  if (!done)
  {
    Verbose(fmt::format(T_("Making {0} from {1}..."), Q_(pathTFMName), Q_(mfPath)));
    if (!RunMETAFONT(name.c_str(), nullptr, nullptr, wrkDir->GetPathName()))
    {
      FatalError(fmt::format(T_("METAFONT failed on {0}."), Q_(name)));
    }
    done = true;
  }

  // install TFM font file in destination directory
  Install(wrkDir->GetPathName() / pathTFMName, pathDest);
}

#if defined(_UNICODE)
#  define MAIN wmain
#  define MAINCHAR wchar_t
#else
#  define MAIN main
#  define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR** argv)
{
  MakeTfm app;
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<char*> newargv;
    newargv.reserve(argc + 1);
    for (int idx = 0; idx < argc; ++idx)
    {
#if defined(_UNICODE)
      utf8args.push_back(StringUtil::WideCharToUTF8(argv[idx]));
#elif defined(MIKTEX_WINDOWS)
      utf8args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
#else
      utf8args.push_back(argv[idx]);
#endif
      // FIXME: eliminate const cast
      newargv.push_back(const_cast<char*>(utf8args[idx].c_str()));
    }
    newargv.push_back(nullptr);
    app.Init(Session::InitInfo(newargv[0]), newargv);
    app.Run(newargv.size() - 1, const_cast<const char**>(&newargv[0]));
    app.Finalize2(0);
    logger = nullptr;
    return 0;
  }
  catch (const MiKTeXException& ex)
  {
    app.Sorry("maketfm", ex);
    logger = nullptr;
    ex.Save();
    return 1;
  }
  catch (const exception& ex)
  {
    app.Sorry("maketfm", ex);
    logger = nullptr;
    return 1;
  }
  catch (int exitCode)
  {
    logger = nullptr;
    return exitCode;
  }
}
