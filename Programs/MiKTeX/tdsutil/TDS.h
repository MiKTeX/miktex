/* TDS.h:                                               -*- C++ -*-

   Copyright (C) 2016-2017 Christian Schenk

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
   USA.  */

#pragma once

#include <string>

#include <miktex/Core/PathName>

class TDS
{
public:
  TDS(const std::string & package) :
    package(package)
  {
  }

public:
  void SetFormat(const std::string & format)
  {
    this->format = format;
  }

public:
  void SetFoundry(const std::string & foundry)
  {
    this->foundry = foundry;
  }

public:
  MiKTeX::Core::PathName GetSourceDir() const
  {
    return MiKTeX::Core::PathName("source") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetTeXDir() const
  {
    return MiKTeX::Core::PathName("tex") / MiKTeX::Core::PathName(format) / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetDocDir() const
  {
    return MiKTeX::Core::PathName("doc") / MiKTeX::Core::PathName(format) / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetBibDir() const
  {
    return MiKTeX::Core::PathName("bibtex") / MiKTeX::Core::PathName("bib") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetBstDir() const
  {
    return MiKTeX::Core::PathName("bibtex") / MiKTeX::Core::PathName("bst") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetCsfDir() const
  {
    return MiKTeX::Core::PathName("bibtex") / MiKTeX::Core::PathName("csf") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetIstDir() const
  {
    return MiKTeX::Core::PathName("makeindex") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetDvipsDir() const
  {
    return MiKTeX::Core::PathName("dvips") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetMapDir() const
  {
    return MiKTeX::Core::PathName("fonts") / MiKTeX::Core::PathName("map") / MiKTeX::Core::PathName("dvips") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetEncDir() const
  {
    return MiKTeX::Core::PathName("fonts") / MiKTeX::Core::PathName("enc") / MiKTeX::Core::PathName("dvips") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetMfDir() const
  {
    return MiKTeX::Core::PathName("fonts") / MiKTeX::Core::PathName("source") / MiKTeX::Core::PathName(foundry) / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetTfmDir() const
  {
    return MiKTeX::Core::PathName("fonts") / MiKTeX::Core::PathName("tfm") / MiKTeX::Core::PathName(foundry) / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetOtfDir() const
  {
    return MiKTeX::Core::PathName("fonts") / MiKTeX::Core::PathName("opentype") / MiKTeX::Core::PathName(foundry) / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetTtfDir() const
  {
    return MiKTeX::Core::PathName("fonts") / MiKTeX::Core::PathName("truetype") / MiKTeX::Core::PathName(foundry) / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetPfbDir() const
  {
    return MiKTeX::Core::PathName("fonts") / MiKTeX::Core::PathName("type1") / MiKTeX::Core::PathName(foundry) / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetVfDir() const
  {
    return MiKTeX::Core::PathName("fonts") / MiKTeX::Core::PathName("vf") / MiKTeX::Core::PathName(foundry) / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetAfmDir() const
  {
    return MiKTeX::Core::PathName("fonts") / MiKTeX::Core::PathName("afm") / MiKTeX::Core::PathName(foundry) / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetMetaPostDir() const
  {
    return MiKTeX::Core::PathName("metapost") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetScriptDir() const
  {
    return MiKTeX::Core::PathName("scripts") / MiKTeX::Core::PathName(package);
  }

public:
  MiKTeX::Core::PathName GetMan1Dir() const
  {
    return MiKTeX::Core::PathName("doc") / MiKTeX::Core::PathName("man") / MiKTeX::Core::PathName("man1");
  }

private:
  std::string package;

private:
  std::string format = "format-not-set";

private:
  std::string foundry = "foundry-not-set";
};
