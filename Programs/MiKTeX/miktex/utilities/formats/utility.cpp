/**
 * @file utilities/formats/topic.cpp
 * @author Christian Schenk
 * @brief formats utility
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

#include "utility.h"

namespace
{
    class FormatsUtility :
        public OneMiKTeXUtility::Utilities::UtilityBase
    {
        std::string Description() override
        {
            return T_("Commands for managing format files");
        }

        std::string Name() override
        {
            return "formats";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Utilities::Formats::Commands::List());
            this->RegisterCommand(OneMiKTeXUtility::Utilities::Formats::Commands::Update());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Utilities::Utility> OneMiKTeXUtility::Utilities::Formats::Create()
{
    return std::make_unique<FormatsUtility>();
}
