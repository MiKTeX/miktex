/**
 * @file miktex/Util/PathNameParser.h
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

#pragma once

#include <miktex/Util/config.h>

#include <memory>
#include <string>

#include "PathName.h"

MIKTEX_UTIL_BEGIN_NAMESPACE;

class PathNameParser
{

public:

    PathNameParser() = delete;
    PathNameParser(const PathNameParser& other) = delete;
    PathNameParser& operator=(const PathNameParser& other) = delete;
    PathNameParser(PathNameParser&& other) = delete;
    PathNameParser& operator=(PathNameParser&& other) = delete;
    virtual MIKTEXUTILEXPORT MIKTEXTHISCALL ~PathNameParser() noexcept;
    explicit MIKTEXUTILEXPORT MIKTEXTHISCALL PathNameParser(const PathName& path);
    explicit MIKTEXUTILEXPORT MIKTEXTHISCALL operator bool() const;
    MIKTEXUTILTHISAPI(std::string) operator*() const;
    MIKTEXUTILTHISAPI(PathNameParser&) operator++();

private:

    class impl;
    std::unique_ptr<impl> pimpl;
};

MIKTEX_UTIL_END_NAMESPACE;
