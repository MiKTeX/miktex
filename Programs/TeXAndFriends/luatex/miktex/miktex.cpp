/* luatex/miktex/miktex.cpp:

   Copyright (C) 2016-2017 Christian Schenk

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

#include <miktex/App/Application>
#include <miktex/Core/Directory>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Registry>
#include <miktex/KPSE/Emulation>

#include <string>

#include "luatex.h"

namespace texmfapp {
#include <miktex/texmfapp.defaults.h>
}

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace std;

void miktex_enable_installer(int onOff)
{
  Application::GetApplication()->EnableInstaller(onOff ? TriState::True : TriState::False);
}

int miktex_luatex_renew_format_file(const char* name)
{
  shared_ptr<Session> session = Session::Get();
  PathName path;
  return session->FindFile(name, FileType::FMT, { Session::FindFileOption::Renew }, path) ? 1 : 0;
}

char* miktex_program_basename(const char* lpszArgv0)
{
  PathName argv0(lpszArgv0);
  std::string baseName = argv0.GetFileNameWithoutExtension().ToString();
  size_t prefixLen = strlen(MIKTEX_PREFIX);
  if (baseName.compare(0, prefixLen, MIKTEX_PREFIX) == 0)
  {
    baseName = baseName.substr(prefixLen);
  }
  return xstrdup(baseName.c_str());
}

int miktex_is_output_file(const char* path)
{
  PathName path_(path);
  if (path_.HasExtension(".gz"))
  {
    path_.SetExtension(nullptr);
  }
  return path_.HasExtension(".dvi")
    || path_.HasExtension(".pdf")
    || path_.HasExtension(".synctex");
}

PathName auxDirectory;

void miktex_set_aux_directory(const char* path)
{
  auxDirectory = path;
  auxDirectory.MakeAbsolute();
  shared_ptr<Session> session = Session::Get();
  if (!Directory::Exists(auxDirectory))
  {
    if (session->GetConfigValue("", MIKTEX_REGVAL_CREATE_AUX_DIRECTORY, texmfapp::texmfapp::CreateAuxDirectory()).GetString() == "t")
    {
      Directory::Create(auxDirectory);
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("The specified auxiliary directory does not exist."), "directory", auxDirectory.ToString());
    }
  }
  session->AddInputDirectory(auxDirectory, true);
}

const char* miktex_get_aux_directory()
{
  if (auxDirectory.Empty())
  {
    return nullptr;
  }
  else
  {
    return auxDirectory.GetData();
  }
}

void miktex_invoke_editor(const char* filename, int lineno)
{
  Application::GetApplication()->InvokeEditor(filename, lineno, FileType::TEX, PathName());
}

void miktex_show_library_versions()
{
  Application::GetApplication()->ShowLibraryVersions();
}

void miktex_add_include_directory(const char* path)
{
  if (Directory::Exists(path))
  {
    PathName includeDirectory(path);
    includeDirectory.MakeAbsolute();
    shared_ptr<Session> session = Session::Get();
    session->AddInputDirectory(includeDirectory, true);
  }
}
