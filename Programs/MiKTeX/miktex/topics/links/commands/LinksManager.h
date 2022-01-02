/**
 * @file topic/links/commands/LinksManager.h
 * @author Christian Schenk
 * @brief Links manager
 *
 * @copyright Copyright © 2002-2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <string>
#include <vector>

#include <miktex/Util/OptionSet>

#include "internal.h"

enum class LinkType
{
  Copy,
  Hard,
  Symbolic,
};

struct FileLink
{
  FileLink(const std::string& target, const std::vector<std::string>& linkNames) :
    linkNames(linkNames),
    target(target)
  {
  }
  FileLink(const std::string& target, const std::vector<std::string>& linkNames, LinkType linkType) :
    linkNames(linkNames),
    linkType(linkType),
    target(target)
  {
  }
  std::vector<std::string> linkNames;
#if defined(MIKTEX_WINDOWS)
  LinkType linkType = LinkType::Hard;
#else
  LinkType linkType = LinkType::Symbolic;
#endif
  std::string target;
};

enum class LinkCategory
{
  Formats,
  MiKTeX,
  Scripts,
};

typedef MiKTeX::Util::OptionSet<LinkCategory> LinkCategoryOptions;

class LinksManager
{
public:

    void Init(OneMiKTeXUtility::ApplicationContext& ctx);
    void Install(bool force);
    void Uninstall();

private:

    std::vector<FileLink> CollectLinks(LinkCategoryOptions linkCategories);

    void ManageLinks(LinkCategoryOptions linkCategories, bool uninstall, bool force);

    void ManageLink(const FileLink& fileLink, bool supportsHardLinks, bool isRemoveRequested, bool allowOverwrite);

#if defined(MIKTEX_UNIX)
    void MakeFilesExecutable();
#endif

    OneMiKTeXUtility::ApplicationContext* ctx;

    static const std::vector<FileLink> lua52texLinks;
    static const std::vector<FileLink> miktexFileLinks;
};
