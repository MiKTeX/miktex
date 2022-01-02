/**
 * @file topic/formats/commands/FormatsManager.cpp
 * @author Christian Schenk
 * @brief Build TeX format files
 *
 * @copyright Copyright © 2002-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Configuration/ConfigurationProvider>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <miktex/Util/PathName>

#include "internal.h"

#include "FormatsManager.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;

void FormatsManager::Init(ApplicationContext& ctx)
{
    this->ctx = &ctx;
}

void FormatsManager::Build(const string& formatKey)
{
  if (find(this->formatsMade.begin(), this->formatsMade.end(), formatKey) != this->formatsMade.end())
  {
    return;
  }

  auto formatInfo = this->Format(formatKey);

  this->ctx->ui->Verbose(1, fmt::format(T_("Building '{0}' with '{1}'..."), formatInfo.key, formatInfo.compiler));

  string maker;

  vector<string> arguments;

  if (formatInfo.compiler == "mf")
  {
    maker = MIKTEX_MAKEBASE_EXE;
  }
  else
  {
    maker = MIKTEX_MAKEFMT_EXE;
    arguments.push_back("--engine="s + formatInfo.compiler);
  }

  arguments.push_back("--dest-name="s + formatInfo.name);

  if (!formatInfo.preloaded.empty())
  {
    if (PathName::Compare(formatInfo.preloaded, formatKey) == 0)
    {
      this->ctx->ui->FatalError(fmt::format(T_("{0}: rule recursion"), formatKey));
    }
    // RECURSION
    this->Build(formatInfo.preloaded);
    arguments.push_back("--preload="s + formatInfo.preloaded);
  }

  if (PathName(formatInfo.inputFile).HasExtension(".ini"))
  {
    arguments.push_back("--no-dump");
  }

  arguments.push_back(formatInfo.inputFile);

  if (!formatInfo.arguments.empty())
  {
    arguments.push_back("--engine-option="s + formatInfo.arguments);
  }

  this->RunMakeTeX(maker, arguments);

  this->formatsMade.push_back(formatKey);
}

void FormatsManager::RunMakeTeX(const string& makeProg, const vector<string>& arguments)
{
  PathName exe;

  if (!this->ctx->session->FindFile(makeProg, FileType::EXE, exe))
  {
    this->ctx->ui->FatalError(fmt::format(T_("{0}: not be found"), Q_(makeProg)));
  }

  vector<string> xArguments{ makeProg };

  xArguments.insert(xArguments.end(), arguments.begin(), arguments.end());

  if (ctx->ui->VerbosityLevel() > 0)
  {
    xArguments.push_back("--verbose");
  }

  if (this->ctx->ui->BeingQuiet())
  {
    xArguments.push_back("--quiet");
  }

  if (this->ctx->session->IsAdminMode())
  {
    xArguments.push_back("--admin");
  }

  if (this->ctx->installer->IsInstallerEnabled())
  {
    xArguments.push_back("--enable-installer");
  }
  else
  {
    xArguments.push_back("--disable-installer");
  }

  xArguments.push_back("--miktex-disable-maintenance");
  xArguments.push_back("--miktex-disable-diagnose");

  this->ctx->processRunner->RunProcess(exe, xArguments);
}

vector<FormatInfo> FormatsManager::Formats()
{
  return this->ctx->session->GetFormats();
}

FormatInfo FormatsManager::Format(const string& formatKey)
{
    FormatInfo formatInfo;
    if (!this->ctx->session->TryGetFormatInfo(formatKey, formatInfo))
    {
        this->ctx->ui->FatalError(fmt::format(T_("{0}: unknown format"), Q_(formatKey)));
    }
    return formatInfo;
}
