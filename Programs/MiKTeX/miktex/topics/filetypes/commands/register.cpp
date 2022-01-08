/**
 * @file topics/filetypes/commands/register.cpp
 * @author Christian Schenk
 * @brief filetypes register
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
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

#include "internal.h"

#include "commands.h"

#include "FileTypeManager.h"

namespace
{
    class RegisterCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Register Windows file types");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "register";
        }

        std::string Synopsis() override
        {
            return "register";
        }
    };
}

using namespace std;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FileTypes;

unique_ptr<Command> Commands::Register()
{
    return make_unique<RegisterCommand>();
}

int RegisterCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() != 2)
    {
        ctx.ui->IncorrectUsage(T_("unexpected command arguments"));
    }
    FileTypeManager mgr;
    mgr.Init(ctx);
    mgr.Register();
    return 0;
}
