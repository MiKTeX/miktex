/**
 * @file wrapper.cpp
 * @author Christian Schenk
 * @brief Wrap a main function
 *
 * @copyright Copyright © 2004-2023 Christian Schenk
 *
 * This file is free software.
 *
 * This file is licensed under GNU General Public License version 2 or any later
 * version.
 */

#if defined(HAVE_STDAFX_H)
#include "StdAfx.h"
#endif

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <iostream>

#include <cstdlib>

#if defined(MIKTEX_WINDOWS)
#include <Windows.h>
#include <VersionHelpers.h>
#endif

#include <miktex/Core/Exceptions>
#include <miktex/Util/StringUtil>
#include <miktex/App/Application>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/ConsoleCodePageSwitcher>
#endif

#include <vector>

#if !defined(stringify_)
#define stringify__(x) #x
#define stringify_(x) stringify__(x)
#endif

#if defined(APPTAG)
#define APPTAGSTR stringify_(APPTAG)
#endif

#if !defined(MAINFUNC)
#define MAINFUNC Main
#endif

#if defined(CPLUSPLUSMAIN)
#define EXTERN_C
#else
#define EXTERN_C extern "C"
#endif

EXTERN_C int MIKTEXCEECALL MAINFUNC(int argc, char** argv);

// Keep the application object in the global scope (C functions might
// call exit())
static MiKTeX::App::Application app;

static std::string nameOfTheGame;

#if defined(main) && !defined(_UNICODE)
#undef main
#endif

#if defined(_UNICODE)
#define WRAPPER_MAIN wmain
#define WRAPPER_CHAR wchar_t
#else
#define WRAPPER_MAIN main
#define WRAPPER_CHAR char
#endif

#define T_(x) MIKTEXTEXT(x)

int MIKTEXCEECALL WRAPPER_MAIN(int argc, WRAPPER_CHAR* argv[])
{
#if defined(MIKTEX_WINDOWS)
    if (!IsWindows10OrGreater())
    {
        std::cerr << T_("MiKTeX requires Windows 10 (or greater): https://miktex.org/announcement/legacy-windows-deprecation") << std::endl;
#if !defined(MIKTEX_SUPPORT_LEGACY_WINDOWS)
        return 1;
#endif
    }
    MiKTeX::Core::ConsoleCodePageSwitcher cpSwitcher;
#endif
    try
    {
#if defined(MIKTEX_WINDOWS)
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
            // FIXME: eliminate const cast
            args.push_back(const_cast<char*>(utf8args[idx].c_str()));
#else
            args.push_back(argv[idx]);
#endif
        }
        args.push_back(nullptr);

        nameOfTheGame = args[0];

        app.Init(args);

#if defined(APPTAGSTR)
        app.GetSession()->PushBackAppName(APPTAGSTR);
#endif

#if defined(DISABLE_INSTALLER)
        app.EnableInstaller(MiKTeX::Configuration::TriState::False);
#endif

#if defined(BEQUIET)
        app.SetQuietFlag(true);
#endif

        int exitCode = MAINFUNC(args.size() - 1, &args[0]);

        app.Finalize2(exitCode);

        return exitCode;
    }
    catch (const MiKTeX::Core::MiKTeXException& ex)
    {
        app.Sorry(nameOfTheGame, ex);
        app.Finalize2(EXIT_FAILURE);
        ex.Save();
        return EXIT_FAILURE;
    }
    catch (const std::exception& ex)
    {
        app.Sorry(nameOfTheGame, ex);
        app.Finalize2(EXIT_FAILURE);
        return EXIT_FAILURE;
    }
    catch (int exitCode)
    {
        app.Finalize2(exitCode);
        return exitCode;
    }
}
