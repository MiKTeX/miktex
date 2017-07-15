/* filetypes.cpp: MiKTeX file type registry

   Copyright (C) 1996-2017 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/ConfigNames.h"
#include "miktex/Core/CsvList.h"
#include "miktex/Core/Environment.h"
#include "miktex/Core/Paths.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

namespace std {
  template <> struct hash<FileType>
  {
    size_t operator()(const FileType& ft) const
    {
      return hash<int>()((int)ft);
    }
  };
}

unordered_map<FileType, string> fileTypeStrings = {
  { FileType::AFM, "afm" },
  { FileType::BASE, "base" },
  { FileType::BIB, "bib" },
  { FileType::BST, "bst" },
  { FileType::CID, "cid maps" },
  { FileType::CLUA, "clua" },
  { FileType::CNF, "cnf" },
  { FileType::CMAP, "cmap files" },
  { FileType::CWEB, "cweb" },
  { FileType::DB, "ls-R" },
  { FileType::DVI, "dvi" },
  { FileType::DVIPSCONFIG, "dvips config" },
  { FileType::ENC, "enc" },
  { FileType::EXE, "executables" },
  { FileType::FEA, "font feature files" },
  { FileType::FMT, "fmt" },
  { FileType::GF, "gf" },
  { FileType::GLYPHFONT, "bitmap font" },
  { FileType::GRAPHICS, "graphic/figure" },
  { FileType::HBF, "hbf" },
  { FileType::IST, "ist" },
  { FileType::LIG, "lig files" },
  { FileType::LUA, "lua" },
  { FileType::MAP, "map" },
  { FileType::MEM, "mem" },
  { FileType::MF, "mf" },
  { FileType::MFPOOL, "mfpool" },
  { FileType::MFT, "mft" },
  { FileType::MISCFONT, "misc fonts" },
  { FileType::MLBIB, "mlbib" },
  { FileType::MLBST, "mlbst" },
  { FileType::MP, "mp" },
  { FileType::MPPOOL, "mppool" },
  { FileType::MPSUPPORT, "MetaPost support" },
  { FileType::OCP, "ocp" },
  { FileType::OFM, "ofm" },
  { FileType::OPL, "opl" },
  { FileType::OTP, "otp" },
  { FileType::OTF, "opentype fonts" },
  { FileType::OVF, "ovf" },
  { FileType::OVP, "ovp" },
  { FileType::PDFTEXCONFIG, "pdftex config" },
  { FileType::PK, "pk" },
  { FileType::PROGRAMBINFILE, "other binary files" },
  { FileType::PROGRAMTEXTFILE, "other text files" },
  { FileType::PSHEADER, "PostScript header" },
  { FileType::SCRIPT, "texmfscripts" },
  { FileType::SFD, "subfont definition files" },
  { FileType::TCX, "tcx" },
  { FileType::TEX, "tex" },
  { FileType::TEXPOOL, "texpool" },
  { FileType::TEXSOURCE, "TeX system sources" },
  { FileType::TEXSYSDOC, "TeX system documentation" },
  { FileType::TFM, "tfm" },
  { FileType::TROFF, "troff fonts" },
  { FileType::TTF, "truetype fonts" },
  { FileType::TYPE1, "type1 fonts" },
  { FileType::TYPE42, "type42 fonts" },
  { FileType::VF, "vf" },
  { FileType::WEB, "web" },
  { FileType::WEB2C, "web2c files" }
};

void SessionImpl::RegisterFileType(FileType fileType)
{
  if ((size_t)fileType >= fileTypes.size())
  {
    fileTypes.resize((size_t)FileType::E_N_D);
  }
  if (fileTypes[(size_t)fileType].fileType == fileType)
  {
    // already registered
    return;
  }
  vector<string> extensions;
  vector<string> searchPath;
  switch (fileType)
  {
  case FileType::EXE:
  {
#if defined(MIKTEX_WINDOWS)
    string pathext;
    if (!Utils::GetEnvironmentString("PATHEXT", pathext) || pathext.empty())
    {
      pathext = ".COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC";
    }
    extensions = StringUtil::Split(pathext, ';');
#elif defined(MIKTEX_EXE_FILE_SUFFIX)
    extensions.push_back(MIKTEX_EXE_FILE_SUFFIX);
#endif
    PathName userBinDir = GetSpecialPath(SpecialPath::UserInstallRoot);
    userBinDir /= MIKTEX_PATH_BIN_DIR;
    userBinDir.Canonicalize();
    // FIXME: case-senstive
    if (!IsAdminMode() && std::find(searchPath.begin(), searchPath.end(), userBinDir.ToString()) == searchPath.end())
    {
      searchPath.push_back(userBinDir.ToString());
    }
    PathName commonBinDir = GetSpecialPath(SpecialPath::CommonInstallRoot);
    commonBinDir /= MIKTEX_PATH_BIN_DIR;
    commonBinDir.Canonicalize();
    if (std::find(searchPath.begin(), searchPath.end(), commonBinDir.ToString()) == searchPath.end())
    {
      searchPath.push_back(commonBinDir.ToString());
    }
    string str;
    if (Utils::GetEnvironmentString(MIKTEX_ENV_BIN_DIR, str))
    {
      PathName binDir = str;
      binDir.Canonicalize();
      if (std::find(searchPath.begin(), searchPath.end(), binDir.ToString()) == searchPath.end())
      {
        searchPath.push_back(binDir.ToString());
      }
    }
    PathName myLocation = GetMyLocation(false);
    if (std::find(searchPath.begin(), searchPath.end(), myLocation.ToString()) == searchPath.end())
    {
      searchPath.push_back(myLocation.ToString());
    }
    PathName myLocationCanon = GetMyLocation(true);
    if (std::find(searchPath.begin(), searchPath.end(), myLocationCanon.ToString()) == searchPath.end())
    {
      searchPath.push_back(myLocationCanon.ToString());
    }
    break;
  }
  case FileType::OTF:
    searchPath = StringUtil::Split(GetLocalFontDirectories(), PathName::PathNameDelimiter);
    break;
  case FileType::TTF:
    searchPath = StringUtil::Split(GetLocalFontDirectories(), PathName::PathNameDelimiter);
    break;
  case FileType::TYPE1:
    searchPath = StringUtil::Split(GetLocalFontDirectories(), PathName::PathNameDelimiter);
    break;
  }
  InternalFileTypeInfo fti;
  fti.fileType = fileType;
  fti.fileTypeString = fileTypeStrings[fileType];
  string section = string(MIKTEX_CONFIG_SECTION_CORE_FILETYPES) + "." + fti.fileTypeString;
  ConfigValue configValue = GetConfigValue(section, MIKTEX_CONFIG_VALUE_EXTENSIONS);
  if (configValue.HasValue())
  {
    fti.fileNameExtensions = GetConfigValue(section, MIKTEX_CONFIG_VALUE_EXTENSIONS).GetStringArray();
  }
  configValue = GetConfigValue(section, MIKTEX_CONFIG_VALUE_ALTEXTENSIONS);
  if (configValue.HasValue())
  {
    fti.alternateExtensions = configValue.GetStringArray();
  }
  configValue = GetConfigValue(section, MIKTEX_CONFIG_VALUE_PATHS);
  if (configValue.HasValue())
  {
    fti.searchPath = configValue.GetStringArray();
  }
  configValue = GetConfigValue(section, MIKTEX_CONFIG_VALUE_ENVVARS);
  if (configValue.HasValue())
  {
    fti.envVarNames = configValue.GetStringArray();
  }
  fti.fileNameExtensions.insert(fti.fileNameExtensions.end(), extensions.begin(), extensions.end());
  fti.searchPath.insert(fti.searchPath.begin(), searchPath.begin(), searchPath.end());
  fileTypes.resize((size_t)FileType::E_N_D);
  fileTypes[(size_t)fileType] = fti;
}

void SessionImpl::RegisterFileTypes()
{
  for (int ft = (int)FileType::None + 1; ft < (int)FileType::E_N_D; ++ft)
  {
    RegisterFileType((FileType)ft);
  }
}

InternalFileTypeInfo* SessionImpl::GetInternalFileTypeInfo(FileType fileType)
{
  RegisterFileType(fileType);
  return &fileTypes[(size_t)fileType];
}

FileTypeInfo SessionImpl::GetFileTypeInfo(FileType fileType)
{
  return *GetInternalFileTypeInfo(fileType);
}

FileType SessionImpl::DeriveFileType(const PathName& fileName)
{
  RegisterFileTypes();
  PathName extension(fileName.GetExtension());
  for (int idx = 1; idx < fileTypes.size(); ++idx)
  {
    const InternalFileTypeInfo& fti = fileTypes[idx];
    if (extension.Empty())
    {
      if (fti.fileTypeString == fileName)
      {
        return fti.fileType;
      }
    }
    else
    {
      if (std::find_if(fti.fileNameExtensions.begin(), fti.fileNameExtensions.end(), [extension](const string& ext) { return extension == ext; }) != fti.fileNameExtensions.end())
      {
        return fti.fileType;
      }
    }
  }
  return FileType::None;
}

vector<FileTypeInfo> SessionImpl::GetFileTypes()
{
  vector<FileTypeInfo> result;
  for (int ft = (int)FileType::None + 1; ft < (int)FileType::E_N_D; ++ft)
  {
    result.push_back(GetFileTypeInfo((FileType)ft));
  }
  return result;
}

void SessionImpl::ClearSearchVectors()
{
  for (InternalFileTypeInfo& info : fileTypes)
  {
    info.searchVec.clear();
  }
}
