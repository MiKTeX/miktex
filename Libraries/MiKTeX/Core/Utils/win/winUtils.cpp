/* winUtil.cpp:

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
#include <shlobj.h>
#include <strsafe.h>
#include <wininet.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/Paths>
#include <miktex/Core/win/WindowsVersion>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include <miktex/Util/PathName>

#include "internal.h"

#include "Session/SessionImpl.h"
#include "Utils/inliners.h"
#include "win/winRegistry.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

PathName Utils::GetFolderPath(int nFolder, int nFallbackFolder, bool getCurrentPath)
{
  DWORD flags = getCurrentPath ? SHGFP_TYPE_CURRENT : SHGFP_TYPE_DEFAULT;
  wchar_t szPath[MAX_PATH];
  HRESULT hr = SHGetFolderPathW(nullptr, nFolder | CSIDL_FLAG_CREATE, nullptr, flags, szPath);
  if ((hr == E_FAIL || hr == E_INVALIDARG) && (nFolder != nFallbackFolder))
  {
    hr = SHGetFolderPathW(nullptr, nFallbackFolder | CSIDL_FLAG_CREATE, nullptr, flags, szPath);
  }
  if (hr == E_FAIL)
  {
    MIKTEX_FATAL_ERROR_2(T_("A required file system folder does not exist."), "nFolder", std::to_string(nFolder));
  }
  if (hr == E_INVALIDARG)
  {
    MIKTEX_FATAL_ERROR_2(T_("Unsupported Windows product."), "nFolder", std::to_string(nFolder));
  }
  if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
  {
    MIKTEX_FATAL_ERROR_2(T_("A required file system folder cannot be accessed."), "nFolder", std::to_string(nFolder));
  }
  if (hr != S_OK)
  {
    MIKTEX_FATAL_ERROR_2(T_("A required file system path could not be retrieved."), "nfolder", std::to_string(nFolder), "hr", std::to_string(hr));
  }
  return PathName(szPath);
}

string GetOperatingSystem(const OSVERSIONINFOEXW& osvi, const SYSTEM_INFO& si)
{
  string str = "Windows ";
  if (osvi.dwMajorVersion == 10)
  {
    if (osvi.wProductType == VER_NT_WORKSTATION)
    {
      str += "10";
    }
    else
    {
      str += "Server 2016";
    }
  }
  else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion <= 3)
  {
    if (osvi.wProductType == VER_NT_WORKSTATION)
    {
      if (osvi.dwMinorVersion == 0)
      {
        str += "Vista";
      }
      else if (osvi.dwMinorVersion == 1)
      {
        str += "7";
      }
      else if (osvi.dwMinorVersion == 2)
      {
        str += "8";
      }
      else if (osvi.dwMinorVersion == 3)
      {
        str += "8.1";
      }
    }
    else
    {
      if (osvi.dwMinorVersion == 0)
      {
        str += "Server 2008";
      }
      else if (osvi.dwMinorVersion == 1)
      {
        str += "Server 2008 R2";
      }
      else if (osvi.dwMinorVersion == 2)
      {
        str += "Server 2012";
      }
      else if (osvi.dwMinorVersion == 3)
      {
        str += "Server 2012 R2";
      }
    }
  }
  else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
  {
    if (GetSystemMetrics(SM_SERVERR2) != 0)
    {
      str += "Server 2003 R2";
    }
    else if ((osvi.wSuiteMask & VER_SUITE_WH_SERVER) != 0)
    {
      str += "Home Server";
    }
    else if (osvi.wProductType == VER_NT_WORKSTATION && (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64))
    {
      str += "XP Professional x64 Edition";
    }
    else
    {
      str += "Server 2003";
    }
  }
  else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
  {
    str += "XP";
  }
  else
  {
    // unknown (or unsupported) Windows version
    MIKTEX_ASSERT(false);
    str += std::to_string(osvi.dwMajorVersion);
    str += ".";
    str += std::to_string(osvi.dwMinorVersion);
  }
  return str;
}

typedef void (WINAPI* PGNSI) (LPSYSTEM_INFO);
typedef BOOL(WINAPI* PGPI) (DWORD, DWORD, DWORD, DWORD, DWORD *);

inline HMODULE GetKernel32Module()
{
  HMODULE h = GetModuleHandleW(L"kernel32.dll");
  if (h == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return h;
}

string GetWindowsProductType(const OSVERSIONINFOEXW& osvi, const SYSTEM_INFO& si)
{
  string str;
  if (osvi.dwMajorVersion >= 6)
  {
    PGPI GetProductInfo = (PGPI)GetProcAddress(GetKernel32Module(), "GetProductInfo");
    if (GetProductInfo == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    DWORD productType;
    if (!GetProductInfo(10, 0, 0, 0, &productType))
    {
      MIKTEX_UNEXPECTED();
    }
    switch (productType)
    {
    case PRODUCT_UNLICENSED:
      str = T_("(not activated, grace period expired)");
      break;
    case PRODUCT_BUSINESS:
      str = "Business";
      break;
    case PRODUCT_BUSINESS_N:
      str = "Business N";
      break;
    case PRODUCT_CLUSTER_SERVER:
      str = "HPC Edition";
      break;
    case PRODUCT_CLUSTER_SERVER_V:
      str = "Server Hyper Core V";
      break;
#if !defined(PRODUCT_CORE)
#  define PRODUCT_CORE 0x00000065
#endif
    case PRODUCT_CORE:
      str = "Home";
      break;
#if !defined(PRODUCT_CORE_N)
#  define PRODUCT_CORE_N 0x00000062
#endif
    case PRODUCT_CORE_N:
      str = "Home N";
      break;
#if !defined(PRODUCT_CORE_COUNTRYSPECIFIC)
#  define PRODUCT_CORE_COUNTRYSPECIFIC 0x00000063
#endif
    case PRODUCT_CORE_COUNTRYSPECIFIC:
      str = "Home China";
      break;
#if !defined(PRODUCT_CORE_SINGLELANGUAGE)
#  define PRODUCT_CORE_SINGLELANGUAGE 0x00000064
#endif
    case PRODUCT_CORE_SINGLELANGUAGE:
      str = "Home Single Language";
      break;
#if !defined(PRODUCT_MOBILE_CORE)
#  define PRODUCT_MOBILE_CORE 0x00000068
#endif
    case PRODUCT_MOBILE_CORE:
      str = "Mobile";
      break;
#if !defined(PRODUCT_MOBILE_ENTERPRISE)
#  define PRODUCT_MOBILE_ENTERPRISE 0x00000085
#endif
    case PRODUCT_MOBILE_ENTERPRISE:
      str = "Mobile Enterprise";
      break;
#if !defined(PRODUCT_EDUCATION)
#  define PRODUCT_EDUCATION 0x00000079
#endif
    case PRODUCT_EDUCATION:
      str = "Education";
      break;
#if !defined(PRODUCT_EDUCATION_N)
#  define PRODUCT_EDUCATION_N 0x0000007A
#endif
    case PRODUCT_EDUCATION_N:
      str = "Education N";
      break;
#if !defined(PRODUCT_DATACENTER_EVALUATION_SERVER)
#  define PRODUCT_DATACENTER_EVALUATION_SERVER 0x00000050
#endif
    case PRODUCT_DATACENTER_EVALUATION_SERVER:
      str = "Server Datacenter (evaluation installation)";
      break;
    case PRODUCT_DATACENTER_SERVER:
      str = "Server Datacenter (full installation)";
      break;
    case PRODUCT_DATACENTER_SERVER_CORE:
      str = "Server Datacenter (core installation)";
      break;
    case PRODUCT_DATACENTER_SERVER_CORE_V:
      str = "Server Datacenter without Hyper-V (core installation)";
      break;
    case PRODUCT_DATACENTER_SERVER_V:
      str = "Server Datacenter without Hyper-V (full installation)";
      break;
    case PRODUCT_ENTERPRISE:
      str = "Enterprise";
      break;
    case PRODUCT_ENTERPRISE_E:
      str = "Enterprise E";
      break;
#if !defined(PRODUCT_ENTERPRISE_N_EVALUATION)
#  define PRODUCT_ENTERPRISE_N_EVALUATION 0x00000054
#endif
    case PRODUCT_ENTERPRISE_N_EVALUATION:
      str = "Enterprise N (evaluation installation)";
      break;
    case PRODUCT_ENTERPRISE_N:
      str = " Enterprise N";
      break;
#if !defined(PRODUCT_ENTERPRISE_EVALUATION)
#  define PRODUCT_ENTERPRISE_EVALUATION 0x00000048
#endif
    case PRODUCT_ENTERPRISE_EVALUATION:
      str = "Enterprise (evaluation installation)";
      break;
    case PRODUCT_ENTERPRISE_SERVER:
      str = "Server Enterprise (full installation)";
      break;
    case PRODUCT_ENTERPRISE_SERVER_CORE:
      str = "Server Enterprise (core installation)";
      break;
    case PRODUCT_ENTERPRISE_SERVER_CORE_V:
      str = "Server Enterprise without Hyper-V (core installation)";
      break;
    case PRODUCT_ENTERPRISE_SERVER_IA64:
      str = "Server Enterprise for Itanium-based Systems";
      break;
    case PRODUCT_ENTERPRISE_SERVER_V:
      str = "Enterprise without Hyper-V (full installation)";
      break;
    case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
      str = "Essential Server Solution Management";
      break;
    case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
      str = "Essential Server Solution Additional";
      break;
    case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
      str = "Essential Server Solution Management SVC";
      break;
    case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
      str = "Essential Server Solution Additional SVC";
      break;
    case PRODUCT_HOME_BASIC:
      str = "Home Basic";
      break;
    case PRODUCT_HOME_BASIC_N:
      str = "Home Basic N";
      break;
    case PRODUCT_HOME_PREMIUM:
      str = "Home Premium";
      break;
    case PRODUCT_HOME_PREMIUM_N:
      str = "Home Premium N";
      break;
    case PRODUCT_HOME_PREMIUM_SERVER:
      str = "Home Server 2011";
      break;
    case PRODUCT_HOME_SERVER:
      str = "Storage Server 2008 R2 Essentials";
      break;
    case PRODUCT_HYPERV:
      str = "Hyper-V Server";
      break;
    case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
      str = "Essential Business Server Management Server";
      break;
    case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
      str = "Essential Business Server Messaging Server";
      break;
    case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
      str = "Essential Business Server Security Server";
      break;
#if !defined(PRODUCT_MULTIPOINT_STANDARD_SERVER)
#  define PRODUCT_MULTIPOINT_STANDARD_SERVER 0x0000004C
#endif
    case PRODUCT_MULTIPOINT_STANDARD_SERVER:
      str = "MultiPoint Server Standard(full installation)";
      break;
#if !defined(PRODUCT_MULTIPOINT_PREMIUM_SERVER)
#  define PRODUCT_MULTIPOINT_PREMIUM_SERVER 0x0000004D
#endif
    case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
      str = "MultiPoint Server Premium (full installation)";
      break;
    case PRODUCT_PROFESSIONAL:
      str = osvi.dwMajorVersion >= 10 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 2) ? "Pro" : "Professional";
      break;
    case PRODUCT_PROFESSIONAL_N:
      str = osvi.dwMajorVersion >= 10 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 2) ? "Pro N" : "Professional N";
      break;
#if !defined(PRODUCT_PROFESSIONAL_WMC)
#  define PRODUCT_PROFESSIONAL_WMC 0x00000067
#endif
    case PRODUCT_PROFESSIONAL_WMC:
      str = "Professional with Media Center";
      break;
    case PRODUCT_SB_SOLUTION_SERVER_EM:
      str = "Server For SB Solutions EM";
      break;
    case PRODUCT_SERVER_FOR_SB_SOLUTIONS:
      str = "Server For SB Solutions";
      break;
    case PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
      str = "Server For SB Solutions EM";
      break;
    case PRODUCT_SERVER_FOR_SMALLBUSINESS:
      str = "Server 2008 for Windows Essential Server Solutions";
      break;
    case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
      str = "Server 2008 without Hyper-V for Windows Essential Server Solutions";
      break;
    case PRODUCT_SERVER_FOUNDATION:
      str = "Server Foundation";
      break;
    case PRODUCT_SB_SOLUTION_SERVER:
      str = "Small Business Server 2011 Essentials";
      break;
    case PRODUCT_SMALLBUSINESS_SERVER:
      str = "Small Business Server";
      break;
    case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
      str = "Small Business Server Premium";
      break;
    case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
      str = "Small Business Server Premium (core installation)";
      break;
    case PRODUCT_SOLUTION_EMBEDDEDSERVER:
      str = "Windows MultiPoint Server";
      break;
#if !defined(PRODUCT_STANDARD_EVALUATION_SERVER)
#  define PRODUCT_STANDARD_EVALUATION_SERVER 0x0000004F
#endif
    case PRODUCT_STANDARD_EVALUATION_SERVER:
      str = "Server Standard (evaluation installation)";
      break;
    case PRODUCT_STANDARD_SERVER:
      str = "Server Standard";
      break;
    case PRODUCT_STANDARD_SERVER_CORE:
      str = "Server Standard (core installation)";
      break;
    case PRODUCT_STANDARD_SERVER_V:
      str = "Server Standard without Hyper-V";
      break;
    case PRODUCT_STANDARD_SERVER_CORE_V:
      str = "Server Standard without Hyper-V (core installation)";
      break;
    case PRODUCT_STANDARD_SERVER_SOLUTIONS:
      str = "Server Solutions Premium";
      break;
    case PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
      str = "Server Solutions Premium (core installation)";
      break;
    case PRODUCT_STARTER:
      str = "Starter";
      break;
    case PRODUCT_STARTER_N:
      str = "Starter N";
      break;
    case PRODUCT_STORAGE_ENTERPRISE_SERVER:
      str = "Storage Server Enterprise";
      break;
    case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
      str = "Storage Server Enterprise (core installation)";
      break;
    case PRODUCT_STORAGE_EXPRESS_SERVER:
      str = "Storage Server Express";
      break;
    case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
      str = "Storage Server Express (core installation)";
      break;
#if !defined(PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER)
#  define PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER 0x00000060
#endif
    case PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
      str = "Storage Server Standard (evaluation installation)";
      break;
    case PRODUCT_STORAGE_STANDARD_SERVER:
      str = "Storage Server Standard";
      break;
    case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
      str = "Storage Server Standard (core installation)";
      break;
#if !defined(PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER)
#  define PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER 0x0000005F
#endif
    case PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
      str = "Storage Server Workgroup (evaluation installation)";
      break;
    case PRODUCT_STORAGE_WORKGROUP_SERVER:
      str = "Storage Server Workgroup";
      break;
    case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
      str = "Storage Server Workgroup (core installation)";
      break;
    case PRODUCT_UNDEFINED:
      str = T_("(unknown product)");
      break;
    case PRODUCT_ULTIMATE:
      str = "Ultimate";
      break;
    case PRODUCT_ULTIMATE_N:
      str = "Ultimate N";
      break;
    case PRODUCT_WEB_SERVER:
      str = "Web Server (full installation)";
      break;
    case PRODUCT_WEB_SERVER_CORE:
      str = "Web Server (core installation)";
      break;
    }
  }
  else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
  {
    if (osvi.wProductType != VER_NT_WORKSTATION)
    {
      if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
      {
        if ((osvi.wSuiteMask & VER_SUITE_DATACENTER) != 0)
        {
          str = "Datacenter Edition for Itanium-based Systems";
        }
        else if ((osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0)
        {
          str = "Enterprise Edition for Itanium-based Systems";
        }
      }
      else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
      {
        if ((osvi.wSuiteMask & VER_SUITE_DATACENTER) != 0)
        {
          str = "Datacenter x64 Edition";
        }
        else if ((osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0)
        {
          str = "Enterprise x64 Edition";
        }
        else
        {
          str = "Standard x64 Edition";
        }
      }
      else if ((osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER) != 0)
      {
        str = "Compute Cluster Edition";
      }
      else if ((osvi.wSuiteMask & VER_SUITE_DATACENTER) != 0)
      {
        str = "Datacenter Edition";
      }
      else if ((osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0)
      {
        str = "Enterprise Edition";
      }
      else if ((osvi.wSuiteMask & VER_SUITE_BLADE) != 0)
      {
        str = "Web Edition";
      }
      else
      {
        str = "Standard Edition";
      }
    }
  }
  else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
  {
    if ((osvi.wSuiteMask & VER_SUITE_PERSONAL) != 0)
    {
      str = "Home Edition";
    }
    else
    {
      str = "Professional";
    }
  }
  else
  {
    // unknown (or unsupported) Windows version
    MIKTEX_ASSERT(false);
  }
  return str;
}

string Utils::GetOSVersionString()
{
  OSVERSIONINFOEXW osvi;
  ZeroMemory(&osvi, sizeof(osvi));
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  if (!GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&osvi)))
  {
    UNSUPPORTED_PLATFORM();
  }
  SYSTEM_INFO si;
  ZeroMemory(&si, sizeof(si));
  GetNativeSystemInfo(&si);
  string str = GetOperatingSystem(osvi, si) + " " + GetWindowsProductType(osvi, si);
  if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
  {
    str += ", 32-bit";
  }
  else
  {
    str += ", 64-bit";
  }
  if (osvi.szCSDVersion[0] != 0)
  {
    str += ", ";
    str += StringUtil::WideCharToUTF8(osvi.szCSDVersion);
  }
  str += ", build ";
  str += std::to_string(osvi.dwBuildNumber);
  return str;
}

bool Utils::RunningOnAServer()
{
  OSVERSIONINFOEXW osvi;
  ZeroMemory(&osvi, sizeof(osvi));
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  if (!GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&osvi)))
  {
    UNSUPPORTED_PLATFORM();
  }
  return osvi.wProductType != VER_NT_WORKSTATION;
}

// see Q246772
bool Utils::GetDefPrinter(string& printerName)
{
  OSVERSIONINFOW osv;
  osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
  GetVersionExW(&osv);
  if (osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
  {
#if defined(MIKTEX_SUPPORT_LEGACY_WINDOWS)
    unsigned long dwNeeded, dwReturned;
    EnumPrintersW(PRINTER_ENUM_DEFAULT, 0, 2, 0, 0, &dwNeeded, &dwReturned);
    if (dwNeeded == 0)
    {
      return false;
    }
    AutoGlobalMemory hMem(GlobalAlloc(GPTR, dwNeeded));
    if (hMem.Get() == nullptr)
    {
      OUT_OF_MEMORY("GlobalAlloc");
    }
    PRINTER_INFO_2W* ppi2 =
      reinterpret_cast<PRINTER_INFO_2W *>(hMem.Get());
    if (!EnumPrintersW(PRINTER_ENUM_DEFAULT, 0, 2, reinterpret_cast<LPBYTE>(ppi2), dwNeeded, &dwNeeded, &dwReturned))
    {
      return false;
    }
    size_t l = StrLen(ppi2->pPrinterName);
    if (l >= *pBufferSize)
    {
      *pBufferSize = l + 1;
      return false;
    }
    StringUtil::CopyString(pPrinterName, *pBufferSize, ppi2->pPrinterName);
    *pBufferSize = l + 1;
    return true;
#else
    UNSUPPORTED_PLATFORM();
#endif
  }
  else
  {
    if (osv.dwPlatformId != VER_PLATFORM_WIN32_NT)
    {
      MIKTEX_UNEXPECTED();
    }
    if (osv.dwMajorVersion >= 5)
    {
      CharBuffer<wchar_t> printerNameBuf;
      DWORD dwBufferSize = static_cast<DWORD>(printerNameBuf.GetCapacity());
      BOOL bDone = GetDefaultPrinterW(printerNameBuf.GetData(), &dwBufferSize);
      if (!bDone)
      {
        if (::GetLastError() == ERROR_FILE_NOT_FOUND)
        {
          return false;
        }
        else
        {
          MIKTEX_FATAL_WINDOWS_ERROR("GetDefaultPrinterW");
        }
      }
      else
      {
        printerName = WU_(printerNameBuf.GetData());
        return true;
      }
    }
    else
    {
#if defined(MIKTEX_SUPPORT_LEGACY_WINDOWS)
      wchar_t cBuffer[4096];
      if (GetProfileStringW(L"windows", L"device", L",,,", cBuffer, 4096) <= 0)
      {
        return false;
      }
      Tokenizer tok(StringUtil::WideCharToUTF8(cBuffer), ",");
      if (tok.GetCurrent() == nullptr)
      {
        return false;
      }
      unsigned long l = static_cast<unsigned long>(StrLen(tok.GetCurrent()));
      if (l >= *pBufferSize)
      {
        *pBufferSize = l + 1;
        return false;
      }
      StringUtil::CopyString(pPrinterName, *pBufferSize, tok.GetCurrent());
      *pBufferSize = l + 1;
      return true;
#else
      UNSUPPORTED_PLATFORM();
#endif
    }
  }
}

void Utils::SetEnvironmentString(const string& valueName, const string& value)
{
  string oldValue;
  if (::GetEnvironmentString(valueName, oldValue) && oldValue == value)
  {
    return;
  }
  auto trace_config = TraceStream::Open(MIKTEX_TRACE_CONFIG);
  trace_config->WriteLine("core", fmt::format(T_("setting env {0}={1}"), valueName, value));
#if defined(_MSC_VER) || defined(__MINGW32__)
  if (_wputenv_s(UW_(valueName), UW_(value)) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("_wputenv_s", "valueName", valueName);
  }
#else
  string str = valueName;
  str += '=';
  str += value;
  if (putenv(str.c_str()) != 0)
  {
    FATAL_CRT_ERROR("putenv", str.c_str());
  }
#endif
}

void Utils::RemoveEnvironmentString(const string& valueName)
{
  SetEnvironmentString(valueName, "");
}

/* See MSDN community content:

Using lpExecInfo->lpClass and lpExecInfo->hkeyClass (KJK_Hyperion)

If you are afraid ShellExecute(Ex) might misidentify the file type,
because the path might be ambiguous or if you use a non-standard
type system, you can force one by filling the lpClass or hkeyClass
fields in lpExecInfo. You set lpClass to either the extension
(e.g. ".txt") or the URL scheme (e.g. "http") or the OLE
ProgId/ClassId; better yet, you use AssocQueryKey to retrieve the
appropriate hkeyClass for the given class name and verb. For
example, this is how you safely execute URLs with ShellExecute(Ex):
*/
BOOL ShellExecuteURLExInternal(LPSHELLEXECUTEINFOW lpExecInfo)
{
  BOOL bRet;
  DWORD dwErr;
  HRESULT hr;
  wchar_t szSchemeBuffer[INTERNET_MAX_SCHEME_LENGTH + 1];
  HKEY hkeyClass;

  /* Default error codes */
  bRet = FALSE;
  dwErr = ERROR_INVALID_PARAMETER;
  hr = S_OK;

  lpExecInfo->hInstApp =
    (HINSTANCE)UlongToHandle(SE_ERR_ACCESSDENIED);

  /* Validate parameters */
  if
    (
      lpExecInfo->cbSize == sizeof(*lpExecInfo) &&
      lpExecInfo->lpFile != NULL &&
      (lpExecInfo->fMask & SEE_MASK_INVOKEIDLIST) == 0 &&
      (lpExecInfo->fMask & SEE_MASK_CLASSNAME) == 0 &&
      (lpExecInfo->fMask & 0x00400000) == 0 /* SEE_MASK_FILEANDURL */
      )
  {
    /* Extract the scheme out of the URL */
    wstring file(lpExecInfo->lpFile);
    size_t colonpos = file.find(L':');

    /* Is the URL really, unambiguously an URL? */
    if
      (
        colonpos != wstring::npos
        )
    {
      wstring scheme = file.substr(0, colonpos);
      /* We need the scheme name NUL-terminated, so we copy it */
      hr = StringCbCopyNW
      (
        szSchemeBuffer, sizeof(szSchemeBuffer), scheme.c_str(), scheme.length() * sizeof(wchar_t)
      );

      if (SUCCEEDED(hr))
      {
        /* Is the URL scheme a registered ProgId? */
        hr = AssocQueryKeyW
        (
          ASSOCF_INIT_IGNOREUNKNOWN, ASSOCKEY_CLASS, szSchemeBuffer, NULL, &hkeyClass
        );

        if (SUCCEEDED(hr))
        {
          /* Is the ProgId really an URL scheme? */
          if (WindowsVersion::IsWindows8OrGreater())
          {
            dwErr = NO_ERROR;
          }
          else
          {
            dwErr = RegQueryValueExW
            (
              hkeyClass, L"URL Protocol", NULL, NULL, NULL, NULL
            );
          }

          /* All clear! */
          if (dwErr == NO_ERROR || dwErr == ERROR_MORE_DATA)
          {
            /* Don't let ShellExecuteEx guess */
            lpExecInfo->fMask |= SEE_MASK_CLASSKEY;
            lpExecInfo->lpClass = NULL;
            lpExecInfo->hkeyClass = hkeyClass;

            /* Finally, execute the damn URL */
            bRet = ShellExecuteExW(lpExecInfo);

            /* To preserve ShellExecuteEx's last error */
            dwErr = NO_ERROR;
          }

          RegCloseKey(hkeyClass);
        }
      }
    }
  }

  /* Last error was a HRESULT */
  if (FAILED(hr))
  {
    /* Try to dissect it */
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
      dwErr = HRESULT_CODE(hr);
    else
      dwErr = hr;
  }

  /* We have a last error to set */
  if (dwErr)
    SetLastError(dwErr);

  return bRet;
}

