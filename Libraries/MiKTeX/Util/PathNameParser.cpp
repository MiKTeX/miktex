/**
 * @file PathNameParser.cpp
 * @author Christian Schenk
 * @brief PathNameParser class
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#define MIKTEXUTILEXPORT
#endif

#include "miktex/Util/PathNameParser.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

enum class PathNameParserState
{
    Start,
    Root,
    Path,
    End
};

class PathNameParser::impl
{

public:

    string current;
    PathName path;
    size_t pos = 0;
    PathNameParserState state = PathNameParserState::Start;
};

PathNameParser::PathNameParser(const PathName& path) :
    pimpl(new impl{})
{
    pimpl->path = path;
    ++(*this);
}

PathNameParser::operator bool() const
{
    return !pimpl->current.empty();
}

string PathNameParser::operator*() const
{
    return pimpl->current;
}

PathNameParser& PathNameParser::operator++()
{
    if (pimpl->state == PathNameParserState::Start && PathNameUtil::IsDirectoryDelimiter(pimpl->path[0]))
    {
        pimpl->current = pimpl->path[0];
        ++pimpl->pos;
        if (PathNameUtil::IsDirectoryDelimiter(pimpl->path[1]))
        {
            pimpl->state = PathNameParserState::Root;
            pimpl->current += pimpl->path[1];
            ++pimpl->pos;
            for (; pimpl->path[pimpl->pos] != 0 && !PathNameUtil::IsDirectoryDelimiter(pimpl->path[pimpl->pos]); ++pimpl->pos)
            {
                pimpl->current += pimpl->path[pimpl->pos];
            }
        }
        else
        {
            pimpl->state = PathNameParserState::Path;
        }
    }
#if defined(MIKTEX_WINDOWS)
    else if (pimpl->state == PathNameParserState::Start && PathNameUtil::IsDosDriveLetter(pimpl->path[0]) && PathNameUtil::IsDosVolumeDelimiter(pimpl->path[1]))
    {
        pimpl->state = PathNameParserState::Root;
        pimpl->current = pimpl->path[0];
        ++pimpl->pos;
        pimpl->current += pimpl->path[1];
        ++pimpl->pos;
    }
#endif
    else if (pimpl->state == PathNameParserState::Root)
    {
        pimpl->current = pimpl->path[pimpl->pos];
        pimpl->state = PathNameParserState::Path;
    }
    else
    {
        for (; pimpl->path[pimpl->pos] != 0 && PathNameUtil::IsDirectoryDelimiter(pimpl->path[pimpl->pos]); ++pimpl->pos)
        {
        }
        pimpl->current = "";
        for (; pimpl->path[pimpl->pos] != 0 && !PathNameUtil::IsDirectoryDelimiter(pimpl->path[pimpl->pos]); ++pimpl->pos)
        {
            pimpl->current += pimpl->path[pimpl->pos];
        }
    }
    return *this;
}

PathNameParser::~PathNameParser() noexcept
{
}
