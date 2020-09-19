/* mfapp.cpp:

   Copyright (C) 1996-2020 Christian Schenk

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

#if defined(MIKTEX_TEXMF_SHARED)
#  define C4PEXPORT MIKTEXDLLEXPORT
#else
#  define C4PEXPORT
#endif
#define C1F0C63F01D5114A90DDF8FC10FF410B
#include "miktex/C4P/C4P.h"

#if defined(MIKTEX_TEXMF_SHARED)
#  define MIKTEXMFEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXMFEXPORT
#endif
#define B8C7815676699B4EA2DE96F0BD727276
#include "miktex/TeXAndFriends/MetafontApp.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::TeXAndFriends;

class MetafontApp::impl
{
public:
  int optBase;
};

MetafontApp::MetafontApp() :
  pimpl(make_unique<impl>())
{
}

MetafontApp::~MetafontApp() noexcept
{
}

void MetafontApp::Init(vector<char*>& args)
{
  TeXMFApp::Init(args);
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
  pimpl->optBase = (int)GetOptions().size();
  AddOption(T_("bistack-size\0Set bistack_size to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_BISTACK_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("lig-table-size\0Set lig_table_size to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_LIG_TABLE_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("path-size\0Set path_size to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_PATH_SIZE, POPT_ARG_STRING, "N");
}

bool MetafontApp::ProcessOption(int opt, const string& optArg)
{
  bool done = true;
  switch (opt - FIRST_OPTION_VAL - pimpl->optBase)
  {
  case OPT_BISTACK_SIZE:
    GetUserParams()["bistack_size"] = std::stoi(optArg);
    break;
  case OPT_LIG_TABLE_SIZE:
    GetUserParams()["lig_table_size"] = std::stoi(optArg);
    break;
  case OPT_PATH_SIZE:
    GetUserParams()["path_size"] = std::stoi(optArg);
    break;
  default:
    done = TeXMFApp::ProcessOption(opt, optArg);
    break;
  }
  return done;
}
