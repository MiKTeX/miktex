/* miktex/TeXAndFriends/ETeXApp.h:                      -*- C++ -*-

   Copyright (C) 1996-2017 Christian Schenk

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

#include <memory>
#include <string>

#include "TeXApp.h"

MIKTEXMF_BEGIN_NAMESPACE;

class MIKTEXMFTYPEAPI(ETeXApp) :
  public TeXApp
{
public:
  MIKTEXMFEXPORT MIKTEXTHISCALL ETeXApp();

public:
  ETeXApp(const ETeXApp& other) = delete;

public:
  ETeXApp& operator=(const ETeXApp& other) = delete;

public:
  ETeXApp(ETeXApp&& other) = delete;

public:
  ETeXApp& operator=(ETeXApp&& other) = delete;

public:
  virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~ETeXApp() noexcept;

public:
  static ETeXApp* GetETeXApp()
  {
    MIKTEX_ASSERT(dynamic_cast<ETeXApp*>(Application::GetApplication()) != nullptr);
    return (ETeXApp*)Application::GetApplication();
  }

protected:
  MIKTEXMFTHISAPI(void) Init(const std::string& programInvocationName) override;

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

protected:
  MIKTEXMFTHISAPI(void) AddOptions() override;

protected:
  MIKTEXMFTHISAPI(bool) ProcessOption(int c, const std::string& optArg) override;

public:
  MIKTEXMFTHISAPI(void) OnTeXMFStartJob() override;

public:
  MIKTEXMFTHISAPI(bool) ETeXP() const;

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

inline bool miktexetexp()
{
  return ETeXApp::GetETeXApp()->ETeXP();
}

MIKTEXMF_END_NAMESPACE;
