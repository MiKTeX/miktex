//========================================================================
//
// This file is under the GPLv2 or later license
//
// Copyright (C) 2005-2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005, 2009, 2014, 2019, 2020, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011 Simon Kellner <kellner@kit.edu>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef PAGELABELINFO_P_H
#define PAGELABELINFO_P_H

/* http://mathworld.wolfram.com/RomanNumerals.html */

#include "config.h"

#include "goo/GooString.h"
#include "Error.h"

static std::pair<int, bool> fromDecimal(const std::string &str, const bool unicode)
{
    if (unicode && (str.size() % 2 == 0)) {
        if (GooString::hasUnicodeMarker(str)) {
            // strip the marker if it is there
            return fromDecimal(str.substr(2), true /*unicode*/);
        }

        // Since we only care about numbers here, the first byte needs to be
        // 0 and second will be the actual ascii number, so we're going to reconstruct a
        // non unicode string that then we will use strtol to "translate"
        std::string newString;
        bool allGood = true;
        for (size_t i = 0; allGood && i < str.size(); i += 2) {
            if (str[i] == 0) {
                newString += str[i + 1];
            } else {
                allGood = false;
            }
        }

        if (allGood) {
            return fromDecimal(newString, false /*unicode*/);
        }
    }

    const char *const begin = str.data();
    const char *const end = begin + str.size();

    char *parsed;
    const int number = std::strtol(begin, &parsed, 10);
    return std::make_pair(number, parsed >= end);
}

static int fromRoman(const char *buffer)
{
    int digit_value, prev_digit_value, value;
    int i;

    prev_digit_value = INT_MAX;
    value = 0;
    for (i = 0; buffer[i] != '\0'; i++) {
        switch (buffer[i]) {
        case 'm':
        case 'M':
            digit_value = 1000;
            break;
        case 'd':
        case 'D':
            digit_value = 500;
            break;
        case 'c':
        case 'C':
            digit_value = 100;
            break;
        case 'l':
        case 'L':
            digit_value = 50;
            break;
        case 'x':
        case 'X':
            digit_value = 10;
            break;
        case 'v':
        case 'V':
            digit_value = 5;
            break;
        case 'i':
        case 'I':
            digit_value = 1;
            break;
        default:
            return -1;
        }

        if (digit_value <= prev_digit_value) {
            value += digit_value;
        } else {
            value += digit_value - prev_digit_value * 2;
        }
        prev_digit_value = digit_value;
    }

    return value;
}

static void toRoman(int number, GooString *str, bool uppercase)
{
    static const char uppercaseNumerals[] = "IVXLCDM";
    static const char lowercaseNumerals[] = "ivxlcdm";
    int divisor;
    int i, j, k;
    const char *wh;

    if (number >= 4000) {
        error(errUnimplemented, -1, "Conversion to roman numerals of numbers >= 4000 not implemented");
        return;
    }

    if (uppercase) {
        wh = uppercaseNumerals;
    } else {
        wh = lowercaseNumerals;
    }

    divisor = 1000;
    for (k = 3; k >= 0; k--) {
        i = number / divisor;
        number = number % divisor;

        switch (i) {
        case 0:
            break;
        case 5:
            str->append(wh[2 * k + 1]);
            break;
        case 9:
            str->append(wh[2 * k + 0]);
            str->append(wh[2 * k + 2]);
            break;
        case 4:
            str->append(wh[2 * k + 0]);
            str->append(wh[2 * k + 1]);
            break;
        default:
            if (i > 5) {
                str->append(wh[2 * k + 1]);
                i -= 5;
            }
            for (j = 0; j < i; j++) {
                str->append(wh[2 * k + 0]);
            }
        }

        divisor = divisor / 10;
    }
}

static int fromLatin(const char *buffer)
{
    const char *p;

    for (p = buffer; *p; p++) {
        if (*p != buffer[0]) {
            return -1;
        }
    }

    const intptr_t diff = p - buffer;
    if (unlikely(diff > std::numeric_limits<int>::max() / 100)) {
        error(errUnimplemented, -1, "Something went wrong in fromLatin conversion");
        return -1;
    }
    const int count = static_cast<int>(diff);
    if (buffer[0] >= 'a' && buffer[0] <= 'z') {
        return 26 * (count - 1) + buffer[0] - 'a' + 1;
    }
    if (buffer[0] >= 'A' && buffer[0] <= 'Z') {
        return 26 * (count - 1) + buffer[0] - 'A' + 1;
    }

    return -1;
}

static void toLatin(int number, GooString *str, bool uppercase)
{
    char base, letter;
    int i, count;

    if (uppercase) {
        base = 'A';
    } else {
        base = 'a';
    }

    count = (number - 1) / 26 + 1;
    letter = base + (number - 1) % 26;

    for (i = 0; i < count; i++) {
        str->append(letter);
    }
}

#endif