BOOL ShellExecuteURLEx(LPSHELLEXECUTEINFOW lpExecInfo)
{
  BOOL bRet;
  SHELLEXECUTEINFOW ExecInfo;

  /* We use a copy of the parameters, because you never know */
  CopyMemory(&ExecInfo, lpExecInfo, sizeof(ExecInfo));

  /* Do the magic */
  bRet = ShellExecuteURLExInternal(&ExecInfo);

  /* These need to be copied back */
  lpExecInfo->hInstApp = ExecInfo.hInstApp;
  lpExecInfo->hProcess = ExecInfo.hProcess;
  return bRet;
}

HINSTANCE ShellExecuteURL(HWND hwnd, const wchar_t* lpOperation, const wchar_t* lpFile, const wchar_t* lpParameters, const wchar_t* lpDirectory, INT nShowCmd)
{
  SHELLEXECUTEINFOW ExecuteInfo;

  ExecuteInfo.fMask = SEE_MASK_FLAG_NO_UI; /* Odd but true */
  ExecuteInfo.hwnd = hwnd;
  ExecuteInfo.cbSize = sizeof(ExecuteInfo);
  ExecuteInfo.lpVerb = lpOperation;
  ExecuteInfo.lpFile = lpFile;
  ExecuteInfo.lpParameters = lpParameters;
  ExecuteInfo.lpDirectory = lpDirectory;
  ExecuteInfo.nShow = nShowCmd;

  ShellExecuteURLExInternal(&ExecuteInfo);

  return ExecuteInfo.hInstApp;
}

