/**
 * @file topics/packages/commands/private.cpp
 * @author Christian Schenk
 * @brief Private parts
 *
 * @copyright Copyright © 2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Util/StringUtil>

#include "private.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;

string Format(const string& outputTemplate, const PackageInfo& packageInfo)
{
    auto delim = PathNameUtil::PathNameDelimiter;
    return fmt::format(outputTemplate,
        fmt::arg("archiveFileSize", packageInfo.archiveFileSize), // FIXME
        fmt::arg("copyrightOwner", packageInfo.copyrightOwner),
        fmt::arg("copyrightYear", packageInfo.copyrightYear),
        fmt::arg("creator", packageInfo.creator),
        fmt::arg("ctanPath", packageInfo.ctanPath),
        fmt::arg("description", packageInfo.description),
        fmt::arg("digest", packageInfo.digest),
        fmt::arg("displayName", packageInfo.displayName),
        fmt::arg("docFiles", StringUtil::Flatten(packageInfo.docFiles, delim)),
        fmt::arg("id", packageInfo.id),
        fmt::arg("isContained", packageInfo.IsContained()),
        fmt::arg("isContainer", packageInfo.IsContainer()),
        fmt::arg("isInstalled", packageInfo.IsInstalled()),
        fmt::arg("isInstalledCommon", packageInfo.IsInstalled(ConfigurationScope::Common)),
        fmt::arg("isInstalledUser", packageInfo.IsInstalled(ConfigurationScope::User)),
        fmt::arg("isObsolete", packageInfo.isObsolete),
        fmt::arg("isPureContainer", packageInfo.IsPureContainer()),
        fmt::arg("isRemovable", packageInfo.isRemovable),
        fmt::arg("licenseType", packageInfo.licenseType),
        fmt::arg("minTargetSystemVersion", packageInfo.minTargetSystemVersion),
        fmt::arg("numFiles", packageInfo.GetNumFiles()),
        fmt::arg("refCount", packageInfo.GetRefCount()),
        fmt::arg("releaseState", packageInfo.releaseState),
        fmt::arg("requiredBy", StringUtil::Flatten(packageInfo.requiredBy, delim)),
        fmt::arg("requiredPackages", StringUtil::Flatten(packageInfo.requiredPackages, delim)),
        fmt::arg("runFiles", StringUtil::Flatten(packageInfo.runFiles, delim)),
        fmt::arg("size", packageInfo.GetSize()),
        fmt::arg("sizeDocFiles", packageInfo.sizeDocFiles),
        fmt::arg("sizeRunFiles", packageInfo.sizeRunFiles),
        fmt::arg("sizeSourceFiles", packageInfo.sizeSourceFiles),
        fmt::arg("sourceFiles", StringUtil::Flatten(packageInfo.sourceFiles, delim)),
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
