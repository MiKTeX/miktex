/**
 * @file miktex/freeglut.cpp
 * @author Christian Schenk
 * @brief MiKTeX freeglut utilities
 *
 * @copyright Copyright © 2018-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include <miktex/Util/StringUtil>

#include "freeglut.h"

using namespace MiKTeX::Util;

wchar_t* miktex_uw_strdup(const char* s)
{
    return _wcsdup(StringUtil::UTF8ToWideChar(s).c_str());
}