// Note that we use ParseURL instead of more sophisticated functions
// like InternetCrackUrl because it's much more forgiving, and we only
// need to extract the scheme anyway

void Utils::ShowWebPage(const string& url)
{
  HINSTANCE hInst = ShellExecuteURL(nullptr, nullptr, StringUtil::UTF8ToWideChar(url).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
  if (reinterpret_cast<size_t>(hInst) <= 32)
  {
    MIKTEX_FATAL_ERROR_2(T_("The web browser could not be started."), "hInst", std::to_string(reinterpret_cast<size_t>(hInst)));
  }
}

#if !USE_CODECVT_UTF8
MIKTEXSTATICFUNC(wchar_t*) UTF8ToWideChar(const char* lpszUtf8, size_t& sizeWideChar, wchar_t* lpszWideChar)
{
  MIKTEX_ASSERT(Utils::IsUTF8(lpszUtf8));
  MIKTEX_ASSERT(sizeWideChar == 0 || lpszWideChar != nullptr);
  MIKTEX_ASSERT(sizeWideChar != 0 || lpszWideChar == nullptr || *lpszWideChar == 0);
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(lpszWideChar, sizeWideChar);
  if (*lpszUtf8 == 0)
  {
    if (lpszWideChar != nullptr && sizeWideChar > 0)
    {
      *lpszWideChar = 0;
    }
    sizeWideChar = 0;
    return lpszWideChar;
  }
  else if (sizeWideChar == 0)
  {
    int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, lpszUtf8, -1, nullptr, 0);
    if (len <= 0)
    {
      DWORD winError = GetLastError();
#if 0
      if (winError == ERROR_NO_UNICODE_TRANSLATION)
      {
        OutputDebugStringA("Bad UTF8ToWideChar() input:");
        OutputDebugStringA(lpszUtf8);
      }
#endif
      MIKTEX_FATAL_WINDOWS_RESULT("MultiByteToWideChar", winError);
    }
    sizeWideChar = len;
    return 0;
  }
  else
  {
    int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, lpszUtf8, -1, lpszWideChar, sizeWideChar);
    if (len <= 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("MultiByteToWideChar");
    }
    sizeWideChar = len;
    return lpszWideChar;
  }
}
#endif

