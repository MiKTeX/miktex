/**
 * @file miktex/Util/DateUtil.h
 * @author Christian Schenk
 * @brief DateUtil class
 *
 * @copyright Copyright © 2020-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include <miktex/Util/config.h>

#include <ctime>

MIKTEX_UTIL_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE DateUtil
{

public:

    DateUtil() = delete;
    DateUtil(const DateUtil& other) = delete;
    DateUtil& operator=(const DateUtil& other) = delete;
    DateUtil(DateUtil&& other) = delete;
    DateUtil& operator=(DateUtil&& other) = delete;
    ~DateUtil() = delete;

    static constexpr std::time_t UNDEFINED_TIME_T_VALUE = static_cast<std::time_t>(0);

    static bool IsDefined(std::time_t time)
    {
        return time != static_cast<std::time_t>(0) && time != static_cast<std::time_t>(-1);
    }
};

MIKTEX_UTIL_END_NAMESPACE;
