/* miktex/TeXAndFriends/MetafontApp.h:                  -*- C++ -*-

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

#if !defined(DE6D6CCC880D4B48BF618E37BDAF1DBB)
#define DE6D6CCC880D4B48BF618E37BDAF1DBB

#include <miktex/TeXAndFriends/config.h>

#include <memory>
#include <string>

#include <miktex/Core/FileType>

#include "TeXMFApp.h"

namespace mfapp {
#include <miktex/mfapp.defaults.h>
}

MIKTEXMF_BEGIN_NAMESPACE;

class MIKTEXMFTYPEAPI(MetafontApp) :
  public TeXMFApp
{
public:
  MIKTEXMFEXPORT MIKTEXTHISCALL MetafontApp();

public:
  MetafontApp(const MetafontApp& other) = delete;

public:
  MetafontApp& operator=(const MetafontApp& other) = delete;

public:
  MetafontApp(MetafontApp&& other) = delete;

public:
  MetafontApp& operator=(MetafontApp&& other) = delete;

public:
  virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~MetafontApp() noexcept;

protected:
  MIKTEXMFTHISAPI(void) Init(const std::string& programInvocationName) override;

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

protected:
  MIKTEXMFTHISAPI(void) AddOptions() override;

protected:
  MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string& optArg) override;

public:
  MiKTeX::Core::FileType GetInputFileType() const override
  {
    return MiKTeX::Core::FileType::MF;
  }

public:
  std::string GetMemoryDumpFileExtension() const override
  {
    return ".base";
  }

public:
  MiKTeX::Core::FileType GetMemoryDumpFileType() const override
  {
    return MiKTeX::Core::FileType::BASE;
  }

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEXMF_END_NAMESPACE;

#endif
