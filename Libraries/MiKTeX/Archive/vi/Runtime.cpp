/**
 * @file vi/Runtime.cpp
 * @author Christian Schenk
 * @brief Library version information for MiKTeX Archive
 *
 * @copyright Copyright © 1996-2025 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#include "config.h"

#include <miktex/Core/vi/Version>
#include <miktex/Archive/vi/Version>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Archive::vi;

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
    return result;
}
