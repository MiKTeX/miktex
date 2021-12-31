/**
 * @file internal.h
 * @author Christian Schenk
 * @brief Internal definitions
 *
 * @copyright Copyright © 2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#pragma once

#include <memory>
#include <string>

#include <miktex/Definitions>

#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()
#define T_(x) MIKTEXTEXT(x)

namespace OneMiKTeXUtility
{
    class MIKTEXNOVTABLE Program
    {
    public:
        virtual bool Canceled() = 0;
        virtual std::string InvocationName() = 0;
    };

    class MIKTEXNOVTABLE Installer
    {
    public:
        virtual void EnableInstaller(bool b) = 0;
        virtual bool IsInstallerEnabled() = 0;
    };

    class MIKTEXNOVTABLE Logger
    {
    public:
        virtual void LogFatal(const std::string& message) = 0;
        virtual void LogInfo(const std::string& message) = 0;
        virtual void LogTrace(const std::string& message) = 0;
        virtual void LogWarn(const std::string& message) = 0;
    };

    class MIKTEXNOVTABLE UI
    {
    public:
        virtual MIKTEXNORETURN void FatalError(const std::string& message) = 0;
        virtual MIKTEXNORETURN void IncorrectUsage(const std::string& message) = 0;
        virtual void Output(const std::string& s) = 0;
        virtual void Verbose(int level, const std::string& message) = 0;
        virtual int VerbosityLevel() = 0;
        virtual void Warning(const std::string& message) = 0;
    };

    struct ApplicationContext
    {
        Installer* installer;
        Logger* logger;
        std::shared_ptr<MiKTeX::Packages::PackageInstaller> packageInstaller;
        std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
        Program* program;
        std::shared_ptr<MiKTeX::Core::Session> session;
        UI* ui;
    };

    inline bool EndsWith(const std::string& s, const std::string& suffix)
    {
        return s.length() >= suffix.length() &&
            s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
    }

    inline std::vector<const char*> MakeArgv(const std::vector<std::string>& arguments)
    {
        std::vector<const char*> argv;
        argv.reserve(arguments.size() - 1 + 1);
        for (int idx = 1; idx < arguments.size(); ++idx)
        {
            argv.push_back(arguments[idx].c_str());
        }
        argv.push_back(nullptr);
        return argv;
    }
}
