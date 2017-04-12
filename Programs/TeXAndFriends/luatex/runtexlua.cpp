/* runtexlua.cpp: run a texlua script

   Copyright (C) 2010-2017 Christian Schenk

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

#include <miktex/App/Application>
#include <miktex/Core/Cfg>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Paths>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace std;

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

extern "C" int MIKTEXCEECALL Main(int argc, char** argv);

// Keep the application object in the global scope (C functions might
// call exit())
MiKTeX::App::Application app;

#if defined(MTXRUN)
#  define CFGKEY "mtxrun"
#else
#  define CFGKEY "texlua"
#endif

#if defined(_UNICODE)
#  define MAIN wmain
#  define MAINCHAR wchar_t
#else
#  define MAIN main
#  define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR** argv)
{
  try
  {
    app.Init(TU_(argv[0]));

    MIKTEX_ASSERT(argc > 0);

    // determine script name
    PathName programName = PathName(argv[0]).GetFileNameWithoutExtension();

    std::string scriptName;

#if defined(MTXRUN)
    bool isLuatools = (PathName::Compare(programName, "luatools") == 0);
    bool isMtxrun = (PathName::Compare(programName, "mtxrun") == 0);
    bool isTexmfstart = (PathName::Compare(programName, "texmfstart") == 0);
    if (isLuatools)
    {
      scriptName = "luatools";
    }
    else
    {
      scriptName = "mtxrun";
    }
#else
    scriptName = programName.ToString();
#endif

    // get relative script path
    PathName scriptsIni = app.GetSession()->GetSpecialPath(SpecialPath::DistRoot);
    scriptsIni /= MIKTEX_PATH_SCRIPTS_INI;
    unique_ptr<Cfg> scriptConfig(Cfg::Create());
    scriptConfig->Read(scriptsIni, true);
    std::string relScriptPath;
    if (!scriptConfig->TryGetValue(CFGKEY, scriptName, relScriptPath))
    {
      MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("The Lua script is not registered."), "programName", programName.ToString());
    }
    scriptConfig = nullptr;

    // find script
    PathName scriptPath;
    if (!app.GetSession()->FindFile(relScriptPath, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptPath))
    {
      MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("The Lua script could not be found."), relScriptPath);
    }

    // build new argv
    vector<string> utf8args;
    utf8args.reserve(argc + 3);
    utf8args.push_back(TU_(argv[0]));
    utf8args.push_back("--luaonly");
    utf8args.push_back(scriptPath.GetData());
#if defined(MTXRUN)
    if (!(isLuatools || isMtxrun || isTexmfstart))
    {
      utf8args.push_back("--script");
      utf8args.push_back(programName);
    }
#endif
    for (int idx = 1; idx < argc; ++idx)
    {
      utf8args.push_back(TU_(argv[idx]));
    }
    vector<char*> newargv;
    newargv.reserve(utf8args.size() + 1);
    for (const string& arg : utf8args)
    {
      newargv.push_back((char*)arg.c_str());
    }
    newargv.push_back(nullptr);

    // run texlua
    int exitCode = Main(newargv.size() - 1, &newargv[0]);

    app.Finalize();

    return exitCode;
  }
  catch (const MiKTeXException& e)
  {
    Utils::PrintException(e);
    app.Finalize();
    return 1;
  }
  catch (const std::exception& e)
  {
    Utils::PrintException(e);
    app.Finalize();
    return 1;
  }
  catch (int exitCode)
  {
    app.Finalize();
    return exitCode;
  }
}
