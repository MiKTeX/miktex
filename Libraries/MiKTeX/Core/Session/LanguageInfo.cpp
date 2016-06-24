/* LanguageInfo.cpp: language information

   Copyright (C) 2010-2016 Christian Schenk

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

#include "miktex/Core/Directory.h"
#include "miktex/Core/Paths.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

vector<LanguageInfo> SessionImpl::GetLanguages()
{
  vector<LanguageInfo> result;
  ReadLanguagesIni();
  for (const auto & l : languages)
  {
    result.push_back(l);
  }
  return result;
}

void SessionImpl::ReadLanguagesIni(const PathName & cfgFile)
{
  unique_ptr<Cfg> cfgLanguages(Cfg::Create());
  cfgLanguages->Read(cfgFile);
  bool custom = TryDeriveTEXMFRoot(cfgFile) != GetDistRoot();
  for (const shared_ptr<Cfg::Key> & key : cfgLanguages->GetKeys())
  {
    LanguageInfo_ languageInfo;
    vector<LanguageInfo_>::iterator itExisting;
    for (itExisting = languages.begin(); itExisting != languages.end(); ++itExisting)
    {
      if (itExisting->key == key->GetName())
      {
        languageInfo = *itExisting;
        break;
      }
    }
    string val;
    languageInfo.cfgFile = cfgFile;
    languageInfo.key = key->GetName();
    if (cfgLanguages->TryGetValue(key->GetName(), "synonyms", val))
    {
      languageInfo.synonyms = val;
    }
    if (cfgLanguages->TryGetValue(key->GetName(), "loader", val))
    {
      languageInfo.loader = val;
    }
    if (cfgLanguages->TryGetValue(key->GetName(), "patterns", val))
    {
      languageInfo.patterns = val;
    }
    if (cfgLanguages->TryGetValue(key->GetName(), "hyphenation", val))
    {
      languageInfo.hyphenation = val;
    }
    if (cfgLanguages->TryGetValue(key->GetName(), "luaspecial", val))
    {
      languageInfo.luaspecial = val;
    }
    if (cfgLanguages->TryGetValue(key->GetName(), "lefthyphenmin", val))
    {
      languageInfo.lefthyphenmin = std::stoi(val);
    }
    if (cfgLanguages->TryGetValue(key->GetName(), "righthyphenmin", val))
    {
      languageInfo.righthyphenmin = std::stoi(val);
    }
    if (cfgLanguages->TryGetValue(key->GetName(), "attributes", val))
    {
      languageInfo.exclude = (val == "exclude");
    }
    if (itExisting == languages.end())
    {
      languageInfo.custom = custom;
      if (key->GetName() == "english")
      {
        languages.insert(languages.begin(), languageInfo);
      }
      else
      {
        languages.push_back(languageInfo);
      }
    }
    else
    {
      *itExisting = languageInfo;
    }
  }
}

void SessionImpl::ReadLanguagesIni()
{
  if (!languages.empty())
  {
    return;
  }
  vector<PathName> iniFiles;
  if (!FindFile(MIKTEX_PATH_LANGUAGES_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, { FindFileOption::All }, iniFiles))
  {
    MIKTEX_FATAL_ERROR(T_("The configuration file languages.ini could not be found."));
  }
  for (vector<PathName>::const_reverse_iterator it = iniFiles.rbegin(); it != iniFiles.rend(); ++it)
  {
    ReadLanguagesIni(*it);
  }
  sort(languages.begin(), languages.end());
}

void SessionImpl::WriteLanguagesIni()
{
  unique_ptr<Cfg> cfgLanguages(Cfg::Create());
  for (vector<LanguageInfo_>::iterator it = languages.begin(); it != languages.end(); ++it)
  {
    if (it->custom)
    {
      if (!it->synonyms.empty())
      {
        cfgLanguages->PutValue(it->key, "synonyms", it->synonyms);
      }
      if (!it->loader.empty())
      {
        cfgLanguages->PutValue(it->key, "loader", it->loader);
      }
      if (!it->patterns.empty())
      {
        cfgLanguages->PutValue(it->key, "patterns", it->patterns);
      }
      if (!it->luaspecial.empty())
      {
        cfgLanguages->PutValue(it->key, "luaspecial", it->luaspecial);
      }
      if (it->lefthyphenmin != -1)
      {
        cfgLanguages->PutValue(it->key, "lefthyphenmin", std::to_string(it->lefthyphenmin));
      }
      if (it->righthyphenmin != -1)
      {
        cfgLanguages->PutValue(it->key, "righthyphenmin", std::to_string(it->righthyphenmin));
      }
    }
    if (it->exclude)
    {
      cfgLanguages->PutValue(it->key, "attributes", "exclude");
    }
    else
    {
      cfgLanguages->PutValue(it->key, "attributes", "");
    }
  }
  PathName pathLocalLanguagesIni(GetSpecialPath(SpecialPath::ConfigRoot), MIKTEX_PATH_LANGUAGES_INI);
  Directory::Create(PathName(pathLocalLanguagesIni).RemoveFileSpec());
  cfgLanguages->Write(pathLocalLanguagesIni);
  if (!Fndb::FileExists(pathLocalLanguagesIni))
  {
    Fndb::Add(pathLocalLanguagesIni);
  }
}

void SessionImpl::SetLanguageInfo(const LanguageInfo & languageInfo)
{
  ReadLanguagesIni();
  vector<LanguageInfo_>::iterator it;
  for (it = languages.begin(); it != languages.end(); ++it)
  {
    if (it->key == languageInfo.key)
    {
      bool custom = it->custom;
      if (!custom)
      {
        bool cannotChange = false;
        if (languageInfo.custom)
        {
          cannotChange = true;
        }
        if (languageInfo.synonyms != it->synonyms)
        {
          cannotChange = true;
        }
        if (languageInfo.loader != it->loader)
        {
          cannotChange = true;
        }
        if (languageInfo.patterns != it->patterns)
        {
          cannotChange = true;
        }
        if (languageInfo.hyphenation != it->hyphenation)
        {
          cannotChange = true;
        }
        if (languageInfo.luaspecial != it->luaspecial)
        {
          cannotChange = true;
        }
        if (languageInfo.lefthyphenmin != it->lefthyphenmin)
        {
          cannotChange = true;
        }
        if (languageInfo.righthyphenmin != it->righthyphenmin)
        {
          cannotChange = true;
        }
        if (cannotChange)
        {
          MIKTEX_FATAL_ERROR_2(T_("Built-in language definitions may not be changed."), "key", languageInfo.key);
        }
      }
      if (languageInfo.exclude && languageInfo.key == "english")
      {
        MIKTEX_FATAL_ERROR(T_("The English language may not be excluded."));
      }
      *it = languageInfo;
      it->custom = custom;
      break;
    }
  }
  if (it == languages.end())
  {
    if (!languageInfo.custom)
    {
      MIKTEX_UNEXPECTED();
    }
    languages.push_back(languageInfo);
  }
  WriteLanguagesIni();
}
