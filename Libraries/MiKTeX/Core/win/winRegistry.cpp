/* winRegistry.cpp: Windows registry operations

   Copyright (C) 1996-2019 Christian Schenk

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

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Registry>
#include <miktex/Core/win/winAutoResource>

#include "internal.h"

#include "Session/SessionImpl.h"
#include "winRegistry.h"

using namespace std;

using namespace MiKTeX::Core;

wstring MakeRegistryPath(const wstring& keyName)
{

  return fmt::format(L"{}\\{}", SessionImpl::GetSession()->IsMiKTeXDirect() ? UW_(MIKTEX_REGPATH_MAJOR_MINOR_MIKTEXDIRECT) : UW_(MIKTEX_REGPATH_SERIES), keyName);
}

HKEY ToHkey(ConfigurationScope scope)
{
  return scope == ConfigurationScope::User ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
}

bool winRegistry::TryGetRegistryValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, vector<BYTE>& value, DWORD& valueType)
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

bool winRegistry::TryGetRegistryValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, wstring& value, DWORD& valueType)
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
    const wchar_t* lpsz = reinterpret_cast<const wchar_t *>(valueBytes.data());
    MIKTEX_ASSERT_STRING(lpsz);
    value = lpsz;
    return true;
  }
  default:
    TraceError(fmt::format(T_("ignoring value {0} of type {1}"), Q_(WU_(valueName)), valueType));
    return false;
  }
}

bool winRegistry::TryGetRegistryValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, wstring& value)
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

bool winRegistry::TryGetRegistryValue(HKEY hkeyParent, const string& path, const string& valueName, string& value)
{
  wstring wideValue;
  bool result = TryGetRegistryValue(hkeyParent, UW_(path), UW_(valueName), wideValue);
  if (result)
  {
    value = WU_(wideValue);
  }
  return result;
}

bool winRegistry::TryDeleteRegistryKey(HKEY hkeyParent, const wstring& path)
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

bool winRegistry::TryDeleteRegistryKey(HKEY hkeyParent, const string& path)
{
  return TryDeleteRegistryKey(hkeyParent, UW_(path));
}

bool winRegistry::TryDeleteRegistryValue(HKEY hkeyParent, const wstring& path, const wstring& valueName)
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

bool winRegistry::TryDeleteRegistryValue(HKEY hkeyParent, const string& path, const string& valueName)
{
  return TryDeleteRegistryValue(hkeyParent, UW_(path), UW_(valueName));
}

void winRegistry::SetRegistryValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, const BYTE* value, size_t valueSize, DWORD valueType)
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
  result = RegSetValueExW(hkey, valueName.c_str(), 0, valueType, value, static_cast<DWORD>(valueSize));
  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT_2("RegSetValueExW", result, "valueName", WU_(valueName));
  }
}

void winRegistry::SetRegistryValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, const wstring& value, DWORD valueType)
{
  SetRegistryValue(hkeyParent, path, valueName, reinterpret_cast<const BYTE *>(value.c_str()), sizeof(value[0]) * (value.length() + 1), valueType);
}

void winRegistry::SetRegistryValue(HKEY hkeyParent, const string& path, const string& valueName, const string& value)
{
  SetRegistryValue(hkeyParent, UW_(path), UW_(valueName), UW_(value), REG_SZ);
}

bool winRegistry::TryGetRegistryValue(ConfigurationScope scope, const wstring& keyName, const wstring& valueName, wstring& value)
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  if (scope == ConfigurationScope::None)
  {
    // RECURSION
    if (!session->IsAdminMode() && TryGetRegistryValue(ConfigurationScope::User, keyName, valueName, value))
    {
      return true;
    }
    // RECURSION
    return TryGetRegistryValue(ConfigurationScope::Common, keyName, valueName, value);
  }
  else
  {
    return TryGetRegistryValue(ToHkey(scope), MakeRegistryPath(keyName), valueName, value);
  }
}

bool winRegistry::TryGetRegistryValue(ConfigurationScope scope, const string& keyName, const string& valueName, string& value)
{
  wstring wideValue;
  bool result = TryGetRegistryValue(scope, UW_(keyName), UW_(valueName), wideValue);
  if (result)
  {
    value = WU_(wideValue);
  }
  return result;
}

bool winRegistry::TryGetRegistryValue(ConfigurationScope scope, const wstring& keyName, const wstring& valueName, PathName& path)
{
  wstring value;
  if (!TryGetRegistryValue(scope, keyName, valueName, value))
  {
    return false;
  }
  path = value;
  return true;
}

bool winRegistry::TryGetRegistryValue(ConfigurationScope scope, const string& keyName, const string& valueName, PathName& path)
{
  return TryGetRegistryValue(scope, UW_(keyName), UW_(valueName), path);
}

bool winRegistry::TryDeleteRegistryValue(ConfigurationScope scope, const wstring& keyName, const wstring& valueName)
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  if (scope == ConfigurationScope::None)
  {
    // RECURSION
    return TryDeleteRegistryValue(session->IsAdminMode() ? ConfigurationScope::Common : ConfigurationScope::User, keyName, valueName);
  }
  else
  {
    return TryDeleteRegistryValue(ToHkey(scope), MakeRegistryPath(keyName), valueName);
  }
}

bool winRegistry::TryDeleteRegistryValue(ConfigurationScope scope, const string& keyName, const string& valueName)
{
  return TryDeleteRegistryValue(scope, UW_(keyName), UW_(valueName));
}

void winRegistry::SetRegistryValue(ConfigurationScope scope, const wstring& keyName, const wstring& valueName, const wstring& value)
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  if (scope == ConfigurationScope::None)
  {
    // RECURSION
    SetRegistryValue(session->IsAdminMode() ? ConfigurationScope::Common : ConfigurationScope::User, keyName, valueName, value);
  }
  else
  {
    SetRegistryValue(ToHkey(scope), MakeRegistryPath(keyName), valueName, value);
    wstring value2;
    if (scope == ConfigurationScope::Common && TryGetRegistryValue(ConfigurationScope::User, keyName, valueName, value2))
    {
      TryDeleteRegistryValue(ConfigurationScope::User, keyName, valueName);
    }
  }
}

void winRegistry::SetRegistryValue(ConfigurationScope scope, const string& keyName, const string& valueName, const string& value)
{
  SetRegistryValue(scope, UW_(keyName), UW_(valueName), UW_(value));
}
