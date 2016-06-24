/* Uri.cpp: Uri operations

   Copyright (C) 2008-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/Uri.h"

using namespace MiKTeX::Core;
using namespace std;

string ToString(const UriTextRangeA & textRange)
{
  string ret;
  if (textRange.first != nullptr)
  {
    MIKTEX_ASSERT(textRange.afterLast > textRange.first);
    ret.assign(textRange.first, textRange.afterLast - textRange.first);
  }
  return ret;
}

class MyUriData
{
public:
  UriParserStateA state;

public:
  UriUriA uri;

public:
  virtual ~MyUriData()
  {
    uriFreeUriMembersA(&uri);
  }
};

#define pData static_cast<MyUriData*>(this->p)

Uri::Uri() :
  p(nullptr)
{
}

Uri::Uri(const Uri & other)
{
  this->operator= (other);
}

Uri::Uri(const char * lpszUri) :
  p(new MyUriData())
{
  pData->state.uri = &pData->uri;
  int result = uriParseUriA(&pData->state, lpszUri);
  if (result == URI_ERROR_SYNTAX)
  {
    string uri = "http://";
    uri += lpszUri;
    result = uriParseUriA(&pData->state, uri.c_str());
  }
  if (result != URI_SUCCESS)
  {
    delete pData;
    p = nullptr;
    MIKTEX_FATAL_ERROR_2(T_("Bad URI."), "uri", lpszUri);
  }
}

Uri::~Uri()
{
  try
  {
    if (pData != nullptr)
    {
      delete pData;
    }
  }
  catch (const exception &)
  {
  }
  p = nullptr;
}

Uri & Uri::operator= (const Uri & other)
{
  UNIMPLEMENTED();
}

string Uri::GetScheme() const
{
  return ToString(pData->uri.scheme);
}

string Uri::GetHost() const
{
  return ToString(pData->uri.hostText);
}

int Uri::GetPort() const
{
  string portText = ToString(pData->uri.portText);
  if (!portText.empty())
  {
    return std::stoi(portText);
  }
  string scheme = GetScheme();
  if (scheme == "http")
  {
    return 80;
  }
  else if (scheme == "ftp")
  {
    return 23;
  }
  return -1;
}

string Uri::GetUserInfo() const
{
  return ToString(pData->uri.userInfo);
}
