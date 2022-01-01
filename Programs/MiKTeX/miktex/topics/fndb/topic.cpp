/**
 * @file topics/fndb/topic.cpp
 * @author Christian Schenk
 * @brief dnsb topic
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
    class FNDBTopic :
        public OneMiKTeXUtility::Topics::TopicBase
    {
        std::string Description() override
        {
            return T_("Commands for managing the file name databases");
        }

        std::string Name() override
        {
            return "fndb";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Topics::FNDB::Commands::Refresh());
            this->RegisterCommand(OneMiKTeXUtility::Topics::FNDB::Commands::Remove());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::FNDB::Create()
{
    return std::make_unique<FNDBTopic>();
}
