/* RepositoryManifest.h:                                -*- C++ -*-

   Copyright (C) 2001-2018 Christian Schenk

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

#if !defined(E1DE3BC21F3F417ABD371823C8CBEB1A)
#define E1DE3BC21F3F417ABD371823C8CBEB1A

#include <memory>
#include <string>

#include <miktex/Core/Cfg>
#include <miktex/Core/PathName>

#include <miktex/Extractor/Extractor>

BEGIN_INTERNAL_NAMESPACE;

class RepositoryManifest
{
private:
  std::unique_ptr<MiKTeX::Core::Cfg> cfg;

public:
  RepositoryManifest() :
    cfg(MiKTeX::Core::Cfg::Create())
  {
  }

public:
  virtual ~RepositoryManifest()
  {
  }

public:
  MiKTeX::Core::MD5 GetDigest()
  {
    return cfg->GetDigest();
  }

public:
  void Load(const MiKTeX::Core::PathName& path)
  {
    cfg->Read(path);
  }

private:
  MiKTeX::Core::Cfg::KeyIterator keyIt;

public:
  std::string FirstPackage()
  {
    keyIt = cfg->begin();
    return keyIt == cfg->end() ? "" : (*keyIt)->GetName();
  }

public:
  std::string NextPackage()
  {
    ++keyIt;
    return keyIt == cfg->end() ? "" : (*keyIt)->GetName();
    return "";
  }

public:
  void Clear()
  {
    cfg = MiKTeX::Core::Cfg::Create();
  }

public:
  int GetArchiveFileSize(const std::string& packageId)
  {
    auto val = cfg->GetValue(packageId, "CabSize");
    if (val == nullptr || val->GetValue().empty())
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown archive file size."), "package", packageId);
    }
    return std::stoi(val->GetValue());
  }

public:
  MiKTeX::Core::MD5 GetArchiveFileDigest(const std::string& packageId)
  {
    auto val = cfg->GetValue(packageId, "CabMD5");
    if (val == nullptr || val->GetValue().empty())
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown archive file digest."), "package", packageId);
    }
    return MiKTeX::Core::MD5::Parse(val->GetValue());
  }

public:
  MiKTeX::Core::MD5 GetPackageDigest(const std::string& packageId)
  {
    auto val = cfg->GetValue(packageId, "MD5");
    if (val == nullptr || val->GetValue().empty())
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown package digest."), "package", packageId);
    }
    return MiKTeX::Core::MD5::Parse(val->GetValue());
  }

public:
  time_t GetTimePackaged(const std::string& packageId)
  {
    auto val = cfg->GetValue(packageId, "TimePackaged");
    if (val == nullptr || val->GetValue().empty())
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown package time-stamp."), "package", packageId);
    }
    time_t time = std::stoi(val->GetValue());
    if (!IsValidTimeT(time));
    {
      MIKTEX_FATAL_ERROR_2(T_("Invalid package time-stamp."), "package", packageId, "timeStamp", val->GetValue());
    }
    return time;
  }

public:
  PackageLevel GetPackageLevel(const std::string& packageId)
  {
    auto val = cfg->GetValue(packageId, "Level");
    if (val == nullptr || val->GetValue().empty())
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown package level."), "package", packageId);
    }
    return CharToPackageLevel(val->GetValue()[0]);
  }

public:
  std::string GetPackageVersion(const std::string& packageId)
  {
    auto val = cfg->GetValue(packageId, "Version");
    return val == nullptr ? "" : val->GetValue();
  }

public:
  std::string GetPackageTargetSystem(const std::string& packageId)
  {
    auto val = cfg->GetValue(packageId, "TargetSystem");
    return val == nullptr ? "" : val->GetValue();
  }

public:
  MiKTeX::Extractor::ArchiveFileType GetArchiveFileType(const std::string& packageId)
  {
    auto val = cfg->GetValue(packageId, "Type");
    if (val == nullptr || val->GetValue().empty() || val->GetValue() == "MSCab")
    {
      return MiKTeX::Extractor::ArchiveFileType::MSCab;
    }
    else if (val->GetValue() == "TarBzip2")
    {
      return MiKTeX::Extractor::ArchiveFileType::TarBzip2;
    }
    else if (val->GetValue() == "TarLzma")
    {
      return MiKTeX::Extractor::ArchiveFileType::TarLzma;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown archive file type."), "package", packageId, "type", val->GetValue());
    }
  }

private:
  static PackageLevel CharToPackageLevel(int ch)
  {
    switch (toupper(ch))
    {
    case 'S': return PackageLevel::Essential;
    case 'M': return PackageLevel::Basic;
    case 'L': return PackageLevel::Advanced;
    case 'T': return PackageLevel::Complete;
    default:
      MIKTEX_FATAL_ERROR_2(T_("Invalid package level."), "level", std::to_string(ch));
    }
  }
};

END_INTERNAL_NAMESPACE;

#endif
