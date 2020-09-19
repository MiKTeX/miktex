/* yap.h: main header file for the Yap application      -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

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

#pragma once

#include "resource.h"
#include "SplashWindow.h"

#define SIXTEENBITGDI 0

#define USEMYSCROLLVIEW SIXTEENBITGDI
#define WIN95NOCONTVIEW 0

const bool DEFAULT_bRenderGraphicsInBackground = true;

#define OUT_OF_MEMORY(functionName) MIKTEX_INTERNAL_ERROR()

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#if !(defined(_UNICODE) && defined(UNICODE))
#  error UNICODE compilation required
#endif

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

struct EditorInfo
{
public:
  EditorInfo(const string& name, const string& program, const string& arguments) :
    name(name),
    program(program),
    arguments(arguments)
  {
  }

public:
  EditorInfo(const char* lpszName, const char* lpszProgram, const char* lpszArguments) :
    name(lpszName),
    program(lpszProgram),
    arguments(lpszArguments)
  {
  }

public:
  string name;

public:
  string program;

public:
  string arguments;

public:
  string GetCommandLine() const
  {
    string ret = Q_(program);
    ret += ' ';
    ret += arguments;
    return ret;
  }
};

enum class Unit
{
  BigPoints,
  Centimeters,
  Inches,
  Millimeters,
  None,
  Picas
};

class YapConfig
{
public:
  bool maintainHorizontalPosition;

public:
  bool maintainVerticalPosition;

public:
  DWORD displayMetafontMode;

public:
  DWORD printerMetafontMode;

public:
  string inverseSearchCommandLine;

public:
  int pageXShift;

public:
  int pageYShift;

public:
  int displayShrinkFactor;

public:
  bool showSourceSpecials;

public:
  string dvipsExtraOptions;

public:
  int lastTool;

public:
  int magGlassOpacity;

public:
  bool magGlassHidesCursor;

public:
  int magGlassSmallWidth;

public:
  int magGlassSmallHeight;

public:
  int magGlassSmallShrinkFactor;

public:
  int magGlassMediumWidth;

public:
  int magGlassMediumHeight;

public:
  int magGlassMediumShrinkFactor;

public:
  int magGlassLargeWidth;

public:
  int magGlassLargeHeight;

public:
  int magGlassLargeShrinkFactor;

public:
  int enableShellCommands;

public:
  enum {
    SEC_DISABLE_COMMANDS = 0,
    SEC_ASK_USER = 1,
    SEC_SECURE_COMMANDS = 2,
    SEC_ENABLE_ALL_COMMANDS = 3
  };

public:
  string secureCommands;

public:
  bool checkFileTypeAssociations;

public:
#if defined(MIKTEX_DEBUG)
  bool showBoundingBoxes;
#endif

public:
  bool renderGraphicsInBackground;

public:
  double gamma;

public:
  string gammaValues;

public:
  bool doublePage;

public:
  bool continuousView;

public:
  bool win95;

public:
  DviPageMode dviPageMode;

public:
  Unit unit;

public:
  bool showSplashWindow;

public:
  YapConfig();

public:
  ~YapConfig();

public:
  void Load();

public:
  void Save();

public:
  bool SetDisplayMetafontMode(const char* lpszMetafontMode);

public:
  bool SetPrinterMetafontMode(const char* lpszMetafontMode);

private:
  template<typename ValueType> void PutValue(const char* lpszKeyName, const char* lpszValueName, const ValueType& value)
  {
    pCfg->PutValue(lpszKeyName, lpszValueName, std::to_string(value));
  }

private:
  template<> void PutValue(const char* lpszKeyName, const char* lpszValueName, const bool& value)
  {
    pCfg->PutValue(lpszKeyName, lpszValueName, value ? "1" : "0");
  }

private:
  template<> void PutValue(const char* lpszKeyName, const char* lpszValueName, const string& value)
  {
    pCfg->PutValue(lpszKeyName, lpszValueName, value.c_str());
  }

private:
  template<typename ValueType> void UpdateValue(const char* lpszKeyName, const char* lpszValueName, const ValueType& value, const ValueType& defaultValue)
  {
    if (value == defaultValue)
    {
      string s;
      if (pCfg->TryGetValueAsString(lpszKeyName == nullptr ? "" : lpszKeyName, lpszValueName, s))
      {
        pCfg->DeleteValue(lpszKeyName == nullptr ? "" : lpszKeyName, lpszValueName);
      }
    }
    else
    {
      PutValue(lpszKeyName, lpszValueName, value);
    }
  }

private:
  template<typename ValueType> ValueType GetValue(const char* lpszKeyName, const char* lpszValueName, const ValueType& defaultValue)
  {
    string value;
    if (pCfg->TryGetValueAsString(lpszKeyName == nullptr ? "" : lpszKeyName, lpszValueName, value))
    {
      return static_cast<ValueType>(atoi(value.c_str()));
    }
    else
    {
      return defaultValue;
    }
  }

private:
  template<> bool GetValue(const char* lpszKeyName, const char* lpszValueName, const bool& defaultValue)
  {
    string value;
    if (pCfg->TryGetValueAsString(lpszKeyName == nullptr ? "" : lpszKeyName, lpszValueName, value))
    {
      return atoi(value.c_str()) == 0 ? false : true;
    }
    else
    {
      return defaultValue;
    }
  }

private:
  template<> double GetValue(const char* lpszKeyName, const char* lpszValueName, const double& defaultValue)
  {
    string value;
    if (pCfg->TryGetValueAsString(lpszKeyName == nullptr ? "" : lpszKeyName, lpszValueName, value))
    {
      return atof(value.c_str());
    }
    else
    {
      return defaultValue;
    }
  }

private:
  template<> string GetValue(const char* lpszKeyName, const char* lpszValueName, const string& defaultValue)
  {
    string value;
    if (pCfg->TryGetValueAsString(lpszKeyName == nullptr ? "" : lpszKeyName, lpszValueName, value))
    {
      return value;
    }
    else
    {
      return defaultValue;
    }
  }

private:
  unique_ptr<Cfg> pCfg;

private:
  shared_ptr<Session> session = Session::Get();
};

#if defined(MIKTEX_WINDOWS)
template<class OBJTYPE> class DeleteObject_
{
public:
  void operator()(OBJTYPE* pObj)
  {
    pObj->DeleteObject();
  }
};

typedef AutoResource<CGdiObject*, DeleteObject_<CGdiObject> > AutoDeleteObject;
#endif

#if defined(MIKTEX_WINDOWS)
template<class OBJTYPE> class Detach_
{
public:
  void operator()(OBJTYPE* pObj)
  {
    pObj->Detach();
  }
};

typedef AutoResource<CMenu*, Detach_<CMenu> > AutoDetachMenu;
#endif

#if defined(MIKTEX_WINDOWS)
class RestoreDC_
{
public:
  void operator()(CDC* pDC, int savedDC)
  {
    pDC->RestoreDC(savedDC);
  }
};

typedef AutoResource2<CDC *, int, RestoreDC_> AutoRestoreDC;
#endif

#if defined(MIKTEX_WINDOWS)
class SelectObject_
{
public:
  void operator() (CDC* pDC, CGdiObject* pObj)
  {
    pDC->SelectObject(pObj);
  }
};

typedef AutoResource2<CDC *, CGdiObject *, SelectObject_> AutoSelectObject;
#endif

class YapCommandLineInfo :
  public CCommandLineInfo
{
public:
  string sourceFile;

public:
  string hyperLabel;

public:
  int sourceLineNum = -1;

public:
  bool singleInstance = false;

public:
  string traceOptions;
};

class YapApplication :
  public CWinApp,
  public MiKTeX::Trace::TraceCallback
{
protected:
  DECLARE_MESSAGE_MAP();

public:
  YapApplication();

public:
  BOOL InitInstance() override;

public:
  int ExitInstance() override;

public:
  BOOL OnDDECommand(LPTSTR lpszCommand) override;

public:
  CDocument* OpenDocumentFile(LPCTSTR lpszFileName);

protected:
  afx_msg void OnAppAbout();

protected:
  afx_msg void OnViewTrace();

protected:
  afx_msg void OnUpdateViewTrace(CCmdUI* pCmdUI);

protected:
  afx_msg void OnRegisterMiKTeX();

private:
  void RegisterWindowClass();

private:
  bool ActivateFirstInstance(const YapCommandLineInfo& cmdInfo);

private:
  bool FindSrcSpecial(int lineNum, const char* lpszFileName);

private:
  bool GotoHyperLabel(const char* lpszLabel);

private:
  std::vector<MiKTeX::Trace::TraceCallback::TraceMessage> pendingTraceMessages;

public:
  bool MIKTEXTHISCALL Trace(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage) override;

private:
  void FlushPendingTraceMessages()
  {
    for (const TraceCallback::TraceMessage& msg : pendingTraceMessages)
    {
      TraceInternal(msg);
    }
    pendingTraceMessages.clear();
  }

private:
  void TraceInternal(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage);

protected:
  bool tracing = false;

public:
  std::vector<std::string> traceOptions;

private:
  shared_ptr<Session> session;

private:
  SplashWindow splashWindow;

public:
  unique_ptr<TraceStream> trace_yap;

public:
  unique_ptr<TraceStream> trace_error;
};

extern YapApplication theApp;

template<class VALTYPE> class AutoRestore
{
public:
  AutoRestore(VALTYPE& val) :
    oldVal(val),
    pVal(&val)
  {
  }

public:
  ~AutoRestore()
  {
    *pVal = oldVal;
  }

private:
  VALTYPE oldVal;

private:
  VALTYPE* pVal;
};

bool AddCommandPrefixChar(char ch);

bool AllowShellCommand(const char* lpszCommand);

void DrawTransparentBitmap(HDC, HBITMAP, short, short, COLORREF);

string GetCommandPrefix(bool clear);

DWORD HLStoRGB(WORD, WORD, WORD);

void MakeTransparent(HWND hwnd, COLORREF transparencyColor, unsigned char opacity);

void RGBtoHLS(DWORD, WORD&, WORD&, WORD&);

void StartEditor(const char* lpszFileName, const char* lpszDocDir, int line);

void UpdateAllDviViews(bool reread = false);

void YapInfo(const std::string& line);

void YapError(const std::string& line);

void ShowError(CWnd* parent, const MiKTeX::Core::MiKTeXException& e);

void ShowError(CWnd* parent, const std::exception& e);

enum {
  WM_DVIPROGRESS = WM_APP + 1, WM_MAKEFONTS
};

extern unique_ptr<YapConfig> g_pYapConfig;
