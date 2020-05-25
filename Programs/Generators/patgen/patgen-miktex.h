/* patgen-miktex.h:

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

#include "patgen-miktex-config.h"

#include "patgen-version.h"

#include <miktex/TeXAndFriends/WebApp>

#if !defined(MIKTEXHELP_PATGEN)
#  include <miktex/Core/Help>
#endif

#include "patgen.h"

extern PATGENPROGCLASS PATGENPROG;

class PATGENAPPCLASS :
  public MiKTeX::TeXAndFriends::WebApp
{
public:
  std::string GetUsage() const override
  {
    return MIKTEXTEXT("FILE");
  }

// TODO
#if 0
public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_PATGEN;
  }
#endif
};

extern PATGENAPPCLASS PATGENAPP;
