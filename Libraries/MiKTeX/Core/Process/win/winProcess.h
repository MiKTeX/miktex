/* winProcess.h: executing secondary process            -*- C++ -*-

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

#include <memory>

#include "miktex/Core/Process.h"
#include "miktex/Core/TemporaryFile.h"

#include "Utils/AutoEnv.h"

BEGIN_INTERNAL_NAMESPACE;

class winProcess :
  public MiKTeX::Core::Process
{
public:
  FILE* MIKTEXTHISCALL get_StandardInput() override;

public:
  FILE* MIKTEXTHISCALL get_StandardOutput() override;

public:
  FILE* MIKTEXTHISCALL get_StandardError() override;

public:
  void MIKTEXTHISCALL WaitForExit() override;

public:
  bool MIKTEXTHISCALL WaitForExit(int milliseconds) override;

public:
  int MIKTEXTHISCALL get_ExitCode() const override;

public:
  MiKTeX::Core::MiKTeXException MIKTEXTHISCALL get_Exception() const override;

public:
  void MIKTEXTHISCALL Close() override;

public:
  int MIKTEXTHISCALL GetSystemId() override;

public:
  std::unique_ptr<MiKTeX::Core::Process> MIKTEXTHISCALL get_Parent() override;

public:
  std::string MIKTEXTHISCALL get_ProcessName() override;

public:
  winProcess();

public:
  winProcess(const MiKTeX::Core::ProcessStartInfo& startinfo);

public:
  virtual ~winProcess();

private:
  void Create();

private:
  MiKTeX::Core::ProcessStartInfo startinfo;

private:
  PROCESS_INFORMATION processInformation;

private:
  HANDLE standardInput = INVALID_HANDLE_VALUE;
  HANDLE standardOutput = INVALID_HANDLE_VALUE;
  HANDLE standardError = INVALID_HANDLE_VALUE;

private:
  FILE* pFileStandardInput = nullptr;
  FILE* pFileStandardOutput = nullptr;
  FILE* pFileStandardError = nullptr;

private:
  bool processStarted = false;

private:
  PROCESSENTRY32W processEntry;

private:
  std::unique_ptr<MiKTeX::Core::TemporaryFile> tmpFile;

private:
  AutoEnv tmpEnv;

private:
  static PROCESSENTRY32W GetProcessEntry(DWORD processId);

private:
  static bool TryGetProcessEntry(DWORD processId, PROCESSENTRY32W& processEntry);

private:
  friend class MiKTeX::Core::Process;
};

END_INTERNAL_NAMESPACE;
