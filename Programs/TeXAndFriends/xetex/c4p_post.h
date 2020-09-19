/* c4p_post.h:
   
   Copyright (C) 2020 Christian Schenk

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
   USA. */

#pragma once

#include "c4p_pre.h"
#include <miktex/C4P/C4P>

class XeTeXProgramBase :
  public C4P::ProgramBase
{
protected:
  void flushunicodefile(unicodefile f)
  {
    if (fflush(f->f) == EOF)
    {
      MIKTEX_FATAL_CRT_ERROR("fflush");
    }
  }
};
