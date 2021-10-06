/* winRegistry.cpp: Windows registry operations

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

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/win/Registry>
#include <miktex/Core/win/winAutoResource>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include "internal.h"

#include "Session/SessionImpl.h"
#include "winRegistry.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

wstring MakeRegistryPath(const wstring& keyName)
{

  return fmt::format(L"{}\\{}", SESSION_IMPL()->IsMiKTeXDirect() ? UW_(MIKTEX_REGPATH_MAJOR_MINOR_MIKTEXDIRECT) : UW_(MIKTEX_REGPATH_SERIES), keyName);
}

HKEY ToHkey(ConfigurationScope scope)
{
  return scope == ConfigurationScope::User ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
}

bool winRegistry::TryGetValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, vector<BYTE>& value, DWORD& valueType)
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

bool winRegistry::TryGetValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, wstring& value, DWORD& valueType)
{
  vector<BYTE> valueBytes;
  if (!TryGetValue(hkeyParent, path, valueName, valueBytes, valueType))
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
    auto trace_error = TraceStream::Open(MIKTEX_TRACE_ERROR);
    trace_error->WriteLine("core", TraceLevel::Error, fmt::format(T_("ignoring value {0} of type {1}"), Q_(WU_(valueName)), valueType));
    return false;
  }
}

bool winRegistry::TryGetValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, wstring& value)
{
  DWORD valueType;
  if (!TryGetValue(hkeyParent, path, valueName, value, valueType))
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

bool winRegistry::TryGetValue(HKEY hkeyParent, const string& path, const string& valueName, string& value)
{
  wstring wideValue;
  bool result = TryGetValue(hkeyParent, UW_(path), UW_(valueName), wideValue);
  if (result)
  {
    value = WU_(wideValue);
  }
  return result;
}

bool winRegistry::TryDeleteKey(HKEY hkeyParent, const wstring& path)
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

bool winRegistry::TryDeleteKey(HKEY hkeyParent, const string& path)
{
  return TryDeleteKey(hkeyParent, UW_(path));
}

bool winRegistry::TryDeleteValue(HKEY hkeyParent, const wstring& path, const wstring& valueName)
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

bool winRegistry::TryDeleteValue(HKEY hkeyParent, const string& path, const string& valueName)
{
  return TryDeleteValue(hkeyParent, UW_(path), UW_(valueName));
}

void winRegistry::SetValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, const BYTE* value, size_t valueSize, DWORD valueType)
{
  SESSION_IMPL()->trace_config->WriteLine("core", fmt::format("RegCreateKeyExW({0}, \"{1}\")", reinterpret_cast<void*>(hkeyParent), WU_(path)));
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

void winRegistry::SetValue(HKEY hkeyParent, const wstring& path, const wstring& valueName, const wstring& value, DWORD valueType)
{
  SetValue(hkeyParent, path, valueName, reinterpret_cast<const BYTE *>(value.c_str()), sizeof(value[0]) * (value.length() + 1), valueType);
}

void winRegistry::SetValue(HKEY hkeyParent, const string& path, const string& valueName, const string& value)
{
  SetValue(hkeyParent, UW_(path), UW_(valueName), UW_(value), REG_SZ);
}

bool winRegistry::TryGetValue(ConfigurationScope scope, const wstring& keyName, const wstring& valueName, wstring& value)
{
  shared_ptr<SessionImpl> session = SESSION_IMPL();
  if (scope == ConfigurationScope::None)
  {
    // RECURSION
    if (!session->IsAdminMode() && TryGetValue(ConfigurationScope::User, keyName, valueName, value))
    {
      return true;
    }
    // RECURSION
    return TryGetValue(ConfigurationScope::Common, keyName, valueName, value);
  }
  else
  {
    return TryGetValue(ToHkey(scope), MakeRegistryPath(keyName), valueName, value);
  }
}

bool winRegistry::TryGetValue(ConfigurationScope scope, const string& keyName, const string& valueName, string& value)
{
  wstring wideValue;
  bool result = TryGetValue(scope, UW_(keyName), UW_(valueName), wideValue);
  if (result)
  {
    value = WU_(wideValue);
  }
  return result;
}

bool winRegistry::TryGetValue(ConfigurationScope scope, const wstring& keyName, const wstring& valueName, PathName& path)
{
  wstring value;
  if (!TryGetValue(scope, keyName, valueName, value))
  {
    return false;
  }
  path = value;
  return true;
}

bool winRegistry::TryGetValue(ConfigurationScope scope, const string& keyName, const string& valueName, PathName& path)
{
  return TryGetValue(scope, UW_(keyName), UW_(valueName), path);
}

bool winRegistry::TryDeleteValue(ConfigurationScope scope, const wstring& keyName, const wstring& valueName)
{
  shared_ptr<SessionImpl> session = SESSION_IMPL();
  if (scope == ConfigurationScope::None)
  {
    // RECURSION
    return TryDeleteValue(session->IsAdminMode() ? ConfigurationScope::Common : ConfigurationScope::User, keyName, valueName);
  }
  else
  {
    return TryDeleteValue(ToHkey(scope), MakeRegistryPath(keyName), valueName);
  }
}

bool winRegistry::TryDeleteValue(ConfigurationScope scope, const string& keyName, const string& valueName)
{
  return TryDeleteValue(scope, UW_(keyName), UW_(valueName));
}

void winRegistry::SetValue(ConfigurationScope scope, const wstring& keyName, const wstring& valueName, const wstring& value)
{
  shared_ptr<SessionImpl> session = SESSION_IMPL();
  if (scope == ConfigurationScope::None)
  {
    // RECURSION
    SetValue(session->IsAdminMode() ? ConfigurationScope::Common : ConfigurationScope::User, keyName, valueName, value);
  }
  else
  {
    SetValue(ToHkey(scope), MakeRegistryPath(keyName), valueName, value);
    wstring value2;
    if (scope == ConfigurationScope::Common && TryGetValue(ConfigurationScope::User, keyName, valueName, value2))
    {
      TryDeleteValue(ConfigurationScope::User, keyName, valueName);
    }
  }
}

void winRegistry::SetValue(ConfigurationScope scope, const string& keyName, const string& valueName, const string& value)
{
  SetValue(scope, UW_(keyName), UW_(valueName), UW_(value));
}
