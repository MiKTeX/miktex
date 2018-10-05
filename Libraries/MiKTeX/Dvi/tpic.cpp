/* tpic.cpp: tpic specials

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

struct TpicContext TpicSpecialRoot::tpicContext;

DviSpecialType TpicPolySpecialImpl::Parse()
{
  m_outline = OutlineStyle::Solid;
  m_length = 0;
  m_bSpline = false;
  if (strncmp(GetXXX(), "ip", 2) == 0)
  {
    m_outline = OutlineStyle::None;
  }
  else if (strncmp(GetXXX(), "da", 2) == 0)
  {
    if (sscanf_s(GetXXX(), "da %f", &m_length) != 1)
    {
      trace_error->WriteLine("libdvi", T_("bad da special"));
    }
    else
    {
      m_outline = OutlineStyle::Dashes;
    }
  }
  else if (strncmp(GetXXX(), "dt", 2) == 0)
  {
    if (sscanf_s(GetXXX(), "dt %f", &m_length) != 1)
    {
      trace_error->WriteLine("libdvi", T_("bad dt special"));
    }
    else
    {
      m_outline = OutlineStyle::Dots;
    }
  }
  else if (strncmp(GetXXX(), "sp", 2) == 0)
  {
    m_bSpline = true;
    if (sscanf_s(GetXXX(), "sp %f", &m_length) == 1)
    {
      if (m_length > 0)
      {
        m_outline = OutlineStyle::Dashes;
      }
      else if (m_length < 0)
      {
        m_length *= -1;
        m_outline = OutlineStyle::Dots;
      }
    }
  }

  return DviSpecialType::Tpic;
}

DviSpecialType TpicArcSpecialImpl::Parse()
{
  m_bOutline = (strncmp(GetXXX(), "ar", 2) == 0);
  if (sscanf_s(GetXXX() + 2, " %d %d %d %d %f %f", &cx, &cy, &m_rx, &m_ry, &m_s, &m_e) != 6)
  {
    trace_error->WriteLine("libdvi", T_("bad ar special"));
    return DviSpecialType::Unknown;
  }
  return DviSpecialType::Tpic;
}
