/* special.cpp: dvi specials

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

#include "StdAfx.h"

#include "internal.h"

float DviImpl::PatternToShadeLevel(const char * lpszTexture)
{
  unsigned long black = 0;
  unsigned long size = 0;

  const char * lpszPattern = lpszTexture;
  while (*lpszPattern)
  {
    switch (*lpszPattern)
    {
    case '0':
      size += 4;
      break;
    case '1':
    case '2':
    case '4':
    case '8':
      black += 1;
      size += 4;
      break;
    case '3':
    case '5':
    case '6':
    case '9':
    case 'a':
    case 'A':
    case 'c':
    case 'C':
      black += 2;
      size += 4;
      break;
    case '7':
    case 'b':
    case 'B':
    case 'd':
    case 'D':
    case 'e':
    case 'E':
      black += 3;
      size += 4;
      break;
    case 'f':
    case 'F':
      black += 4;
      size += 4;
      break;
    case ' ':
      break;
    default:
      trace_error->WriteFormattedLine
        ("libdvi", T_("invalid texture: %s"), lpszTexture);
      return 0.5;
    }
    lpszPattern++;
  }
  return static_cast<float>(1.0
    - (static_cast<float>(black) / static_cast<float>(size)));
}

bool DviImpl::InterpretSpecial(DviPageImpl * pPage, int x, int y, InputStream & inputStream, unsigned long p, DviSpecial * & pSpecial)
{
  CharBuffer<char> autoBuffer(p + 1);
  char * lpszBuf = autoBuffer.GetData();

  for (unsigned long k = 0; k < p; ++k)
  {
    int q = inputStream.ReadByte();
    lpszBuf[k] = static_cast<char>(q);
  }

  lpszBuf[p] = 0;

  char * lpszSpecial = lpszBuf;

  while (isspace(*lpszSpecial))
  {
    ++lpszSpecial;
  }

  bool ret = false;

  pSpecial = nullptr;

  switch (lpszSpecial[0])
  {
  case '!':
    pSpecial = new DviSpecialObject<PsdefSpecialImpl>(pPage, x, y, lpszSpecial);
    ret = true;
    break;
  case '"':
    pSpecial = new DviSpecialObject<DvipsSpecialImpl>(pPage, x, y, lpszSpecial);
    ret = true;
    break;
  case 'a':
    if (strncmp(lpszSpecial, "ar", 2) == 0)
    {
      pSpecial = new DviSpecialObject<TpicSpecialObject<TpicArcSpecialImpl>>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    else if (strncmp(lpszSpecial, "anisoscale", 10) == 0)
    {
      pSpecial = new DviSpecialObject<GraphicsSpecialImpl>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    break;
  case 'b':
    if (strncmp(lpszSpecial, "bk", 2) == 0)
    {
      TpicSpecialRoot::tpicContext.shade = 1.0;
      ret = true;
    }
    break;
  case 'c':
    if (strncmp(lpszSpecial, "center", 6) == 0)
    {
      pSpecial = new DviSpecialObject<GraphicsSpecialImpl>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    else if (strncmp(lpszSpecial, "color", 5) == 0)
    {
      ret = SetCurrentColor(lpszSpecial);
    }
    break;
  case 'd':
    if (strncmp(lpszSpecial, "da", 2) == 0
      || strncmp(lpszSpecial, "dt", 2) == 0)
    {
      pSpecial = new DviSpecialObject<TpicSpecialObject<TpicPolySpecialImpl>>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    break;
  case 'e':
    if (strncmp(lpszSpecial, "em:", 3) == 0)
    {
      const char * lpsz = lpszSpecial + 3;
      while (*lpsz != 0 && *lpsz == ' ')
      {
        ++lpsz;
      }
      if (strncmp(lpsz, "graph", 5) == 0)
      {
        pSpecial = new DviSpecialObject<GraphicsSpecialImpl>(pPage, x, y, lpszSpecial);
        ret = true;
      }
      else if (strncmp(lpsz, "linewidth", 9) == 0)
      {
        float texWidth;
        char unit[3];
        lpsz += 9;
        if (sscanf(lpsz, "%f%2s", &texWidth, unit) != 2)
        {
          trace_error->WriteFormattedLine("libdvi", T_("invalid special: %s"), lpsz);
        }
        else
        {
          lineWidth = CalculateWidth(texWidth, unit, GetResolution());
          ret = true;
        }
      }
      else if (strncmp(lpsz, "lineto", 6) == 0 || strncmp(lpsz, "line", 4) == 0)
      {
        pSpecial = new DviSpecialObject<SolidLineSpecialImpl>(pPage, x, y, lpszSpecial);
        ret = true;
      }
      else if (strncmp(lpsz, "moveto", 6) == 0)
      {
        pointTable[-1] = DviPoint(x, y);
        ret = true;
      }
      else if (strncmp(lpsz, "point", 5) == 0)
      {
        lpsz += 5;
        while (*lpsz == ' ')
        {
          ++lpsz;
        }
        pointTable[atoi(lpsz)] = DviPoint(x, y);
        ret = true;
      }
      else if (strncmp(lpsz, "message", 7) == 0)
      {
        trace_dvifile->WriteLine("libdvi", lpsz + 7);
        ret = true;
      }
    }
    break;
  case 'f':
    if (strncmp(lpszSpecial, "fp", 2) == 0)
    {
      pSpecial = new DviSpecialObject<TpicSpecialObject<TpicPolySpecialImpl>>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    break;
  case 'h':
    if (strncmp(lpszSpecial, "header=", 7) == 0)
    {
      pSpecial = new DviSpecialObject<PsdefSpecialImpl>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    else if (strncmp(lpszSpecial, "html:", 5) == 0)
    {
      pSpecial = ProcessHtmlSpecial(pPage, x, y, lpszSpecial);
      ret = (pSpecial != 0);
    }
    break;
  case 'i':
    if (strncmp(lpszSpecial, "ia", 2) == 0)
    {
      pSpecial = new DviSpecialObject<TpicSpecialObject<TpicArcSpecialImpl>>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    else if (strncmp(lpszSpecial, "ip", 2) == 0)
    {
      pSpecial = new DviSpecialObject<TpicSpecialObject<TpicPolySpecialImpl>>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    else if (strncmp(lpszSpecial, "isoscale", 8) == 0)
    {
      pSpecial = new DviSpecialObject<GraphicsSpecialImpl>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    break;
  case 'l':
    if (strncmp(lpszSpecial, "landscape", 9) == 0)
    {
      pSpecial = new DviSpecialObject<LandscapeSpecialImpl>(pPage, x, y, lpszSpecial);
      landscape = true;
      haveLandscapeSpecial = true;
      ret = true;
    }
    break;
  case 'P':
  case 'p':
    if (strncmp(lpszSpecial, "pn", 2) == 0)
    {
      if (sscanf_s(lpszSpecial, "pn %d", &TpicSpecialRoot::tpicContext.penSize) != 1)
      {
        trace_error->WriteFormattedLine("libdvi", T_("bad pn special: %s"), lpszSpecial);
      }
      else
      {
        ret = true;
      }
    }
    else if (strncmp(lpszSpecial, "papersize=", 10) == 0 || strncmp(lpszSpecial, "papersize ", 10) == 0)
    {
      pSpecial = new DviSpecialObject<PaperSizeSpecialImpl>(pPage, x, y, lpszSpecial);
      havePaperSizeSpecial = true;
      paperSizeInfo = dynamic_cast<PaperSizeSpecial*>(pSpecial)->GetPaperSizeInfo();
      ret = true;
    }
    else if (strncmp(lpszSpecial, "pa", 2) == 0)
    {
      TpicSpecial::point p;
      if (sscanf_s(lpszSpecial, "pa %d %d", &p.x, &p.y) != 2)
      {
        trace_error->WriteFormattedLine("libdvi", T_("bad pa special: %s"), lpszSpecial);
      }
      else
      {
        TpicSpecialRoot::tpicContext.tpicPath.push_back(p);
        ret = true;
      }
    }
    else if (strncmp(lpszSpecial, "PSfile=", 7) == 0 || strncmp(lpszSpecial, "psfile=", 7) == 0)
    {
      pSpecial = new DviSpecialObject<PsfileSpecialImpl>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    else if (strncmp(lpszSpecial, "ps:", 3) == 0)
    {
      pSpecial = new DviSpecialObject<DvipsSpecialImpl>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    break;
  case 's':
    if (strncmp(lpszSpecial, "src:", 4) == 0)
    {
      pSpecial = new DviSpecialObject<SourceSpecialImpl>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    else if (strncmp(lpszSpecial, "sh", 2) == 0)
    {
      if (sscanf_s(lpszSpecial, "sh %f", &TpicSpecialRoot::tpicContext.shade) != 2)
      {
        trace_error->WriteFormattedLine("libdvi", T_("bad sh special: %s"), lpszSpecial);
      }
      else
      {
        ret = true;
      }
    }
    else if (strncmp(lpszSpecial, "sp", 2) == 0)
    {
      pSpecial = new DviSpecialObject<TpicSpecialObject< TpicPolySpecialImpl>>(pPage, x, y, lpszSpecial);
      ret = true;
    }
    break;
  case 't':
    if (strncmp(lpszSpecial, "tx", 2) == 0)
    {
      TpicSpecialRoot::tpicContext.shade = PatternToShadeLevel(lpszSpecial + 2);
      ret = true;
    }
    break;
  case 'w':
    if (strncmp(lpszSpecial, "wh", 2) == 0)
    {
      TpicSpecialRoot::tpicContext.shade = 0.0;
      ret = true;
    }
    break;
  }

  if (!ret)
  {
    trace_error->WriteFormattedLine("libdvi", T_("unimplemented special: %s"), lpszSpecial);
  }

  return ret;
}

DviSpecialType SolidLineSpecialImpl::Parse()
{
  DviPageImpl * pPage = GetPage();
  DviImpl * dvi = pPage->GetDviObject();
  MAPNUMTOPOINT & mapnumtopoint = dvi->GetPoints();
  const char * lpsz = GetXXX();
  if (strncmp(lpsz, "em:", 3) != 0)
  {
    return DviSpecialType::Unknown;
  }
  lpsz += 3;
  while (*lpsz == ' ')
  {
    ++lpsz;
  }
  if (strncmp(lpsz, "lineto", 6) == 0)
  {
    MAPNUMTOPOINT::const_iterator it = mapnumtopoint.find(-1);
    if (it == mapnumtopoint.end())
    {
      return DviSpecialType::Unknown;
    }
    xStart = it->second.x;
    yStart = it->second.y;
    xEnd = x;
    yEnd = y;
    mapnumtopoint[-1] = DviPoint(x, y);
    color = dvi->GetCurrentColor();
    width = dvi->GetLineWidth();
    return DviSpecialType::SolidLine;
  }
  else if (strncmp(lpsz, "line", 4) == 0)
  {
    lpsz += 4;
    while (*lpsz == ' ')
    {
      ++lpsz;
    }
    MAPNUMTOPOINT::iterator it = mapnumtopoint.find(atoi(lpsz));
    if (it == mapnumtopoint.end())
    {
      return DviSpecialType::Unknown;
    }
    xStart = it->second.x;
    yStart = it->second.y;
    while (isdigit(*lpsz))
    {
      ++lpsz;
    }
    if (strchr("hpv", *lpsz) != 0)
    {
      ++lpsz;
    }
    while (*lpsz == ' ')
    {
      ++lpsz;
    }
    if (*lpsz == ',')
    {
      ++lpsz;
    }
    while (*lpsz == ' ')
    {
      ++lpsz;
    }
    it = mapnumtopoint.find(atoi(lpsz));
    if (it == mapnumtopoint.end())
    {
      return DviSpecialType::Unknown;
    }
    xEnd = it->second.x;
    yEnd = it->second.y;
    while (isdigit(*lpsz))
    {
      ++lpsz;
    }
    if (strchr("hpv", *lpsz) != 0)
    {
      ++lpsz;
    }
    while (*lpsz == ' ')
    {
      ++lpsz;
    }
    if (*lpsz == ',')
    {
      ++lpsz;
    }
    while (*lpsz == ' ')
    {
      ++lpsz;
    }
    if (isdigit(*lpsz))
    {
      float texWidth;
      char unit[3];
      sscanf(lpsz, "%f%2s", &texWidth, unit);
      width = CalculateWidth(texWidth, unit, dvi->GetResolution());
    }
    else
    {
      width = dvi->GetLineWidth();
    }
    color = dvi->GetCurrentColor();
    return DviSpecialType::SolidLine;
  }
  else
  {
    return DviSpecialType::Unknown;
  }
}

DviSpecialType PaperSizeSpecialImpl::Parse()
{
  const char * lpsz = GetXXX() + 10;
  paperSizeInfo = PaperSizeInfo::Parse(lpsz);
  return DviSpecialType::PaperSize;
}
