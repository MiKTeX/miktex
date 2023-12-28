//========================================================================
//
// This file is under the GPLv2 or later license
//
// Copyright (C) 2005-2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005, 2018-2020, 2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef PAGELABELINFO_H
#define PAGELABELINFO_H

#include <climits>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cassert>
#include <string>
#include <vector>

#include "Object.h"

class PageLabelInfo
{
public:
    PageLabelInfo(Object *tree, int numPages);

    PageLabelInfo(const PageLabelInfo &) = delete;
    PageLabelInfo &operator=(const PageLabelInfo &) = delete;

    bool labelToIndex(GooString *label, int *index) const;
    bool indexToLabel(int index, GooString *label) const;

private:
    void parse(const Object *tree, RefRecursionChecker &parsedRefs);

private:
    struct Interval
    {
        Interval(Object *dict, int baseA);

        std::string prefix;
        enum NumberStyle
        {
            None,
            Arabic,
            LowercaseRoman,
            UppercaseRoman,
            UppercaseLatin,
            LowercaseLatin
        } style;
        int first, base, length;
    };

    std::vector<Interval> intervals;
};

#endif
