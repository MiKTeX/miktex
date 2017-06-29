/* PropSheet.h:                                         -*- C++ -*-

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of MiKTeX Options.

   MiKTeX Options is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Options is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Options; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#include "PropPageFormats.h"
#include "PropPageGeneral.h"
#include "PropPageLanguages.h"
#include "PropPagePackages.h"
#include "PropPageRoots.h"

class PropSheet
  : public CPropertySheet, public IRunProcessCallback
{
private:
  DECLARE_DYNAMIC(PropSheet);

public:
  PropSheet(std::shared_ptr<MiKTeX::Packages::PackageManager>);

public:
  virtual ~PropSheet();

public:
  bool RunIniTeXMF(const char * lpszTitle, const std::vector<std::string>& args, ProgressDialog * pProgressDialog);

public:
  void BuildFormats();

public:
  void ScheduleBuildFormats()
  {
    mustBuildFormats = true;
  }

public:
  bool MustBuildFormats() const
  {
    return mustBuildFormats;
  }

protected:
  DECLARE_MESSAGE_MAP();

private:
  PropPageGeneral generalPage;

private:
  PropPageTeXMFRoots texmfRootsPage;

private:
  PropPageFormats formatsPage;

private:
  PropPageLanguages languagesPage;

private:
  PropPagePackages packagesPage;

private:
  virtual BOOL OnInitDialog();

private:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n);

private:
  string processOutput;

private:
  ProgressDialog * pProgressDialog = nullptr;

private:
  bool mustBuildFormats = false;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;

private:
  std::shared_ptr<MiKTeX::Core::Session> pSession = Session::Get();
};
