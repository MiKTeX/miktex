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

#include <config.h>

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/FileSystemWatcher>
#include <miktex/Util/PathName>
#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

namespace
{
    class WatchCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Watch for changes in directories");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "watch";
        }

        std::string Synopsis() override
        {
            return "watch [--template <template>] <directory>";
        }

        const std::string defaultTemplate = "{action} {fileName}";
    };
}

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FileSystem;

unique_ptr<Command> Commands::Watch()
{
    return make_unique<WatchCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_TEMPLATE,
};

static const struct poptOption options[] =
{
    {
        "template", 0,
        POPT_ARG_STRING, nullptr,
        OPT_TEMPLATE,
        T_("Specify the output template."),
        "TEMPLATE"
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

int WatchCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string outputTemplate = this->defaultTemplate;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_TEMPLATE:
            outputTemplate = Unescape(popt.GetOptArg());
            break;
        }
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    auto leftOvers = popt.GetLeftovers();
    if (leftOvers.size() != 1)
    {
        ctx.ui->IncorrectUsage(T_("expected one <directory> argument"));
    }
    PathName dir(leftOvers[0]);
    if (!Directory::Exists(dir))
    {
        ctx.ui->FatalError(fmt::format(T_("{0}: directory does not exist"), dir));
    }
    auto fsWatcher = FileSystemWatcher::Create();
    class Callback : public FileSystemWatcherCallback
    {
        void OnChange(const FileSystemChangeEvent& ev) override
        {
            ctx->ui->Output(fmt::format(outputTemplate,
                fmt::arg("action", ev.action),
                fmt::arg("fileName", ev.fileName)
            ));
        }
    public:
        OneMiKTeXUtility::ApplicationContext* ctx;
        string outputTemplate;
    };
    Callback callback;
    callback.ctx = &ctx;
    callback.outputTemplate = outputTemplate;
    fsWatcher->AddDirectories({dir});
    fsWatcher->Subscribe(&callback);
    fsWatcher->Start();
    while (!ctx.program->Canceled())
    {
        this_thread::sleep_for(200ms);
    }
    return 0;
}
