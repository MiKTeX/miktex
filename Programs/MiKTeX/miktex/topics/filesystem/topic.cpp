/**
 * @file topics/filesystem/topic.cpp
 * @author Christian Schenk
 * @brief filesystem topic
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
    class FileSystemTopic :
        public OneMiKTeXUtility::Topics::TopicBase
    {
        std::string Description() override
        {
            return T_("Commands for watching the file system");
        }

        std::string Name() override
        {
            return "filesystem";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Topics::FileSystem::Commands::Watch());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::FileSystem::Create()
{
    return std::make_unique<FileSystemTopic>();
}
