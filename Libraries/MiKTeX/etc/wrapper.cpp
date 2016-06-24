/* wrapper.cpp: wrap a main function

   Copyright (C) 2004-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#if defined(HAVE_STDAFX_H)
#  include "StdAfx.h"
#endif

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <cstdlib>

#include <miktex/Core/Exceptions>
#include <miktex/Util/StringUtil>
#include <miktex/App/Application>

#include <vector>

#if !defined(stringify_)
#  define stringify__(x) #x
#  define stringify_(x) stringify__(x)
#endif

#if defined(APPTAG)
#  define APPTAGSTR stringify_(APPTAG)
#endif

#if !defined(MAINFUNC)
#  define MAINFUNC Main
#endif

#if defined(CPLUSPLUSMAIN)
#  define EXTERN_C
#else
#  define EXTERN_C extern "C"
#endif

EXTERN_C int MIKTEXCEECALL MAINFUNC(int argc, char ** argv);

// Keep the application object in the global scope (C functions might
// call exit())
static MiKTeX::App::Application app;

static std::string nameOfTheGame;

#if defined(main) && !defined(_UNICODE)
#  undef main
#endif

#if defined(_UNICODE)
#  define WRAPPER_MAIN wmain
#  define WRAPPER_CHAR wchar_t
#else
#  define WRAPPER_MAIN main
#  define WRAPPER_CHAR char
#endif

int MIKTEXCEECALL WRAPPER_MAIN(int argc, WRAPPER_CHAR * argv[])
{
  try
  {
#if defined(MIKTEX_WINDOWS)
    std::vector<std::string> utf8args;
    utf8args.reserve(argc);
#endif
    std::vector<char *> args;
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
    args.push_back(0);

    nameOfTheGame = args[0];

    app.Init(args);

#if defined(APPTAGSTR)
    app.GetSession()->PushBackAppName(APPTAGSTR);
#endif

#if defined(DISABLE_INSTALLER)
    app.EnableInstaller(MiKTeX::Core::TriState::False);
#endif

#if defined(BEQUIET)
    app.SetQuietFlag(true);
#endif

    int exitCode = MAINFUNC(args.size() - 1, &args[0]);

    app.Finalize();

    return exitCode;
  }
  catch (const MiKTeX::Core::MiKTeXException & ex)
  {
    MiKTeX::App::Application::Sorry(nameOfTheGame, ex);
    app.Finalize();
    return EXIT_FAILURE;
  }
  catch (const std::exception & ex)
  {
    MiKTeX::App::Application::Sorry(nameOfTheGame, ex);
    app.Finalize();
    return EXIT_FAILURE;
  }
  catch (int exitCode)
  {
    app.Finalize();
    return exitCode;
  }
}
