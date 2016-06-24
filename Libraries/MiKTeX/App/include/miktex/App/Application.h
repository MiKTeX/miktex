/* miktex/App/Application.h:                            -*- C++ -*-

   Copyright (C) 2005-2016 Christian Schenk

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

#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Trace/TraceCallback>

#include <memory>
#include <set>
#include <string>
#include <vector>

// DLL import/export switch
#if !defined(BDF6E2537F116547846406B5B2B65949)
#  if defined(MIKTEX_APP_SHARED)
#    define MIKTEXAPPEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXAPPEXPORT
#  endif
#endif

// API decoration for exported member functions
#define MIKTEXAPPTHISAPI(type) MIKTEXAPPEXPORT type MIKTEXTHISCALL
#define MIKTEXAPPCEEAPI(type) MIKTEXAPPEXPORT type MIKTEXCEECALL

#if defined(__GNUC__)
#  define MIKTEXAPPTYPEAPI(type) MIKTEXAPPEXPORT type
#else
#  define MIKTEXAPPTYPEAPI(type) type
#endif

#define MIKTEX_APP_BEGIN_NAMESPACE              \
  namespace MiKTeX {                            \
    namespace App {

#define MIKTEX_APP_END_NAMESPACE                \
    }                                           \
  }

MIKTEX_APP_BEGIN_NAMESPACE;

class MIKTEXAPPTYPEAPI(Application) :
  public MiKTeX::Core::IFindFileCallback,
  public MiKTeX::Packages::PackageInstallerCallback,
  public MiKTeX::Trace::TraceCallback
{
public:
  MIKTEXAPPEXPORT MIKTEXTHISCALL Application();

public:
  virtual MIKTEXAPPEXPORT MIKTEXTHISCALL ~Application();

public:
  virtual MIKTEXAPPTHISAPI(void) Init(const MiKTeX::Core::Session::InitInfo & initInfo);

public:
  virtual MIKTEXAPPTHISAPI(void) Init(std::vector<char *> & args);

public:
  virtual MIKTEXAPPTHISAPI(void) Init(const std::string & programInvocationName);

public:
  virtual MIKTEXAPPTHISAPI(void) Init(const std::string & programInvocationName, const std::string & theNameOfTheGame);

public:
  virtual MIKTEXAPPTHISAPI(void) Finalize();

public:
  struct LibraryVersion
  {
    LibraryVersion(const std::string & name, const std::string & fromHeader, const std::string & fromRuntime) :
      name(name),
      fromHeader(fromHeader),
      fromRuntime(fromRuntime)
    {
    }
    LibraryVersion(const std::string & name, const MiKTeX::Core::VersionNumber * fromHeader, const MiKTeX::Core::VersionNumber * fromRuntime) :
      name(name),
      fromHeader(fromHeader == nullptr ? std::string() : fromHeader->ToString()),
      fromRuntime(fromRuntime == nullptr ? std::string() : fromRuntime->ToString())
    {
    }
    std::string name;
    std::string fromHeader;
    std::string fromRuntime;
  };

public:
  virtual MIKTEXAPPTHISAPI(void) GetLibraryVersions(std::vector<LibraryVersion> & versions) const;

public:
  virtual MIKTEXAPPTHISAPI(void) ShowLibraryVersions() const;

public:
  MIKTEXAPPTHISAPI(bool) InstallPackage(const std::string & deploymentName, const MiKTeX::Core::PathName & trigger, MiKTeX::Core::PathName & installRoot) override;

public:
  MIKTEXAPPTHISAPI(bool) TryCreateFile(const MiKTeX::Core::PathName & fileName, MiKTeX::Core::FileType fileType) override;

public:
  MIKTEXAPPTHISAPI(void) ReportLine(const std::string & str) override;

public:
  MIKTEXAPPTHISAPI(bool) OnRetryableError(const std::string & message) override;

public:
  MIKTEXAPPTHISAPI(bool) OnProgress(MiKTeX::Packages::Notification nf) override;

public:
  MIKTEXAPPTHISAPI(void) Trace(const TraceCallback::TraceMessage & traceMessage) override;

public:
  MIKTEXAPPTHISAPI(void) EnableInstaller(MiKTeX::Core::TriState tri);

public:
  MIKTEXNORETURN MIKTEXAPPCEEAPI(void) FatalError(const char * lpszFormat, ...);

public:
  static MIKTEXAPPCEEAPI(void) Sorry(const std::string & name, const MiKTeX::Core::MiKTeXException & ex);

public:
  static MIKTEXAPPCEEAPI(void) Sorry(const std::string & name, const std::exception & ex);

public:
  static MIKTEXAPPCEEAPI(void) Sorry(const std::string & name, const std::string & reason);

public:
  static void Sorry(const std::string & name)
  {
    Sorry(name, "");
  }

public:
  bool GetQuietFlag() const
  {
    return beQuiet;
  }

public:
  void SetQuietFlag(bool b)
  {
    beQuiet = b;
  }

public:
  std::shared_ptr<MiKTeX::Core::Session> GetSession() const
  {
    return session;
  }

public:
  MiKTeX::Core::TriState GetEnableInstaller() const
  {
    return enableInstaller;
  }

public:
  static MIKTEXAPPCEEAPI(Application*) GetApplication();

protected:
  static MIKTEXAPPCEEAPI(bool) Cancelled();

public:
  static MIKTEXAPPCEEAPI(void) CheckCancel();

private:
  bool beQuiet;

private:
  MiKTeX::Core::TriState enableInstaller = MiKTeX::Core::TriState::Undetermined;

private:
  MiKTeX::Core::TriState autoAdmin = MiKTeX::Core::TriState::Undetermined;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

private:
  std::shared_ptr<MiKTeX::Packages::PackageInstaller> installer;

private:
  std::set<std::string> ignoredPackages;

protected:
  std::shared_ptr<MiKTeX::Core::Session> session;

private:
  bool initialized = false;

protected:
  bool isLog4cxxConfigured = false;

public:
  bool IsLog4cxxConfigured() const
  {
    return isLog4cxxConfigured;
  }

private:
  std::vector<MiKTeX::Trace::TraceCallback::TraceMessage> pendingTraceMessages;

private:
  void FlushPendingTraceMessages();

private:
  void TraceInternal(const MiKTeX::Trace::TraceCallback::TraceMessage & traceMessage);
};

MIKTEX_APP_END_NAMESPACE;

#undef MIKTEX_APP_BEGIN_NAMESPACE
#undef MIKTEX_APP_END_NAMESPACE

#endif
