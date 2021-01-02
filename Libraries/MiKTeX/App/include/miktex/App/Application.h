/* miktex/App/Application.h:                            -*- C++ -*-

   Copyright (C) 2005-2021 Christian Schenk

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

#pragma once

#if !defined(F4C0E5199356C44CBA46523020038822)
#define F4C0E5199356C44CBA46523020038822

#include "config.h"

#include <memory>
#include <string>
#include <vector>

#include <miktex/App/vi/Version>
#include <miktex/Core/LibraryVersion>
#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Trace/TraceCallback>

/// @namespace MiKTeX::App
/// @brief The application namespace.
MIKTEX_APP_BEGIN_NAMESPACE;

/// An instance of this class controls the run-time behaviour of a MiKTeX application.
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

  /// Filters out and processes special MiKTeX command-line arguments.
  /// @param[in,out] args The command-line arguments to examine and modify.
  /// @param[in,out] initInfo The session initialization options to modify.
  /// @return Returns the modified command-line.
public:
  virtual MIKTEXAPPTHISAPI(std::string) ExamineArgs(std::vector<const char*>& args, MiKTeX::Core::Session::InitInfo& initInfo);

  /// Filters out and processes special MiKTeX command-line arguments.
  /// @param[in,out] args The command-line arguments to examine and modify.
  /// @param[in,out] initInfo The session initialization options to modify.
  /// @return Returns the modified command-line.
public:
  virtual MIKTEXAPPTHISAPI(std::string) ExamineArgs(std::vector<char*>& args, MiKTeX::Core::Session::InitInfo& initInfo);

  /// Initializes the application.
  /// @param initInfo Session initialization options.
  /// @param[in,out] args The command-line arguments to examine and modify.
public:
  virtual MIKTEXAPPTHISAPI(void) Init(const MiKTeX::Core::Session::InitInfo& initInfo, std::vector<const char*>& args);

  /// Initializes the application.
  /// @param initInfo Session initialization options.
  /// @param[in,out] args The command-line arguments to examine and modify.
public:
  virtual MIKTEXAPPTHISAPI(void) Init(const MiKTeX::Core::Session::InitInfo& initInfo, std::vector<char*>& args);

  /// Initializes the application.
  /// @param initInfo Session initialization options.
public:
  virtual MIKTEXAPPTHISAPI(void) Init(const MiKTeX::Core::Session::InitInfo& initInfo);

  /// Initializes the application.
  /// @param[in,out] args The command-line arguments to examine and modify.
public:
  virtual MIKTEXAPPTHISAPI(void) Init(std::vector<const char*>& args);

  /// Initializes the application.
  /// @param[in,out] args The command-line arguments to examine and modify.
public:
  virtual MIKTEXAPPTHISAPI(void) Init(std::vector<char*>& args);

  /// Initializes the application.
  /// @param programInvocationName The program invocation name.
public:
  virtual MIKTEXAPPTHISAPI(void) Init(const std::string& programInvocationName);

  /// Initializes the application.
  /// @param programInvocationName The program invocation name.
  /// @param theNameOfTheGame The user friendly program name.
public:
  virtual MIKTEXAPPTHISAPI(void) Init(const std::string& programInvocationName, const std::string& theNameOfTheGame);

  /// Frees application resources.
public:
  virtual MIKTEXAPPTHISAPI(void) Finalize();

  /// Frees application resources.
  /// @param exitCode The exit code to write to the log file.
public:
  virtual MIKTEXAPPTHISAPI(void) Finalize2(int exitCode);

  /// Collects version information of loaded libraries.
  /// @param[in,out] versions The version table to modify.
public:
  virtual void GetLibraryVersions(std::vector<MiKTeX::Core::LibraryVersion>& versions) const
  {
    versions.push_back(MiKTeX::App::vi::Version::GetLibraryVersion());
    auto deps = MiKTeX::App::vi::Runtime::GetDependencies();
    versions.insert(std::end(versions), std::begin(deps), std::end(deps));
  }

  /// Prints version information of loaded libraries.
public:
  virtual MIKTEXAPPTHISAPI(void) ShowLibraryVersions() const;

  /// @brief Install a package.
  ///
  /// This method gets called by the session object when a package
  /// needs to be installed.
  ///
  /// @param packageId The ID of the requested package.
  /// @param trigger The path to the file which triggered the auto-installer.
  /// @param installRoot The path to the installation root directory.
  /// @return Returns `true`, if the package has been installed.
public:
  MIKTEXAPPTHISAPI(bool) InstallPackage(const std::string& packageId, const MiKTeX::Core::PathName& trigger, MiKTeX::Core::PathName& installRoot) override;

  /// @brief Creates a file.
  ///
  /// This method gets called by the session object when a file (e.g.,
  /// `pdflatex.fmt`) needs to be created.
  ///
  /// @param fileName The path to the file.
  /// @param fileType The file type.
  /// @return Returns `true`, if the file has been created.
public:
  MIKTEXAPPTHISAPI(bool) TryCreateFile(const MiKTeX::Core::PathName& fileName, MiKTeX::Core::FileType fileType) override;

  /// @brief Prints an installer message.
  ///
  /// This method gets called by the installer when a message should
  /// be written to the report stream.
  ///
  /// @param message The message text.
public:
  MIKTEXAPPTHISAPI(void) ReportLine(const std::string& message) override;

  /// @brief Handles an installer error.
  ///
  /// This method gets called by the installer when an error occured.
  ///
  /// @param message The error message.
  /// @returns Returns `false` to cancel the installer.
public:
  MIKTEXAPPTHISAPI(bool) OnRetryableError(const std::string& message) override;

  /// @brief Handles an installer notification.
  ///
  /// This method gets called by the installer when the current phase
  /// ends or when a new phase begins.
  ///
  /// @param nf The notification.
public:
  MIKTEXAPPTHISAPI(bool) OnProgress(MiKTeX::Packages::Notification nf) override;

  /// @brief Logs a trace message.
  ///
  /// This method gets called when a trace message should be logged.
  ///
  /// @param traceMessage The trace message to log.
public:
  MIKTEXAPPTHISAPI(bool) Trace(const TraceCallback::TraceMessage& traceMessage) override;

  /// Enables or disables the auto-installer.
  /// @param tri The new state (on, off, inherit configuration default).
public:
  MIKTEXAPPTHISAPI(void) EnableInstaller(MiKTeX::Configuration::TriState tri);

  /// Terminates the application with an error. 
  /// @param message The error message.
public:
  MIKTEXNORETURN MIKTEXAPPCEEAPI(void) FatalError(const std::string& message);

  /// Print a warning nessage.
  /// @param message The warning message.
public:
  MIKTEXAPPCEEAPI(void) Warning(const std::string& message);

  /// Print a security warning nessage.
  /// @param message The security warning message.
public:
  MIKTEXAPPCEEAPI(void) SecurityRisk(const std::string& message);

  /// Starts a text editor.
  /// @param editFileName Path to the file to be edited.
  /// @param editLineNumber The line where the edit cursor should be moved to.
  /// @param editFileType The file type.
  /// @param transcriptFileName Path to a secondary file (usually a log file).
public:
  MIKTEXAPPTHISAPI(void) InvokeEditor(const MiKTeX::Core::PathName& editFileName, int editLineNumber, MiKTeX::Core::FileType editFileType, const MiKTeX::Core::PathName& transcriptFileName) const;

  /// Prints a user friendly error message.
  /// @param name User friendly program name.
  /// @param ex Error information.
public:
  static MIKTEXAPPCEEAPI(void) Sorry(const std::string& name, const MiKTeX::Core::MiKTeXException& ex);

  /// Prints a user friendly error message.
  /// @param name User friendly program name.
  /// @param ex Error information.
public:
  static MIKTEXAPPCEEAPI(void) Sorry(const std::string& name, const std::exception& ex);

  /// Prints a user friendly error message.
  /// @param name User friendly program name.
  /// @param description A user friendly description of the error.
  /// @param remedy A user friendly recipe to remedy the error.
  /// @param url A link to a help page.
public:
  static MIKTEXAPPCEEAPI(void) Sorry(const std::string& name, const std::string& description, const std::string& remedy, const std::string& url);

  /// Prints a user friendly error message.
  /// @param name User friendly program name.
public:
  static void Sorry(const std::string& name)
  {
    Sorry(name, "", "", "");
  }

  /// Gets an indication whether the application is in quiet mode.
  /// @return Returns `true`, if the application is in quiet mode.
  /// @sa SetQuietFlag
public:
  MIKTEXAPPTHISAPI(bool) GetQuietFlag() const;

  /// Turns quiet mode on/off.
  /// @param b The new quiet mode.
  /// @sa GetQuietFlag
public:
  MIKTEXAPPTHISAPI(void) SetQuietFlag(bool b);

  /// Gets the current session.
  /// @return Returns a smart pointer to the current session object.
public:
  MIKTEXAPPTHISAPI(std::shared_ptr<MiKTeX::Core::Session>) GetSession() const;

  /// Gets the current auto-installer mode.
  /// @return Returns the current auto-installer mode (on, off, inherit configuration default).
public:
  MIKTEXAPPTHISAPI(MiKTeX::Configuration::TriState) GetEnableInstaller() const;

  /// Logs an informational message.
  /// @param message The message to log.
public:
  MIKTEXAPPTHISAPI(void) LogInfo(const std::string& message) const;

  /// Logs a warning message.
  /// @param message The message to log.
public:
  MIKTEXAPPTHISAPI(void) LogWarn(const std::string& message) const;

  /// Logs an error message.
  /// @param message The message to log.
public:
  MIKTEXAPPTHISAPI(void) LogError(const std::string& message) const;

  /// Gets the current application object.
  /// @return Returns a pointer to the current application object.
public:
  static MIKTEXAPPCEEAPI(Application*) GetApplication();

  /// Gets an indication whether the application should be terminated.
  /// @return Returns `true`, if `SIGINT` or `SIGTERM` has been received.
protected:
  static MIKTEXAPPCEEAPI(bool) Cancelled();

  /// Throws an exception if the application should be terminated.
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
  void AutoDiagnose();

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEX_APP_END_NAMESPACE;

#endif
