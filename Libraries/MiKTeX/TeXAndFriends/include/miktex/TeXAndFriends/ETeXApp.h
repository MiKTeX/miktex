/* miktex/TeXAndFriends/ETeXApp.h:                      -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#include <miktex/TeXAndFriends/config.h>

#include <string>

#include "TeXApp.h"

MIKTEXMF_BEGIN_NAMESPACE;

class MIKTEXMFTYPEAPI(ETeXApp) : public TeXApp
{
public:
  MIKTEXMFEXPORT MIKTEXTHISCALL ETeXApp();

protected:
  MIKTEXMFTHISAPI(void) Init(const char * lpszProgramInvocationName) override;

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

protected:
  MIKTEXMFTHISAPI(void) AddOptions() override;

protected:
  MIKTEXMFTHISAPI(bool) ProcessOption(int c, const std::string & optArg) override;

public:
  MIKTEXMFTHISAPI(void) OnTeXMFStartJob() override;

#if defined(MIKTEX_DEBUG)
public:
  void CheckMemory() override
  {
#if defined(THEDATA)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(eofseen));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(grpstack));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(ifstack));
#endif
    TeXMFApp::CheckMemory();
  }
#endif

#if defined(THEDATA)
public:
  void AllocateMemory()
  {
    TeXApp::AllocateMemory();

    Allocate("eofseen", THEDATA(eofseen), THEDATA(maxinopen));
    Allocate("grpstack", THEDATA(grpstack), THEDATA(maxinopen));
    Allocate("ifstack", THEDATA(ifstack), THEDATA(maxinopen));
  }
#endif

#if defined(THEDATA)
public:
  void FreeMemory()
  {
    TeXApp::FreeMemory();

    Free("eofseen", THEDATA(eofseen));
    Free("grpstack", THEDATA(grpstack));
    Free("ifstack", THEDATA(ifstack));
  }
#endif

public:
  bool ETeXP() const
  {
    return enableETeX;
  }

private:
  bool enableETeX;

private:
  int optBase;
};

MIKTEXMF_END_NAMESPACE;
