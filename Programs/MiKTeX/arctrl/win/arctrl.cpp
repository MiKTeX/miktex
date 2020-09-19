/* arctrl.cpp: Acrobat Reader (AR) Controller

   Written in the years 2006-2020 by Christian Schenk.

   This file is based on public domain work by Fabrice Popineau.

   To the extent possible under law, the author(s) have dedicated all
   copyright and related and neighboring rights to this file to the
   public domain worldwide.  This file is distributed without any
   warranty.  You should have received a copy of the CC0 Public Domain
   Dedication along with this file.  If not, see
   http://creativecommons.org/publicdomain/zero/1.0/. */

#if defined(MIKTEX_WINDOWS)
#  include <Windows.h>
#endif

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <iomanip>
#include <iostream>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "arctrl-version.h"

#include <miktex/App/Application>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/StreamReader>
#include <miktex/Core/win/winAutoResource>
#include <miktex/Util/CharBuffer>
#include <miktex/Util/Tokenizer>
#include <miktex/Wrappers/PoptWrapper>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace MiKTeX;
using namespace std;

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#define WU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

#if !defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

const char* const TheNameOfTheGame = T_("MiKTeX AR Controller");
const char* const PROGNAME = T_("arctrl");

class ArCtrl :
  public Application
{
public:
  ArCtrl() = default;

public:
  ArCtrl(const ArCtrl& other) = default;

public:
  ArCtrl& operator=(const ArCtrl& other) = default;

public:
  ArCtrl(ArCtrl&& other) = default;

public:
  ArCtrl& operator=(ArCtrl&& other) = default;

public:
  virtual ~ArCtrl();

private:
  void ShowVersion();

public:
  void Run(int argc, const char** argv);

private:
  enum {
    None, Open, Close, CommandLoop
  } mode = None;

private:
  HSZ CreateDdeString(const char* lpsz);

private:
  void EstablishConversation();

private:
  void TerminateConversation();

private:
  void ExecuteDdeCommand(const string& s);

private:
  void DocOpen(const PathName& path);

private:
  void DocClose(const PathName& path);

private:
  void CloseAllDocs();

private:
  void AppShow();

private:
  void AppHide();

private:
  void AppExit();

private:
  void DocGoTo(const PathName& path, int pageNum);

private:
  void DocGoToNameDest(const PathName& path, const string& nameDest);

private:
  void FileOpen(const PathName& path);

private:
  bool Execute(const string& command);

private:
  void ReadAndExecute();

private:
  void ArCtrl::StartAR();

private:
  static HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hdata, ULONG_PTR dwData1, ULONG_PTR dwData2);

private:
  HCONV hConv = nullptr;

private:
  shared_ptr<Session> session;

private:
  bool ARstarted = false;

private:
  static DWORD idInst;

private:
  static const struct poptOption aoption[];

private:
  static const struct poptOption aoptionOpen[];

private:
  static const struct poptOption aoptionClose[];

private:
  static const struct poptOption aoptionCommandLoop[];
};

enum Option
{
  OPT_AAA = 256,
  OPT_ALL,
  OPT_FILE,
  OPT_GOTO,
  OPT_PAGE,
  OPT_VERSION,
};

