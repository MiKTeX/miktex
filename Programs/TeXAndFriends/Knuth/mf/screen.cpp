/* screen.cpp: METAFONT screen display

   Copyright (C) 1998-2020 Christian Schenk
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

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "mf-miktex.h"

#include "mf.rc"

using namespace std;

constexpr wchar_t* WINDOW_CLASS = L"MF_Screen";

namespace {
  DWORD threadId;
  HANDLE threadHandle;
  HANDLE readyEvent;
  HDC deviceContext;
  HINSTANCE instance;
  HWND window;
  mutex screenMutex;
  POINT screenSize;
}

LRESULT CALLBACK ScreenWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static HBITMAP bitmap;       // <fixme/>

  switch (uMsg)
  {

  case WM_CREATE:
  {
    lock_guard<mutex> lockGuard(screenMutex);
    HDC hPaintDC = GetDC(hWnd);
    deviceContext = CreateCompatibleDC(hPaintDC);
    if (deviceContext == nullptr)
    {
      SetEvent(readyEvent);
      PostQuitMessage(1);
    }
    bitmap = CreateCompatibleBitmap(hPaintDC, screenSize.x, screenSize.y);
    if (bitmap == nullptr)
    {
      DeleteObject(deviceContext);
      deviceContext = nullptr;
      SetEvent(readyEvent);
      PostQuitMessage(1);
    }
    ReleaseDC(hWnd, hPaintDC);
    hPaintDC = nullptr;
    SelectObject(deviceContext, bitmap);
    SelectObject(deviceContext, GetStockObject(LTGRAY_BRUSH));
    SelectObject(deviceContext, GetStockObject(NULL_PEN));
    Rectangle(deviceContext, 0, 0, screenSize.x, screenSize.y);
    return 0;
    break;
  }

  case WM_DESTROY:
  {
    lock_guard<mutex> lockGuard(screenMutex);
    DeleteObject(bitmap);
    bitmap = nullptr;
    DeleteDC(deviceContext);
    deviceContext = nullptr;
    PostQuitMessage(0);
    return 0;
    break;
  }

  case WM_PAINT:
  {
    lock_guard<mutex> lockGuard(screenMutex);
    PAINTSTRUCT ps;
    HDC hPaintDC = BeginPaint(hWnd, &ps);
    BitBlt(hPaintDC, 0, 0, screenSize.x, screenSize.y, deviceContext, 0, 0, SRCCOPY);
    EndPaint(hWnd, &ps);
    return 0;
    break;
  }

  default:
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
  }
}

DWORD WINAPI ScreenThreadFunc(LPVOID lpv)
{
  LPPOINT ppt = reinterpret_cast<LPPOINT>(lpv);
  wstring windowName = fmt::format(L"METAFONT Screen ({0} x {1})", ppt->x, ppt->y);
  window = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, WINDOW_CLASS, windowName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, instance, nullptr);
  if (window == nullptr)
  {
    SetEvent(readyEvent);
    PostQuitMessage(1);
    return 0;
  }
  ShowWindow(window, SW_SHOWNOACTIVATE);
  SetEvent(readyEvent);
  MSG msg;
  while (GetMessageW(&msg, window, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  UnregisterClassW(WINDOW_CLASS, instance);
  return 0;
}

C4P_boolean miktexinitscreen(int screenWidth, int screenHeight)
{
#if defined(MIKTEX_TRAPMF)
  return true;
#endif

  if (!MFAPP.ScreenEnabled())
  {
    return false;
  }

  WNDCLASSW wc = { 0 };
  wc.style = CS_NOCLOSE;
  wc.lpfnWndProc = ScreenWindowProc;
  wc.hInstance = instance;
  wc.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDR_MFSCREEN));
  wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
  wc.lpszClassName = WINDOW_CLASS;
  RegisterClassW(&wc);

  screenSize.x = screenWidth;
  screenSize.y = screenHeight;

  readyEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (readyEvent == nullptr)
  {
    return false;
  }

  threadHandle = CreateThread(nullptr, 0, ScreenThreadFunc, &screenSize, 0, &threadId);
  if (threadHandle == nullptr)
  {
    return false;
  }

  WaitForSingleObject(readyEvent, INFINITE);

  if (window == nullptr)
  {
    return false;
  }

  if (deviceContext == nullptr)
  {
    return false;
  }

  return true;
}

void miktexblankrectangle(MFPROGCLASS::screencol leftColumn, MFPROGCLASS::screencol rightColumn, MFPROGCLASS::screenrow topRow, MFPROGCLASS::screenrow bottomRow)
{
#if defined(MIKTEX_TRAPMF)
  return;
#endif
  lock_guard<mutex> lockGuard(screenMutex);
  HGDIOBJ hgdiobj1 = SelectObject(deviceContext, GetStockObject(WHITE_BRUSH));
  HGDIOBJ hgdiobj2 = SelectObject(deviceContext, GetStockObject(NULL_PEN));
  Rectangle(deviceContext, leftColumn, topRow, rightColumn, bottomRow);
  SelectObject(deviceContext, hgdiobj1);
  SelectObject(deviceContext, hgdiobj2);
}

void miktexupdatescreen()
{
#if defined(MIKTEX_TRAPMF)
  return;
#endif
  lock_guard<mutex> lockGuard(screenMutex);
  // ShowWindow (g_hwnd, SW_SHOWNORMAL);
  InvalidateRect(window, nullptr, FALSE);
}

void miktexpaintrow(MFPROGCLASS::screenrow row, MFPROGCLASS::pixelcolor startColor, MFPROGCLASS::transspec blackWhiteTransitions, MFPROGCLASS::screencol numColumns)
{
#if defined(MIKTEX_TRAPMF)
  return;
#endif
  lock_guard<mutex> lockGuard(screenMutex);
  HGDIOBJ hgdiobj = SelectObject(deviceContext, GetStockObject(BLACK_PEN));
  METAFONTProgram::screencol k = numColumns;
  MoveToEx(deviceContext, blackWhiteTransitions[k], row, nullptr);
  do
  {
    --k;
    if (startColor)
    {
      LineTo(deviceContext, blackWhiteTransitions[k] - 1, row);
    }
    else
    {
      MoveToEx(deviceContext, blackWhiteTransitions[k], row, nullptr);
    }
    startColor = !startColor;
  } while (k);
  SelectObject(deviceContext, hgdiobj);
}

int APIENTRY DllMain(HINSTANCE hInstance, DWORD reason, LPVOID ignore2)
{
  if (reason == DLL_PROCESS_ATTACH)
  {
    instance = hInstance;
  }
  return 1;
}
