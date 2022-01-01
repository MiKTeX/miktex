/**
 * @file utilities/fndb/topic.cpp
 * @author Christian Schenk
 * @brief fndb utility
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
    class FNDBUtility :
        public OneMiKTeXUtility::Utilities::UtilityBase
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
            this->RegisterCommand(OneMiKTeXUtility::Utilities::FNDB::Commands::Remove());
            this->RegisterCommand(OneMiKTeXUtility::Utilities::FNDB::Commands::Update());
        }
    };
}

std::unique_ptr<OneMiKTeXUtility::Utilities::Utility> OneMiKTeXUtility::Utilities::FNDB::Create()
{
    return std::make_unique<FNDBUtility>();
}
