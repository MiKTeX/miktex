/* ps.cpp: PostScript specials

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

DviSpecialType PsdefSpecialImpl::Parse()
{
  const char * lpsz = GetXXX();
  if (strncmp(GetXXX(), "header=", 7) == 0)
  {
    isFileName = true;
    lpsz += 7;
  }
  else
  {
    isFileName = false;
    lpsz += 1;                        // !
  }
  while (*lpsz)
  {
    str += *lpsz++;
  }
  return DviSpecialType::Psdef;
}

DviSpecialType DvipsSpecialImpl::Parse()
{
  protection = false;
  isFileName = false;
  const char * lpsz = GetXXX();
  if (*lpsz == '"')
  {
    lpsz += 1;
    protection = true;
  }
  else if (strncmp(lpsz, "ps:", 3) == 0)
  {
    if (strncmp(GetXXX(), "ps::[begin]", 11) == 0)
    {
      lpsz += 11;
    }
    else if (strncmp(GetXXX(), "ps::[end]", 9) == 0)
    {
      lpsz += 9;
    }
    else if (strncmp(GetXXX(), "ps::", 4) == 0)
    {
      lpsz += 4;
    }
    else
    {
      lpsz += 3;
      const char * lpsz2 = lpsz;
      while (*lpsz2 && isspace(*lpsz2))
      {
        ++lpsz2;
      }
      if (strncmp(lpsz2, "plotfile", 8) == 0)
      {
        lpsz2 += 8;
        while (*lpsz2 && isspace(*lpsz2))
        {
          ++lpsz2;
        }
        while (*lpsz2 && !isspace(*lpsz2))
        {
          str += *lpsz2++;
        }
        if (str.length() == 0)
        {
          trace_error->WriteLine("libdvi", T_("bad ps: plotfile special"));
        }
        else
        {
          isFileName = true;
        }
        return DviSpecialType::Ps;
      }
    }
  }
  while (*lpsz != 0)
  {
    str += *lpsz++;
  }
  return DviSpecialType::Ps;
}


bool getkv(char * & lpsz, pair<char *, char *> & keyVal)
{
  for (; *lpsz <= ' ' && *lpsz != 0; ++lpsz)
  {
    ;
  }

  if (*lpsz == 0)
  {
    return false;
  }

  keyVal.first = lpsz;
  while (*lpsz > ' ' && *lpsz != '=')
  {
    ++lpsz;
  }

  char lastChar = *lpsz;
  if (*lpsz != 0)
  {
    *lpsz++ = 0;
  }

  if (lastChar != 0 && lastChar <= ' ')
  {
    for (; *lpsz && *lpsz <= ' '; lpsz++)
    {
      ;
    }
    lastChar = *lpsz;
    if (lastChar == '=')
    {
      ++lpsz;
    }
  }

  keyVal.second = 0;
  if (lastChar == '=')
  {
    while (*lpsz != 0 && *lpsz <= ' ')
    {
      ++lpsz;
    }
    char delim;
    if (*lpsz == '\'' || *lpsz == '\"')
    {
      delim = *lpsz++;
    }
    else
    {
      delim = ' ';
    }
    keyVal.second = lpsz;
    while (*lpsz != 0 && *lpsz != delim)
    {
      ++lpsz;
    }
    if (*lpsz != 0)
    {
      *lpsz++ = 0;
    }
  }

  return true;
}

DviSpecialType PsfileSpecialImpl::Parse()
{
  hSize = 0;
  vSize = 0;
  hOffset = 0;
  vOffset = 0;
  hScale = 100;
  vScale = 100;
  angle = 0;
  llx = 0;
  lly = 0;
  urx = 0;
  ury = 0;
  rwi = 0;
  rhi = 0;
  isClipped = false;
  hasHSize = false;
  hasVSize = false;
  hasHOffset = false;
  hasVOffset = false;
  hasHSale = false;
  hasVScale = false;
  hasAngle = false;
  hasLlx = false;
  hasLLy = false;
  hasUrx = false;
  hasUry = false;
  hasRwi = false;
  hasRhi = false;
  hasClipFlag = false;

  CharBuffer<char> autoBuffer(specialString.length() + 1);
  char * lpszSpecial = autoBuffer.GetData();
  strcpy_s(lpszSpecial, specialString.length() + 1, GetXXX());
  pair<char *, char *> keyVal;
  while (*lpszSpecial && getkv(lpszSpecial, keyVal))
  {
    if (_strcmpi(keyVal.first, "psfile") == 0)
    {
      if (keyVal.second != nullptr)
      {
        fileName = keyVal.second;
      }
    }
    else if (_strcmpi(keyVal.first, "hsize") == 0)
    {
      if (keyVal.second != nullptr)
      {
        hSize = atoi(keyVal.second);
        hasHSize = true;
      }
    }
    else if (_strcmpi(keyVal.first, "vsize") == 0)
    {
      if (keyVal.second != nullptr)
      {
        vSize = atoi(keyVal.second);
        hasVSize = true;
      }
    }
    else if (_strcmpi(keyVal.first, "hoffset") == 0)
    {
      if (keyVal.second != nullptr)
      {
        hOffset = atoi(keyVal.second);
        hasHOffset = true;
      }
    }
    else if (_strcmpi(keyVal.first, "voffset") == 0)
    {
      if (keyVal.second != nullptr)
      {
        vOffset = atoi(keyVal.second);
        hasVOffset = true;
      }
    }
    else if (_strcmpi(keyVal.first, "hscale") == 0)
    {
      if (keyVal.second != nullptr)
      {
        hScale = atoi(keyVal.second);
        hasHSale = true;
      }
    }
    else if (_strcmpi(keyVal.first, "vscale") == 0)
    {
      if (keyVal.second != nullptr)
      {
        vScale = atoi(keyVal.second);
        hasVScale = true;
      }
    }
    else if (_strcmpi(keyVal.first, "angle") == 0)
    {
      if (keyVal.second != nullptr)
      {
        angle = atoi(keyVal.second);
        hasAngle = true;
      }
    }
    else if (_strcmpi(keyVal.first, "llx") == 0)
    {
      if (keyVal.second != nullptr)
      {
        llx = atoi(keyVal.second);
        hasLlx = true;
      }
    }
    else if (_strcmpi(keyVal.first, "lly") == 0)
    {
      if (keyVal.second != nullptr)
      {
        lly = atoi(keyVal.second);
        hasLLy = true;
      }
    }
    else if (_strcmpi(keyVal.first, "urx") == 0)
    {
      if (keyVal.second != nullptr)
      {
        urx = atoi(keyVal.second);
        hasUrx = true;
      }
    }
    else if (_strcmpi(keyVal.first, "ury") == 0)
    {
      if (keyVal.second != nullptr)
      {
        ury = atoi(keyVal.second);
        hasUry = true;
      }
    }
    else if (_strcmpi(keyVal.first, "rwi") == 0)
    {
      if (keyVal.second != nullptr)
      {
        rwi = atoi(keyVal.second);
        hasRwi = true;
      }
    }
    else if (_strcmpi(keyVal.first, "rhi") == 0)
    {
      if (keyVal.second != nullptr)
      {
        rhi = atoi(keyVal.second);
        hasRhi = true;
      }
    }
    else if (_strcmpi(keyVal.first, "clip") == 0)
    {
      isClipped = true;
      hasClipFlag = true;
    }
  }

  return DviSpecialType::Psfile;
}

bool PsfileSpecialImpl::GetBoundingBox(float & left, float & bottom, float & right, float & top)
{
  float my_hsize = 0;
  float my_vsize = 0;
  float my_hoffset = 0;
  float my_voffset = 0;
  float my_hscale = 1.0;
  float my_vscale = 1.0;

  if (hasHSize)
  {
    my_hsize = static_cast<float>(hSize);
  }

  if (hasVSize)
  {
    my_vsize = static_cast<float>(vSize);
  }

  if (hasHOffset)
  {
    my_hoffset = static_cast<float>(hOffset);
  }

  if (hasVOffset)
  {
    my_voffset = static_cast<float>(vOffset);
  }

  if (hasHSale)
  {
    my_hscale = static_cast<float>(hScale / 100.0);
  }

  if (hasVScale)
  {
    my_vscale = static_cast<float>(vScale / 100.0);
  }

  if (hasLlx && hasUrx)
  {
    my_hsize = static_cast<float>(urx - llx);
  }

  if (hasLLy && hasUry)
  {
    my_vsize = static_cast<float>(ury - lly);
  }

  if (hasRwi || hasRhi)
  {
    if (hasRwi && !hasRhi)
    {
      if (my_hsize == 0)
      {
        return false;
      }
      my_hscale = my_vscale =
        static_cast<float>(rwi / (10.0 * my_hsize));
    }
    else if (!hasRwi && hasRhi)
    {
      if (my_vsize == 0)
      {
        return false;
      }
      my_hscale = my_vscale =
        static_cast<float>(rhi / (10.0 * my_vsize));
    }
    else
    {
      if (my_hsize == 0 || my_vsize == 0)
      {
        return false;
      }
      my_hscale = static_cast<float>(rwi / (10.0 * my_hsize));
      my_vscale = static_cast<float>(rhi / (10.0 * my_vsize));
    }
  }

  left = my_hoffset;
  bottom = my_voffset;
  right = my_hoffset + my_hsize * my_hscale;
  top = my_voffset + my_vsize * my_vscale;

  return true;
}

bool PsfileSpecialImpl::GetBoundingBox(int shrinkFactor, int & left, int & bottom, int & right, int & top)
{
  float l, b, r, t;
  if (!GetBoundingBox(l, b, r, t))
  {
    return false;
  }
  DviImpl * pDviImpl = pDviPageImpl->GetDviObject();
  double conv =
    (((72.0 * pDviImpl->GetMagnification())
      / pDviImpl->GetResolution())
      / 1000);
  left = pDviImpl->PixelShrink(shrinkFactor, static_cast<int>(x + l / conv));
  top = pDviImpl->PixelShrink(shrinkFactor, static_cast<int>(y - t / conv));
  right = pDviImpl->PixelShrink(shrinkFactor, static_cast<int>(x + r / conv));
  bottom = pDviImpl->PixelShrink(shrinkFactor, static_cast<int>(y + b / conv));
  return true;
}
