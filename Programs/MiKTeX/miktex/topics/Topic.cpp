/* topics/Topic.cpp:

   Copyright (C) 2021 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

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
        ctx.ui->BadUsage(T_("missing command; try help"), "");
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
            ctx.ui->Output(fmt::format("  {0} {1}", c.second->Synopsis(), c.second->Description()));
        }
        return 0;
    }
    auto& cmd = this->commands[arguments[1]];
    if (cmd == nullptr)
    {
        ctx.ui->BadUsage(fmt::format(T_("{0}: unknown command"), arguments[1]), "");
    }
    cmd->Execute(ctx, arguments);
}
