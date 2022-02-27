/**
 * @file topics/repositories/commands/private.cpp
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
#include <miktex/util/StringUtil>

#include "private.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;

string Format(const string& outputTemplate, const RepositoryInfo& repositoryInfo)
{
    return fmt::format(outputTemplate,
        fmt::arg("bandwidth", repositoryInfo.dataTransferRate / 125000.0),
        fmt::arg("country", repositoryInfo.country),
        fmt::arg("delay", repositoryInfo.delay),
        fmt::arg("description", repositoryInfo.description),
        fmt::arg("integrity", repositoryInfo.integrity),
        fmt::arg("lastCheckTime", repositoryInfo.lastCheckTime),
        fmt::arg("lastVisitTime", repositoryInfo.lastVisitTime),
        fmt::arg("packageLevel", repositoryInfo.packageLevel),
        fmt::arg("ranking", repositoryInfo.ranking),
        fmt::arg("relativeDelay", repositoryInfo.relativeDelay),
        fmt::arg("releaseState", repositoryInfo.releaseState),
        fmt::arg("status", repositoryInfo.status),
        fmt::arg("timeDate", repositoryInfo.timeDate),
        fmt::arg("town", repositoryInfo.town),
        fmt::arg("type", repositoryInfo.type),
        fmt::arg("url", repositoryInfo.url),
        fmt::arg("version", repositoryInfo.version)
    );
}
