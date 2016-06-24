/* winRegistry.cpp: Windows registry operations

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

#include "miktex/Core/Registry.h"
#include "miktex/Core/win/winAutoResource.h"

#include "Session/SessionImpl.h"
#include "winRegistry.h"

using namespace MiKTeX::Core;
using namespace std;

bool winRegistry::TryGetRegistryValue(HKEY hkeyParent, const wstring & path, const wstring & valueName, vector<BYTE> & value, DWORD & valueType)
{
  HKEY hkey;
  long result = RegOpenKeyExW(hkeyParent, path.c_str(), 0, KEY_READ, &hkey);
  if (result == ERROR_SUCCESS)
  {
    DWORD valueSize = 0;
    result = RegQueryValueExW(hkey, valueName.c_str(), nullptr, &valueType, nullptr, &valueSize);
    if (result == ERROR_SUCCESS)
    {
      value.reserve(valueSize);
      result = RegQueryValueExW(hkey, valueName.c_str(), nullptr, &valueType, value.data(), &valueSize);
    }
    long result2 = RegCloseKey(hkey);
    if (result2 != ERROR_SUCCESS)
    {
      TraceWindowsError("RegCloseKey", result2, WU_(path), __FILE__, __LINE__);
    }
    if (result == ERROR_SUCCESS)
    {
    }
    else if (result != ERROR_FILE_NOT_FOUND)
    {
      TraceWindowsError("RegQueryValueExW", result, WU_(valueName), __FILE__, __LINE__);
    }
  }
  else if (result != ERROR_FILE_NOT_FOUND)
  {
    TraceWindowsError("RegOpenKeyExW", result, WU_(path), __FILE__, __LINE__);
  }
  return result == ERROR_SUCCESS;
}

bool winRegistry::TryGetRegistryValue(HKEY hkeyParent, const wstring & path, const wstring & valueName, wstring & value, DWORD & valueType)
{
  vector<BYTE> valueBytes;
  if (!TryGetRegistryValue(hkeyParent, path, valueName, valueBytes, valueType))
  {
    return false;
  }
  switch (valueType)
  {
  case REG_SZ:
  case REG_EXPAND_SZ:
  {
    const wchar_t * lpsz = reinterpret_cast<const wchar_t *>(valueBytes.data());
    MIKTEX_ASSERT_STRING(lpsz);
    value = lpsz;
    return true;
  }
  default:
    TraceError(T_("ignoring value %s of type %u"), Q_(WU_(valueName)), static_cast<unsigned>(valueType));
    return false;
  }
}

bool winRegistry::TryGetRegistryValue(HKEY hkeyParent, const wstring & path, const wstring & valueName, wstring & value)
{
  DWORD valueType;
  if (!TryGetRegistryValue(hkeyParent, path, valueName, value, valueType))
  {
    return false;
  }
  if (valueType == REG_EXPAND_SZ)
  {
    DWORD n = ExpandEnvironmentStringsW(value.c_str(), nullptr, 0);
    if (n == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("ExpandEnvironmentStringsW", "str", WU_(path));
    }
    vector<wchar_t> buf;
    buf.reserve(n);
    DWORD n2 = ExpandEnvironmentStringsW(value.c_str(), buf.data(), n);
    if (n2 == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("ExpandEnvironmentStringsW", "str", WU_(path));
    }
    MIKTEX_ASSERT(n2 == n);
    value.assign(buf.data(), n2);
  }
  return true;
}

bool winRegistry::TryGetRegistryValue(HKEY hkeyParent, const string & path, const string & valueName, string & value)
{
  wstring wideValue;
  bool result = TryGetRegistryValue(hkeyParent, UW_(path), UW_(valueName), wideValue);
  if (result)
  {
    value = WU_(wideValue);
  }
  return result;
}

bool winRegistry::TryDeleteRegistryKey(HKEY hkeyParent, const wstring & path)
{
  long result = SHDeleteKeyW(hkeyParent, path.c_str());
  if (result != ERROR_SUCCESS)
  {
    if (result == ERROR_FILE_NOT_FOUND)
    {
      return false;
    }
    MIKTEX_FATAL_WINDOWS_RESULT_2("SHDeleteKeyW", result, "path", WU_(path));
  }
  return true;
}

bool winRegistry::TryDeleteRegistryKey(HKEY hkeyParent, const string & path)
{
  return TryDeleteRegistryKey(hkeyParent, UW_(path));
}

bool winRegistry::TryDeleteRegistryValue(HKEY hkeyParent, const wstring & path, const wstring & valueName)
{
  HKEY hkey;
  long result = RegOpenKeyExW(hkeyParent, path.c_str(), 0, KEY_ALL_ACCESS, &hkey);
  if (result != ERROR_SUCCESS)
  {
    if (result == ERROR_FILE_NOT_FOUND)
    {
      return false;
    }
    MIKTEX_FATAL_WINDOWS_RESULT_2("RegOpenKeyExW", result, "path", WU_(path));
  }
  AutoHKEY autoClose(hkey);
  result = RegDeleteValueW(hkey, valueName.c_str());
  if (result != ERROR_SUCCESS)
  {
    if (result == ERROR_FILE_NOT_FOUND)
    {
      return false;
    }
    MIKTEX_FATAL_WINDOWS_RESULT_2("RegDeleteValueW", result, "valueName", WU_(valueName));
  }
  return true;
}

bool winRegistry::TryDeleteRegistryValue(HKEY hkeyParent, const string & path, const string & valueName)
{
  return TryDeleteRegistryValue(hkeyParent, UW_(path), UW_(valueName));
}

void winRegistry::SetRegistryValue(HKEY hkeyParent, const wstring & path, const wstring & valueName, const BYTE * value, size_t valueSize, DWORD valueType)
{
  SessionImpl::GetSession()->trace_config->WriteFormattedLine("core", "RegCreateKeyExW (%p, \"%s\")", reinterpret_cast<void*>(hkeyParent), WU_(path));
  HKEY hkey;
  DWORD disp;
  long result = RegCreateKeyExW(hkeyParent, path.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hkey, &disp);
  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT_2("RegCreateKeyExW", result, "path", WU_(path));
  }
  AutoHKEY autoClose(hkey);
  result = RegSetValueExW(hkey, valueName.c_str(), 0, valueType, value, valueSize);
  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT_2("RegSetValueExW", result, "valueName", WU_(valueName));
  }
}

void winRegistry::SetRegistryValue(HKEY hkeyParent, const wstring & path, const wstring & valueName, const wstring & value, DWORD valueType)
{
  SetRegistryValue(hkeyParent, path, valueName, reinterpret_cast<const BYTE *>(value.c_str()), sizeof(value[0]) * (value.length() + 1), valueType);
}

void winRegistry::SetRegistryValue(HKEY hkeyParent, const string & path, const string & valueName, const string & value)
{
  SetRegistryValue(hkeyParent, UW_(path), UW_(valueName), UW_(value), REG_SZ);
}

bool winRegistry::TryGetRegistryValue(TriState shared, const wstring & keyName, const wstring & valueName, wstring & value)
{
  if (shared == TriState::Undetermined)
  {
#if 1
    // RECURSION
    if (TryGetRegistryValue(TriState::False, keyName, valueName, value))
    {
      return true;
    }
    // RECURSION
    return TryGetRegistryValue(TriState::True, keyName, valueName, value);
#else
    shared = SessionImpl::GetSession()->IsSharedMiKTeXSetup();
    if (shared == TriState::Undetermined)
    {
      shared = TriState::False;
    }
    // RECURSION
    return TryGetRegistryValue(shared, keyName, valueName, value);
#endif
  }
  wstring registryPath = SessionImpl::GetSession()->IsMiKTeXDirect() ? UW_(MIKTEX_REGPATH_SERIES_MIKTEXDIRECT) : UW_(MIKTEX_REGPATH_SERIES);
  registryPath += L'\\';
  registryPath += keyName;
  return TryGetRegistryValue(shared == TriState::False ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, registryPath, valueName, value);
}

bool winRegistry::TryGetRegistryValue(TriState shared, const string & keyName, const string & valueName, string & value)
{
  wstring wideValue;
  bool result = TryGetRegistryValue(shared, UW_(keyName), UW_(valueName), wideValue);
  if (result)
  {
    value = WU_(wideValue);
  }
  return result;
}

bool winRegistry::TryGetRegistryValue(TriState shared, const wstring & keyName, const wstring & valueName, PathName & path)
{
  wstring value;
  if (!TryGetRegistryValue(shared, keyName, valueName, value))
  {
    return false;
  }
  path = value;
  return true;
}

bool winRegistry::TryGetRegistryValue(TriState shared, const string & keyName, const string & valueName, PathName & path)
{
  return TryGetRegistryValue(shared, UW_(keyName), UW_(valueName), path);
}

bool winRegistry::TryDeleteRegistryValue(TriState shared, const wstring & keyName, const wstring & valueName)
{
  if (shared == TriState::Undetermined)
  {
    // RECURSION
    bool done = TryDeleteRegistryValue(TriState::False, keyName, valueName);
    return TryDeleteRegistryValue(TriState::True, keyName, valueName) || done;
  }
  wstring registryPath = SessionImpl::GetSession()->IsMiKTeXDirect() ? UW_(MIKTEX_REGPATH_SERIES_MIKTEXDIRECT) : UW_(MIKTEX_REGPATH_SERIES);
  registryPath += L'\\';
  registryPath += keyName;
  return TryDeleteRegistryValue((shared == TriState::False ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE), registryPath, valueName);
}

bool winRegistry::TryDeleteRegistryValue(TriState shared, const string & keyName, const string & valueName)
{
  return TryDeleteRegistryValue(shared, UW_(keyName), UW_(valueName));
}

void winRegistry::SetRegistryValue(TriState shared, const wstring & keyName, const wstring & valueName, const wstring & value)
{
  if (shared == TriState::Undetermined)
  {
    shared = SessionImpl::GetSession()->IsAdminMode() ? TriState::True : TriState::False;
    // RECURSION
    SetRegistryValue(shared, keyName, valueName, value);
    return;
  }
  wstring registryPath = SessionImpl::GetSession()->IsMiKTeXDirect() ? UW_(MIKTEX_REGPATH_SERIES_MIKTEXDIRECT) : UW_(MIKTEX_REGPATH_SERIES);
  registryPath += L'\\';
  registryPath += keyName;
  HKEY hkeyParent = shared == TriState::False ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  SetRegistryValue(hkeyParent, registryPath, valueName, value);
  wstring value2;
  if (hkeyParent == HKEY_LOCAL_MACHINE && TryGetRegistryValue(TriState::False, keyName, valueName, value2))
  {
    TryDeleteRegistryValue(TriState::False, keyName, valueName);
  }
}

void winRegistry::SetRegistryValue(TriState shared, const string & keyName, const string & valueName, const string & value)
{
  SetRegistryValue(shared, UW_(keyName), UW_(valueName), UW_(value));
}
