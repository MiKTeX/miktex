/* CurlWebSession.h:                                    -*- C++ -*-

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

#if defined(HAVE_LIBCURL) && ! defined(A9010DA64FD34372A24071A29FA59EEC)
#define A9010DA64FD34372A24071A29FA59EEC

BEGIN_INTERNAL_NAMESPACE;

class CurlWebSession : public WebSession
{
public:
  CurlWebSession(IProgressNotify_ * pIProgressNotify);

public:
  virtual ~CurlWebSession();

public:
  virtual WebFile * OpenUrl(const char * lpszUrl);

public:
  virtual void Dispose();

public:
  void Connect();

public:
  void Perform();

private:
  void ReadInformationals();

public:
  std::string GetCurlErrorString(CURLMcode code) const
  {
#if LIBCURL_VERSION_NUM >= 0x70c00
    return curl_multi_strerror(code);
#else
    std::string str = T_("The cURL multi interface returned an error code: ");
    str += std::to_string(code);
    return str;
#endif
  }

private:
  CURL * pCurl = nullptr;

private:
  CURLM * pCurlm = nullptr;

private:
  int runningHandles = -1;

private:
  IProgressNotify_ * pIProgressNotify;

public:
  bool IsReady() const
  {
    return runningHandles == 0;
  }

public:
  CURL * GetEasyHandle() const throw ()
  {
    return pCurl;
  }

public:
  CURLM * GetMultiHandle() const throw ()
  {
    return pCurlm;
  }

public:
  std::string GetCurlErrorString(CURLcode code) const
  {
#if LIBCURL_VERSION_NUM >= 0x70c00
    return curl_easy_strerror(code);
#else
    std::string str = T_("The cURL easy interface returned an error code: ");
    str += std::to_string(code);
    return str;
#endif
  }

public:
  template<class T> void SetOption(CURLoption option, T val) const
  {
    CURLcode code = curl_easy_setopt(pCurl, option, val);
    if (code != CURLE_OK)
    {
      MIKTEX_FATAL_ERROR_2(GetCurlErrorString(code), "option", std::to_string(option));
    }
  }

private:
  void Initialize();

private:
  static int ProgressCallback(void * pv, double dltotal, double dlnow, double ultotal, double ulnow);

private:
  static int DebugCallback(CURL * pCurl, curl_infotype infoType, char * pData, size_t sizeData, void * pv);

private:
  std::string proxyPort;

private:
  std::string userPassword;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_curl;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
  };

END_INTERNAL_NAMESPACE;

#endif // libCURL
