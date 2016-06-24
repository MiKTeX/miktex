/* winWebFile.cpp:

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

#include "internal.h"

#include "win/winWebFile.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace std;

BEGIN_INTERNAL_NAMESPACE;

class InternetCloseHandle_
{
public:
  void operator () (HINTERNET h)
  {
    InternetCloseHandle(h);
  }
};

typedef AutoResource<HINTERNET, InternetCloseHandle_> AutoInternetHandle;

winWebFile::winWebFile(winWebSession * pSession, const char * lpszUrl)
  : url(lpszUrl), trace_error(TraceStream::Open("error")), trace_mpm(TraceStream::Open("mpm"))
{
  // open the rempote file
  trace_mpm->WriteFormattedLine("libmpm", T_("opening %s"), lpszUrl);
  unsigned int flags = (0
    | INTERNET_FLAG_EXISTING_CONNECT
    | INTERNET_FLAG_KEEP_CONNECTION
    | INTERNET_FLAG_NO_CACHE_WRITE
    | INTERNET_FLAG_PASSIVE
    | INTERNET_FLAG_PRAGMA_NOCACHE
    | INTERNET_FLAG_RELOAD
    | INTERNET_FLAG_TRANSFER_BINARY
    | 0);
  HINTERNET hInternet = pSession->GetHandle();
  hUrl = InternetOpenUrlA(hInternet, lpszUrl, nullptr, 0, flags, 0);
  if (hUrl == nullptr)
  {
    if (winWebSession::IsGlobalOffline())
    {
      trace_mpm->WriteLine("libmpm", T_("we are offline"));
      if (InternetGoOnlineA(const_cast<char *>(lpszUrl), GetDesktopWindow(), 0))
      {
	trace_mpm->WriteLine("libmpm", T_("gone online"));
	hUrl = InternetOpenUrlA(hInternet, lpszUrl, nullptr, 0, flags, 0);
      }
      else
      {
	trace_mpm->WriteLine("libmpm", T_("still offline"));
	MIKTEX_FATAL_ERROR_2(T_("Packages cannot be downloaded when Internet Explorer is in offline mode."), "url", lpszUrl);
      }
    }
    else
    {
      trace_mpm->WriteLine("libmpm", T_("we are online"));
    }
    if (hUrl == nullptr)
    {
      string error;
      winWebSession::GetLastErrorMessage(error);
      MIKTEX_FATAL_ERROR_2(error, "url", lpszUrl);
    }
  }

  AutoInternetHandle autoCloseUrl(hUrl);

  // get the service type
  unsigned long type;
  unsigned long size = sizeof(type);
  if (!InternetQueryOption(hUrl, INTERNET_OPTION_HANDLE_TYPE, &type, &size))
  {
    string error;
    winWebSession::GetLastErrorMessage(error);
    MIKTEX_FATAL_ERROR_2(error, "url", lpszUrl);
  }

  // we are ready, unless it is an HTTP connection
  if (!(type == INTERNET_HANDLE_TYPE_HTTP_REQUEST || type == INTERNET_HANDLE_TYPE_CONNECT_HTTP))
  {
    autoCloseUrl.Detach();
    return;
  }

  bool retry = true;
  do
  {
    // get the status code of the last request
    unsigned long code;
    size = sizeof(code);
    if (!HttpQueryInfoA(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &code, &size, 0))
    {
      string error;
      winWebSession::GetLastErrorMessage(error);
      MIKTEX_FATAL_ERROR_2(error, "url", lpszUrl);
    }

    // we are ready, if the status is OK
    if (code == HTTP_STATUS_OK)
    {
      break;
    }

    // we only handle "proxy authentication required" errors
    if (code != HTTP_STATUS_PROXY_AUTH_REQ)
    {
      trace_error->WriteFormattedLine("libmpm", "HTTP %u", static_cast<unsigned>(code));
      // return a meaningful error message
      char szText[1024];
      unsigned long size = sizeof(szText);
      string error;
      if (HttpQueryInfoA(hUrl, HTTP_QUERY_STATUS_TEXT, szText, &size, 0))
      {
	error = szText;
      }
      else
      {
	winWebSession::GetLastErrorMessage(error);
      }
      MIKTEX_FATAL_ERROR_2(error, "url", lpszUrl);
    }

    trace_mpm->WriteLine("libmpm", T_("proxy authentication required"));

    // read and ignore the HTML page that follows
    unsigned long length;
    if (!InternetQueryDataAvailable(hUrl, &length, 0, 0))
    {
      string error;
      winWebSession::GetLastErrorMessage(error);
      MIKTEX_FATAL_ERROR_2(error, "url", lpszUrl);
    }
    void * pv = _alloca(length);
    do
    {
      InternetReadFile(hUrl, pv, length, &size);
    } while (size != 0);

    if (InternetErrorDlg(GetDesktopWindow(), hUrl, ERROR_INTERNET_INCORRECT_PASSWORD,
      (0
	| FLAGS_ERROR_UI_FILTER_FOR_ERRORS
	| FLAGS_ERROR_UI_FLAGS_GENERATE_DATA
	| FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS
	|
	0),
      0)
      != ERROR_INTERNET_FORCE_RETRY)
    {
      trace_mpm->WriteLine("libmpm", T_("internet error dialog was cancelled"));
      MIKTEX_FATAL_ERROR_2(T_("Proxy authentication cancelled."), "url", lpszUrl);
    }

    // retry the request
    if (!HttpSendRequestA(hUrl, nullptr, 0, nullptr, 0))
    {
      string error;
      winWebSession::GetLastErrorMessage(error);
      MIKTEX_FATAL_ERROR_2(error, "url", lpszUrl);
    }
  } while (retry);

  autoCloseUrl.Detach();
}

winWebFile::~winWebFile()
{
  try
  {
    Close();
  }
  catch (const exception &)
  {
  }
}

size_t winWebFile::Read(void * pBuffer, size_t n)
{
  unsigned long nRead;
  if (!InternetReadFile(hUrl, pBuffer, static_cast<unsigned long>(n), &nRead))
  {
    string error;
    winWebSession::GetLastErrorMessage(error);
    MIKTEX_FATAL_ERROR_2(error, "url", url);
  }
  return nRead;
}

void winWebFile::Close()
{
  if (this->hUrl != nullptr)
  {
    HINTERNET hUrl = this->hUrl;
    this->hUrl = nullptr;
    if (!InternetCloseHandle(hUrl))
    {
      string error;
      winWebSession::GetLastErrorMessage(error);
      MIKTEX_FATAL_ERROR(error);
    }
  }
  if (trace_mpm.get() != nullptr)
  {
    trace_mpm->Close();
    trace_mpm.reset();
  }
  if (trace_error.get() != nullptr)
  {
    trace_error->Close();
    trace_error.reset();
  }
}

END_INTERNAL_NAMESPACE;
