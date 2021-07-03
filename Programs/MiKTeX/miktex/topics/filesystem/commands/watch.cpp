/* topics/filesystem/commands/watch.cpp:

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

#include <miktex/Core/Directory>
#include <miktex/Core/FileSystemWatcher>
#include <miktex/Core/Text>
#include <miktex/Util/PathName>

#include "commands.h"

#define T_(x) MIKTEXTEXT(x)

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

int Topics::FileSystem::Commands::Watch(const vector<string>& arguments)
{
    if (arguments.size() != 3)
    {
        cerr << fmt::format(T_("Usage: {0} {1} DIR"), arguments[0], arguments[1]) << endl;
        return 1;
    }
    PathName dir(arguments[2]);
    if (!Directory::Exists(dir))
    {
        cerr << fmt::format(T_("no such directory: {0}"), dir);
        return 1;
    }
    auto fsWatcher = FileSystemWatcher::Create();
    class Callback : public FileSystemWatcherCallback{
        private :
            void OnChange(const FileSystemChangeEvent& ev) override {
                cout << ev.fileName << endl;
            }
    };
    Callback callback;
    fsWatcher->AddDirectories({dir});
    fsWatcher->Subscribe(&callback);
    fsWatcher->Start();
    // TODO: wait SIGINT
    return 0;
}
