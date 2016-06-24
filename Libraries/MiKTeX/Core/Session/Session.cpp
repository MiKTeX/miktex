/* Session.cpp: session initialization

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

#include "StdAfx.h"

#include "internal.h"

#include "core-version.h"

#include "miktex/Core/CSVList.h"
#include "miktex/Core/Environment.h"
#include "miktex/Core/Paths.h"
#include "miktex/Core/Registry.h"

#include "Session/SessionImpl.h"

#if defined(MIKTEX_WINDOWS)
#include "win/winRegistry.h"
#endif

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace std;

weak_ptr<SessionImpl> SessionImpl::theSession;

shared_ptr<Session> Session::Create(const Session::InitInfo & initInfo)
{
  if (!SessionImpl::theSession.expired())
  {
    MIKTEX_UNEXPECTED();
  }
  shared_ptr<SessionImpl> session = make_shared<SessionImpl>();
  SessionImpl::theSession = session;
  session->Initialize(initInfo);
  return session;
}

Session::~Session()
{
}

shared_ptr<Session> Session::Get()
{
  return SessionImpl::GetSession();
}

shared_ptr<Session> Session::TryGet()
{
  return SessionImpl::theSession.lock();
}

SessionImpl::SessionImpl() :
  // passing an empty string to the locale constructor is ok; it
  // means: "the user's preferred locale" (cf. "The C++ Programming
  // Language, Appendix D: Locales")
#if !defined(__MINGW32__)
  defaultLocale("")
#else
  // FIXME: work around MingW bug
  defaultLocale()
#endif
{
}

SessionImpl::~SessionImpl()
{
  try
  {
    Uninitialize();
  }
  catch (const exception &)
  {
  }
}

void SessionImpl::Initialize(const Session::InitInfo & initInfo)
{
  string val;

  PathName programInvocationName = initInfo.GetProgramInvocationName();
  programInvocationName = programInvocationName.GetFileNameWithoutExtension();
  const char * lpsz = strstr(programInvocationName.Get(), MIKTEX_ADMIN_SUFFIX);
  bool forceAdminMode = lpsz != nullptr && strlen(lpsz) == strlen(MIKTEX_ADMIN_SUFFIX);

#if defined(MIKTEX_WINDOWS)
  if (!forceAdminMode)
  {
    programInvocationName = GetMyProgramFile(false).TransformForComparison().GetFileNameWithoutExtension().Get();
    lpsz = strstr(programInvocationName.Get(), MIKTEX_ADMIN_SUFFIX);
    forceAdminMode = lpsz != nullptr && strlen(lpsz) == strlen(MIKTEX_ADMIN_SUFFIX);
  }
#endif

  bool oldAdminMode = adminMode;

  if (forceAdminMode)
  {
    adminMode = true;
  }
  else
  {
    adminMode = initInfo.GetOptions()[InitOption::AdminMode];
  }

#if defined(_MSC_VER)
  if (Utils::GetEnvironmentString("MIKTEX_DEBUG_ON_STD_EXCEPTION", val))
  {
    debugOnStdException = std::stoi(val);
  }
#endif

#if defined(MIKTEX_WINDOWS)
  if (initInfo.GetOptions()[InitOption::InitializeCOM])
  {
    MyCoInitialize();
  }
#endif

  Botan::LibraryInitializer::initialize();

  initialized = true;

  this->initInfo = initInfo;

  theNameOfTheGame = initInfo.GetTheNameOfTheGame();

  RegisterLibraryTraceStreams();

  // enable trace streams
  string traceOptions;
  traceOptions = initInfo.GetTraceFlags();
  if (traceOptions.empty())
  {
    Utils::GetEnvironmentString(MIKTEX_ENV_TRACE, traceOptions);
  }
#if defined(MIKTEX_WINDOWS)
  if (traceOptions.empty() && (!initInfo.GetOptions()[InitOption::NoConfigFiles]))
  {
    if (!winRegistry::TryGetRegistryValue(TriState::False, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_TRACE, traceOptions))
    {
      traceOptions = "";
    }
  }
#endif
  if (!traceOptions.empty())
  {
    TraceStream::SetTraceFlags(traceOptions.c_str());
  }

  if (!forceAdminMode)
  {
    adminMode = oldAdminMode;
    SetAdminMode(initInfo.GetOptions()[InitOption::AdminMode], false);
  }

  DoStartupConfig();

  InitializeRootDirectories();

  Utils::GetEnvironmentString(MIKTEX_ENV_PACKAGE_LIST_FILE, packageHistoryFile);

  PushAppName(Utils::GetExeName());

  startDirectory.SetToCurrentDirectory();

  string miktexCwd;
  if (Utils::GetEnvironmentString(MIKTEX_ENV_CWD_LIST, miktexCwd))
  {
    for (CSVList cwd(miktexCwd, PATH_DELIMITER); cwd.GetCurrent() != nullptr; ++cwd)
    {
      AddInputDirectory(cwd.GetCurrent(), true);
    }
  }

  SetEnvironmentVariables();

  trace_core->WriteFormattedLine("core", T_("initializing MiKTeX Core version %s"), MIKTEX_COMPONENT_VERSION_STR);

#if defined(MIKTEX_WINDOWS) && defined(MIKTEX_CORE_SHARED)
  if (dynamicLoad == TriState::True)
  {
    trace_core->WriteFormattedLine("core", T_("dynamic load"));
  }
#endif

  trace_core->WriteFormattedLine("core", T_("operating system: %s"), Utils::GetOSVersionString().c_str());
  trace_core->WriteFormattedLine("core", T_("program file: %s"), GetMyProgramFile(true).Get());
  trace_core->WriteFormattedLine("core", T_("current directory: %s"), Q_(PathName().SetToCurrentDirectory()));

  trace_config->WriteFormattedLine("core", T_("session locale: %s"), Q_(defaultLocale.name()));

  if (adminMode && !IsSharedSetup())
  {
    MIKTEX_FATAL_ERROR(T_("Administrative startup refused because this is not a shared MiKTeX setup."));
  }
}

void SessionImpl::Uninitialize()
{
  if (!initialized)
  {
    return;
  }
  try
  {
    initialized = false;
    trace_core->WriteFormattedLine("core", T_("uninitializing core library"));
    CheckOpenFiles();
    WritePackageHistory();
    inputDirectories.clear();
    UnregisterLibraryTraceStreams();
    configurationSettings.clear();
    Botan::LibraryInitializer::deinitialize();
  }
  catch (const exception &)
  {
#if defined(MIKTEX_WINDOWS)
    while (numCoInitialize > 0)
    {
      MyCoUninitialize();
    }
#endif
    throw;
  }
#if defined(MIKTEX_WINDOWS)
  while (numCoInitialize > 0)
  {
    MyCoUninitialize();
  }
#endif
}

void SessionImpl::SetEnvironmentVariables()
{
#if MIKTEX_WINDOWS
  string str;

  // used in pdfcrop.pl
  Utils::SetEnvironmentString("TEXSYSTEM", "miktex");

  // Ghostscript
  Utils::SetEnvironmentString("GSC", MIKTEX_GS_EXE);
  PathName root1 = GetSpecialPath(SpecialPath::CommonInstallRoot);
  PathName root2 = GetSpecialPath(SpecialPath::UserInstallRoot);
  PathName gsbase1 = root1;
  gsbase1 /= "ghostscript";
  gsbase1 /= "base";
  PathName gsbase2 = root2;
  gsbase2 /= "ghostscript";
  gsbase2 /= "base";
  str = gsbase1.Get();
  if (gsbase1 != gsbase2)
  {
    str += PATH_DELIMITER;
    str += gsbase2.Get();
  }
  PathName fonts1 = root1;
  fonts1 /= "fonts";
  PathName fonts2 = root2;
  fonts2 /= "fonts";
  str += PATH_DELIMITER;
  str += fonts1.Get();
  if (fonts1 != fonts2)
  {
    str += PATH_DELIMITER;
    str += fonts2.Get();
  }
  Utils::SetEnvironmentString("MIKTEX_GS_LIB", str.c_str());
#endif

  PathName path = GetTempDirectory();

  if (!HaveEnvironmentString("TEMPDIR") || IsMiKTeXPortable())
  {
    Utils::SetEnvironmentString("TEMPDIR", path.Get());
  }

  if (!HaveEnvironmentString("TMPDIR") || IsMiKTeXPortable())
  {
    Utils::SetEnvironmentString("TMPDIR", path.Get());
  }

  if (!HaveEnvironmentString("TEMP") || IsMiKTeXPortable())
  {
    Utils::SetEnvironmentString("TEMP", path.Get());
  }

  if (!HaveEnvironmentString("TMP") || IsMiKTeXPortable())
  {
    Utils::SetEnvironmentString("TMP", path.Get());
  }

  if (!HaveEnvironmentString("HOME"))
  {
    Utils::SetEnvironmentString("HOME", GetHomeDirectory().Get());
  }

  SetCWDEnv();
}

void SessionImpl::SetTheNameOfTheGame(const char * lpszTheNameOfTheGame)
{
  MIKTEX_ASSERT_STRING(lpszTheNameOfTheGame);
  fileTypes.clear();
  theNameOfTheGame = lpszTheNameOfTheGame;
}

