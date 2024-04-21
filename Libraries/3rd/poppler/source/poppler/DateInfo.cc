//========================================================================
//
// DateInfo.cc
//
// Copyright (C) 2008, 2018, 2019, 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright (C) 2015 André Esser <bepandre@hotmail.com>
// Copyright (C) 2016, 2018, 2021 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by Technische Universität Dresden
// Copyright (C) 2021 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2024 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
// Copyright (C) 2024 Erich E. Hoover <erich.e.hoover@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

//========================================================================
//
// Based on code from pdfinfo.cc
//
// Copyright 1998-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#include "glibc.h"
#include "gmem.h"
#include "DateInfo.h"
#include "UTF.h"

#include <cstdio>
#include <cstring>

/* See PDF Reference 1.3, Section 3.8.2 for PDF Date representation */
bool parseDateString(const GooString *date, int *year, int *month, int *day, int *hour, int *minute, int *second, char *tz, int *tzHour, int *tzMinute)
{
    std::vector<Unicode> u = TextStringToUCS4(date->toStr());
    GooString s;
    for (auto &c : u) {
        // Ignore any non ASCII characters
        if (c < 128) {
            s.append(c);
        }
    }
    const char *dateString = s.c_str();

    if (strlen(dateString) < 2) {
        return false;
    }

    if (dateString[0] == 'D' && dateString[1] == ':') {
        dateString += 2;
    }

    *month = 1;
    *day = 1;
    *hour = 0;
    *minute = 0;
    *second = 0;
    *tz = 0x00;
    *tzHour = 0;
    *tzMinute = 0;

    if (sscanf(dateString, "%4d%2d%2d%2d%2d%2d%c%2d%*c%2d", year, month, day, hour, minute, second, tz, tzHour, tzMinute) > 0) {
        /* Workaround for y2k bug in Distiller 3 stolen from gpdf, hoping that it won't
         * be used after y2.2k */
        if (*year < 1930 && strlen(dateString) > 14) {
            int century, years_since_1900;
            if (sscanf(dateString, "%2d%3d%2d%2d%2d%2d%2d", &century, &years_since_1900, month, day, hour, minute, second) == 7) {
                *year = century * 100 + years_since_1900;
            } else {
                return false;
            }
        }

        if (*year <= 0) {
            return false;
        }

        return true;
    }

    return false;
}

std::string timeToStringWithFormat(const time_t *timeA, const char *format)
{
    const time_t timet = timeA ? *timeA : time(nullptr);

    struct tm localtime_tm;
    localtime_r(&timet, &localtime_tm);

    char timeOffset[12];

    // strftime "%z" does not work on windows (it prints zone name, not offset)
    // calculate time zone offset by comparing local and gmtime time_t value for same
    // time.
    const time_t timeg = timegm(&localtime_tm);
    const int offset = static_cast<int>(difftime(timeg, timet)); // find time zone offset in seconds
    if (offset > 0) {
        snprintf(timeOffset, sizeof(timeOffset), "+%02d'%02d'", offset / 3600, (offset % 3600) / 60);
    } else if (offset < 0) {
        snprintf(timeOffset, sizeof(timeOffset), "-%02d'%02d'", -offset / 3600, (-offset % 3600) / 60);
    } else {
        snprintf(timeOffset, sizeof(timeOffset), "Z");
    }
    std::string fmt(format);
    const char timeOffsetPattern[] = "%z";
    size_t timeOffsetPosition = fmt.find(timeOffsetPattern);
    if (timeOffsetPosition != std::string::npos) {
        fmt.replace(timeOffsetPosition, sizeof(timeOffsetPattern) - 1, timeOffset);
    }

    if (fmt.length() == 0) {
        return "";
    }
    size_t bufLen = 50;
    std::string buf(bufLen, ' ');
    while (strftime(&buf[0], buf.size(), fmt.c_str(), &localtime_tm) == 0) {
        buf.resize(bufLen *= 2);
    }
    return buf;
}

GooString *timeToDateString(const time_t *timeA)
{
    return new GooString(timeToStringWithFormat(timeA, "D:%Y%m%d%H%M%S%z"));
}

// Convert PDF date string to time. Returns -1 if conversion fails.
time_t dateStringToTime(const GooString *dateString)
{
    int year, mon, day, hour, min, sec, tz_hour, tz_minute;
    char tz;
    struct tm tm;
    time_t time;

    if (!parseDateString(dateString, &year, &mon, &day, &hour, &min, &sec, &tz, &tz_hour, &tz_minute)) {
        return -1;
    }

    tm.tm_year = year - 1900;
    tm.tm_mon = mon - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_wday = -1;
    tm.tm_yday = -1;
    tm.tm_isdst = -1; /* 0 = DST off, 1 = DST on, -1 = don't know */

    /* compute tm_wday and tm_yday and check date */
    time = timegm(&tm);
    if (time == (time_t)-1) {
        return time;
    }

    time_t offset = (tz_hour * 60 + tz_minute) * 60;
    if (tz == '-') {
        offset *= -1;
    }
    time -= offset;

    return time;
}
