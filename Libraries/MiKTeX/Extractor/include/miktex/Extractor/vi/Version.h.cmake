/* miktex/Extractor/vi/Version.h: library version       -*- C++ -*-

   Copyright (C) 1996-2017 Christian Schenk

   This file is part of MiKTeX Extractor.

   MiKTeX Extractor is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Extractor is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Extractor; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(F484D8E24F8A4225BCCEAAD8291F2899)
#define F484D8E24F8A4225BCCEAAD8291F2899

#include <miktex/Extractor/config.h>

#include <string>
#include <vector>

#include <miktex/Core/LibraryVersion>

#define MIKTEX_EXTRACTOR_VI_BEGIN_NAMESPACE     \
  MIKTEX_EXTRACTOR_BEGIN_NAMESPACE              \
  namespace vi {

#define MIKTEX_EXTRACTOR_VI_END_NAMESPACE       \
  }                                             \
  MIKTEX_EXTRACTOR_END_NAMESPACE

MIKTEX_EXTRACTOR_VI_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE Runtime
{
public:
  Runtime() = delete;

public:
  Runtime(const Runtime& other) = delete;

public:
  Runtime& operator=(const Runtime& other) = delete;

public:
  Runtime(Runtime&& other) = delete;

public:
  Runtime& operator=(Runtime&& other) = delete;

public:
  ~Runtime() = delete;

public:
  static MIKTEXEXTRACTORCEEAPI(std::string) GetName();

public:
  static MIKTEXEXTRACTORCEEAPI(std::string) GetDescription();

public:
  static MIKTEXEXTRACTORCEEAPI(int) GetInterfaceVersion();

public:
  static MIKTEXEXTRACTORCEEAPI(MiKTeX::Core::VersionNumber) GetVersion();

public:
  static MIKTEXEXTRACTORCEEAPI(std::vector<MiKTeX::Core::LibraryVersion>) GetDependencies();
};

class MIKTEXNOVTABLE Header
{
public:
  Header() = delete;

public:
  Header(const Header& other) = delete;

public:
  Header& operator=(const Header& other) = delete;

public:
  Header(Header&& other) = delete;

public:
  Header& operator=(Header&& other) = delete;

public:
  ~Header() = delete;

public:
  static std::string GetName()
  {
    return "MiKTeX Archive Extractor";
  }

public:
  static std::string GetDescription()
  {
    return "${MIKTEX_COMP_DESCRIPTION}";
  }

public:
  static int GetInterfaceVersion()
  {
    return ${MIKTEX_COMP_INTERFACE_VERSION};
  }

public:
  static MiKTeX::Core::VersionNumber GetVersion()
  {
    return MiKTeX::Core::VersionNumber(GetInterfaceVersion(), ${MIKTEX_COMP_J2000_VERSION}, 0, 0);
  }
};

class MIKTEXNOVTABLE Version
{
public:
  Version() = delete;

public:
  Version(const Version& other) = delete;

public:
  Version& operator=(const Version& other) = delete;

public:
  Version(Version&& other) = delete;

public:
  Version& operator=(Version&& other) = delete;

public:
  ~Version() = delete;

public:
  static MiKTeX::Core::LibraryVersion GetLibraryVersion()
  {
    return MiKTeX::Core::LibraryVersion("miktex-extractor", Header::GetName(), Header::GetVersion().ToString(), Runtime::GetVersion().ToString());
  }
};

MIKTEX_EXTRACTOR_VI_END_NAMESPACE;

#endif
