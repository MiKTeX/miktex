/* screen.h: METAFONT screen display

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

#pragma once

#if defined(MIKTEX_WINDOWS)
bool miktexinitscreen(int screenWidth, int screenHeight);

void miktexblankrectangle(MFPROGCLASS::screencol leftColumn, MFPROGCLASS::screencol rightColumn, MFPROGCLASS::screenrow topRow, MFPROGCLASS::screenrow bottomRow);

void miktexupdatescreen();

void miktexpaintrow(MFPROGCLASS::screenrow row, MFPROGCLASS::pixelcolor startColor, MFPROGCLASS::transspec blackWhiteTransitions, MFPROGCLASS::screencol numColumns);
#else
inline bool miktexinitscreen(int screenWidth, int screenHeight)
{
  return false;
}

inline void miktexblankrectangle(MFPROGCLASS::screencol leftColumn, MFPROGCLASS::screencol rightColumn, MFPROGCLASS::screenrow topRow, MFPROGCLASS::screenrow bottomRow)
{
}

inline void miktexupdatescreen()
{
}

inline void miktexpaintrow(MFPROGCLASS::screenrow row, MFPROGCLASS::pixelcolor startColor, MFPROGCLASS::transspec blackWhiteTransitions, MFPROGCLASS::screencol numColumns)
{
}
#endif
