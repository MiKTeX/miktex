/* miktex.cpp:

   Copyright (C) 2020 Christian Schenk

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
   USA.  */

#include <mutex>

#include <miktex/Core/Process>

#include "lua.h"

using namespace std;
using namespace MiKTeX::Core;

int miktex_system(const char* commandLine)
{
  if (commandLine == nullptr)
  {
    return 1;
  }
  try
  {
    int exitCode;
    if (Process::ExecuteSystemCommand(commandLine, &exitCode))
    {
      return exitCode;
    }
    else
    {
      return -1;
    }
  }
  catch (const MiKTeXException&)
  {
    return -1;
  }
}

unordered_map<FILE*, unique_ptr<Process>> processes;
mutex mux;

FILE* miktex_popen(const char* commandLine, const char* mode)
{
  try
  {
    FILE* file = nullptr;
    unique_ptr<Process> process;
    if (strcmp(mode, "r") == 0)
    {
      process = Process::StartSystemCommand(commandLine, nullptr, &file);
    }
    else
    {
      process = Process::StartSystemCommand(commandLine, &file, nullptr);
    }
    lock_guard<mutex> lockGuard(mux);
    processes[file] = std::move(process);
    return file;
  }
  catch (const MiKTeXException&)
  {
    return nullptr;
  }
}

int miktex_pclose(FILE* file)
{
  try
  {
    lock_guard<mutex> lockGuard(mux);
    auto it = processes.find(file);
    if (it == processes.end())
    {
      return -1;
    }
    it->second->WaitForExit();
    fclose(file);
    int exitCode = it->second->get_ExitCode();
    it->second->Close();
    processes.erase(it);
    return exitCode;
  }
  catch (const MiKTeXException&)
  {
    return -1;
  }
}
