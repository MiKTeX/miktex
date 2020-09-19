/* mplib/miktex/mpost.cpp:

   Copyright (C) 2017 Christian Schenk

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

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/App/Application>
#include <miktex/Core/Utils>

#include "mpost.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;

void miktex_show_library_versions()
{
  Application::GetApplication()->ShowLibraryVersions();
}

inline std::string GetBanner(const char* name, const char* version)
{
  return fmt::format("This is {0} {1} ({2})", name, version, Utils::GetMiKTeXBannerString());
}

void miktex_print_banner(FILE* file, const char* name, const char* version)
{
  fprintf(file, "%s\n", GetBanner(name, version).c_str());
}
