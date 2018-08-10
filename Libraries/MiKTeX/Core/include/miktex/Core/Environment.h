/* miktex/Core/Environment.h:                           -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(B644C2380CA33546A66551BC11D5194C)
#define B644C2380CA33546A66551BC11D5194C

#define MIKTEX_ENV_PREFIX "MIKTEX"
#define MIKTEX_ENV_PREFIX_ MIKTEX_ENV_PREFIX "_"

#define MIKTEX_ENV_BIN_DIR MIKTEX_ENV_PREFIX_ "BINDIR"
#define MIKTEX_ENV_COMMON_CONFIG MIKTEX_ENV_PREFIX_ "COMMONCONFIG"
#define MIKTEX_ENV_COMMON_DATA MIKTEX_ENV_PREFIX_ "COMMONDATA"
#define MIKTEX_ENV_COMMON_INSTALL MIKTEX_ENV_PREFIX_ "COMMONINSTALL"
#define MIKTEX_ENV_COMMON_ROOTS MIKTEX_ENV_PREFIX_ "COMMONROOTS"
#define MIKTEX_ENV_COMMON_STARTUP_FILE MIKTEX_ENV_PREFIX_ "COMMONSTARTUPFILE"
#define MIKTEX_ENV_CWD_LIST MIKTEX_ENV_PREFIX_ "CWDLIST"
#define MIKTEX_ENV_EXCEPTION_PATH MIKTEX_ENV_PREFIX_ "EXCEPTION_PATH"
#define MIKTEX_ENV_OTHER_COMMON_ROOTS MIKTEX_ENV_PREFIX_ "OTHERCOMMONROOTS"
#define MIKTEX_ENV_OTHER_USER_ROOTS MIKTEX_ENV_PREFIX_ "OTHERUSERROOTS"
#define MIKTEX_ENV_PACKAGE_LIST_FILE MIKTEX_ENV_PREFIX_ "PKGLISTFILE"
#define MIKTEX_ENV_REPOSITORY MIKTEX_ENV_PREFIX_ "REPOSITORY"
#define MIKTEX_ENV_TRACE MIKTEX_ENV_PREFIX_ "TRACE"
#define MIKTEX_ENV_USER_CONFIG MIKTEX_ENV_PREFIX_ "USERCONFIG"
#define MIKTEX_ENV_USER_DATA MIKTEX_ENV_PREFIX_ "USERDATA"
#define MIKTEX_ENV_USER_INSTALL MIKTEX_ENV_PREFIX_ "USERINSTALL"
#define MIKTEX_ENV_USER_ROOTS MIKTEX_ENV_PREFIX_ "USERROOTS"
#define MIKTEX_ENV_USER_STARTUP_FILE MIKTEX_ENV_PREFIX_ "USERSTARTUPFILE"

#endif
