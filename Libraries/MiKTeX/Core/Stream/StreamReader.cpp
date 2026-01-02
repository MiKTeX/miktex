/**
 * @file Stream/StreamReader.cpp
 * @author Christian Schenk
 * @brief StreamReader implementation
 *
 * @copyright Copyright © 1996-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <miktex/Core/StreamReader>
#include <miktex/Core/Utils>
#include <miktex/Util/PathName>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

StreamReader::StreamReader(const PathName& path) :
    stream(File::Open(path, FileMode::Open, FileAccess::Read))
{
}

StreamReader::~StreamReader() noexcept
{
    try
    {
        Close();
    }
    catch (const exception&)
    {
    }
}

void StreamReader::Close()
{
    stream.Close();
}

bool StreamReader::ReadLine(string& line)
{
    return Utils::ReadLine(line, stream.GetFile(), false);
}
