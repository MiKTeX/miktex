/**
 * @file topics/Command.h
 * @author Christian Schenk
 * @brief Command interface
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
#include <vector>

#include <miktex/Definitions>

#include "internal.h"

namespace OneMiKTeXUtility::Topics
{
    class MIKTEXNOVTABLE Command
    {
    public:
        virtual MIKTEXTHISCALL ~Command() noexcept = 0;
        virtual std::string Description() = 0;
        virtual int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) = 0;
        virtual std::string Name() = 0;
        virtual std::string Synopsis() = 0;
    };
}
