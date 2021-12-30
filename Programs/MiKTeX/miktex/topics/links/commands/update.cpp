/**
 * @file topics/links/commands/update.cpp
 * @author Christian Schenk
 * @brief links update
 *
 * @copyright Copyright © 2021 Christian Schenk
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
    class UpdateCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Update all links");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "update";
        }

        std::string Synopsis() override
        {
            return "update";
        }
    };
}

using namespace std;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Links;

unique_ptr<Command> Commands::Update()
{
    return make_unique<UpdateCommand>();
}

int UpdateCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() != 2)
    {
        ctx.ui->IncorrectUsage(T_("expected no arguments"));
    }
    LinksManager mgr;
    mgr.Init(ctx);
    bool force = true; // TODO
    mgr.Update(force);
    return 0;
}
