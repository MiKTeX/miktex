/* miktex/Core/Uri.h:                                   -*- C++ -*-

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

#if !defined(EFEE2F6C2B524088BC8A0490CBAD6BD9)
#define EFEE2F6C2B524088BC8A0490CBAD6BD9

#include <miktex/Core/config.h>

#include <string>

MIKTEX_CORE_BEGIN_NAMESPACE;

class Uri
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL Uri();

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL Uri(const Uri & other);

public:
  virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~Uri();

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL Uri(const char * lpszUri);

public:
  MIKTEXCORETHISAPI(Uri &) operator= (const Uri & other);

public:
  MIKTEXCORETHISAPI(std::string) GetHost() const;

public:
  MIKTEXCORETHISAPI(int) GetPort() const;

public:
  MIKTEXCORETHISAPI(std::string) GetUserInfo() const;

public:
  MIKTEXCORETHISAPI(std::string) GetScheme() const;

private:
  void * p = nullptr;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
