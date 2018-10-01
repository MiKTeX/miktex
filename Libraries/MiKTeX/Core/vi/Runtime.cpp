/* vi/Runtime.cpp:

   Copyright (C) 1996-2018 Christian Schenk

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

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <bzlib.h>
#include <lzma.h>
#include <uriparser/Uri.h>
#include <zlib.h>

#include "internal.h"

#include "miktex/Core/vi/Version.h"

using namespace MiKTeX::Core::vi;
using namespace MiKTeX::Core;
using namespace std;

string Runtime::GetName()
{
  return Header::GetName();
}

string Runtime::GetDescription()
{
  return Header::GetDescription();
}

int Runtime::GetInterfaceVersion()
{
  return Header::GetInterfaceVersion();
}

VersionNumber Runtime::GetVersion()
{
  return Header::GetVersion();
}

vector<LibraryVersion> Runtime::GetDependencies()
{
  vector<LibraryVersion> result;
  result.push_back(LibraryVersion("bzip2", "", BZ2_bzlibVersion()));
  result.push_back(LibraryVersion("liblzma", std::to_string(LZMA_VERSION), std::to_string(lzma_version_number())));
#if defined(USE_SYSTEM_OPENSSL_CRYPTO)
  result.push_back(LibraryVersion("openssl", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION)));
#elif defined(WITH_LIBRESSL_CRYPTO)
  result.push_back(LibraryVersion("libressl", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION)));
#endif
  result.push_back(LibraryVersion("uriparser", VersionNumber(URI_VER_MAJOR, URI_VER_MINOR, URI_VER_RELEASE, 0).ToString(), ""));
  result.push_back(LibraryVersion("zlib", ZLIB_VERSION, zlib_version));  
  return result;
}
