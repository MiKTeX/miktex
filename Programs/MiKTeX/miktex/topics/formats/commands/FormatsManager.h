/**
 * @file topic/formats/commands/FormatsManager.h
 * @author Christian Schenk
 * @brief Update format files
 *
 * @copyright Copyright © 2002-2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 *
 * The code in this file is based on the updmap Perl script
 * (updmap.pl):
 */

#include <string>
#include <vector>

#include <miktex/Core/Session>

#include "internal.h"

class FormatsManager
{
public:

    MiKTeX::Core::FormatInfo Format(const std::string& formatKey);
    std::vector<MiKTeX::Core::FormatInfo> Formats();
    void Init(OneMiKTeXUtility::ApplicationContext& ctx);
    void Update(const std::string& formatKey);

private:

    void RunMakeTeX(const std::string& makeProg, const std::vector<std::string>& arguments);

    OneMiKTeXUtility::ApplicationContext* ctx;
    std::vector<std::string> formatsMade;
};
