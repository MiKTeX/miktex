/**
 * @file utilities/fontmaps/topic.cpp
 * @author Christian Schenk
 * @brief fontmaps utility
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
    class FontMapsUtility :
        public OneMiKTeXUtility::Utilities::UtilityBase
    {
        std::string Description() override
        {
            return T_("Commands for managing TeX font map files");
        }

        std::string Name() override
        {
            return "fontmaps";
        }

        void RegisterCommands() override
        {
            this->RegisterCommand(OneMiKTeXUtility::Utilities::FontMaps::Commands::SetOption());
            this->RegisterCommand(OneMiKTeXUtility::Utilities::FontMaps::Commands::ShowOption());
            this->RegisterCommand(OneMiKTeXUtility::Utilities::FontMaps::Commands::Update());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Utilities::Utility> OneMiKTeXUtility::Utilities::FontMaps::Create()
{
    return std::make_unique<FontMapsUtility>();
}
