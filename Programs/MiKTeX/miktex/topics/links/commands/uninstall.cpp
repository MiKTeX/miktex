/**
 * @file topics/links/commands/uninstall.cpp
 * @author Christian Schenk
 * @brief links uninstall
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "internal.h"

#include "commands.h"

#include "LinksManager.h"

namespace
{
    class UninstallCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Uninstall all links");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "uninstall";
        }

        std::string Synopsis() override
        {
            return "uninstall";
        }
    };
}

using namespace std;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Links;

unique_ptr<Command> Commands::Uninstall()
{
    return make_unique<UninstallCommand>();
}

int UninstallCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (ctx.session->IsSharedSetup() && !ctx.session->IsAdminMode())
    {
        ctx.ui->FatalError(T_("this command must be run in admin mode"));
    }
    if (arguments.size() != 2)
    {
        ctx.ui->IncorrectUsage(T_("unexpected command arguments"));
    }
    LinksManager mgr;
    mgr.Init(ctx);
    mgr.Uninstall();
    return 0;
}
