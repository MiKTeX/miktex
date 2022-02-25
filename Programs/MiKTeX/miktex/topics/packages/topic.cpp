/**
 * @file topics/packages/topic.cpp
 * @author Christian Schenk
 * @brief packages topic
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
#include <memory>

#include "internal.h"

#include "commands/commands.h"

#include "topic.h"

namespace
{
    class PackagesTopic :
        public OneMiKTeXUtility::Topics::TopicBase
    {
        std::string Description() override
        {
            return T_("Commands for managing MiKTeX packages");
        }

        std::string Name() override
        {
            return "packages";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Topics::Packages::Commands::Info());
            this->RegisterCommand(OneMiKTeXUtility::Topics::Packages::Commands::Install());
            this->RegisterCommand(OneMiKTeXUtility::Topics::Packages::Commands::List());
            this->RegisterCommand(OneMiKTeXUtility::Topics::Packages::Commands::Remove());
            this->RegisterCommand(OneMiKTeXUtility::Topics::Packages::Commands::Require());
            this->RegisterCommand(OneMiKTeXUtility::Topics::Packages::Commands::Update());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::Packages::Create()
{
    return std::make_unique<PackagesTopic>();
}
