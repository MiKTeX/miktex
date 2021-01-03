/* Translator.cpp:

   Copyright (C) 2020-2021 Christian Schenk

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
   USA. */

#include "config.h"

#if defined(MIKTEX_WINDOWS)
#include <Windows.h>
#endif

#include <locale>
#include <mutex>
#include <string>
#include <vector>

#if !defined(MIKTEX_LOC_STATIC)
#define WITH_BOOST_LOCALE
#endif

#if defined(WITH_BOOST_LOCALE)
#include <boost/locale.hpp>
#endif

#include <miktex/Configuration/ConfigNames>
#include <miktex/Util/CharBuffer>
#include <miktex/Util/StringUtil>

#include "miktex/Locale/Translator.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Locale;
using namespace MiKTeX::Resources;
using namespace MiKTeX::Util;

namespace {
  class Exception :
    public exception
  {
  public:
    Exception(const string& msg) :
      msg(msg)
    {
    }
  public:
    const char* what() const noexcept override
    {
      return msg.c_str();
    }
  private:
    string msg;
  };

  class InvalidLocaleIdentifier :
    public Exception
  {
  public:
    InvalidLocaleIdentifier(const string& localeIdentifier) :
      Exception("Invalid locale identifier: " + localeIdentifier)
    {
    }
  };
}

class Translator::impl
{
public:
  shared_ptr<ConfigurationProvider> config;
public:
  string domain;
public:
  once_flag initFlag;
public:
  ResourceRepository* resources = nullptr;
public:
  std::locale uiLocale;
};

Translator::Translator(const string& domain, ResourceRepository* resources, shared_ptr<ConfigurationProvider> config) :
  pimpl(new impl)
{
  pimpl->config = config;
  pimpl->domain = domain;
  pimpl->resources = resources;
}

Translator::~Translator()
{
  delete pimpl;
}

static vector<char> LoadFile(ResourceRepository* resources, string const& fileName, string const& encoding)
{
  if (encoding != "UTF-8" && encoding != "utf-8")
  {
    return vector<char>();
  }
  if (resources == nullptr)
  {
    return vector<char>();
  }
  const Resource& resource = resources->GetResource(fileName.c_str());
  if (resource.data == nullptr)
  {
    return vector<char>();
  }
  const char* data = static_cast<const char*>(resource.data);
  return vector<char>(data, data + resource.len);
}

void Translator::Init()
{
#if defined(WITH_BOOST_LOCALE)
  boost::locale::gnu_gettext::messages_info messagesInfo;
  messagesInfo.paths.push_back("/usr/share/locale");
  messagesInfo.domains.push_back(boost::locale::gnu_gettext::messages_info::domain(pimpl->domain));
  std::function<vector<char>(const string&, const string&)> callback = [this](const string& fileName, const string& encoding) { return LoadFile(pimpl->resources, fileName, encoding); };
  messagesInfo.callback = callback;
  boost::locale::generator gen;
  string localeIdentifier;
  auto uiLanguages = GetSystemUILanguages();
  if (!uiLanguages.empty())
  {
    localeIdentifier = uiLanguages[0] + ".UTF-8";
  }
  std::locale baseLocale = gen(localeIdentifier);
  boost::locale::info const& properties = std::use_facet<boost::locale::info>(baseLocale);
  messagesInfo.country = properties.country();
  messagesInfo.encoding = properties.encoding();
  messagesInfo.language = properties.language();
  messagesInfo.variant = properties.variant();
  pimpl->uiLocale = std::locale(baseLocale, boost::locale::gnu_gettext::create_messages_facet<char>(messagesInfo));
#endif
}

std::string Translator::Translate(const char* msgId)
{
#if defined(WITH_BOOST_LOCALE)
  std::call_once(pimpl->initFlag, [this]() { Init(); });
  return boost::locale::gettext(msgId, pimpl->uiLocale);
#else
  return msgId;
#endif
}