#if !USE_CODECVT_UTF8
MIKTEXSTATICFUNC(char *) WideCharToUTF8(const wchar_t* lpszWideChar, size_t& sizeUtf8, char* lpszUtf8)
{
  MIKTEX_ASSERT(sizeUtf8 == 0 || lpszUtf8 != nullptr);
  MIKTEX_ASSERT(sizeUtf8 != 0 || lpszUtf8 == nullptr);
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(lpszUtf8, sizeUtf8);
  if (*lpszWideChar == 0)
  {
    if (lpszUtf8 != nullptr && sizeUtf8 > 0)
    {
      *lpszUtf8 = 0;
    }
    sizeUtf8 = 0;
    return lpszUtf8;
  }
  else if (sizeUtf8 == 0)
  {
    int len = WideCharToMultiByte(CP_UTF8, 0, lpszWideChar, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("WideCharToMultiByte");
    }
    sizeUtf8 = len;
    return 0;
  }
  else
  {
    int len = WideCharToMultiByte(CP_UTF8, 0, lpszWideChar, -1, lpszUtf8, sizeUtf8, nullptr, nullptr);
    if (len <= 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("WideCharToMultiByte");
    }
    sizeUtf8 = len;
    return lpszUtf8;
  }
}
#endif

#if !USE_CODECVT_UTF8
wstring Utils::UTF8ToWideChar(const char* lpszUtf8)
{
  size_t len = 0;
  ::UTF8ToWideChar(lpszUtf8, len, 0);
  CharBuffer<wchar_t, 200> buf(len);
  ::UTF8ToWideChar(lpszUtf8, len, buf.GetData());
  return buf.Get();
}
#endif

