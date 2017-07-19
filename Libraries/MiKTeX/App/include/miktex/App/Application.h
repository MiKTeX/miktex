/* miktex/App/Application.h:                            -*- C++ -*-

   Copyright (C) 2005-2017 Christian Schenk

   This file is part of the MiKTeX App Library.

   The MiKTeX App Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX App Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX App Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(F4C0E5199356C44CBA46523020038822)
#define F4C0E5199356C44CBA46523020038822

#include "config.h"

#include <miktex/App/vi/Version>
#include <miktex/Core/LibraryVersion>
#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Trace/TraceCallback>

#include <memory>
#include <string>
#include <vector>

MIKTEX_APP_BEGIN_NAMESPACE;

class MIKTEXAPPTYPEAPI(Application) :
  public MiKTeX::Core::IFindFileCallback,
  public MiKTeX::Packages::PackageInstallerCallback,
  public MiKTeX::Trace::TraceCallback
{
public:
  MIKTEXAPPEXPORT MIKTEXTHISCALL Application();

public:
  Application(const Application& other) = delete;

public:
  Application& operator=(const Application& other) = delete;

public:
  Application(Application&& other) = delete;

public:
  Application& operator=(Application&& other) = delete;

public:
  virtual MIKTEXAPPEXPORT MIKTEXTHISCALL ~Application() noexcept;

public:
  virtual MIKTEXAPPTHISAPI(void) Init(const MiKTeX::Core::Session::InitInfo& initInfo, std::vector<char*>& args);

public:
  virtual MIKTEXAPPTHISAPI(void) Init(const MiKTeX::Core::Session::InitInfo& initInfo);

public:
  virtual MIKTEXAPPTHISAPI(void) Init(std::vector<char*>& args);

public:
  virtual MIKTEXAPPTHISAPI(void) Init(const std::string& programInvocationName);

public:
  virtual MIKTEXAPPTHISAPI(void) Init(const std::string& programInvocationName, const std::string& theNameOfTheGame);

public:
  virtual MIKTEXAPPTHISAPI(void) Finalize();

public:
  virtual void GetLibraryVersions(std::vector<MiKTeX::Core::LibraryVersion>& versions) const
  {
    versions.push_back(MiKTeX::App::vi::Version::GetLibraryVersion());
    auto deps = MiKTeX::App::vi::Runtime::GetDependencies();
    versions.insert(std::end(versions), std::begin(deps), std::end(deps));
  }

public:
  virtual MIKTEXAPPTHISAPI(void) ShowLibraryVersions() const;

public:
  MIKTEXAPPTHISAPI(bool) InstallPackage(const std::string& deploymentName, const MiKTeX::Core::PathName& trigger, MiKTeX::Core::PathName& installRoot) override;

public:
  MIKTEXAPPTHISAPI(bool) TryCreateFile(const MiKTeX::Core::PathName& fileName, MiKTeX::Core::FileType fileType) override;

public:
  MIKTEXAPPTHISAPI(void) ReportLine(const std::string& str) override;

public:
  MIKTEXAPPTHISAPI(bool) OnRetryableError(const std::string& message) override;

public:
  MIKTEXAPPTHISAPI(bool) OnProgress(MiKTeX::Packages::Notification nf) override;

public:
  MIKTEXAPPTHISAPI(void) Trace(const TraceCallback::TraceMessage& traceMessage) override;

public:
  MIKTEXAPPTHISAPI(void) EnableInstaller(MiKTeX::Core::TriState tri);

public:
  MIKTEXNORETURN MIKTEXAPPCEEAPI(void) FatalError(const char* lpszFormat, ...);

public:
  MIKTEXAPPTHISAPI(void) InvokeEditor(const MiKTeX::Core::PathName& editFileName, int editLineNumber, MiKTeX::Core::FileType editFileType, const MiKTeX::Core::PathName& transcriptFileName) const;

public:
  static MIKTEXAPPCEEAPI(void) Sorry(const std::string& name, const MiKTeX::Core::MiKTeXException& ex);

public:
  static MIKTEXAPPCEEAPI(void) Sorry(const std::string& name, const std::exception& ex);

public:
  static MIKTEXAPPCEEAPI(void) Sorry(const std::string& name, const std::string& reason);

public:
  static void Sorry(const std::string& name)
  {
    Sorry(name, "");
  }

public:
  MIKTEXAPPTHISAPI(bool) GetQuietFlag() const;

public:
  MIKTEXAPPTHISAPI(void) SetQuietFlag(bool b);

public:
  MIKTEXAPPTHISAPI(std::shared_ptr<MiKTeX::Core::Session>) GetSession() const;

public:
  MIKTEXAPPTHISAPI(MiKTeX::Core::TriState) GetEnableInstaller() const;

public:
  MIKTEXAPPTHISAPI(void) LogInfo(const std::string& message) const;

public:
  MIKTEXAPPTHISAPI(void) LogWarn(const std::string& message) const;

public:
  MIKTEXAPPTHISAPI(void) LogError(const std::string& message) const;

public:
  static MIKTEXAPPCEEAPI(Application*) GetApplication();

protected:
  static MIKTEXAPPCEEAPI(bool) Cancelled();

public:
  static MIKTEXAPPCEEAPI(void) CheckCancel();

private:
  void FlushPendingTraceMessages();

private:
  void TraceInternal(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage);

private:
  void ConfigureLogging();

private:
  void AutoMaintenance();

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEX_APP_END_NAMESPACE;

#endif
