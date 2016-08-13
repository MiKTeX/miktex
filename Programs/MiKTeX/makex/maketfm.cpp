/* maketfm.cpp:

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

class MakeTfm : public MakeUtility
{
public:
  virtual void Run(int argc, const char ** argv);

private:
  virtual void CreateDestinationDirectory();

private:
  virtual void Usage();

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
  bool MakeFromHBF(const char * lpszName, const PathName & workingDirectory);
};

void MakeTfm::Usage()
{
  OUT__
    << T_("Usage:") << " " << Utils::GetExeName() << " " << T_("[OPTION]... name") << endl
    << endl
    << T_("This program makes a TeX font metric file.") << endl
    << endl
    << T_("NAME is the name of the font, such as 'cmr10'.") << endl
    << endl
    << T_("Options:") << endl
    << "--debug, -d " << T_("Print debugging information.") << endl
    << "--disable-installer " << T_("Disable the package installer.") << endl
    << "--enable-installer " << T_("Enable the package installer.") << endl
    << "--help, -h " << T_("Print this help screen and exit.") << endl
    << "--print-only, -n " << T_("Print what commands would be executed.") << endl
    << "--verbose, -v " << T_("Print information on what is being done.") << endl
    << "--version, -V " << T_("Print the version number and exit.") << endl;
}

namespace {
  const struct option aLongOptions[] =
  {
    COMMON_OPTIONS,
    nullptr, no_argument, nullptr, 0,
  };
}

void MakeTfm::CreateDestinationDirectory()
{
  // get basic font information (supplier & typeface)
  PathName supplier;
  PathName typeface;
  if (!session->GetFontInfo(name.c_str(), supplier.GetData(), typeface.GetData(), nullptr))
  {
    supplier = "public";
    typeface = "misc";
  }

  // get destination path template
  string templ1 = session->GetConfigValue(MIKTEX_REGKEY_MAKETFM, MIKTEX_REGVAL_DESTDIR, "%R/fonts/tfm/%s/%t");

  string templ2;
  for (const char * lpsz = templ1.c_str(); *lpsz != 0; ++lpsz)
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
        templ2 += supplier.Get();
        break;
      case 't':
        templ2 += typeface.Get();
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

bool MakeTfm::MakeFromHBF(const char * lpszName, const PathName & workingDirectory)
{
  // run hbf2gf to make a .pl file
  CommandLineBuilder arguments;
  if (debug)
  {
    arguments.AppendOption("-q");
  }
  arguments.AppendOption("-g");
  arguments.AppendArgument(lpszName);
  arguments.AppendArgument(std::to_string(300));
  if (!RunProcess(MIKTEX_HBF2GF_EXE, arguments.ToString(), workingDirectory))
  {
    return false;
  }

  // run PLtoTF
  arguments.Clear();
  arguments.AppendArgument(PathName(0, lpszName, ".pl"));
  arguments.AppendArgument(PathName(0, lpszName, ".tfm"));
  if (!RunProcess(MIKTEX_PLTOTF_EXE, arguments.ToString(), workingDirectory))
  {
    FatalError(T_("PLtoTF failed on %s."), Q_(lpszName));
  }

  return true;
}

void MakeTfm::Run(int argc, const char ** argv)
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
  PathName pathTFMName(0, name, ".tfm");

  // make fully qualified destination path name
  PathName pathDest(destinationDirectory, pathTFMName, 0);

  // quit, if destination font file already exists
  if (File::Exists(pathDest))
  {
    Message(T_("TFM file %s already exists."), Q_(pathDest));
    return;
  }

  // try to create the METAFONT if it does not exist
  PathName mfPath;
  bool done = false;
  if (!session->FindFile(name.c_str(), FileType::MF, mfPath))
  {
    CommandLineBuilder arguments;
    if (debug)
    {
      arguments.AppendOption("--debug");
    }
    if (verbose)
    {
      arguments.AppendOption("--verbose");
    }
    if (printOnly)
    {
      arguments.AppendOption("--print-only");
    }
    arguments.AppendArgument(name);
    if (!RunProcess(MIKTEX_MAKEMF_EXE, arguments.ToString(), wrkDir->GetPathName()))
    {
      // no METAFONT input file; try to make from HBF file
      if (!MakeFromHBF(name.c_str(), wrkDir->GetPathName()))
      {
        FatalError(T_("No creation rule for font %s."), Q_(name));
      }
      done = true;
    }
  }

  // run METAFONT, if an appropriate input file exists
  if (!done)
  {
    Verbose(T_("Making %s from %s..."), Q_(pathTFMName), Q_(mfPath));
    if (!RunMETAFONT(name.c_str(), nullptr, nullptr, wrkDir->GetPathName()))
    {
      FatalError(T_("METAFONT failed on %s."), Q_(name));
    }
    done = true;
  }

  // install TFM font file in destination directory
  Install(wrkDir->GetPathName() / pathTFMName, pathDest);
}

MKTEXAPI(maketfm) (int argc, const char ** argv)
{
  try
  {
    MakeTfm app;
    app.Init(Session::InitInfo(argv[0]));
    app.Run(argc, argv);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException & e)
  {
    Application::Sorry("maketfm", e);
    return 1;
  }
  catch (const exception & e)
  {
    Application::Sorry("maketfm", e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
