/* CurlWebFile.cpp:

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

#include "StdAfx.h"

#if defined(HAVE_LIBCURL)

#include "internal.h"

#include "CurlWebFile.h"

using namespace MiKTeX::Trace;
using namespace std;

const int READ_TIMEOUT_SECONDS = 40;

CurlWebFile::CurlWebFile(shared_ptr<CurlWebSession> webSession, const std::string & url) :
  webSession(webSession),
  url(url),
  trace_mpm(TraceStream::Open(MIKTEX_TRACE_MPM))
{
  try
  {
    Initialize();
    webSession->Connect();
  }
  catch (const exception &)
  {
    if (initialized)
    {
      curl_multi_remove_handle(webSession->GetMultiHandle(), webSession->GetEasyHandle());
    }
    throw;
  }
}

CurlWebFile::~CurlWebFile()
{
  try
  {
    Close();
  }
  catch (const exception &)
  {
  }
}

void CurlWebFile::Initialize()
{
  webSession->SetOption(CURLOPT_URL, url.c_str());
  webSession->SetOption(CURLOPT_WRITEDATA, reinterpret_cast<void*>(this));
  curl_write_callback writeCallback = WriteCallback;
  webSession->SetOption(CURLOPT_WRITEFUNCTION, writeCallback);
  CURLMcode code = curl_multi_add_handle(webSession->GetMultiHandle(), webSession->GetEasyHandle());
  if (code != CURLM_OK && code != CURLM_CALL_MULTI_PERFORM)
  {
    MIKTEX_FATAL_ERROR(webSession->GetCurlErrorString(code));
  }
  initialized = true;
}

size_t CurlWebFile::WriteCallback(char * pData, size_t elemSize, size_t numElements, void * pv)
{
  try
  {
    CurlWebFile * This = reinterpret_cast<CurlWebFile*>(pv);
    This->TakeData(pData, elemSize * numElements);
    return elemSize * numElements;
  }
  catch (const exception &)
  {
    return 0;
  }
}

void CurlWebFile::TakeData(const void * pData, size_t size)
{
  const char * beg = reinterpret_cast<const char*>(pData);
  buffer.insert(buffer.end(), beg, beg + size);
}

size_t CurlWebFile::Read(void * pBuffer, size_t n)
{
  clock_t now = clock();
  clock_t due = now + READ_TIMEOUT_SECONDS * CLOCKS_PER_SEC;
  do
  {
    webSession->Perform();
  } while (buffer.size() < n && !webSession->IsReady() && clock() < due);
  if (buffer.size() == 0 && !webSession->IsReady())
  {
    MIKTEX_FATAL_ERROR(T_("A timeout was reached while receiving data from the server."));
  }
  n = min(n, buffer.size());
  if (n > 0)
  {
    memcpy(pBuffer, &this->buffer[0], n);
    buffer.erase(buffer.begin(), buffer.begin() + n);
  }
  return n;
}

void CurlWebFile::Close()
{
  if (initialized)
  {
    trace_mpm->WriteLine("libmpm", T_("closing Web file"));
    initialized = false;
    CURLMcode code = curl_multi_remove_handle(webSession->GetMultiHandle(), webSession->GetEasyHandle());
    if (code != CURLM_OK)
    {
      MIKTEX_FATAL_ERROR(webSession->GetCurlErrorString(code));
    }
  }
  buffer.clear();
}

#endif // libCURL
