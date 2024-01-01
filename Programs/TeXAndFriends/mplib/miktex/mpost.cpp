/**
 * @file mplib/miktex/mpost.cpp
 * @author Christian Schenk
 * @brief MiKTeX Metapost helpers
 *
 * @copyright Copyright © 2017-2024 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/App/Application>
#include <miktex/Core/Utils>

#include "mpost.h"

using namespace std;

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

void miktex_show_library_versions()
{
    Application::GetApplication()->ShowLibraryVersions();
}

inline std::string GetBanner(const char* name, const char* version)
{
    return fmt::format("This is {0} {1} ({2})", name, version, Utils::GetMiKTeXBannerString());
}

void miktex_print_banner(FILE* file, const char* name, const char* version)
{
    fprintf(file, "%s\n", GetBanner(name, version).c_str());
}

int miktex_emulate__do_spawn(void* mpx, const char* fileName, char* const* argv)
{
    vector<string> arguments;
    for (; *argv != nullptr; ++argv)
    {
        arguments.push_back(*argv);
    }
    try
    {
        int exitCode;
        Process::Run(PathName(fileName), arguments, nullptr, &exitCode, nullptr);
        return exitCode;
    }
    catch (const MiKTeXException&)
    {
        errno = ENOENT;
        return -1;
    }
}
