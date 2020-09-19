/* graphics.cpp: graphics specials

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

int CalculateWidth(float width, const char* lpszUnit, int resolution)
{
  static const struct
  {
    const char* lpszUnit;
    double factor;
  } unittable[] = {
    { "pt", 72.27 },
    { "pc", 72.27 / 12 },
    { "in", 1.0 },
    { "bp", 72.0 },
    { "cm", 2.54 },
    { "mm", 25.4 },
    { "dd", 72.27 / (1238.0 / 1157) },
    { "cc", 72.27 / 12 / (1238.0 / 1157) },
    { "sp", 72.27 * 65536 },
  };
  for (size_t i = 0; i < sizeof(unittable) / sizeof(unittable[0]); ++i)
  {
    if (strcmp(unittable[i].lpszUnit, lpszUnit) == 0)
    {
      return static_cast<int>((width* resolution / unittable[i].factor));
    }
  }
  if (strcmp(lpszUnit, "px") == 0)
  {
    return static_cast<int>(width);
  }
  return -1;
}

DviSpecialType GraphicsSpecialImpl::Parse()
{
  width = -1;
  height = -1;

  const char* lpsz = GetXXX();

  if (strncmp(GetXXX(), "em:", 3) == 0)
  {
    graphicsAction = GraphicsSpecialAction::EmGraph;
    lpsz += 3;
    while (*lpsz == ' ')
    {
      ++lpsz;
    }
    lpsz += 5;
  }
  else if (strncmp(GetXXX(), "anisoscale", 10) == 0)
  {
    graphicsAction = GraphicsSpecialAction::DviWinAnisoScale;
    lpsz = GetXXX() + 10;
  }
  else if (strncmp(GetXXX(), "isoscale", 8) == 0)
  {
    graphicsAction = GraphicsSpecialAction::DviWinIsoScale;
    lpsz += 8;
  }
  else if (strncmp(GetXXX(), "center", 6) == 0)
  {
    graphicsAction = GraphicsSpecialAction::DviWinCenter;
    lpsz += 6;
  }
  else
  {
    pDviPageImpl->Error(fmt::format(T_("unknown graphics special: {0}"), lpsz));
    return DviSpecialType::Unknown;
  }

  while (*lpsz != 0 && isspace(*lpsz))
  {
    ++lpsz;
  }

  while (*lpsz != 0 && !isspace(*lpsz) && *lpsz != ',')
  {
    fileName += *lpsz++;
  }

  while (*lpsz != 0 && (isspace(*lpsz) || (*lpsz == ',')))
  {
    ++lpsz;
  }

  if (*lpsz != 0)
  {
    float texWidth;
    char unit[3];
    if (sscanf_s(lpsz, "%f%2s", &texWidth, unit, 3) != 2)
    {
      pDviPageImpl->Error(fmt::format(T_("invalid width specification: {0}"), lpsz));
      return DviSpecialType::Unknown;
    }
    width = CalculateWidth(texWidth, unit, GetPage()->GetDviObject()->GetResolution());
    while (*lpsz != 0 && (*lpsz == '.' || isdigit(*lpsz)))
    {
      ++lpsz;
    }
    while (*lpsz != 0 && isalpha(*lpsz))
    {
      ++lpsz;
    }
    while (*lpsz != 0 && (isspace(*lpsz) || (*lpsz == ',')))
    {
      ++lpsz;
    }
    if (*lpsz != 0)
    {
      float texHeight;
      if (sscanf_s(lpsz, "%f%2s", &texHeight, unit, 3) != 2)
      {
        pDviPageImpl->Error(fmt::format(T_("invalid width specification: {0}"), lpsz));
        return DviSpecialType::Unknown;
      }
      height = CalculateWidth(texHeight, unit, GetPage()->GetDviObject()->GetResolution());
    }
  }

  return DviSpecialType::IncludeGraphics;
}