tuple<string, string, string, string> Translator::ParseLocaleIdentifier(const string& localeIdentifier)
{
  string country;
  string encoding;
  string language;
  string variant;
  size_t pos = localeIdentifier.find_first_of("_-.@");
  language = localeIdentifier.substr(0, pos);
  for (char& ch : language)
  {
    if (ch >= 'A' && ch <= 'Z')
    {
      ch = ch - 'A' + 'a';
    }
    else if (!(ch >= 'a' && ch <= 'z'))
    {
      throw InvalidLocaleIdentifier(localeIdentifier);
    }
  }
  if (pos == string::npos)
  {
    return make_tuple(language, country, encoding, variant);
  }
  if (localeIdentifier[pos] == '-' || localeIdentifier[pos] == '_')
  {
    pos += 1;
    auto nextPos = localeIdentifier.find_first_of(".@", pos);
    country = localeIdentifier.substr(pos, nextPos - pos);
    for (char& ch : country)
    {
      if (ch >= 'a' && ch <= 'z')
      {
        ch = ch - 'a' + 'A';
      }
      else if (!(ch >= 'A' && ch <= 'Z'))
      {
        throw InvalidLocaleIdentifier(localeIdentifier);
      }
    }
    pos = nextPos;
  }
  if (pos == string::npos)
  {
    return make_tuple(language, country, encoding, variant);
  }
  if (localeIdentifier[pos] == '.')
  {
    pos += 1;
    auto nextPos = localeIdentifier.find_first_of("@", pos);
    encoding = localeIdentifier.substr(pos, nextPos - pos);
    for (char& ch : encoding)
    {
      if (ch >= 'A' && ch <= 'Z')
      {
        ch = ch - 'A' + 'a';
      }
    }
    pos = nextPos;
  }
  if (pos == string::npos)
  {
    return make_tuple(language, country, encoding, variant);
  }
  if (localeIdentifier[pos] == '@')
  {
    pos += 1;
    variant = localeIdentifier.substr(pos);
    for (char& ch : encoding)
    {
      if (ch >= 'A' && ch <= 'Z')
      {
        ch = ch - 'A' + 'a';
      }
    }
  }
  return make_tuple(language, country, encoding, variant);
}

static bool GetEnvironmentString(const string& name, string& value)
{
#if defined(MIKTEX_WINDOWS)
  wchar_t* lpszValue = _wgetenv(CharBuffer<wchar_t>(name).GetData());
  if (lpszValue == nullptr)
  {
    return false;
  }
  else
  {
    value = CharBuffer<char>(lpszValue).GetData();
    return true;
  }
#else
  const char* lpszValue = getenv(name.c_str());
  if (lpszValue == nullptr)
  {
    return false;
  }
  else
  {
    value = lpszValue;
    return true;
  }
#endif
}

vector<string> Translator::GetSystemUILanguages()
{
  if (pimpl->config != nullptr)
  {
    auto configValue = pimpl->config->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_UI_LANGUAGES);
    if (configValue.HasValue())
    {
      vector<string> uiLanguages = configValue.GetStringArray();
      if (!uiLanguages.empty() && !uiLanguages[0].empty())
      {
        return uiLanguages;
      }
    }
  }
#if defined(MIKTEX_WINDOWS)
  ULONG numLanguages;
  PZZWSTR nullBuffer = nullptr;
  ULONG bufferSize = 0;
  if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLanguages, nullBuffer, &bufferSize))
  {
    return vector<string>();
  }
  CharBuffer<wchar_t> buffer(bufferSize);
  if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLanguages, &buffer[0], &bufferSize))
  {
    return vector<string>();
  }
  const wchar_t* bufPtr = buffer.GetData();
  vector<string> windowsUILanguages;
  for (auto n = 0; n < numLanguages; ++n)
  {
    windowsUILanguages.push_back(StringUtil::WideCharToUTF8(bufPtr));
    bufPtr += wcslen(bufPtr) + 1;
  }
  if (!windowsUILanguages.empty() && !windowsUILanguages[0].empty())
  {
    return windowsUILanguages;
  }
#endif
  string envValue;
  if (GetEnvironmentString("LANGUAGE", envValue))
  {
    auto languages = StringUtil::Split(envValue, ':');
    if (!languages.empty() && !languages[0].empty())
    {
      return languages;
    }
  }
  vector<string> result;
  for (const auto& envName : { "LC_ALL", "LC_MESSAGES", "LANG" })
  {
    string lang;
    if (GetEnvironmentString(envName, lang) && !lang.empty())
    {
      string language, country, encoding, variant;
      std::tie(language, country, encoding, variant) = ParseLocaleIdentifier(lang);
      if (language.empty())
      {
        continue;
      }
      if (country.empty())
      {
        result.push_back(language);
      }
      else
      {
        result.push_back(language + "_" + country);
      }
      break;
    }
  }
  return result;
}

