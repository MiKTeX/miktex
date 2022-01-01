/**
 * @file utilities/languages/topic.cpp
 * @author Christian Schenk
 * @brief languages utility
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
    class LanguagesUtility :
        public OneMiKTeXUtility::Utilities::UtilityBase
    {
        std::string Description() override
        {
            return T_("Commands for managing TeX languages");
        }

        std::string Name() override
        {
            return "languages";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Utilities::Languages::Commands::Update());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Utilities::Utility> OneMiKTeXUtility::Utilities::Languages::Create()
{
    return std::make_unique<LanguagesUtility>();
}