#if !USE_CODECVT_UTF8
string Utils::WideCharToUTF8(const wchar_t* lpszWideChar)
{
  size_t len = 0;
  ::WideCharToUTF8(lpszWideChar, len, 0);
  CharBuffer<char, 200> buf(len);
  ::WideCharToUTF8(lpszWideChar, len, buf.GetData());
  return buf.Get();
}
#endif

void Utils::CheckHeap()
{
  int heapStatus = _heapchk();
  switch (heapStatus)
  {
  case _HEAPOK:
    break;
  case _HEAPEMPTY:
    break;
  case _HEAPBADBEGIN:
    MIKTEX_INTERNAL_ERROR();
    break;
  case _HEAPBADNODE:
    MIKTEX_INTERNAL_ERROR();
    break;
  case _HEAPBADPTR:
    MIKTEX_INTERNAL_ERROR();
    break;
  }
#if defined(_DEBUG)
  if (_CrtCheckMemory() == 0)
  {
    MIKTEX_INTERNAL_ERROR();
  }
#endif
}

bool Utils::CheckPath(bool repair)
{
  constexpr wchar_t REGSTR_KEY_ENVIRONMENT_COMMON[] = L"System\\CurrentControlSet\\Control\\Session Manager\\Environment";
  constexpr wchar_t REGSTR_KEY_ENVIRONMENT_USER[] = L"Environment";

  shared_ptr<Session> session = MIKTEX_SESSION();

  wstring systemPath;
  DWORD systemPathType;

  if (!winRegistry::TryGetValue(HKEY_LOCAL_MACHINE, REGSTR_KEY_ENVIRONMENT_COMMON, L"Path", systemPath, systemPathType))
  {
    systemPath = L"";
    systemPathType = REG_SZ;
  }

  wstring userPath;
  DWORD userPathType;

  if (!session->IsAdminMode())
  {
    if (!winRegistry::TryGetValue(HKEY_CURRENT_USER, REGSTR_KEY_ENVIRONMENT_USER, L"Path", userPath, userPathType))
    {
      userPath = L"";
      userPathType = REG_SZ;
    }
  }

  PathName commonBinDir;
  string repairedSystemPath;
  bool systemPathCompetition = false;
  bool systemPathOkay = true;

  if (session->IsSharedSetup())
  {
    commonBinDir = session->GetSpecialPath(SpecialPath::CommonInstallRoot) / PathName(MIKTEX_PATH_BIN_DIR);
    systemPathOkay = !Directory::Exists(commonBinDir) || !FixProgramSearchPath(WU_(systemPath), commonBinDir, true, repairedSystemPath, systemPathCompetition);
  }

  bool repaired = false;
  bool userPathOkay = true;

  if (session->IsAdminMode())
  {
    if (!systemPathOkay && !repair)
    {
      SESSION_IMPL()->trace_error->WriteLine("core", TraceLevel::Error, T_("Something is wrong with the system PATH:"));
      SESSION_IMPL()->trace_error->WriteLine("core", TraceLevel::Error, WU_(systemPath));
    }
    else if (!systemPathOkay && repair)
    {
      SESSION_IMPL()->trace_error->WriteLine("core", TraceLevel::Error, T_("Setting new system PATH:"));
      SESSION_IMPL()->trace_error->WriteLine("core", TraceLevel::Error, repairedSystemPath);
      systemPath = UW_(repairedSystemPath);
      winRegistry::SetValue(HKEY_LOCAL_MACHINE, REGSTR_KEY_ENVIRONMENT_COMMON, L"Path", systemPath, systemPathType);
      systemPathOkay = true;
      repaired = true;
    }
  }
  else
  {
    if (!systemPathOkay && !systemPathCompetition)
    {
      string repairedUserPath;
      bool userPathCompetition;
      systemPathOkay = !FixProgramSearchPath(WU_(userPath), commonBinDir, true, repairedUserPath, userPathCompetition);
      if (!systemPathOkay && repair)
      {
        SESSION_IMPL()->trace_error->WriteLine("core", TraceLevel::Error, T_("Setting new user PATH:"));
        SESSION_IMPL()->trace_error->WriteLine("core", TraceLevel::Error, repairedUserPath);
        userPath = UW_(repairedUserPath);
        winRegistry::SetValue(HKEY_CURRENT_USER, REGSTR_KEY_ENVIRONMENT_USER, L"Path", userPath, userPathType);
        systemPathOkay = true;
        repaired = true;
      }
    }
    PathName userBinDir = session->GetSpecialPath(SpecialPath::UserInstallRoot) / PathName(MIKTEX_PATH_BIN_DIR);
    string repairedUserPath;
    bool userPathCompetition;
    userPathOkay = !Directory::Exists(userBinDir) || !FixProgramSearchPath(WU_(userPath), userBinDir, true, repairedUserPath, userPathCompetition);
    if (!userPathOkay && repair)
    {
      SESSION_IMPL()->trace_error->WriteLine("core", TraceLevel::Error, T_("Setting new user PATH:"));
      SESSION_IMPL()->trace_error->WriteLine("core", TraceLevel::Error, repairedUserPath);
      userPath = UW_(repairedUserPath);
      winRegistry::SetValue(HKEY_CURRENT_USER, REGSTR_KEY_ENVIRONMENT_USER, L"Path", userPath, userPathType);
      userPathOkay = true;
      repaired = true;
    }
  }

  if (repaired)
  {
    DWORD_PTR sendMessageResult;
    if (SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>(L"Environment"), SMTO_ABORTIFHUNG, 5000, &sendMessageResult) == 0)
    {
      if (::GetLastError() != ERROR_SUCCESS)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("SendMessageTimeoutW");
      }
    }
  }

  return repaired || (systemPathOkay && userPathOkay);
}

