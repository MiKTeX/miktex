/* Ruler.cpp:

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

#include "Ruler.h"

#define RULER_COLOR RGB(186, 218, 254)

namespace {
  const _TCHAR * const YAP_RULER_CLASS = _T("MiKTeX_ruler");
  bool classRegistered = false;
}

void MakeTransparent(HWND hwnd, COLORREF transparencyColor, unsigned char opacity)
{
  //const long my_LWA_COLORKEY = 0x00000001;
  const long my_LWA_ALPHA = 0x00000002;
  const LONG_PTR my_WS_EX_LAYERED = 0x00080000;

  static DllProc4<BOOL, HWND, COLORREF, BYTE, DWORD> SetLayeredWindowAttributes_("user32", "SetLayeredWindowAttributes");
#if defined(_WIN64)
  LONG_PTR newStyles;
#else
  LONG newStyles;
#endif
  newStyles = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
  newStyles |= my_WS_EX_LAYERED;
  SetWindowLongPtr(hwnd, GWL_EXSTYLE, newStyles);
  if (!SetLayeredWindowAttributes_(hwnd, transparencyColor, opacity, my_LWA_ALPHA))
  {MIKTEX_FATAL_WINDOWS_ERROR("SetLayeredWindowAttributes");
  }
}

BEGIN_MESSAGE_MAP(Ruler, CWnd)
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP();

Ruler::Ruler(CWnd * pParent, const CSize & size, Unit unit, double pixelsPerInch, Kind kind) :
  pParent(pParent),
  unit(unit),
  kind(kind)
{
  RECT parentSize;
  pParent->GetClientRect(&parentSize);
  if (kind == Horizontal)
  {
    sizeWindow.cx =
      (size.cx < parentSize.right
        ? size.cx + static_cast<int>(pixelsPerInch / 4)
        : parentSize.right);
    sizeWindow.cy = 50;
  }
  else
  {
    sizeWindow.cx = 50;
    sizeWindow.cy =
      (size.cy < parentSize.bottom
        ? size.cy + static_cast<int>(pixelsPerInch / 4)
        : parentSize.bottom);
  }

  switch (unit)
  {
  case Unit::Centimeters:
    pixelsPerUnit = pixelsPerInch / 2.54;
    break;
  case Unit::Inches:
    pixelsPerUnit = pixelsPerInch;
    break;
  case Unit::Millimeters:
    pixelsPerUnit = pixelsPerInch / 25.4;
    break;
  case Unit::Picas:
    pixelsPerUnit = pixelsPerInch / 12.0;
    break;
  case Unit::BigPoints:
    pixelsPerUnit = pixelsPerInch / 72.0;
    break;
  default:
    MIKTEX_UNEXPECTED();
  }
}

void Ruler::OnPaint()
{
#define TICK_POSITION(m, n)                     \
  static_cast<int>((pixelsPerUnit * m)          \
                   + (pixelsPerUnit / 10) * n   \
                   + 0.5)
#define MIN_SPACE 3
#define MARKER_TICK_LENGTH 20
#define TICK_LENGTH(n) \
  (n == 0 ? TICK_LENGTH_1 : (n == 2 ? TICK_LENGTH_2 : TICK_LENGTH_4))
#define TICK_LENGTH_1 15
#define TICK_LENGTH_2 10
#define TICK_LENGTH_3 5
  try
  {
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);
    int lastPosition;
    if (kind == Horizontal)
    {
      lastPosition = sizeWindow.cx;
    }
    else
    {
      lastPosition = sizeWindow.cy;
    }
    CString sample;
    sample.Format(_T("%d"), static_cast<int>(lastPosition / pixelsPerUnit));
    CRect rect(0, 0, 0, 0);
    dc.DrawText(sample, &rect, DT_CALCRECT);
    int markerWidth = rect.right;
    int markerHeight = rect.bottom;
    int markerStep = 1;
    switch (kind)
    {
    case Horizontal:
      if (pixelsPerUnit < markerWidth)
      {
        markerStep = static_cast<int>(markerWidth / pixelsPerUnit + 0.5);
        markerStep = ((markerStep + 5 - 1) / 5) * 5;
      }
      break;
    case Vertical:
      if (kind == Vertical && pixelsPerUnit < markerHeight)
      {
        markerStep =
          static_cast<int>(markerHeight / pixelsPerUnit + 0.5);
        markerStep = ((markerStep + 5 - 1) / 5) * 5;
      }
      break;
    default:
      MIKTEX_UNEXPECTED();
    }
    for (int i1 = 0; TICK_POSITION(i1, 0) < lastPosition; i1 += markerStep)
    {
      CString marker;
      marker.Format(_T("%d"), i1);
      int p = TICK_POSITION(i1, 0);
      UINT format;
      if (kind == Horizontal)
      {
        dc.MoveTo(p, 0);
        dc.LineTo(p, MARKER_TICK_LENGTH);
        format =
          (0
            | DT_TOP
            | DT_SINGLELINE
            | 0);
        if (i1 == 0)
        {
          rect.left = 0;
          rect.right = markerWidth;
        }
        else
        {
          rect.left = p - markerWidth / 2;
          rect.right = rect.left + markerWidth;
          format += DT_CENTER;
        }
        rect.top = MARKER_TICK_LENGTH;
        rect.bottom = rect.top + markerHeight;
      }
      else
      {
        dc.MoveTo(0, p);
        dc.LineTo(MARKER_TICK_LENGTH, p);
        format =
          (0
            | DT_VCENTER
            | DT_SINGLELINE
            | 0);
        if (i1 == 0)
        {
          rect.top = 0;
          rect.bottom = markerHeight;
        }
        else
        {
          rect.top = p - markerHeight / 2;
          rect.bottom = rect.top + markerHeight;
        }
        rect.left = MARKER_TICK_LENGTH;
        rect.right = rect.left + markerWidth;
      }
      dc.DrawText(marker, &rect, format);
    }
    if (pixelsPerUnit > MIN_SPACE)
    {
      for (int i1 = 0; TICK_POSITION(i1, 0) < lastPosition; i1 += 1)
      {
        int p = TICK_POSITION(i1, 0);
        if (kind == Horizontal)
        {
          dc.MoveTo(p, 0);
          dc.LineTo(p, TICK_LENGTH_1);
        }
        else
        {
          dc.MoveTo(0, p);
          dc.LineTo(TICK_LENGTH_1, p);
        }
      }
    }
    if (unit == Unit::Centimeters || unit == Unit::Inches)
    {
      if (pixelsPerUnit / 2 > MIN_SPACE)
      {
        for (int i2 = 0; TICK_POSITION(i2, 5) < lastPosition; i2 += 1)
        {
          int p = TICK_POSITION(i2, 5);
          if (kind == Horizontal)
          {
            dc.MoveTo(p, 0);
            dc.LineTo(p, TICK_LENGTH_2);
          }
          else
          {
            dc.MoveTo(0, p);
            dc.LineTo(TICK_LENGTH_2, p);
          }
        }
      }
      if (pixelsPerUnit / 10 > MIN_SPACE)
      {
        for (int i3 = 0;
        TICK_POSITION((i3 / 10), (i3 % 10)) < lastPosition;
          i3 += 1)
        {
          int p = TICK_POSITION((i3 / 10), (i3 % 10));
          if (kind == Horizontal)
          {
            dc.MoveTo(p, 0);
            dc.LineTo(p, TICK_LENGTH_3);
          }
          else
          {
            dc.MoveTo(0, p);
            dc.LineTo(TICK_LENGTH_3, p);
          }
        }
      }
    }
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

BOOL Ruler::OnEraseBkgnd(CDC * pDC)
{
  try
  {
    ASSERT_VALID(pDC);
    CBrush brushBack(RULER_COLOR);
    CBrush * pOldBrush = pDC->SelectObject(&brushBack);
    if (pOldBrush == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    AutoSelectObject autoSelectOldBrush(pDC, pOldBrush);
    CRect rect;
    if (pDC->GetClipBox(&rect) == ERROR)
    {
      MIKTEX_UNEXPECTED();
    }
    if (!pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY))
    {
      MIKTEX_UNEXPECTED();
    }
    return TRUE;
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
}

BOOL Ruler::PreCreateWindow(CREATESTRUCT & cs)
{
  cs.lpszClass = YAP_RULER_CLASS;
  return CWnd::PreCreateWindow(cs);
}

void Ruler::Create(const CPoint & point)
{
  if (!classRegistered)
  {
    WNDCLASS wndcls;
    ZeroMemory(&wndcls, sizeof(WNDCLASS));
    wndcls.style = 0;
    wndcls.lpfnWndProc = ::DefWindowProc;
    wndcls.hInstance = AfxGetInstanceHandle();
    wndcls.hIcon = nullptr;
    wndcls.hCursor = nullptr;
    wndcls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wndcls.lpszMenuName = nullptr;
    wndcls.lpszClassName = YAP_RULER_CLASS;
    if (!AfxRegisterClass(&wndcls))
    {
      MIKTEX_UNEXPECTED();
    }
    else
    {
      classRegistered = true;
    }
  }
  if (!CreateEx(0, YAP_RULER_CLASS, 0, WS_POPUP | WS_DISABLED, point.x, point.y, sizeWindow.cx, sizeWindow.cy, pParent->m_hWnd, nullptr))
  {
    MIKTEX_UNEXPECTED();
  }
  MakeTransparent(m_hWnd, RGB(255, 255, 255), 128);
  ShowWindow(SW_SHOWNA);
}

void Ruler::MoveRuler(const CPoint & point)
{
  MoveWindow(point.x, point.y, sizeWindow.cx, sizeWindow.cy);
}

void Ruler::PostNcDestroy()
{
  CWnd::PostNcDestroy();
  delete this;
}
