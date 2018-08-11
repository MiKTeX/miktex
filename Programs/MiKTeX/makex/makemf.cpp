/* makemf.cpp:

   Copyright (C) 1998-2018 Christian Schenk

   This file is part of MiKTeX MakeMF.

   MiKTeX MakeMF is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX MakeMF is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX MakeMF; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

/* Some algorithms are borrowed from the web2c mktex* shell scripts. */

#include "makemf-version.h"
#include "MakeUtility.h"

log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("makemf"));

const char* const lhpref[] = {
  "la",
  "lb",
  "lc",
  "lh",
  "ll",
  "rx",
  "wn",
  nullptr,
};

const char* const cspref[] = {
  "cs",
  "ics",
  "lcs",
  nullptr,
};

const char* const cbpref[] = {
  "glic",
  "glii",
  "glin",
  "glio",
  "gliu",
  "gljc",
  "gljn",
  "gljo",
  "glmc",
  "glmi",
  "glmn",
  "glmo",
  "glmu",
  "gltc",
  "gltn",
  "glto",
  "glwc",
  "glwi",
  "glwn",
  "glwo",
  "glwu",
  "glxc",
  "glxi",
  "glxn",
  "glxo",
  "glxu",
  "gmmn",
  "gmmo",
  "gmtr",
  "gmxn",
  "gmxo",
  "gomc",
  "gomi",
  "gomn",
  "gomo",
  "gomu",
  "goxc",
  "goxi",
  "goxn",
  "goxo",
  "goxu",
  "grbl",
  "grmc",
  "grmi",
  "grml",
  "grmn",
  "grmo",
  "grmu",
  "grxc",
  "grxi",
  "grxl",
  "grxn",
  "grxo",
  "grxu",
  "gsmc",
  "gsmi",
  "gsmn",
  "gsmo",
  "gsmu",
  "gsxc",
  "gsxi",
  "gsxn",
  "gsxo",
  "gsxu",
  "gttc",
  "gtti",
  "gttn",
  "gtto",
  "gttu",
  nullptr
};

bool HasPrefix(const string& s, const char* const prefixes[])
{
  for (size_t i = 0; prefixes[i] != 0; ++i)
  {
    if (HasPrefix(s, prefixes[i]))
    {
      return true;
    }
  }
  return false;
}

class MakeMf :
  public MakeUtility
{
public:
  virtual void Run(int argc, const char** argv);

private:
  virtual void CreateDestinationDirectory();

private:
  virtual void Usage();

private:
  BEGIN_OPTION_MAP(MakeMf)
    OPTION_ENTRY_TRUE('c', toStdout)
  END_OPTION_MAP();

private:
  bool toStdout = false;

private:
  string supplier;

private:
  string typeface;
};

void MakeMf::Usage()
{
  OUT__
    << T_("Usage:") << " " << Utils::GetExeName() << " " << T_("[OPTION]... name") << endl
    << endl
    << T_("This program makes a METAFONT file.") << endl
    << endl
    << T_("NAME is the name of the font, such as 'cmr10'.") << endl
    << endl
    << T_("Options:") << endl
    << "--debug, -d " << T_("Print debugging information.") << endl
    << "--disable-installer " << T_("Disable the package installer.") << endl
    << "--enable-installer " << T_("Enable the package installer.") << endl
    << "--help, -h " << T_("Print this help screen and exit.") << endl
    << "--print-only, -n " << T_("Print what commands would be executed.") << endl
    << "--stdout, -c " << T_("Write MF file on standard output.") << endl
    << "--verbose, -v " << T_("Print information on what is being done.") << endl
    << "--version, -V " << T_("Print the version number and exit.") << endl;

}

namespace {
  const struct option aLongOptions[] =
  {
    COMMON_OPTIONS,
    "stdout",       no_argument, nullptr,      'c',
    nullptr,        no_argument, nullptr,      0,
  };
}

void MakeMf::CreateDestinationDirectory()
{
  string templ;
  templ = "%R";
  templ += PathName::DirectoryDelimiter;
  templ += T_("fonts");
  templ += PathName::DirectoryDelimiter;
  templ += T_("source");
  if (!supplier.empty())
  {
    templ += PathName::DirectoryDelimiter;
    templ += supplier;
  }
  templ += PathName::DirectoryDelimiter;
  templ += typeface;
  destinationDirectory = CreateDirectoryFromTemplate(templ);
}