void Utils::RegisterShellFileAssoc(const string& extension, const string& progId, bool takeOwnership)
{
  shared_ptr<Session> session = MIKTEX_SESSION();
  PathName regPath("Software\\Classes");
  regPath /= extension;
  string otherProgId;
  bool haveOtherProgId = false;
  if (!session->IsAdminMode())
  {
    haveOtherProgId = winRegistry::TryGetValue(HKEY_CURRENT_USER, regPath.GetData(), "", otherProgId);
    haveOtherProgId = haveOtherProgId && StringCompare(progId.c_str(), otherProgId.c_str(), true) != 0;
  }
  if (!haveOtherProgId)
  {
    haveOtherProgId = winRegistry::TryGetValue(HKEY_LOCAL_MACHINE, regPath.GetData(), "", otherProgId);
    haveOtherProgId = haveOtherProgId && StringCompare(progId.c_str(), otherProgId.c_str(), true) != 0;
  }
  HKEY hkeyRoot = session->IsAdminMode() ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
  PathName openWithProgIds(regPath);
  openWithProgIds /= "OpenWithProgIds";
  if (haveOtherProgId)
  {
    winRegistry::SetValue(hkeyRoot, openWithProgIds.ToString(), otherProgId, "");
    winRegistry::SetValue(hkeyRoot, openWithProgIds.ToString(), progId, "");
  }
  if (!haveOtherProgId || takeOwnership)
  {
    if (haveOtherProgId)
    {
      winRegistry::SetValue(hkeyRoot, regPath.ToString(), "MiKTeX." MIKTEX_COM_MAJOR_MINOR_STR ".backup", otherProgId);
    }
    winRegistry::SetValue(hkeyRoot, regPath.ToString(), "", progId);
  }
}

