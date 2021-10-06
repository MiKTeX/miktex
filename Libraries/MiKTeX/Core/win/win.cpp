/* win.cpp: Windows specials

   Copyright (C) 1996-2021 Christian Schenk

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

#include "config.h"

#include <Windows.h>

#include <AclAPI.h>
#include <ShlObj.h>
#include <Shlwapi.h>

#include <iomanip>

#include <fmt/format.h>
#include <fmt/ostream.h>

// FIXME: must come first
#include "core-version.h"

#include <miktex/Core/BufferSizes>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Session>
#include <miktex/Core/Utils>
#include <miktex/Core/win/HResult>
#include <miktex/Core/win/winAutoResource>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include "internal.h"

#include "Utils/inliners.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

MIKTEXINTERNALFUNC(bool) GetSystemFontDirectory(PathName& path)
{
  wchar_t szPath[BufferSizes::MaxPath];
  if (SHGetFolderPathW(nullptr, CSIDL_FONTS, nullptr, SHGFP_TYPE_CURRENT, szPath) != S_OK)
  {
    return false;
  }
  path = szPath;
  return Directory::Exists(path);
}

MIKTEXINTERNALFUNC(bool) GetUserFontDirectory(PathName& path)
{
  wchar_t szPath[BufferSizes::MaxPath];
  if (SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) != S_OK)
  {
    return false;
  }
  path = szPath;
  path /= "Microsoft";
  path /= "Windows";
  path /= "Fonts";
  return Directory::Exists(path);
}

SharingViolationException::SharingViolationException()
{
}

SharingViolationException::SharingViolationException(const string& programInvocationName, const string& message, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation) :
  IOException(programInvocationName, message, description, remedy, tag, info, sourceLocation)
{
}

MIKTEXINTERNALFUNC(bool) GetUserProfileDirectory(PathName& path)
{
  return Utils::GetEnvironmentString("USERPROFILE", path);
}

extern "C" __declspec(dllexport) HRESULT CALLBACK DllGetVersion(DLLVERSIONINFO* versionInfo)
{
  MIKTEX_ASSERT(versionInfo != nullptr);
  if (versionInfo->cbSize != sizeof(*versionInfo))
  {
    return E_INVALIDARG;
  }
  unsigned a[4] = {
    MIKTEX_COMP_MAJOR_VERSION, MIKTEX_COMP_MINOR_VERSION, MIKTEX_COMP_PATCH_VERSION, 0
  };
  versionInfo->dwMajorVersion = a[0];
  versionInfo->dwMinorVersion = a[1];
  versionInfo->dwBuildNumber = a[2];
  versionInfo->dwPlatformID = DLLVER_PLATFORM_WINDOWS;
  return S_OK;
}

MIKTEXINTERNALFUNC(bool) GetWindowsErrorMessage(unsigned long functionResult, string& errorMessage)
{
  void* messageBuffer;
  unsigned long len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, functionResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<wchar_t *>(&messageBuffer), 0, 0);
  if (len == 0)
  {
    auto trace_error = TraceStream::Open(MIKTEX_TRACE_ERROR);
    trace_error->WriteLine("core", TraceLevel::Error, T_("FormatMessageW() failed for some reason"));
    return false;
  }
  AutoLocalMemory autoFree(messageBuffer);
  errorMessage = StringUtil::WideCharToUTF8(reinterpret_cast<wchar_t *>(messageBuffer));
  return true;
}

MIKTEXINTERNALFUNC(void) TraceWindowsError(const char* windowsFunction, unsigned long functionResult, const char* info, const char* sourceFile, int sourceLine)
{
  string errorMessage;
  if (!GetWindowsErrorMessage(functionResult, errorMessage))
  {
    return;
  }
  auto trace_error = TraceStream::Open(MIKTEX_TRACE_ERROR);
  trace_error->WriteLine("core", TraceLevel::Error, errorMessage);
  trace_error->WriteLine("core", TraceLevel::Error, fmt::format("Function: {0}", windowsFunction));
  trace_error->WriteLine("core", TraceLevel::Error, fmt::format("Result: {0}", functionResult));
  if (info != nullptr)
  {
    trace_error->WriteLine("core", TraceLevel::Error, fmt::format("Data: {0}", info));
  }
  trace_error->WriteLine("core", TraceLevel::Error, fmt::format("Source: {0}:{1}", GetShortSourceFile(sourceFile), sourceLine));
}

MIKTEXSTATICFUNC(unsigned int) GetMediaType(const char* path)
{
  PathName pathRootName;
  if (PathNameUtil::IsDosDriveLetter(path[0])
    && PathNameUtil::IsDosVolumeDelimiter(path[1])
    && PathNameUtil::IsDirectoryDelimiter(path[2]))
  {
    pathRootName += path[0];
    pathRootName += PathNameUtil::DosVolumeDelimiter;
    pathRootName += PathNameUtil::DirectoryDelimiter;
  }
  else if (!Utils::GetUncRootFromPath(PathName(path), pathRootName))
  {
    return DRIVE_UNKNOWN;
  }
  return GetDriveTypeW(pathRootName.ToExtendedLengthPathName().ToWideCharString().c_str());
}

MIKTEXINTERNALFUNC(bool) FileIsOnROMedia(const char* path)
{
  return GetMediaType(path) == DRIVE_CDROM;
}

#define SET_SECURITY 1

#if SET_SECURITY
MIKTEXSTATICFUNC(void) CreateDirectoryForEveryone(const char* path)
{
  AutoSid pEveryoneSID;
  PACL pACL = nullptr;

  SID_IDENTIFIER_AUTHORITY SIDAuthWorld = { SECURITY_WORLD_SID_AUTHORITY };
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

  if (!CreateDirectoryW(PathName(path).ToExtendedLengthPathName().ToWideCharString().c_str(), &sa))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateDirectoryW", "path", path);
  }
}
#endif

MIKTEXINTERNALFUNC(void) CreateDirectoryPath(const PathName& path)
{
  if (!path.IsFullyQualified())
  {
    PathName fqPath(path);
    fqPath.MakeFullyQualified();
    // RECURSION
    CreateDirectoryPath(fqPath);
  }

  // do nothing, if the directory already exists
  if (Directory::Exists(path))
  {
    return;
  }

  // create the parent directory
  PathName pathParent = path.GetDirectoryName();
  RemoveDirectoryDelimiter(pathParent.GetData());
  // RECURSION
  CreateDirectoryPath(pathParent);

  // we're done, if we have no file name
  if (path.GetFileName().Empty())
  {
    return;
  }

  auto trace_files = TraceStream::Open(MIKTEX_TRACE_FILES);

  trace_files->WriteLine("core", fmt::format(T_("creating directory {0}"), Q_(path)));

#if SET_SECURITY
  // create the directory itself
  shared_ptr<Session> session = Session::TryGet();
  if (session != nullptr
    && session->IsAdminMode()
    && (session->GetSpecialPath(SpecialPath::CommonConfigRoot) == path || session->GetSpecialPath(SpecialPath::CommonDataRoot) == path)
    && session->RunningAsAdministrator())
  {
    CreateDirectoryForEveryone(path.GetData());
    return;
  }
#endif
  if (!CreateDirectoryW(path.ToExtendedLengthPathName().ToWideCharString().c_str(), nullptr))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateDirectoryW", "path", path.ToString());
  }
}

HResult::~HResult()
{
  try
  {
    if (message != nullptr)
    {
      LocalFree(reinterpret_cast<HLOCAL>(message));
      message = nullptr;
    }
  }
  catch (const exception&)
  {
  }
}

string HResult::ToString() const
{
  stringstream s;
  s << "0x" << std::setfill('0') << std::setw(8) << std::hex << hr;
  return s.str();
}

string HResult::GetText()
{
  if (message == nullptr)
  {
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<wchar_t*>(&message), 0, nullptr);
    if (message == nullptr)
    {
      string str = ToString();
      size_t sizeChars = str.length() + 1;
      message = reinterpret_cast<wchar_t*>(LocalAlloc(0, sizeChars * sizeof(message[0])));
      if (message != nullptr)
      {
	StringUtil::CopyString(message, sizeChars, str.c_str());
      }
    }
  }
  return StringUtil::WideCharToUTF8(message);
}
