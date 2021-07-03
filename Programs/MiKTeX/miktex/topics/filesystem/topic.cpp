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

#include "commands/commands.h"
#include "topic.h"

#include <miktex/Core/Text>

using namespace std;

using namespace Topics;
using namespace Topics::FileSystem;
using namespace Topics::FileSystem::Commands;

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
    int MIKTEXTHISCALL Execute(const std::vector<std::string>& arguments) override;

private:
    std::string Name() override
    {
        return "filesystem";
    }

private:
    void BadUsage(const std::string s)
    {
        cerr << fmt::format(T_("Bad usage: {0}"), s) << endl;
    }
};

unique_ptr<Topic> Topics::FileSystem::Create()
{
    return make_unique<FileSystemTopic>();
}

int FileSystemTopic::Execute(const vector<string>& arguments)
{
    if (arguments.size() < 2)
    {
        BadUsage(T_("missing command"));
        return 1;
    }
    if (arguments[1] == "help")
    {
        return 0;
    }
    if (arguments[1] == "watch")
    {
        return Watch(arguments);
    }
    BadUsage(fmt::format(T_("unknown filesystem command: {0}"), arguments[0]));
    return 1;
}
