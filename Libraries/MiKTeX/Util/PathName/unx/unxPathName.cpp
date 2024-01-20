/**
 * @file unxPathName.cpp
 * @author Christian Schenk
 * @brief PathName class (Unix)
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include <unistd.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#define MIKTEXUTILEXPORT
#endif

#include "miktex/Util/PathName.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

PathName& PathName::SetToCurrentDirectory()
{
    while (getcwd(GetData(), GetCapacity()) == nullptr)
    {
        if (errno == ERANGE)
        {
            Reserve(GetCapacity() * 2);
        }
        else
        {
            throw CRuntimeError("getcwd");
        }
    }
    return *this;
}

PathName& PathName::SetToTempDirectory()
{
    if (Helpers::GetEnvironmentString("TMPDIR", *this) && this->IsAbsolute())
    {
        return *this;
    }
#if defined(P_tmpdir)
    *this = P_tmpdir;
#else
    * this = "/tmp";
#endif
    return *this;
}

PathName& PathName::SetToTempFile(const PathName& directory)
{
    *this = directory;
    AppendComponent("mikXXXXXX");
    int fd = mkstemp(GetData());
    if (fd < 0)
    {
        throw CRuntimeError("mkstemp");
    }
    close(fd);
    return *this;
}

PathName PathName::GetMountPoint() const
{
    throw Unexpected("not implemented");
}
