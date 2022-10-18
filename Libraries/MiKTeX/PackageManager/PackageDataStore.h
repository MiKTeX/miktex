/**
 * @file PackageDataStore.h
 * @author Christian Schenk
 * @brief Package data store
 *
 * @copyright Copyright © 2018-2022 Christian Schenk
 *
 * This file is part of MiKTeX Package Manager.
 *
 * MiKTeX Package Manager is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <ctime>

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

#include <miktex/Util/PathName>
#include <miktex/Core/Session>
#include <miktex/Core/equal_icase>
#include <miktex/Core/hash_icase>
#include <miktex/Core/less_icase_dos>

#include <miktex/PackageManager/PackageManager>

#include "ComboCfg.h"

MPM_INTERNAL_BEGIN_NAMESPACE;

/**
 * @brief An instance of this class collects package records.
 *
 * The record data is retrieved from two sources:
 * - `miktex/config/package-manifests.ini`: immutable package manifests
 * - `miktex/config/packages.ini`: mutable package data such as installation
 *   timestamps
 */
class PackageDataStore
{
    typedef std::unordered_map<std::string, MiKTeX::Packages::PackageInfo, MiKTeX::Core::hash_icase, MiKTeX::Core::equal_icase> PackageDefinitionTable;

public:

    PackageDataStore();

    /**
     * @brief Clears the contents of the data store.
     */
    void Clear();

    /**
     * Declares a package as obsolete.
     * @param packageId The package ID.
     */
    void DeclareObsolete(const std::string& packageId);

    /**
     * Decrements the reference count of an installed file.
     * @param path The path to the file.
     * @return Returns the new (decremented) reference count.
     */
    unsigned long DecrementFileRefCount(const MiKTeX::Util::PathName& path);

    /**
     * Adds a record to the data store.
     * @param packageInfo The record to be added.
     */
    void DefinePackage(const MiKTeX::Packages::PackageInfo& packageinfo);

    /**
     * Gets the number of installed packages.
     * @param common Indicates whether to retrieve the number of packages for
     * the current user.
     * @return Returns the number of installed packages.
     */
    std::size_t GetNumberOfInstalledPackages(bool userScope);

    /**
     * Gets the reference count of a file.
     * @param path The path to the file.
     * @return Returns reference count of the file.
     */
    unsigned long GetFileRefCount(const MiKTeX::Util::PathName& path);

    /**
     * Retrieves a record from the data store.
     * @exception std::exception Record not found.
     * @param packageId The package ID.
     * @return Returns the requested record.
     */
    MiKTeX::Packages::PackageInfo GetPackage(const std::string& packageId)
    {
        return (*this)[packageId];
    }

    /**
     * Increments the reference counts of all files in a package.
     * @param path The package ID.
     */
    void IncrementFileRefCounts(const std::string& packageId);

    /**
     * @brief Loads package records into the data store.
     *
     * This method loads
     * - mutable data from a given INI file
     * - immutable package data from `miktex/config/packages.ini`
     * 
     * In addition, package dependencies and file reference counts are
     * calculated.
     *
     * @param path Path to the INI file.
     */
    void LoadAllPackageManifests(const MiKTeX::Util::PathName& path, bool mustBeSigned);

    PackageDataStore& Load();

    bool LoadedAllPackageManifests() const
    {
        return loadedAllPackageManifests;
    }

    /**
     * Migrates TPM files into a single INI file.
     *
     * If the INI file `miktex/config/package-manifests.ini` does not yet
     * exists, it will be created by parsing all TPM files. This method can be
     * used to migrate an old (pre 2019) package database into the new format.
     */
    void NeedPackageManifestsIni();

    /**
     * @brief Saves mutable package data.
     * 
     */
    void SaveVarData();

    /**
     * Updates a record in the data store.
     * @param packageInfo The record to update.
     */
    void SetPackage(const MiKTeX::Packages::PackageInfo& packageInfo)
    {
        (*this)[packageInfo.id] = packageInfo;
    }

    /**
     * Sets the release state of an installed package.
     * @param packageId The package ID.
     * @param releaseState The release state.
     */
    void SetReleaseState(const std::string& packageId, MiKTeX::Packages::RepositoryReleaseState releaseState);

    /**
     * @brief Sets the package installation timestamp.
     *
     * If the timestamp is zero (`InvalidTimeT`), the mutable package data will
     * cleared (the package is not installed).
     *
     * @param packageId The package ID.
     * @param timeInstalled The installation timestamp.
     */
    void SetTimeInstalled(const std::string& packageId, std::time_t timeInstalled);

    /**
     * Tries to retrieve a record from the data store.
     * @param packageId The package ID.
     * @return The first return value indicates whether the record (second
     * return value) could be retrieved.
     */
    std::tuple<bool, MiKTeX::Packages::PackageInfo> TryGetPackage(const std::string& packageId);

    class iterator
    {
    public:
        iterator(PackageDefinitionTable::iterator it) :
            it(it)
        {
        }
        MiKTeX::Packages::PackageInfo& operator*()
        {
            return it->second;
        }
        iterator& operator++()
        {
            it++;
            return *this;
        }
        bool operator==(const iterator& rhs)
        {
            return it == rhs.it;
        }
        bool operator!=(const iterator& rhs)
        {
            return it != rhs.it;
        }
    private:
        PackageDefinitionTable::iterator it;
    };

    /**
     * @brief Gets an iterator to the first record.
     * 
     * @return iterator 
     */
    iterator begin();

    /**
     * @brief Gets an iterator referring to the past-the-end record.
     *
     * @return iterator 
     */
    iterator end();

private:

    struct InstalledFileInfo
    {
        unsigned long refCount = 0;
    };

    typedef std::unordered_map<std::string, InstalledFileInfo, hash_path, equal_path> InstalledFileInfoTable;

    MiKTeX::Packages::PackageInfo& operator[](const std::string& packageId);
    MiKTeX::Packages::RepositoryReleaseState GetReleaseState(const std::string& packageId);
    bool IsObsolete(const std::string& packageId);
    bool IsRemovable(const std::string& packageId);
    std::time_t GetTimeInstalled(const std::string& packageId);
    std::time_t GetTimeInstalled(const std::string& packageId, MiKTeX::Core::ConfigurationScope scope);
    void IncrementFileRefCounts(const std::vector<std::string>& files);
    void Load(MiKTeX::Core::Cfg& cfg);
    void LoadVarData();

    ComboCfg comboCfg;
    InstalledFileInfoTable installedFileInfoTable;
    bool loadedAllPackageManifests = false;
    PackageDefinitionTable packageTable;
    std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_stopwatch;
};

MPM_INTERNAL_END_NAMESPACE;
