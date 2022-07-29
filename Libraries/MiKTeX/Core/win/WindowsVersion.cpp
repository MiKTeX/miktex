/**
 * @file WindowsVersion.cpp
 * @author Christian Schenk
 * @brief Windows version information
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <miktex/Core/win/WindowsVersion>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;

class LazyWindowsVersion
{

public:

    operator DWORD()
    {
        if (windowsVersion == 0)
        {
            // TODO: atomic
            windowsVersion = ::GetVersion();
        }
        return windowsVersion;
    }

private:

    DWORD windowsVersion = 0;
};

LazyWindowsVersion windowsVersion;

string WindowsVersion::GetMajorMinorString()
{
    unsigned major = LOBYTE(LOWORD(windowsVersion));
    unsigned minor = HIBYTE(LOWORD(windowsVersion));
    return std::to_string(major) + "." + std::to_string(minor);
}

bool WindowsVersion::IsWindows7OrGreater()
{
    unsigned major = LOBYTE(LOWORD(windowsVersion));
    unsigned minor = HIBYTE(LOWORD(windowsVersion));
    return major > 6 || (major == 6 && minor >= 1);
}

bool WindowsVersion::IsWindows8OrGreater()
{
    unsigned major = LOBYTE(LOWORD(windowsVersion));
    unsigned minor = HIBYTE(LOWORD(windowsVersion));
    return major > 6 || (major == 6 && minor >= 2);
}
