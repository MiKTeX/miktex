/**
 * @file miktex/Core/win/WindowsVersion.h
 * @author Christian Schenk
 * @brief Windows version information
 *
 * @copyright Copyright © 2015-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include <miktex/Core/config.h>

#include <string>

class MIKTEXNOVTABLE WindowsVersion
{

public:

    WindowsVersion() = delete;
    WindowsVersion(const WindowsVersion& other) = delete;
    WindowsVersion& operator=(const WindowsVersion& other) = delete;
    WindowsVersion(WindowsVersion&& other) = delete;
    WindowsVersion& operator=(WindowsVersion&& other) = delete;
    ~WindowsVersion() = delete;

    static MIKTEXCORECEEAPI(std::string) GetMajorMinorString();
    static MIKTEXCORECEEAPI(bool) IsWindows7OrGreater();
    static MIKTEXCORECEEAPI(bool) IsWindows8OrGreater();
};
