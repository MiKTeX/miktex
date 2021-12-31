/**
 * @file topics/filetypes/topic.cpp
 * @author Christian Schenk
 * @brief filetypes topic
 *
 * @copyright Copyright © 2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <string>
#include <memory>

#include "internal.h"

#include "commands/commands.h"

#include "topic.h"

namespace
{
    class FileTypesTopic :
        public OneMiKTeXUtility::Topics::TopicBase
    {
        std::string Description() override
        {
            return T_("Commands for managing Windows shell file types");
        }

        std::string Name() override
        {
            return "filetypes";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Topics::FileTypes::Commands::Register());
            this->RegisterCommand(OneMiKTeXUtility::Topics::FileTypes::Commands::Unregister());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::FileTypes::Create()
{
    return std::make_unique<FileTypesTopic>();
}
