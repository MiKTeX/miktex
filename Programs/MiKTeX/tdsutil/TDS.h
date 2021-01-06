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

#include <miktex/Util/PathName>

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
  MiKTeX::Util::PathName GetSourceDir() const
  {
    return MiKTeX::Util::PathName("source") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetTeXDir() const
  {
    return MiKTeX::Util::PathName("tex") / MiKTeX::Util::PathName(format) / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetDocDir() const
  {
    return MiKTeX::Util::PathName("doc") / MiKTeX::Util::PathName(format) / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetBibDir() const
  {
    return MiKTeX::Util::PathName("bibtex") / MiKTeX::Util::PathName("bib") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetBstDir() const
  {
    return MiKTeX::Util::PathName("bibtex") / MiKTeX::Util::PathName("bst") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetCsfDir() const
  {
    return MiKTeX::Util::PathName("bibtex") / MiKTeX::Util::PathName("csf") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetIstDir() const
  {
    return MiKTeX::Util::PathName("makeindex") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetDvipsDir() const
  {
    return MiKTeX::Util::PathName("dvips") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetMapDir() const
  {
    return MiKTeX::Util::PathName("fonts") / MiKTeX::Util::PathName("map") / MiKTeX::Util::PathName("dvips") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetEncDir() const
  {
    return MiKTeX::Util::PathName("fonts") / MiKTeX::Util::PathName("enc") / MiKTeX::Util::PathName("dvips") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetMfDir() const
  {
    return MiKTeX::Util::PathName("fonts") / MiKTeX::Util::PathName("source") / MiKTeX::Util::PathName(foundry) / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetTfmDir() const
  {
    return MiKTeX::Util::PathName("fonts") / MiKTeX::Util::PathName("tfm") / MiKTeX::Util::PathName(foundry) / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetOtfDir() const
  {
    return MiKTeX::Util::PathName("fonts") / MiKTeX::Util::PathName("opentype") / MiKTeX::Util::PathName(foundry) / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetTtfDir() const
  {
    return MiKTeX::Util::PathName("fonts") / MiKTeX::Util::PathName("truetype") / MiKTeX::Util::PathName(foundry) / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetPfbDir() const
  {
    return MiKTeX::Util::PathName("fonts") / MiKTeX::Util::PathName("type1") / MiKTeX::Util::PathName(foundry) / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetVfDir() const
  {
    return MiKTeX::Util::PathName("fonts") / MiKTeX::Util::PathName("vf") / MiKTeX::Util::PathName(foundry) / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetAfmDir() const
  {
    return MiKTeX::Util::PathName("fonts") / MiKTeX::Util::PathName("afm") / MiKTeX::Util::PathName(foundry) / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetMetaPostDir() const
  {
    return MiKTeX::Util::PathName("metapost") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetScriptDir() const
  {
    return MiKTeX::Util::PathName("scripts") / MiKTeX::Util::PathName(package);
  }

public:
  MiKTeX::Util::PathName GetMan1Dir() const
  {
    return MiKTeX::Util::PathName("doc") / MiKTeX::Util::PathName("man") / MiKTeX::Util::PathName("man1");
  }

private:
  std::string package;

private:
  std::string format = "format-not-set";

private:
  std::string foundry = "foundry-not-set";
};