void MakeMf::Run(int argc, const char** argv)
{
  // get options and file name
  int optionIndex = 0;
  GetOptions(argc, argv, aLongOptions, optionIndex);
  if (argc - optionIndex != 1)
  {
    FatalError(T_("Invalid command-line."));
  }
  name = argv[optionIndex];

  // derive TeX font name from name (e.g., "ecbi3583.mf" =>
  // "ecbi3583")
  string texFontname = PathName(name).GetFileNameWithoutExtension().ToString();

  // derive driver name from the TeX font name (e.g., "ecbi3583" =>
  // "ecbi"
  string driverName;
  session->SplitFontPath(texFontname, nullptr, nullptr, nullptr, &driverName, nullptr);

  // find the driver file
  {
    // try a sauterized driver first
    string strSauterDriverName = "b-";
    strSauterDriverName += driverName;
    PathName driverPath;
    if (!session->FindFile(strSauterDriverName, FileType::MF, driverPath))
    {
      // lh fonts get special treatment
      if (HasPrefix(driverName, lhpref))
      {
        string strLHDriverName;
        strLHDriverName += driverName[0];
        strLHDriverName += driverName[1];
        strLHDriverName += "codes";
        if (!session->FindFile(strLHDriverName, FileType::MF, driverPath))
        {
          FatalError(T_("The %s source file could not be found."), strLHDriverName.c_str());
        }
      }
      else if (HasPrefix(driverName, cspref))
      {
        if (!session->FindFile("cscode", FileType::MF, driverPath))
        {
          FatalError(T_("The cscode source file could not be found."));
        }
      }
      else if (HasPrefix(driverName, cbpref))
      {
        if (!session->FindFile("cbgreek", FileType::MF, driverPath))
        {
          FatalError(T_("The cbgreek source file could not be found."));
        }
      }
      else if (!session->FindFile(driverName, FileType::MF, driverPath))
      {
        FatalError(T_("The %s source file could not be found."), driverName.c_str());
      }
    }
  }

  // get information about the font
  double true_pt_size;
  if (!session->GetFontInfo(texFontname, supplier, typeface, &true_pt_size))
  {
    FatalError(T_("No info available for %s."), texFontname.c_str());
  }

  // create destination directory
  CreateDestinationDirectory();

  // open the output stream
  FILE* stream = nullptr;
  PathName pathDest;
  AutoFILE autoClose;
  if (toStdout || printOnly)
  {
    stream = stdout;
  }
  else
  {
    // make fully qualified destination file name
    pathDest = destinationDirectory / texFontname;
    pathDest.AppendExtension(".mf");
    Verbose(T_("Writing on %s..."), Q_(pathDest));
    if (!printOnly)
    {
      stream = File::Open(pathDest, FileMode::Create, FileAccess::Write);
      autoClose.Attach(stream);
    }
  }

  PrintOnly("cat <<__END__ > %s", Q_(pathDest));

  if (HasPrefix(texFontname, "ec") || HasPrefix(texFontname, "tc"))
  {
    fprintf(stream, "if unknown exbase: input exbase fi;\n");
    fprintf(stream, "gensize:=%0.2f;\n", true_pt_size);
    fprintf(stream, "generate %s;\n", driverName.c_str());
  }
  else if (HasPrefix(texFontname, "dc"))
  {
    fprintf(stream, "if unknown dxbase: input dxbase fi;\n");
    fprintf(stream, "gensize:=%f;\n", true_pt_size);
    fprintf(stream, "generate %s;\n", driverName.c_str());
  }
  else if (HasPrefix(texFontname, lhpref))
  {
    fprintf(stream, "input fikparm;\n");
  }
  else if (HasPrefix(texFontname, cspref))
  {
    fprintf(stream, "input cscode\nuse_driver;\n");
  }
  else if (HasPrefix(texFontname, cbpref))
  {
    fprintf(stream, "input cbgreek;\n");
  }
  else
  {
    fprintf(stream, "design_size:=%f;\n", true_pt_size);
    fprintf(stream, "input b-%s;\n", driverName.c_str());
  }

  PrintOnly("__END__");

  // close output stream
  if (stream != stdout)
  {
    autoClose.Reset();

    // add to file name database
    if (!Fndb::FileExists(pathDest))
    {
      Fndb::Add(pathDest);
    }
  }
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
  MakeMf app;
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
    Application::Sorry("makemf", ex);
    logger = nullptr;
    ex.Save();
    return 1;
  }
  catch (const exception& ex)
  {
    Application::Sorry("makemf", ex);
    logger = nullptr;
    return 1;
  }
  catch (int exitCode)
  {
    logger = nullptr;
    return exitCode;
  }
}
