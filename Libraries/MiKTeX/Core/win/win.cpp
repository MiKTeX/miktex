/* win.cpp: Windows specials

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/Directory.h"
#include "miktex/Core/Exceptions.h"
#include "miktex/Core/win/DllProc.h"
#include "miktex/Core/win/HResult.h"
#include "miktex/Core/win/winAutoResource.h"

#include "Session/SessionImpl.h"
#include "Utils/inliners.h"
#include "core-version.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

MIKTEXINTERNALFUNC(bool) GetWindowsFontsDirectory(PathName & path)
{
  wchar_t szWinDir[BufferSizes::MaxPath];
  if (GetWindowsDirectoryW(szWinDir, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetWindowsDirectoryW");
  }
  path = szWinDir;
  path /= "Fonts";
  return Directory::Exists(path);
}

SharingViolationException::SharingViolationException()
{
}

SharingViolationException::SharingViolationException(const string & programInvocationName, const string & message, const KVMAP & info, const SourceLocation & sourceLocation) :
  IOException(programInvocationName, message, info, sourceLocation)
{
}

MIKTEXINTERNALFUNC(bool) GetUserProfileDirectory(PathName & path)
{
  return Utils::GetEnvironmentString("USERPROFILE", path);
}

extern "C" __declspec(dllexport) HRESULT CALLBACK DllGetVersion(DLLVERSIONINFO * pdvi)
{
  MIKTEX_ASSERT(pdvi != nullptr);
  if (pdvi->cbSize != sizeof(*pdvi))
  {
    return E_INVALIDARG;
  }
  unsigned a[4] = {
    MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0
  };
  pdvi->dwMajorVersion = a[0];
  pdvi->dwMinorVersion = a[1];
  pdvi->dwBuildNumber = a[2];
  pdvi->dwPlatformID = DLLVER_PLATFORM_WINDOWS;
  return S_OK;
}

MIKTEXINTERNALFUNC(bool) GetWindowsErrorMessage(unsigned long functionResult, string & errorMessage)
{
  void * pMessageBuffer;
  unsigned long len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, functionResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<wchar_t *>(&pMessageBuffer), 0, 0);
  if (len == 0)
  {
    TraceError(T_("FormatMessageW() failed for some reason"));
    return false;
  }
  AutoLocalMemory autoFree(pMessageBuffer);
  errorMessage = StringUtil::WideCharToUTF8(reinterpret_cast<wchar_t *>(pMessageBuffer));
  return true;
}

MIKTEXINTERNALFUNC(void) TraceWindowsError(const char * lpszWindowsFunction, unsigned long functionResult, const char * lpszInfo, const char * lpszSourceFile, int sourceLine)
{
  string errorMessage;
  if (!GetWindowsErrorMessage(functionResult, errorMessage))
  {
    return;
  }
  shared_ptr<SessionImpl> pSession = SessionImpl::TryGetSession();
  if (pSession == nullptr || pSession->trace_error == nullptr)
  {
    return;
  }
  pSession->trace_error->WriteLine("core", errorMessage.c_str());
  pSession->trace_error->WriteFormattedLine("core", "Function: %s", lpszWindowsFunction);
  pSession->trace_error->WriteFormattedLine("core", "Result: %u", static_cast<unsigned>(functionResult));
  if (lpszInfo != nullptr)
  {
    pSession->trace_error->WriteFormattedLine("core", "Data: %s", lpszInfo);
  }
  pSession->trace_error->WriteFormattedLine("core", "Source: %s:%d", GetShortSourceFile(lpszSourceFile), sourceLine);
}

MIKTEXSTATICFUNC(unsigned int) GetMediaType(const char * lpszPath)
{
  PathName pathRootName;
  if (IsAlpha(lpszPath[0])
    && PathName::IsVolumeDelimiter(lpszPath[1])
    && PathName::IsDirectoryDelimiter(lpszPath[2]))
  {
    CopyString2(pathRootName.GetData(), pathRootName.GetCapacity(), lpszPath, 3);
  }
  else if (!Utils::GetUncRootFromPath(lpszPath, pathRootName))
  {
    return DRIVE_UNKNOWN;
  }
  return GetDriveTypeW(pathRootName.ToWideCharString().c_str());
}

MIKTEXINTERNALFUNC(bool) FileIsOnROMedia(const char * lpszPath)
{
  return GetMediaType(lpszPath) == DRIVE_CDROM;
}

#define SET_SECURITY 1

#if SET_SECURITY
MIKTEXSTATICFUNC(void) CreateDirectoryForEveryone(const char * lpszPath)
{
  AutoSid pEveryoneSID;
  PACL pACL = nullptr;

  SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
  if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("AllocateAndInitializeSid");
  }

  EXPLICIT_ACCESSW ea[1];
  ZeroMemory(&ea, sizeof(ea));
  ea[0].grfAccessPermissions = FILE_ALL_ACCESS;
  ea[0].grfAccessMode = GRANT_ACCESS;
  ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
  ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
  ea[0].Trustee.ptstrName = reinterpret_cast<wchar_t *>(pEveryoneSID.Get());

  if (SetEntriesInAclW(1, ea, nullptr, &pACL) != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SetEntriesInAclW");
  }

  AutoLocalMemory xxx(reinterpret_cast<void*>(pACL));

  PSECURITY_DESCRIPTOR pSD = reinterpret_cast<PSECURITY_DESCRIPTOR>(_alloca(SECURITY_DESCRIPTOR_MIN_LENGTH));

  if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("InitializeSecurityDescriptor");
  }

  if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SetSecurityDescriptorDacl");
  }

  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = pSD;
  sa.bInheritHandle = FALSE;

  if (!CreateDirectoryW(PathName(lpszPath).ToWideCharString().c_str(), &sa))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateDirectoryW", "path", lpszPath);
  }
}
#endif

MIKTEXINTERNALFUNC(void) CreateDirectoryPath(const char * lpszPath)
{
  MIKTEX_ASSERT_STRING(lpszPath);

  if (!Utils::IsAbsolutePath(lpszPath))
  {
    PathName path(lpszPath);
    path.MakeAbsolute();
    // RECURSION
    CreateDirectoryPath(path.Get());
  }

  // do nothing, if the directory already exists
  if (Directory::Exists(lpszPath))
  {
    return;
  }

  // create the parent directory
  char szDir[BufferSizes::MaxPath];
  char szFname[BufferSizes::MaxPath];
  char szExt[BufferSizes::MaxPath];
  PathName::Split(lpszPath, szDir, BufferSizes::MaxPath, szFname, BufferSizes::MaxPath, szExt, BufferSizes::MaxPath);
  PathName pathParent(szDir, nullptr, nullptr);
  RemoveDirectoryDelimiter(pathParent.GetData());
  // RECURSION
  CreateDirectoryPath(pathParent.Get());

  // we're done, if szFname is empty (this happens when lpszPath ends
  // with a directory delimiter)
  if (szFname[0] == 0 && szExt[0] == 0)
  {
    return;
  }

  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();

  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("creating directory %s"), Q_(lpszPath));
  }

#if SET_SECURITY
  // create the directory itself
  if (session != nullptr && session->IsAdminMode()
    && (session->GetSpecialPath(SpecialPath::CommonConfigRoot) == lpszPath || session->GetSpecialPath(SpecialPath::CommonDataRoot) == lpszPath)
    && session->RunningAsAdministrator())
  {
    CreateDirectoryForEveryone(lpszPath);
  }
  else if (!CreateDirectoryW(PathName(lpszPath).ToWideCharString().c_str(), nullptr))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateDirectoryW", "path", lpszPath);
  }
#else
  if (!CreateDirectoryW(PathName(lpszPath).ToWideCharString().c_str(), nullptr))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateDirectoryW", "path", lpszPath);
  }
#endif
}

HResult::~HResult()
{
  try
  {
    if (lpszMessage != nullptr)
    {
      LocalFree(reinterpret_cast<HLOCAL>(lpszMessage));
      lpszMessage = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

string HResult::ToString() const
{
  string ret;
  ret += std::to_string(GetSeverity());
  ret += ':';
  ret += std::to_string(GetFacility());
  ret += ':';
  ret += std::to_string(GetCode());
  return ret;
}

const char * HResult::GetText()
{
  if (lpszMessage == nullptr)
  {
    // FIXME: use Unicode version
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&lpszMessage), 0, nullptr);
    if (lpszMessage == nullptr)
    {
      string str = ToString();
      size_t sizeChars = str.length() + 1;
      lpszMessage = reinterpret_cast<char*>(LocalAlloc(0, sizeChars * sizeof(lpszMessage[0])));
      if (lpszMessage != nullptr)
      {
	StringUtil::CopyString(lpszMessage, sizeChars, str.c_str());
      }
    }
  }
  return lpszMessage;
}
