/* makemf.cpp:

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

   /* Some algorithms are borrowed from the web2c mktex* shell scripts. */

#include "internal.h"

const char * const lhpref[] = {
  "la",
  "lb",
  "lc",
  "lh",
  "ll",
  "rx",
  "wn",
  nullptr,
};

const char * const cspref[] = {
  "cs",
  "ics",
  "lcs",
  nullptr,
};

const char * const cbpref[] = {
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

bool HasPrefix(const char * lpsz, const char * const lpszPrefixes[])
{
  for (size_t i = 0; lpszPrefixes[i] != 0; ++i)
  {
    if (HasPrefix(lpsz, lpszPrefixes[i]))
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
  virtual void Run(int argc, const char ** argv);

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
  PathName supplier;

private:
  PathName typeface;
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
  if (!supplier.Empty())
  {
    templ += PathName::DirectoryDelimiter;
    templ += supplier.Get();
  }
  templ += PathName::DirectoryDelimiter;
  templ += typeface.Get();
  destinationDirectory = CreateDirectoryFromTemplate(templ);
}

void MakeMf::Run(int argc, const char ** argv)
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
  char szTeXFontname[BufferSizes::MaxPath];
  PathName::Split(name.c_str(), nullptr, 0, szTeXFontname, BufferSizes::MaxPath, nullptr, 0);

  // derive driver name from the TeX font name (e.g., "ecbi3583" =>
  // "ecbi"
  char szDriverName[BufferSizes::MaxPath];
  session->SplitFontPath(szTeXFontname, nullptr, nullptr, nullptr, szDriverName, nullptr);

  // find the driver file
  {
    // try a sauterized driver first
    string strSauterDriverName = "b-";
    strSauterDriverName += szDriverName;
    PathName driverPath;
    if (!session->FindFile(strSauterDriverName.c_str(), FileType::MF, driverPath))
    {
      // lh fonts get special treatment
      if (HasPrefix(szDriverName, lhpref))
      {
        string strLHDriverName;
        strLHDriverName += szDriverName[0];
        strLHDriverName += szDriverName[1];
        strLHDriverName += "codes";
        if (!session->FindFile(strLHDriverName.c_str(), FileType::MF, driverPath))
        {
          FatalError(T_("The %s source file could not be found."), strLHDriverName.c_str());
        }
      }
      else if (HasPrefix(szDriverName, cspref))
      {
        if (!session->FindFile("cscode", FileType::MF, driverPath))
        {
          FatalError(T_("The cscode source file could not be found."));
        }
      }
      else if (HasPrefix(szDriverName, cbpref))
      {
        if (!session->FindFile("cbgreek", FileType::MF, driverPath))
        {
          FatalError(T_("The cbgreek source file could not be found."));
        }
      }
      else if (!session->FindFile(szDriverName, FileType::MF, driverPath))
      {
        FatalError(T_("The %s source file could not be found."), szDriverName);
      }
    }
  }

  // get information about the font
  double true_pt_size;
  if (!session->GetFontInfo(szTeXFontname, supplier.GetData(), typeface.GetData(), &true_pt_size))
  {
    FatalError(T_("No info available for %s."), szTeXFontname);
  }

  // create destination directory
  CreateDestinationDirectory();

  // open the output stream
  FILE * stream = nullptr;
  PathName pathDest;
  AutoFILE autoClose;
  if (toStdout || printOnly)
  {
    stream = stdout;
  }
  else
  {
    // make fully qualified destination file name
    pathDest.Set(destinationDirectory, szTeXFontname, ".mf");
    Verbose(T_("Writing on %s..."), Q_(pathDest));
    if (!printOnly)
    {
      stream = File::Open(pathDest, FileMode::Create, FileAccess::Write);
      autoClose.Attach(stream);
    }
  }

  PrintOnly("cat <<__END__ > %s", Q_(pathDest));

  if (HasPrefix(szTeXFontname, "ec") || HasPrefix(szTeXFontname, "tc"))
  {
    fprintf(stream, "if unknown exbase: input exbase fi;\n");
    fprintf(stream, "gensize:=%0.2f;\n", true_pt_size);
    fprintf(stream, "generate %s;\n", szDriverName);
  }
  else if (HasPrefix(szTeXFontname, "dc"))
  {
    fprintf(stream, "if unknown dxbase: input dxbase fi;\n");
    fprintf(stream, "gensize:=%f;\n", true_pt_size);
    fprintf(stream, "generate %s;\n", szDriverName);
  }
  else if (HasPrefix(szTeXFontname, lhpref))
  {
    fprintf(stream, "input fikparm;\n");
  }
  else if (HasPrefix(szTeXFontname, cspref))
  {
    fprintf(stream, "input cscode\nuse_driver;\n");
  }
  else if (HasPrefix(szTeXFontname, cbpref))
  {
    fprintf(stream, "input cbgreek;\n");
  }
  else
  {
    fprintf(stream, "design_size:=%f;\n", true_pt_size);
    fprintf(stream, "input b-%s;\n", szDriverName);
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

MKTEXAPI(makemf) (int argc, const char ** argv)
{
  try
  {
    MakeMf app;
    app.Init(Session::InitInfo(argv[0]));
    app.Run(argc, argv);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException & e)
  {
    Application::Sorry("makemf", e);
    return 1;
  }
  catch (const exception & e)
  {
    Application::Sorry("makemf", e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
