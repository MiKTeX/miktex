/* makepk.cpp:

   Copyright (C) 1998-2018 Christian Schenk

   This file is part of MiKTeX MakePK.

   The MiKTeX MakePK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   The MiKTeX MakePK is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX MakePK; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "makepk-version.h"
#include "MakeUtility.h"

log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("makepk"));

#define OPT_MAP_FILE 1
#define OPT_FORCE 2

class MakePk :
  public MakeUtility
{
public:
  void Run(int argc, const char** argv) override;

private:
  void Usage() override;

private:
  void CreateDestinationDirectory() override;

private:
  BEGIN_OPTION_MAP(MakePk)
    OPTION_ENTRY(OPT_MAP_FILE, mapFiles.push_back(lpszOptArg))
    OPTION_ENTRY_TRUE(OPT_FORCE, overwriteExisting)
  END_OPTION_MAP();

private:
  string MakeModeName(int bdpi);

private:
  void MakePKFilename(const char* lpszName, int bdpi, int dpi, PathName& result);

private:
  void CheckOptions(int* pBaseDPI, int dpi, const string& mode);

private:
  void ExtraPS2PKOptions(const FontMapEntry& mapEntry, vector<string>& arguments);

private:
  void RunGSF2PK(const FontMapEntry& mapEntry, const char* lpszPkName, int dpi, const PathName& workingDirectory);

private:
  void RunPS2PK(const FontMapEntry& mapEntry, const char* lpszPkName, int dpi, const PathName& workingDirectory);

private:
  bool FindFontMapping(const char* lpszTeXFontName, const char* lpszMapFileName, FontMapEntry& mapEntry);

private:
  bool SearchPostScriptFont(const char* lpszTeXFontName, FontMapEntry& mapEntry);

private:
  bool IsHbf(const char* lpszName);

private:
  bool overwriteExisting = false;

private:
  bool modeless;

private:
  int dpi;

private:
  int bdpi;

private:
  string magnification;

private:
  string mfMode;

private:
  vector<string> mapFiles;
};

void
MakePk::Usage()
{
  OUT__
    << T_("Usage:") << " " << Utils::GetExeName() << " " << T_("[OPTION]... name dpi bdpi magnification [MODE]") << endl
    << endl
    << T_("This program makes a PK font.") << endl
    << endl
    << T_("NAME is the name of the TeX font, such as 'cmr10'.  DPI is") << endl
    << T_("the resolution the font is needed at.  BDPI is the base") << endl
    << T_("resolution, useful for figuring out the mode to make the font") << endl
    << T_("in.  MAGNIFICATION is a string to pass to METAFONT as the") << endl
    << T_("magnification.  MODE, if supplied, is the mode to use.") << endl
    << endl
    << T_("You can specify 0 as BDPI. In that case, BDPI is calculated from") << endl
    << T_("the MODE.") << endl
    << endl
    << T_("Options:") << endl
    << "--debug, -d " << T_("Print debugging information.") << endl
    << "--disable-installer " << T_("Disable the package installer.") << endl
    << "--enable-installer " << T_("Enable the package installer.") << endl
    << "--force " << T_("Make PK font, even if it exists already.") << endl
    << "--help, -h " << T_("Print this help screen and exit.") << endl
    << "--map-file=FILE " << T_("Consult additional map file.") << endl
    << "--print-only, -n " << T_("Print what commands would be executed.") << endl
    << "--verbose, -v " << T_("Print information on what is being done.") << endl
    << "--version, -V " << T_("Print the version number and exit.") << endl;
}

namespace {
  const struct option aLongOptions[] =
  {
    COMMON_OPTIONS,
    "force",                no_argument,            nullptr,      OPT_FORCE,
    "map-file",             required_argument,      nullptr,      OPT_MAP_FILE,
    nullptr,                no_argument,            nullptr,      0,
  };
}

void MakePk::CreateDestinationDirectory()
{
  // get basic font information (supplier & typeface)
  string supplier;
  string typeface;
  if (!session->GetFontInfo(name, supplier, typeface, nullptr))
  {
    Verbose(T_("No information available for font %s."), Q_(name));
    supplier = "unknown";
    typeface = "unknown";
  }
  if (supplier.empty())
  {
    supplier = "unknown";
  }
  if (typeface.empty())
  {
    typeface = "unknown";
  }

  // get destination path template
  string templ1 = session->GetConfigValue(MIKTEX_REGKEY_MAKEPK, MIKTEX_REGVAL_DESTDIR, "%R/fonts/pk/%m/%s/%t/dpi%d").GetString();

  string templ2;
  for (const char* lpsz = templ1.c_str(); *lpsz != 0; ++lpsz)
  {
    if (lpsz[0] == '%')
    {
      switch (lpsz[1])
      {
      default:
        break;
      case '%':
        templ2 += '%';
        break;
      case 'R':
        templ2 += "%R";
        break;
      case 'b':
        templ2 += std::to_string(bdpi);
        break;
      case 'd':
        templ2 += std::to_string(dpi);
        break;
      case 'm':
        templ2 += modeless ? "modeless" : mfMode;
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

void MakePk::MakePKFilename(const char* lpszName, int bdpi, int dpi, PathName& result)
{
  string templ = session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_PK_FN_TEMPLATE, "%f.pk").GetString();
  string temp;
  for (const char* lpsz = templ.c_str(); *lpsz != 0; ++lpsz)
  {
    if (lpsz[0] == '%')
    {
      switch (lpsz[1])
      {
      case '%':
        temp += '%';
        break;
      case 'f':
        temp += lpszName;
        break;
      case 'b':
        temp += std::to_string(bdpi);
        break;
      case 'd':
        temp += std::to_string(dpi);
        break;
      }
      ++lpsz;
    }
    else
    {
      temp += *lpsz;
    }
  }
  result = temp;
}

string MakePk::MakeModeName(int bdpi)
{
  if (bdpi == 0)
  {
    FatalError(T_("Neither BDPI nor MODE were specified."));
  }
  MIKTEXMFMODE mfmode;
  if (!session->DetermineMETAFONTMode(bdpi, mfmode))
  {
    FatalError(T_("The METAFONT mode could not be determined."));
  }
  return mfmode.mnemonic;
}

bool GetInstructionParam(const string& str, const string& instruction, string& param)
{
  param = "";
  for (Tokenizer tok(str, " \t"); tok; ++tok)
  {
    if (instruction == *tok)
    {
      return true;
    }
    param = *tok;
  }
  return false;
}

void MakePk::ExtraPS2PKOptions(const FontMapEntry& mapEntry, vector<string>& arguments)
{
  if (!mapEntry.encFile.empty())
  {
    arguments.push_back("-e");
    arguments.push_back(mapEntry.encFile);
  }

  string param;

  if (GetInstructionParam(mapEntry.specialInstructions, "ExtendFont", param))
  {
    arguments.push_back("-E");
    arguments.push_back(param);
  }

  if (GetInstructionParam(mapEntry.specialInstructions, "SlantFont", param))
  {
    arguments.push_back("-S");
    arguments.push_back(param);
  }
}

void MakePk::RunGSF2PK(const FontMapEntry& mapEntry, const char* lpszPkName, int dpi, const PathName& workingDirectory)
{
  vector<string> arguments;
  arguments.push_back(mapEntry.texName);
  arguments.push_back(mapEntry.psName);
  arguments.push_back(mapEntry.specialInstructions);
  arguments.push_back(mapEntry.encFile);
  arguments.push_back(mapEntry.fontFile);
  arguments.push_back(std::to_string(dpi));
  arguments.push_back(lpszPkName);
  if (!RunProcess(MIKTEX_GSF2PK_EXE, arguments, workingDirectory))
  {
    FatalError(fmt::format(T_("GSF2PK failed on {0}."), Q_(mapEntry.fontFile)));
  }
}

void MakePk::RunPS2PK(const FontMapEntry& mapEntry, const char* lpszPkName, int dpi, const PathName& workingDirectory)
{
  bool oldFonts = false;        // FIXME

  vector<string> arguments;

  if (verbose)
  {
    arguments.push_back("-v");
  }

  if (oldFonts)
  {
    arguments.push_back("-O");
  }

  arguments.push_back("-X" + std::to_string(dpi));

  ExtraPS2PKOptions(mapEntry, arguments);

  arguments.push_back(mapEntry.fontFile);

  arguments.push_back(lpszPkName);

  if (!RunProcess(MIKTEX_PS2PK_EXE, arguments, workingDirectory))
  {
    FatalError(fmt::format(T_("PS2PK failed on {0}."), Q_(mapEntry.fontFile)));
  }
}

void MakePk::CheckOptions(int* baseDpi, int dpi, const string& mode)
{
  UNUSED_ALWAYS(dpi);
  MIKTEXMFMODE mfmode;
  int i = 0;
  bool found = false;
  while (!found && session->GetMETAFONTMode(i, mfmode))
  {
    if (mfmode.mnemonic == mode)
    {
      found = true;
    }
    else
    {
      ++i;
    }
  }
  if (!found)
  {
    FatalError(fmt::format(T_("{0} is an unknown METAFONT mode."), Q_(mode)));
  }
  if (*baseDpi == 0)
  {
    *baseDpi = mfmode.horizontalResolution;
  }
  if (mfmode.horizontalResolution != *baseDpi)
  {
    FatalError(fmt::format(T_("Specified BDPI ({0}) doesn't match {1} resolution ({2})."), *baseDpi, Q_(mode), mfmode.horizontalResolution));
  }
}

bool MakePk::FindFontMapping(const char* lpszTeXFontName, const char* lpszMapFileName, FontMapEntry& mapEntry)
{
  // locate the map file
  PathName mapFile;
  if (!session->FindFile(lpszMapFileName, FileType::MAP, mapFile))
  {
    return false;
  }

  // open the map file
  AutoFILE pStream(File::Open(mapFile, FileMode::Open, FileAccess::Read));

  // try to find the font mapping
  bool found = false;
  string line;
  while (!found && Utils::ReadUntilDelim(line, '\n', pStream.Get()))
  {
    if (Utils::ParseDvipsMapLine(line, mapEntry) && mapEntry.texName == lpszTeXFontName)
    {
      found = true;
    }
  }

  pStream.Reset();

  return found;
}

bool MakePk::SearchPostScriptFont(const char* lpszTeXFontName, FontMapEntry& mapEntry)
{
  // search via "ps2pk.map" (also used for gsf2pk)
  if (FindFontMapping(lpszTeXFontName, "ps2pk.map", mapEntry))
  {
    return true;
  }

  // search via user supplied map files
  for (const string& mapFile : mapFiles)
  {
    if (FindFontMapping(lpszTeXFontName, mapFile.c_str(), mapEntry))
    {
      return !mapEntry.fontFile.empty();
    }
  }

  return false;
}

bool MakePk::IsHbf(const char* lpszName)
{
  PathName hbfcfg(lpszName);
  size_t l = hbfcfg.GetLength();
  if (l > 2)
  {
    hbfcfg[l - 2] = 0;
  }
  hbfcfg.SetExtension(".cfg", false);
  PathName path;
  return session->FindFile(hbfcfg.ToString(), "%R/HBF2GF//", path);
}

void MakePk::Run(int argc, const char** argv)
{
  // get command line options and arguments
  int optionIndex = 0;
  GetOptions(argc, argv, aLongOptions, optionIndex);
  if (argc - optionIndex < 4 || argc - optionIndex > 5)
  {
    FatalError(T_("Invalid command-line."));
  }
  name = argv[optionIndex++];
  dpi = atoi(argv[optionIndex++]);
  bdpi = atoi(argv[optionIndex++]);
  magnification = argv[optionIndex++];
  if (optionIndex < argc)
  {
    mfMode = argv[optionIndex++];
  }

  Verbose(T_("Trying to make PK font %s at %d DPI..."), Q_(name), static_cast<int>(dpi));

  // make a mode name if none was specified
  if (mfMode.empty() || mfMode == "default")
  {
    mfMode = MakeModeName(bdpi);
    Verbose(T_("The METFAONT mode is: %s"), mfMode.c_str());
  }

  // validate command-line arguments
  CheckOptions(&bdpi, dpi, mfMode);

  // create a temporary working directory
  unique_ptr<TemporaryDirectory> wrkDir = TemporaryDirectory::Create();

  string gfName = name + '.' + std::to_string(dpi) + "gf";

  FontMapEntry mapEntry;

  modeless = false;

  bool isTTF = false;
  bool isHBF = false;

  bool haveSource = false;

  PathName path;

  // try to convert a METAFONT file
  if (session->FindFile(name, FileType::MF, path))
  {
    haveSource = true;
  }
  else
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
    if (RunProcess(MIKTEX_MAKEMF_EXE, arguments, wrkDir->GetPathName()))
    {
      haveSource = true;
    }
  }

  // try to convert a TTF file
  if (!haveSource)
  {
    if (RunProcess(MIKTEX_TTF2PK_EXE, { "-q", "-t", name }, wrkDir->GetPathName()))
    {
      vector<string> arguments;
      if (!debug)
      {
        arguments.push_back("-q");
      }
      arguments.push_back("-n"); // only use '.pk' as extension
      arguments.push_back(name);
      arguments.push_back(std::to_string(dpi));
      if (RunProcess(MIKTEX_TTF2PK_EXE, arguments, wrkDir->GetPathName()))
      {
        isTTF = true;
        modeless = true;
        haveSource = true;
      }
    }
  }

  // try to convert an HBF file
  if (!haveSource && IsHbf(name.c_str()))
  {
    vector<string> arguments;
    if (debug)
    {
      arguments.push_back("-q");
    }
    arguments.push_back("-p");
    arguments.push_back(name);
    arguments.push_back(std::to_string(dpi));
    if (RunProcess(MIKTEX_HBF2GF_EXE, arguments, wrkDir->GetPathName()))
    {
      isHBF = true;
      modeless = true;
      haveSource = true;
    }
  }

  // try to find a PFB file
  if (!haveSource && SearchPostScriptFont(name.c_str(), mapEntry))
  {
    modeless = true;
    haveSource = true;
  }

  if (!haveSource)
  {
    FatalError(fmt::format(T_("PK font {0} could not be created."), Q_(name)));
  }

  // create destination directory
  CreateDestinationDirectory();

  // make PK file name
  PathName pkName;
  MakePKFilename(name.c_str(), bdpi, dpi, pkName);

  // make fully qualified destination file name
  PathName pathDest(destinationDirectory.GetData(), pkName.GetData());

  // quit, if destination file already exists
  if (File::Exists(pathDest))
  {
    Message(T_("The PK font file %s already exists."), Q_(pathDest));
    if (!overwriteExisting)
    {
      return;
    }
  }

  Verbose(T_("Creating %s..."), Q_(pkName));

  // now make the font
  if (modeless)
  {
    if (isTTF)
    {
      // ttf2pk made it already
    }
    else if (isHBF)
    {
      // convert GF file into PK file
      if (!RunProcess(MIKTEX_GFTOPK_EXE, { gfName, pkName.ToString() }, wrkDir->GetPathName()))
      {
        FatalError(fmt::format(T_("GFtoPK failed on {0}."), Q_(gfName)));
      }
    }
    else
    {
      bool done = false;
      // run gsf2pk/ps2pk to make a PK font from the PFB file
      try
      {
        RunGSF2PK(mapEntry, pkName.GetData(), dpi, wrkDir->GetPathName());
        done = true;
      }
      catch (int)
      {
      }
      if (!done)
      {
        RunPS2PK(mapEntry, pkName.GetData(), dpi, wrkDir->GetPathName());
      }
    }
  }
  else
  {
    // run METAFONT/GFtoPK to make a PK font
    if (!RunMETAFONT(name.c_str(), mfMode.c_str(), magnification.c_str(), wrkDir->GetPathName()))
    {
      FatalError(fmt::format(T_("METAFONT failed on {0}."), Q_(name)));
    }
    if (!RunProcess(MIKTEX_GFTOPK_EXE, { gfName, pkName.ToString() }, wrkDir->GetPathName()))
    {
      FatalError(fmt::format(T_("GFtoPK failed on {0}."), Q_(gfName)));
    }
  }

  // install PK font file
  Install(wrkDir->GetPathName() / pkName, pathDest);
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
  MakePk app;
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
    Application::Sorry("makepk", ex);
    logger = nullptr;
    ex.Save();
    return 1;
  }
  catch (const exception& ex)
  {
    Application::Sorry("makepk", ex);
    logger = nullptr;
    return 1;
  }
  catch (int exitCode)
  {
    logger = nullptr;
    return exitCode;
  }
}
