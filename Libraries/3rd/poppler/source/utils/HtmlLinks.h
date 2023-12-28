//========================================================================
//
// This file comes from pdftohtml project
// http://pdftohtml.sourceforge.net
//
// Copyright from:
// Gueorgui Ovtcharov
// Rainer Dorsch <http://www.ra.informatik.uni-stuttgart.de/~rainer/>
// Mikhail Kruk <meshko@cs.brandeis.edu>
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2010, 2018, 2021, 2022 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef _HTML_LINKS
#define _HTML_LINKS

#include <cstdlib>
#include <cstring>
#include <vector>
#include "goo/GooString.h"

class HtmlLink
{

private:
    double Xmin;
    double Ymin;
    double Xmax;
    double Ymax;
    GooString *dest;

public:
    HtmlLink(const HtmlLink &x);
    HtmlLink(double xmin, double ymin, double xmax, double ymax, GooString *_dest);
    ~HtmlLink();
    HtmlLink &operator=(const HtmlLink &) = delete;
    bool isEqualDest(const HtmlLink &x) const;
    GooString *getDest() const { return new GooString(dest); }
    double getX1() const { return Xmin; }
    double getX2() const { return Xmax; }
    double getY1() const { return Ymin; }
    double getY2() const { return Ymax; }
    bool inLink(double xmin, double ymin, double xmax, double ymax) const;
    // GooString *Link(GooString *content);
    GooString *getLinkStart() const;
};

class HtmlLinks
{
private:
    std::vector<HtmlLink> accu;

public:
    HtmlLinks();
    ~HtmlLinks();
    HtmlLinks(const HtmlLinks &) = delete;
    HtmlLinks &operator=(const HtmlLinks &) = delete;
    void AddLink(const HtmlLink &x) { accu.push_back(x); }
    bool inLink(double xmin, double ymin, double xmax, double ymax, size_t &p) const;
    const HtmlLink *getLink(size_t i) const;
};

#endif
