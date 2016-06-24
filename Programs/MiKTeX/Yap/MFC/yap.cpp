/* yap.cpp: Yet Another Previewer

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of Yap.

   Yap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Yap is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Yap; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "yap.h"

#include "ChildFrame.h"
#include "DviDoc.h"
#include "DviView.h"
#include "InverseSearchOptionsPage.h"
#include "MainFrame.h"
#include "winplace.h"

// the one and only YapApplication object
YapApplication theApp;

enum {
  OPT_DDE = 1,
  OPT_FIND_SRC_SPECIAL,
  OPT_GOTO_HYPER_LABEL,
  OPT_PRINT,
  OPT_REGISTER,
  OPT_SINGLE_INSTANCE,
  OPT_TRACE,
  OPT_UNREGISTER,
};

namespace {
  struct poptOption const aoption[] = {
    {
      "dde", 0, POPT_ARG_NONE, nullptr, OPT_DDE,
      T_("Starts up for DDE."),
      nullptr
    },
    {
      "find-src-special", 's', POPT_ARG_STRING, nullptr, OPT_FIND_SRC_SPECIAL,
      T_("Initiates forward search."),
      T_("SrcSpecial")
    },
    {
      "goto-hyper-label", 'g', POPT_ARG_STRING, nullptr, OPT_GOTO_HYPER_LABEL,
      T_("Jumps to a HyperTeX label."),
      T_("HYPERLABEL")
    },
    {
      "print", 0, POPT_ARG_NONE, nullptr, OPT_PRINT,
      T_("Sends the document to the default printer."),
      nullptr
    },
    {
      "register", 0, POPT_ARG_NONE, nullptr, OPT_REGISTER,
      T_("Stores Yap related keys in the Windows Registry."),
      nullptr
    },
    {
      "single-instance", '1', POPT_ARG_NONE, nullptr, OPT_SINGLE_INSTANCE,
      T_("Reuses an application window, if one exists."),
      nullptr
    },
    {
      "trace", 0, POPT_ARG_STRING, nullptr, OPT_TRACE,
      T_("Turns tracing on."),
      T_("TRACEFLAGS")
    },
    {
      "unregister", 0, POPT_ARG_NONE, nullptr, OPT_UNREGISTER,
      T_("Removes Yap related keys from the Windows Registry."),
      nullptr
    },
    POPT_TABLEEND
  };
}

namespace {
  bool registering = false;
  bool unregistering = false;
}

void ParseYapCommandLine(const char * lpszCommandLine, YapCommandLineInfo & cmdInfo)
{
  shared_ptr<Session> session = Session::Get();

  cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

  if (*lpszCommandLine == 0)
  {
    return;
  }

  int option;

  Argv argv;

  argv.Build("yap", lpszCommandLine);

  PoptWrapper popt(argv.GetArgc(), const_cast<const char **>(argv.GetArgv()), aoption);

  popt.SetOtherOptionHelp(T_("[OPTION...] [DVIFILE]"));

  while ((option = popt.GetNextOpt()) >= 0)
  {
    switch (option)
    {

    case OPT_DDE:
      AfxOleSetUserCtrl(FALSE);
      cmdInfo.m_nShellCommand = CCommandLineInfo::FileDDE;
      break;

    case OPT_FIND_SRC_SPECIAL:
    {
      char * lpszFileName = nullptr;
      cmdInfo.sourceLineNum = strtol(popt.GetOptArg().c_str(), &lpszFileName, 10);
      if (lpszFileName != nullptr)
      {
        while (*lpszFileName == ' ')
        {
          ++lpszFileName;
        }
        if (*lpszFileName != 0)
        {
          cmdInfo.sourceFile = lpszFileName;
        }
      }
      break;
    }

    case OPT_GOTO_HYPER_LABEL:
      cmdInfo.hyperLabel = popt.GetOptArg();
      break;

    case OPT_PRINT:
      cmdInfo.m_nShellCommand = CCommandLineInfo::FilePrint;
      break;

    case OPT_SINGLE_INSTANCE:
      cmdInfo.singleInstance = true;
      break;

    case OPT_REGISTER:
      cmdInfo.m_nShellCommand = CCommandLineInfo::AppRegister;
      registering = true;
      break;

    case OPT_TRACE:
      cmdInfo.traceFlags = popt.GetOptArg();
      break;

    case OPT_UNREGISTER:
      cmdInfo.m_nShellCommand = CCommandLineInfo::AppUnregister;
      unregistering = true;
      break;

    }
  }

  if (option < -1)
  {
    MIKTEX_FATAL_ERROR(T_("Invalid Yap command-line."));
  }

  vector<string> leftovers = popt.GetLeftovers();

  // parse the rest
  for (const string & arg : leftovers)
  {
    if (_stricmp(arg.c_str(), "/dde") == 0)
    {
      AfxOleSetUserCtrl(FALSE);
      cmdInfo.m_nShellCommand = CCommandLineInfo::FileDDE;
    }
    else if (cmdInfo.m_strFileName.IsEmpty())
    {
      // we have a file name argument; append .dvi to the file
      // name, if necessary
      PathName pathFileName = arg;
      if (!pathFileName.HasExtension(".dvi"))
      {
        pathFileName.Append(".dvi", false);
      }

      // locate the .dvi file
      PathName pathDvi;
      if (!session->FindFile(pathFileName.Get(), FileType::DVI, pathDvi))
      {
        pathDvi = pathFileName;
      }

      cmdInfo.m_strFileName = UT_(pathDvi.Get());

      if (cmdInfo.m_nShellCommand != CCommandLineInfo::FilePrint)
      {
        cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
      }
    }
  }
}

BEGIN_MESSAGE_MAP(YapApplication, CWinApp)
  ON_COMMAND(ID_APP_ABOUT, &YapApplication::OnAppAbout)
  ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
  ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
  ON_COMMAND(ID_VIEW_TRACE, &YapApplication::OnViewTrace)
  ON_UPDATE_COMMAND_UI(ID_VIEW_TRACE, &YapApplication::OnUpdateViewTrace)
  ON_COMMAND(ID_REGISTER_MIKTEX, &YapApplication::OnRegisterMiKTeX)
END_MESSAGE_MAP();

YapApplication::YapApplication()
{
  SetAppID(UT_("MiKTeXorg.MiKTeX.Yap." MIKTEX_COMPONENT_VERSION_STR));
  EnableHtmlHelp();
}

namespace {
  bool classRegistered = false;
}

void YapApplication::RegisterWindowClass()
{
  if (!classRegistered)
  {
    WNDCLASS wndcls;
    memset(&wndcls, 0, sizeof(WNDCLASS));
    wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = ::DefWindowProc;
    wndcls.hInstance = AfxGetInstanceHandle();
    wndcls.hIcon = LoadIcon(IDR_MAINFRAME);
    wndcls.hCursor = LoadCursor(IDC_ARROW);
    wndcls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wndcls.lpszMenuName = nullptr;
    wndcls.lpszClassName = YAP_WND_CLASS;
    if (!AfxRegisterClass(&wndcls))
    {
      MIKTEX_UNEXPECTED();
    }
    else
    {
      classRegistered = true;
    }
  }
}

namespace {
  bool initialized = false;
  const char * const COMMERCIAL_INVOKERS =
    // yap invokers
    "winedt"              // http://www.winedt.com
    ";" "Inlage 4"        // http://www.inlage.com/
    ";" "CodingStudio"    // http://www.dfordsoft.com/
#if 0
    // latex invokers
    ";" "aurora"          // http://elevatorlady.ca/
#endif
    ;
}

BOOL YapApplication::InitInstance()
{
  INITCOMMONCONTROLSEX initCtrls;
  initCtrls.dwSize = sizeof(initCtrls);
  initCtrls.dwICC = ICC_WIN95_CLASSES;
  if (!InitCommonControlsEx(&initCtrls))
  {
    AfxMessageBox(T_(_T("The application could not be initialized (1).")), MB_ICONSTOP | MB_OK);
    return FALSE;
  }

  if (!CWinApp::InitInstance())
  {
    AfxMessageBox(T_(_T("The application could not be initialized (2).")), MB_ICONSTOP | MB_OK);
    return FALSE;
  }

  EnableTaskbarInteraction(FALSE);

  // don't use COINIT_MULTITHREADED; see KB828643
  if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
  {
    AfxMessageBox(T_(_T("The application could not be initialized (3).")), MB_ICONSTOP | MB_OK);
    return FALSE;
  }

  try
  {
    // initialize MiKTeX Library
    session = Session::Create(Session::InitInfo("yap"));

    trace_yap = TraceStream::Open("yap");
    trace_error = TraceStream::Open("error");

    // get command-line arguments
    YapCommandLineInfo cmdInfo;
    ParseYapCommandLine(TU_(m_lpCmdLine), cmdInfo);

    // set trace flags
    if (!cmdInfo.traceFlags.empty())
    {
      tracing = true;
      traceFlags = cmdInfo.traceFlags;
      TraceStream::SetTraceFlags(cmdInfo.traceFlags.c_str());
    }

    YapLog(T_("Yap arguments: %s"), TU_(m_lpCmdLine));

    // return, if another application instance was found
    if (cmdInfo.singleInstance && ActivateFirstInstance(cmdInfo))
    {
      trace_yap->Close();
      trace_error->Close();
      session = nullptr;
      return FALSE;
    }

    // register the app window class
    RegisterWindowClass();

    // search the help file
    PathName helpFileName;
    if (session->FindFile(MIKTEX_YAP_HELP_FILE, MIKTEX_PATH_TEXMF_PLACEHOLDER MIKTEX_PATH_DIRECTORY_DELIMITER_STRING MIKTEX_PATH_MIKTEX_DOC_DIR MIKTEX_PATH_RECURSION_INDICATOR, helpFileName))
    {
      MIKTEX_ASSERT(_CrtIsValidHeapPointer(m_pszHelpFilePath));
      free(reinterpret_cast<void*>(const_cast<LPTSTR>(m_pszHelpFilePath)));
      m_pszHelpFilePath = _tcsdup(UT_(helpFileName.Get()));
    }

    // change the registry key under which our settings are stored
    if (!session->IsMiKTeXPortable())
    {
      SetRegistryKey(UT_(MIKTEX_COMP_COMPANY_STR "\\" MIKTEX_PRODUCTNAME_STR "\\" MIKTEX_SERIES_STR));
    }

    // load standard ini file options (including MRU)
    LoadStdProfileSettings();

    // read Yap configuration settings
    g_pYapConfig = auto_ptr<YapConfig>(new YapConfig());

    // now we are initialized
    initialized = true;

    // register the application's document templates
    CMultiDocTemplate * pDocTemplate;
    pDocTemplate = new CMultiDocTemplate(IDR_DVITYPE, RUNTIME_CLASS(DviDoc), RUNTIME_CLASS(ChildFrame), RUNTIME_CLASS(DviView));
    AddDocTemplate(pDocTemplate);

    // create main MDI Frame window
    MainFrame * pMainFrame = new MainFrame;
    if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
    {
      MIKTEX_UNEXPECTED();
    }
    m_pMainWnd = pMainFrame;

    // enable drag/drop open
    m_pMainWnd->DragAcceptFiles();
    YapApplication::EnableShellOpen();

    // check to see if Yap is the default viewer
    if (!session->IsMiKTeXPortable()
      && (registering || g_pYapConfig->checkFileTypeAssociations)
      && !unregistering)
    {
      _TCHAR szClass[BufferSizes::MaxPath];
      long size = sizeof(_TCHAR) * BufferSizes::MaxPath;
      if ((::RegQueryValue(HKEY_CLASSES_ROOT, _T(".dvi"), szClass, &size) == ERROR_SUCCESS)
        && !Utils::EqualsIgnoreCase(TU_(szClass), Utils::MakeProgId("dvi"))
        && (registering || (AfxMessageBox(IDP_FILE_ASSOCIATION, MB_YESNO, IDP_FILE_ASSOCIATION) == IDYES)))
      {
        // remove .dvi file association; will be restored later
        SHDeleteKey(HKEY_CLASSES_ROOT, _T(".dvi"));
      }
    }

    // register .dvi file association
    if (!session->IsMiKTeXPortable())
    {
      RegisterShellFileTypes(TRUE);
    }

    // dispatch commands specified on the command line
    if (!ProcessShellCommand(cmdInfo))
    {
      return FALSE;
    }

    unsigned showSplashWindow = 0;

    vector<string> invokers = Process2::GetInvokerNames();
    for (vector<string>::const_iterator it = invokers.begin(); it != invokers.end(); ++it)
    {
      if (StringUtil::Contains(COMMERCIAL_INVOKERS, PathName(*it).GetFileNameWithoutExtension().Get()))
      {
        showSplashWindow = 5;
      }
    }

    if (!g_pYapConfig->showSplashWindow)
    {
      showSplashWindow = 0;
    }

    if (showSplashWindow > 0)
    {
      splashWindow.Show(showSplashWindow);
    }

    // the main window has been initialized, so show and update it
    CWindowPlacement wp;
    if (!wp.Restore(_T("Settings"), pMainFrame))
    {
      pMainFrame->ShowWindow(m_nCmdShow);
    }
    pMainFrame->UpdateWindow();

    // perform a forward search, if requested
    if (cmdInfo.sourceLineNum >= 0)
    {
      FindSrcSpecial(cmdInfo.sourceLineNum, cmdInfo.sourceFile.c_str());
    }

    // perform a hyper-goto, if requested
    if (!cmdInfo.hyperLabel.empty())
    {
      GotoHyperLabel(cmdInfo.hyperLabel.c_str());
    }

    return TRUE;
  }

  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(0, e);
    return FALSE;
  }

  catch (const exception & e)
  {
    ErrorDialog::DoModal(0, e);
    return FALSE;
  }
}

class AboutDialog :
  public CDialog
{
private:
  enum { IDD = IDD_ABOUTBOX };

protected:
  DECLARE_MESSAGE_MAP();

public:
  AboutDialog() :
    CDialog(AboutDialog::IDD)
  {
  }

protected:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  afx_msg void OnClickRegisterMiKTeX();

private:
  shared_ptr<Session> session = Session::Get();
};

void AboutDialog::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);
  if (!pDX->m_bSaveAndValidate)
  {
    CString str;
    str.Format(T_(_T("Yet Another Previewer %s")), static_cast<LPTSTR>(UT_((MIKTEX_COMPONENT_VERSION_STR))));
    str += _T("\r\n");
    str += _T(MIKTEX_COMP_COPYRIGHT_STR);
    GetDlgItem(IDC_THE_NAME_OF_THE_GAME)->SetWindowText(str);
  }
}

BEGIN_MESSAGE_MAP(AboutDialog, CDialog)
  ON_BN_CLICKED(ID_REGISTER_MIKTEX, &AboutDialog::OnClickRegisterMiKTeX)
END_MESSAGE_MAP();

void AboutDialog::OnClickRegisterMiKTeX()
{
  try
  {
#if HAVE_MIKTEX_USER_INFO
    MiKTeXUserInfo info;
    session->RegisterMiKTeXUser(info);
#else
    Utils::RegisterMiKTeXUser();
#endif
  }
  catch (const OperationCancelledException &)
  {
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void YapApplication::OnAppAbout()
{
  AboutDialog dlg;
  dlg.DoModal();
}

int YapApplication::ExitInstance()
{
  try
  {
    // save YAP configuration settings
    if (!(registering || unregistering) && initialized)
    {
      try
      {
        MIKTEX_ASSERT(g_pYapConfig != nullptr);
        g_pYapConfig->Save();
        g_pYapConfig = nullptr;
      }
      catch (const exception &)
      {
      }
    }

    // unregister app window class
    if (classRegistered)
    {
      UnregisterClass(YAP_WND_CLASS, AfxGetInstanceHandle());
      classRegistered = false;
    }

    if (trace_yap != nullptr)
    {
      trace_yap->Close();
      trace_yap = nullptr;
    }

    if (trace_error != nullptr)
    {
      trace_error->Close();
      trace_error = nullptr;
    }

    // uninitialize MiKTeX Library
    session = nullptr;
  }

  catch (const exception &)
  {
  }

  CoUninitialize();

  return CWinApp::ExitInstance();
}

void DdeExecute(const char * server, const char * topic, const char * command);

bool YapApplication::ActivateFirstInstance(const YapCommandLineInfo & cmdInfo)
{
  // check to see whether there is another Yap instance running
  CWnd * pwndPrev = CWnd::FindWindow(YAP_WND_CLASS, nullptr);
  if (pwndPrev == nullptr)
  {
    return false;           // we are alone
  }

  CWnd * wndChild = pwndPrev->GetLastActivePopup();
  MIKTEX_ASSERT(wndChild != nullptr);

  // restore the other app window
  if (pwndPrev->IsIconic())
  {
    pwndPrev->ShowWindow(SW_RESTORE);
  }
  wndChild->SetForegroundWindow();

  if (cmdInfo.m_strFileName.IsEmpty())
  {
    return true;
  }

  // open the DVI document via DDE
  PathName path(cmdInfo.m_strFileName);
  path.MakeAbsolute();

  CStringA ddeCommand;
  ddeCommand.Format("[open(\"%s\")]", path.Get());
  DdeExecute("yap", "system", ddeCommand);

  // delegate DVI search
  if (cmdInfo.sourceLineNum >= 0)
  {
    ddeCommand.Format("[findsrc(\"%d %s\")]", static_cast<int>(cmdInfo.sourceLineNum), cmdInfo.sourceFile.c_str());
    DdeExecute("yap", "system", ddeCommand);
  }

  // delegate hyper-goto
  if (!cmdInfo.hyperLabel.empty())
  {
    ddeCommand.Format("[gotohyperlabel(\"%s\")]", cmdInfo.hyperLabel.c_str());
    DdeExecute("yap", "system", ddeCommand);
  }

  return true;
}

HDDEDATA CALLBACK YapClientDDECallback(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, ULONG_PTR, ULONG_PTR)
{
  return nullptr;
}

#if defined(MIKTEX_WINDOWS) && defined(DDE_FACK)
struct DdeUninitialize_
{
public:
  void operator() (unsigned long inst) const
  {
    if (!DdeUninitialize(inst))
    {
    }
  }
};

typedef AutoResource<unsigned long, DdeUninitialize_> AutoDdeUninitialize;
#endif

#if defined(MIKTEX_WINDOWS) && defined(DDE_FACK)
struct DdeFreeDataHandle_
{
public:
  void operator() (HDDEDATA hData) const
  {
    if (!DdeFreeDataHandle(hData))
    {
    }
  }
};

typedef AutoResource<HDDEDATA, DdeFreeDataHandle_> AutoDdeFreeDataHandle;
#endif

#if defined(MIKTEX_WINDOWS) && defined(DDE_FACK)
struct DdeDisconnect_
{
public:
  void operator() (HCONV hConv) const
  {
    if (!DdeDisconnect(hConv))
    {
    }
  }
};

typedef AutoResource<HCONV, DdeDisconnect_> AutoDdeDisconnect;
#endif

void DdeExecute(const char * lpszServer, const char * lpszTopic, const char * lpszCommand)
{
  YapLog("DdeExecute(\"%s\", \"%s\", \"%s\")", lpszServer, lpszTopic, lpszCommand);
  unsigned long inst = 0;
  UINT result = DdeInitialize(&inst, YapClientDDECallback, APPCMD_CLIENTONLY, 0);
  if (result != DMLERR_NO_ERROR)
  {
    MIKTEX_FATAL_ERROR_2(T_("DDE communication could not be initiated."), "result", std::to_string(result));
  }
  AutoDdeUninitialize autoDdeUninitialize(inst);
  HSZ hszServer = DdeCreateStringHandle(inst, UT_(lpszServer), CP_WINNEUTRAL);
  if (hszServer == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("DDE communication could not be initiated."), "inst", std::to_string(DdeGetLastError(inst)));
  }
  AutoDdeFreeStringHandle autoDdeFreeStringHandle1(inst, hszServer);
  HSZ hszTopic = DdeCreateStringHandle(inst, UT_(lpszTopic), CP_WINNEUTRAL);
  if (hszTopic == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("DDE communication could not be initiated."), "inst", std::to_string(DdeGetLastError(inst)));
  }
  AutoDdeFreeStringHandle autoDdeFreeStringHandle2(inst, hszTopic);
  HCONV hconv = DdeConnect(inst, hszServer, hszTopic, 0);
  if (hconv == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("DDE connection could not be established."), "inst", std::to_string(DdeGetLastError(inst)));
  }
  AutoDdeDisconnect autoDdeDisconnect(hconv);
  HDDEDATA hddedata;
#if defined(_UNICODE)
  UINT fmt = CF_UNICODETEXT;
#else
  UINT fmt = CF_TEXT;
#endif
  hddedata = DdeCreateDataHandle(inst, reinterpret_cast<BYTE*>(static_cast<LPTSTR>(UT_(lpszCommand))), static_cast<unsigned long>((strlen(lpszCommand) + 1) * sizeof(_TCHAR)), 0, nullptr, fmt, 0);
  if (hddedata == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("DDE connection could not be established."), "inst", std::to_string(DdeGetLastError(inst)));
  }
  AutoDdeFreeDataHandle autoDdeFreeDataHandle(hddedata);
  if (DdeClientTransaction(reinterpret_cast<BYTE*>(hddedata), static_cast<unsigned long>(-1), hconv, nullptr, 0, XTYP_EXECUTE, 10000, nullptr) == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("DDE transaction failed."), "inst", std::to_string(DdeGetLastError(inst)));
  }
}

namespace {
  bool ddeServing = false;
}

BOOL YapApplication::OnDDECommand(LPTSTR lpszCommand)
{
  ddeServing = true;

  BOOL done = FALSE;

  try
  {
    YapLog("OnDDECommand(\"%s\")", static_cast<const char *>(TU_(lpszCommand)));

    done = CWinApp::OnDDECommand(lpszCommand);

    if (!done)
    {
      CString ddeCommand = lpszCommand;
      if (ddeCommand.Left(10) == _T("[findsrc(\""))
      {
        CString src = ddeCommand.Right(ddeCommand.GetLength() - 10);
        int i = src.Find(_T('"'));
        if (i != -1)
        {
          src = src.Left(i);
          LPTSTR lpszFileName = nullptr;
          long line = _tcstol(src, &lpszFileName, 10);
          while (*lpszFileName == _T(' '))
          {
            ++lpszFileName;
          }
          FindSrcSpecial(line, TU_(lpszFileName));
          done = TRUE;
        }
      }
      else if (ddeCommand.Left(17) == _T("[gotohyperlabel(\""))
      {
        CString label = ddeCommand.Right(ddeCommand.GetLength() - 17);
        int i = label.Find(_T('"'));
        if (i != -1)
        {
          label = label.Left(i);
          GotoHyperLabel(TU_(label));
          done = TRUE;
        }
      }
    }
  }

  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  catch (const exception & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  ddeServing = false;

  return done;
}

bool YapApplication::FindSrcSpecial(int line, const char * lpszFileName)
{
  POSITION posTemplate = GetFirstDocTemplatePosition();
  while (posTemplate != nullptr)
  {
    CDocTemplate * pTemplate = GetNextDocTemplate(posTemplate);
    POSITION posDoc = pTemplate->GetFirstDocPosition();
    while (posDoc != nullptr)
    {
      CDocument * pDoc = pTemplate->GetNextDoc(posDoc);
      if (!pDoc->IsKindOf(RUNTIME_CLASS(DviDoc)))
      {
        continue;
      }
      DviDoc * pDviDoc = reinterpret_cast<DviDoc*>(pDoc);
      POSITION posView = pDviDoc->GetFirstViewPosition();
      while (posView != nullptr)
      {
        CView * pView = pDviDoc->GetNextView(posView);
        if (!pView->IsKindOf(RUNTIME_CLASS(DviView)))
        {
          continue;
        }
        DviView * pDviview = reinterpret_cast<DviView*>(pView);
        if (pDviview->GotoSrcSpecial(line, lpszFileName))
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool YapApplication::GotoHyperLabel(const char * lpszLabel)
{
  string hashLabel;
  hashLabel = '#';
  hashLabel += lpszLabel;
  POSITION posTemplate = GetFirstDocTemplatePosition();
  while (posTemplate != nullptr)
  {
    CDocTemplate * pTemplate = GetNextDocTemplate(posTemplate);
    POSITION posDoc = pTemplate->GetFirstDocPosition();
    while (posDoc != nullptr)
    {
      CDocument * pDoc = pTemplate->GetNextDoc(posDoc);
      if (!pDoc->IsKindOf(RUNTIME_CLASS(DviDoc)))
      {
        continue;
      }
      DviDoc * pDviDoc = reinterpret_cast<DviDoc*>(pDoc);
      POSITION posView = pDviDoc->GetFirstViewPosition();
      while (posView != nullptr)
      {
        CView * pView = pDviDoc->GetNextView(posView);
        if (!pView->IsKindOf(RUNTIME_CLASS(DviView)))
        {
          continue;
        }
        DviView * pDviView = reinterpret_cast<DviView *>(pView);
        if (pDviView->Navigate(hashLabel.c_str(), false))
        {
          return true;
        }
      }
    }
  }
  return false;
}

void StartEditor(const char * lpszFileName, const char * lpszDocDir, int line)
{
  shared_ptr<Session> session = Session::Get();

  // find the source file
  PathName path;
  if (Utils::IsAbsolutePath(lpszFileName) || lpszDocDir == nullptr)
  {
    path = lpszFileName;
  }
  else
  {
    path.Set(lpszDocDir, lpszFileName);
  }
  PathName path2;
  if (!session->FindFile(path.Get(), FileType::TEX, path2))
  {
    MIKTEX_FATAL_ERROR_2(T_("The source file could not be found."), "fileName", lpszFileName);
  }

  // make command line
  string commandLine;
  const char * lpsz = g_pYapConfig->inverseSearchCommandLine.c_str();
  bool haveName = false;
  bool haveLine = false;
  while (*lpsz != 0)
  {
    if (*lpsz == '%')
    {
      switch (lpsz[1])
      {
      case '%':
        commandLine += '%';
        break;
      case 'f':
        commandLine += path2.Get();
        haveName = true;
        break;
      case 'l':
        commandLine += std::to_string(line);
        haveLine = true;
        break;
      default:
        MIKTEX_FATAL_ERROR_2(T_("The editor command is not valid."), "command", g_pYapConfig->inverseSearchCommandLine);
      }
      lpsz += 2;
    }
    else
    {
      commandLine += *lpsz++;
    }
  }

  if (!haveName)
  {
    if (AfxMessageBox(IDP_CHOOSE_EDITOR, MB_YESNO, IDP_CHOOSE_EDITOR) != IDYES)
    {
      return;
    }
    CPropertySheet dlg(T_(_T("Options")), 0, 0);
    InverseSearchOptionsPage pageIsearch;
    dlg.AddPage(&pageIsearch);
    if (dlg.DoModal() == IDOK)
    {
      // RECUSION
      StartEditor(lpszFileName, lpszDocDir, line);
    }
    return;
  }

  // start process
  STARTUPINFO startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);
  startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
  startupInfo.wShowWindow = SW_SHOWNORMAL;
  PROCESS_INFORMATION processInfo;
  if (!::CreateProcess(nullptr, UT_(commandLine), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInfo))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateProcess", "commandLine", commandLine);
  }
  CloseHandle(processInfo.hThread);
  CloseHandle(processInfo.hProcess);
}

void VYapLog(const char * lpszFormat, va_list argptr)
{
  if (theApp.trace_yap != nullptr)
  {
    theApp.trace_yap->VTrace("yap", lpszFormat, argptr);
  }
}

void YapLog(const char * lpszFormat, ...)
{
  if (theApp.trace_yap != nullptr && theApp.trace_yap->IsEnabled())
  {
    va_list argptr;
    va_start(argptr, lpszFormat);
    VYapLog(lpszFormat, argptr);
    va_end(argptr);
  }
}

void TraceError(const char * lpszFormat, ...)
{
  if (theApp.trace_error != nullptr)
  {
    va_list argptr;
    va_start(argptr, lpszFormat);
    theApp.trace_error->VTrace("yap", lpszFormat, argptr);
    va_end(argptr);
  }
}

CDocument * YapApplication::OpenDocumentFile(LPCTSTR lpszFileName)
{
  try
  {
    PathName pathShort(lpszFileName);
#if defined(REMOVE_BLANKS_FROM_DOCUMENT_FILENAMES)
    Utils::RemoveBlanksFromPathName(pathShort);
#endif
    return CWinApp::OpenDocumentFile(UT_(pathShort.Get()));
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(nullptr, e);
    return 0;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(nullptr, e);
    return 0;
  }
}

void UpdateAllDviViews(bool reread)
{
  YapApplication * pYapApp = reinterpret_cast<YapApplication*>(AfxGetApp());
  MIKTEX_ASSERT(pYapApp != nullptr);
  POSITION posTemplate = pYapApp->GetFirstDocTemplatePosition();
  while (posTemplate != nullptr)
  {
    CDocTemplate * pTemplate = pYapApp->GetNextDocTemplate(posTemplate);
    POSITION posDoc = pTemplate->GetFirstDocPosition();
    while (posDoc != nullptr)
    {
      CDocument * pDoc = pTemplate->GetNextDoc(posDoc);
      if (pDoc->IsKindOf(RUNTIME_CLASS(DviDoc)))
      {
        DviDoc * pDviDoc = reinterpret_cast<DviDoc*>(pDoc);
        if (reread)
        {
          pDviDoc->Reread();
        }
        pDviDoc->UpdateAllViews(0);
      }
    }
  }
}

string GetCommandPrefix(bool clear)
{
  ASSERT_VALID(AfxGetApp());
  ASSERT_VALID(AfxGetApp()->m_pMainWnd);
  MainFrame * pMain = reinterpret_cast<MainFrame*>(AfxGetApp()->m_pMainWnd);
  return pMain->GetCommandPrefix(clear);
}

bool AddCommandPrefixChar(char ch)
{
  ASSERT_VALID(AfxGetApp());
  ASSERT_VALID(AfxGetApp()->m_pMainWnd);
  MainFrame * pMain = reinterpret_cast<MainFrame*>(AfxGetApp()->m_pMainWnd);
  return pMain->AddCommandPrefixChar(ch);
}

void YapApplication::OnViewTrace()
{
  try
  {
    tracing = !tracing;
    if (tracing)
    {
      if (!traceFlags.empty())
      {
        traceFlags = YAP_TRACE_FLAGS_LVL_3;
      }
    }
    TraceStream::SetTraceFlags(tracing ? traceFlags.c_str() : nullptr);
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(nullptr, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(nullptr, e);
  }
}

void YapApplication::OnUpdateViewTrace(CCmdUI * pCmdUI)
{
  pCmdUI->SetCheck(tracing ? 1 : 0);
}


void YapApplication::OnRegisterMiKTeX()
{
  try
  {
#if HAVE_MIKTEX_USER_INFO
    MiKTeXUserInfo info;
    session->RegisterMiKTeXUser(info);
#else
    Utils::RegisterMiKTeXUser();
#endif
  }
  catch (const OperationCancelledException &)
  {
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(nullptr, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(nullptr, e);
  }
}

string trim(const string & s)
{
  const size_t start = s.find_first_not_of(" \t");
  if (start == string::npos)
  {
    return "";
  }
  size_t end = s.find_last_not_of(" \t");
  return s.substr(start, end - start + 1);
}

bool AllowShellCommand(const char * lpszCommand)
{
  switch (g_pYapConfig->enableShellCommands)
  {
  default:
  case YapConfig::SEC_DISABLE_COMMANDS:
    return false;
  case YapConfig::SEC_ASK_USER:
  {
    CString message;
    // FIXME: hard-coded string
    message.Format((T_(_T("The following script is embedded in the document:\n\n%s\n\nDo you allow to execute this script?"))), static_cast<LPTSTR>(UT_(lpszCommand)));
    return AfxMessageBox(message, MB_YESNO | MB_ICONQUESTION) == IDYES;
  }
  case YapConfig::SEC_SECURE_COMMANDS:
  {
    string name = trim(lpszCommand);
    for (Tokenizer tok(g_pYapConfig->secureCommands.c_str(), " ,;"); tok.GetCurrent() != 0; ++tok)
    {
      if (PathName::Compare(tok.GetCurrent(), name) == 0)
      {
        return true;
      }
    }
    return false;
  }
  case YapConfig::SEC_ENABLE_ALL_COMMANDS:
    return true;
  }
}
