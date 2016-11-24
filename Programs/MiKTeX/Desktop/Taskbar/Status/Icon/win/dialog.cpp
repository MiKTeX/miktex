/* StdAfx.cpp:

   Copyright (C) 2009-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "StdAfx.h"
#include "resource.h"

using namespace std;
using namespace MiKTeX::Core;

#define SHOW_DIALOG 0

#define TRAYICONID 1

#define SWM_TRAYMSG             WM_APP

#if SHOW_DIALOG
#define SWM_SHOW                WM_APP + 1
#define SWM_HIDE                WM_APP + 2
#endif

#define SWM_SETTINGS            WM_APP + 3
#define SWM_UPDATE              WM_APP + 4
#define SWM_BROWSE_PACKAGES     WM_APP + 5
#define SWM_PREVIEWER           WM_APP + 6
#define SWM_COMMAND_PROMPT      WM_APP + 7
#define SWM_TEXWORKS            WM_APP + 8
#define SWM_EXIT                WM_APP + 10

static HINSTANCE hInst;
static NOTIFYICONDATAW niData;

void ShowContextMenu(HWND hWnd)
{
  POINT pt;
  GetCursorPos(&pt);
  HMENU hMenu = CreatePopupMenu();
  if (hMenu == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
#if SHOW_DIALOG
  if (IsWindowVisible(hWnd))
  {
    InsertMenuW(hMenu, -1, MF_BYPOSITION, SWM_HIDE, L"Hide");
  }
  else
  {
    InsertMenuW(hMenu, -1, MF_BYPOSITION, SWM_SHOW, L"Show");
  }
#endif
  InsertMenuW(hMenu, -1, MF_BYPOSITION, SWM_TEXWORKS, L"TeXworks");
  InsertMenuW(hMenu, -1, MF_BYPOSITION, SWM_PREVIEWER, L"Yap");
  InsertMenuW(hMenu, -1, MF_BYPOSITION, SWM_COMMAND_PROMPT, L"Command Prompt");
  InsertMenuW(hMenu, -1, MF_SEPARATOR, 0, nullptr);
  InsertMenuW(hMenu, -1, MF_BYPOSITION, SWM_UPDATE, L"Update MiKTeX");
  InsertMenuW(hMenu, -1, MF_BYPOSITION, SWM_BROWSE_PACKAGES, L"MiKTeX Package Manager");
  InsertMenuW(hMenu, -1, MF_BYPOSITION, SWM_SETTINGS, L"MiKTeX Options");
  InsertMenuW(hMenu, -1, MF_SEPARATOR, 0, nullptr);
  InsertMenuW(hMenu, -1, MF_BYPOSITION, SWM_EXIT, L"Exit");
  SetForegroundWindow(hWnd);
  TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, nullptr);
  DestroyMenu(hMenu);
}

static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
  {
    return TRUE;
  }
  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
    {
      EndDialog(hDlg, LOWORD(wParam));
      return TRUE;
    }
    break;
  }
  return FALSE;
}

BOOL OnInitDialog(HWND hWnd)
{
  HMENU hMenu = GetSystemMenu(hWnd, FALSE);
  if (hMenu != nullptr)
  {
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_ABOUT, L"About");
  }
  HICON hIcon = static_cast<HICON>(LoadImageW(hInst, MAKEINTRESOURCEW(IDI_MIKTEX), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
  SendMessageW(hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
  SendMessageW(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));
  return TRUE;
}

static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  try
  {
    shared_ptr<Session> session = Session::Get();

    int wmId, wmEvent;

    switch (message)
    {
    case SWM_TRAYMSG:
      switch (lParam)
      {
#if SHOW_DIALOG
      case WM_LBUTTONDBLCLK:
        ShowWindow(hWnd, SW_RESTORE);
        break;
#endif
      case WM_RBUTTONDOWN:
      case WM_CONTEXTMENU:
        ShowContextMenu(hWnd);
      }
      break;
    case WM_SYSCOMMAND:
      if ((wParam & 0xFFF0) == SC_MINIMIZE)
      {
        ShowWindow(hWnd, SW_HIDE);
        return 1;
      }
      else if (wParam == IDM_ABOUT)
      {
        DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_ABOUT), hWnd, About);
      }
      break;
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      switch (wmId)
      {
#if SHOW_DIALOG
      case SWM_SHOW:
        ShowWindow(hWnd, SW_RESTORE);
        break;
      case SWM_HIDE:
      case IDOK:
        ShowWindow(hWnd, SW_HIDE);
        break;
#endif
      case SWM_TEXWORKS:
      {
        PathName exePath;
        if (session->FindFile(MIKTEX_TEXWORKS_EXE, FileType::EXE, exePath))
        {
          session->UnloadFilenameDatabase();
          Process::Start(exePath);
        }
        break;
      }
      case SWM_SETTINGS:
      {
        PathName exePath;
        if (session->FindFile(MIKTEX_MO_EXE, FileType::EXE, exePath))
        {
          session->UnloadFilenameDatabase();
          Process::Start(exePath);
        }
        break;
      }
      case SWM_UPDATE:
      {
        PathName exePath;
        if (session->FindFile(MIKTEX_UPDATE_EXE, FileType::EXE, exePath))
        {
          session->UnloadFilenameDatabase();
          Process::Start(exePath);
        }
        break;
      }
      case SWM_BROWSE_PACKAGES:
      {
        PathName exePath;
        if (session->FindFile(MIKTEX_MPM_MFC_EXE, FileType::EXE, exePath))
        {
          session->UnloadFilenameDatabase();
          Process::Start(exePath);
        }
        break;
      }
      case SWM_PREVIEWER:
      {
        PathName exePath;
        if (session->FindFile(MIKTEX_YAP_EXE, FileType::EXE, exePath))
        {
          session->UnloadFilenameDatabase();
          Process::Start(exePath);
        }
        break;
      }
      case SWM_COMMAND_PROMPT:
      {
        PathName userBinDir = session->GetSpecialPath(SpecialPath::UserInstallRoot);
        userBinDir /= MIKTEX_PATH_BIN_DIR;
        PathName commonBinDir = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
        commonBinDir /= MIKTEX_PATH_BIN_DIR;
        string newPath;
        if (!session->IsAdminMode())
        {
          if (!newPath.empty())
          {
            newPath += ';';
          }
          newPath += userBinDir.GetData();
        }
        if (commonBinDir != userBinDir || session->IsAdminMode())
        {
          if (!newPath.empty())
          {
            newPath += ';';
          }
          newPath += commonBinDir.GetData();
        }
        string oldPath;
        bool haveOldPath = Utils::GetEnvironmentString("PATH", oldPath);
        if (haveOldPath)
        {
          newPath += ';';
          newPath += oldPath;
        }
        Utils::SetEnvironmentString("PATH", newPath.c_str());
        string cmd;
        if (!Utils::GetEnvironmentString("COMSPEC", cmd))
        {
          cmd = "cmd.exe";
        }
        session->UnloadFilenameDatabase();
        Process::Start(cmd);
        if (haveOldPath)
        {
          Utils::SetEnvironmentString("PATH", oldPath.c_str());
        }
        break;
      }
      case SWM_EXIT:
        DestroyWindow(hWnd);
        break;
      case IDM_ABOUT:
        DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_ABOUT), hWnd, About);
        break;
      }
      return 1;
    case WM_INITDIALOG:
      return OnInitDialog(hWnd);
    case WM_CLOSE:
      DestroyWindow(hWnd);
      break;
    case WM_DESTROY:
      niData.uFlags = 0;
      Shell_NotifyIconW(NIM_DELETE, &niData);
      PostQuitMessage(0);
      break;
    }
    return 0;
  }
  catch (const exception &)
  {
    return 0;
  }
}

static ULONGLONG GetDllVersion(const char * lpszDllName)
{
  ULONGLONG ullVersion = 0;
  DllProc1<HRESULT, DLLVERSIONINFO*> dllGetVersion(lpszDllName, "DllGetVersion");
  DLLVERSIONINFO dvi;
  ZeroMemory(&dvi, sizeof(dvi));
  dvi.cbSize = sizeof(dvi);
  HRESULT hr = dllGetVersion(&dvi);
  if (SUCCEEDED(hr))
  {
    ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion, 0, 0);
  }
  return ullVersion;
}

void InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  InitCommonControls();

  shared_ptr<Session> session = Session::Get();

  if (session->IsMiKTeXPortable())
  {
    PathName initexmf;
    if (session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, initexmf))
    {
      ProcessStartInfo startInfo;
      startInfo.FileName = initexmf.GetData();
      startInfo.Arguments = "--mkmaps";
      AutoFILE nul(fopen("nul", "w"));
      startInfo.StandardOutput = nul.Get();
      startInfo.StandardError = nul.Get();
      Process::Start(startInfo);
    }
  }

  hInst = hInstance;

  HWND hWnd = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_STATUS), nullptr, DlgProc);

  if (hWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }

  ZeroMemory(&niData, sizeof(NOTIFYICONDATA));

  ULONGLONG ullVersion = GetDllVersion("Shell32.dll");
  if (ullVersion >= MAKEDLLVERULL(5, 0, 0, 0))
  {
    niData.cbSize = sizeof(NOTIFYICONDATA);
  }
  else
  {
    niData.cbSize = NOTIFYICONDATA_V2_SIZE;
  }

  niData.uID = TRAYICONID;

  niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

  niData.hIcon = reinterpret_cast<HICON>(LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_MIKTEX), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));

  niData.hWnd = hWnd;

  niData.uCallbackMessage = SWM_TRAYMSG;

  if (session->IsMiKTeXPortable())
  {
    wcscpy_s(niData.szTip, sizeof(niData.szTip), L"MiKTeX Portable");
  }
  else
  {
    wcscpy_s(niData.szTip, sizeof(niData.szTip), L"MiKTeX");
  }

  Shell_NotifyIconW(NIM_ADD, &niData);

  if (niData.hIcon != nullptr && DestroyIcon(niData.hIcon))
  {
    niData.hIcon = nullptr;
  }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t * lpCmdLine, int nCmdShow)
{
  try
  {
    shared_ptr<Session> session = Session::Create(Session::InitInfo("miktex-taskbar-icon"));
    InitInstance(hInstance, nCmdShow);
    HACCEL hAccelTable = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDC_STATUSDIALOG));
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
      if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) || !IsDialogMessage(msg.hwnd, &msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    return msg.wParam;
  }
  catch (const exception &)
  {
    return 0;
  }
}
