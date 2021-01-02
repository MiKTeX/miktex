/* miktex/Locale/Translator.h:

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

#pragma once

#include <miktex/Locale/config.h>

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <miktex/Configuration/ConfigurationProvider>
#include <miktex/Resources/ResourceRepository>

MIKTEX_LOC_BEGIN_NAMESPACE;

class MIKTEXLOCTYPEAPI(Translator)
{
public:
  Translator() = delete;

public:
  MIKTEXLOCEXPORT MIKTEXTHISCALL Translator(const std::string& domain, MiKTeX::Resources::ResourceRepository* resources, std::shared_ptr<MiKTeX::Configuration::ConfigurationProvider> config);

public:
  Translator(const Translator& other) = delete;

public:
  Translator& operator=(const Translator& other) = delete;

public:
  Translator(const Translator&& other) = delete;

public:
  Translator& operator=(const Translator&& other) = delete;

public:
  virtual MIKTEXLOCEXPORT MIKTEXTHISCALL ~Translator() noexcept;

public:
  MIKTEXLOCTHISAPI(std::string) Translate(const char* msgId);

public:
  MIKTEXLOCTHISAPI(std::vector<std::string>) GetSystemUILanguages();

public:
  static MIKTEXLOCEXPORT std::tuple<std::string, std::string, std::string, std::string> MIKTEXCEECALL ParseLocaleIdentifier(const std::string& localeIdentifier);


private:
  void Init();

private:
  class impl;
  impl* pimpl;
};

MIKTEX_RES_END_NAMESPACE;
