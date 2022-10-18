/**
 * @file shims/mkfntmap.cpp
 * @author Christian Schenk
 * @brief mkfntmap shim
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <config.h>

#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "mkfntmap.h"

using namespace std;

using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;

enum Option
{
    OPT_AAA = 1,
    OPT_FORCE,
    OPT_OUTPUT_DIRECTORY,
};

static const struct poptOption aoption[] =
{
    {
        "force", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_FORCE, nullptr, nullptr
    },
    {
        "output-directory", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_OUTPUT_DIRECTORY, nullptr
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

void Shims::mkfntmap(OneMiKTeXUtility::ApplicationContext* ctx, vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], aoption);
    int option;
    arguments = { arguments[0], "fontmaps", "configure" };
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_FORCE:
            arguments.push_back("--force");
            break;
        case OPT_OUTPUT_DIRECTORY:
            arguments.push_back("--output-directory");
            arguments.push_back(popt.GetOptArg());
            break;
        }
    }
    if (option != -1)
    {
        ctx->ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    if (!popt.GetLeftovers().empty())
    {
        ctx->ui->IncorrectUsage(T_("unexpected leftover arguments"));
    }
}
