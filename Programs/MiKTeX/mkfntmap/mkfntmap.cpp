/* mkfntmap.cpp:

   Copyright (C) 2002-2020 Christian Schenk

   This file is part of MkFntMap.

   MkFntMap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   MkFntMap is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MkFntMap; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

/* This program is based on the updmap shell script:
   (C) 2002 Thomas Esser
   licensed under the following agreement:
   Anyone may freely use, modify, and/or distribute this file... */

//#undef NDEBUG
#include "config.h"
#include "internal.h"

using namespace std;

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

#define PROGRAM_NAME "mkfntmap"

#if !defined(THE_NAME_OF_THE_GAME)
#  define THE_NAME_OF_THE_GAME T_("MiKTeX Fontmap Maintenance Utility")
#endif

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(PROGRAM_NAME));

enum Option
{
  OPT_AAA = 1,
  OPT_ADMIN,
  OPT_DISABLE_INSTALLER,
  OPT_ENABLE_INSTALLER,
  OPT_FORCE,
  OPT_OUTPUT_DIRECTORY,
  OPT_VERBOSE,
  OPT_VERSION,
};

const struct poptOption aoption[] = {
  {
    "admin", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ADMIN,
    T_("Run in administrator mode."),
    nullptr
  },

  {
    "disable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_DISABLE_INSTALLER,
    T_("Disable the package installer (do not automatically install missing files)."),
    nullptr
  },

  {
    "enable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ENABLE_INSTALLER,
    T_("Enable the package installer (automatically install missing files)."),
    nullptr
  },

  {
    "force", 0,
    POPT_ARG_NONE, nullptr,
    OPT_FORCE,
    T_("Force re-generation of apparently up-to-date fontconfig cache files, overriding the timestamp checking."),
    nullptr,
  },

  {
    "output-directory", 0,
    POPT_ARG_STRING, nullptr,
    OPT_OUTPUT_DIRECTORY,
    T_("Set the output directory."),
    "DIR"
  },

  {
    "verbose", 0,
    POPT_ARG_NONE, nullptr,
    OPT_VERBOSE,
    T_("Increase verbosity level."),
    nullptr
  },

  {
    "version", 0,
    POPT_ARG_NONE, nullptr,
    OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

struct FileContext
{
  PathName path;
  int line = 0;
};

class MakeFontMapApp :
  public Application,
  public IRunProcessCallback
{
public:
  void MyInit(int argc, const char** argv);

public:
  void Run();

private:
  void ProcessOptions(int argc, const char** argv);

private:
  void ShowVersion();

private:
  bool ToBool(const string& param);

private:
  bool ScanConfigLine(const string& line, string& directive, string& param);

private:
  void ParseConfigFile(const PathName& path);

private:
  bool LocateMap(const string& fileName, PathName& path, bool mustExist);

private:
  void ReadMap(const string& fileName, set<FontMapEntry>& fontMapEntries, bool mustExist);

private:
  void WriteHeader(ostream& writer, const PathName& fileName);

private:
  PathName CreateOutputDir(const string& relPath);

private:
  PathName GetDvipsOutputDir()
  {
    return CreateOutputDir(MIKTEX_PATH_DVIPS_CONFIG_DIR);
  }

private:
  PathName GetDvipdfmxOutputDir()
  {
    return CreateOutputDir(MIKTEX_PATH_DVIPDFMX_CONFIG_DIR);
  }

private:
  PathName GetPdfTeXOutputDir()
  {
    return CreateOutputDir(MIKTEX_PATH_PDFTEX_CONFIG_DIR);
  }

private:
  void WriteMap(ostream& writer, const set<FontMapEntry>& set1);

private:
  void WriteDvipdfmMap(ostream& writer, const set<FontMapEntry>& set1);

private:
  bool GetInstructionParam(const string& str, const string& instruction, string& param);

private:
  void WriteDvipsMapFile(const PathName& fileName, const set<FontMapEntry>& set1, const set<FontMapEntry>& set2, const set<FontMapEntry>& set3);

private:
  void WritePdfTeXMapFile(const PathName& fileName, const set<FontMapEntry>& set1, const set<FontMapEntry>& set2, const set<FontMapEntry>& set3);

private:
  void WriteDvipdfmMapFile(const PathName& fileName, const set<FontMapEntry>& set1, const set<FontMapEntry>& set2, const set<FontMapEntry>& set3);

private:
  set<FontMapEntry> CatMaps(const set<string>& fileNames);

private:
  set<FontMapEntry> TranslateLW35(const set<FontMapEntry>& set1);

private:
  void TranslateFontFile(const map<string, string>& transMap, FontMapEntry& fontMapEntry);

private:
  void TranslatePSName(const map<string, string>& files, FontMapEntry& fontMapEntry);

private:
  void CopyFile(const PathName& pathSrc, const PathName& pathDest);

private:
  void CopyFiles();

private:
  void BuildFontconfigCache();

private:
  void CreateFontconfigLocalfontsConf();

private:
  void Verbose(int level, const string& s);

private:
  void Verbose(const string& s)
  {
    Verbose(1, s);
  }

private:
  MIKTEXNORETURN void CfgError(const string& s);

private:
  MIKTEXNORETURN void MapError(const string& s);

private:
  void ParseDvipsMapFile(const PathName& mapFile, set<FontMapEntry>& fontMapEntries);

private:
  bool dvipdfmDownloadBase14 = true;

private:
  bool dvipsDownloadBase35 = false;

private:
  bool dvipsPreferOutline = true;

private:
  bool pdftexDownloadBase14 = true;

private:
  enum class NamingConvention {
    URW,
    URWkb,
    ADOBE,
    ADOBEkb
  };

private:
  NamingConvention namingConvention = NamingConvention::URWkb;

private:
  set<string> mapFiles;

private:
  set<string> mixedMapFiles;

private:
  int verbosityLevel = 0;

  // transform file names from URWkb (berry names) to URW (vendor
  // names)
private:
  static map<string, string> fileURW;

  // transform file names from URWkb (berry names) to ADOBE (vendor
  // names)
private:
  static map<string, string> fileADOBE;

  // transform file names from URW to ADOBE (both berry names)
private:
  static map<string, string> fileADOBEkb;

  // transform font names from URW to Adobe
private:
  static map<string, string> psADOBE;

private:
  string outputDirectory;

private:
  bool optAdminMode = false;

private:
  bool optDisableleInstaller = false;

private:
  bool optEnableInstaller = false;

private:
  bool force = false;

private:
  FileContext cfgContext;

private:
  FileContext mapContext;

private:
  bool disableInstaller = false;

private:
  bool InstallPackage(const string& packageId, const PathName& trigger, PathName& installRoot) override
  {
    if (disableInstaller)
    {
      return false;
    }
    return Application::InstallPackage(packageId, trigger, installRoot);
  }

private:
  bool OnProcessOutput(const void* output, size_t n) override;

private:
  string currentProcessOutputLine;

private:
  shared_ptr<Session> session;
};

map<string, string> MakeFontMapApp::fileURW;
map<string, string> MakeFontMapApp::fileADOBE;
map<string, string> MakeFontMapApp::fileADOBEkb;
map<string, string> MakeFontMapApp::psADOBE;

void MakeFontMapApp::ShowVersion()
{
  cout
    << Utils::MakeProgramVersionString(THE_NAME_OF_THE_GAME, VersionNumber(MIKTEX_COMPONENT_VERSION_STR)) << endl
    << endl
    << MIKTEX_COMP_COPYRIGHT_STR << endl
    << endl
    << "This is free software; see the source for copying conditions.  There is NO" << endl
    << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

void MakeFontMapApp::ProcessOptions(int argc, const char** argv)
{
  PoptWrapper popt(argc, argv, aoption);

  int option;

  while ((option = popt.GetNextOpt()) >= 0)
  {
    switch (option)
    {
    case OPT_ADMIN:
      optAdminMode = true;
      break;
    case OPT_DISABLE_INSTALLER:
      optDisableleInstaller = true;
      break;
    case OPT_ENABLE_INSTALLER:
      optEnableInstaller = true;
      break;
    case OPT_FORCE:
      force = true;
      break;
    case OPT_OUTPUT_DIRECTORY:
      outputDirectory = popt.GetOptArg();
      break;
    case OPT_VERBOSE:
      verbosityLevel++;
      break;
    case OPT_VERSION:
      ShowVersion();
      throw 0;
      break;
    }
  }

  if (option != -1)
  {
    string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
    msg += ": ";
    msg += popt.Strerror(option);
    FatalError(msg);
  }

  if (!popt.GetLeftovers().empty())
  {
    FatalError(T_("This utility does not accept non-option arguments."));
  }
}

void MakeFontMapApp::Verbose(int level, const string& s)
{
  if (level >= 4)
  {
    LOG4CXX_TRACE(logger, s);
  }
  else if (level >= 2)
  {
    LOG4CXX_DEBUG(logger, s);
  }
  else
  {
    LOG4CXX_INFO(logger, s);
  }
  if (verbosityLevel >= level)
  {
    cout << s << endl;
  }
}

MIKTEXNORETURN void MakeFontMapApp::CfgError(const string& s)
{
  LOG4CXX_FATAL(logger, s);
  LOG4CXX_FATAL(logger, "cfg file: " << cfgContext.path);
  LOG4CXX_FATAL(logger, "line: " << cfgContext.line);
  Sorry(PROGRAM_NAME);
  throw 1;
}

MIKTEXNORETURN void MakeFontMapApp::MapError(const string& s)
{
  LOG4CXX_FATAL(logger, s);
  LOG4CXX_FATAL(logger, "map file: " << mapContext.path.GetData());
  LOG4CXX_FATAL(logger, "line: " << mapContext.line);
  Sorry(PROGRAM_NAME);
  throw 1;
}

bool MakeFontMapApp::ToBool(const string& param)
{
  if (param.empty())
  {
    CfgError(T_("missing bool value"));
  }
  if (Utils::EqualsIgnoreCase(param, BOOLSTR(false)))
  {
    return false;
  }
  else if (Utils::EqualsIgnoreCase(param, BOOLSTR(true)))
  {
    return true;
  }
  else
  {
    CfgError(T_("invalid bool value"));
  }
}

bool MakeFontMapApp::ScanConfigLine(const string& line, string& directive, string& param)
{
  if (line.empty() || string("*#;%").find_first_of(line[0]) != string::npos)
  {
    return false;
  }
  Tokenizer tok(line, " \t\n");
  if (!tok)
  {
    return false;
  }
  directive = *tok;
  ++tok;
  if (!tok)
  {
    param = "";
  }
  else
  {
    param = *tok;
  }
  return true;
}

void MakeFontMapApp::ParseConfigFile(const PathName& path)
{
  Verbose(fmt::format(T_("Parsing config file {0}..."), Q_(path)));
  StreamReader reader(path);
  cfgContext.path = path;
  cfgContext.line = 0;
  string line;
  while (reader.ReadLine(line))
  {
    ++cfgContext.line;
    string directive;
    string param;
    if (!ScanConfigLine(line, directive, param))
    {
      continue;
    }
    if (Utils::EqualsIgnoreCase(directive, "dvipsPreferOutline"))
    {
      dvipsPreferOutline = ToBool(param);
    }
    else if (Utils::EqualsIgnoreCase(directive, "LW35"))
    {
      if (param.empty())
      {
        CfgError(T_("missing value"));
      }
      if (Utils::EqualsIgnoreCase(param, "URW"))
      {
        namingConvention = NamingConvention::URW;
      }
      else if (Utils::EqualsIgnoreCase(param, "URWkb"))
      {
        namingConvention = NamingConvention::URWkb;
      }
      else if (Utils::EqualsIgnoreCase(param, "ADOBE"))
      {
        namingConvention = NamingConvention::ADOBE;
      }
      else if (Utils::EqualsIgnoreCase(param, "ADOBEkb"))
      {
        namingConvention = NamingConvention::ADOBEkb;
      }
      else
      {
        CfgError(T_("invalid value"));
      }
    }
    else if (Utils::EqualsIgnoreCase(directive, "dvipsDownloadBase35"))
    {
      dvipsDownloadBase35 = ToBool(param);
    }
    else if (Utils::EqualsIgnoreCase(directive, "pdftexDownloadBase14"))
    {
      pdftexDownloadBase14 = ToBool(param);
    }
    else if (Utils::EqualsIgnoreCase(directive, "dvipdfmDownloadBase14"))
    {
      dvipdfmDownloadBase14 = ToBool(param);
    }
    else if (Utils::EqualsIgnoreCase(directive, "Map"))
    {
      if (param.empty())
      {
        CfgError(T_("missing map file name"));
      }
      mapFiles.insert(param);
    }
    else if (Utils::EqualsIgnoreCase(directive, "MixedMap"))
    {
      if (param.empty())
      {
        CfgError(T_("missing map file name"));
      }
      mixedMapFiles.insert(param);
    }
    else
    {
      CfgError(T_("invalid configuration setting"));
    }
  }
  reader.Close();
}

void MakeFontMapApp::MyInit(int argc, const char** argv)
{
  Session::InitInfo initInfo(argv[0]);
  vector<const char*> newargv(&argv[0], &argv[argc + 1]);
  ExamineArgs(newargv, initInfo);
  ProcessOptions(static_cast<int>(newargv.size() - 1), &newargv[0]);
  if (optAdminMode)
  {
    initInfo.AddOption(Session::InitOption::AdminMode);
  }
  Application::Init(initInfo);
  session = GetSession();
  if (optAdminMode)
  {
    Verbose(T_("Entering administrator mode..."));
  }
  if (optDisableleInstaller)
  {
    EnableInstaller(TriState::False);
  }
  else if (optEnableInstaller)
  {
    EnableInstaller(TriState::True);
  }

  bool parsedConfig = false;

  static vector<string> configFiles = {
    MIKTEX_PATH_MKFNTMAP_CFG,
    MIKTEX_PATH_MIKTEX_CONFIG_DIR MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "updmap.cfg",
    MIKTEX_PATH_WEB2C_DIR MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "updmap.cfg"
  };

  for (const string& cfgFile : configFiles)
  {
    vector<PathName> cfgFiles;
    if (session->FindFile(cfgFile, MIKTEX_PATH_TEXMF_PLACEHOLDER, { Session::FindFileOption::All }, cfgFiles))
    {
      for (vector<PathName>::const_reverse_iterator it = cfgFiles.rbegin(); it != cfgFiles.rend(); ++it)
      {
        ParseConfigFile(*it);
        parsedConfig = true;
      }
    }
  }

  if (!parsedConfig)
  {
    FatalError(T_("The config file could not be found."));
  }

  fileURW["uagd8a.pfb"] = "a010015l.pfb";
  fileURW["uagdo8a.pfb"] = "a010035l.pfb";
  fileURW["uagk8a.pfb"] = "a010013l.pfb";
  fileURW["uagko8a.pfb"] = "a010033l.pfb";
  fileURW["ubkd8a.pfb"] = "b018015l.pfb";
  fileURW["ubkdi8a.pfb"] = "b018035l.pfb";
  fileURW["ubkl8a.pfb"] = "b018012l.pfb";
  fileURW["ubkli8a.pfb"] = "b018032l.pfb";
  fileURW["ucrb8a.pfb"] = "n022004l.pfb";
  fileURW["ucrbo8a.pfb"] = "n022024l.pfb";
  fileURW["ucrr8a.pfb"] = "n022003l.pfb";
  fileURW["ucrro8a.pfb"] = "n022023l.pfb";
  fileURW["uhvb8a.pfb"] = "n019004l.pfb";
  fileURW["uhvb8ac.pfb"] = "n019044l.pfb";
  fileURW["uhvbo8a.pfb"] = "n019024l.pfb";
  fileURW["uhvbo8ac.pfb"] = "n019064l.pfb";
  fileURW["uhvr8a.pfb"] = "n019003l.pfb";
  fileURW["uhvr8ac.pfb"] = "n019043l.pfb";
  fileURW["uhvro8a.pfb"] = "n019023l.pfb";
  fileURW["uhvro8ac.pfb"] = "n019063l.pfb";
  fileURW["uncb8a.pfb"] = "c059016l.pfb";
  fileURW["uncbi8a.pfb"] = "c059036l.pfb";
  fileURW["uncr8a.pfb"] = "c059013l.pfb";
  fileURW["uncri8a.pfb"] = "c059033l.pfb";
  fileURW["uplb8a.pfb"] = "p052004l.pfb";
  fileURW["uplbi8a.pfb"] = "p052024l.pfb";
  fileURW["uplr8a.pfb"] = "p052003l.pfb";
  fileURW["uplri8a.pfb"] = "p052023l.pfb";
  fileURW["usyr.pfb"] = "s050000l.pfb";
  fileURW["utmb8a.pfb"] = "n021004l.pfb";
  fileURW["utmbi8a.pfb"] = "n021024l.pfb";
  fileURW["utmr8a.pfb"] = "n021003l.pfb";
  fileURW["utmri8a.pfb"] = "n021023l.pfb";
  fileURW["uzcmi8a.pfb"] = "z003034l.pfb";
  fileURW["uzdr.pfb"] = "d050000l.pfb";

  fileADOBE["uagd8a.pfb"] = "agd_____.pfb";
  fileADOBE["uagdo8a.pfb"] = "agdo____.pfb";
  fileADOBE["uagk8a.pfb"] = "agw_____.pfb";
  fileADOBE["uagko8a.pfb"] = "agwo____.pfb";
  fileADOBE["ubkd8a.pfb"] = "bkd_____.pfb";
  fileADOBE["ubkdi8a.pfb"] = "bkdi____.pfb";
  fileADOBE["ubkl8a.pfb"] = "bkl_____.pfb";
  fileADOBE["ubkli8a.pfb"] = "bkli____.pfb";
  fileADOBE["ucrb8a.pfb"] = "cob_____.pfb";
  fileADOBE["ucrbo8a.pfb"] = "cobo____.pfb";
  fileADOBE["ucrr8a.pfb"] = "com_____.pfb";
  fileADOBE["ucrro8a.pfb"] = "coo_____.pfb";
  fileADOBE["uhvb8a.pfb"] = "hvb_____.pfb";
  fileADOBE["uhvb8ac.pfb"] = "hvnb____.pfb";
  fileADOBE["uhvbo8a.pfb"] = "hvbo____.pfb";
  fileADOBE["uhvbo8ac.pfb"] = "hvnbo___.pfb";
  fileADOBE["uhvr8a.pfb"] = "hv______.pfb";
  fileADOBE["uhvr8ac.pfb"] = "hvn_____.pfb";
  fileADOBE["uhvro8a.pfb"] = "hvo_____.pfb";
  fileADOBE["uhvro8ac.pfb"] = "hvno____.pfb";
  fileADOBE["uncb8a.pfb"] = "ncb_____.pfb";
  fileADOBE["uncbi8a.pfb"] = "ncbi____.pfb";
  fileADOBE["uncr8a.pfb"] = "ncr_____.pfb";
  fileADOBE["uncri8a.pfb"] = "nci_____.pfb";
  fileADOBE["uplb8a.pfb"] = "pob_____.pfb";
  fileADOBE["uplbi8a.pfb"] = "pobi____.pfb";
  fileADOBE["uplr8a.pfb"] = "por_____.pfb";
  fileADOBE["uplri8a.pfb"] = "poi_____.pfb";
  fileADOBE["usyr.pfb"] = "sy______.pfb";
  fileADOBE["utmb8a.pfb"] = "tib_____.pfb";
  fileADOBE["utmbi8a.pfb"] = "tibi____.pfb";
  fileADOBE["utmr8a.pfb"] = "tir_____.pfb";
  fileADOBE["utmri8a.pfb"] = "tii_____.pfb";
  fileADOBE["uzcmi8a.pfb"] = "zcmi____.pfb";
  fileADOBE["uzdr.pfb"] = "zd______.pfb";

  fileADOBEkb["uagd8a.pfb"] = "pagd8a.pfb";
  fileADOBEkb["uagdo8a.pfb"] = "pagdo8a.pfb";
  fileADOBEkb["uagk8a.pfb"] = "pagk8a.pfb";
  fileADOBEkb["uagko8a.pfb"] = "pagko8a.pfb";
  fileADOBEkb["ubkd8a.pfb"] = "pbkd8a.pfb";
  fileADOBEkb["ubkdi8a.pfb"] = "pbkdi8a.pfb";
  fileADOBEkb["ubkl8a.pfb"] = "pbkl8a.pfb";
  fileADOBEkb["ubkli8a.pfb"] = "pbkli8a.pfb";
  fileADOBEkb["ucrb8a.pfb"] = "pcrb8a.pfb";
  fileADOBEkb["ucrbo8a.pfb"] = "pcrbo8a.pfb";
  fileADOBEkb["ucrr8a.pfb"] = "pcrr8a.pfb";
  fileADOBEkb["ucrro8a.pfb"] = "pcrro8a.pfb";
  fileADOBEkb["uhvb8a.pfb"] = "phvb8a.pfb";
  fileADOBEkb["uhvb8ac.pfb"] = "phvb8an.pfb";
  fileADOBEkb["uhvbo8a.pfb"] = "phvbo8a.pfb";
  fileADOBEkb["uhvbo8ac.pfb"] = "phvbo8an.pfb";
  fileADOBEkb["uhvr8a.pfb"] = "phvr8a.pfb";
  fileADOBEkb["uhvr8ac.pfb"] = "phvr8an.pfb";
  fileADOBEkb["uhvro8a.pfb"] = "phvro8a.pfb";
  fileADOBEkb["uhvro8ac.pfb"] = "phvro8an.pfb";
  fileADOBEkb["uncb8a.pfb"] = "pncb8a.pfb";
  fileADOBEkb["uncbi8a.pfb"] = "pncbi8a.pfb";
  fileADOBEkb["uncr8a.pfb"] = "pncr8a.pfb";
  fileADOBEkb["uncri8a.pfb"] = "pncri8a.pfb";
  fileADOBEkb["uplb8a.pfb"] = "pplb8a.pfb";
  fileADOBEkb["uplbi8a.pfb"] = "pplbi8a.pfb";
  fileADOBEkb["uplr8a.pfb"] = "pplr8a.pfb";
  fileADOBEkb["uplri8a.pfb"] = "pplri8a.pfb";
  fileADOBEkb["usyr.pfb"] = "psyr.pfb";
  fileADOBEkb["utmb8a.pfb"] = "ptmb8a.pfb";
  fileADOBEkb["utmbi8a.pfb"] = "ptmbi8a.pfb";
  fileADOBEkb["utmr8a.pfb"] = "ptmr8a.pfb";
  fileADOBEkb["utmri8a.pfb"] = "ptmri8a.pfb";
  fileADOBEkb["uzcmi8a.pfb"] = "pzcmi8a.pfb";
  fileADOBEkb["uzdr.pfb"] = "pzdr.pfb";

  psADOBE["URWGothicL-Demi"] = "AvantGarde-Demi";
  psADOBE["URWGothicL-DemiObli"] = "AvantGarde-DemiOblique";
  psADOBE["URWGothicL-Book"] = "AvantGarde-Book";
  psADOBE["URWGothicL-BookObli"] = "AvantGarde-BookOblique";
  psADOBE["URWBookmanL-DemiBold"] = "Bookman-Demi";
  psADOBE["URWBookmanL-DemiBoldItal"] = "Bookman-DemiItalic";
  psADOBE["URWBookmanL-Ligh"] = "Bookman-Light";
  psADOBE["URWBookmanL-LighItal"] = "Bookman-LightItalic";
  psADOBE["NimbusMonL-Bold"] = "Courier-Bold";
  psADOBE["NimbusMonL-BoldObli"] = "Courier-BoldOblique";
  psADOBE["NimbusMonL-Regu"] = "Courier";
  psADOBE["NimbusMonL-ReguObli"] = "Courier-Oblique";
  psADOBE["NimbusSanL-Bold"] = "Helvetica-Bold";
  psADOBE["NimbusSanL-BoldCond"] = "Helvetica-Narrow-Bold";
  psADOBE["NimbusSanL-BoldItal"] = "Helvetica-BoldOblique";
  psADOBE["NimbusSanL-BoldCondItal"] = "Helvetica-Narrow-BoldOblique";
  psADOBE["NimbusSanL-Regu"] = "Helvetica";
  psADOBE["NimbusSanL-ReguCond"] = "Helvetica-Narrow";
  psADOBE["NimbusSanL-ReguItal"] = "Helvetica-Oblique";
  psADOBE["NimbusSanL-ReguCondItal"] = "Helvetica-Narrow-Oblique";
  psADOBE["CenturySchL-Bold"] = "NewCenturySchlbk-Bold";
  psADOBE["CenturySchL-BoldItal"] = "NewCenturySchlbk-BoldItalic";
  psADOBE["CenturySchL-Roma"] = "NewCenturySchlbk-Roman";
  psADOBE["CenturySchL-Ital"] = "NewCenturySchlbk-Italic";
  psADOBE["URWPalladioL-Bold"] = "Palatino-Bold";
  psADOBE["URWPalladioL-BoldItal"] = "Palatino-BoldItalic";
  psADOBE["URWPalladioL-Roma"] = "Palatino-Roman";
  psADOBE["URWPalladioL-Ital"] = "Palatino-Italic";
  psADOBE["StandardSymL"] = "Symbol";
  psADOBE["NimbusRomNo9L-Medi"] = "Times-Bold";
  psADOBE["NimbusRomNo9L-MediItal"] = "Times-BoldItalic";
  psADOBE["NimbusRomNo9L-Regu"] = "Times-Roman";
  psADOBE["NimbusRomNo9L-ReguItal"] = "Times-Italic";
  psADOBE["URWChanceryL-MediItal"] = "ZapfChancery-MediumItalic";
  psADOBE["Dingbats"] = "ZapfDingbats";
}

bool MakeFontMapApp::LocateMap(const string& fileName, PathName& path, bool mustExist)
{
  disableInstaller = !mustExist;
  bool found = session->FindFile(fileName, FileType::MAP, path);
  disableInstaller = false;
  if (!found && mustExist)
  {
    FatalError(fmt::format(T_("Font map file {0} could not be found."), Q_(fileName)));
  }
  if (!found)
  {
    Verbose(3, fmt::format(T_("Not using map file {0}"), Q_(fileName)));
  }
  return found;
}

void MakeFontMapApp::WriteHeader(ostream& writer, const PathName& fileName)
{
  UNUSED_ALWAYS(fileName);
  writer
    << T_("%%% DO NOT EDIT THIS FILE! It will be replaced when MiKTeX is updated.") << "\n"
    << T_("%%% Run the following command to edit a local version of this file:") << "\n"
    << "%%%   initexmf --edit-config-file updmap" << endl;
}

void MakeFontMapApp::WriteMap(ostream& writer, const set<FontMapEntry>& set1)
{
  for (const FontMapEntry& fme : set1)
  {
    writer << fmt::format("{} {}", fme.texName, fme.psName);
    if (!fme.specialInstructions.empty())
    {
      writer << fmt::format(" \" {} \"", fme.specialInstructions);
    }
    for (Tokenizer tok(fme.headerList, ";"); tok; ++tok)
    {
      writer << fmt::format(" {}", *tok);
    }
    writer << endl;
  }
}

bool MakeFontMapApp::GetInstructionParam(const string& str, const string& instruction, string& param)
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

void MakeFontMapApp::WriteDvipdfmMap(ostream& writer, const set<FontMapEntry>& set1)
{
  for (const FontMapEntry& fme : set1)
  {
    string field1 = fme.texName;
    string field2;
    if (!fme.encFile.empty())
    {
      field2 = PathName(fme.encFile).GetFileNameWithoutExtension().GetData();
    }
    string field3;
    if (!fme.fontFile.empty())
    {
      field3 = PathName(fme.fontFile).GetFileNameWithoutExtension().GetData();
    }
    else if (fme.texName != fme.psName)
    {
      field3 = fme.psName;
    }
    string options;
    string param;
    if (GetInstructionParam(fme.specialInstructions, "ExtendFont", param))
    {
      options += " -e ";
      options += param;
    }
    if (GetInstructionParam(fme.specialInstructions, "SlantFont", param))
    {
      options += " -s ";
      options += param;
    }
    if (fme.texName.substr(0, 2) == "cm"
      || fme.texName.substr(0, 2) == "eu"
      || (fme.texName.substr(0, 2) == "la" && !(fme.encFile.substr(0, 12) == "cm-super-t2a"))
      || (fme.texName.substr(0, 2) == "lc" && !(fme.encFile.substr(0, 12) == "cm-super-t2c"))
      || fme.texName.substr(0, 4) == "line"
      || fme.texName.substr(0, 4) == "msam"
      || fme.texName.substr(0, 2) == "xy")
    {
      if (!(fme.fontFile.substr(0, 4) == "fmex"))
      {
        options += " -r";
      }
    }
    if (field2 == "" && field3 == "" && options == "")
    {
      continue;
    }
    writer << field1;
    if (!field2.empty())
    {
      writer << fmt::format(" {}", field2);
    }
    else if (!field3.empty())
    {
      writer << fmt::format(" {}", "default");
    }
    if (!field3.empty())
    {
      writer << fmt::format(" {}", field3);
    }
    if (!options.empty())
    {
      writer << options;
    }
    writer << endl;
  }
}

PathName MakeFontMapApp::CreateOutputDir(const string& relPath)
{
  PathName path;
  if (!outputDirectory.empty())
  {
    path = outputDirectory;
  }
  else
  {
    path = session->GetSpecialPath(SpecialPath::DataRoot) / PathName(relPath);
  }
  if (!Directory::Exists(path))
  {
    Directory::Create(path);
  }
  return path;
}

void MakeFontMapApp::WriteDvipsMapFile(const PathName& fileName, const set<FontMapEntry>& set1, const set<FontMapEntry>& set2, const set<FontMapEntry>& set3)
{
  PathName path(GetDvipsOutputDir());
  path /= fileName;
  Verbose(fmt::format(T_("Writing {0}..."), Q_(path)));
  // TODO: backup old file
  ofstream writer = File::CreateOutputStream(path, ios_base::binary);
  WriteHeader(writer, path);
  set<FontMapEntry> setAll = set1;
  setAll.insert(set2.begin(), set2.end());
  setAll.insert(set3.begin(), set3.end());
  WriteMap(writer, setAll);
  writer.close();
  if (!Fndb::FileExists(path))
  {
    Fndb::Add({ {path} });
  }
}

void MakeFontMapApp::WriteDvipdfmMapFile(const PathName& fileName, const set<FontMapEntry>& set1, const set<FontMapEntry>& set2, const set<FontMapEntry>& set3)
{
  PathName path(GetDvipdfmxOutputDir());
  path /= fileName;
  Verbose(fmt::format(T_("Writing {0}..."), Q_(path)));
  // TODO: backup old file
  ofstream writer = File::CreateOutputStream(path, ios_base::binary);
  WriteHeader(writer, path);
  set<FontMapEntry> setAll = set1;
  setAll.insert(set2.begin(), set2.end());
  setAll.insert(set3.begin(), set3.end());
  WriteDvipdfmMap(writer, setAll);
  writer.close();
  if (!Fndb::FileExists(path))
  {
    Fndb::Add({ {path} });
  }
}

void MakeFontMapApp::WritePdfTeXMapFile(const PathName& fileName, const set<FontMapEntry>& set1, const set<FontMapEntry>& set2, const set<FontMapEntry>& set3)
{
  PathName path(GetPdfTeXOutputDir());
  path /= fileName;
  Verbose(fmt::format(T_("Writing {0}..."), Q_(path)));
  // TODO: backup old file
  ofstream writer = File::CreateOutputStream(path, ios_base::binary);
  WriteHeader(writer, path);
  set<FontMapEntry> setAll = set1;
  setAll.insert(set2.begin(), set2.end());
  setAll.insert(set3.begin(), set3.end());
  WriteMap(writer, setAll);
  writer.close();
  if (!Fndb::FileExists(path))
  {
    Fndb::Add({ {path} });
  }
}

void MakeFontMapApp::ParseDvipsMapFile(const PathName& mapFile, set<FontMapEntry>& fontMapEntries)
{
  Verbose(2, fmt::format(T_("Parsing {0}..."), Q_(mapFile)));

  StreamReader reader(mapFile);

  string line;

  mapContext.path = mapFile;
  mapContext.line = 0;

  while (reader.ReadLine(line))
  {
    ++mapContext.line;
    FontMapEntry fontMapEntry;
    try
    {
      if (Utils::ParseDvipsMapLine(line, fontMapEntry))
      {
        fontMapEntries.insert(fontMapEntry);
      }
    }
    catch (const MiKTeXException& e)
    {
      MapError(e.GetErrorMessage());
    }
  }

  reader.Close();
}

bool MIKTEXTHISCALL MakeFontMapApp::OnProcessOutput(const void* output, size_t n)
{
  const char* text = (const char*)output;
  for (size_t idx = 0; idx < n; ++idx)
  {
    char ch = text[idx];
    if (ch == '\r')
    {
      if (idx < n - 1)
      {
        ++idx;
        ch = text[idx];
      }
    }
    if (ch == '\n')
    {
      LOG4CXX_INFO(logger, currentProcessOutputLine);
      currentProcessOutputLine.clear();
    }
    else
    {
      currentProcessOutputLine += ch;
    }
  }
  return true;
}

void MakeFontMapApp::ReadMap(const string& fileName, set<FontMapEntry>& result, bool mustExist)
{
  PathName path;
  if (!LocateMap(fileName, path, mustExist))
  {
    return;
  }
  ParseDvipsMapFile(path, result);
}

set<FontMapEntry> MakeFontMapApp::CatMaps(const set<string>& fileNames)
{
  set<FontMapEntry> result;
  for (const string& fn : fileNames)
  {
    ReadMap(fn, result, false);
  }
  return result;
}

void MakeFontMapApp::TranslateFontFile(const map<string, string>& transMap, FontMapEntry& fontMapEntry)
{
  map<string, string>::const_iterator it = transMap.find(fontMapEntry.fontFile);
  if (it != transMap.end())
  {
    fontMapEntry.fontFile = it->second;
  }
  Tokenizer header(fontMapEntry.headerList, ";");
  fontMapEntry.headerList = "";
  for (; header; ++header)
  {
    if (!fontMapEntry.headerList.empty())
    {
      fontMapEntry.headerList += ';';
    }
    string s = *header;
    const char* lpsz;
    for (lpsz = s.c_str(); *lpsz == '<' || *lpsz == '['; ++lpsz)
    {
      fontMapEntry.headerList += *lpsz;
    }
    it = transMap.find(lpsz);
    if (it == transMap.end())
    {
      fontMapEntry.headerList += lpsz;
    }
    else
    {
      fontMapEntry.headerList += it->second;
    }
  }
}

void MakeFontMapApp::TranslatePSName(const map<string, string>& names, FontMapEntry& fontMapEntry)
{
  map<string, string>::const_iterator it = names.find(fontMapEntry.psName);
  if (it != names.end())
  {
    fontMapEntry.psName = it->second;
  }
}

set<FontMapEntry> MakeFontMapApp::TranslateLW35(const set<FontMapEntry>& set1)
{
  set<FontMapEntry> result;
  for (const FontMapEntry& fme : set1)
  {
    FontMapEntry fontMapEntry = fme;
    switch (namingConvention)
    {
    case NamingConvention::URWkb:
      break;
    case NamingConvention::URW:
      TranslateFontFile(fileURW, fontMapEntry);
      break;
    case NamingConvention::ADOBE:
      TranslatePSName(psADOBE, fontMapEntry);
      TranslateFontFile(fileADOBE, fontMapEntry);
      break;
    case NamingConvention::ADOBEkb:
      TranslatePSName(psADOBE, fontMapEntry);
      TranslateFontFile(fileADOBEkb, fontMapEntry);
      break;
    }
    result.insert(fontMapEntry);
  }
  return result;
}

void MakeFontMapApp::CopyFile(const PathName& pathSrc, const PathName& pathDest)
{
  Verbose(fmt::format(T_("Copying {0}"), Q_(pathSrc)));
  Verbose(fmt::format(T_("     to {0}..."), Q_(pathDest)));
  File::Copy(pathSrc, pathDest);
  if (!Fndb::FileExists(pathDest))
  {
    Fndb::Add({ {pathDest} });
  }
}

void MakeFontMapApp::CopyFiles()
{
  PathName dvipsOutputDir(GetDvipsOutputDir());
  PathName dvipdfmxOutputDir(GetDvipdfmxOutputDir());
  PathName pdftexOutputDir(GetPdfTeXOutputDir());

  PathName pathSrc;

  pathSrc = dvipsOutputDir / PathName(dvipsPreferOutline ? "psfonts_t1" : "psfonts_pk");
  pathSrc.AppendExtension(".map");
  CopyFile(pathSrc, PathName(dvipsOutputDir, PathName("psfonts.map")));

  pathSrc = dvipdfmxOutputDir / PathName(dvipdfmDownloadBase14 ? "dvipdfm_dl14" : "dvipdfm_ndl14");
  pathSrc.AppendExtension(".map");
  CopyFile(pathSrc, PathName(dvipdfmxOutputDir, PathName("dvipdfm.map")));

  pathSrc = pdftexOutputDir / PathName(pdftexDownloadBase14 ? "pdftex_dl14" : "pdftex_ndl14");
  pathSrc.AppendExtension(".map");
  CopyFile(pathSrc, PathName(pdftexOutputDir, PathName("pdftex.map")));
}

static const char* const topDirs[] = {
  "fonts" MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "type1",
  "fonts" MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "opentype",
  "fonts" MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "truetype",
};

void MakeFontMapApp::BuildFontconfigCache()
{
#if !defined(USE_SYSTEM_FONTCONFIG)
  session->ConfigureFile(PathName(MIKTEX_PATH_FONTCONFIG_CONFIG_FILE));
#endif
  CreateFontconfigLocalfontsConf();
  PathName fcCacheExe;
#if !defined(USE_SYSTEM_FONTCONFIG)
  if (!session->FindFile(MIKTEX_FC_CACHE_EXE, FileType::EXE, fcCacheExe))
  {
    FatalError(T_("The fc-cache executable could not be found."));
  }
#else
  if (!Utils::FindProgram("fc-cache", fcCacheExe))
  {
    FatalError(T_("The fc-cache executable could not be found."));
  }
#endif
  vector<string> arguments{ fcCacheExe.GetFileNameWithoutExtension().ToString() };
  if (optAdminMode)
  {
#if !defined(USE_SYSTEM_FONTCONFIG)
    arguments.push_back("--miktex-admin");
#else
    arguments.push_back("--system-only");
#endif
  }
  if (force)
  {
    arguments.push_back("--force");
  }
  for (int n = 0; n < verbosityLevel; ++n)
  {
    arguments.push_back("--verbose");
  }
#if !defined(USE_SYSTEM_FONTCONFIG)
  arguments.push_back("--miktex-disable-maintenance");
  arguments.push_back("--miktex-disable-diagnose");
#endif
  LOG4CXX_INFO(logger, "running: " << CommandLineBuilder(arguments).ToString());
  Process::Run(fcCacheExe, arguments, this);
}

void MakeFontMapApp::CreateFontconfigLocalfontsConf()
{
  PathName configFile(session->GetSpecialPath(SpecialPath::ConfigRoot));
  configFile /= MIKTEX_PATH_FONTCONFIG_LOCALFONTS_FILE;
  StreamWriter writer(configFile);
  writer.WriteLine("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  writer.WriteLine();
  writer.WriteLine("<!--");
  writer.WriteLine(T_("  DO NOT EDIT THIS FILE! It will be replaced when MiKTeX is updated."));
#if !defined(USE_SYSTEM_FONTCONFIG)
  writer.WriteLine(fmt::format(T_("  Instead, edit the configuration file {0}."), MIKTEX_LOCALFONTS2_CONF));
#endif
  writer.WriteLine("-->");
  writer.WriteLine();
  writer.WriteLine("<fontconfig>");
#if !defined(USE_SYSTEM_FONTCONFIG)
  writer.WriteLine(fmt::format("<include>{}</include>", MIKTEX_LOCALFONTS2_CONF));
#endif
  vector<string> paths;
#if !defined(USE_SYSTEM_FONTCONFIG)
  for (const string& path : session->GetFontDirectories())
  {
    paths.push_back(path);
  }
#endif
  for (unsigned r = 0; r < session->GetNumberOfTEXMFRoots(); ++r)
  {
    PathName root = session->GetRootDirectoryPath(r);
    for (const char* dir : topDirs)
    {
      PathName path = root;
      path /= dir;
      if (Directory::Exists(path))
      {
        paths.push_back(path.GetData());
      }
    }
  }
  for (const string& path : paths)
  {
    writer.WriteLine(fmt::format("<dir>{}</dir>", path));
  }
  writer.WriteLine("</fontconfig>");
  writer.Close();
#if defined(USE_SYSTEM_FONTCONFIG)
  if (optAdminMode)
  {
    PathName confd(MIKTEX_SYSTEM_ETC_FONTS_CONFD_DIR);
    File::CreateLink(configFile, confd / PathName("09-miktex.conf"), { CreateLinkOption::ReplaceExisting, CreateLinkOption::Symbolic });
  }
#endif
#if !defined(USE_SYSTEM_FONTCONFIG)
  configFile.RemoveFileSpec();
  configFile /= MIKTEX_LOCALFONTS2_CONF;
  if (!File::Exists(configFile))
  {
    StreamWriter writer(configFile);
    writer.WriteLine("<?xml version=\"1.0\"?>");
    writer.WriteLine("<fontconfig>");
    writer.WriteLine(T_("<!-- REMOVE THIS LINE"));
    writer.WriteLine(T_("<dir>Your font directory here</dir>"));
    writer.WriteLine(T_("<dir>Your font directory here</dir>"));
    writer.WriteLine(T_("<dir>Your font directory here</dir>"));
    writer.WriteLine(T_("     REMOVE THIS LINE -->"));
    writer.WriteLine("</fontconfig>");
    writer.Close();
  }
#endif
}

bool HasPaintType(const FontMapEntry& fontMapEntry)
{
  return fontMapEntry.specialInstructions.find("PaintType") != string::npos;
}

void MakeFontMapApp::Run()
{
  set<FontMapEntry> dvips35;
  ReadMap("dvips35.map", dvips35, true);
  set<FontMapEntry> pdftex35;
  ReadMap("pdftex35.map", pdftex35, true);
  set<FontMapEntry> dvipdfm35;
  ReadMap("dvipdfm35.map", dvipdfm35, true);
  set<FontMapEntry> ps2pk35;
  ReadMap("ps2pk35.map", ps2pk35, true);

  set<FontMapEntry> transLW35_ps2pk35(TranslateLW35(ps2pk35));

  set<FontMapEntry> transLW35_dvips35(TranslateLW35(dvips35));

  set<FontMapEntry> transLW35_pdftex35(TranslateLW35(pdftex35));

  set<FontMapEntry> transLW35_dvipdfm35(TranslateLW35(dvipdfm35));

  set<FontMapEntry> tmp1(CatMaps(mixedMapFiles));

  set<FontMapEntry> tmp2(CatMaps(mapFiles));

  WriteDvipsMapFile(PathName("ps2pk.map"), transLW35_ps2pk35, tmp1, tmp2);

  set<FontMapEntry> empty;

  WriteDvipsMapFile(PathName("download35.map"), transLW35_ps2pk35, empty, empty);

  WriteDvipsMapFile(PathName("builtin35.map"), transLW35_dvips35, empty, empty);

  set<FontMapEntry> transLW35_dftdvips(TranslateLW35(dvipsDownloadBase35 ? ps2pk35 : dvips35));

  WriteDvipsMapFile(PathName("psfonts_t1.map"), transLW35_dftdvips, tmp1, tmp2);

  WriteDvipsMapFile(PathName("psfonts_pk.map"), transLW35_dftdvips, empty, tmp2);

  set<FontMapEntry> tmp3 = transLW35_pdftex35;
  tmp3.insert(tmp1.begin(), tmp1.end());
  tmp3.insert(tmp2.begin(), tmp2.end());
  set<FontMapEntry>::iterator it = tmp3.begin();
  while (it != tmp3.end())
  {
    if (HasPaintType(*it))
    {
      it = tmp3.erase(it);
    }
    else
    {
      ++it;
    }
  }

  set<FontMapEntry> tmp6 = transLW35_dvipdfm35;
  tmp6.insert(tmp1.begin(), tmp1.end());
  tmp6.insert(tmp2.begin(), tmp2.end());
  it = tmp6.begin();
  while (it != tmp6.end())
  {
    if (HasPaintType(*it))
    {
      it = tmp6.erase(it);
    }
    else
    {
      ++it;
    }
  }

  set<FontMapEntry> tmp7 = transLW35_ps2pk35;
  tmp7.insert(tmp1.begin(), tmp1.end());
  tmp7.insert(tmp2.begin(), tmp2.end());
  it = tmp7.begin();
  while (it != tmp7.end())
  {
    if (HasPaintType(*it))
    {
      it = tmp7.erase(it);
    }
    else
    {
      ++it;
    }
  }

  WritePdfTeXMapFile(PathName("pdftex_ndl14.map"), tmp3, empty, empty);
  WritePdfTeXMapFile(PathName("pdftex_dl14.map"), tmp7, empty, empty);

  WriteDvipdfmMapFile(PathName("dvipdfm_dl14.map"), tmp7, empty, empty);
  WriteDvipdfmMapFile(PathName("dvipdfm_ndl14.map"), tmp6, empty, empty);

  CopyFiles();

  BuildFontconfigCache();
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
  MakeFontMapApp app;
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<const char*> newargv;
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
      newargv.push_back(utf8args[idx].c_str());
    }
    newargv.push_back(nullptr);
    app.MyInit(static_cast<int>(newargv.size() - 1), &newargv[0]);
    app.Run();
    app.Finalize2(0);
    logger = nullptr;
    return 0;
  }
  catch (const MiKTeXException& ex)
  {
    Application::Sorry(THE_NAME_OF_THE_GAME, ex);
    logger = nullptr;
    ex.Save();
    return 1;
  }
  catch (const exception& ex)
  {
    Application::Sorry(THE_NAME_OF_THE_GAME, ex);
    logger = nullptr;
    return 1;
  }
  catch (int exitCode)
  {
    logger = nullptr;
    return exitCode;
  }
}
