/**
 * @file topics/diag/topic.cpp
 * @author Christian Schenk
 * @brief diag topic
 *
 * @copyright Copyright © 2026 Christian Schenk
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
    class DiagnosticsTopic :
        public OneMiKTeXUtility::Topics::TopicBase
    {
        std::string Description() override
        {
            return T_("Commands for collecting diagnostic information");
        }

        std::string Name() override
        {
            return "diag";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Topics::Diagnostics::Commands::Collect());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::Diagnostics::Create()
{
    return std::make_unique<DiagnosticsTopic>();
}
