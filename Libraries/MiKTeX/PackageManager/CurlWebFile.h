/* CurlWebFile.h:                                       -*- C++ -*-

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

#if defined(HAVE_LIBCURL) && ! defined(DB88FC1DC9B0497C965F3CFB916FA93A)
#define DB88FC1DC9B0497C965F3CFB916FA93A

#include "WebFile.h"
#include "CurlWebSession.h"

BEGIN_INTERNAL_NAMESPACE;

class
  CurlWebFile : public WebFile
{
public:
  CurlWebFile(CurlWebSession * pSession, const char * lpszUrl);

public:
  virtual ~CurlWebFile();

public:
  virtual size_t Read(void * pBuffer, size_t n);

public:
  virtual void Close();

private:
  static size_t WriteCallback(char * pData, size_t elemSize, size_t numElements, void * pv);

private:
  void TakeData(const void * pData, size_t size);

private:
  void Initialize();

private:
  bool initialized = false;

private:
  CurlWebSession * pSession;

private:
  std::string url;

private:
  std::vector<char> buffer;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;
};

END_INTERNAL_NAMESPACE;

#endif // libCURL
