/**
 * @file topics/fontmaps/topic.cpp
 * @author Christian Schenk
 * @brief fontmaps topic
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

class FontMapsTopic :
    public OneMiKTeXUtility::Topics::TopicBase
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
        this->RegisterCommand(OneMiKTeXUtility::Topics::FontMaps::Commands::SetOption());
        this->RegisterCommand(OneMiKTeXUtility::Topics::FontMaps::Commands::ShowOption());
        this->RegisterCommand(OneMiKTeXUtility::Topics::FontMaps::Commands::Update());
    }
};

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::FontMaps::Create()
{
    return std::make_unique<FontMapsTopic>();
}
