/* Translator.h:

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

#pragma once

#include <string>

#include <ResourceRepository.h>

class Translator
{
public:
  Translator() = delete;

public:
  Translator(const std::string& domain, const ResourceRepository& resources);

public:
  Translator(const Translator& other) = delete;

public:
  Translator& operator=(const Translator& other) = delete;

public:
  Translator(const Translator&& other) = delete;

public:
  Translator& operator=(const Translator&& other) = delete;

public:
  virtual ~Translator();

public:
  std::string Translate(const char* msgId);

private:
  void Init();

private:
  class impl;
  impl* pimpl;
};