const struct poptOption ArCtrl::aoption[] =
{
  {
    "version", 'V', POPT_ARG_NONE, nullptr, OPT_VERSION, T_("Print version information and exit."), nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

const struct poptOption ArCtrl::aoptionOpen[] =
{
  {
    "file", 0, POPT_ARG_STRING, nullptr, OPT_FILE, T_("Open FILE."), T_("FILE")
  },

  {
    "page", 0, POPT_ARG_STRING, nullptr, OPT_PAGE, T_("Go to the N-th page."), "N"
  },

  {
    "goto", 0, POPT_ARG_STRING, nullptr, OPT_GOTO, T_("Go to the specified named destination within the document."), T_("NAMEDEST")
  },

  {
    "version", 'V', POPT_ARG_NONE, nullptr, OPT_VERSION, T_("Print version information and exit."), nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

const struct poptOption ArCtrl::aoptionClose[] =
{
  {
    "all", 0, POPT_ARG_NONE, nullptr, OPT_ALL, T_("Close all files."), nullptr
  },

  {
    "file", 0, POPT_ARG_STRING, nullptr, OPT_FILE, T_("Close FILE."), T_("FILE")
  },

  {
    "version", 'V', POPT_ARG_NONE, nullptr, OPT_VERSION, T_("Print version information and exit."), nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

const struct poptOption ArCtrl::aoptionCommandLoop[] =
{
  {
    "version", 'V', POPT_ARG_NONE, nullptr, OPT_VERSION, T_("Print version information and exit."), nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

DWORD ArCtrl::idInst = 0;

ArCtrl::~ArCtrl()
{
  try
  {
    TerminateConversation();
  }
  catch (const exception&)
  {
  }
}

void ArCtrl::ShowVersion()
{
  cout
    << Utils::MakeProgramVersionString(TheNameOfTheGame, VersionNumber(MIKTEX_COMPONENT_VERSION_STR))
    << endl
    << T_("Written by Christian Schenk in 2006.") << endl
    << T_("Based on public domain work by Fabrice Popineau.") << endl
    << T_("Based on Adobe documentation.") << endl;
}

HSZ ArCtrl::CreateDdeString(const char* lpsz)
{

  HSZ hsz = DdeCreateStringHandleW(idInst, UW_(lpsz), CP_WINUNICODE);
  if (hsz == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return hsz;
}

void ArCtrl::StartAR()
{
  if (ARstarted)
  {
    return;
  }
  PathName pdfFile;
  if (!session->FindFile("miktex.pdf", FileType::TEXSYSDOC, pdfFile))
  {
    MIKTEX_UNEXPECTED();
  }
  PathName dir = pdfFile;
  dir.RemoveFileSpec();
  char szExecutable[BufferSizes::MaxPath];
  if (FindExecutableA("miktex.pdf", dir.GetData(), szExecutable) <= reinterpret_cast<HINSTANCE>(32))
  {
    FatalError(T_("The PDF viewer could not be located."));
  }
  unique_ptr<Process> pProcess(Process::Start(ProcessStartInfo(PathName(szExecutable))));
  ARstarted = true;
#if 0
  // <todo>
  WaitForInputIdle(hProcess, INFINITE);
  // </todo>
#endif
}

HDDEDATA CALLBACK ArCtrl::DdeCallback(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hdata, ULONG_PTR dwData1, ULONG_PTR dwData2)
{
  UNUSED_ALWAYS(dwData1);
  UNUSED_ALWAYS(dwData2);
  UNUSED_ALWAYS(hconv);
  UNUSED_ALWAYS(hsz1);
  UNUSED_ALWAYS(hsz2);
  UNUSED_ALWAYS(uFmt);
  DWORD_PTR ret = DDE_FNOTPROCESSED;
  try
  {
    switch (uType)
    {
    case XTYP_ADVDATA:
    {
      DWORD len = DdeGetData(hdata, nullptr, 0, 0);
      if (DdeGetLastError(idInst) != DMLERR_NO_ERROR)
      {
        MIKTEX_UNEXPECTED();
      }
      CharBuffer<wchar_t> buf(len / sizeof(wchar_t) + sizeof(wchar_t));
      DdeGetData(hdata, reinterpret_cast<LPBYTE>(const_cast<wchar_t*>(buf.ToString().c_str())), buf.GetCapacity() * sizeof(wchar_t), 0);
      if (DdeGetLastError(idInst) != DMLERR_NO_ERROR)
      {
        MIKTEX_UNEXPECTED();
      }
      ret = DDE_FACK;
    }
    }
  }
  catch (const exception&)
  {
    ret = DDE_FNOTPROCESSED;
  }
  return reinterpret_cast<HDDEDATA>(ret);
}

const char* const ServiceNames[] = {
  "acroviewR15", "acroviewR11", "acroviewR10", "acroview"
};

void ArCtrl::EstablishConversation()
{
  DWORD idInst = 0;

  UINT ret = DdeInitializeW(&idInst, DdeCallback, APPCMD_CLIENTONLY, 0);

  if (ret != DMLERR_NO_ERROR)
  {
    FatalError(T_("The DDE library could not be initialized."));
  }

  this->idInst = idInst;

  HSZ hszTopic = CreateDdeString("control");
  AutoDdeFreeStringHandle autoFree2(idInst, hszTopic);

  MIKTEX_ASSERT(hConv == nullptr);

  for (int idx = 0; hConv == nullptr && idx < sizeof(ServiceNames) / sizeof(ServiceNames[0]); ++idx)
  {
    HSZ hszService = CreateDdeString(ServiceNames[idx]);
    AutoDdeFreeStringHandle autoFree1(idInst, hszService);
    hConv = DdeConnect(idInst, hszService, hszTopic, nullptr);
    if (hConv == nullptr)
    {
      StartAR();
      for (int rounds = 0; hConv == nullptr && rounds < 5; ++rounds)
      {
        Sleep(500);
        hConv = DdeConnect(idInst, hszService, hszTopic, nullptr);
      }
    }
  }

  if (hConv == nullptr)
  {
    FatalError(T_("The DDE conversation could not be established."));
  }
}

void ArCtrl::TerminateConversation()
{
  if (hConv != nullptr)
  {
    if (!DdeDisconnect(hConv))
    {
      MIKTEX_UNEXPECTED();
    }
    hConv = nullptr;
  }
  if (idInst != 0)
  {
    if (!DdeUninitialize(idInst))
    {
      MIKTEX_UNEXPECTED();
    }
    idInst = 0;
  }
}

void ArCtrl::ExecuteDdeCommand(const string& s)
{
  wstring data = UW_(s);
  HDDEDATA h = DdeClientTransaction(const_cast<BYTE *>(reinterpret_cast<const BYTE *>(data.c_str())), static_cast<DWORD>((data.length() + 1) * sizeof(data[0])), hConv, nullptr, 0, XTYP_EXECUTE, 5000, nullptr);
  if (h == nullptr)
  {
    FatalError(T_("The command could not be executed."));
  }
}

void ArCtrl::DocOpen(const PathName& path)
{
  if (!File::Exists(path))
  {
    FatalError(T_("The specified file could not be found."));
  }
  PathName fullPath(path);
  fullPath.MakeFullyQualified();
  ExecuteDdeCommand(fmt::format("[DocOpen(\"{0}\")]", fullPath));
}

void ArCtrl::DocClose(const PathName& path)
{
  if (!File::Exists(path))
  {
    FatalError(T_("The specified file could not be found."));
  }
  PathName fullPath(path);
  fullPath.MakeFullyQualified();
  ExecuteDdeCommand(fmt::format("[DocClose(\"{0}\")]", fullPath));
}

void ArCtrl::CloseAllDocs()
{
  ExecuteDdeCommand("[CloseAllDocs()]");
}

void ArCtrl::AppShow()
{
  ExecuteDdeCommand("[AppShow()]");
}

void ArCtrl::AppHide()
{
  ExecuteDdeCommand("[AppHide()]");
}

void ArCtrl::AppExit()
{
  ExecuteDdeCommand("[AppExit()]");
}

void ArCtrl::DocGoTo(const PathName& path, int pageNum)
{
  if (!File::Exists(path))
  {
    FatalError(T_("The specified file could not be found."));
  }
  PathName fullPath(path);
  fullPath.MakeFullyQualified();
  ExecuteDdeCommand(fmt::format("[DocGoTo(\"{0}\",{1})]", fullPath, pageNum));
}

void ArCtrl::DocGoToNameDest(const PathName& path, const string& nameDest)
{
  if (!File::Exists(path))
  {
    FatalError(T_("The specified file could not be found."));
  }
  PathName fullPath(path);
  fullPath.MakeFullyQualified();
  ExecuteDdeCommand(fmt::format("[DocGoToNameDest(\"{0}\",\"{1}\")]", fullPath, nameDest));
}

void ArCtrl::FileOpen(const PathName& path)
{
  if (!File::Exists(path))
  {
    FatalError(T_("The specified file could not be found."));
  }
  PathName fullPath(path);
  fullPath.MakeFullyQualified();
  ExecuteDdeCommand(fmt::format("[FileOpen(\"{0}\")]", fullPath));
}

bool ArCtrl::Execute(const string& commandLine)
{
  Tokenizer tok(commandLine, " \t\r\n");
  if (!tok)
  {
    return true;
  }
  string command = *tok;
  ++tok;
  if (Utils::EqualsIgnoreCase(command, "open"))
  {
    if (!tok)
    {
      cerr << T_("Error: missing file name argument.") << endl;
      return true;
    }
    DocOpen(PathName(*tok));
  }
  else if (Utils::EqualsIgnoreCase(command, "close"))
  {
    if (!tok)
    {
      cerr << T_("Error: missing file name argument.") << endl;
      return true;
    }
    DocClose(PathName(*tok));
  }
  else if (Utils::EqualsIgnoreCase(command, "closeall"))
  {
    if (tok)
    {
      cerr << T_("Error: too many arguments.") << endl;
      return true;
    }
    CloseAllDocs();
  }
  else if (Utils::EqualsIgnoreCase(command, "goto"))
  {
    if (!tok)
    {
      cerr << T_("Error: missing file name argument.") << endl;
      return true;
    }
    string fileName = *tok;
    ++tok;
    if (!tok)
    {
      cerr << T_("Error: missing page number argument.") << endl;
      return true;
    }
    DocGoTo(PathName(fileName), std::stoi(*tok) - 1);
  }
  else if (Utils::EqualsIgnoreCase(command, "gotoname"))
  {
    if (!tok)
    {
      cerr << T_("Error: missing file name argument.") << endl;
      return true;
    }
    string fileName = *tok;
    ++tok;
    if (!tok)
    {
      cerr << T_("Error: missing name dest argument.") << endl;
      return true;
    }
    DocGoToNameDest(PathName(fileName), *tok);
  }
  else if (Utils::EqualsIgnoreCase(command, "show"))
  {
    AppShow();
  }
  else if (Utils::EqualsIgnoreCase(command, "hide"))
  {
    AppHide();
  }
  else if (Utils::EqualsIgnoreCase(command, "exit"))
  {
    AppExit();
    return false;
  }
  else if (Utils::EqualsIgnoreCase(command, "help"))
  {
    cout
      << "close FILE" << endl
      << "closeall" << endl
      << "exit" << endl
      << "goto FILE PAGE" << endl
      << "gotoname FILE NAMEDEST" << endl
      << "help" << endl
      << "hide" << endl
      << "open FILE" << endl
      << "show" << endl;
  }
  return true;
}

void ArCtrl::ReadAndExecute()
{
  StreamReader reader(true);
  string command;
  while (reader.ReadLine(command))
  {
    if (!Execute(command))
    {
      break;
    }
  }
}

void ArCtrl::Run(int argc, const char** argv)
{
  session = GetSession();

  string program = Utils::GetExeName();

  const poptOption* pOption = &aoption[0];

  PathName file;
  int pageNum = -1;
  string nameDest;
  bool all = false;

  if (PathName::Compare(program, "pdfopen") == 0)
  {
    mode = Open;
    pOption = &aoptionOpen[0];
  }
  else if (PathName::Compare(program, "pdfclose") == 0)
  {
    mode = Close;
    pOption = &aoptionClose[0];
  }
  else if (PathName::Compare(program, "pdfdde") == 0)
  {
    mode = CommandLoop;
    pOption = &aoptionCommandLoop[0];
  }

  PoptWrapper popt(argc, argv, pOption);

  int option;
  while ((option = popt.GetNextOpt()) >= 0)
  {
    switch (option)
    {
    case OPT_ALL:

      if (!file.Empty())
      {
        FatalError(T_("Options --all and --file cannot be used both."));
      }
      all = true;
      break;

    case OPT_FILE:

      if (all)
      {
        FatalError(T_("Options --all and --file cannot be used both."));
      }
      if (!file.Empty())
      {
        FatalError(T_("Multiple --file arguments are not supported."));
      }
      file = popt.GetOptArg();
      break;

    case OPT_GOTO:

      if (!nameDest.empty())
      {
        FatalError(T_("Multiple --goto arguments are not supported."));
      }
      nameDest = popt.GetOptArg();
      break;


    case OPT_PAGE:

      if (pageNum >= 0)
      {
        FatalError(T_("Multiple --page arguments are not supported."));
      }
      pageNum = std::stoi(popt.GetOptArg()) - 1;
      break;

    case OPT_VERSION:

      ShowVersion();
      throw (0);
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
    FatalError(T_("This program does not process non-option arguments."));
  }

  EstablishConversation();

  switch (mode)
  {
  case Open:
    if (file.Empty())
    {
      FatalError(T_("No file name was specified."));
    }
    DocOpen(file);
    FileOpen(file);
    if (pageNum >= 0)
    {
      DocGoTo(file, pageNum);
    }
    if (!nameDest.empty())
    {
      DocGoToNameDest(file, nameDest);
    }
    break;
  case Close:
    if (all)
    {
      CloseAllDocs();
    }
    else
    {
      if (file.Empty())
      {
        FatalError(T_("No file name was specified."));
      }
      DocClose(file);
    }
    break;
  case CommandLoop:
    ReadAndExecute();
    break;
  }

  TerminateConversation();
}

extern "C" __declspec(dllexport) int __cdecl arctrlmain(int argc, char** argv)
{
  ArCtrl app;
  try
  {
    vector<char*> newargv{ argv, argv + argc + 1 };
    app.Init(newargv);
    app.Run(newargv.size(), const_cast<const char**>(&newargv[0]));
    app.Finalize2(0);
    return 0;
  }
  catch (const MiKTeXException& e)
  {
    Application::Sorry(TheNameOfTheGame, e);
    e.Save();
    return 1;
  }
  catch (const exception& e)
  {
    Application::Sorry(TheNameOfTheGame, e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
