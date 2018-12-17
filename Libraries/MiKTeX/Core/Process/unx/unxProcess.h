/* unxProcess.h:                                        -*- C++ -*-

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

#pragma once

#if !defined(B6278A08DFEE4038A08449DD17C4E3D3)
#define B6278A08DFEE4038A08449DD17C4E3D3

#include <memory>

#include "miktex/Core/Process.h"
#include "miktex/Core/TemporaryFile.h"

#include "Utils/AutoEnv.h"

BEGIN_INTERNAL_NAMESPACE;

class unxProcess :
  public MiKTeX::Core::Process
{
public:
  FILE* get_StandardInput() override;

public:
  FILE* get_StandardOutput() override;

public:
  FILE* get_StandardError() override;

public:
  void WaitForExit() override;

public:
  bool WaitForExit(int milliseconds) override;

public:
  int get_ExitCode() const override;

public:
  bool MIKTEXTHISCALL get_Exception(MiKTeX::Core::MiKTeXException& ex) const override;

public:
  void Close() override;

public:
  int GetSystemId() override;

public:
  std::unique_ptr<MiKTeX::Core::Process> get_Parent() override;

public:
  std::string get_ProcessName() override;
  
public:
  MiKTeX::Core::ProcessInfo GetProcessInfo() override;

public:
  unxProcess()
  {
  }
  
public:
  unxProcess(const MiKTeX::Core::ProcessStartInfo& startinfo);

public:
  ~unxProcess() override;

private:
  void Create();

private:
  MiKTeX::Core::ProcessStartInfo startinfo;

private:
  int status;

private:
  pid_t pid = -1;

private:
  int fdStandardInput = -1;
  int fdStandardOutput = -1;
  int fdStandardError = -1;

private:
  FILE* pFileStandardInput = nullptr;
  FILE* pFileStandardOutput = nullptr;
  FILE* pFileStandardError = nullptr;

private:
  std::unique_ptr<MiKTeX::Core::TemporaryFile> tmpFile;

private:
  AutoEnv tmpEnv;

private:
  friend class MiKTeX::Core::Process;
};

END_INTERNAL_NAMESPACE;

#endif
