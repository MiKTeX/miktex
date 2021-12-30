/**
 * @file topics/filetypes/commands/unregister.cpp
 * @author Christian Schenk
 * @brief filetypes register
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

#include "FileTypeManager.h"

class UnegisterCommand :
    public OneMiKTeXUtility::Topics::Command
{
    std::string Description() override
    {
        return T_("Unregister shell file types");
    }

    int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

    std::string Name() override
    {
        return "unregister";
    }

    std::string Synopsis() override
    {
        return "unregister";
    }
};

using namespace std;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FileTypes;

unique_ptr<Command> Commands::Unregister()
{
    return make_unique<UnegisterCommand>();
}

int UnegisterCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() != 2)
    {
        ctx.ui->IncorrectUsage(T_("expected no arguments"));
    }
    FileTypeManager mgr;
    mgr.Init(ctx);
    mgr.Unregister();
    return 0;
}
