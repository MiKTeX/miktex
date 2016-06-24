/* texapp.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

void MetafontApp::Init(const char * lpszProgramInvocationName)
{
  TeXMFApp::Init(lpszProgramInvocationName);
  param_bistack_size = -1;
  param_lig_table_size = -1;
  param_path_size = -1;
}

void MetafontApp::Finalize()
{
  TeXMFApp::Finalize();
}

enum {
  OPT_BISTACK_SIZE,
  OPT_LIG_TABLE_SIZE,
  OPT_PATH_SIZE,
};

void MetafontApp::AddOptions()
{
  TeXMFApp::AddOptions();
  optBase = static_cast<int>(GetOptions().size());
  AddOption(T_("bistack-size\0Set bistack_size to N."), FIRST_OPTION_VAL + optBase + OPT_BISTACK_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("lig-table-size\0Set lig_table_size to N."), FIRST_OPTION_VAL + optBase + OPT_LIG_TABLE_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("path-size\0Set path_size to N."), FIRST_OPTION_VAL + optBase + OPT_PATH_SIZE, POPT_ARG_STRING, "N");
}

bool MetafontApp::ProcessOption(int opt, const string & optArg)
{
  bool done = true;
  switch (opt - FIRST_OPTION_VAL - optBase)
  {
  case OPT_BISTACK_SIZE:
    param_bistack_size = std::stoi(optArg);
    break;
  case OPT_LIG_TABLE_SIZE:
    param_lig_table_size = std::stoi(optArg);
    break;
  case OPT_PATH_SIZE:
    param_path_size = std::stoi(optArg);
    break;
  default:
    done = TeXMFApp::ProcessOption(opt, optArg);
    break;
  }
  return done;
}
