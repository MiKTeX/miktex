/* miktex/Core/win/Registry.h: MiKTeX settings          -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include <miktex/Version>

#define MIKTEX_REGPATH_COMPANY                                           \
  "Software\\MiKTeX.org"
#define MIKTEX_REGPATH_PRODUCT                                           \
  "Software\\MiKTeX.org\\MiKTeX"
#define MIKTEX_REGPATH_SERIES                                            \
  "Software\\MiKTeX.org\\MiKTeX\\" MIKTEX_LEGACY_MAJOR_MINOR_STR
#define MIKTEX_REGPATH_PRODUCT_MIKTEXDIRECT                              \
  "Software\\MiKTeX.org\\MiKTeXDirect"
#define MIKTEX_REGPATH_MAJOR_MINOR_MIKTEXDIRECT                          \
  "Software\\MiKTeX.org\\MiKTeXDirect\\"  MIKTEX_LEGACY_MAJOR_MINOR_STR

#define MIKTEX_GPL_GHOSTSCRIPT                                           \
  "Software\\MiKTeX GPL Ghostscript"
