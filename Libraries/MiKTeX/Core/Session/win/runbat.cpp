/**
 * @file runbat.cpp
 * @author Christian Schenk
 * @brief Running Cmd scripts
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <miktex/Core/CommandLineBuilder>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

int SessionImpl::RunBatch(int argc, const char** argv)
{
    MIKTEX_ASSERT(argc > 0);

    PathName name = PathName(argv[0]).GetFileNameWithoutExtension();

    PathName scriptPath;
    vector<string> scriptEngineOptions;
    vector<string> scriptOptions;
    tie(scriptPath, scriptEngineOptions, scriptOptions) = GetScript("bat", name.ToString());

    CommandLineBuilder commandLine;
    commandLine.SetQuotingConvention(QuotingConvention::Bat);
    commandLine.AppendArgument(scriptPath);
    if (argc > 1)
    {
        commandLine.AppendArguments(argc - 1, &argv[1]);
    }

    int exitCode;

    Process::ExecuteSystemCommand(commandLine.ToString(), &exitCode);

    return exitCode;
}
