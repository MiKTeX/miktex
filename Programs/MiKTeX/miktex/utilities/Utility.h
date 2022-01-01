/**
 * @file utilities/Utility.cpp
 * @author Christian Schenk
 * @brief Utility interface
 *
 * @copyright Copyright © 2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#pragma once

#include <string>
#include <map>
#include <vector>

#include <miktex/Definitions>

#include "internal.h"

#include "Command.h"

namespace OneMiKTeXUtility::Utilities
{
    class MIKTEXNOVTABLE Utility
    {
    public:
        virtual MIKTEXTHISCALL ~Utility() noexcept = 0;
        virtual std::string Description() = 0;
        virtual int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) = 0;
        virtual std::string Name() = 0;
    };

    class UtilityBase :
        public Utility
    {
    protected:
        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;
        void RegisterCommand(std::unique_ptr<OneMiKTeXUtility::Utilities::Command> c)
        {
            auto name = c->Name();
            this->commands[name] = std::move(c);
        }
        virtual void RegisterCommands() = 0;
        std::map<std::string, std::unique_ptr<OneMiKTeXUtility::Utilities::Command>> commands;
    };
}
