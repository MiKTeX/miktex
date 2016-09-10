/* dvipdfmx-/miktex/miktex.cpp:

   Copyright (C) 2016 Christian Schenk

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

#include "dvipdfm-x.h"

#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>

using namespace MiKTeX::Core;
using namespace std;

#include <memory>

extern "C"
{
  // in dvipdfmx.c
  void read_config_file(const char *config);
}

extern "C" void miktex_read_config_files()
{
  shared_ptr<Session> session = Session::Get();
  vector<PathName> configFiles;
  if (session->FindFile(MIKTEX_PATH_DVIPDFMX_CONFIG, MIKTEX_PATH_TEXMF_PLACEHOLDER, { Session::FindFileOption::All }, configFiles))
  {
    for (std::vector<PathName>::const_reverse_iterator it = configFiles.rbegin(); it != configFiles.rend(); ++it)
    {
      read_config_file(it->GetData());
    }
  }
}
