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

#include <thread>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/FileSystemWatcher>
#include <miktex/Core/Text>
#include <miktex/Util/PathName>

#include "commands.h"
#include "internal.h"

#define T_(x) MIKTEXTEXT(x)

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;

int Topics::FileSystem::Commands::Watch(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() != 3)
    {
        ctx.ui->Error(fmt::format(T_("Usage: {0} {1} DIR"), arguments[0], arguments[1]));
        return 1;
    }
    PathName dir(arguments[2]);
    if (!Directory::Exists(dir))
    {
        ctx.ui->Error(fmt::format(T_("no such directory: {0}"), dir));
        return 1;
    }
    auto fsWatcher = FileSystemWatcher::Create();
    class Callback : public FileSystemWatcherCallback{
        public:
            Callback(OneMiKTeXUtility::ApplicationContext& ctx) : ctx(ctx) {}
        private :
            void OnChange(const FileSystemChangeEvent& ev) override {
                string action;
                switch (ev.action)
                {
                    case FileSystemChangeAction::Added: action = "added"; break;
                    case FileSystemChangeAction::Modified: action = "modified"; break;
                    case FileSystemChangeAction::Removed: action = "removed"; break;
                    default: action = "-"; break;
                }
                ctx.ui->Output(fmt::format("{0}: {1}\n"));
            }
        private:
            OneMiKTeXUtility::ApplicationContext& ctx;
    };
    Callback callback(ctx);
    fsWatcher->AddDirectories({dir});
    fsWatcher->Subscribe(&callback);
    fsWatcher->Start();
    while (!ctx.controller->Canceled())
    {
        this_thread::sleep_for(2000ms);
    }
    return 0;
}
