/* CurlWebSession.h:                                    -*- C++ -*-

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

#if !defined(A9010DA64FD34372A24071A29FA59EEC)
#define A9010DA64FD34372A24071A29FA59EEC

#if defined(HAVE_LIBCURL)

#include <memory>
#include <string>
#include <unordered_map>

#include <curl/curl.h>

#include <miktex/Core/Session>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include "WebSession.h"
#include "text.h"

MPM_INTERNAL_BEGIN_NAMESPACE;

class CurlWebSession :
  public std::enable_shared_from_this<CurlWebSession>,
  public WebSession
{
public:
  CurlWebSession(IProgressNotify_* callback);

public:
  ~CurlWebSession() override;

public:
  std::unique_ptr<WebFile> OpenUrl(const std::string& url) override
  {
    return OpenUrl(url, {});
  }

public:
  std::unique_ptr<WebFile> OpenUrl(const std::string& url, const std::unordered_map<std::string, std::string>& formData) override;

public:
  void Dispose() override;

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
    if (curlVersionInfo->version_num >= 0x70c00)
    {
      return curl_multi_strerror(code);
    }
#endif
    std::string str = T_("The cURL multi interface returned an error code: ");
    str += std::to_string(code);
    return str;
  }

public:
  void FatalCurlError(CURLMcode code) const
  {
    MIKTEX_FATAL_ERROR(GetCurlErrorString(code));
  }

public:
  void ExpectOK(CURLMcode code) const
  {
    if (code != CURLM_OK)
    {
      FatalCurlError(code);
    }
  }

private:
  CURL* pCurl = nullptr;

private:
  CURLM* pCurlm = nullptr;

private:
  int runningHandles = -1;

public:
  bool IsReady() const
  {
    return runningHandles == 0;
  }

public:
  CURL* GetEasyHandle() const throw ()
  {
    return pCurl;
  }

public:
  CURLM* GetMultiHandle() const throw ()
  {
    return pCurlm;
  }

public:
  std::string GetCurlErrorString(CURLcode code) const
  {
#if LIBCURL_VERSION_NUM >= 0x70c00
    if (curlVersionInfo->version_num >= 0x70c00)
    {
      return curl_easy_strerror(code);
    }
    else
#endif
    {
      std::string str = T_("The cURL easy interface returned an error code: ");
      str += std::to_string(code);
      return str;
    }
  }

public:
  void FatalCurlError(CURLcode code, const char* effectiveUrl) const;

public:
  void ExpectOK(CURLcode code, const char* effectiveUrl) const
  {
    if (code != CURLE_OK)
    {
      FatalCurlError(code, effectiveUrl);
    }
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

public:
  std::string UrlEncode(const std::string& s) const
  {
    char* encoded = curl_easy_escape(pCurl, s.c_str(), static_cast<int>(s.length()));
    std::string result = encoded;
    curl_free(encoded);
    return result;
  }

public:
  void SetCustomHeaders(const std::unordered_map<std::string, std::string>& headers) override;

private:
  void Initialize();

private:
  static int ProgressCallback(void* pv, double dltotal, double dlnow, double ultotal, double ulnow);

private:
  static int DebugCallback(CURL* pCurl, curl_infotype infoType, char* pData, std::size_t sizeData, void* pv);

private:
  std::string proxyPort;

private:
  std::string userPassword;

private:
  struct curl_slist* headers = nullptr;

private:
  curl_version_info_data* curlVersionInfo = nullptr;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_curl;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();
};

MPM_INTERNAL_END_NAMESPACE;

#endif

#endif
