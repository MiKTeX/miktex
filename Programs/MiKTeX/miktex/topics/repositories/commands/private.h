/**
 * @file topics/repositories/commands/private.h
 * @author Christian Schenk
 * @brief Private parts
 *
 * @copyright Copyright © 2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <string>

#include <miktex/PackageManager/PackageManager>
#include <miktex/Util/StringUtil>

#include "internal.h"

class CountryComparer
{
public:
    inline bool operator() (const MiKTeX::Packages::RepositoryInfo& lhs, const MiKTeX::Packages::RepositoryInfo& rhs)
    {
        if (lhs.ranking == rhs.ranking)
        {
            return MiKTeX::Util::StringCompare(lhs.country.c_str(), rhs.country.c_str(), true) < 0;
        }
        else
        {
            return lhs.ranking < rhs.ranking;
        }
    }
};


std::string Format(const std::string& outputTemplate, const MiKTeX::Packages::RepositoryInfo& repositoryInfo);
