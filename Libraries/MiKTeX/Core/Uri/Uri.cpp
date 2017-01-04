/* Uri.cpp: Uri operations

   Copyright (C) 2008-2017 Christian Schenk

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
using namespace MiKTeX::Util;
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

class Uri::impl
{
public:
  UriParserStateA state;

public:
  UriUriA uri;

public:
  virtual ~impl()
  {
    uriFreeUriMembersA(&uri);
  }

public:
  CharBuffer<char> buf;
};

Uri::Uri(const std::string & uri) :
  pimpl(make_unique<impl>())
{
  pimpl->buf = uri.c_str();
  pimpl->state.uri = &pimpl->uri;
  int result = uriParseUriA(&pimpl->state, pimpl->buf.GetData());
  if (result == URI_ERROR_SYNTAX)
  {
    pimpl->buf = "http://";
    pimpl->buf += uri.c_str();
    result = uriParseUriA(&pimpl->state, pimpl->buf.GetData());
  }
  if (result != URI_SUCCESS)
  {
    MIKTEX_FATAL_ERROR_2(T_("Bad URI."), "uri", uri);
  }
}

Uri::~Uri() noexcept
{
}

string Uri::GetScheme() const
{
  return ToString(pimpl->uri.scheme);
}

string Uri::GetHost() const
{
  return ToString(pimpl->uri.hostText);
}

int Uri::GetPort() const
{
  string portText = ToString(pimpl->uri.portText);
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
  return ToString(pimpl->uri.userInfo);
}
