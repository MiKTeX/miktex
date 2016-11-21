/* CurlWebSession.cpp:

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

#include "CurlWebSession.h"
#include "CurlWebFile.h"

using namespace MiKTeX::Trace;
using namespace std;

const long DEFAULT_CONNECTION_TIMEOUT_SECONDS = 30;
const long DEFAULT_FTP_RESPONSE_TIMEOUT_SECONDS = 30;

#define ALLOW_REDIRECTS 1
#define DEFAULT_MAX_REDIRECTS 20

CurlWebSession::CurlWebSession(IProgressNotify_ * pIProgressNotify) :
  pIProgressNotify(pIProgressNotify),
  trace_mpm(TraceStream::Open(MIKTEX_TRACE_MPM)),
  trace_curl(TraceStream::Open(MIKTEX_TRACE_CURL))
{
}

void CurlWebSession::Initialize()
{
  trace_curl->WriteFormattedLine("libmpm", T_("initializing cURL library version %s"), LIBCURL_VERSION);

  pCurlm = curl_multi_init();

  if (pCurlm == nullptr)
  {
    MIKTEX_FATAL_ERROR(T_("The cURL multi interface could not be initialized."));
  }

  pCurl = curl_easy_init();

  if (pCurl == nullptr)
  {
    MIKTEX_FATAL_ERROR(T_("The cURL easy interface could not be initialized."));
  }

  SetOption(CURLOPT_USERAGENT, MPM_AGENT);

  string ftpMode = session->GetConfigValue(nullptr, MIKTEX_REGVAL_FTP_MODE, "default");

  if (ftpMode == "default")
  {
  }
  else if (ftpMode == "port")
  {
    SetOption(CURLOPT_FTPPORT, "-");
  }
  else if (ftpMode == "pasv")
  {
    SetOption(CURLOPT_FTP_USE_EPSV, static_cast<long>(false));
  }
  else if (ftpMode == "epsv")
  {
    SetOption(CURLOPT_FTP_USE_EPSV, static_cast<long>(true));
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }

  SetOption(CURLOPT_PROGRESSDATA, reinterpret_cast<void*>(this));
  curl_progress_callback progressCallback = ProgressCallback;
  SetOption(CURLOPT_PROGRESSFUNCTION, progressCallback);

  if (trace_curl->IsEnabled())
  {
    SetOption(CURLOPT_VERBOSE, static_cast<long>(true));
    curl_debug_callback debugCallback = DebugCallback;
    SetOption(CURLOPT_DEBUGFUNCTION, debugCallback);
    SetOption(CURLOPT_DEBUGDATA, reinterpret_cast<void*>(this));
  }
  else
  {
    SetOption(CURLOPT_VERBOSE, static_cast<long>(false));
  }

  SetOption(CURLOPT_CONNECTTIMEOUT, DEFAULT_CONNECTION_TIMEOUT_SECONDS);

#if LIBCURL_VERSION_NUM >= 0x70a08
  SetOption(CURLOPT_FTP_RESPONSE_TIMEOUT, DEFAULT_FTP_RESPONSE_TIMEOUT_SECONDS);
#endif

  // SF 2855025
#if ALLOW_REDIRECTS
  int maxRedirects = session->GetConfigValue(nullptr, MIKTEX_REGVAL_MAX_REDIRECTS, DEFAULT_MAX_REDIRECTS);
  SetOption(CURLOPT_FOLLOWLOCATION, static_cast<long>(true));
  SetOption(CURLOPT_MAXREDIRS, static_cast<long>(maxRedirects));
#endif

  // SF #2548
#if LIBCURL_VERSION_NUM >= 0x72c00
  SetOption(CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
#endif

  SetOption(CURLOPT_NOSIGNAL, static_cast<long>(true));

  ProxySettings proxySettings;

  bool haveProxySettings = PackageManager::TryGetProxy(proxySettings);

  if (haveProxySettings && proxySettings.useProxy)
  {
    proxyPort = proxySettings.proxy;
    proxyPort += ":";
    proxyPort += std::to_string(proxySettings.port);
    SetOption(CURLOPT_PROXY, proxyPort.c_str());
    if (proxySettings.authenticationRequired)
    {
      if (proxySettings.user.find(':') != string::npos)
      {
        MIKTEX_UNEXPECTED();
      }
      if (proxySettings.password.find(':') != string::npos)
      {
        MIKTEX_UNEXPECTED();
      }
      userPassword = proxySettings.user;
      userPassword += ':';
      userPassword += proxySettings.password;
      SetOption(CURLOPT_PROXYUSERPWD, userPassword.c_str());
    }
  }
}

CurlWebSession::~CurlWebSession()
{
  try
  {
    Dispose();
  }
  catch (const exception &)
  {
  }
}

unique_ptr<WebFile> CurlWebSession::OpenUrl(const string & url, const std::unordered_map<std::string, std::string> & formData)
{
  runningHandles = -1;
  if (pCurl == nullptr)
  {
    Initialize();
  }
  trace_mpm->WriteFormattedLine("libmpm", T_("going to download %s"), Q_(url));
  return make_unique<CurlWebFile>(shared_from_this(), url, formData);
}

void CurlWebSession::SetCustomHeaders(const unordered_map<string, string> & headers)
{
  if (this->headers != nullptr)
  {
    curl_slist_free_all(this->headers);
    this->headers = nullptr;
  }
  for (const auto & kv : headers)
  {
    string header = kv.first + ": " + kv.second;
    this->headers = curl_slist_append(this->headers, header.c_str());
  }
  if (pCurl == nullptr)
  {
    Initialize();
  }
  SetOption(CURLOPT_HTTPHEADER, this->headers);
}

void CurlWebSession::Dispose()
{
  if (headers != nullptr)
  {
    curl_slist_free_all(headers);
    headers = nullptr;
  }
  if (pCurl != nullptr)
  {
    trace_curl->WriteLine("libmpm", T_("releasing cURL easy handle"));
    curl_easy_cleanup(pCurl);
    pCurl = nullptr;
  }
  if (pCurlm != nullptr)
  {
    trace_curl->WriteLine("libmpm", T_("releasing cURL multi handle"));
    CURLMcode code = curl_multi_cleanup(pCurlm);
    pCurlm = nullptr;
    if (code != CURLM_OK)
    {
      MIKTEX_FATAL_ERROR(GetCurlErrorString(code));
    }
  }
  runningHandles = -1;
}

void CurlWebSession::Connect()
{
  CURLMcode code;
  do
  {
    code = curl_multi_perform(pCurlm, &runningHandles);
    if (code != CURLM_OK && code != CURLM_CALL_MULTI_PERFORM)
    {
      MIKTEX_FATAL_ERROR(GetCurlErrorString(code));
    }
  } while (code == CURLM_CALL_MULTI_PERFORM);
  if (runningHandles == 0)
  {
    ReadInformationals();
  }
}

void CurlWebSession::Perform()
{
  Connect();

  if (runningHandles == 0)
  {
    return;
  }

  int oldRunningHandles = runningHandles;

  if (runningHandles > 0)
  {
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    int maxfd;

    CURLMcode code = curl_multi_fdset(pCurlm, &fdread, &fdwrite, &fdexcep, &maxfd);

    if (code != CURLM_OK)
    {
      MIKTEX_FATAL_ERROR(GetCurlErrorString(code));
    }

    long timeout;

#if LIBCURL_VERSION_NUM >= 0x70f04 && 0
    code = curl_multi_timeout(pCurlm, &timeout);

    if (code != CURLM_OK)
    {
      MIKTEX_FATAL_ERROR(GetCurlErrorString(code));
    }
#else
    timeout = 100;
#endif

    if (timeout < 0)
    {
      timeout = 100;
    }

    if (maxfd < 0)
    {
      this_thread::sleep_for(chrono::milliseconds(timeout));
    }
    else
    {
      struct timeval tv;
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;

      int n = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &tv);

      if (n < 0)
      {
        MIKTEX_FATAL_ERROR_2(T_("select() did not succeed."), "result", std::to_string(n));
      }

      if (n > 0)
      {
        do
        {
          code = curl_multi_perform(pCurlm, &runningHandles);
          if (code != CURLM_OK && code != CURLM_CALL_MULTI_PERFORM)
          {
            MIKTEX_FATAL_ERROR(GetCurlErrorString(code));
          }
        } while (code == CURLM_CALL_MULTI_PERFORM);
      }
    }
  }

  if (oldRunningHandles >= 0 && runningHandles != oldRunningHandles)
  {
    ReadInformationals();
  }
}

void CurlWebSession::ReadInformationals()
{
  CURLMsg * pCurlMsg;
  int remaining;
  while ((pCurlMsg = curl_multi_info_read(pCurlm, &remaining)) != nullptr)
  {
    if (pCurlMsg->msg != CURLMSG_DONE)
    {
      MIKTEX_FATAL_ERROR_2(T_("Unexpected cURL message."), "msg", std::to_string(pCurlMsg->msg));
    }
    if (pCurlMsg->data.result != CURLE_OK)
    {
      MIKTEX_FATAL_ERROR(GetCurlErrorString(pCurlMsg->data.result));
    }
    long responseCode;
    CURLcode r;
#if LIBCURL_VERSION_NUM >= 0x70a08
    r = curl_easy_getinfo(pCurlMsg->easy_handle, CURLINFO_RESPONSE_CODE, &responseCode);
#else
    r = curl_easy_getinfo(pCurlMsg->easy_handle, CURLINFO_HTTP_CODE, &responseCode);
#endif
    if (r != CURLE_OK)
    {
      MIKTEX_FATAL_ERROR(GetCurlErrorString(r));
    }
    trace_mpm->WriteFormattedLine("libmpm", T_("response code: %ld"), responseCode);
    char * lpszEffectiveUrl = nullptr;
    r = curl_easy_getinfo(pCurlMsg->easy_handle, CURLINFO_EFFECTIVE_URL, &lpszEffectiveUrl);
    if (r != CURLE_OK)
    {
      MIKTEX_FATAL_ERROR(GetCurlErrorString(r));
    }
    if (lpszEffectiveUrl != nullptr)
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("effective URL: %s"), lpszEffectiveUrl);
    }
    if (responseCode >= 300 && responseCode <= 399)
    {
#if ALLOW_REDIRECTS
      MIKTEX_UNEXPECTED();
#else
      string msg = T_("The server returned status code ");
      msg += std::to_string(responseCode);
      msg += T_(", but redirection is not supported. You must choose ");
      msg += T_("another package repository.");
      MIKTEX_FATAL_ERROR(msg);
#endif
    }
    else if (responseCode >= 400)
    {
      string msg = T_("Error response from server: ");
      msg += std::to_string(responseCode);
      MIKTEX_FATAL_ERROR(msg);
    }
  }
}

int CurlWebSession::ProgressCallback(void * pv, double dltotal, double dlnow, double ultotal, double ulnow)
{
  UNUSED_ALWAYS(dltotal);
  UNUSED_ALWAYS(dlnow);
  UNUSED_ALWAYS(ultotal);
  UNUSED_ALWAYS(ulnow);
#if 1
  UNUSED_ALWAYS(pv);
  return 0;
#else
  try
  {
    CurlWebFile * This = reinterpret_cast<CurlWebSession*>(pv);
    if (This->pIProgressNotify != nullptr)
    {
      This->pIProgressNotify->OnProgress();
    }
    return 0;
  }
  catch (const exception &)
  {
    return -1;
  }
#endif
    }

int CurlWebSession::DebugCallback(CURL * pCurl, curl_infotype infoType, char * pData, size_t sizeData, void * pv)
{
  UNUSED_ALWAYS(pCurl);
  try
  {
    CurlWebSession * This = reinterpret_cast<CurlWebSession*>(pv);
    if (infoType == CURLINFO_TEXT)
    {
      MIKTEX_ASSERT(pData != nullptr);
      string text(pData, sizeData);
      This->trace_curl->Write("libmpm", text.c_str());
    }
  }
  catch (const exception &)
  {
  }
  return 0;
}

#endif // libCURL
