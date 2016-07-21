/* TDS.h:                                               -*- C++ -*-

   Copyright (C) 2016 Christian Schenk

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
    return MiKTeX::Core::PathName("source") / package;
  }

public:
  MiKTeX::Core::PathName GetTeXDir() const
  {
    return MiKTeX::Core::PathName("tex") / format / package;
  }

public:
  MiKTeX::Core::PathName GetDocDir() const
  {
    return MiKTeX::Core::PathName("doc") / format / package;
  }

public:
  MiKTeX::Core::PathName GetBibDir() const
  {
    return MiKTeX::Core::PathName("bibtex") / "bib" / package;
  }

public:
  MiKTeX::Core::PathName GetBstDir() const
  {
    return MiKTeX::Core::PathName("bibtex") / "bst" / package;
  }

public:
  MiKTeX::Core::PathName GetCsfDir() const
  {
    return MiKTeX::Core::PathName("bibtex") / "csf" / package;
  }

public:
  MiKTeX::Core::PathName GetIstDir() const
  {
    return MiKTeX::Core::PathName("makeindex") / package;
  }

public:
  MiKTeX::Core::PathName GetDvipsDir() const
  {
    return MiKTeX::Core::PathName("dvips") / package;
  }

public:
  MiKTeX::Core::PathName GetMapDir() const
  {
    return MiKTeX::Core::PathName("fonts") / "map" / "dvips" / package;
  }

public:
  MiKTeX::Core::PathName GetEncDir() const
  {
    return MiKTeX::Core::PathName("fonts") / "enc" / "dvips" / package;
  }

public:
  MiKTeX::Core::PathName GetMfDir() const
  {
    return MiKTeX::Core::PathName("fonts") / "source" / foundry / package;
  }

public:
  MiKTeX::Core::PathName GetTfmDir() const
  {
    return MiKTeX::Core::PathName("fonts") / "tfm" / foundry / package;
  }

public:
  MiKTeX::Core::PathName GetOtfDir() const
  {
    return MiKTeX::Core::PathName("fonts") / "opentype" / foundry / package;
  }

public:
  MiKTeX::Core::PathName GetPfbDir() const
  {
    return MiKTeX::Core::PathName("fonts") / "type1" / foundry / package;
  }

public:
  MiKTeX::Core::PathName GetVfDir() const
  {
    return MiKTeX::Core::PathName("fonts") / "vf" / foundry / package;
  }

public:
  MiKTeX::Core::PathName GetAfmDir() const
  {
    return MiKTeX::Core::PathName("fonts") / "afm" / foundry / package;
  }

public:
  MiKTeX::Core::PathName GetMetaPostDir() const
  {
    return MiKTeX::Core::PathName("metapost") / package;
  }

public:
  MiKTeX::Core::PathName GetScriptDir() const
  {
    return MiKTeX::Core::PathName("scripts") / package;
  }

public:
  MiKTeX::Core::PathName GetMan1Dir() const
  {
    return MiKTeX::Core::PathName("doc") / "man" / "man1";
  }

private:
  std::string package;

private:
  std::string format = "format-not-set";

private:
  std::string foundry = "foundry-not-set";
};
