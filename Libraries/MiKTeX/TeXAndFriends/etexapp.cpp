/* etexapp.cpp:

   Copyright (C) 1996-2018 Christian Schenk

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

#include "internal.h"

class ETeXApp::impl
{
public:
  bool enableETeX;
public:
  int optBase;
};

ETeXApp::ETeXApp() :
  pimpl(make_unique<impl>())
{
}

ETeXApp::~ETeXApp() noexcept
{
}

void ETeXApp::Init(vector<char*>& args)
{
  TeXApp::Init(args);
  pimpl->enableETeX = false;
}

void ETeXApp::OnTeXMFStartJob()
{
  TeXApp::OnTeXMFStartJob();
}

void ETeXApp::Finalize()
{
  TeXApp::Finalize();
}

enum {
  OPT_ENABLE_ETEX,
};

void ETeXApp::AddOptions()
{
  TeXApp::AddOptions();
  pimpl->optBase = (int)GetOptions().size();
  AddOption(T_("enable-etex\0Enable e-TeX extensions."), FIRST_OPTION_VAL + pimpl->optBase + OPT_ENABLE_ETEX);
  // supported Web2C options
  AddOption("etex", "enable-etex");
}

bool ETeXApp::ProcessOption(int optchar, const string& optArg)
{
  bool done = true;
  switch (optchar - FIRST_OPTION_VAL - pimpl->optBase)
  {
  case OPT_ENABLE_ETEX:
    pimpl->enableETeX = true;
    break;
  default:
    done = TeXApp::ProcessOption(optchar, optArg);
    break;
  }
  return done;
}

bool ETeXApp::ETeXP() const
{
  return pimpl->enableETeX;
}
