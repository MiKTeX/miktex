/**
 * @file utilities/links/topic.cpp
 * @author Christian Schenk
 * @brief links utility
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
    class LinksUtility :
        public OneMiKTeXUtility::Utilities::UtilityBase
    {
        std::string Description() override
        {
            return T_("Commands for managing links to MiKTeX executables");
        }

        std::string Name() override
        {
            return "links";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Utilities::Links::Commands::Remove());
            this->RegisterCommand(OneMiKTeXUtility::Utilities::Links::Commands::Update());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Utilities::Utility> OneMiKTeXUtility::Utilities::Links::Create()
{
    return std::make_unique<LinksUtility>();
}
