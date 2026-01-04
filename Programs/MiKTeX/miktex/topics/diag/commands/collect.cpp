/**
 * @file topics/diag/commands/collect.cpp
 * @author Christian Schenk
 * @brief diag collect command
 *
 * @copyright Copyright © 2026 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <config.h>

#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Configuration/ConfigurationProvider>
#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Setup/SetupService>
#include <miktex/Util/PathName>
#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

namespace
{
    class CollectCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Collect diagnostic information into a tar.bz2 archive");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "collect";
        }

        std::string Synopsis() override
        {
            return "collect [--output <file>]";
        }
    };
}

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Diagnostics;

unique_ptr<Command> Commands::Collect()
{
    return make_unique<CollectCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_OUTPUT,
};

static const struct poptOption options[] =
{
    {
        "output", 0,
        POPT_ARG_STRING, nullptr,
        OPT_OUTPUT,
        T_("Specify the output file (default: miktex-diag.tar.bz2)"),
        "FILE"
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

int CollectCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string outputFileName = "miktex-diag.tar.bz2";
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_OUTPUT:
            outputFileName = popt.GetOptArg();
            break;
        }
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    if (!popt.GetLeftovers().empty())
    {
        ctx.ui->IncorrectUsage(T_("unexpected command arguments"));
    }
    auto setupService = SetupService::Create();
    ctx.ui->Verbose(1, T_("Collecting diagnostic information..."));
    setupService->CollectDiagnosticInfo(PathName(outputFileName));
    if (!ctx.ui->BeingQuiet())
    {
        ctx.ui->Output(fmt::format(T_("Diagnostic information saved in {0}"), outputFileName));
    }
    return 0;
}
