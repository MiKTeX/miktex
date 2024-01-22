/* TDS.h:                                               -*- C++ -*-

   Copyright (C) 2016-2024 Christian Schenk

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
    return MiKTeX::Util::PathName("source") / package;
  }

public:
  MiKTeX::Util::PathName GetTeXDir() const
  {
    return MiKTeX::Util::PathName("tex") / format / package;
  }

public:
  MiKTeX::Util::PathName GetDocDir() const
  {
    return MiKTeX::Util::PathName("doc") / format / package;
  }

public:
  MiKTeX::Util::PathName GetBibDir() const
  {
    return MiKTeX::Util::PathName("bibtex") / "bib" / package;
  }

public:
  MiKTeX::Util::PathName GetBstDir() const
  {
    return MiKTeX::Util::PathName("bibtex") / "bst" / package;
  }

public:
  MiKTeX::Util::PathName GetCsfDir() const
  {
    return MiKTeX::Util::PathName("bibtex") / "csf" / package;
  }

public:
  MiKTeX::Util::PathName GetIstDir() const
  {
    return MiKTeX::Util::PathName("makeindex") / package;
  }

public:
  MiKTeX::Util::PathName GetDvipsDir() const
  {
    return MiKTeX::Util::PathName("dvips") / package;
  }

public:
  MiKTeX::Util::PathName GetMapDir() const
  {
    return MiKTeX::Util::PathName("fonts") / "map" / "dvips" / package;
  }

public:
  MiKTeX::Util::PathName GetEncDir() const
  {
    return MiKTeX::Util::PathName("fonts") / "enc" / "dvips" / package;
  }

public:
  MiKTeX::Util::PathName GetMfDir() const
  {
    return MiKTeX::Util::PathName("fonts") / "source" / foundry / package;
  }

public:
  MiKTeX::Util::PathName GetTfmDir() const
  {
    return MiKTeX::Util::PathName("fonts") / "tfm" / foundry / package;
  }

public:
  MiKTeX::Util::PathName GetOtfDir() const
  {
    return MiKTeX::Util::PathName("fonts") / "opentype" / foundry / package;
  }

public:
  MiKTeX::Util::PathName GetTtfDir() const
  {
    return MiKTeX::Util::PathName("fonts") / "truetype" / foundry / package;
  }

public:
  MiKTeX::Util::PathName GetPfbDir() const
  {
    return MiKTeX::Util::PathName("fonts") / "type1" / foundry / package;
  }

public:
  MiKTeX::Util::PathName GetVfDir() const
  {
    return MiKTeX::Util::PathName("fonts") / "vf" / foundry / package;
  }

public:
  MiKTeX::Util::PathName GetAfmDir() const
  {
    return MiKTeX::Util::PathName("fonts") / "afm" / foundry / package;
  }

public:
  MiKTeX::Util::PathName GetMetaPostDir() const
  {
    return MiKTeX::Util::PathName("metapost") / package;
  }

public:
  MiKTeX::Util::PathName GetScriptDir() const
  {
    return MiKTeX::Util::PathName("scripts") / package;
  }

public:
  MiKTeX::Util::PathName GetMan1Dir() const
  {
    return MiKTeX::Util::PathName("doc") / "man" / "man1";
  }

private:
  std::string package;

private:
  std::string format = "format-not-set";

private:
  std::string foundry = "foundry-not-set";
};
