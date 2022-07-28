/**
 * @file miktex/PackageManager/PackageInfo.h
 * @author Christian Schenk
 * @brief Package information
 *
 * @copyright Copyright © 2001-2022 Christian Schenk
 *
 * This file is part of MiKTeX Package Manager.
 *
 * MiKTeX Package Manager is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include "config.h"

#include <cstddef>
#include <ctime>

#include <string>
#include <vector>

#include <miktex/Core/MD5>
#include <miktex/Core/Session>

#include "definitions.h"
#include "RepositoryInfo.h"

MIKTEX_PACKAGES_BEGIN_NAMESPACE;

/// Package record.
struct PackageInfo
{
    /// Size of the archive file.
    std::size_t archiveFileSize = 0;

    /// The copyright owner of the package.
    std::string copyrightOwner;

    /// Copyright year
    std::string copyrightYear;

    /// Name of creator (packer).
    std::string creator;

    /// Relative path to the package directory on a CTAN mirror.
    std::string ctanPath;

    /// Multi-line package description.
    std::string description;

    /// MD5 of the package.
    MiKTeX::Core::MD5 digest;

    /// Display name.
    std::string displayName;

    /// Package ID.
    std::string id;

    /// `true`, if the package is obsolete.
    bool isObsolete = false;

    /// `true`, if the package can be removed.
    bool isRemovable = false;

    /// The doc file list.
    std::vector<std::string> docFiles;

    /// The license type of the package.
    std::string licenseType;

    /// The release state of the package.
    RepositoryReleaseState releaseState = RepositoryReleaseState::Unknown;

    /// List of dependents.
    std::vector<std::string> requiredBy;

    /// List of required packages.
    std::vector<std::string> requiredPackages;

    /// The run file list.
    std::vector<std::string> runFiles;

    /// Total size of all doc files.
    std::size_t sizeDocFiles = 0;

    /// Total size of all source files.
    std::size_t sizeSourceFiles = 0;

    /// Total size of all run files.
    std::size_t sizeRunFiles = 0;

    /// The source file list.
    std::vector<std::string> sourceFiles;

    /// Target system.
    std::string targetSystem;

    /// Date/time when the package was installed in the common scope.
    std::time_t timeInstalledCommon = InvalidTimeT;

    /// Date/time when the package was installed in the user scope..
    std::time_t timeInstalledUser = InvalidTimeT;

    /// Date/time when the package was created.
    std::time_t timePackaged = InvalidTimeT;

    /// One-line package description.
    std::string title;

    /// Informal version information.
    std::string version;

    /// The version/date of the package.
    std::string versionDate;

    /// Gets the number of dependents.
    unsigned long GetRefCount() const
    {
        return static_cast<unsigned long>(requiredBy.size());
    }

    /// Gets the total number of files in the package.
    /// @return Returns the number of files.
    unsigned long GetNumFiles() const
    {
        return static_cast<unsigned long>(runFiles.size() + docFiles.size() + sourceFiles.size());
    }

    /// Gets the total size (in bytes) of all files in the package.
    /// @return Returns the package size.
    std::size_t GetSize() const
    {
        return sizeRunFiles + sizeDocFiles + sizeSourceFiles;
    }

    /// Gets the time, the package was installed.
    std::time_t GetTimeInstalled() const
    {
        return IsValidTimeT(timeInstalledUser) ? timeInstalledUser : timeInstalledCommon;
    }

    /// Checks to see if this package has dependents.
    bool IsContained() const
    {
        return GetRefCount() > 0;
    }

    /// Tests for a container package.
    bool IsContainer() const
    {
        return !requiredPackages.empty();
    }

    /// Checks to see whether the package is installed.
    bool IsInstalled() const
    {
        return IsValidTimeT(timeInstalledUser) || IsValidTimeT(timeInstalledCommon);
    }

    /// Checks to see whether the package is installed in the specified scope.
    bool IsInstalled(MiKTeX::Core::ConfigurationScope scope) const
    {
        switch (scope)
        {
        case MiKTeX::Core::ConfigurationScope::User:
            return IsValidTimeT(timeInstalledUser);
        case MiKTeX::Core::ConfigurationScope::Common:
            return IsValidTimeT(timeInstalledCommon);
        default:
            MIKTEX_UNEXPECTED();
        }
    }

    /// Tests for a pure container package.
    /// @return Returns true, if this is a pure container package.
    bool IsPureContainer() const
    {
        return IsContainer() && GetNumFiles() <= 1;
    }

    /// Gets the time, the package was installed.
    void SetTimeInstalled(std::time_t timeInstalled, MiKTeX::Core::ConfigurationScope scope)
    {
        switch (scope)
        {
        case MiKTeX::Core::ConfigurationScope::User:
            timeInstalledUser = timeInstalled;
            break;
        case MiKTeX::Core::ConfigurationScope::Common:
            timeInstalledCommon = timeInstalled;
            break;
        default:
            MIKTEX_UNEXPECTED();
        }
    }
};

inline bool operator==(const PackageInfo& lhs, const PackageInfo& rhs)
{
    return true
        && lhs.copyrightOwner == rhs.copyrightOwner
        && lhs.copyrightYear == rhs.copyrightYear
        && lhs.creator == rhs.creator
        && lhs.ctanPath == rhs.ctanPath
        && lhs.description == rhs.description
        && lhs.digest == rhs.digest
        && lhs.displayName == rhs.displayName
        && lhs.id == rhs.id
        && lhs.licenseType == rhs.licenseType
        && lhs.requiredPackages == rhs.requiredPackages
        && lhs.sizeDocFiles == rhs.sizeDocFiles
        && lhs.sizeSourceFiles == rhs.sizeSourceFiles
        && lhs.targetSystem == rhs.targetSystem
        && lhs.timePackaged == rhs.timePackaged
        && lhs.title == rhs.title
        && lhs.version == rhs.version
        && lhs.versionDate == rhs.versionDate
#if 0
        // there was a time when the calculation of sizeRunFiles was broken
        // maybe due to .tpm file treatment
        && lhs.sizeRunFiles == rhs.sizeRunFiles
#endif
        ;
}

MIKTEX_PACKAGES_END_NAMESPACE;
