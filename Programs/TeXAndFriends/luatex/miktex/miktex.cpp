/* luatex/miktex/miktex.cpp:

   Copyright (C) 2016-2021 Christian Schenk

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

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/App/Application>
#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/KPSE/Emulation>
#include <miktex/Util/PathNameUtil>

#include "luatex.h"

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

using namespace MiKTeX::App;
using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

extern "C" {
  extern int lua_only;
  extern int restrictedshell;
  extern int shellenabledp;
}

void miktex_enable_installer(int enable)
{
  Application::GetApplication()->EnableInstaller(enable ? TriState::True : TriState::False);
}

int miktex_open_format_file(const char* fileNameArg, FILE** ppFile, int renew)
{
  MIKTEX_ASSERT_STRING(fileNameArg);
  MIKTEX_ASSERT(ppFile != nullptr);

  shared_ptr<Session> session = Application::GetApplication()->GetSession();

  PathName fileName(fileNameArg);
  if (!fileName.HasExtension())
  {
    fileName.SetExtension(".fmt");
  }

  Session::FindFileOptionSet findFileOptions;
  findFileOptions += Session::FindFileOption::Create;
  if (renew)
  {
    findFileOptions += Session::FindFileOption::Renew;
  }

  PathName found;
  if (!session->FindFile(fileName.ToString(), FileType::FMT, findFileOptions, found))
  {
    MIKTEX_FATAL_ERROR_2(T_("The memory dump file could not be found."), "fileName", fileName.ToString());
  }

#if 1
  if (!renew)
  {
    time_t modificationTime = File::GetLastWriteTime(found);
    time_t lastAdminMaintenance = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_ADMIN_MAINTENANCE, ConfigValue("0")).GetTimeT();
    renew = lastAdminMaintenance > modificationTime;
    if (!renew && !session->IsAdminMode())
    {
      time_t lastUserMaintenance = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_USER_MAINTENANCE, ConfigValue("0")).GetTimeT();
      renew = lastUserMaintenance > modificationTime;
    }
    if (renew)
    {
      // RECURSION
      return miktex_open_format_file(fileNameArg, ppFile, true);
    }
  }
#endif

  *ppFile = session->OpenFile(found, FileMode::Open, FileAccess::Read, false);

  std::string dumpName = fileName.GetFileNameWithoutExtension().ToString();
  session->PushAppName(dumpName);

  return true;
}

FILE* miktex_open_output_file(const char* fileName)
{
  MIKTEX_ASSERT_STRING(fileNameArg);
  shared_ptr<Session> session = Application::GetApplication()->GetSession();
  return session->TryOpenFile(PathName(fileName), FileMode::Open, FileAccess::Write, false);
}

int miktex_is_output_file(const char* pathArg)
{
  PathName path(pathArg);
  if (path.HasExtension(".gz"))
  {
    path.SetExtension(nullptr);
  }
  return path.HasExtension(".dvi")
    || path.HasExtension(".pdf")
    || path.HasExtension(".synctex");
}

PathName auxDirectory;

void miktex_set_aux_directory(const char* path)
{
  auxDirectory = path;
  auxDirectory.MakeFullyQualified();
  shared_ptr<Session> session = Application::GetApplication()->GetSession();
  if (!Directory::Exists(auxDirectory))
  {
    if (session->GetConfigValue(MIKTEX_CONFIG_SECTION_TEXANDFRIENDS, MIKTEX_CONFIG_VALUE_CREATEAUXDIRECTORY).GetString() == "t")
    {
      Directory::Create(auxDirectory);
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("The specified auxiliary directory does not exist."), "directory", auxDirectory.ToString());
    }
  }
  session->AddInputDirectory(auxDirectory, true);
}

const char* miktex_get_aux_directory()
{
  if (auxDirectory.Empty())
  {
    return nullptr;
  }
  else
  {
    return auxDirectory.GetData();
  }
}

void miktex_invoke_editor(const char* filename, int lineno)
{
  Application::GetApplication()->InvokeEditor(PathName(filename), lineno, FileType::TEX, PathName());
}

void miktex_show_library_versions()
{
  Application::GetApplication()->ShowLibraryVersions();
}

void miktex_add_include_directory(const char* path)
{
  if (Directory::Exists(PathName(path)))
  {
    PathName includeDirectory(path);
    includeDirectory.MakeFullyQualified();
    shared_ptr<Session> session = Application::GetApplication()->GetSession();
    session->AddInputDirectory(includeDirectory, true);
  }
}

int miktex_is_fully_qualified_path(const char* path)
{
  return PathNameUtil::IsFullyQualifiedPath(path);
}

void miktex_convert_to_unix(char* path)
{
  for (char* ch = path; *ch != 0; ++ch)
  {
    if (*ch == '\\')
    {
      *ch = '/';
    }
  }
}

int miktex_emulate__shell_cmd_is_allowed(const char* commandLine, char** safeCommandLineRet, char** examinedCommandRet)
{
  shared_ptr<Session> session = Application::GetApplication()->GetSession();
  Session::ExamineCommandLineResult examineResult;
  std::string examinedCommand;
  std::string safeCommandLine;
  tie(examineResult, examinedCommand, safeCommandLine) = session->ExamineCommandLine(commandLine);
  *safeCommandLineRet = xstrdup(safeCommandLine.c_str());
  *examinedCommandRet = xstrdup(examinedCommand.c_str());
  switch (examineResult)
  {
  case Session::ExamineCommandLineResult::SyntaxError:
    return -1;
  case Session::ExamineCommandLineResult::ProbablySafe:
    return 2;
  default:
    return 0;
  }
}

int miktex_allow_unrestricted_shell_escape()
{
  shared_ptr<Session> session = Application::GetApplication()->GetSession();
  bool okay = lua_only || !session->RunningAsAdministrator() ||
    session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_ALLOW_UNRESTRICTED_SUPER_USER).GetBool();
  return okay ? 1 : 0;
}

int miktex_emulate__spawn_command(const char* fileName, char* const* argv, char* const* env)
{
  vector<std::string> arguments;
  for (; *argv != nullptr; ++argv)
  {
    arguments.push_back(*argv);
  }
  try
  {
    MIKTEX_EXPECT(env == nullptr);
    Process::Start(PathName(fileName), arguments);
    return 0;
  }
  catch (const MiKTeXException&)
  {
    errno = ENOENT;
    return -1;
  }
}

int miktex_emulate__exec_command(const char* fileName, char* const* argv, char* const* env)
{
  vector<std::string> arguments;
  for (; *argv != nullptr; ++argv)
  {
    arguments.push_back(*argv);
  }
  try
  {
    MIKTEX_EXPECT(env == nullptr);
    Process::Overlay(PathName(fileName), arguments);
    errno = ENOENT;
    return -1;
  }
  catch (const MiKTeXException&)
  {
    errno = ENOENT;
    return -1;
  }
}

char** miktex_emulate__do_split_command(const char* commandLine, char** argv0Ret)
{
  Argv argv(commandLine);
  MIKTEX_EXPECT(argv0Ret != nullptr);
  *argv0Ret = xstrdup(argv[0]);
  char** result = reinterpret_cast<char**>(xmalloc(sizeof(char*) * (static_cast<size_t>(argv.GetArgc()) + 1)));
  for (int idx = 0; idx < argv.GetArgc(); ++idx)
  {
    result[idx] = xstrdup(argv[idx]);
  }
  result[argv.GetArgc()] = nullptr;
  return result;
}

FILE* miktex_emulate__runpopen(const char* commandLineArg, const char* mode)
{
  MIKTEX_EXPECT(shellenabledp);
  std::string commandLine = commandLineArg;
#if defined(MIKTEX_WINDOWS)
  std::replace(commandLine.begin(), commandLine.end(), '\'', '"');
#endif
  Application* app = Application::GetApplication();
  shared_ptr<Session> session = app->GetSession();
  Session::ExamineCommandLineResult examineResult;
  std::string examinedCommand;
  std::string safeCommandLine;
  tie(examineResult, examinedCommand, safeCommandLine) = session->ExamineCommandLine(commandLine);
  if (examineResult == Session::ExamineCommandLineResult::SyntaxError)
  {
    app->LogError(fmt::format("syntax error: {0}", commandLineArg));
    return nullptr;
  }
  if (examineResult != Session::ExamineCommandLineResult::ProbablySafe && examineResult != Session::ExamineCommandLineResult::MaybeSafe)
  {
    app->LogError(fmt::format("command is unsafe: {0}", commandLineArg));
    return nullptr;
  }
  std::string toBeExecuted;
  if (restrictedshell)
  {
    if (examineResult != Session::ExamineCommandLineResult::ProbablySafe)
    {
      app->LogError(fmt::format("command not allowed: {0}", commandLineArg));
      return nullptr;
    }
    toBeExecuted = safeCommandLine;
  }
  else
  {
    if (session->RunningAsAdministrator() && !session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_ALLOW_UNRESTRICTED_SUPER_USER).GetBool())
    {
      app->LogError(fmt::format("not allowed with elevated privileges: {0}", commandLineArg));
      return nullptr;
    }
    toBeExecuted = commandLine;
  }
  FileAccess access;
  if (mode == "w"s)
  {
    access = FileAccess::Write;
  }
  else if (mode == "r"s)
  {
    access = FileAccess::Read;
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
  if (examineResult == Session::ExamineCommandLineResult::ProbablySafe)
  {
    app->LogInfo(fmt::format("initiating restricted {0} pipe stream: {1}", access == FileAccess::Read ? "input"s : "output"s, toBeExecuted));
  }
  else
  {
    app->LogWarn(fmt::format("initiating unrestricted {0} pipe stream: {1}", access == FileAccess::Read ? "input"s : "output"s, toBeExecuted));
  }
  try
  {
    return session->OpenFile(PathName(toBeExecuted), FileMode::Command, access, true);
  }
  catch (const MiKTeXException&)
  {
    return nullptr;
  }
}

int miktex_is_pipe(FILE* file)
{
  Application* app = Application::GetApplication();
  shared_ptr<Session> session = app->GetSession();
  auto openFileInfo = session->TryGetOpenFileInfo(file);
  return openFileInfo.first && openFileInfo.second.mode == FileMode::Command ? 1 : 0;
}

void miktex_emulate__close_file_or_pipe(FILE* file)
{
  if (file == nullptr)
  {
    return;
  }
  Application* app = Application::GetApplication();
  shared_ptr<Session> session = app->GetSession();
  auto openFileInfo = session->TryGetOpenFileInfo(file);
  if (openFileInfo.first)
  {
    session->CloseFile(file);
  }
  else
  {
    fclose(file);
  }
}

#if defined(MIKTEX_WINDOWS)
char* miktex_wchar_to_utf8(const wchar_t* w)
{
  return xstrdup(StringUtil::WideCharToUTF8(w).c_str());
}
#endif

#if defined(MIKTEX_WINDOWS)
void miktex_copy_wchar_to_utf8(char* dest, size_t destSize, const wchar_t* source)
{
  StringUtil::CopyString(dest, destSize, source);
}
#endif

inline std::string GetBanner(const char* name, const char* version)
{
  return fmt::format("This is {0}, Version {1} ({2})", name, version, Utils::GetMiKTeXBannerString());
}

char* miktex_banner(const char* name, const char* version)
{
  return xstrdup(GetBanner(name, version).c_str());
}

void miktex_print_banner(FILE* file, const char* name, const char* version)
{
  fputs(GetBanner(name, version).c_str(), file);
}
