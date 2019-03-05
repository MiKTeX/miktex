/* ExpatTpmParser.h:                                    -*- C++ -*-

   Copyright (C) 2001-2019 Christian Schenk

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

#if !defined(A9A53FEF7D1649898F9735A9554FBE4D)
#define A9A53FEF7D1649898F9735A9554FBE4D

#include <stack>
#include <vector>

#include <expat.h>

#include <miktex/Core/PathName>
#include <miktex/Util/CharBuffer>

#include <miktex/PackageManager/PackageManager>

#include "TpmParser.h"

MPM_INTERNAL_BEGIN_NAMESPACE;

class ExpatTpmParser :
  public TpmParser
{
public:
  ExpatTpmParser();

public:
  ~ExpatTpmParser() override;

public:
  void Parse(const MiKTeX::Core::PathName& path, const std::string& texmfPrefix) override;

  // get the result
public:
  const MiKTeX::Packages::PackageInfo& GetPackageInfo() const override
  {
    return packageInfo;
  }

private:
  static void OnStartElement(void* pv, const XML_Char* name, const XML_Char** aAttr);

private:
  static void OnEndElement(void* pv, const XML_Char* name);

private:
  static void OnCharacterData(void* pv, const XML_Char* lpsz, int len);

private:
  void GetFiles(const XML_Char* text, std::vector<std::string>& files);

  // the result
private:
  MiKTeX::Packages::PackageInfo packageInfo;

  // character buffer
private:
  MiKTeX::Util::CharBuffer<XML_Char, 8192> charBuffer;

  // element stack
private:
  std::stack<std::string> elementStack;

private:
  void* parser = nullptr;

private:
  std::string texMFPrefix;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceError;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceStopWatch;
};

MPM_INTERNAL_END_NAMESPACE;

#endif
