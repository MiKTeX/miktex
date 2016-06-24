/* winWebSession.cpp:

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
#include "win/winWebSession.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace std;

BEGIN_INTERNAL_NAMESPACE;

winWebSession::winWebSession()
{
}

winWebSession::~winWebSession()
{
  try
  {
    Dispose();
  }
  catch (const exception &)
  {
  }
}

WebFile * winWebSession::OpenUrl(const char * lpszUrl)
{
  if (hInternet == nullptr)
  {
    // check to see if computer is connected to the Internet
    if (InternetAttemptConnect(0) != ERROR_SUCCESS)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("InternetAttemptConnect");
    }

#if !defined(UW_)
#  define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif
    hInternet = InternetOpenW(UW_(MPM_AGENT), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, INTERNET_FLAG_KEEP_CONNECTION);
    if (hInternet == nullptr)
    {
      string error;
      GetLastErrorMessage(error);
      MIKTEX_FATAL_ERROR_2(error);
    }
  }
  return new winWebFile(this, lpszUrl);
}

void winWebSession::Dispose()
{
  if (this->hInternet != nullptr)
  {
    HINTERNET hInternet = this->hInternet;
    this->hInternet = nullptr;
    if (!InternetCloseHandle(hInternet))
    {
      string error;
      GetLastErrorMessage(error);
      MIKTEX_FATAL_ERROR_2(error);
    }
  }
}

bool winWebSession::IsGlobalOffline()
{
  unsigned long state = 0;
  unsigned long size = sizeof(unsigned int);
  return InternetQueryOptionA(nullptr, INTERNET_OPTION_CONNECTED_STATE, &state, &size) && (state & INTERNET_STATE_DISCONNECTED_BY_USER) != FALSE;
}

void winWebSession::GetLastErrorMessage(string & message)
{
  unsigned int lastError = ::GetLastError();
  void * pMsgBuf = nullptr;
  if (lastError >= INTERNET_ERROR_BASE && lastError <= INTERNET_ERROR_LAST)
  {
    if (lastError == ERROR_INTERNET_EXTENDED_ERROR)
    {
      unsigned long inetError;
      unsigned long length;
      if (!InternetGetLastResponseInfoA(&inetError, nullptr, &length) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
	length += 1;
	pMsgBuf = LocalAlloc(LMEM_FIXED, length);
	if (pMsgBuf != nullptr)
	{
	  if (!InternetGetLastResponseInfoA(&inetError, reinterpret_cast<char*>(pMsgBuf), &length))
	  {
	    LocalFree(pMsgBuf);
	    pMsgBuf = nullptr;
	  }
	}
      }
    }
    else
    {
      FormatMessageA((FORMAT_MESSAGE_ALLOCATE_BUFFER
	| FORMAT_MESSAGE_IGNORE_INSERTS
	| FORMAT_MESSAGE_FROM_HMODULE), GetModuleHandleA("wininet.dll"), lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&pMsgBuf), 0, nullptr);
    }
  }
  else
  {
    FormatMessageA((FORMAT_MESSAGE_ALLOCATE_BUFFER
      | FORMAT_MESSAGE_IGNORE_INSERTS
      | FORMAT_MESSAGE_FROM_SYSTEM), nullptr, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&pMsgBuf), 0, nullptr);
  }
  if (pMsgBuf == nullptr)
  {
    message = T_("Unexpected error condition.");
  }
  else
  {
    message = reinterpret_cast<const char*>(pMsgBuf);
    LocalFree(pMsgBuf);
  }
}

END_INTERNAL_NAMESPACE;
