/* FormatInfo.cpp: format file info

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

#include "config.h"

#include <miktex/Core/Directory>
#include <miktex/Core/Paths>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

vector<FormatInfo> SessionImpl::GetFormats()
{
  ReadFormatsIni();
  vector<FormatInfo> result;
  for (const auto& f : formats)
  {
    result.push_back(f);
  }
  return result;
}

FormatInfo SessionImpl::GetFormatInfo(const string& key)
{
  FormatInfo formatInfo;
  if (!TryGetFormatInfo(key, formatInfo))
  {
    INVALID_ARGUMENT("key", key);
  }
  return formatInfo;
}

bool SessionImpl::TryGetFormatInfo(const string& key, FormatInfo& formatInfo)
{
  ReadFormatsIni();
  for (const FormatInfo& fmt : formats)
  {
    if (PathName::Equals(fmt.key, key))
    {
      formatInfo = fmt;
      return true;
    }
  }
  return false;
}

void SessionImpl::ReadFormatsIni(const PathName& cfgFile)
{
  unique_ptr<Cfg> cfgFormats(Cfg::Create());
  cfgFormats->Read(cfgFile);
  unsigned distRoot = GetInstallRoot();
  bool isCustom = distRoot != INVALID_ROOT_INDEX && DeriveTEXMFRoot(cfgFile) != distRoot;
  for (const shared_ptr<Cfg::Key>& key : *cfgFormats)
  {
    FormatInfo_ formatInfo;
    vector<FormatInfo_>::iterator it;
    for (it = formats.begin(); it != formats.end(); ++it)
    {
      if (PathName::Equals(it->key, key->GetName()))
      {
        formatInfo = *it;
        break;
      }
    }
    string val;
    formatInfo.cfgFile = cfgFile;
    formatInfo.key = key->GetName();
    if (cfgFormats->TryGetValueAsString(key->GetName(), "name", val))
    {
      formatInfo.name = val;
    }
    else
    {
      formatInfo.name = key->GetName();
    }
    if (cfgFormats->TryGetValueAsString(key->GetName(), "description", val))
    {
      formatInfo.description = val;
    }
    if (cfgFormats->TryGetValueAsString(key->GetName(), "compiler", val))
    {
      formatInfo.compiler = val;
    }
    if (cfgFormats->TryGetValueAsString(key->GetName(), "input", val))
    {
      formatInfo.inputFile = val;
    }
    if (cfgFormats->TryGetValueAsString(key->GetName(), "output", val))
    {
      formatInfo.outputFile = val;
    }
    if (cfgFormats->TryGetValueAsString(key->GetName(), "preloaded", val))
    {
      formatInfo.preloaded = val;
    }
    if (cfgFormats->TryGetValueAsString(key->GetName(), "attributes", val))
    {
      formatInfo.exclude = false;
      formatInfo.noExecutable = false;
      for (const string& flag : StringUtil::Split(val, ','))
      {
        if (flag == "exclude")
        {
          formatInfo.exclude = true;
        }
        else if (flag == "noexe")
        {
          formatInfo.noExecutable = true;
        }
      }
    }
    if (cfgFormats->TryGetValueAsString(key->GetName(), "arguments", val))
    {
      formatInfo.arguments = val;
    }
    if (it == formats.end())
    {
      formatInfo.custom = isCustom;
      formats.push_back(formatInfo);
    }
    else
    {
      *it = formatInfo;
    }
  }
}

void SessionImpl::ReadFormatsIni()
{
  if (!formats.empty())
  {
    return;
  }
  vector<PathName> iniFiles;
  if (!FindFile(MIKTEX_PATH_FORMATS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, { FindFileOption::All }, iniFiles))
  {
    MIKTEX_FATAL_ERROR(T_("The configuration file formats.ini could not be found."));
  }
  for (vector<PathName>::const_reverse_iterator it = iniFiles.rbegin(); it != iniFiles.rend(); ++it)
  {
    ReadFormatsIni(*it);
  }
}

void SessionImpl::WriteFormatsIni()
{
  unique_ptr<Cfg> cfgFormats(Cfg::Create());

  for (const FormatInfo_& fmt : formats)
  {
    if (fmt.custom)
    {
      if (PathName::Equals(fmt.key, fmt.name))
      {
        cfgFormats->PutValue(fmt.key, "name", fmt.name);
      }
      if (!fmt.description.empty())
      {
        cfgFormats->PutValue(fmt.key, "description", fmt.description);
      }
      if (fmt.compiler.empty())
      {
        MIKTEX_FATAL_ERROR_2(T_("Invalid custom format definition: no compiler specified."), "format", fmt.key);
      }
      cfgFormats->PutValue(fmt.key, "compiler", fmt.compiler);
      if (fmt.inputFile.empty())
      {
        MIKTEX_FATAL_ERROR_2(T_("Invalid custom format definition: no input file specified."), "format", fmt.key);
      }
      cfgFormats->PutValue(fmt.key, "input", fmt.inputFile);
      if (!fmt.outputFile.empty())
      {
        cfgFormats->PutValue(fmt.key, "output", fmt.outputFile);
      }
      if (!fmt.preloaded.empty())
      {
        cfgFormats->PutValue(fmt.key, "preloaded", fmt.preloaded);
      }
      if (!fmt.arguments.empty())
      {
        cfgFormats->PutValue(fmt.key, "arguments", fmt.arguments);
      }
    }
    string attributes;
    if (fmt.exclude)
    {
      if (!attributes.empty())
      {
        attributes += ',';
      }
      attributes += "exclude";
    }
    if (fmt.noExecutable)
    {
      if (!attributes.empty())
      {
        attributes += ',';
      }
      attributes += "noexe";
    }
    cfgFormats->PutValue(fmt.key, "attributes", attributes);
  }

  PathName pathLocalFormatsIni(GetSpecialPath(SpecialPath::ConfigRoot), MIKTEX_PATH_FORMATS_INI);

  Directory::Create(PathName(pathLocalFormatsIni).RemoveFileSpec());

  cfgFormats->Write(pathLocalFormatsIni);

  cfgFormats = nullptr;

  if (!Fndb::FileExists(pathLocalFormatsIni))
  {
    Fndb::Add({ { pathLocalFormatsIni } });
  }
}

void SessionImpl::DeleteFormatInfo(const string& key)
{
  ReadFormatsIni();
  for (vector<FormatInfo_>::iterator it = formats.begin(); it != formats.end(); ++it)
  {
    if (PathName::Equals(it->key, key))
    {
      if (!it->custom)
      {
        MIKTEX_FATAL_ERROR(T_("Built-in format definitions may not be deleted."));
      }
      formats.erase(it);
      WriteFormatsIni();
      return;
    }
  }
  MIKTEX_FATAL_ERROR_2(T_("The format could not be found."), "formatName", key);
}

void SessionImpl::SetFormatInfo(const FormatInfo& formatInfo)
{
  ReadFormatsIni();
  vector<FormatInfo_>::iterator it;
  for (it = formats.begin(); it != formats.end(); ++it)
  {
    if (PathName::Equals(it->key, formatInfo.key))
    {
      bool custom = it->custom;
      if (!custom)
      {
        bool cannotChange = false;
        if (formatInfo.custom)
        {
          cannotChange = true;
        }
        if (formatInfo.name != it->name)
        {
          cannotChange = true;
        }
        if (formatInfo.compiler != it->compiler)
        {
          cannotChange = true;
        }
        if (formatInfo.inputFile != it->inputFile)
        {
          cannotChange = true;
        }
        if (formatInfo.outputFile != it->outputFile)
        {
          cannotChange = true;
        }
        if (formatInfo.preloaded != it->preloaded)
        {
          cannotChange = true;
        }
        if (formatInfo.arguments != it->arguments)
        {
          cannotChange = true;
        }
        if (cannotChange)
        {
          MIKTEX_FATAL_ERROR_2(T_("Built-in format definitions may not be changed."), "formatName", formatInfo.name);
        }
      }
      *it = formatInfo;
      it->custom = custom;
      break;
    }
  }
  if (it == formats.end())
  {
    if (!formatInfo.custom)
    {
      MIKTEX_UNEXPECTED();
    }
    formats.push_back(formatInfo);
  }
  WriteFormatsIni();
}
