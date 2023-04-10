/**
 * @file runtexlua.cpp
 * @author Christian Schenk
 * @brief Run a texlua script
 *
 * @copyright Copyright © 2010-2023 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include <miktex/App/Application>
#include <miktex/Core/Cfg>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Paths>

using namespace std;

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

extern "C" int MIKTEXCEECALL Main(int argc, char** argv);

// Keep the application object in the global scope (C functions might
// call exit())
MiKTeX::App::Application app;

#if defined(MTXRUN)
#define CFGKEY "mtxrun"
#else
#define CFGKEY "texlua"
#endif

#if defined(_UNICODE)
#define MAIN wmain
#define MAINCHAR wchar_t
#else
#define MAIN main
#define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR** argv)
{
    try
    {
        // build new argv
        vector<string> utf8args;
        for (int idx = 0; idx < argc; ++idx)
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

        app.Init(newargv);

        // determine script name
        PathName programName = PathName(newargv[0]).GetFileNameWithoutExtension();

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
        PathName scriptsIni;
        if (!app.GetSession()->FindFile(MIKTEX_PATH_SCRIPTS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptsIni))
        {
            MIKTEX_FATAL_ERROR(MIKTEXTEXT("The script configuration file cannot be found."));
        }
        unique_ptr<Cfg> scriptConfig(Cfg::Create());
        scriptConfig->Read(scriptsIni, true);
        std::string relScriptPath;
        if (!scriptConfig->TryGetValueAsString(CFGKEY, scriptName, relScriptPath))
        {
            MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("The Lua script is not registered."), "programName", programName.ToString());
        }
        scriptConfig = nullptr;

        // find script
        PathName scriptPath;
        if (!app.GetSession()->FindFile(relScriptPath, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptPath))
        {
            MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("The Lua script could not be found."), "path", relScriptPath);
        }

        // inject arguments
        vector<char*> extraArgs;
        extraArgs.push_back("--luaonly");
        extraArgs.push_back(scriptPath.GetData());
#if defined(MTXRUN)
        if (!(isLuatools || isMtxrun || isTexmfstart))
        {
            extraArgs.push_back("--script");
            extraArgs.push_back(programName);
        }
#endif
        newargv.insert(newargv.begin() + 1, extraArgs.begin(), extraArgs.end());

        // run texlua
        int exitCode = Main(newargv.size() - 1, &newargv[0]);

        app.Finalize2(exitCode);

        return exitCode;
    }
    catch (const MiKTeXException& e)
    {
        Utils::PrintException(e);
        app.Finalize2(1);
        return 1;
    }
    catch (const std::exception& e)
    {
        Utils::PrintException(e);
        app.Finalize2(1);
        return 1;
    }
    catch (int exitCode)
    {
        app.Finalize2(exitCode);
        return exitCode;
    }
}