void Utils::UnregisterShellFileAssoc(const string& extension, const string& progId)
{
  shared_ptr<Session> session = MIKTEX_SESSION();
  HKEY hkeyRoot = session->IsAdminMode() ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
  PathName regPath("Software\\Classes");
  regPath /= extension;
  string existingProgId;
  if (!winRegistry::TryGetValue(hkeyRoot, regPath.ToString(), "", existingProgId))
  {
    return;
  }
  string backupProgId;
  bool haveBackupProgId = winRegistry::TryGetValue(hkeyRoot, regPath.GetData(), "MiKTeX." MIKTEX_COM_MAJOR_MINOR_STR ".backup", backupProgId);
  if (haveBackupProgId || StringCompare(existingProgId.c_str(), progId.c_str(), true) != 0)
  {
    if (haveBackupProgId)
    {
      winRegistry::SetValue(hkeyRoot, regPath.GetData(), "", backupProgId.c_str());
      winRegistry::TryDeleteValue(hkeyRoot, regPath.GetData(), "MiKTeX." MIKTEX_COM_MAJOR_MINOR_STR ".backup");
    }
    PathName openWithProgIds(regPath);
    openWithProgIds /= "OpenWithProgIds";
    winRegistry::TryDeleteValue(hkeyRoot, openWithProgIds.ToString(), progId);
  }
  else
  {
    winRegistry::TryDeleteKey(hkeyRoot, regPath.ToString());
  }
}

