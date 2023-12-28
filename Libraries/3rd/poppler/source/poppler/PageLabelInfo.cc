//========================================================================
//
// This file is under the GPLv2 or later license
//
// Copyright (C) 2005-2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005, 2009, 2013, 2017, 2018, 2020, 2021, 2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011 Simon Kellner <kellner@kit.edu>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include <algorithm>

#include "PageLabelInfo.h"
#include "PageLabelInfo_p.h"

PageLabelInfo::Interval::Interval(Object *dict, int baseA)
{
    style = None;
    Object obj = dict->dictLookup("S");
    if (obj.isName()) {
        if (obj.isName("D")) {
            style = Arabic;
        } else if (obj.isName("R")) {
            style = UppercaseRoman;
        } else if (obj.isName("r")) {
            style = LowercaseRoman;
        } else if (obj.isName("A")) {
            style = UppercaseLatin;
        } else if (obj.isName("a")) {
            style = LowercaseLatin;
        }
    }

    obj = dict->dictLookup("P");
    if (obj.isString()) {
        const auto str = obj.getString();
        prefix.assign(str->c_str(), str->getLength());
    }

    obj = dict->dictLookup("St");
    if (obj.isInt()) {
        first = obj.getInt();
    } else {
        first = 1;
    }

    base = baseA;
}

PageLabelInfo::PageLabelInfo(Object *tree, int numPages)
{
    RefRecursionChecker alreadyParsedRefs;
    parse(tree, alreadyParsedRefs);

    if (intervals.empty()) {
        return;
    }

    auto curr = intervals.begin();
    for (auto next = curr + 1; next != intervals.end(); ++next, ++curr) {
        curr->length = std::max(0, next->base - curr->base);
    }
    curr->length = std::max(0, numPages - curr->base);
}

void PageLabelInfo::parse(const Object *tree, RefRecursionChecker &alreadyParsedRefs)
{
    // leaf node
    Object nums = tree->dictLookup("Nums");
    if (nums.isArray()) {
        for (int i = 0; i < nums.arrayGetLength(); i += 2) {
            Object obj = nums.arrayGet(i);
            if (!obj.isInt()) {
                continue;
            }
            const int base = obj.getInt();
            if (base < 0) {
                continue;
            }
            obj = nums.arrayGet(i + 1);
            if (!obj.isDict()) {
                continue;
            }

            intervals.emplace_back(&obj, base);
        }
    }

    Object kids = tree->dictLookup("Kids");
    if (kids.isArray()) {
        const Array *kidsArray = kids.getArray();
        for (int i = 0; i < kidsArray->getLength(); ++i) {
            Ref ref;
            const Object kid = kidsArray->get(i, &ref);
            if (!alreadyParsedRefs.insert(ref)) {
                error(errSyntaxError, -1, "loop in PageLabelInfo (ref.num: {0:d})", ref.num);
                continue;
            }
            if (kid.isDict()) {
                parse(&kid, alreadyParsedRefs);
            }
        }
    }
}

bool PageLabelInfo::labelToIndex(GooString *label, int *index) const
{
    const char *const str = label->c_str();
    const std::size_t strLen = label->getLength();
    const bool strUnicode = label->hasUnicodeMarker();
    int number;
    bool ok;

    for (const auto &interval : intervals) {
        const std::size_t prefixLen = interval.prefix.size();
        if (strLen < prefixLen || interval.prefix.compare(0, prefixLen, str, prefixLen) != 0) {
            continue;
        }

        switch (interval.style) {
        case Interval::Arabic:
            std::tie(number, ok) = fromDecimal(label->toStr().substr(prefixLen), strUnicode);
            if (ok && number - interval.first < interval.length) {
                *index = interval.base + number - interval.first;
                return true;
            }
            break;
        case Interval::LowercaseRoman:
        case Interval::UppercaseRoman:
            number = fromRoman(str + prefixLen);
            if (number >= 0 && number - interval.first < interval.length) {
                *index = interval.base + number - interval.first;
                return true;
            }
            break;
        case Interval::UppercaseLatin:
        case Interval::LowercaseLatin:
            number = fromLatin(str + prefixLen);
            if (number >= 0 && number - interval.first < interval.length) {
                *index = interval.base + number - interval.first;
                return true;
            }
            break;
        case Interval::None:
            if (interval.length == 1 && label->toStr() == interval.prefix) {
                *index = interval.base;
                return true;
            } else {
                error(errSyntaxError, -1, "asking to convert label to page index in an unknown scenario, report a bug");
            }
            break;
        }
    }

    return false;
}

bool PageLabelInfo::indexToLabel(int index, GooString *label) const
{
    char buffer[32];
    int base, number;
    const Interval *matching_interval;
    GooString number_string;

    base = 0;
    matching_interval = nullptr;
    for (const auto &interval : intervals) {
        if (base <= index && index < base + interval.length) {
            matching_interval = &interval;
            break;
        }
        base += interval.length;
    }

    if (!matching_interval) {
        return false;
    }

    number = index - base + matching_interval->first;
    switch (matching_interval->style) {
    case Interval::Arabic:
        snprintf(buffer, sizeof(buffer), "%d", number);
        number_string.append(buffer);
        break;
    case Interval::LowercaseRoman:
        toRoman(number, &number_string, false);
        break;
    case Interval::UppercaseRoman:
        toRoman(number, &number_string, true);
        break;
    case Interval::LowercaseLatin:
        toLatin(number, &number_string, false);
        break;
    case Interval::UppercaseLatin:
        toLatin(number, &number_string, true);
        break;
    case Interval::None:
        break;
    }

    label->clear();
    label->append(matching_interval->prefix.c_str(), matching_interval->prefix.size());
    if (label->hasUnicodeMarker()) {
        int i, len;
        char ucs2_char[2];

        /* Convert the ascii number string to ucs2 and append. */
        len = number_string.getLength();
        ucs2_char[0] = 0;
        for (i = 0; i < len; ++i) {
            ucs2_char[1] = number_string.getChar(i);
            label->append(ucs2_char, 2);
        }
    } else {
        label->append(&number_string);
    }

    return true;
}
