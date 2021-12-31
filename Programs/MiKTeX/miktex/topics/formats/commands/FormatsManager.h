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

#include "internal.h"

class FormatsManager
{
public:

    void Init(OneMiKTeXUtility::ApplicationContext& ctx);
    void Update();

private:

};
