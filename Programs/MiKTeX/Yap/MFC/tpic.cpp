/* tpic.cpp: tpic specials

   Copyright (C) 1996-2018 Christian Schenk

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

/* _________________________________________________________________________

   NOTE: This module is based on prior work as noted below:

   Support drawing routines for TeXsun and TeX
   
   Copyright, (C) 1987, 1988 Tim Morgan, UC Irvine
   Adapted for xdvi by Jeffrey Lee, U. of Toronto
   Adapted for Yap by Christian Schenk
   _________________________________________________________________________ */

#include "StdAfx.h"

#include "yap.h"

#include "DviView.h"

// Tpic dimensions are in thousands of an inch.
inline double CalcTpicConv(int dpi, int mag)
{
  return (dpi * mag) / 1000000.0;
}

void DviView::DrawTpicSpecial(CDC* pDC, TpicSpecial* pTpicSpecial)
{
  tpicConv = CalcTpicConv(GetDocument()->GetResolution(), GetDocument()->GetMagnification());
  CPen pen;
  int penSize = PixelShrink(static_cast<int>(pTpicSpecial->GetPenSize() * tpicConv));
  if (penSize == 0)
  {
    penSize = 1;
  }
  if (!pen.CreatePen(PS_SOLID, penSize, RGB(0, 0, 0)))
  {
    MIKTEX_UNEXPECTED();
  }
  AutoDeleteObject autoDeletePen(&pen);
  CPen* pOldPen = pDC->SelectObject(&pen);
  if (pOldPen == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  AutoSelectObject selectOldPen(pDC, pOldPen);
  TpicPolySpecial* pPolySpecial;
  TpicArcSpecial* pArcSpecial;
  if ((pPolySpecial = dynamic_cast<TpicPolySpecial *>(pTpicSpecial)) != nullptr)
  {
    if (pPolySpecial->IsSpline())
    {
      DrawTpicSpline(pDC, pPolySpecial);
    }
    else
    {
      DrawTpicPoly(pDC, pPolySpecial);
    }
  }
  else if ((pArcSpecial = dynamic_cast<TpicArcSpecial *>(pTpicSpecial)) != nullptr)
  {
    DrawTpicArc(pDC, pArcSpecial);
  }
}

void DviView::DrawTpicPoly(CDC* pDC, TpicPolySpecial* pPoly)
{
  float length;
  switch (pPoly->GetOutlineStyle(length))
  {
  case OutlineStyle::Solid:
    FlushTpicPath(pDC, pPoly->GetX(), pPoly->GetY(), pPoly->GetPath());
    break;
  case OutlineStyle::Dashes:
    FlushTpicPathD(pDC, pPoly->GetX(), pPoly->GetY(), pPoly->GetPath(), length, false);
    break;
  case OutlineStyle::Dots:
    FlushTpicPathD(pDC, pPoly->GetX(), pPoly->GetY(), pPoly->GetPath(), length, true);
    break;
  default:
    // todo
    break;
  }
}

void DviView::DrawTpicSpline(CDC* pDC, TpicPolySpecial* pPoly)
{
  float length;
  switch (pPoly->GetOutlineStyle(length))
  {
  case OutlineStyle::Solid:
    FlushTpicSpline(pDC, pPoly->GetX(), pPoly->GetY(), pPoly->GetPath());
    break;
  default:
    // TODO
    break;
  }
}

const double TWOPI = 3.14159265359 * 2.0;

void DviView::DrawTpicEllipse(CDC* pDC, int x, int y, int xc, int yc, int xr, int yr)
{
  double angle = (xr + yr) / 2.0;
  double theta = sqrt(1.0 / angle);
  int n = static_cast<int>(TWOPI / theta + 0.5);
  if (n < 12)
  {
    n = 12;
  }
  else if (n > 80)
  {
    n = 80;
  }
  n /= 2;
  theta = TWOPI / n;
  angle = 0.0;
  TpicSpecial::point p0;
  TpicSpecial::point p1;
  p0.x = xc + xr;
  p0.y = yc;
  while ((angle += theta) <= TWOPI)
  {
    p1.x = static_cast<int>(xc + xr*cos(angle) + 0.5);
    p1.y = static_cast<int>(yc + yr*sin(angle) + 0.5);
    DrawTpicLine(pDC, x, y, p0, p1);
    p0 = p1;
  }
  p1.x = xc + xr;
  p1.y = yc;
  DrawTpicLine(pDC, x, y, p0, p1);
}

void DviView::DrawTpicArc(CDC* pDC, TpicArcSpecial* pArc)
{
  if (pArc->GetS() <= 0.0 && pArc->GetE() >= 6.282)
  {
    DrawTpicEllipse(pDC, pArc->GetX(), pArc->GetY(), pArc->GetCx(), pArc->GetCy(), pArc->GetRx(), pArc->GetRy());
    return;
  }

  TpicSpecial::path path;
  path.reserve(200);

  double xcenter = pArc->GetCx();
  double ycenter = pArc->GetCy();
  double xradius = pArc->GetRx();
  double yradius = pArc->GetRy();

  double r = (xradius + yradius) / 2.0;
  double theta = sqrt(1.0 / r);

  double start_angle = pArc->GetS();
  double end_angle = pArc->GetE();

  int n = static_cast<int>(0.3 * TWOPI / theta + 0.5);
  if (n < 12)
  {
    n = 12;
  }
  else if (n > 80)
  {
    n = 80;
  }
  n /= 2;

  theta = TWOPI / n;

  TpicSpecial::point pt;

  pt.x = static_cast<int>(xcenter + xradius * cos(start_angle) + 0.5);
  pt.y = static_cast<int>(ycenter + yradius * sin(start_angle) + 0.5);
  path.push_back(pt);

  double angle = start_angle + theta;

  if (end_angle < start_angle)
  {
    end_angle += TWOPI;
  }

  while (angle < end_angle)
  {
    pt.x = static_cast<int>(xcenter + xradius * cos(angle) + 0.5);
    pt.y = static_cast<int>(ycenter + yradius * sin(angle) + 0.5);
    path.push_back(pt);
    angle += theta;
  }

  pt.x = static_cast<int>(xcenter + xradius * cos(end_angle) + 0.5);
  pt.y = static_cast<int>(ycenter + yradius * sin(end_angle) + 0.5);
  path.push_back(pt);

  FlushTpicPath(pDC, pArc->GetX(), pArc->GetY(), path);
}

void DviView::FlushTpicPath(CDC* pDC, int x, int y, const TpicSpecial::path& path)
{
  vector<TpicSpecial::point>::const_iterator it = path.begin();
  if (it == path.end())
  {
    return;
  }
  vector<TpicSpecial::point>::const_iterator it2 = it;
  ++it2;
  for (; it2 != path.end(); ++it, ++it2)
  {
    DrawTpicLine(pDC, x, y, *it, *it2);
  }
}

inline int dist(const TpicSpecial::point& pt0, const TpicSpecial::point& pt1)
{
  return abs(pt0.x - pt1.x) + abs(pt0.y - pt1.y);
}

void DviView::FlushTpicSpline(CDC* pDC, int x, int y, const TpicSpecial::path& path)
{
  TpicSpecial::path path2;
  path2.reserve(path.size() + 2);
  path2.push_back(path.front());
  path2.insert(path2.end(), path.begin(), path.end());
  path2.push_back(path.back());
  TpicSpecial::point ptlast;
  bool lastvalid = false;
  for (unsigned i = 0; i < path2.size() - 2; ++i)
  {
    int steps = (dist(path2[i], path2[i + 1]) + dist(path2[i + 1], path2[i + 2])) / 80;
    for (int j = 0; j < steps; ++j)
    {
      int w = (j * 1000 + 500) / steps;
      int t1 = w * w / 20;
      w -= 500;
      int t2 = (750000 - w * w) / 10;
      w -= 500;
      int t3 = w * w / 20;
      TpicSpecial::point pt;
      pt.x = (t1*path2[i + 2].x + t2*path2[i + 1].x + t3*path2[i].x + 50000) / 100000;
      pt.y = (t1*path2[i + 2].y + t2*path2[i + 1].y + t3*path2[i].y + 50000) / 100000;
      if (lastvalid)
      {
        DrawTpicLine(pDC, x, y, ptlast, pt);
      }
      ptlast = pt;
      lastvalid = true;
    }
  }
}

void DviView::DrawTpicLine(CDC* pDC, int x, int y, const TpicSpecial::point& ptFrom, const TpicSpecial::point& ptTo)
{
  pDC->MoveTo(PixelShrink(static_cast<int>(x + ptFrom.x * tpicConv)), PixelShrink(static_cast<int>(y + ptFrom.y * tpicConv)));
  if (!pDC->LineTo(PixelShrink(static_cast<int>(x + ptTo.x * tpicConv)), PixelShrink(static_cast<int>(y + ptTo.y * tpicConv))))
  {
    MIKTEX_UNEXPECTED();
  }
}

void DviView::FlushTpicPathD(CDC* pDC, int x, int y, const TpicSpecial::path& path, double inchesPerDash, bool dotted)
{
  if (path.size() < 2 || inchesPerDash <= 0.0)
  {
    return;
  }

  double milliperdash = inchesPerDash * 1000.0;

  TpicSpecial::point ptl0;
  TpicSpecial::point ptl1;
  ptl0.x = path[0].x;
  ptl0.y = path[0].y;
  ptl1.x = path[1].x;
  ptl1.y = path[1].y;

  double dx = ptl1.x - ptl0.x;
  double dy = ptl1.y - ptl0.y;

  if (dotted)
  {
    int numdots = static_cast<int>(sqrt(dx*dx + dy*dy) / milliperdash + 0.5);
    if (numdots == 0)
    {
      numdots = 1;
    }
    for (int i = 0; i <= numdots; ++i)
    {
      double a = static_cast<double>(i) / static_cast<double>(numdots);
      int cx0 = static_cast<int>(ptl0.x + a * dx + 0.5);
      int cy0 = static_cast<int>(ptl0.y + a * dy + 0.5);
#if 0
      pDC->SetPixel(PixelShrink(x + cx0 * tpicConv), PixelShrink(y + cy0 * tpicConv), RGB(0, 0, 0)); // fixme
#else
      pDC->MoveTo(PixelShrink(static_cast<int>(x + cx0 * tpicConv)), PixelShrink(static_cast<int>(y + cy0 * tpicConv)));
      if (!pDC->LineTo(PixelShrink(static_cast<int>(x + cx0 * tpicConv))
        + 1, PixelShrink(static_cast<int>(y + cy0 * tpicConv))))
      {
        MIKTEX_UNEXPECTED();
      }
#endif
    }
  }
  else
  {
    double d = sqrt(dx*dx + dy*dy);
    int numdots = static_cast<int>(d / (2.0 * milliperdash) + 1.0);
    if (numdots <= 1)
    {
      DrawTpicLine(pDC, x, y, ptl0, ptl1);
    }
    else
    {
      double spacesize = (d - numdots * milliperdash) / (numdots - 1);
      TpicSpecial::point ptc0;
      TpicSpecial::point ptc1;
      double b = 0;
      for (int i = 0; i < numdots - 1; ++i)
      {
        double a = i * (milliperdash + spacesize) / d;
        b = a + milliperdash / d;
        ptc0.x = static_cast<int>(ptl0.x + a * dx + 0.5);
        ptc0.y = static_cast<int>(ptl0.y + a * dy + 0.5);
        ptc1.x = static_cast<int>(ptl0.x + b * dx + 0.5);
        ptc1.y = static_cast<int>(ptl0.y + b * dy + 0.5);
        DrawTpicLine(pDC, x, y, ptc0, ptc1);
        b += spacesize / d;
      }
      ptc0.x = static_cast<int>(ptl0.x + b * dx + 0.5);
      ptc0.y = static_cast<int>(ptl0.y + b * dy + 0.5);
      DrawTpicLine(pDC, x, y, ptc0, ptl1);
    }
  }
}
