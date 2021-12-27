/* topics/filesystem/topic.cpp:

   Copyright (C) 2021 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#include <string>
#include <memory>

#include "internal.h"

#include "commands/commands.h"

#include "topic.h"

class FileSystemTopic :
    public OneMiKTeXUtility::Topics::TopicBase
{
    std::string Description() override
    {
        return T_("Commands for watching the file system");
    }

    std::string Name() override
    {
        return "filesystem";
    }

    void RegisterCommands() override
    {
        this->RegisterCommand(OneMiKTeXUtility::Topics::FileSystem::Commands::Watch());
    }
};

std::unique_ptr<OneMiKTeXUtility::Topics::Topic> OneMiKTeXUtility::Topics::FileSystem::Create()
{
    return std::make_unique<FileSystemTopic>();
}
