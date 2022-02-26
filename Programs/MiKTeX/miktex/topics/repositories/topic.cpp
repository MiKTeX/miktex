/**
 * @file topics/repositories/topic.cpp
 * @author Christian Schenk
 * @brief repositories topic
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
    class RepositoriesTopic :
        public OneMiKTeXUtility::Topics::TopicBase
    {
        std::string Description() override
        {
            return T_("Commands for managing MiKTeX package repositories");
        }

        std::string Name() override
        {
            return "repositories";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Topics::Repositories::Commands::List());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::Repositories::Create()
{
    return std::make_unique<RepositoriesTopic>();
}
