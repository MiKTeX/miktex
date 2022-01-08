/**
 * @file topics/links/topic.cpp
 * @author Christian Schenk
 * @brief links topic
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
#include <memory>

#include "internal.h"

#include "commands/commands.h"

#include "topic.h"

namespace
{
    class LinksTopic :
        public OneMiKTeXUtility::Topics::TopicBase
    {
        std::string Description() override
        {
            return T_("Commands for managing links from scripts and formats to executables");
        }

        std::string Name() override
        {
            return "links";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Topics::Links::Commands::Install());
            this->RegisterCommand(OneMiKTeXUtility::Topics::Links::Commands::List());
            this->RegisterCommand(OneMiKTeXUtility::Topics::Links::Commands::Uninstall());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::Links::Create()
{
    return std::make_unique<LinksTopic>();
}
