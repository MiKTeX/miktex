/**
 * @file runperl.cpp
 * @author Christian Schenk
 * @brief running scripts
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Paths>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

string GetScriptName(const PathName& path)
{
#if defined(MIKTEX_WINDOWS)
    if (path.HasExtension(".exe"))
    {
        return path.GetFileNameWithoutExtension().ToString();
    }
#endif
    return path.GetFileName().ToString();
}

tuple<PathName, vector<string>, vector<string>> SessionImpl::GetScript(const string &scriptEngine, const string &name)
{
    PathName scriptsIni;
    if (!FindFile(MIKTEX_PATH_SCRIPTS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptsIni))
    {
        MIKTEX_FATAL_ERROR_2(T_("The '{filename}' configuration file could not be found."), "filename", MIKTEX_SCRIPTS_INI_FILENAME);
    }
    unique_ptr<Cfg> config(Cfg::Create());
    config->Read(scriptsIni, true);
    string value;
    if (!config->TryGetValueAsString(scriptEngine, name, value))
    {
        MIKTEX_FATAL_ERROR_2(T_("The '{filename}' configuration file contains no record for '{name}'."), "filename", MIKTEX_SCRIPTS_INI_FILENAME, "engine", scriptEngine, "name", name);
    }
    string relScriptPath = Expand(value);
    PathName scriptPath;
    if (!FindFile(relScriptPath, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptPath))
    {
        MIKTEX_FATAL_ERROR_2(T_("The program '{name}' could not be found."), "engine", scriptEngine, "name", name, "path", relScriptPath);
    }
    vector<string> args{scriptEngine};
    vector<string> scriptEngineOptions;
    if (!config->TryGetValueAsStringVector(scriptEngine, name + "." + scriptEngine + "." + "options" + "[]", scriptEngineOptions))
    {
        scriptEngineOptions.clear();
    }
    vector<string> scriptOptions;
    if (!config->TryGetValueAsStringVector(scriptEngine, name + "." + "options" + "[]", scriptOptions))
    {
        scriptOptions.clear();
    }
    return make_tuple(scriptPath, scriptEngineOptions, scriptOptions);
}

int SessionImpl::RunScript(const string &scriptEngine, const vector<string> &lastScriptEngineOptions, int argc, const char **argv)
{
    MIKTEX_ASSERT(argc > 0);

    string name = GetScriptName(PathName(argv[0]));

    PathName scriptEnginePath;
    if (!Utils::FindProgram(scriptEngine, scriptEnginePath))
    {
        MIKTEX_FATAL_ERROR_5(
            T_("The script engine could not be found."),
            T_("MiKTeX could not find the script engine '{scriptEngine}' which is required to execute '{scriptName}'."),
            T_("Make sure '{scriptEngine}' is installed on your system."),
            "script-engine-not-found",
            "scriptEngine", scriptEngine,
            "scriptName", name);
    }

    PathName scriptPath;
    vector<string> scriptEngineOptions;
    vector<string> scriptOptions;
    tie(scriptPath, scriptEngineOptions, scriptOptions) = GetScript(scriptEngine, name);

#if defined(MIKTEX_WINDOWS)
    bool isCmd = scriptEnginePath.HasExtension(".bat") || scriptEnginePath.HasExtension(".cmd");
#endif

    vector<string> args;

#if defined(MIKTEX_WINDOWS)
    args.push_back(isCmd ? scriptEnginePath.ToString() : scriptEngine);
#else
    args.push_back(scriptEngine);
#endif

    if (!scriptEngineOptions.empty())
    {
        args.insert(args.end(), scriptEngineOptions.begin(), scriptEngineOptions.end());
    }

    if (!lastScriptEngineOptions.empty())
    {
        args.insert(args.end(), lastScriptEngineOptions.begin(), lastScriptEngineOptions.end());
    }

    args.push_back(scriptPath.ToUnix().ToString());

    if (!scriptOptions.empty())
    {
        args.insert(args.end(), scriptOptions.begin(), scriptOptions.end());
    }

    if (argc > 1)
    {
        args.insert(args.end(), &argv[1], &argv[argc]);
    }

    int exitCode;

#if defined(MIKTEX_WINDOWS)
    if (isCmd)
    {
        CommandLineBuilder commandLine;
        commandLine.SetQuotingConvention(QuotingConvention::Bat);
        commandLine.AppendArguments(args);
        Process::ExecuteSystemCommand(commandLine.ToString(), &exitCode);
        return exitCode;
    }
#endif

    Process::Run(scriptEnginePath, args, nullptr, &exitCode, nullptr);

    return exitCode;
}

int SessionImpl::RunPerl(int argc, const char **argv)
{
    MIKTEX_ASSERT(argc > 0);
    PathName scriptPath;
    vector<string> scriptEngineOptions;
    vector<string> scriptOptions;
    tie(scriptPath, scriptEngineOptions, scriptOptions) = GetScript("perl", GetScriptName(PathName(argv[0])));
    return RunScript("perl", {fmt::format("-I{0}", scriptPath.GetDirectoryName().ToUnix().ToString())}, argc, argv);
}

int SessionImpl::RunPython(int argc, const char **argv)
{
    return RunScript("python", {}, argc, argv);
}

int SessionImpl::RunJava(int argc, const char **argv)
{
    return RunScript("java", {"-jar"}, argc, argv);
}
