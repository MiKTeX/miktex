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

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/FileSystemWatcher>
#include <miktex/Util/PathName>

#include "internal.h"

#include "commands.h"

class WatchCommand :
    public OneMiKTeXUtility::Topics::Command
{
    std::string Description() override
    {
        return T_("Watch for DIRECTORY changes");
    }

    int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

    std::string Name() override
    {
        return "watch";
    }

    std::string Synopsis() override
    {
        return "watch DIRECTORY";
    }
};

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FileSystem;

unique_ptr<Command> Commands::Watch()
{
    return make_unique<WatchCommand>();
}

int WatchCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() != 3)
    {
        ctx.ui->IncorrectUsage(T_("expected one argument: DIRECTORY"));
    }
    PathName dir(arguments[2]);
    if (!Directory::Exists(dir))
    {
        ctx.ui->FatalError(fmt::format(T_("{0}: directory does not exist"), dir));
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
                ctx.ui->Output(fmt::format("{0}: {1}", action, ev.fileName));
            }
        private:
            OneMiKTeXUtility::ApplicationContext& ctx;
    };
    Callback callback(ctx);
    fsWatcher->AddDirectories({dir});
    fsWatcher->Subscribe(&callback);
    fsWatcher->Start();
    while (!ctx.program->Canceled())
    {
        this_thread::sleep_for(200ms);
    }
    return 0;
}
