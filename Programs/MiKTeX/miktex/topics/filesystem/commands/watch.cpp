/**
 * @file topics/filesystem/commands/watch.cpp
 * @author Christian Schenk
 * @brief filesystem watch
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

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

namespace
{
    class WatchCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Watch for changes in a directory");
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
}

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
