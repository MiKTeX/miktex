/* winUnused.cpp:

   Copyright (C) 1996-2016 Christian Schenk

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

#error not to be used

#if REPORT_EVENTS

#define SOURCE "MiKTeX"

#define EVTLOGAPP \
  "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application"

#if defined(MIKTEX_CORE_SHARED)
MIKTEXSTATICFUNC(bool) AddEventSource()
{
  string registryPath = EVTLOGAPP;
  registryPath += '\\';
  registryPath += SOURCE;

  AutoHKEY hkey;
  unsigned long disp;
  long res =
    RegCreateKeyEx(HKEY_LOCAL_MACHINE, registryPath.c_str(), 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hkey, &disp);
  if (res != ERROR_SUCCESS)
  {
    TraceWindowsError("", res, registryPath.c_str(), __FILE__, __LINE__);
    return false;
  }

  // set the name of the message file
  char szModule[BufferSizes::MaxPath];
  if (GetModuleFileName(SessionImpl::hinstDLL, szModule, BufferSizes::MaxPath)
    == 0)
  {
    TraceStream::TraceLastWin32Error("GetModuleFileName", 0, __FILE__, __LINE__);
    return false;
  }
  res =
    RegSetValueEx(hkey.Get(), "EventMessageFile", 0, REG_EXPAND_SZ, reinterpret_cast<unsigned char *>(&szModule), static_cast<unsigned long>((StrLen(szModule) + 1)
      * sizeof(char)));
  if (res != ERROR_SUCCESS)
  {
    TraceWindowsError("RegSetValueEx", res, "EventMessageFile", __FILE__, __LINE__);
    return false;
  }

  // set the supported event types in the TypesSupported subkey
  unsigned long dwData = (EVENTLOG_ERROR_TYPE
    | EVENTLOG_WARNING_TYPE
    | EVENTLOG_INFORMATION_TYPE);
  res =
    RegSetValueEx(hkey.Get(), "TypesSupported", 0, REG_DWORD, reinterpret_cast<unsigned char *>(&dwData), sizeof(unsigned long));

  if (res != ERROR_SUCCESS)
  {
    TraceWindowsError("RegSetValueEx", res, "TypesSupported", __FILE__, __LINE__);
    return false;
  }

  return true;
}
#endif

#endif

#if REPORT_EVENTS

#if defined(MIKTEX_CORE_SHARED)
MIKTEXSTATICFUNC(bool) RemoveEventSource()
{
  AutoHKEY hkey;
  LONG res =
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, EVTLOGAPP, 0, KEY_SET_VALUE, &hkey);
  if (res != ERROR_SUCCESS)
  {
    TraceWindowsError("RemoveEventSource", res, "EventLog\\Application", __FILE__, __LINE__);
    return false;
  }
  res = RegDeleteKey(hkey.Get(), SOURCE);
  if (res != ERROR_SUCCESS)
  {
    TraceWindowsError("RemoveEventSource", res, "EventLog\\Application\\MiKTeX", __FILE__, __LINE__);
    return false;
  }
  return true;
}
#endif

#endif

#if REPORT_EVENTS
MIKTEXINTERNALFUNC(bool) ReportMiKTeXEvent(unsigned short eventType, unsigned long eventId, ...)
{
#if ! REPORT_EVENTS
  UNUSED_ALWAYS(eventType);
  UNUSED_ALWAYS(eventId);
  return false;
#else
  vector<const char *> vecStrings;
  va_list marker;
  va_start(marker, eventId);
  for (const char * lpsz = va_arg(marker, const char *);
  lpsz != nullptr;
    lpsz = va_arg(marker, const char *))
  {
    vecStrings.push_back(lpsz);
  }
  va_end(marker);

  HANDLE hEventSource = RegisterEventSource(0, SOURCE);
  if (hEventSource == nullptr)
  {
    return false;
  }

  PSID pSid = 0;
  size_t bufSize = 8192;
  AutoMemoryPointer pBuf(malloc(bufSize));
  if (pBuf.Get() == nullptr)
  {
    OUT_OF_MEMORY("malloc");
  }
  char szAccountName[BufferSizes::MaxPath];
  unsigned long n = BufferSizes::MaxPath;
  if (GetUserName(szAccountName, &n))
  {
    unsigned long sidSize = static_cast<unsigned long>(bufSize);
    pSid = reinterpret_cast<PSID>(pBuf.Get());
    char szDomainName[BufferSizes::MaxPath];
    unsigned long domainNameSize = BufferSizes::MaxPath;
    SID_NAME_USE use;
    if (!LookupAccountName(0, szAccountName, pSid, &sidSize, szDomainName, &domainNameSize, &use))
    {
      pSid = 0;
    }
  }
  BOOL done =
    ReportEvent(hEventSource, eventType, 0, eventId, pSid, static_cast<unsigned short>(vecStrings.size()), 0, &vecStrings[0], 0);
  DeregisterEventSource(hEventSource);

  return done ? true : false;
#endif
}
#endif
