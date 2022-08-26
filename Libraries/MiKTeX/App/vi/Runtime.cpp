/**
 * @file vi/Runtime.cpp
 * @author Christian Schenk
 * @brief Runtime version information
 *
 * @copyright Copyright © 2005-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Application Framework.
 *
 * The MiKTeX Application Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#include "config.h"

#include <miktex/App/vi/Version>
#include <miktex/Core/vi/Version>
#include <miktex/PackageManager/vi/Version>

using namespace std;

using namespace MiKTeX::App::vi;
using namespace MiKTeX::Core;

string Runtime::GetName()
{
    return Header::GetName();
}

string Runtime::GetDescription()
{
    return Header::GetDescription();
}

int Runtime::GetInterfaceVersion()
{
    return Header::GetInterfaceVersion();
}

VersionNumber Runtime::GetVersion()
{
    return Header::GetVersion();
}

vector<LibraryVersion> Runtime::GetDependencies()
{
    vector<LibraryVersion> result;
    result.push_back(MiKTeX::Core::vi::Version::GetLibraryVersion());
    auto deps = MiKTeX::Core::vi::Runtime::GetDependencies();
    result.insert(std::end(result), std::begin(deps), std::end(deps));
    result.push_back(MiKTeX::Packages::vi::Version::GetLibraryVersion());
    deps = MiKTeX::Packages::vi::Runtime::GetDependencies();
    result.insert(std::end(result), std::begin(deps), std::end(deps));
    return result;
}
