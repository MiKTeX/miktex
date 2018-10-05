/* inliners.h: inlined functions                        -*- C++ -*-

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

inline int PkChar::PixelShrink(int shrinkFactor, int pxl)
{
  return pDviFont->PixelShrink(shrinkFactor, pxl);
}

inline int PkChar::WidthShrink(int shrinkFactor, int pxl)
{
  return pDviFont->WidthShrink(shrinkFactor, pxl);
}

inline int DviPageImpl::PixelShrink(int shrinkFactor, int pxl)
{
  return pDviImpl->PixelShrink(shrinkFactor, pxl);
}

inline int DviPageImpl::WidthShrink(int shrinkFactor, int pxl)
{
  return pDviImpl->WidthShrink(shrinkFactor, pxl);
}

inline int DviItem::GetWidthShr(int shrinkFactor)
{
  return pPkChar->GetWidthShr(shrinkFactor);
}

inline int DviItem::GetHeightShr(int shrinkFactor)
{
  return pPkChar->GetHeightShr(shrinkFactor);
}

inline int DviItem::GetLeftShr(int shrinkFactor)
{
  return pPkChar->GetLeftShr(shrinkFactor, x);
}

inline int DviItem::GetTopShr(int shrinkFactor)
{
  return pPkChar->GetTopShr(shrinkFactor, y);
}

inline int DviItem::GetWidthUns()
{
  return pPkChar->GetWidthUns();
}

inline int DviItem::GetHeightUns()
{
  return pPkChar->GetHeightUns();
}

inline int DviItem::GetLeftUns()
{
  return pPkChar->GetLeftUns(x);
}

inline int DviItem::GetTopUns()
{
  return pPkChar->GetTopUns(y);
}

inline int DviChar::GetScaledAt()
{
  return pDviFont->GetScaledAt();
}

inline int DviFont::PixelShrink(int shrinkFactor, int pxl)
{
  return pDviImpl->PixelShrink(shrinkFactor, pxl);
}

inline int DviFont::WidthShrink(int shrinkFactor, int pxl)
{
  return pDviImpl->WidthShrink(shrinkFactor, pxl);
}

inline unsigned long Round(double r)
{
  return static_cast<unsigned long>(r + 0.5);
}
