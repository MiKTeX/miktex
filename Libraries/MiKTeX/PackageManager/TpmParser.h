/* TpmParser.h:                                         -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if ! defined(F16B91F7E26E4BF69460B977B1C0BC5A)
#define F16B91F7E26E4BF69460B977B1C0BC5A

BEGIN_INTERNAL_NAMESPACE;

class TpmParser
{
public:
  TpmParser();

public:
  virtual ~TpmParser();

public:
  void Parse(const MiKTeX::Core::PathName & path);

public:
  void Parse(const MiKTeX::Core::PathName & path, const std::string & texmfPrefix);

  // get the result
public:
  const MiKTeX::Packages::PackageInfo & GetPackageInfo() const
  {
    return packageInfo;
  }

private:
  static void OnStartElement(void * pv, const XML_Char * lpszName, const XML_Char ** aAttr);

private:
  static void OnEndElement(void * pv, const XML_Char * lpszName);

private:
  static void OnCharacterData(void * pv, const XML_Char * lpsz, int len);

private:
  void GetFiles(const XML_Char * lpszFiles, std::vector<std::string> & files);

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
  void * pParser = nullptr;

private:
  std::string texMFPrefix;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceError;
};

END_INTERNAL_NAMESPACE;

#endif
