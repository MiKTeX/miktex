/**
 * @file alias.cpp
 * @author Christian Schenk
 * @brief Function wrapper
 *
 * @copyright Copyright © 1991-2023 Christian Schenk
 *
 * This file is free software.
 *
 * This file is licensed under GNU General Public License version 2 or any later
 * version.
 */

#include <iostream>
#include <memory>

#if defined(MIKTEX_WINDOWS)
#include <Windows.h>
#include <VersionHelpers.h>
#endif

#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/Session>
#include <miktex/Definitions>
#include <miktex/Util/StringUtil>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/ConsoleCodePageSwitcher>
#endif

#if !defined(FUNC)
#define FUNC c4pmain
#endif

#define T_(x) MIKTEXTEXT(x)

extern "C" int MIKTEXCEECALL FUNC(int argc, char* argv[]);

#if defined(_UNICODE)
#define WRAPPER_MAIN wmain
#define WRAPPER_CHAR wchar_t
#else
#define WRAPPER_MAIN main
#define WRAPPER_CHAR char
#endif

int MIKTEXCEECALL WRAPPER_MAIN(int argc, WRAPPER_CHAR* argv[])
{
#if defined(MIKTEX_WINDOWS)
    if (!IsWindows10OrGreater())
    {
        std::cerr << T_("This application requires Windows 10 (or greater).") << std::endl;
        return 1;
    }
    MiKTeX::Core::ConsoleCodePageSwitcher cpSwitcher;
    std::vector<std::string> utf8args;
    utf8args.reserve(argc);
#endif
    std::vector<char*> args;
    args.reserve(argc + 1);
    for (int idx = 0; idx < argc; ++idx)
    {
#if defined(MIKTEX_WINDOWS)
#if defined(_UNICODE)
        utf8args.push_back(MiKTeX::Util::StringUtil::WideCharToUTF8(argv[idx]));
#else
        utf8args.push_back(MiKTeX::Util::StringUtil::AnsiToUTF8(argv[idx]));
#endif
        args.push_back(const_cast<char*>(utf8args[idx].c_str()));
#else
        args.push_back(argv[idx]);
#endif
    }
    args.push_back(nullptr);
    return FUNC(argc, &args[0]);
}
