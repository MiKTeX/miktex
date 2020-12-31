/* Translator.cpp:

   Copyright (C) 2020 Christian Schenk

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


#include <mutex>
#include <string>
#include <vector>

#if !defined(MIKTEX_SETUP_STATIC)
#define WITH_BOOST_LOCALE
#endif

#if defined(WITH_BOOST_LOCALE)
#include <boost/locale.hpp>
#endif

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Utils.h>

#include "ResourceRepository.h"
#include "Translator.h"

using namespace std;

using namespace MiKTeX::Core;

class Translator::impl
{
public:
  string domain;
public:
  const ResourceRepository* resources;
public:
  std::locale uiLocale;
public:
  std::once_flag initFlag;
};

Translator::Translator(const string& domain, const ResourceRepository& resources) :
  pimpl(new impl)
{
  pimpl->domain = domain;
  pimpl->resources = &resources;
}

Translator::~Translator()
{
  delete pimpl;
}

vector<char> LoadFile(const ResourceRepository& resources, string const& fileName, string const& encoding)
{
  if (encoding != "UTF-8" && encoding != "utf-8")
  {
    return vector<char>();
  }
  const Resource& resource = resources.GetResource(fileName.c_str());
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
  messagesInfo.paths.push_back("/i18n");
  messagesInfo.domains.push_back(boost::locale::gnu_gettext::messages_info::domain(pimpl->domain));
  std::function<vector<char>(const string&, const string&)> callback = [this](const string& fileName, const string& encoding) { return LoadFile(*pimpl->resources, fileName, encoding); };
  messagesInfo.callback = callback;
  boost::locale::generator gen;
  string localeIdentifier;
  auto uiLanguages = Utils::GetUILanguages();
  if (!uiLanguages.empty())
  {
    localeIdentifier = fmt::format("{0}.UTF-8", uiLanguages[0]);
  }
  std::locale base_locale = gen(localeIdentifier);
  boost::locale::info const& properties = std::use_facet<boost::locale::info>(base_locale);
  messagesInfo.language = properties.language();
  messagesInfo.country = properties.country();
  messagesInfo.encoding = properties.encoding();
  messagesInfo.variant = properties.variant();
  pimpl->uiLocale = std::locale(base_locale, boost::locale::gnu_gettext::create_messages_facet<char>(messagesInfo));
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
