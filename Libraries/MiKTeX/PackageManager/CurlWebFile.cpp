/* CurlWebFile.cpp:

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

#include "StdAfx.h"

#if defined(HAVE_LIBCURL)

#include "internal.h"

#include "CurlWebFile.h"

using namespace MiKTeX::Trace;
using namespace std;

const int READ_TIMEOUT_SECONDS = 40;

CurlWebFile::CurlWebFile(shared_ptr<CurlWebSession> webSession, const std::string& url, const std::unordered_map<std::string, std::string>& formData) :
  webSession(webSession),
  url(url),
  trace_mpm(TraceStream::Open(MIKTEX_TRACE_MPM))
{
  try
  {
    for (const auto& kv : formData)
    {
      if (!urlEncodedpostFields.empty())
      {
        urlEncodedpostFields += "&";
      }
      urlEncodedpostFields +=  webSession->UrlEncode(kv.first) + "=" +  webSession->UrlEncode(kv.second);
    }
    Initialize();
    webSession->Connect();
  }
  catch (const exception&)
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
  catch (const exception&)
  {
  }
}

void CurlWebFile::Initialize()
{
  webSession->SetOption(CURLOPT_URL, url.c_str());
  if (!urlEncodedpostFields.empty())
  {
    webSession->SetOption(CURLOPT_POSTFIELDS, urlEncodedpostFields.data());
  }
  else
  {
    webSession->SetOption(CURLOPT_HTTPGET, 1);
  }
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

size_t CurlWebFile::WriteCallback(char* data, size_t elemSize, size_t numElements, void* pv)
{
  try
  {
    CurlWebFile* This = reinterpret_cast<CurlWebFile*>(pv);
    size_t size = elemSize * numElements;
    if (!This->buffer.CanWrite(size))
    {
      size_t newCapacity = This->buffer.GetCapacity() + 2 * size;
      This->trace_mpm->WriteFormattedLine("libmpm", T_("reserve buffer: %u"), (unsigned)newCapacity);
      This->buffer.Reserve(newCapacity);
      MIKTEX_ASSERT(This->buffer.CanWrite(size));
    }
    This->buffer.Write(data, size);
    return size;
  }
  catch (const exception&)
  {
    return 0;
  }
}

size_t CurlWebFile::Read(void* data, size_t n)
{
  clock_t now = clock();
  clock_t due = now + READ_TIMEOUT_SECONDS * CLOCKS_PER_SEC;
  while (buffer.GetSize() < n && !webSession->IsReady() && clock() < due)
  {
    webSession->Perform();
  }
  if (buffer.GetSize() == 0 && !webSession->IsReady())
  {
    MIKTEX_FATAL_ERROR(T_("A timeout was reached while receiving data from the server."));
  }
  n = min(n, buffer.GetSize());
  if (n > 0)
  {
    buffer.Read(data, n);
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
  buffer.Clear();
}

#endif // libCURL
