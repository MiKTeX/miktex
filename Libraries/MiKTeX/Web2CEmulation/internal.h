/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 2010-2016 Christian Schenk

   This file is part of the MiKTeX W2CEMU Library.

   The MiKTeX W2CEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX W2CEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX W2CEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#define MIKTEXW2CEXPORT MIKTEXDLLEXPORT

#define B96BCD894353492A9CF685C84B9AB5E0
#include "miktex/W2C/Emulation.h"

#define T_(x) MIKTEXTEXT(x)

#define MIKTEXSTATICFUNC(type) static type
#define MIKTEXINTERNALFUNC(type) type
#define MIKTEXINTERNALVAR(type) type

#define C_FUNC_BEGIN()
#define C_FUNC_END()