void Utils::RegisterShellFileType(const string& progId, const string& userFriendlyName, const string& iconPath)
{
  shared_ptr<Session> session = MIKTEX_SESSION();
  HKEY hkeyRoot = session->IsAdminMode() ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
  PathName regPath("Software\\Classes");
  regPath /= progId;
  if (!userFriendlyName.empty())
  {
    winRegistry::SetValue(hkeyRoot, regPath.ToString(), "", userFriendlyName);
  }
  if (!iconPath.empty())
  {
    PathName defaultIcon(regPath);
    defaultIcon /= "DefaultIcon";
    winRegistry::SetValue(hkeyRoot, defaultIcon.GetData(), "", iconPath);
  }
}

void Utils::UnregisterShellFileType(const string& progId)
{
  shared_ptr<Session> session = MIKTEX_SESSION();
  HKEY hkeyRoot = session->IsAdminMode() ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
  PathName regPath("Software\\Classes");
  regPath /= progId;
  winRegistry::TryDeleteKey(hkeyRoot, regPath.ToString());
}

void Utils::RegisterShellVerb(const string& progId, const string& verb, const string& command, const string& ddeExec)
{
  shared_ptr<Session> session = MIKTEX_SESSION();
  HKEY hkeyRoot = session->IsAdminMode() ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
  PathName regPath("Software\\Classes");
  regPath /= progId;
  if (!command.empty())
  {
    PathName path(regPath);
    path /= "shell";
    path /= verb;
    path /= "command";
    winRegistry::SetValue(hkeyRoot, path.ToString(), "", command);
  }
  if (!ddeExec.empty())
  {
    PathName path(regPath);
    path /= "shell";
    path /= verb;
    path /= "ddeexec";
    winRegistry::SetValue(hkeyRoot, path.ToString(), "", ddeExec);
  }
}

string Utils::MakeProgId(const string& progId)
{
  return string("MiKTeX") + "." + progId + "." + MIKTEX_COM_MAJOR_MINOR_STR;
}

bool Utils::SupportsHardLinks(const PathName& path)
{
  DWORD fileSystemFlags;
  wchar_t fileSystemName[_MAX_PATH];
  PathName root = path.GetMountPoint();
  if (GetVolumeInformationW(root.ToWideCharString().c_str(), nullptr, 0, nullptr, nullptr, &fileSystemFlags, fileSystemName, _MAX_PATH) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("GetVolumeInformationW", "root", root.ToString());
  }
  if (WindowsVersion::IsWindows7OrGreater())
  {
    return (fileSystemFlags & FILE_SUPPORTS_HARD_LINKS) != 0;
  }
  else
  {
    return _wcsicmp(fileSystemName, L"NTFS") == 0;
  }
}

PathName Utils::GetExe()
{
    const size_t bufSize = 1024;
    wchar_t path[bufSize];
    DWORD n = GetModuleFileNameW(nullptr, path, bufSize);
    if (n == 0)
    {
        MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
    }
    if (n == bufSize)
    {
        BUF_TOO_SMALL();
    }
    return PathName(path);
}

string Utils::GetExeName()
{
    return GetExe().GetFileNameWithoutExtension().ToString();
}
