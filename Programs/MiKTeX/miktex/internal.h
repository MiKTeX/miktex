/**
 * @file internal.h
 * @author Christian Schenk
 * @brief Internal definitions
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#pragma once

#include <memory>
#include <sstream>
#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Definitions>

#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Util/PathName>
#include <miktex/Util/PathNameUtil>
#include <miktex/Util/StringUtil>

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()
#define T_(x) MIKTEXTEXT(x)

namespace OneMiKTeXUtility
{
    class MIKTEXNOVTABLE ProcessRunner
    {
    public:
        virtual void RunProcess(const MiKTeX::Util::PathName& fileName, const std::vector<std::string>& arguments) = 0;
    };

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
        virtual bool BeingQuiet() = 0;
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
        ProcessRunner* processRunner;
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

    inline std::string Unescape(const std::string& s)
    {
        std::stringstream out;
        bool wasEsc = false;
        for (auto ch : s)
        {
            if (wasEsc)
            {
                wasEsc = false;
                switch (ch)
                {
                    case '\\':
                        out << "\\";
                        break;
                    case 'n':
                        out << "\n";
                        break;
                    default:
                        out << "\\" << ch;
                        break;
                }
            }
            else if (ch == '\\')
            {
                wasEsc = true;
            }
            else
            {
                out << ch;
            }
        }
        return out.str();
    }

    inline std::string Format(const std::string& outputTemplate, const MiKTeX::Packages::PackageInfo& packageInfo)
    {
        auto delim = MiKTeX::Util::PathNameUtil::PathNameDelimiter;
        return fmt::format(outputTemplate,
            fmt::arg("archiveFileSize", packageInfo.archiveFileSize), // FIXME
            fmt::arg("copyrightOwner", packageInfo.copyrightOwner),
            fmt::arg("copyrightYear", packageInfo.copyrightYear),
            fmt::arg("creator", packageInfo.creator),
            fmt::arg("ctanPath", packageInfo.ctanPath),
            fmt::arg("description", packageInfo.description),
            fmt::arg("digest", packageInfo.digest),
            fmt::arg("displayName", packageInfo.displayName),
            fmt::arg("docFiles", MiKTeX::Util::StringUtil::Flatten(packageInfo.docFiles, delim)),
            fmt::arg("id", packageInfo.id),
            fmt::arg("isContained", packageInfo.IsContained()),
            fmt::arg("isContainer", packageInfo.IsContainer()),
            fmt::arg("isInstalled", packageInfo.IsInstalled()),
            fmt::arg("isInstalledCommon", packageInfo.IsInstalled(MiKTeX::Core::ConfigurationScope::Common)),
            fmt::arg("isInstalledUser", packageInfo.IsInstalled(MiKTeX::Core::ConfigurationScope::User)),
            fmt::arg("isObsolete", packageInfo.isObsolete),
            fmt::arg("isPureContainer", packageInfo.IsPureContainer()),
            fmt::arg("isRemovable", packageInfo.isRemovable),
            fmt::arg("licenseType", packageInfo.licenseType),
            fmt::arg("numFiles", packageInfo.GetNumFiles()),
            fmt::arg("refCount", packageInfo.GetRefCount()),
            fmt::arg("releaseState", packageInfo.releaseState),
            fmt::arg("requiredBy", MiKTeX::Util::StringUtil::Flatten(packageInfo.requiredBy, delim)),
            fmt::arg("requiredPackages", MiKTeX::Util::StringUtil::Flatten(packageInfo.requiredPackages, delim)),
            fmt::arg("runFiles", MiKTeX::Util::StringUtil::Flatten(packageInfo.runFiles, delim)),
            fmt::arg("size", packageInfo.GetSize()),
            fmt::arg("sizeDocFiles", packageInfo.sizeDocFiles),
            fmt::arg("sizeRunFiles", packageInfo.sizeRunFiles),
            fmt::arg("sizeSourceFiles", packageInfo.sizeSourceFiles),
            fmt::arg("sourceFiles", MiKTeX::Util::StringUtil::Flatten(packageInfo.sourceFiles, delim)),
            fmt::arg("targetSystem", packageInfo.targetSystem),
            fmt::arg("timeInstalled", packageInfo.GetTimeInstalled()),
            fmt::arg("timeInstalledCommon", packageInfo.timeInstalledCommon),
            fmt::arg("timeInstalledUser", packageInfo.timeInstalledUser),
            fmt::arg("timePackaged", packageInfo.timePackaged),
            fmt::arg("title", packageInfo.title),
            fmt::arg("version", packageInfo.version),
            fmt::arg("versionDate", packageInfo.versionDate)
        );
    }
}
