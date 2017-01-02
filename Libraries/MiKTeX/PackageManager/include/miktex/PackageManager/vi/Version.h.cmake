/* miktex/PackageManager/vi/Version.h: library version  -*- C++ -*-

   Copyright (C) 1996-2017 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(A0AAFFDD84D34452A5909319E490029B)
#define A0AAFFDD84D34452A5909319E490029B

#include <miktex/PackageManager/config.h>

#include <string>
#include <vector>

#include <miktex/Core/LibraryVersion>

#define MIKTEX_MPM_VI_BEGIN_NAMESPACE          \
  MPM_BEGIN_NAMESPACE                          \
  namespace vi {

#define MIKTEX_MPM_VI_END_NAMESPACE            \
  }                                            \
  MPM_END_NAMESPACE

MIKTEX_MPM_VI_BEGIN_NAMESPACE;

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
  static MIKTEXMPMCEEAPI(std::string) GetName();

public:
  static MIKTEXMPMCEEAPI(std::string) GetDescription();

public:
  static MIKTEXMPMCEEAPI(int) GetInterfaceVersion();

public:
  static MIKTEXMPMCEEAPI(MiKTeX::Core::VersionNumber) GetVersion();

public:
  static MIKTEXMPMCEEAPI(std::vector<MiKTeX::Core::LibraryVersion>) GetDependencies();
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
    return "MiKTeX App";
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
    return MiKTeX::Core::LibraryVersion("miktex-mpm", Header::GetName(), Header::GetVersion().ToString(), Runtime::GetVersion().ToString());
  }
};

MIKTEX_MPM_VI_END_NAMESPACE;

#endif
