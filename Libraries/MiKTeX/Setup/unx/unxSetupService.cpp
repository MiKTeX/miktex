/**
 * @file  unxSetupService.cpp
 * @author Christian Schenk
 * @brief Setup service implementation (Unix-alikes)
 *
 * @copyright Copyright © 2016-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Setup Library.
 *
 * The MiKTeX Setup Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"
#include "internal.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Util;

PathName SetupService::GetDefaultCommonInstallDir()
{
    // TODO
    return PathName("/usr/local/miktex");
}

PathName SetupService::GetDefaultUserInstallDir()
{
    // TODO
    return PathName("~/miktex");
}

PathName SetupService::GetDefaultPortableRoot()
{
    // TODO
    return PathName("~/miktex-portable");
}
