/**
 * @file topics/formats/topic.cpp
 * @author Christian Schenk
 * @brief formats topic
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
    class FormatsTopic :
        public OneMiKTeXUtility::Topics::TopicBase
    {
        std::string Description() override
        {
            return T_("Commands for managing TeX formats and METAFONT bases");
        }

        std::string Name() override
        {
            return "formats";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Topics::Formats::Commands::Build());
            this->RegisterCommand(OneMiKTeXUtility::Topics::Formats::Commands::List());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::Formats::Create()
{
    return std::make_unique<FormatsTopic>();
}
