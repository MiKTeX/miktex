/* miktex/Core/Registry.h: MiKTeX settings              -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#include <miktex/Version>

#if !defined(A5D53972CA7CCD40B4DB3ABBC0D957AF)
#define A5D53972CA7CCD40B4DB3ABBC0D957AF

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_REGPATH_COMPANY                \
  "Software\\MiKTeX.org"
#  define MIKTEX_REGPATH_PRODUCT                \
  "Software\\MiKTeX.org\\MiKTeX"
#  define MIKTEX_REGPATH_SERIES                         \
  "Software\\MiKTeX.org\\MiKTeX\\" MIKTEX_SERIES_STR
#  define MIKTEX_REGPATH_PRODUCT_MIKTEXDIRECT   \
  "Software\\MiKTeX.org\\MiKTeXDirect"
#  define MIKTEX_REGPATH_SERIES_MIKTEXDIRECT    \
  "Software\\MiKTeX.org\\MiKTeXDirect\\"  MIKTEX_SERIES_STR
#endif

#define MIKTEX_GPL_GHOSTSCRIPT                  \
  "Software\\MiKTeX GPL Ghostscript"

#define MIKTEX_REGKEY_BIBTEX "BibTeX"
#define MIKTEX_REGKEY_CORE "Core"
#define MIKTEX_REGKEY_MAKEBASE "MakeBase"
#define MIKTEX_REGKEY_MAKEFMT "MakeFMT"
#define MIKTEX_REGKEY_MAKEMEM "MakeMEM"
#define MIKTEX_REGKEY_MAKEPK "MakePk"
#define MIKTEX_REGKEY_MAKETFM "MakeTFM"
#define MIKTEX_REGKEY_METAPOST "MetaPost"
#define MIKTEX_REGKEY_MIGRATE "Migrate"
#define MIKTEX_REGKEY_PACKAGE_MANAGER "MPM"
#define MIKTEX_REGKEY_SETTINGS "Settings"
#define MIKTEX_REGKEY_TEXMF "TeXMF"
#define MIKTEX_REGKEY_YAP "Yap"
#define MIKTEX_REGKEY_YAP_SETTINGS \
  MIKTEX_REGKEY_YAP "\\" MIKTEX_REGKEY_SETTINGS

#define MIKTEX_REGVAL_ALLOWED_SHELL_COMMANDS "AllowedShellCommands"
#define MIKTEX_REGVAL_AUTO_ADMIN "AutoAdmin"
#define MIKTEX_REGVAL_AUTO_INSTALL "AutoInstall"
#define MIKTEX_REGVAL_COMMON_CONFIG "CommonConfig"
#define MIKTEX_REGVAL_COMMON_DATA "CommonData"
#define MIKTEX_REGVAL_COMMON_INSTALL "CommonInstall"
#define MIKTEX_REGVAL_COMMON_ROOTS "CommonRoots"
#define MIKTEX_REGVAL_CREATE_AUX_DIRECTORY "CreateAuxDirectory"
#define MIKTEX_REGVAL_CREATE_OUTPUT_DIRECTORY "CreateOutputDirectory"
#define MIKTEX_REGVAL_C_STYLE_ERRORS "CStyleErrors"
#define MIKTEX_REGVAL_DESTDIR "DestDir"
#define MIKTEX_REGVAL_DOC_EXTENSIONS "DocExtensions"
#define MIKTEX_REGVAL_EDITOR "Editor"
#define MIKTEX_REGVAL_ENABLE_PIPES "EnablePipes"
#define MIKTEX_REGVAL_ENABLE_WRITE18 "EnableWrite18"
#define MIKTEX_REGVAL_FORCE_LOCAL_SERVER "ForceLocalServer"
#define MIKTEX_REGVAL_FTP_MODE "FTPMode"
#define MIKTEX_REGVAL_GUI_FRAMEWORK "GUIFramework"
#define MIKTEX_REGVAL_INSIST_ON_REPAIR "InsistOnRepair"
#define MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE "LastAdminMaintenance"
#define MIKTEX_REGVAL_LAST_USER_MAINTENANCE "LastUserMaintenance"
#define MIKTEX_REGVAL_USERINFO_FILE "UserInfoFile"
#define MIKTEX_REGVAL_LOCAL_REPOSITORY "LocalRepository"
#define MIKTEX_REGVAL_MAX_REDIRECTS "MaxRedirects"
#define MIKTEX_REGVAL_MIKTEXDIRECT_ROOT "MiKTeXDirectRoot"
#define MIKTEX_REGVAL_NO_REGISTRY "NoRegistry"
#define MIKTEX_REGVAL_PARSE_FIRST_LINE "ParseFirstLine"
#define MIKTEX_REGVAL_PK_FN_TEMPLATE "PKFnTemplate"
#define MIKTEX_REGVAL_PROXY_AUTH_REQ "ProxyAuthReq"
#define MIKTEX_REGVAL_PROXY_HOST "ProxyHost"
#define MIKTEX_REGVAL_PROXY_PORT "ProxyPort"
#define MIKTEX_REGVAL_REMOTE_REPOSITORY "RemoteRepository"
#define MIKTEX_REGVAL_REPOSITORY_RELEASE_STATE "RepositoryReleaseState"
#define MIKTEX_REGVAL_RENEW_FORMATS_ON_UPDATE "RenewFormatsOnUpdate"
#define MIKTEX_REGVAL_REPOSITORY_TYPE "RepositoryType"
#define MIKTEX_REGVAL_SHARED_SETUP "SharedSetup"
#define MIKTEX_REGVAL_STARTUP_FILE "StartupFile"
#define MIKTEX_REGVAL_TEMPDIR "TempDir"
#define MIKTEX_REGVAL_TIMESTAMP "TimeStamp"
#define MIKTEX_REGVAL_TRACE "Trace"
#define MIKTEX_REGVAL_USER_CONFIG "UserConfig"
#define MIKTEX_REGVAL_USER_DATA "UserData"
#define MIKTEX_REGVAL_USER_INSTALL "UserInstall"
#define MIKTEX_REGVAL_USER_ROOTS "UserRoots"
#define MIKTEX_REGVAL_USE_PROXY "UseProxy"
#define MIKTEX_REGVAL_VERSION "Version"

#endif
