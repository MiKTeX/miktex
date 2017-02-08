/* screen.cpp: METAFONT online displays

   Copyright (C) 1998-2017 Christian Schenk
   Copyright (C) 1998 Wolfgang Kleinschmidt

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
   USA.  */

#include <mutex>

#if defined(_MSC_VER)
#include <conio.h>
#endif

#include "mf-miktex.h"

#include "mf.rc"

using namespace std;

namespace {
  DWORD g_dwThreadID;
  HANDLE g_hEvtReady;
  HANDLE g_hThread;
  HDC g_hdc;
  HINSTANCE g_hinstance;
  HWND g_hwnd;
  mutex screenMutex;
  POINT g_pt;
}

LRESULT CALLBACK ScreenWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static HBITMAP hbitmap;       // <fixme/>

  lock_guard<mutex> lockGuard(screenMutex);

  switch (uMsg)
  {

  case WM_CREATE:
  {
    HDC hPaintDC = GetDC(hwnd);
    g_hdc = CreateCompatibleDC(hPaintDC);
    if (g_hdc == nullptr)
    {
      SetEvent(g_hEvtReady);
      PostQuitMessage(1);
    }
    hbitmap = CreateCompatibleBitmap(hPaintDC, g_pt.x, g_pt.y);
    if (hbitmap == nullptr)
    {
      DeleteObject(g_hdc);
      g_hdc = nullptr;
      SetEvent(g_hEvtReady);
      PostQuitMessage(1);
    }
    ReleaseDC(hwnd, hPaintDC);
    hPaintDC = nullptr;
    SelectObject(g_hdc, hbitmap);
    SelectObject(g_hdc, GetStockObject(LTGRAY_BRUSH));
    SelectObject(g_hdc, GetStockObject(NULL_PEN));
    Rectangle(g_hdc, 0, 0, g_pt.x, g_pt.y);
    return 0;
    break;
  }

  case WM_DESTROY:
    DeleteObject(hbitmap);
    hbitmap = nullptr;
    DeleteDC(g_hdc);
    g_hdc = nullptr;
    PostQuitMessage(0);
    return 0;
    break;

  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hPaintDC = BeginPaint(hwnd, &ps);
    BitBlt(hPaintDC, 0, 0, g_pt.x, g_pt.y, g_hdc, 0, 0, SRCCOPY);
    EndPaint(hwnd, &ps);
    return 0;
    break;
  }

  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

DWORD WINAPI ScreenThreadFunc(LPVOID lpv)
{
  LPPOINT ppt = reinterpret_cast<LPPOINT>(lpv);
  char szWindowName[100];
#if _MSC_VER >= 1400
  sprintf_s(szWindowName, 100, MIKTEXTEXT("METAFONT Screen (%d x %d)"), ppt->x, ppt->y);
#else
  sprintf(szWindowName, MIKTEXTEXT("METAFONT Screen (%d x %d)"), ppt->x, ppt->y);
#endif
  g_hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, "MF_Screen", szWindowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, g_hinstance, nullptr);
  if (g_hwnd == nullptr)
  {
    SetEvent(g_hEvtReady);
    PostQuitMessage(1);
  }
  ShowWindow(g_hwnd, SW_SHOWNOACTIVATE);
  SetEvent(g_hEvtReady);
  MSG msg;
  while (GetMessage(&msg, g_hwnd, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  UnregisterClass("MF_Screen", g_hinstance);
  return 0;
}

C4P_boolean miktexinitscreen(int w, int h)
{
#if defined(MIKTEX_TRAPMF)
  return true;
#endif

  if (!MFAPP.ScreenEnabled())
  {
    return false;
  }

  WNDCLASS wc;
  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = CS_NOCLOSE;
  wc.lpfnWndProc = ScreenWindowProc;
  wc.hInstance = g_hinstance;
  wc.hIcon = LoadIcon(g_hinstance, MAKEINTRESOURCE(IDR_MFSCREEN));
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
  wc.lpszClassName = "MF_Screen";
  RegisterClass(&wc);

  g_pt.x = w;
  g_pt.y = h;

  g_hEvtReady = CreateEvent(0, FALSE, FALSE, 0);
  if (g_hEvtReady == nullptr)
  {
    return false;
  }

  g_hThread = CreateThread(nullptr, 0, ScreenThreadFunc, &g_pt, 0, &g_dwThreadID);
  if (g_hThread == nullptr)
  {
    return false;
  }

  WaitForSingleObject(g_hEvtReady, INFINITE);

  if (g_hwnd == nullptr)
  {
    return false;
  }

  if (g_hdc == nullptr)
  {
    return false;
  }

  return true;
}

void miktexblankrectangle(MFPROGCLASS::screencol left_col, MFPROGCLASS::screencol right_col, MFPROGCLASS::screenrow top_row, MFPROGCLASS::screenrow bot_row)
{
#if defined(MIKTEX_TRAPMF)
  return;
#endif
  lock_guard<mutex> lockGuard(screenMutex);
  HGDIOBJ hgdiobj1 = SelectObject(g_hdc, GetStockObject(WHITE_BRUSH));
  HGDIOBJ hgdiobj2 = SelectObject(g_hdc, GetStockObject(NULL_PEN));
  Rectangle(g_hdc, left_col, top_row, right_col, bot_row);
  SelectObject(g_hdc, hgdiobj1);
  SelectObject(g_hdc, hgdiobj2);
}

void miktexupdatescreen()
{
#if defined(MIKTEX_TRAPMF)
  return;
#endif
  lock_guard<mutex> lockGuard(screenMutex);
  // ShowWindow (g_hwnd, SW_SHOWNORMAL);
  InvalidateRect(g_hwnd, nullptr, FALSE);
}

void miktexpaintrow(MFPROGCLASS::screenrow r, MFPROGCLASS::pixelcolor b, MFPROGCLASS::transspec a, MFPROGCLASS::screencol n)
{
#if defined(MIKTEX_TRAPMF)
  return;
#endif
  lock_guard<mutex> lockGuard(screenMutex);
  HGDIOBJ hgdiobj = SelectObject(g_hdc, GetStockObject(BLACK_PEN));
  METAFONTProgram::screencol k = n;
  MoveToEx(g_hdc, a[k], r, nullptr);
  do
  {
    --k;
    if (b)
    {
      LineTo(g_hdc, a[k] - 1, r);
    }
    else
    {
      MoveToEx(g_hdc, a[k], r, nullptr);
    }
    b = !b;
  } while (k);
  SelectObject(g_hdc, hgdiobj);
}

int APIENTRY DllMain(HINSTANCE hinstance, DWORD reason, LPVOID ignore2)
{
  if (reason == DLL_PROCESS_ATTACH)
  {
    g_hinstance = hinstance;
  }
  return 1;
}
