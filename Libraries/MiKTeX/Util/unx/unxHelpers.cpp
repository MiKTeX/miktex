/**
 * @file unxHelpers.cpp
 * @author Christian Schenk
 * @brief Helpers class (Unix)
 *
 * @copyright Copyright © 2021-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include <sys/stat.h>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

BEGIN_INTERNAL_NAMESPACE;

void Helpers::CanonicalizePathName(PathName& path)
{
    char* resolved = realpath(path.GetData(), nullptr);
    if (resolved == nullptr)
    {
        if (errno == ENOENT)
        {
            return;
        }
        throw CRuntimeError("realpath");
    }
    path = resolved;
    free(resolved);
}

bool Helpers::GetEnvironmentString(const string& name, string& value)
{
    const char* path = getenv(name.c_str());
    if (path == nullptr)
    {
        return false;
    }
    else
    {
        value = path;
        return true;
    }
}

bool Helpers::DirectoryExists(const PathName& path)
{
    struct stat statbuf;
    if (stat(path.GetData(), &statbuf) == 0)
    {
        if (S_ISDIR(statbuf.st_mode) == 0)
        {
            return false;
        }
        return true;
    }
    int error = errno;
    if (error != ENOENT)
    {
        throw CRuntimeError("stat");
    }
    return false;
}

END_INTERNAL_NAMESPACE;
