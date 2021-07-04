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

#include <iostream>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Text>

#include "commands/commands.h"
#include "internal.h"
#include "topic.h"

using namespace std;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FileSystem;

#define T_(x) MIKTEXTEXT(x)

class FileSystemTopic :
    public Topic
{
private:
    std::string Description() override
    {
        return T_("Commands for watching the file system.");
    }

private:
    int MIKTEXTHISCALL Execute(ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

private:
    std::string Name() override
    {
        return "filesystem";
    }

private:
    string BadUsage(const std::string s)
    {
        return fmt::format(T_("Bad usage: {0}"), s);
    }
};

unique_ptr<Topic> OneMiKTeXUtility::Topics::FileSystem::Create()
{
    return make_unique<FileSystemTopic>();
}

int FileSystemTopic::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() < 2)
    {
        ctx.ui->Error(BadUsage(T_("missing command")));
        return 1;
    }
    if (arguments[1] == "help")
    {
        return 0;
    }
    if (arguments[1] == "watch")
    {
        return Commands::Watch(ctx, arguments);
    }
    ctx.ui->Error(BadUsage(fmt::format(T_("unknown filesystem command: {0}"), arguments[0])));
    return 1;
}
