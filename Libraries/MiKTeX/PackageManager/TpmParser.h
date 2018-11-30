/* TpmParser.h:                                         -*- C++ -*-

   Copyright (C) 2001-2018 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if !defined(F16B91F7E26E4BF69460B977B1C0BC5A)
#define F16B91F7E26E4BF69460B977B1C0BC5A

#include <memory>
#include <string>

#include <miktex/Core/PathName>
#include <miktex/Core/Session>

#include <miktex/PackageManager/PackageManager>

BEGIN_INTERNAL_NAMESPACE;

class MIKTEXNOVTABLE TpmParser
{
public:
  virtual ~TpmParser() = 0;

public:
  virtual void Parse(const MiKTeX::Core::PathName& path, const std::string& texmfPrefix) = 0;

public:
  virtual const MiKTeX::Packages::PackageInfo& GetPackageInfo() const = 0;

public:
  void Parse(const MiKTeX::Core::PathName& path);

public:
  static std::unique_ptr<TpmParser> Create();
};

END_INTERNAL_NAMESPACE;

#endif
