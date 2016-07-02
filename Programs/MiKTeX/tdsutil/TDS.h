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

  void SetFormat(const std::string & format)
  {
    this->format = format;
  }

public:
  MiKTeX::Core::PathName GetSourceDir() const
  {
    return MiKTeX::Core::PathName("source") / package;
  }

public:
  MiKTeX::Core::PathName GetTeXDir() const
  {
    if (format.empty())
    {
      MIKTEX_UNEXPECTED();
    }
    return MiKTeX::Core::PathName("tex") / format / package;
  }

private:
  std::string package;

private:
  std::string format;
};
