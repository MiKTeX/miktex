/**
 * @file Pipe.h
 * @author Christian Schenk
 * @brief Uncompress utilities
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/BZip2Stream>
#include <miktex/Core/FileStream>
#include <miktex/Core/GzipStream>
#include <miktex/Core/LzmaStream>
#include <miktex/Util/PathName>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

void Utils::UncompressFile(const PathName& pathIn, PathName& pathOut)
{
    SESSION_IMPL()->trace_process->WriteLine("core", fmt::format(T_("uncompressing {0}..."), Q_(pathIn)));
    if (!File::Exists(pathIn))
    {
        MIKTEX_FATAL_ERROR_2(T_("The file could not be found."), "path", pathIn.ToString());
    }
    PathName pathTempFileName;
    pathTempFileName.SetToTempFile();
    FileStream stream(File::Open(pathTempFileName, FileMode::Create, FileAccess::Write, false));
    unique_ptr<Stream> inputStream;
    if (pathIn.HasExtension(".gz"))
    {
        inputStream = GzipStream::Create(pathIn, true);
    }
    else if (pathIn.HasExtension(".bz2"))
    {
        inputStream = BZip2Stream::Create(pathIn, true);
    }
    else if (pathIn.HasExtension(".lzma") || pathIn.HasExtension(".xz"))
    {
        inputStream = LzmaStream::Create(pathIn, true);
    }
    else
    {
        MIKTEX_FATAL_ERROR_2(T_("Could not uncompress file."), "path", pathIn.ToString());
    }
    unsigned char buf[4096];
    size_t len;
    while ((len = inputStream->Read(buf, ARRAY_SIZE(buf))) > 0)
    {
        stream.Write(buf, len);
    }
    pathOut = pathTempFileName;
}
