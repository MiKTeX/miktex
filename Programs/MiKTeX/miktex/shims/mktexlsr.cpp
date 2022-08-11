/**
 * @file shims/mktexlsr.cpp
 * @author Christian Schenk
 * @brief mktexlsr shim
 *
 * @copyright Copyright © 2022 Christian Schenk
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

#include "mktexlsr.h"

using namespace std;

using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;

enum Option
{
    OPT_AAA = 1,
};

static const struct poptOption aoption[] =
{
    POPT_AUTOHELP
    POPT_TABLEEND
};

void Shims::mktexlsr(OneMiKTeXUtility::ApplicationContext* ctx, vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], aoption);
    int option;
    arguments = { arguments[0], "fndb", "refresh" };
    while ((option = popt.GetNextOpt()) >= 0)
    {
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
