/**
 * @file topics/Topic.cpp
 * @author Christian Schenk
 * @brief Topic interface
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

#include "internal.h"

#include "Topic.h"

using namespace std;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;

Topic::~Topic() noexcept
{
}

int TopicBase::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() < 2)
    {
        ctx.ui->IncorrectUsage(T_("missing command; try help"));
    }
    if (this->commands.empty())
    {
        this->RegisterCommands();
    }
    if (arguments[1] == "help")
    {
        ctx.ui->Output("Commands:");
        for (auto& c : this->commands)
        {
            ctx.ui->Output(fmt::format("  {0}", c.second->Synopsis()));
            ctx.ui->Output(fmt::format("    {0}", c.second->Description()));
        }
        return 0;
    }
    auto& cmd = this->commands[arguments[1]];
    if (cmd == nullptr)
    {
        ctx.ui->IncorrectUsage(fmt::format(T_("{0}: unknown command"), arguments[1]));
    }
    return cmd->Execute(ctx, arguments);
}
