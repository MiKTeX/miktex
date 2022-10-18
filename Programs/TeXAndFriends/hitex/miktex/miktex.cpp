/**
 * @file miktex.cpp
 * @author Christian Schenk
 * @brief MiKTeX HiTeX
 *
 * @copyright Copyright © 2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include <cstdio>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Utils>

#include "hitex.h"

using namespace MiKTeX::Core;

void miktex_print_banner(FILE* file)
{
    fputs(fmt::format(" ({0})", Utils::GetMiKTeXBannerString()).c_str(), file);
}
