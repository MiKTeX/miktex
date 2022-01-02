/**
 * @file shims/updmap.cpp
 * @author Christian Schenk
 * @brief updmap shim
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

#include "updmap.h"

using namespace std;

using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;

enum Option
{
    OPT_AAA = 1,
    OPT_NOHASH,
    OPT_NOMKMAP,
    OPT_QUIET,
    OPT_SETOPTION,
    OPT_SHOWOPTION,
    OPT_SYS,
    OPT_USER,
};

static const struct poptOption aoption[] =
{
    { "nohash", 0, POPT_ARG_NONE, nullptr, OPT_NOHASH, nullptr, nullptr },
    { "nomkmap", 0, POPT_ARG_NONE, nullptr, OPT_NOMKMAP, nullptr, nullptr },
    { "quiet", 0, POPT_ARG_NONE, nullptr, OPT_QUIET, nullptr, nullptr },
    { "setoption", 0, POPT_ARG_STRING, nullptr, OPT_SETOPTION, nullptr, nullptr },
    { "showoption", 0, POPT_ARG_STRING, nullptr, OPT_SHOWOPTION, nullptr, nullptr },
    { "sys", 0, POPT_ARG_NONE, nullptr, OPT_SYS, nullptr, nullptr },
    { "user", 0, POPT_ARG_NONE, nullptr, OPT_USER, nullptr, nullptr },
    POPT_AUTOHELP
    POPT_TABLEEND
};

void Shims::updmap(OneMiKTeXUtility::ApplicationContext* ctx, vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], aoption);
    int option;
    bool optSetOption = false;
    bool optShowOption = false;
    string optionName;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_NOHASH:
            // NOOP
            break;
        case OPT_NOMKMAP:
            // NOOP
            break;
        case OPT_QUIET:
            // NOOP
            break;
        case OPT_SETOPTION:
            optSetOption = true;
            optionName = popt.GetOptArg();
            break;
        case OPT_SHOWOPTION:
            optShowOption = true;
            optionName = popt.GetOptArg();
            break;
        case OPT_SYS:
            if (!ctx->session->IsSharedSetup())
            {
                ctx->ui->FatalError(T_("option --sys only makes sense for a shared MiKTeX setup"));
            }
            if (!ctx->session->RunningAsAdministrator())
            {
               ctx->ui->Warning(T_("option --sys may require administrator privileges"));
            }
            ctx->session->SetAdminMode(true, false);
            break;
        case OPT_USER:
            // NOOP
            break;
        }
    }
    if (option != -1)
    {
        ctx->ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    auto leftovers = popt.GetLeftovers();
    if (optShowOption)
    {
        if (!leftovers.empty())
        {
            ctx->ui->IncorrectUsage(T_("unexpected leftover arguments"));
        }
        arguments = { "fontmaps", "show-option", "--name", optionName };
    }
    else if (optSetOption)
    {
        if (leftovers.size() != 1)
        {
            ctx->ui->IncorrectUsage(T_("expected arguments: OPTION VALUE"));
        }
        arguments = { "fontmaps", "set-option", "--name", optionName, "--value", leftovers[0] };
    }
    else
    {
        arguments = { "fontmaps", "configure" };
    }
}
