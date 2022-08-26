/**
 * @file miktex/App/Application.h
 * @author Christian Schenk
 * @brief Application class
 *
 * @copyright Copyright © 2005-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Application Framework.
 *
 * The MiKTeX Application Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

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
    Application(const Application& other) = delete;
    Application& operator=(const Application& other) = delete;
    Application(Application && other) = delete;
    Application& operator=(Application && other) = delete;
    virtual MIKTEXAPPEXPORT MIKTEXTHISCALL ~Application() noexcept;

    /// Filters out and processes special MiKTeX command-line arguments.
    /// @param[in,out] args The command-line arguments to examine and modify.
    /// @param[in,out] initInfo The session initialization options to modify.
    /// @return Returns the modified command-line.
    virtual MIKTEXAPPTHISAPI(std::string) ExamineArgs(std::vector<const char*>&args, MiKTeX::Core::Session::InitInfo& initInfo);

    /// Filters out and processes special MiKTeX command-line arguments.
    /// @param[in,out] args The command-line arguments to examine and modify.
    /// @param[in,out] initInfo The session initialization options to modify.
    /// @return Returns the modified command-line.
    virtual MIKTEXAPPTHISAPI(std::string) ExamineArgs(std::vector<char*>&args, MiKTeX::Core::Session::InitInfo& initInfo);

    /// Initializes the application.
    /// @param initInfo Session initialization options.
    /// @param[in,out] args The command-line arguments to examine and modify.
    virtual MIKTEXAPPTHISAPI(void) Init(const MiKTeX::Core::Session::InitInfo& initInfo, std::vector<const char*>&args);

    /// Initializes the application.
    /// @param initInfo Session initialization options.
    /// @param[in,out] args The command-line arguments to examine and modify.
    virtual MIKTEXAPPTHISAPI(void) Init(const MiKTeX::Core::Session::InitInfo& initInfo, std::vector<char*>&args);

    /// Initializes the application.
    /// @param initInfo Session initialization options.
    virtual MIKTEXAPPTHISAPI(void) Init(const MiKTeX::Core::Session::InitInfo& initInfo);

    /// Initializes the application.
    /// @param[in,out] args The command-line arguments to examine and modify.
    virtual MIKTEXAPPTHISAPI(void) Init(std::vector<const char*>&args);

    /// Initializes the application.
    /// @param[in,out] args The command-line arguments to examine and modify.
    virtual MIKTEXAPPTHISAPI(void) Init(std::vector<char*>&args);

    /// Initializes the application.
    /// @param programInvocationName The program invocation name.
    virtual MIKTEXAPPTHISAPI(void) Init(const std::string& programInvocationName);

    /// Initializes the application.
    /// @param programInvocationName The program invocation name.
    /// @param theNameOfTheGame The user friendly program name.
    virtual MIKTEXAPPTHISAPI(void) Init(const std::string& programInvocationName, const std::string& theNameOfTheGame);

    /// Frees application resources.
    virtual MIKTEXAPPTHISAPI(void) Finalize();

    /// Frees application resources.
    /// @param exitCode The exit code to write to the log file.
    virtual MIKTEXAPPTHISAPI(void) Finalize2(int exitCode);

    /// Collects version information of loaded libraries.
    /// @param[in,out] versions The version table to modify.
    virtual void GetLibraryVersions(std::vector<MiKTeX::Core::LibraryVersion>&versions) const
    {
        versions.push_back(MiKTeX::App::vi::Version::GetLibraryVersion());
        auto deps = MiKTeX::App::vi::Runtime::GetDependencies();
        versions.insert(std::end(versions), std::begin(deps), std::end(deps));
    }

    /// Prints version information of loaded libraries.
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
    MIKTEXAPPTHISAPI(bool) InstallPackage(const std::string& packageId, const MiKTeX::Util::PathName& trigger, MiKTeX::Util::PathName& installRoot) override;

    /// @brief Creates a file.
    ///
    /// This method gets called by the session object when a file (e.g.,
    /// `pdflatex.fmt`) needs to be created.
    ///
    /// @param fileName The path to the file.
    /// @param fileType The file type.
    /// @return Returns `true`, if the file has been created.
    MIKTEXAPPTHISAPI(bool) TryCreateFile(const MiKTeX::Util::PathName& fileName, MiKTeX::Core::FileType fileType) override;

    /// @brief Prints an installer message.
    ///
    /// This method gets called by the installer when a message should
    /// be written to the report stream.
    ///
    /// @param message The message text.
    MIKTEXAPPTHISAPI(void) ReportLine(const std::string& message) override;

    /// @brief Handles an installer error.
    ///
    /// This method gets called by the installer when an error occured.
    ///
    /// @param message The error message.
    /// @returns Returns `false` to cancel the installer.
    MIKTEXAPPTHISAPI(bool) OnRetryableError(const std::string& message) override;

    /// @brief Handles an installer notification.
    ///
    /// This method gets called by the installer when the current phase
    /// ends or when a new phase begins.
    ///
    /// @param nf The notification.
    MIKTEXAPPTHISAPI(bool) OnProgress(MiKTeX::Packages::Notification nf) override;

    /// @brief Logs a trace message.
    ///
    /// This method gets called when a trace message should be logged.
    ///
    /// @param traceMessage The trace message to log.
    MIKTEXAPPTHISAPI(bool) Trace(const TraceCallback::TraceMessage& traceMessage) override;

    /// Enables or disables the auto-installer.
    /// @param tri The new state (on, off, inherit configuration default).
    MIKTEXAPPTHISAPI(void) EnableInstaller(MiKTeX::Configuration::TriState tri);

    /// Terminates the application with an error. 
    /// @param message The error message.
    MIKTEXNORETURN MIKTEXAPPCEEAPI(void) FatalError(const std::string& message);

    /// Print a warning nessage.
    /// @param message The warning message.
    MIKTEXAPPCEEAPI(void) Warning(const std::string& message);

    /// Print a security warning nessage.
    /// @param message The security warning message.
    MIKTEXAPPCEEAPI(void) SecurityRisk(const std::string& message);

    /// Starts a text editor.
    /// @param editFileName Path to the file to be edited.
    /// @param editLineNumber The line where the edit cursor should be moved to.
    /// @param editFileType The file type.
    /// @param transcriptFileName Path to a secondary file (usually a log file).
    MIKTEXAPPTHISAPI(void) InvokeEditor(const MiKTeX::Util::PathName& editFileName, int editLineNumber, MiKTeX::Core::FileType editFileType, const MiKTeX::Util::PathName& transcriptFileName) const;

    /// Prints a user friendly error message.
    /// @param name User friendly program name.
    /// @param ex Error information.
    MIKTEXAPPTHISAPI(void) Sorry(const std::string& name, const MiKTeX::Core::MiKTeXException& ex);

    /// Prints a user friendly error message.
    /// @param name User friendly program name.
    /// @param ex Error information.
    MIKTEXAPPTHISAPI(void) Sorry(const std::string& name, const std::exception& ex);

    /// Prints a user friendly error message.
    /// @param name User friendly program name.
    /// @param description A user friendly description of the error.
    /// @param remedy A user friendly recipe to remedy the error.
    /// @param url A link to a help page.
    MIKTEXAPPTHISAPI(void) Sorry(const std::string& name, const std::string& description, const std::string& remedy, const std::string& url);

    /// Prints a user friendly error message.
    /// @param name User friendly program name.
    void Sorry(const std::string& name)
    {
        Sorry(name, "", "", "");
    }

    /// Gets an indication whether the application is in quiet mode.
    /// @return Returns `true`, if the application is in quiet mode.
    /// @sa SetQuietFlag
    MIKTEXAPPTHISAPI(bool) GetQuietFlag() const;

    /// Turns quiet mode on/off.
    /// @param b The new quiet mode.
    /// @sa GetQuietFlag
    MIKTEXAPPTHISAPI(void) SetQuietFlag(bool b);

    /// Gets the current session.
    /// @return Returns a smart pointer to the current session object.
    MIKTEXAPPTHISAPI(std::shared_ptr<MiKTeX::Core::Session>) GetSession() const;

    /// Gets the current auto-installer mode.
    /// @return Returns the current auto-installer mode (on, off, inherit configuration default).
    MIKTEXAPPTHISAPI(MiKTeX::Configuration::TriState) GetEnableInstaller() const;

    /// Logs an informational message.
    /// @param message The message to log.
    MIKTEXAPPTHISAPI(void) LogInfo(const std::string& message) const;

    /// Logs a warning message.
    /// @param message The message to log.
    MIKTEXAPPTHISAPI(void) LogWarn(const std::string& message) const;

    /// Logs an error message.
    /// @param message The message to log.
    MIKTEXAPPTHISAPI(void) LogError(const std::string& message) const;

    /// Throws an exception if the application should be terminated.
    MIKTEXAPPTHISAPI(void) CheckCancel();

    /// Gets the current application object.
    /// @return Returns a pointer to the current application object.
    static MIKTEXAPPCEEAPI(Application*) GetApplication();

protected:

    /// Gets an indication whether the application should be terminated.
    /// @return Returns `true`, if `SIGINT` or `SIGTERM` has been received.
    static MIKTEXAPPCEEAPI(bool) Cancelled();

private:

    void FlushPendingTraceMessages();
    void TraceInternal(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage);
    void ConfigureLogging();
    void AutoMaintenance();
    void AutoDiagnose();

    class impl;
    std::unique_ptr<impl> pimpl;
};

MIKTEX_APP_END_NAMESPACE;
