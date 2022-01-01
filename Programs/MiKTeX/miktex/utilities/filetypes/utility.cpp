/**
 * @file utilities/filetypes/topic.cpp
 * @author Christian Schenk
 * @brief filetypes utility
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
    class FileTypesUtility :
        public OneMiKTeXUtility::Utilities::UtilityBase
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
            this->RegisterCommand(OneMiKTeXUtility::Utilities::FileTypes::Commands::Register());
            this->RegisterCommand(OneMiKTeXUtility::Utilities::FileTypes::Commands::Unregister());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Utilities::Utility> OneMiKTeXUtility::Utilities::FileTypes::Create()
{
    return std::make_unique<FileTypesUtility>();
}
