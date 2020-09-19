/* tpic.cpp: tpic specials

   Copyright (C) 1996-2020 Christian Schenk

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

#include "config.h"

#include "internal.h"

struct TpicContext TpicSpecialRoot::tpicContext;

DviSpecialType TpicPolySpecialImpl::Parse()
{
  outlineStyle = OutlineStyle::Solid;
  polyLength = 0;
  isSpline = false;
  if (strncmp(GetXXX(), "ip", 2) == 0)
  {
    outlineStyle = OutlineStyle::None;
  }
  else if (strncmp(GetXXX(), "da", 2) == 0)
  {
    if (sscanf_s(GetXXX(), "da %f", &polyLength) != 1)
    {
      pDviPageImpl->Error(T_("bad da special"));
    }
    else
    {
      outlineStyle = OutlineStyle::Dashes;
    }
  }
  else if (strncmp(GetXXX(), "dt", 2) == 0)
  {
    if (sscanf_s(GetXXX(), "dt %f", &polyLength) != 1)
    {
      pDviPageImpl->Error(T_("bad dt special"));
    }
    else
    {
      outlineStyle = OutlineStyle::Dots;
    }
  }
  else if (strncmp(GetXXX(), "sp", 2) == 0)
  {
    isSpline = true;
    if (sscanf_s(GetXXX(), "sp %f", &polyLength) == 1)
    {
      if (polyLength > 0)
      {
        outlineStyle = OutlineStyle::Dashes;
      }
      else if (polyLength < 0)
      {
        polyLength *= -1;
        outlineStyle = OutlineStyle::Dots;
      }
    }
  }

  return DviSpecialType::Tpic;
}

DviSpecialType TpicArcSpecialImpl::Parse()
{
  hasOutline = (strncmp(GetXXX(), "ar", 2) == 0);
  if (sscanf_s(GetXXX() + 2, " %d %d %d %d %f %f", &cx, &cy, &m_rx, &m_ry, &m_s, &m_e) != 6)
  {
    pDviPageImpl->Error(T_("bad ar special"));
    return DviSpecialType::Unknown;
  }
  return DviSpecialType::Tpic;
}
