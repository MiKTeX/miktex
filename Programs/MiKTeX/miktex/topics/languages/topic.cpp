/**
 * @file topics/languages/topic.cpp
 * @author Christian Schenk
 * @brief languages topic
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
    class LanguagesTopic :
        public OneMiKTeXUtility::Topics::TopicBase
    {
        std::string Description() override
        {
            return T_("Commands for managing LaTeX language definitions");
        }

        std::string Name() override
        {
            return "languages";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Topics::Languages::Commands::Configure());
            this->RegisterCommand(OneMiKTeXUtility::Topics::Languages::Commands::List());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::Languages::Create()
{
    return std::make_unique<LanguagesTopic>();
}
